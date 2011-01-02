/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file PWScore.cpp
//-----------------------------------------------------------------------------

#include "PWScore.h"
#include "core.h"
#include "BlowFish.h"
#include "PWSprefs.h"
#include "PWSrand.h"
#include "Util.h"
#include "UUIDGen.h"
#include "SysInfo.h"
#include "UTF8Conv.h"
#include "Report.h"
#include "VerifyFormat.h"
#include "PWSfileV3.h" // XXX cleanup with dynamic_cast
#include "StringXStream.h"

#include "os/typedefs.h"
#include "os/dir.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/mem.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <iterator>

using namespace std;

unsigned char PWScore::m_session_key[20];
unsigned char PWScore::m_session_salt[20];
unsigned char PWScore::m_session_initialized = false;
Asker *PWScore::m_pAsker = NULL;
Reporter *PWScore::m_pReporter = NULL;

uuid_array_t PWScore::NULL_UUID = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

PWScore::PWScore() : 
                     m_currfile(_T("")),
                     m_passkey(NULL), m_passkey_len(0),
                     m_lockFileHandle(INVALID_HANDLE_VALUE),
                     m_lockFileHandle2(INVALID_HANDLE_VALUE),
                     m_LockCount(0), m_LockCount2(0),
                     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
                     m_bDBChanged(false), m_bDBPrefsChanged(false),
                     m_IsReadOnly(false), m_bUniqueGTUValidated(false), 
                     m_nRecordsWithUnknownFields(0),
                     m_bNotifyDB(false), m_pUIIF(NULL), m_fileSig(NULL)
{
  // following should ideally be wrapped in a mutex
  if (!PWScore::m_session_initialized) {
    PWScore::m_session_initialized = true;
    CItemData::SetSessionKey(); // per-session initialization
    pws_os::mlock(m_session_key, sizeof(m_session_key));
    PWSrand::GetInstance()->GetRandomData(m_session_key, sizeof(m_session_key));
    PWSrand::GetInstance()->GetRandomData(m_session_salt,
                                          sizeof(m_session_salt));
  }
  m_undo_iter = m_redo_iter = m_vpcommands.end();
}

PWScore::~PWScore()
{
  // do NOT trash m_session_*, as there may be other cores around
  // relying on it. Trashing the ciphertext encrypted with it is enough
  const unsigned int BS = BlowFish::BLOCKSIZE;
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS - 1)) / BS) * BS);
    delete[] m_passkey;
    m_passkey = NULL;
    m_passkey_len = 0;
  }

  m_UHFL.clear();
  m_vnodes_modified.clear();

  delete m_fileSig;
}

void PWScore::SetApplicationNameAndVersion(const stringT &appName,
                                           DWORD dwMajorMinor)
{
  int nMajor = HIWORD(dwMajorMinor);
  int nMinor = LOWORD(dwMajorMinor);
  Format(m_AppNameAndVersion, _T("%s V%d.%02d"), appName.c_str(),
         nMajor, nMinor);
}

// Return whether first [g:t:u] is greater than the second [g:t:u]
// used in std::sort in SortDependents below.
static bool GTUCompare(const StringX &elem1, const StringX &elem2)
{
  StringX::size_type i1 = elem1.find(_T(':'));
  StringX g1 = (i1 == StringX::npos) ? elem1 : elem1.substr(0, i1);
  StringX::size_type i2 = elem2.find(_T(':'));
  StringX g2 = (i2 == StringX::npos) ? elem2 : elem2.substr(0, i2);
  if (g1 != g2)
    return g1.compare(g2) < 0;

  StringX tmp1 = elem1.substr(g1.length() + 1);
  StringX tmp2 = elem2.substr(g2.length() + 1);
  i1 = tmp1.find(_T(':'));
  StringX t1 = (i1 == StringX::npos) ? tmp1 : tmp1.substr(0, i1);
  i2 = tmp2.find(_T(':'));
  StringX t2 = (i2 == StringX::npos) ? tmp2 : tmp2.substr(0, i2);
  if (t1 != t2)
    return t1.compare(t2) < 0;

  tmp1 = tmp1.substr(t1.length() + 1);
  tmp2 = tmp2.substr(t2.length() + 1);
  i1 = tmp1.find(_T(':'));
  StringX u1 = (i1 == StringX::npos) ? tmp1 : tmp1.substr(0, i1);
  i2 = tmp2.find(_T(':'));
  StringX u2 = (i2 == StringX::npos) ? tmp2 : tmp2.substr(0, i2);
  return u1.compare(u2) < 0;
}

void PWScore::SortDependents(UUIDVector &dlist, StringX &csDependents)
{
  std::vector<StringX> sorted_dependents;
  std::vector<StringX>::iterator sd_iter;

  ItemListIter iter;
  UUIDVectorIter diter;
  StringX cs_dependent;

  for (diter = dlist.begin(); diter != dlist.end(); diter++) {
    uuid_array_t dependent_uuid;
    diter->GetUUID(dependent_uuid);
    iter = Find(dependent_uuid);
    if (iter != GetEntryEndIter()) {
      cs_dependent = iter->second.GetGroup() + _T(":") +
                     iter->second.GetTitle() + _T(":") +
                     iter->second.GetUser();
      sorted_dependents.push_back(cs_dependent);
    }
  }

  std::sort(sorted_dependents.begin(), sorted_dependents.end(), GTUCompare);
  csDependents.clear();

  for (sd_iter = sorted_dependents.begin(); sd_iter != sorted_dependents.end(); sd_iter++)
    csDependents += _T("\t[") +  *sd_iter + _T("]\r\n");
}

void PWScore::DoAddEntry(const CItemData &item)
{
  // Also "UndoDeleteEntry" !
  uuid_array_t entry_uuid;
  item.GetUUID(entry_uuid);
  ASSERT(m_pwlist.find(entry_uuid) == m_pwlist.end());
  m_pwlist[entry_uuid] = item;

  if (item.NumberUnknownFields() > 0)
    IncrementNumRecordsWithUnknownFields();

  m_bDBChanged = true;
}

bool PWScore::ConfirmDelete(const CItemData *pci)
{
  ASSERT(pci != NULL);
  if (pci->IsBase() && m_pAsker != NULL) {
    UUIDVector dependentslist;
    uuid_array_t entry_uuid;
    pci->GetUUID(entry_uuid);
    CItemData::EntryType entrytype = pci->GetEntryType();

    // If we're deleting a base (entry with aliases or shortcuts
    // referencing it), notify user and give her a chance to bail out:
    if (entrytype == CItemData::ET_ALIASBASE)
      GetAllDependentEntries(entry_uuid, dependentslist, CItemData::ET_ALIAS);
    else if (entrytype == CItemData::ET_SHORTCUTBASE)
      GetAllDependentEntries(entry_uuid, dependentslist, CItemData::ET_SHORTCUT);

    size_t num_dependents = dependentslist.size();
    ASSERT(num_dependents > 0); // otherwise pci shouldn't be a base!
    if (num_dependents > 0) {
      StringX csDependents;
      SortDependents(dependentslist, csDependents);

      stringT cs_msg, cs_type;
      stringT cs_title;
      LoadAString(cs_title, IDSC_DELETEBASET);
      if (entrytype == CItemData::ET_ALIASBASE) {
        LoadAString(cs_type, num_dependents == 1 ? IDSC_ALIAS : IDSC_ALIASES);
        Format(cs_msg, IDSC_DELETEABASE, dependentslist.size(),
               cs_type.c_str(), csDependents.c_str());
      } else {
        LoadAString(cs_type, num_dependents == 1 ? IDSC_SHORTCUT : IDSC_SHORTCUTS);
        Format(cs_msg, IDSC_DELETESBASE, dependentslist.size(),
               cs_type.c_str(), csDependents.c_str());
      }
        return (*m_pAsker)(cs_title, cs_msg);
    } // num_dependents > 0
  } 
  return true;
}

void PWScore::DoDeleteEntry(const CItemData &item)
{
  // Also "UndoAddEntry" !

  // Added handling of alias/shortcut/base entry types
  // Most of this will go away once the entry types
  // are implemented as subclasses.

  uuid_array_t entry_uuid;
  item.GetUUID(entry_uuid);
  ItemListIter pos = m_pwlist.find(entry_uuid);
  if (pos != m_pwlist.end()) {
    // Simple cases first: Aliases or shortcuts, update maps
    // and refresh base's display, if changed
    CItemData::EntryType entrytype = item.GetEntryType();
    uuid_array_t base_uuid;
    if (item.IsDependent()) {
      GetDependentEntryBaseUUID(entry_uuid, base_uuid, entrytype);
      DoRemoveDependentEntry(base_uuid, entry_uuid, entrytype);
    } else if (item.IsAliasBase()) {
      ResetAllAliasPasswords(entry_uuid);
    } else if (item.IsShortcutBase()) {
      // Recurse on all dependents
      ItemMMap deps(m_base2shortcuts_mmap.lower_bound(entry_uuid),
                    m_base2shortcuts_mmap.upper_bound(entry_uuid));
      for (ItemMMapIter iter = deps.begin(); iter != deps.end(); iter++) {
        uuid_array_t dep_uuid;
        iter->second.GetUUID(dep_uuid);
        CItemData depItem = Find(dep_uuid)->second;
        DoDeleteEntry(depItem);
        // Set deleted for GUIRefreshEntry() which will remove from display
        depItem.SetStatus(CItemData::ES_DELETED);
        GUIRefreshEntry(depItem);
      }
    }

    m_bDBChanged = true;
    m_pwlist.erase(pos); // at last!
    if (item.NumberUnknownFields() > 0)
      DecrementNumRecordsWithUnknownFields();

    NotifyDBModified();
  } // pos != m_pwlist.end()
}

void PWScore::DoReplaceEntry(const CItemData &old_ci, const CItemData &new_ci)
{
  // Assumes that old_uuid == new_uuid
  uuid_array_t old_uuid, new_uuid;
  old_ci.GetUUID(old_uuid);
  new_ci.GetUUID(new_uuid);
  ASSERT(memcmp(old_uuid, new_uuid, sizeof(uuid_array_t)) == 0);
  m_pwlist[old_uuid] = new_ci;
  if (old_ci.GetEntryType() != new_ci.GetEntryType() || old_ci.IsProtected() != new_ci.IsProtected())
    GUIRefreshEntry(new_ci);
  m_bDBChanged = true;
}

void PWScore::ClearData(void)
{
  const unsigned int BS = BlowFish::BLOCKSIZE;
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS - 1)) / BS) * BS);
    delete[] m_passkey;
    m_passkey = NULL;
    m_passkey_len = 0;
  }
  m_passkey = NULL;

  //Composed of ciphertext, so doesn't need to be overwritten
  m_pwlist.clear();

  // Clear out out dependents mappings
  m_base2aliases_mmap.clear();
  m_alias2base_map.clear();
  m_base2shortcuts_mmap.clear();
  m_shortcut2base_map.clear();

  // Clear out unknown fields
  m_UHFL.clear();

  // Clear out database filters
  m_MapFilters.clear();

  // Clear out commands
  ClearCommands();
}

void PWScore::ReInit(bool bNewFile)
{
  // Now reset all values as if created from new
  if (bNewFile)
    m_ReadFileVersion = PWSfile::NEWFILE;
  else
    m_ReadFileVersion = PWSfile::UNKNOWN_VERSION;

  const unsigned int BS = BlowFish::BLOCKSIZE;
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS - 1)) / BS) * BS);
    delete[] m_passkey;
    m_passkey = NULL;
    m_passkey_len = 0;
  }

  m_nRecordsWithUnknownFields = 0;
  m_UHFL.clear();
  ClearChangedNodes();

  SetChanged(false, false);
}

void PWScore::NewFile(const StringX &passkey)
{
  ClearData();
  SetPassKey(passkey);
  m_ReadFileVersion = PWSfile::VCURRENT;
  SetChanged(false, false);
}

// functor object type for for_each:
struct RecordWriter {
  RecordWriter(PWSfile *pout, PWScore *pcore) : m_pout(pout), m_pcore(pcore) {}
  void operator()(pair<CUUIDGen const, CItemData> &p)
  {
    StringX savePassword;

    savePassword = p.second.GetPassword();
    if (p.second.IsAlias()) {
      uuid_array_t item_uuid, base_uuid;
      p.second.GetUUID(item_uuid);
      m_pcore->GetDependentEntryBaseUUID(item_uuid, base_uuid, CItemData::ET_ALIAS);

      const CUUIDGen buuid(base_uuid);
      StringX uuid_str(_T("[["));
      uuid_str += buuid.GetHexStr();
      uuid_str += _T("]]");
      p.second.SetPassword(uuid_str);
    } else if (p.second.IsShortcut()) {
      uuid_array_t item_uuid, base_uuid;
      p.second.GetUUID(item_uuid);
      m_pcore->GetDependentEntryBaseUUID(item_uuid, base_uuid, CItemData::ET_SHORTCUT);

      const CUUIDGen buuid(base_uuid);
      StringX uuid_str(_T("[~"));
      uuid_str += buuid.GetHexStr();
      uuid_str += _T("~]");
      p.second.SetPassword(uuid_str);
    }
 
    m_pout->WriteRecord(p.second);
    p.second.SetPassword(savePassword);
    p.second.ClearStatus();
  }

private:
  PWSfile *m_pout;
  PWScore *m_pcore;
};

int PWScore::WriteFile(const StringX &filename, PWSfile::VERSION version)
{
  int status;
  PWSfile *out = PWSfile::MakePWSfile(filename, version,
                                      PWSfile::Write, status);

  if (status != PWSfile::SUCCESS) {
    delete out;
    return status;
  }

  // since we're writing a new file, the previous sig's
  // about to be invalidated
  delete m_fileSig;
  m_fileSig = NULL;

  m_hdr.m_prefString = PWSprefs::GetInstance()->Store();
  m_hdr.m_whatlastsaved = m_AppNameAndVersion.c_str();
  m_hdr.m_RUEList = m_RUEList;

  out->SetHeader(m_hdr);

  // Give PWSfileV3 the unknown headers to write out
  // XXX cleanup gross dynamic_cast
  PWSfileV3 *out3 = dynamic_cast<PWSfileV3 *>(out);
  if (out3 != NULL) {
    out3->SetUnknownHeaderFields(m_UHFL);
    out3->SetFilters(m_MapFilters); // Give it the filters to write out
  }

  try { // exception thrown on write error
    status = out->Open(GetPassKey());

    if (status != PWSfile::SUCCESS) {
      delete out;
      return status;
    }

    RecordWriter write_record(out, this);
    for_each(m_pwlist.begin(), m_pwlist.end(), write_record);

    m_hdr = out->GetHeader(); // update time saved, etc.
  } catch (...) {
    out->Close();
    delete out;
    return FAILURE;
  }
  out->Close();
  delete out;

  SetChanged(false, false);

  m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]

  return SUCCESS;
}

void PWScore::ClearCommands()
{
  while (!m_vpcommands.empty()) {
    delete m_vpcommands.back();
    m_vpcommands.pop_back();
  }
  m_undo_iter = m_redo_iter = m_vpcommands.end();
}

void PWScore::ResetStateAfterSave()
{
  // After a Save/SaveAs, need to change all saved DB modified states
  // in any commands in the list that can be undone or redone
  // State = whether DB changed (false: no, true: yes)

  // After a save, all commands in the undo/redo chain should now have 
  //   saved state = changed
  std::vector<Command *>::iterator cmd_iter;
  for (cmd_iter = m_vpcommands.begin(); cmd_iter != m_vpcommands.end(); cmd_iter++) {
    (*cmd_iter)->ResetSavedState(true);
  }

  // However, the next command in the list (redo) should now have
  //   saved state = unchanged. 
  if (m_redo_iter != m_vpcommands.end())
    (*m_redo_iter)->ResetSavedState(false);
}

int PWScore::Execute(Command *pcmd)
{
  pws_os::Trace(_T("PWScore Execute-Start: m_vpcommands.size()=%d; undo offset=%d; redo offset=%d\n"),
        m_vpcommands.size(),
        (m_undo_iter != m_vpcommands.end()) ? distance(m_undo_iter, m_vpcommands.begin()) : -1,
        (m_redo_iter != m_vpcommands.end()) ? distance(m_redo_iter, m_vpcommands.begin()) : -1);

  if (m_redo_iter != m_vpcommands.end()) {
    std::vector<Command *>::iterator cmd_Iter;

    for (cmd_Iter = m_redo_iter; cmd_Iter != m_vpcommands.end(); cmd_Iter++) {
      delete (*cmd_Iter);
    }
    m_vpcommands.erase(m_redo_iter, m_vpcommands.end());
  }

  m_vpcommands.push_back(pcmd);
  m_undo_iter = m_redo_iter = m_vpcommands.end();
  int rc = pcmd->Execute();
  m_undo_iter--;

  uuid_array_t entry_uuid = {'0'};  // Valid value not required for this particular call.
  NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR, entry_uuid);

  pws_os::Trace(_T("PWScore Execute-End: m_vpcommands.size()=%d; undo offset=%d; redo offset=%d\n"),
        m_vpcommands.size(),
        (m_undo_iter != m_vpcommands.end()) ? distance(m_undo_iter, m_vpcommands.begin()) : -1,
        (m_redo_iter != m_vpcommands.end()) ? distance(m_redo_iter, m_vpcommands.begin()) : -1);

  return rc;
}

void PWScore::Undo()
{
  pws_os::Trace(_T("PWScore Undo-Start: m_vpcommands.size()=%d; undo offset=%d; redo offset=%d\n"),
        m_vpcommands.size(),
        (m_undo_iter != m_vpcommands.end()) ? distance(m_undo_iter, m_vpcommands.begin()) : -1,
        (m_redo_iter != m_vpcommands.end()) ? distance(m_redo_iter, m_vpcommands.begin()) : -1);

  ASSERT(m_undo_iter != m_vpcommands.end());
  m_redo_iter = m_undo_iter;

  (*m_undo_iter)->Undo();

  if (m_undo_iter == m_vpcommands.begin())
    m_undo_iter = m_vpcommands.end();
  else
    m_undo_iter--;

  uuid_array_t entry_uuid = {'0'};  // Valid value not required for this particular call.
  NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR, entry_uuid);

  pws_os::Trace(_T("PWScore Undo-End  : m_vpcommands.size()=%d; undo offset=%d; redo offset=%d\n"),
        m_vpcommands.size(),
        (m_undo_iter != m_vpcommands.end()) ? distance(m_undo_iter, m_vpcommands.begin()) : -1,
        (m_redo_iter != m_vpcommands.end()) ? distance(m_redo_iter, m_vpcommands.begin()) : -1);
}

void PWScore::Redo()
{
  pws_os::Trace(_T("PWScore Redo-Start: m_vpcommands.size()=%d; undo offset=%d; redo offset=%d\n"),
        m_vpcommands.size(),
        (m_undo_iter != m_vpcommands.end()) ? distance(m_undo_iter, m_vpcommands.begin()) : -1,
        (m_redo_iter != m_vpcommands.end()) ? distance(m_redo_iter, m_vpcommands.begin()) : -1);

  ASSERT(m_redo_iter != m_vpcommands.end());
  m_undo_iter = m_redo_iter;

  (*m_redo_iter)->Redo();

  if (m_redo_iter != m_vpcommands.end())
    m_redo_iter++;

  uuid_array_t entry_uuid = {'0'};  // Valid value not required for this particular call.
  NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR, entry_uuid);

  pws_os::Trace(_T("PWScore Redo-End  : m_vpcommands.size()=%d; undo offset=%d; redo offset=%d\n"),
        m_vpcommands.size(),
        (m_undo_iter != m_vpcommands.end()) ? distance(m_undo_iter, m_vpcommands.begin()) : -1,
        (m_redo_iter != m_vpcommands.end()) ? distance(m_redo_iter, m_vpcommands.begin()) : -1);
}

bool PWScore::AnyToUndo() const
{
  return (m_undo_iter != m_vpcommands.end());
}

bool PWScore::AnyToRedo() const
{
  return (m_redo_iter != m_vpcommands.end());
}

int PWScore::CheckPasskey(const StringX &filename, const StringX &passkey)
{
  int status;

  if (!filename.empty())
    status = PWSfile::CheckPasskey(filename, passkey, m_ReadFileVersion);
  else { // can happen if tries to export b4 save
    size_t t_passkey_len = passkey.length();
    if (t_passkey_len != m_passkey_len) // trivial test
      return WRONG_PASSWORD;
    size_t BlockLength = ((m_passkey_len + 7) / 8) * 8;
    unsigned char *t_passkey = new unsigned char[BlockLength];
    LPCTSTR plaintext = LPCTSTR(passkey.c_str());
    EncryptPassword(reinterpret_cast<const unsigned char *>(plaintext), t_passkey_len, t_passkey);
    if (memcmp(t_passkey, m_passkey, BlockLength) == 0)
      status = PWSfile::SUCCESS;
    else
      status = PWSfile::WRONG_PASSWORD;
    delete[] t_passkey;
  }

  switch (status) {
    case PWSfile::SUCCESS:
      return SUCCESS;
    case PWSfile::CANT_OPEN_FILE:
      return CANT_OPEN_FILE;
    case PWSfile::WRONG_PASSWORD:
      return WRONG_PASSWORD;
    default:
      ASSERT(0);
      return status; // should never happen
  }
}

#define MRE_FS _T("\xbb")

int PWScore::ReadFile(const StringX &a_filename,
                      const StringX &a_passkey, const size_t iMAXCHARS)
{
  int status;
  PWSfile *in = PWSfile::MakePWSfile(a_filename, m_ReadFileVersion,
                                     PWSfile::Read, status, m_pAsker, m_pReporter);

  if (status != PWSfile::SUCCESS) {
    delete in;
    return status;
  }

  status = in->Open(a_passkey);

  // in the old times we could open even 1.x files
  // for compatibility reasons, we open them again, to see if this is really a "1.x" file
  if ((m_ReadFileVersion == PWSfile::V20) && (status == PWSfile::WRONG_VERSION)) {
    PWSfile::VERSION tmp_version;  // only for getting compatible to "1.x" files
    tmp_version = m_ReadFileVersion;
    m_ReadFileVersion = PWSfile::V17;

    //Closing previously opened file
    in->Close();
    in->SetCurVersion(PWSfile::V17);
    status = in->Open(a_passkey);
    if (status != PWSfile::SUCCESS) {
      m_ReadFileVersion = tmp_version;
    }
  }

  if (status != PWSfile::SUCCESS) {
    delete in;
    return CANT_OPEN_FILE;
  }

  if (m_ReadFileVersion == PWSfile::UNKNOWN_VERSION) {
    delete in;
    return UNKNOWN_VERSION;
  }

  m_hdr = in->GetHeader();
  m_OrigDisplayStatus = m_hdr.m_displaystatus; // for WasDisplayStatusChanged
  m_RUEList = m_hdr.m_RUEList;

  // Get pref string and tree display status & who saved when
  // all possibly empty!
  PWSprefs *prefs = PWSprefs::GetInstance();
  prefs->Load(m_hdr.m_prefString);

  // prepare handling of pre-2.0 DEFUSERCHR conversion
  if (m_ReadFileVersion == PWSfile::V17) {
    in->SetDefUsername(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
    m_hdr.m_nCurrentMajorVersion = PWSfile::V17;
    m_hdr.m_nCurrentMinorVersion = 0;
  } else {
    // for 2.0 & later...
    in->SetDefUsername(prefs->GetPref(PWSprefs::DefaultUsername).c_str());
  }

  ClearData(); //Before overwriting old data, but after opening the file...
  SetChanged(false, false);

  SetPassKey(a_passkey); // so user won't be prompted for saves

  CItemData ci_temp;
  uuid_array_t base_uuid, temp_uuid;
  StringX csMyPassword, cs_possibleUUID;
  bool go = true;
#ifdef DEMO
  bool limited = false;
#endif

  PWSfileV3 *in3 = dynamic_cast<PWSfileV3 *>(in); // XXX cleanup
  if (in3 != NULL  && !in3->GetFilters().empty())
    m_MapFilters = in3->GetFilters();

  UUIDVector Possible_Aliases, Possible_Shortcuts;
  size_t uimaxsize(0);
  int numlarge(0);
  do {
    ci_temp.Clear(); // Rather than creating a new one each time.
    status = in->ReadRecord(ci_temp);
    switch (status) {
      case PWSfile::FAILURE:
      {
        // Show a useful (?) error message - better than
        // silently losing data (but not by much)
        // Best if title intact. What to do if not?
        if (m_pReporter != NULL) {
          stringT cs_msg, cs_caption;
          LoadAString(cs_caption, IDSC_READ_ERROR);
          Format(cs_msg, IDSC_ENCODING_PROBLEM, ci_temp.GetTitle().c_str());
          cs_msg = cs_caption + _S(": ") + cs_caption;
          (*m_pReporter)(cs_msg);
        }
      }
      // deliberate fall-through
      case PWSfile::SUCCESS:
        if (iMAXCHARS > 0 && ci_temp.GetSize() > iMAXCHARS) {
          numlarge++;
          uimaxsize = MAX(uimaxsize, ci_temp.GetSize());
        }
        uuid_array_t uuid;
        ci_temp.GetUUID(uuid);
        /*
         * If, for some reason, we're reading in a uuid that we already have
         * we will change the uuid, rather than overwrite an entry.
         * This is to protect the user from possible bugs that break
         * the uniqueness requirement of uuids.
         */
         if (m_pwlist.find(uuid) != m_pwlist.end()) {
#if defined( _DEBUG ) || defined( DEBUG )
           pws_os::Trace0(_T("Non-Unique uuid detected:\n"));
           CItemData::FieldBits bf;
           bf.flip();
           StringX dump = ci_temp.GetPlaintext(TCHAR(':'), bf, TCHAR('-'), NULL);
           pws_os::Trace(_T("%s\n"), dump.c_str());
#endif
           ci_temp.CreateUUID(); // replace duplicated uuid
           ci_temp.GetUUID(uuid); // refresh uuid_array
         }
         // following is duplicated in Validate() - need to refactor
         csMyPassword = ci_temp.GetPassword();
         if (csMyPassword.length() == 36) { // look for "[[uuid]]" or "[~uuid~]"
           cs_possibleUUID = csMyPassword.substr(2, 32);  // try to extract uuid
           ToLower(cs_possibleUUID);
           if (((csMyPassword.substr(0, 2) == _T("[[") &&
                 csMyPassword.substr(csMyPassword.length() - 2) == _T("]]")) ||
                (csMyPassword.substr(0, 2) == _T("[~") &&
                 csMyPassword.substr(csMyPassword.length() - 2) == _T("~]"))) &&
               cs_possibleUUID.find_first_not_of(_T("0123456789abcdef")) == 
               StringX::npos) {
             CUUIDGen buuid(cs_possibleUUID.c_str());
             buuid.GetUUID(base_uuid);
             ci_temp.GetUUID(temp_uuid);
             if (csMyPassword.substr(1, 1) == _T("[")) {
               m_alias2base_map[temp_uuid] = base_uuid;
               Possible_Aliases.push_back(temp_uuid);
             } else {
               m_shortcut2base_map[temp_uuid] = base_uuid;
               Possible_Shortcuts.push_back(temp_uuid);
             }
           }
         } // uuid matching
#ifdef DEMO
         if (m_pwlist.size() < MAXDEMO) {
           m_pwlist.insert(make_pair(CUUIDGen(uuid), ci_temp));
         } else {
           limited = true;
         }
#else
         m_pwlist.insert(make_pair(CUUIDGen(uuid), ci_temp));
#endif
         break;
      case PWSfile::END_OF_FILE:
        go = false;
        break;
      default:
        break;
    } // switch
  } while (go);

  m_nRecordsWithUnknownFields = in->GetNumRecordsWithUnknownFields();
  in->GetUnknownHeaderFields(m_UHFL);
  int closeStatus = in->Close(); // in V3 this checks integrity
#ifdef DEMO
  if (closeStatus == PWSfile::SUCCESS && limited)
    closeStatus = LIMIT_REACHED; // if integrity OK but LIMIT_REACHED, return latter
#endif
  delete in;

  // No Undo/Redo when reading file.  Therefore NOT via Commands
  DoAddDependentEntries(Possible_Aliases, NULL, CItemData::ET_ALIAS, CItemData::UUID);
  DoAddDependentEntries(Possible_Shortcuts, NULL, CItemData::ET_SHORTCUT, CItemData::UUID);
  Possible_Aliases.clear();
  Possible_Shortcuts.clear();
  SetDBChanged(false);

  // Setup file signature for checking file integrity upon backup.
  // Goal is to prevent overwriting a good backup with a corrupt file.
  if (a_filename == m_currfile) {
    delete m_fileSig;
    m_fileSig = new PWSFileSig(a_filename.c_str());
  }

  if (numlarge > 0 && 
      (closeStatus == PWSfile::SUCCESS || closeStatus == LIMIT_REACHED) &&
      m_pReporter != NULL) {
    stringT cs_msg, cs_caption, cs_entry;
    int units(0);
    LoadAString(cs_caption, IDSC_LARGEENTRIES);
    LoadAString(cs_entry, numlarge == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
    uimaxsize >>= 10;  // make bytes -> KB
    if (uimaxsize > 999) {
      uimaxsize >>= 10;  // make KB -> MB
      units++;
    }
    Format(cs_msg, IDSC_WARNINGENTRYLENGTH, numlarge, cs_entry.c_str(), uimaxsize,
           units == 0 ? _T("KB") : _T("MB"));
    (*m_pReporter)(cs_caption, cs_msg);
  }

  return closeStatus;
}

static void ManageIncBackupFiles(const stringT &cs_filenamebase,
                                 size_t maxnumincbackups, stringT &cs_newname)
{
  // make sure we've no more than maxnumincbackups backup files,
  // and return the base name of the next backup file
  // (sans the suffix, which will be added by caller)

  stringT cs_filenamemask(cs_filenamebase);
  vector<stringT> files;
  vector<int> file_nums;

  cs_filenamemask += _T("_???.ibak");

  pws_os::FindFiles(cs_filenamemask, files);

  for (vector<stringT>::iterator iter = files.begin();
       iter != files.end(); iter++) {
    stringT ibak_number_str = iter->substr(iter->length() - 8, 3);
    if (ibak_number_str.find_first_not_of(_T("0123456789")) != stringT::npos)
      continue;
    istringstreamT is(ibak_number_str);
    int n;
    is >> n;
    file_nums.push_back(n);
  }


  if (file_nums.empty()) {
    cs_newname = cs_filenamebase + _T("_001");
    return;
  }

  sort(file_nums.begin(), file_nums.end());

  int nnn = file_nums.back();
  nnn++;
  if (nnn > 999) nnn = 1;

  Format(cs_newname, _T("%s_%03d"), cs_filenamebase.c_str(), nnn);

  int i = 0;
  size_t num_found = file_nums.size();
  stringT excess_file;
  while (num_found >= maxnumincbackups) {
    nnn = file_nums[i];
    Format(excess_file, _T("%s_%03d.ibak"), cs_filenamebase.c_str(), nnn);
    i++;
    num_found--;
    if (!pws_os::DeleteAFile(excess_file)) {
      pws_os::Trace(_T("DeleteFile(%s) failed"), excess_file.c_str());
      continue;
    }
  }
}

bool PWScore::BackupCurFile(int maxNumIncBackups, int backupSuffix,
                            const stringT &userBackupPrefix,
                            const stringT &userBackupDir, stringT &bu_fname)
{
  stringT cs_temp;
  const stringT path(m_currfile.c_str());
  stringT drv, dir, name, ext;

  // Check if the file we're about to backup is unchanged since
  // we opened it, to avoid overwriting a good file with a bad one
  if (m_fileSig != NULL) {
    PWSFileSig curSig(m_currfile.c_str());
    bool passed = (curSig == *m_fileSig);
    if (!passed) // XXX yell scream & shout
      return false;
  }

  pws_os::splitpath(path, drv, dir, name, ext);
  // Get location for intermediate backup
  if (userBackupDir.empty()) {
    // directory same as database's
    // Get directory containing database
    cs_temp = drv + dir;
    // (in Windows, need to verify for non-Windows)
    // splitpath directory ends with a '/', therefore do not need:
    // cs_temp += pws_os::PathSeparator;
  } else {
    // User specified somewhere else
    cs_temp = userBackupDir;
    // Ensure ends with path separator
    if (userBackupDir[userBackupDir.length() - 1] != pws_os::PathSeparator)
      cs_temp += pws_os::PathSeparator;
  }

  // generate prefix of intermediate backup file name
  if (userBackupPrefix.empty()) {
    cs_temp += name;
  } else {
    cs_temp += userBackupPrefix;
  }

  // Add on suffix
  switch (backupSuffix) { // case values from order in listbox.
    case 1: // YYYYMMDD_HHMMSS suffix
      {
        time_t now;
        time(&now);
        StringX cs_datetime = PWSUtil::ConvertToDateTimeString(now,
                                                               TMC_EXPORT_IMPORT);
        cs_temp += _T("_");
        StringX nf = cs_temp.c_str() + 
                     cs_datetime.substr( 0, 4) +  // YYYY
                     cs_datetime.substr( 5, 2) +  // MM
                     cs_datetime.substr( 8, 2) +  // DD
                     StringX(_T("_")) +
                     cs_datetime.substr(11, 2) +  // HH
                     cs_datetime.substr(14, 2) +  // MM
                     cs_datetime.substr(17, 2);   // SS
        bu_fname = nf.c_str();
        break;
      }
    case 2: // _nnn suffix
      ManageIncBackupFiles(cs_temp, maxNumIncBackups, bu_fname);
      break;
    case 0: // no suffix
    default:
      bu_fname = cs_temp;
      break;
  }

  bu_fname +=  _T(".ibak");

  // Current file becomes backup
  // Directories along the specified backup path are created as needed
  return pws_os::RenameFile(m_currfile.c_str(), bu_fname);
}

void PWScore::ChangePasskey(const StringX &newPasskey)
{
  SetPassKey(newPasskey);
  SetDBChanged(true);
}

// functor object type for find_if:
struct FieldsMatch {
  bool operator()(pair<CUUIDGen, CItemData> p) {
    const CItemData &item = p.second;
    return (m_group == item.GetGroup() &&
            m_title == item.GetTitle() &&
            m_user  == item.GetUser());
  }
  FieldsMatch(const StringX &a_group, const StringX &a_title,
              const StringX &a_user) :
  m_group(a_group), m_title(a_title), m_user(a_user) {}

private:
  FieldsMatch& operator=(const FieldsMatch&); // Do not implement
  const StringX &m_group;
  const StringX &m_title;
  const StringX &m_user;
};

// Finds stuff based on group, title & user fields only
ItemListIter PWScore::Find(const StringX &a_group,const StringX &a_title,
                           const StringX &a_user)
{
  FieldsMatch fields_match(a_group, a_title, a_user);

  ItemListIter retval = find_if(m_pwlist.begin(), m_pwlist.end(),
                                fields_match);
  return retval;
}

struct TitleMatch {
  bool operator()(pair<CUUIDGen, CItemData> p) {
    const CItemData &item = p.second;
    return (m_title == item.GetTitle());
  }

  TitleMatch(const StringX &a_title) :
    m_title(a_title) {}

private:
  TitleMatch& operator=(const TitleMatch&); // Do not implement
  const StringX &m_title;
};

ItemListIter PWScore::GetUniqueBase(const StringX &a_title, bool &bMultiple)
{
  ItemListIter retval(m_pwlist.end());
  int num(0);
  TitleMatch TitleMatch(a_title);

  ItemListIter found(m_pwlist.begin());
  do {
    found = find_if(found, m_pwlist.end(), TitleMatch);
    if (found != m_pwlist.end()) {
      num++;
      if (num == 1) {
        // Save first
        retval = found;
      } else {
        // More than 1, set to end()
        retval = m_pwlist.end();
        break;
      }
      found++;
    } else
      break;
  } while (found != m_pwlist.end());

  // It is 1 if only 1, but 0 if none & 2 if more than 1 (we just stopped at the second)
  bMultiple = (num > 1);
  return retval;
}

struct GroupTitle_TitleUserMatch {
  bool operator()(pair<CUUIDGen, CItemData> p) {
    const CItemData &item = p.second;
    return ((m_gt == item.GetGroup() && m_tu == item.GetTitle()) ||
            (m_gt == item.GetTitle() && m_tu == item.GetUser()));
  }

  GroupTitle_TitleUserMatch(const StringX &a_grouptitle,
                            const StringX &a_titleuser) :
                            m_gt(a_grouptitle),  m_tu(a_titleuser) {}

private:
  GroupTitle_TitleUserMatch& operator=(const GroupTitle_TitleUserMatch&); // Do not implement
  const StringX &m_gt;
  const StringX &m_tu;
};

ItemListIter PWScore::GetUniqueBase(const StringX &grouptitle, 
                                    const StringX &titleuser, bool &bMultiple)
{
  ItemListIter retval(m_pwlist.end());
  int num(0);
  GroupTitle_TitleUserMatch GroupTitle_TitleUserMatch(grouptitle, titleuser);

  ItemListIter found(m_pwlist.begin());
  do {
    found = find_if(found, m_pwlist.end(), GroupTitle_TitleUserMatch);
    if (found != m_pwlist.end()) {
      num++;
      if (num == 1) {
        // Save first
        retval = found;
      } else {
        // More than 1, set to end()
        retval = m_pwlist.end();
        break;
      }
      found++;
    } else
      break;
  } while (found != m_pwlist.end());

  // It is 1 if only 1, but 0 if none & 2 if more than 1 (we just stopped at the second)
  bMultiple = (num > 1);
  return retval;
}

void PWScore::EncryptPassword(const unsigned char *plaintext, size_t len,
                              unsigned char *ciphertext) const
{
  // Chicken out of an interface change, or just a sanity check?
  // Maybe both...
  ASSERT(len > 0);
  unsigned int ulen = static_cast<unsigned int>(len);

  // ciphertext is '((len + 7) / 8) * 8' bytes long
  const unsigned int BS = BlowFish::BLOCKSIZE;

  BlowFish *bf = BlowFish::MakeBlowFish(m_session_key,
                                        sizeof(m_session_key),
                                        m_session_salt,
                                        sizeof(m_session_salt));
  unsigned int BlockLength = ((ulen + (BS - 1)) / BS) * BS;
  unsigned char curblock[BS];

  for (unsigned int x = 0; x < BlockLength; x += BS) {
    unsigned int i;
    if ((ulen == 0) ||
      ((ulen % BS != 0) && (ulen - x < BS))) {
        //This is for an uneven last block
        memset(curblock, 0, BS);
        for (i = 0; i < len % BS; i++)
          curblock[i] = plaintext[x + i];
    } else
      for (i = 0; i < BS; i++) {
        curblock[i] = plaintext[x + i];
      }
    bf->Encrypt(curblock, curblock);
    memcpy(ciphertext + x, curblock, BS);
  }
  trashMemory(curblock, sizeof(curblock));
  delete bf;
}

void PWScore::SetPassKey(const StringX &new_passkey)
{
  // Only used when opening files and for new files
  const unsigned int BS = BlowFish::BLOCKSIZE;
  // if changing, clear old
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS -1)) / BS) * BS);
    delete[] m_passkey;
  }

  m_passkey_len = new_passkey.length() * sizeof(TCHAR);

  size_t BlockLength = ((m_passkey_len + (BS - 1)) / BS) * BS;
  m_passkey = new unsigned char[BlockLength];
  LPCTSTR plaintext = LPCTSTR(new_passkey.c_str());
  EncryptPassword(reinterpret_cast<const unsigned char *>(plaintext), m_passkey_len, m_passkey);
}

StringX PWScore::GetPassKey() const
{
  StringX retval(_T(""));
  if (m_passkey_len > 0) {
    const unsigned int BS = BlowFish::BLOCKSIZE;
    size_t BlockLength = ((m_passkey_len + (BS - 1)) / BS) * BS;
    BlowFish *bf = BlowFish::MakeBlowFish(m_session_key,
                                          sizeof(m_session_key),
                                          m_session_salt,
                                          sizeof(m_session_salt));
    unsigned char curblock[BS];
    for (unsigned int x = 0; x < BlockLength; x += BS) {
      unsigned int i;
      for (i = 0; i < BS; i++) {
        curblock[i] = m_passkey[x + i];
      }

      bf->Decrypt(curblock, curblock);
      for (i = 0; i < BS; i += sizeof(TCHAR)) {
        if (x + i < m_passkey_len) {
          retval += *(reinterpret_cast<TCHAR*>(curblock + i));
        }
      }
    }
    trashMemory(curblock, sizeof(curblock));
    delete bf;
  }
  return retval;
}

void PWScore::SetDisplayStatus(const vector<bool> &s)
{ 
  // DON'T set m_bDBChanged!
  // Application should use WasDisplayStatusChanged()
  // to determine if state has changed.
  // This allows app to silently save without nagging user
  m_hdr.m_displaystatus = s;
}

const vector<bool> &PWScore::GetDisplayStatus() const
{
  return m_hdr.m_displaystatus;
}

bool PWScore::WasDisplayStatusChanged() const
{
  // m_OrigDisplayStatus is set while reading file.
  // m_hdr.m_displaystatus may be changed via SetDisplayStatus
  return m_hdr.m_displaystatus != m_OrigDisplayStatus;
}

// GetUniqueGroups - returns an array of all group names, with no duplicates.
void PWScore::GetUniqueGroups(vector<stringT> &aryGroups) const
{
  // use the fact that set eliminates dups for us
  set<stringT> setGroups;

  ItemListConstIter iter;

  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++ ) {
    const CItemData &ci = iter->second;
    setGroups.insert(ci.GetGroup().c_str());
  }

  aryGroups.clear();
  // copy unique results from set to caller's vector
  copy(setGroups.begin(), setGroups.end(), back_inserter(aryGroups));
}

void PWScore::CopyPWList(const ItemList &in)
{
  m_pwlist = in;
  SetDBChanged(true);
}

// For Validate only
struct st_GroupTitleUser2 {
  StringX group;
  StringX title;
  StringX user;
  StringX newtitle;

  st_GroupTitleUser2() {}

  st_GroupTitleUser2(const StringX &g, const StringX &t, const StringX &u,
    const StringX &n)
  : group(g), title(t), user(u), newtitle(n) {}

  st_GroupTitleUser2 &operator=(const st_GroupTitleUser2 &that) {
    if (this != &that) {
      group = that.group; title = that.title; user = that.user;
      newtitle = that.newtitle;
    }
    return *this;
  }
};

static bool GTUCompareV1(const st_GroupTitleUser &gtu1, const st_GroupTitleUser &gtu2)
{
  if (gtu1.group != gtu2.group)
    return gtu1.group.compare(gtu2.group) < 0;
  else if (gtu1.title != gtu2.title)
    return gtu1.title.compare(gtu2.title) < 0;
  else
    return gtu1.user.compare(gtu2.user) < 0;
}

static bool GTUCompareV2(const st_GroupTitleUser2 &gtu1, const st_GroupTitleUser2 &gtu2)
{
  if (gtu1.group != gtu2.group)
    return gtu1.group.compare(gtu2.group) < 0;
  else if (gtu1.title != gtu2.title)
    return gtu1.title.compare(gtu2.title) < 0;
  else if (gtu1.user != gtu2.user)
    return gtu1.user.compare(gtu2.user) < 0;
  else
    return gtu1.newtitle.compare(gtu2.newtitle) < 0;
}

bool PWScore::Validate(stringT &status, CReport &rpt, const size_t iMAXCHARS)
{
  // Check uuid is valid
  // Check PWH is valid
  // Check alias password has corresponding base entry
  // Check shortcut password has corresponding base entry
  // Note that with m_pwlist implemented as a map keyed on uuids, each
  // entry is guaranteed to have a unique uuid. The uniqueness invariant
  // should be enforced elsewhere (upon read/import).
  // Also group/title/user must be unique.
  // Check that no text field has more than MAXCHARS, that can displayed
  // in the GUI's text control.

  uuid_array_t uuid_array, base_uuid, temp_uuid;
  int n = -1;
  unsigned int num_PWH_fixed = 0;
  unsigned int num_uuid_fixed = 0;
  unsigned int num_duplicates_fixed = 0;
  unsigned int num_excessivetxt_found = 0;
  int num_alias_warnings, num_shortcuts_warnings;

  MultiCommands *pmulticmds = MultiCommands::Create(this);

  stringT cs_Error;
  pws_os::Trace(_T("Start validation\n"));

  st_GroupTitleUser st_gtu;
  GTUSet setGTU;
  GTUSetPair pr_gtu;
  std::vector<st_GroupTitleUser> vGTU_UUID, vGTU_PWH, vGTU_TEXT;
  std::vector<st_GroupTitleUser2> vGTU_NONUNIQUE;

  UUIDVector Possible_Aliases, Possible_Shortcuts;
  ItemListIter iter;

  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    CItemData &ci = iter->second;
    CItemData fixedItem(ci);
    bool bFixed(false);

    ci.GetUUID(uuid_array);
    n++;

    // Fix GTU uniqueness
    StringX sxgroup(ci.GetGroup()), sxtitle(ci.GetTitle()), sxuser(ci.GetUser());
    st_gtu.group = sxgroup;
    st_gtu.title = sxtitle;
    st_gtu.user = sxuser;
    pr_gtu = setGTU.insert(st_gtu);
    if (!pr_gtu.second) {
      int i = 0;
      StringX s_copy, sxnewtitle(sxtitle);
      do {
        i++;
        Format(s_copy, IDSC_DUPLICATENUMBER, i);
        sxnewtitle = sxtitle + s_copy;
        st_gtu.title = sxnewtitle;
        pr_gtu =  setGTU.insert(st_gtu);
      } while (!pr_gtu.second);

      bFixed = true;
      vGTU_NONUNIQUE.push_back(st_GroupTitleUser2(sxgroup, sxtitle, sxuser, sxnewtitle));
      fixedItem.SetTitle(sxnewtitle);
      sxtitle = sxnewtitle;
      num_duplicates_fixed++;
    }

    // Fix bad UUID
    if (uuid_array[0] == 0x00) {
      bFixed = true;
      num_uuid_fixed += fixedItem.ValidateUUID(m_hdr.m_nCurrentMajorVersion,
                                               m_hdr.m_nCurrentMinorVersion,
                                               uuid_array);
      vGTU_UUID.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
    }

    // Fix bad History
    if (!fixedItem.ValidatePWHistory()) {
      bFixed = true;
      vGTU_PWH.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      num_PWH_fixed++;
    }

    // Fix possible bad Alias/Shortcut
    StringX csMyPassword = ci.GetPassword();
    if (csMyPassword.length() == 36) { // look for "[[uuid]]" or "[~uuid~]"
      StringX cs_possibleUUID = csMyPassword.substr(2, 32); // try to extract uuid
      ToLower(cs_possibleUUID);
      if (((csMyPassword.substr(0,2) == _T("[[") &&
            csMyPassword.substr(csMyPassword.length() - 2) == _T("]]")) ||
           (csMyPassword.substr(0, 2) == _T("[~") &&
            csMyPassword.substr(csMyPassword.length() - 2) == _T("~]"))) &&
          cs_possibleUUID.find_first_not_of(_T("0123456789abcdef")) == StringX::npos) {
        CUUIDGen uuid(cs_possibleUUID.c_str());
        uuid.GetUUID(base_uuid);
        ci.GetUUID(temp_uuid);
        if (csMyPassword.substr(0, 2) == _T("[[")) {
          m_alias2base_map[temp_uuid] = base_uuid;
          Possible_Aliases.push_back(temp_uuid);
        } else {
          m_shortcut2base_map[temp_uuid] = base_uuid;
          Possible_Shortcuts.push_back(temp_uuid);
        }
      }
    }

    // Note excessively sized text fields
    if (iMAXCHARS > 0) {
      bool bEntryHasBigField(false);
      for (unsigned char uc = static_cast<unsigned char>(CItemData::GROUP); 
           uc < static_cast<unsigned char>(CItemData::LAST); uc++) {
        if (CItemData::IsTextField(uc)) {
          StringX sxvalue = ci.GetFieldValue(static_cast<CItemData::FieldType>(uc));
          if (sxvalue.length() > iMAXCHARS) {
            bEntryHasBigField = true;
            //fixedItem.SetFieldValue((CItemData::FieldType)uc, sxvalue.substr(0, iMAXCHARS));
          }
        }
      }
      if (bEntryHasBigField) {
        vGTU_TEXT.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
        num_excessivetxt_found++;
      }
    }

    if (bFixed) {
      fixedItem.SetStatus(CItemData::ES_MODIFIED);
      Command *pcmd = EditEntryCommand::Create(this, ci, fixedItem);
      pmulticmds->Add(pcmd);
    }
  } // iteration over m_pwlist

  Command *pcmdA = AddDependentEntriesCommand::Create(this,
                                                      Possible_Aliases, &rpt, 
                                                      CItemData::ET_ALIAS,
                                                      CItemData::UUID);
  pmulticmds->Add(pcmdA);
  Command *pcmdS = AddDependentEntriesCommand::Create(this,
                                                      Possible_Shortcuts, &rpt, 
                                                      CItemData::ET_SHORTCUT,
                                                      CItemData::UUID);
  pmulticmds->Add(pcmdS);
  Execute(pmulticmds);
  pmulticmds->GetRC(pcmdA, num_alias_warnings);
  pmulticmds->GetRC(pcmdS, num_shortcuts_warnings);

  Possible_Aliases.clear();
  Possible_Shortcuts.clear();

  if (!vGTU_NONUNIQUE.empty()) {
    std::sort(vGTU_NONUNIQUE.begin(), vGTU_NONUNIQUE.end(), GTUCompareV2);
    rpt.WriteLine();
    LoadAString(cs_Error, IDSC_VALIDATEDUPLICATES);
    rpt.WriteLine(cs_Error);
    for (size_t iv = 0; iv < vGTU_NONUNIQUE.size(); iv++) {
      st_GroupTitleUser2 &gtu2 = vGTU_NONUNIQUE[iv];
      stringT cs_newtitle;
      Format(cs_newtitle, IDSC_VALIDATEENTRY2, gtu2.newtitle.c_str());
      Format(cs_Error, IDSC_VALIDATEENTRY,
             gtu2.group.c_str(), gtu2.title.c_str(), gtu2.user.c_str(), cs_newtitle.c_str());
      rpt.WriteLine(cs_Error);
    }
  }

  if (!vGTU_UUID.empty()) {
    std::sort(vGTU_UUID.begin(), vGTU_UUID.end(), GTUCompareV1);
    rpt.WriteLine();
    LoadAString(cs_Error, IDSC_VALIDATEUUID);
    rpt.WriteLine(cs_Error);
    for (size_t iv = 0; iv < vGTU_UUID.size(); iv++) {
      st_GroupTitleUser &gtu = vGTU_UUID[iv];
      Format(cs_Error, IDSC_VALIDATEENTRY,
             gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
      rpt.WriteLine(cs_Error);
    }
  }

  if (!vGTU_PWH.empty()) {
    std::sort(vGTU_PWH.begin(), vGTU_PWH.end(), GTUCompareV1);
    rpt.WriteLine();
    LoadAString(cs_Error, IDSC_VALIDATEPWH);
    rpt.WriteLine(cs_Error);
    for (size_t iv = 0; iv < vGTU_PWH.size(); iv++) {
      st_GroupTitleUser &gtu = vGTU_PWH[iv];
      Format(cs_Error, IDSC_VALIDATEENTRY,
             gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
      rpt.WriteLine(cs_Error);
    }
  }

  if (!vGTU_TEXT.empty()) {
    std::sort(vGTU_TEXT.begin(), vGTU_TEXT.end(), GTUCompareV1);
    rpt.WriteLine();
    Format(cs_Error, IDSC_VALIDATETEXT, iMAXCHARS);
    rpt.WriteLine(cs_Error);
    for (size_t iv = 0; iv < vGTU_TEXT.size(); iv++) {
      st_GroupTitleUser &gtu = vGTU_TEXT[iv];
      Format(cs_Error, IDSC_VALIDATEENTRY,
             gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
      rpt.WriteLine(cs_Error);
    }
  }

  pws_os::Trace(_T("End validation. %d entries processed\n"), n + 1);

  m_bUniqueGTUValidated = true;
  if ((num_uuid_fixed + num_PWH_fixed + num_duplicates_fixed + 
       num_alias_warnings + num_shortcuts_warnings + num_excessivetxt_found) > 0) {
    Format(status, IDSC_NUMPROCESSED,
           n + 1, num_uuid_fixed, num_PWH_fixed, num_duplicates_fixed,
           num_alias_warnings, num_shortcuts_warnings, num_excessivetxt_found);
    SetDBChanged(true);
    return true;
  } else {
    return false;
  }
}

void PWScore::ClearFileUUID()
{
  memset(m_hdr.m_file_uuid_array, 0, sizeof(uuid_array_t));
}

void PWScore::SetFileUUID(uuid_array_t &file_uuid_array)
{
  memcpy(m_hdr.m_file_uuid_array, file_uuid_array,
         sizeof(uuid_array_t));
}

void PWScore::GetFileUUID(uuid_array_t &file_uuid_array) const
{
  memcpy(file_uuid_array, m_hdr.m_file_uuid_array, sizeof(uuid_array_t));
}

StringX PWScore::GetUniqueTitle(const StringX &group, const StringX &title,
                                const StringX &user, const int IDS_MESSAGE)
{
  StringX new_title(title);
  if (Find(group, title, user) != m_pwlist.end()) {
    // Find a unique "Title"
    ItemListConstIter listpos;
    int i = 0;
    StringX s_copy;
    do {
      i++;
      Format(s_copy, IDS_MESSAGE, i);
      new_title = title + s_copy;
      listpos = Find(group, new_title, user);
    } while (listpos != m_pwlist.end());
  }
  return new_title;
}

bool PWScore::InitialiseGTU(GTUSet &setGTU)
{
  // Populate the set of all group/title/user entries
  GTUSetPair pr_gtu;
  ItemListConstIter citer;

  setGTU.clear();
  for (citer = m_pwlist.begin(); citer != m_pwlist.end(); citer++) {
    const CItemData &ci = citer->second;
    pr_gtu = setGTU.insert(st_GroupTitleUser(ci.GetGroup(), ci.GetTitle(), ci.GetUser()));
    if (!pr_gtu.second) {
      // Could happen if merging or synching a bad database!
      setGTU.clear();
      return false;
    }
  }
  m_bUniqueGTUValidated = true;
  return true;
}

bool PWScore::InitialiseUUID(UUIDSet &setUUID)
{
  // Populate the set of all UUID entries
  UUIDSetPair pr_uuid;
  ItemListConstIter citer;

  setUUID.clear();
  uuid_array_t entry_uuid;
  for (citer = m_pwlist.begin(); citer != m_pwlist.end(); citer++) {
    citer->second.GetUUID(entry_uuid);
    pr_uuid = setUUID.insert(st_UUID(entry_uuid));
    if (!pr_uuid.second) {
      // Could happen if merging or synching a bad database!
      setUUID.clear();
      return false;
    }
  }
  return true;
}

bool PWScore::MakeEntryUnique(GTUSet &setGTU,
                              const StringX &sxgroup, StringX &sxtitle,
                              const StringX &sxuser, const int IDS_MESSAGE)
{
  StringX sxnewtitle(_T(""));
  GTUSetPair pr_gtu;
  bool retval = true;

  // Add supplied GTU - if already present, change title until a 
  // unique combination is found.
  pr_gtu =  setGTU.insert(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
  if (!pr_gtu.second) { // insert failed => already in set!
    retval = false;
    int i = 0;
    StringX s_copy;
    do {
      i++;
      Format(s_copy, IDS_MESSAGE, i);
      sxnewtitle = sxtitle + s_copy;
      pr_gtu =  setGTU.insert(st_GroupTitleUser(sxgroup, sxnewtitle, sxuser));
    } while (!pr_gtu.second);
    sxtitle = sxnewtitle;
  }
  return retval; // false iff we had to modify sxtitle
}

void PWScore::DoAddDependentEntry(const uuid_array_t &base_uuid, 
                                  const uuid_array_t &entry_uuid, 
                                  const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  ItemListIter iter = m_pwlist.find(base_uuid);
  ASSERT(iter != m_pwlist.end());

  bool baseWasNormal = iter->second.IsNormal();
  if (type == CItemData::ET_ALIAS) {
    // Mark base entry as a base entry - must be a normal entry or already an alias base
    ASSERT(iter->second.IsNormal() || iter->second.IsAliasBase());
    iter->second.SetAliasBase();
    if (baseWasNormal)
      GUIRefreshEntry(iter->second);
  } else if (type == CItemData::ET_SHORTCUT) {
    // Mark base entry as a base entry - must be a normal entry or already a shortcut base
    ASSERT(iter->second.IsNormal() || iter->second.IsShortcutBase());
    iter->second.SetShortcutBase();
    if (baseWasNormal)
      GUIRefreshEntry(iter->second);
  }

  // Add to both the base->type multimap and the type->base map
  pmmap->insert(ItemMMap_Pair(base_uuid, entry_uuid));
  pmap->insert(ItemMap_Pair(entry_uuid, base_uuid));
}

void PWScore::DoRemoveDependentEntry(const uuid_array_t &base_uuid, 
                                     const uuid_array_t &entry_uuid,
                                     const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  // Remove from entry -> base map
  pmap->erase(entry_uuid);

  // Remove from base -> entry multimap
  ItemMMapIter mmiter;
  ItemMMapIter mmlastElement;

  mmiter = pmmap->find(base_uuid);
  if (mmiter == pmmap->end())
    return;

  mmlastElement = pmmap->upper_bound(base_uuid);
  uuid_array_t mmiter_uuid;

  for ( ; mmiter != mmlastElement; mmiter++) {
    mmiter->second.GetUUID(mmiter_uuid);
    if (memcmp(entry_uuid, mmiter_uuid, sizeof(uuid_array_t)) == 0) {
      pmmap->erase(mmiter);
      break;
    }
  }

  // Reset base entry to normal if it has no more aliases
  if (pmmap->find(base_uuid) == pmmap->end()) {
    ItemListIter iter = m_pwlist.find(base_uuid);
    if (iter != m_pwlist.end()) {
      iter->second.SetNormal();
      GUIRefreshEntry(iter->second);
    }
  }
}

void PWScore::DoRemoveAllDependentEntries(const uuid_array_t &base_uuid, 
                                          const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  // Remove from entry -> base map for each entry
  ItemMMapIter itr;
  ItemMMapIter lastElement;

  itr = pmmap->find(base_uuid);
  if (itr == pmmap->end())
    return;

  lastElement = pmmap->upper_bound(base_uuid);
  uuid_array_t itr_uuid;

  for ( ; itr != lastElement; itr++) {
    itr->second.GetUUID(itr_uuid);
    // Remove from entry -> base map
    pmap->erase(itr_uuid);
  }

  // Remove from base -> entry multimap
  pmmap->erase(base_uuid);

  // Reset base entry to normal
  ItemListIter iter = m_pwlist.find(base_uuid);
  if (iter != m_pwlist.end())
    iter->second.SetNormal();
}

void PWScore::DoMoveDependentEntries(const uuid_array_t &from_baseuuid,
                                     const uuid_array_t &to_baseuuid, 
                                     const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  ItemMMapIter from_itr;
  ItemMMapIter lastfromElement;

  from_itr = pmmap->find(from_baseuuid);
  if (from_itr == pmmap->end())
    return;

  lastfromElement = pmmap->upper_bound(from_baseuuid);

  for ( ; from_itr != lastfromElement; from_itr++) {
    // Add to new base in base -> entry multimap
    pmmap->insert(ItemMMap_Pair(to_baseuuid, from_itr->second));
    // Remove from entry -> base map
    pmap->erase(from_itr->second);
    // Add to entry -> base map (new base)
    pmap->insert(ItemMap_Pair(from_itr->second, to_baseuuid));    
  }

  // Now delete all old base entries
  pmmap->erase(from_baseuuid);
}

int PWScore::DoAddDependentEntries(UUIDVector &dependentlist, CReport *pRpt,
                                   const CItemData::EntryType type, const int &iVia,
                                   ItemList *pmapDeletedItems,
                                   SaveTypePWMap *pmapSaveTypePW)
{
  // When called during validation of a database  - *pRpt is valid
  // When called during the opening of a database or during drag & drop
  //   - *pRpt is NULL and no report generated

  // type is either CItemData::ET_ALIAS or CItemData::ET_SHORTCUT

  // If iVia == CItemData::UUID, the password was "[[uuidstr]]" or "[~uuidstr~]" of the
  //   associated base entry
  // If iVia == CItemData::PASSWORD, the password is expected to be in the full format 
  // [g:t:u], where g and/or u may be empty.

  if (pmapDeletedItems != NULL)
    pmapDeletedItems->clear();

  if (pmapSaveTypePW != NULL)
    pmapSaveTypePW->clear();

  ItemMap *pmap;
  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return -1;

  int num_warnings(0);
  st_SaveTypePW st_typepw;

  if (!dependentlist.empty()) {
    UUIDVectorIter paiter;
    ItemListIter iter;
    StringX csPwdGroup, csPwdTitle, csPwdUser, tmp;
    uuid_array_t base_uuid, entry_uuid;
    bool bwarnings(false);
    stringT strError;

    for (paiter = dependentlist.begin();
         paiter != dependentlist.end(); paiter++) {
      iter = m_pwlist.find(*paiter);
      if (iter == m_pwlist.end())
        return num_warnings;

      CItemData *pci_curitem = &iter->second;
      pci_curitem->GetUUID(entry_uuid);
      GetDependentEntryBaseUUID(entry_uuid, base_uuid, type);

      // Delete it - we will put it back if it is an alias/shortcut
      pmap->erase(entry_uuid);

      if (iVia == CItemData::UUID) {
        iter = m_pwlist.find(base_uuid);
      } else {
        tmp = pci_curitem->GetPassword();
        // Remove leading '[['/'[~' & trailing ']]'/'~]'
        tmp = tmp.substr(2, tmp.length() - 4);
        csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(":")));
        // Skip over 'group:'
        tmp = tmp.substr(csPwdGroup.length() + 1);
        csPwdTitle = tmp.substr(0, tmp.find_first_of(_T(":")));
        // Skip over 'title:'
        csPwdUser = tmp.substr(csPwdTitle.length() + 1);
        iter = Find(csPwdGroup, csPwdTitle, csPwdUser);
      }

      if (iter != m_pwlist.end()) {
        if (type == CItemData::ET_SHORTCUT) {
          // Adding shortcuts -> Base must be normal or already a shortcut base
          if (!iter->second.IsNormal() && !iter->second.IsShortcutBase()) {
            // Bad news!
            if (pRpt != NULL) {
              if (!bwarnings) {
                bwarnings = true;
                LoadAString(strError, IDSC_IMPORTWARNINGHDR);
                pRpt->WriteLine(strError);
              }
              stringT cs_type;
              LoadAString(cs_type, IDSC_SHORTCUT);
              Format(strError, IDSC_IMPORTWARNING3, cs_type.c_str(),
                     pci_curitem->GetGroup().c_str(), pci_curitem->GetTitle().c_str(), 
                     pci_curitem->GetUser().c_str(), cs_type.c_str());
              pRpt->WriteLine(strError);
            }
            // Invalid - delete!
            if (pmapDeletedItems != NULL)
              pmapDeletedItems->insert(ItemList_Pair(*paiter, *pci_curitem));
            m_pwlist.erase(iter);
            continue;
          } 
        }
        if (type == CItemData::ET_ALIAS) {
          // Adding Aliases -> Base must be normal or already a alias base
          if (!iter->second.IsNormal() && !iter->second.IsAliasBase()) {
            // Bad news!
            if (pRpt != NULL) {
              if (!bwarnings) {
                bwarnings = true;
                LoadAString(strError, IDSC_IMPORTWARNINGHDR);
                pRpt->WriteLine(strError);
              }
              stringT cs_type;
              LoadAString(cs_type, IDSC_ALIAS);
              Format(strError, IDSC_IMPORTWARNING3, cs_type.c_str(),
                     pci_curitem->GetGroup().c_str(), pci_curitem->GetTitle().c_str(), 
                     pci_curitem->GetUser().c_str(), cs_type.c_str());
              pRpt->WriteLine(strError);
            }
            // Invalid - delete!
            if (pmapDeletedItems != NULL)
              pmapDeletedItems->insert(ItemList_Pair(*paiter, *pci_curitem));
            m_pwlist.erase(iter);
            continue;
          }
          if (iter->second.IsAlias()) {
            // This is an alias too!  Not allowed!  Make new one point to original base
            // Note: this may be random as who knows the order of reading records?
            uuid_array_t temp_uuid;
            iter->second.GetUUID(temp_uuid);
            GetDependentEntryBaseUUID(temp_uuid, base_uuid, type);
            if (pRpt != NULL) {
              if (!bwarnings) {
                bwarnings = true;
                LoadAString(strError, IDSC_IMPORTWARNINGHDR);
                pRpt->WriteLine(strError);
              }
              Format(strError, IDSC_IMPORTWARNING1, pci_curitem->GetGroup().c_str(),
                     pci_curitem->GetTitle().c_str(), pci_curitem->GetUser().c_str());
              pRpt->WriteLine(strError);
              LoadAString(strError, IDSC_IMPORTWARNING1A);
              pRpt->WriteLine(strError);
            }
            if (pmapSaveTypePW != NULL) {
              st_typepw.et = iter->second.GetEntryType();
              st_typepw.sxpw = _T("");
              pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
            }
            pci_curitem->SetAlias();
            num_warnings++;
          }
        }
        iter->second.GetUUID(base_uuid);
        if (type == CItemData::ET_ALIAS) {
          if (pmapSaveTypePW != NULL) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = _T("");
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          iter->second.SetAliasBase();
        } else
        if (type == CItemData::ET_SHORTCUT) {
          if (pmapSaveTypePW != NULL) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = _T("");
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          iter->second.SetShortcutBase();
        }

        pmmap->insert(ItemMMap_Pair(base_uuid, entry_uuid));
        pmap->insert(ItemMap_Pair(entry_uuid, base_uuid));
        if (type == CItemData::ET_ALIAS) {
          if (pmapSaveTypePW != NULL) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = pci_curitem->GetPassword();
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          pci_curitem->SetPassword(_T("[Alias]"));
          pci_curitem->SetAlias();
        } else
        if (type == CItemData::ET_SHORTCUT) {
          if (pmapSaveTypePW != NULL) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = pci_curitem->GetPassword();
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          pci_curitem->SetPassword(_T("[Shortcut]"));
          pci_curitem->SetShortcut();
        }
      } else {
        // Specified base does not exist!
        if (pRpt != NULL) {
          if (!bwarnings) {
            bwarnings = true;
            LoadAString(strError, IDSC_IMPORTWARNINGHDR);
            pRpt->WriteLine(strError);
          }
          Format(strError, IDSC_IMPORTWARNING2, pci_curitem->GetGroup().c_str(),
                 pci_curitem->GetTitle().c_str(), pci_curitem->GetUser().c_str());
          pRpt->WriteLine(strError);
          LoadAString(strError, IDSC_IMPORTWARNING2A);
          pRpt->WriteLine(strError);
        }
        if (type == CItemData::ET_SHORTCUT) {
          if (pmapDeletedItems != NULL)
            pmapDeletedItems->insert(ItemList_Pair(*paiter, *pci_curitem));
        } else {
          if (pmapSaveTypePW != NULL) {
            st_typepw.et = CItemData::ET_ALIAS;
            st_typepw.sxpw = _T("");
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          pci_curitem->SetNormal(); // but can make invalid alias a normal entry
        }

        num_warnings++;
      }
    }
  }
  return num_warnings;
}

void PWScore::UndoAddDependentEntries(ItemList *pmapDeletedItems,
                                      SaveTypePWMap *pmapSaveTypePW)
{
  ItemListIter iter, add_iter;
  SaveTypePWMap::iterator restore_iter;

  for (add_iter = pmapDeletedItems->begin(); 
       add_iter != pmapDeletedItems->end(); 
       add_iter++) {
    m_pwlist[add_iter->first] = add_iter->second;
  }

  for (restore_iter = pmapSaveTypePW->begin();
       restore_iter != pmapSaveTypePW->end(); 
       restore_iter++) {
    iter = m_pwlist.find(restore_iter->first);
    if (iter == m_pwlist.end())
      continue;

    CItemData *pci_changeditem = &iter->second;
    st_SaveTypePW *pst_typepw = &restore_iter->second;
    pci_changeditem->SetEntryType(pst_typepw->et);
    if (!pst_typepw->sxpw.empty())
      pci_changeditem->SetPassword(pst_typepw->sxpw);
  }
}

void PWScore::ResetAllAliasPasswords(const uuid_array_t &base_uuid)
{
  // Alias ONLY - no shortcut version needed
  ItemMMapIter itr;
  ItemMMapIter lastElement;
  ItemListIter base_itr, alias_itr;
  uuid_array_t alias_uuid;
  StringX csBasePassword;

  itr = m_base2aliases_mmap.find(base_uuid);
  if (itr == m_base2aliases_mmap.end())
    return;

  base_itr = m_pwlist.find(base_uuid);
  if (base_itr != m_pwlist.end()) {
    csBasePassword = base_itr->second.GetPassword();
  } else {
    m_base2aliases_mmap.erase(base_uuid);
    return;
  }

  lastElement = m_base2aliases_mmap.upper_bound(base_uuid);

  for ( ; itr != lastElement; itr++) {
    itr->second.GetUUID(alias_uuid);
    alias_itr = m_pwlist.find(alias_uuid);
    if (alias_itr != m_pwlist.end()) {
      alias_itr->second.SetPassword(csBasePassword);
      alias_itr->second.SetNormal();
      GUIRefreshEntry(alias_itr->second);
    }
  }
  m_base2aliases_mmap.erase(base_uuid);
}

void PWScore::GetAllDependentEntries(const uuid_array_t &base_uuid, UUIDVector &tlist,
                                     const CItemData::EntryType type)
{
  ItemMMapIter itr;
  ItemMMapIter lastElement;
  uuid_array_t uuid;

  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS)
    pmmap = &m_base2aliases_mmap;
  else if (type == CItemData::ET_SHORTCUT)
    pmmap = &m_base2shortcuts_mmap;
  else
    return;

  itr = pmmap->find(base_uuid);
  if (itr == pmmap->end())
    return;

  lastElement = pmmap->upper_bound(base_uuid);

  for ( ; itr != lastElement; itr++) {
    itr->second.GetUUID(uuid);
    tlist.push_back(uuid);
  }
}

bool PWScore::ParseBaseEntryPWD(const StringX &Password, BaseEntryParms &pl)
{
  // pl.ibasedata is:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but either no base entry found or no unique entry found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and is only valid if n = -1 or -2.

  pl.bMultipleEntriesFound = false;
  memset(pl.base_uuid, 0, sizeof(uuid_array_t));

  // Take a copy of the Password field to do the counting!
  StringX passwd(Password);

  int num_colonsP1 = Replace(passwd, _T(':'), _T(';')) + 1;
  if ((Password[0] == _T('[')) &&
      (Password[Password.length() - 1] == _T(']')) &&
      num_colonsP1 <= 3) {
    StringX tmp;
    ItemListIter iter;
    switch (num_colonsP1) {
      case 1:
        // [X] - OK if unique entry [g:X:u], [g:X:], [:X:u] or [:X:] exists for any value of g or u
        pl.csPwdTitle = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
        iter = GetUniqueBase(pl.csPwdTitle, pl.bMultipleEntriesFound);
        if (iter != m_pwlist.end()) {
          // Fill in the fields found during search
          pl.csPwdGroup = iter->second.GetGroup();
          pl.csPwdUser = iter->second.GetUser();
        }
        break;
      case 2:
        // [X:Y] - OK if unique entry [X:Y:u] or [g:X:Y] exists for any value of g or u
        pl.csPwdUser = _T("");
        tmp = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
        pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(":")));
        pl.csPwdTitle = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
        iter = GetUniqueBase(pl.csPwdGroup, pl.csPwdTitle, pl.bMultipleEntriesFound);
        if (iter != m_pwlist.end()) {
          // Fill in the fields found during search
          pl.csPwdGroup = iter->second.GetGroup();
          pl.csPwdTitle = iter->second.GetTitle();
          pl.csPwdUser = iter->second.GetUser();
        }
        break;
      case 3:
        // [X:Y:Z], [X:Y:], [:Y:Z], [:Y:] (title cannot be empty)
        tmp = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
        pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(":")));
        tmp = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
        pl.csPwdTitle = tmp.substr(0, tmp.find_first_of(_T(":")));    // Skip over 'title:'
        pl.csPwdUser = tmp.substr(pl.csPwdTitle.length() + 1);
        iter = Find(pl.csPwdGroup, pl.csPwdTitle, pl.csPwdUser);
        break;
      default:
        ASSERT(0);
    }
    if (iter != m_pwlist.end()) {
      pl.TargetType = iter->second.GetEntryType();
      if (pl.InputType == CItemData::ET_ALIAS && pl.TargetType == CItemData::ET_ALIAS) {
        // Check if base is already an alias, if so, set this entry -> real base entry
        uuid_array_t temp_uuid;
        iter->second.GetUUID(temp_uuid);
        GetDependentEntryBaseUUID(temp_uuid, pl.base_uuid, CItemData::ET_ALIAS);
      } else {
        // This may not be a valid combination of source+target entries - sorted out by caller
        iter->second.GetUUID(pl.base_uuid);
      }
      // Valid and found
      pl.ibasedata = num_colonsP1;
      return true;
    }
    // Valid but either exact [g:t:u] not found or
    //  no or multiple entries satisfy [x] or [x:y]
    pl.ibasedata  = -num_colonsP1;
    return true;
  }
  pl.ibasedata = 0; // invalid password format for an alias
  return false;
}

const CItemData *PWScore::GetBaseEntry(const CItemData *pAliasOrSC) const
{
  return const_cast<PWScore *>(this)->GetBaseEntry(pAliasOrSC);
}

CItemData *PWScore::GetBaseEntry(const CItemData *pAliasOrSC)
{
  // Alas, we need both a const and non-const version. 
  ASSERT(pAliasOrSC != NULL);
  CItemData::EntryType et = pAliasOrSC->GetEntryType();
  if (et != CItemData::ET_ALIAS && et != CItemData::ET_SHORTCUT) {
    pws_os::Trace(_T("PWScore::GetBaseEntry called with non-dependent element!\n"));
    return NULL;
  }
  uuid_array_t dep_uuid, base_uuid;
  pAliasOrSC->GetUUID(dep_uuid);
  if (!GetDependentEntryBaseUUID(dep_uuid, base_uuid, et)) {
    pws_os::Trace(_T("PWScore::GetBaseEntry - couldn't find base uuid!\n"));
    return NULL;
  }
  ItemListIter iter = Find(base_uuid);
  if (iter == GetEntryEndIter()) {
    pws_os::Trace(_T("PWScore::GetBaseEntry - Find(base_uuid) failed!\n"));
    return NULL;
  }
  return &iter->second;
}

bool PWScore::GetDependentEntryBaseUUID(const uuid_array_t &entry_uuid, 
                                        uuid_array_t &base_uuid, 
                                        const CItemData::EntryType type) const
{
  memset(base_uuid, 0, sizeof(uuid_array_t));

  const ItemMap *pmap;
  if (type == CItemData::ET_ALIAS)
    pmap = &m_alias2base_map;
  else if (type == CItemData::ET_SHORTCUT)
    pmap = &m_shortcut2base_map;
  else
    return false;

  ItemMapConstIter iter = pmap->find(entry_uuid);
  if (iter != pmap->end()) {
    iter->second.GetUUID(base_uuid);
    return true;
  } else {
    return false;
  }
}

// NotifyDBModified - used by GUI if the Database has changed
// particularly to invalidate any current Find results and to populate
// message during Vista and later shutdowns

void PWScore::NotifyDBModified()
{
  if (m_bNotifyDB && m_pUIIF != NULL)
    m_pUIIF->DatabaseModified(m_bDBChanged || m_bDBPrefsChanged);
}


void PWScore::NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action ga, 
                                     uuid_array_t &entry_uuid,
                                     CItemData::FieldType ft,
                                     bool bUpdateGUI)
{
  if (m_pUIIF != NULL)
    m_pUIIF->UpdateGUI(ga, entry_uuid, ft, bUpdateGUI);
}

void PWScore::GUISetupDisplayInfo(CItemData &ci)
{
  if (m_pUIIF != NULL)
    m_pUIIF->GUISetupDisplayInfo(ci);
}

void PWScore::GUIRefreshEntry(const CItemData &ci)
{
  if (m_pUIIF != NULL)
    m_pUIIF->GUIRefreshEntry(ci);
}

bool PWScore::LockFile(const stringT &filename, stringT &locker)
{
  return pws_os::LockFile(filename, locker,
                          m_lockFileHandle, m_LockCount);
}

bool PWScore::IsLockedFile(const stringT &filename) const
{
  return pws_os::IsLockedFile(filename);
}

void PWScore::UnlockFile(const stringT &filename)
{
  return pws_os::UnlockFile(filename, 
                            m_lockFileHandle, m_LockCount);
}

bool PWScore::LockFile2(const stringT &filename, stringT &locker)
{
  return pws_os::LockFile(filename, locker,
                          m_lockFileHandle2, m_LockCount2);
}

void PWScore::UnlockFile2(const stringT &filename)
{
  return pws_os::UnlockFile(filename, 
                            m_lockFileHandle2, m_LockCount2);
}

bool PWScore::IsNodeModified(StringX &path) const
{
  return std::find(m_vnodes_modified.begin(),
                   m_vnodes_modified.end(), path) != m_vnodes_modified.end();
}

void PWScore::AddChangedNodes(StringX path)
{
  StringX nextpath(path);
  while (!nextpath.empty()) {
    if (std::find(m_vnodes_modified.begin(), m_vnodes_modified.end(), nextpath) == m_vnodes_modified.end())
      m_vnodes_modified.push_back(nextpath);
    size_t i = nextpath.find_last_of(_T("."));
    if (i == nextpath.npos)
      i = 0;
    nextpath = nextpath.substr(0, i);
  }
}

// functor objects for updating password history for each entry

struct HistoryUpdater {
  HistoryUpdater(int &num_altered, 
                 SavePWHistoryMap &mapSavedHistory)
  : m_num_altered(num_altered), m_mapSavedHistory(mapSavedHistory)
  {}
  virtual void operator() (CItemData &ci) = 0;

protected:
  int &m_num_altered;
  std::map<CUUIDGen, StringX, CUUIDGen::ltuuid> &m_mapSavedHistory;

private:
  HistoryUpdater& operator=(const HistoryUpdater&); // Do not implement
};

struct HistoryUpdateResetOff : public HistoryUpdater {
  HistoryUpdateResetOff(int &num_altered, 
                        SavePWHistoryMap &mapSavedHistory)
 : HistoryUpdater(num_altered, mapSavedHistory) {}

  void operator()(CItemData &ci) {
    uuid_array_t item_uuid;
    ci.GetUUID(item_uuid);
    StringX cs_tmp = ci.GetPWHistory();
    if (cs_tmp.length() >= 5 && cs_tmp[0] == L'1') {
      m_mapSavedHistory[item_uuid] = cs_tmp;
      cs_tmp[0] = L'0';
      ci.SetPWHistory(cs_tmp);
      m_num_altered++;
    }
  }

private:
  HistoryUpdateResetOff& operator=(const HistoryUpdateResetOff&); // Do not implement
};

struct HistoryUpdateResetOn : public HistoryUpdater {
  HistoryUpdateResetOn(int &num_altered, int new_default_max,
                       SavePWHistoryMap &mapSavedHistory)
    : HistoryUpdater(num_altered, mapSavedHistory)
  {Format(m_text, _T("1%02x00"), new_default_max);}

  void operator()(CItemData &ci) {
    uuid_array_t item_uuid;
    ci.GetUUID(item_uuid);
    StringX cs_tmp = ci.GetPWHistory();
    if (cs_tmp.length() < 5) {
      m_mapSavedHistory[item_uuid] = cs_tmp;
      ci.SetPWHistory(m_text);
      m_num_altered++;
    } else {
      if (cs_tmp[0] == L'0') {
        m_mapSavedHistory[item_uuid] = cs_tmp;
        cs_tmp[0] = L'1';
        ci.SetPWHistory(cs_tmp);
        m_num_altered++;
      }
    }
  }

private:
  HistoryUpdateResetOn& operator=(const HistoryUpdateResetOn&); // Do not implement
  StringX m_text;
};

struct HistoryUpdateSetMax : public HistoryUpdater {
  HistoryUpdateSetMax(int &num_altered, int new_default_max,
                      SavePWHistoryMap &mapSavedHistory)
    : HistoryUpdater(num_altered, mapSavedHistory),
    m_new_default_max(new_default_max)
  {Format(m_text, _T("1%02x"), new_default_max);}

  void operator()(CItemData &ci) {
    uuid_array_t item_uuid;
    ci.GetUUID(item_uuid);
    StringX cs_tmp = ci.GetPWHistory();

    size_t len = cs_tmp.length();
    if (len >= 5) {
      m_mapSavedHistory[item_uuid] = cs_tmp;
      int status, old_max, num_saved;
      const wchar_t *lpszPWHistory = cs_tmp.c_str();
#if (_MSC_VER >= 1400)
      int iread = swscanf_s(lpszPWHistory, _T("%01d%02x%02x"), 
                             &status, &old_max, &num_saved);
#else
      int iread = swscanf(lpszPWHistory, _T("%01d%02x%02x"),
                           &status, &old_max, &num_saved);
#endif
      if (iread == 3 && status == 1 && num_saved <= m_new_default_max) {
        cs_tmp = m_text + cs_tmp.substr(3);
        ci.SetPWHistory(cs_tmp);
        m_num_altered++;
      }
    }
  }

private:
  HistoryUpdateSetMax& operator=(const HistoryUpdateSetMax&); // Do not implement
  int m_new_default_max;
  StringX m_text;
};

int PWScore::DoUpdatePasswordHistory(int iAction, int new_default_max,
                                     SavePWHistoryMap &mapSavedHistory)
{
  int num_altered = 0;
  HistoryUpdater *updater = NULL;

  HistoryUpdateResetOff reset_off(num_altered, mapSavedHistory);
  HistoryUpdateResetOn reset_on(num_altered, new_default_max, mapSavedHistory);
  HistoryUpdateSetMax set_max(num_altered, new_default_max, mapSavedHistory);

  switch (iAction) {
    case 1:   // reset off
      updater = &reset_off;
      break;
    case 2:   // reset on
      updater = &reset_on;
      break;
    case 3:   // setmax
      updater = &set_max;
      break;
    default:
      ASSERT(0);
      break;
  } // switch (iAction)

  /**
  * Interesting problem - a for_each iterator
  * cause a copy c'tor of the pair to be invoked, resulting
  * in a temporary copy of the CItemDatum being modified.
  * Couldn't find a handy way to workaround this (e.g.,
  * operator()(pair<...> &p) failed to compile
  * so reverted to slightly less elegant for loop
  * using polymorphism for the history updater
  * is an unrelated tweak.
  */

  if (updater != NULL) {
    ItemListIter listPos;
    for (listPos = m_pwlist.begin(); listPos != m_pwlist.end(); listPos++) {
      CItemData &curitem = listPos->second;
      (*updater)(curitem);
    }
  }
  return num_altered;
}

void PWScore::UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory)
{
  SavePWHistoryMap::iterator itr;

  for (itr = mapSavedHistory.begin(); itr != mapSavedHistory.end(); itr++) {
    ItemListIter listPos = m_pwlist.find(itr->first);
    if (listPos != m_pwlist.end()) {
      listPos->second.SetPWHistory(itr->second);
    }
  }
}

void PWScore::GetDBProperties(st_DBProperties &st_dbp)
{
  st_dbp.database = m_currfile;

  Format(st_dbp.databaseformat, _T("%d.%02d"),
                          m_hdr.m_nCurrentMajorVersion,
                          m_hdr.m_nCurrentMinorVersion);

  std::vector<std::wstring> aryGroups;
  GetUniqueGroups(aryGroups);
  Format(st_dbp.numgroups, _T("%d"), aryGroups.size());
  Format(st_dbp.numentries, _T("%d"), m_pwlist.size());

  time_t twls = m_hdr.m_whenlastsaved;
  if (twls == 0) {
    LoadAString(st_dbp.whenlastsaved, IDSC_UNKNOWN);
  } else {
    st_dbp.whenlastsaved = PWSUtil::ConvertToDateTimeString(twls, TMC_EXPORT_IMPORT);
  }

  if (m_hdr.m_lastsavedby.empty() && m_hdr.m_lastsavedon.empty()) {
    LoadAString(st_dbp.wholastsaved, IDSC_UNKNOWN);
  } else {
    StringX user = m_hdr.m_lastsavedby.empty() ?
                          _T("?") : m_hdr.m_lastsavedby.c_str();
    StringX host = m_hdr.m_lastsavedon.empty() ?
                          _T("?") : m_hdr.m_lastsavedon.c_str();
    Format(st_dbp.wholastsaved, IDSC_USERONHOST, user.c_str(), host.c_str());
  }

  if (m_hdr.m_whatlastsaved.empty()) {
    LoadAString(st_dbp.whatlastsaved, IDSC_UNKNOWN);
  } else
    st_dbp.whatlastsaved = m_hdr.m_whatlastsaved;

  uuid_array_t file_uuid_array;
  memcpy(file_uuid_array, m_hdr.m_file_uuid_array, sizeof(uuid_array_t));

  if (memcmp(file_uuid_array, NULL_UUID, sizeof(uuid_array_t)) == 0)
    st_dbp.file_uuid = _T("N/A");
  else {
    ostringstreamT os;
    CUUIDGen huuid(file_uuid_array, true); // true for canonical format
    os << uppercase << huuid;
    st_dbp.file_uuid = os.str().c_str();
  }

  int num = m_nRecordsWithUnknownFields;
  if (num != 0 || !m_UHFL.empty()) {
    StringX cs_Yes, cs_No, cs_HdrYesNo;
    LoadAString(cs_Yes, IDSC_YES);
    LoadAString(cs_No, IDSC_NO);
    cs_HdrYesNo = m_UHFL.empty() ? cs_No : cs_Yes;

    Format(st_dbp.unknownfields, IDSC_UNKNOWNFIELDS, cs_HdrYesNo.c_str());
    if (num == 0) {
      st_dbp.unknownfields += cs_No;
      st_dbp.unknownfields += _T(")");
    } else {
      StringX wls;
      Format(wls, _T("%d"), num);
      st_dbp.unknownfields += wls;
      st_dbp.unknownfields += _T(")");
    }
  } else {
    LoadAString(st_dbp.unknownfields, IDSC_NONE);
  }
}
