/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file PWScore.cpp
//-----------------------------------------------------------------------------

#include "PWScore.h"
#include "core.h"
#include "TwoFish.h"
#include "PWSprefs.h"
#include "PWHistory.h"
#include "PWSrand.h"
#include "Util.h"
#include "SysInfo.h"
#include "UTF8Conv.h"
#include "Report.h"
#include "VerifyFormat.h"
#include "StringXStream.h"

#include "os/pws_tchar.h"
#include "os/typedefs.h"
#include "os/dir.h"
#include "os/debug.h"
#include "os/env.h"
#include "os/file.h"
#include "os/mem.h"
#include "os/logit.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <set>
#include <iterator>

extern const TCHAR *GROUPTITLEUSERINCHEVRONS;

using pws_os::CUUID;

unsigned char PWScore::m_session_key[32];
bool PWScore::m_session_initialized = false;
Asker *PWScore::m_pAsker = nullptr;
Reporter *PWScore::m_pReporter = nullptr;

// Following structure used in ReadFile and Validate and entries using
// Named Password Policy
static bool GTUCompareV1(const st_GroupTitleUser &gtu1, const st_GroupTitleUser &gtu2)
{
  if (gtu1.group != gtu2.group)
    return gtu1.group.compare(gtu2.group) < 0;
  else if (gtu1.title != gtu2.title)
    return gtu1.title.compare(gtu2.title) < 0;
  else
    return gtu1.user.compare(gtu2.user) < 0;
}

// Helper struct for results of a database verification
struct st_ValidateResults {
  int num_invalid_UUIDs;
  int num_duplicate_UUIDs;
  int num_empty_titles;
  int num_empty_passwords;
  int num_duplicate_GTU_fixed;
  int num_PWH_fixed;
  int num_excessivetxt_found;
  int num_alias_warnings;
  int num_shortcuts_warnings;
  int num_missing_att;
  int num_orphan_att;

  st_ValidateResults()
    : num_invalid_UUIDs(0), num_duplicate_UUIDs(0),
    num_empty_titles(0), num_empty_passwords(0),
    num_duplicate_GTU_fixed(0),
    num_PWH_fixed(0), num_excessivetxt_found(0),
    num_alias_warnings(0), num_shortcuts_warnings(0),
    num_missing_att(0), num_orphan_att(0)
  {}

  st_ValidateResults(const st_ValidateResults &that)
    : num_invalid_UUIDs(that.num_invalid_UUIDs),
    num_duplicate_UUIDs(that.num_duplicate_UUIDs),
    num_empty_titles(that.num_empty_titles),
    num_empty_passwords(that.num_empty_passwords),
    num_duplicate_GTU_fixed(that.num_duplicate_GTU_fixed),
    num_PWH_fixed(that.num_PWH_fixed),
    num_excessivetxt_found(that.num_excessivetxt_found),
    num_alias_warnings(that.num_alias_warnings),
    num_shortcuts_warnings(that.num_shortcuts_warnings),
    num_missing_att(that.num_missing_att), num_orphan_att(that.num_orphan_att)
  {}

  st_ValidateResults &operator=(const st_ValidateResults &that) {
    if (this != &that) {
      num_invalid_UUIDs = that.num_invalid_UUIDs;
      num_duplicate_UUIDs = that.num_duplicate_UUIDs;
      num_empty_titles = that.num_empty_titles;
      num_empty_passwords = that.num_empty_passwords;
      num_duplicate_GTU_fixed = that.num_duplicate_GTU_fixed;
      num_PWH_fixed = that.num_PWH_fixed;
      num_excessivetxt_found = that.num_excessivetxt_found;
      num_alias_warnings = that.num_alias_warnings;
      num_shortcuts_warnings = that.num_shortcuts_warnings;
      num_missing_att = that.num_missing_att;
      num_orphan_att = that.num_orphan_att;
    }
    return *this;
  }

  int TotalIssues()
  {
    return (num_invalid_UUIDs + num_duplicate_UUIDs +
      num_empty_titles + num_empty_passwords +
      num_duplicate_GTU_fixed +
      num_PWH_fixed + num_excessivetxt_found +
      num_alias_warnings + num_shortcuts_warnings +
      num_missing_att + num_orphan_att);
  }
};

//-----------------------------------------------------------------

PWScore::PWScore() :
                     m_isAuxCore(false),
                     m_currfile(_T("")),
                     m_passkey(nullptr), m_passkey_len(0),
                     m_hashIters(MIN_HASH_ITERATIONS),
                     m_lockFileHandle(INVALID_HANDLE_VALUE),
                     m_lockFileHandle2(INVALID_HANDLE_VALUE),
                     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
                     m_bIsReadOnly(false),
                     m_bNotifyDB(false),
                     m_bIsOpen(false),
                     m_nRecordsWithUnknownFields(0),
                     m_DBCurrentState(CLEAN),
                     m_pFileSig(nullptr),
                     m_iAppHotKey(0)
{
  // following should ideally be wrapped in a mutex
  if (!PWScore::m_session_initialized) {
    PWScore::m_session_initialized = true;
    pws_os::mlock(m_session_key, sizeof(m_session_key));
    PWSrand::GetInstance()->GetRandomData(m_session_key, sizeof(m_session_key));
    if (!pws_os::mcryptProtect(m_session_key, sizeof(m_session_key))) {
      pws_os::Trace(_T("pws_os::mcryptProtect failed"));
    }
  }
  m_undo_iter = m_redo_iter = m_vpcommands.end();
  m_undo_DBState_iter = m_redo_DBState_iter = m_vDBState.end();
}

PWScore::~PWScore()
{
  // do NOT trash m_session_*, as there may be other cores around
  // relying on it. Trashing the ciphertext encrypted with it is enough
  const unsigned int BS = TwoFish::BLOCKSIZE;
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS - 1)) / BS) * BS);
    delete[] m_passkey;
    m_passkey = nullptr;
    m_passkey_len = 0;
  }

  m_UHFL.clear();
  m_vModifiedNodes.clear();

  delete m_pFileSig;
}

void PWScore::SetApplicationNameAndVersion(const stringT &appName,
                                           DWORD dwMajorMinor,
                                           DWORD dwBuildRevision)
{
  int nMajor = HIWORD(dwMajorMinor);
  int nMinor = LOWORD(dwMajorMinor);
  int nRevison = HIWORD(dwBuildRevision);
  if (nRevison == 0)
    Format(m_AppNameAndVersion, L"%ls V%d.%02d", appName.c_str(),
           nMajor, nMinor);
  else
    Format(m_AppNameAndVersion, L"%ls V%d.%02d.%d", appName.c_str(),
           nMajor, nMinor, nRevison);

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

void PWScore::SortDependents(UUIDVector &dlist, StringX &sxDependents)
{
  std::vector<StringX> sorted_dependents;
  std::vector<StringX>::iterator sd_iter;

  SortDependents(dlist, sorted_dependents);
  sxDependents = _T("");

  for (sd_iter = sorted_dependents.begin(); sd_iter != sorted_dependents.end(); sd_iter++)
    sxDependents += _T("\t[") +  *sd_iter + _T("]\r\n");
}

void PWScore::SortDependents(UUIDVector &dlist, std::vector<StringX> &vsxDependents)
{
  ItemListIter iter;
  UUIDVectorIter diter;

  for (diter = dlist.begin(); diter != dlist.end(); diter++) {
    iter = Find(*diter);
    if (iter != GetEntryEndIter()) {
      StringX sx_dependent;
      sx_dependent = _T("[") +
                            iter->second.GetGroup() + _T(":") +
                            iter->second.GetTitle() + _T(":") +
                            iter->second.GetUser()  +
                     _T("]");
      vsxDependents.push_back(sx_dependent);
    }
  }

  std::sort(vsxDependents.begin(), vsxDependents.end(), GTUCompare);
}

void PWScore::DoAddEntry(const CItemData &item, const CItemAtt *att)
{
  // Also "UndoDeleteEntry" !
  ASSERT(m_pwlist.find(item.GetUUID()) == m_pwlist.end());
  m_pwlist[item.GetUUID()] = item;

  if (item.NumberUnknownFields() > 0)
    IncrementNumRecordsWithUnknownFields();

  if (item.IsNormal() && item.IsPolicyNameSet()) {
    IncrementPasswordPolicy(item.GetPolicyName());
  }

  if (att != nullptr && att->HasContent()) {
    m_pwlist[item.GetUUID()].SetAttUUID(att->GetUUID());
    if (m_attlist.find(att->GetUUID()) == m_attlist.end())
      m_attlist.insert(std::make_pair(att->GetUUID(), *att));
    m_attlist[att->GetUUID()].IncRefcount();
  }

  int32 iKBShortcut;
  item.GetKBShortcut(iKBShortcut);

  if (iKBShortcut != 0)
    VERIFY(AddKBShortcut(iKBShortcut, item.GetUUID()));
}

bool PWScore::ConfirmDelete(const CItemData *pci)
{
  ASSERT(pci != nullptr);
  if (pci->IsBase() && m_pAsker != nullptr) {
    UUIDVector dependentslist;
    CUUID entry_uuid = pci->GetUUID();
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
      StringX sxDependents;
      SortDependents(dependentslist, sxDependents);

      stringT cs_msg, cs_type;
      stringT cs_title;
      LoadAString(cs_title, IDSC_DELETEBASET);
      if (entrytype == CItemData::ET_ALIASBASE) {
        LoadAString(cs_type, num_dependents == 1 ? IDSC_ALIAS : IDSC_ALIASES);
        Format(cs_msg, IDSC_DELETEABASE, dependentslist.size(),
               cs_type.c_str(), sxDependents.c_str());
      } else {
        LoadAString(cs_type, num_dependents == 1 ? IDSC_SHORTCUT : IDSC_SHORTCUTS);
        Format(cs_msg, IDSC_DELETESBASE, dependentslist.size(),
               cs_type.c_str(), sxDependents.c_str());
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

  CUUID entry_uuid = item.GetUUID();
  auto pos = m_pwlist.find(entry_uuid);
  if (pos != m_pwlist.end()) {
    // Simple cases first: Aliases or shortcuts, update maps
    // and refresh base's display, if changed
    if (item.IsDependent()) {
      CUUID base_uuid = item.GetBaseUUID();
      CItemData::EntryType entrytype = item.GetEntryType();
      DoRemoveDependentEntry(base_uuid, entry_uuid, entrytype);
    } else if (item.IsAliasBase()) {
      ResetAllAliasPasswords(entry_uuid);
    } else if (item.IsShortcutBase()) {
      // Recurse on all dependents
      ItemMMap deps(m_base2shortcuts_mmap.lower_bound(entry_uuid),
                    m_base2shortcuts_mmap.upper_bound(entry_uuid));
      for (ItemMMapIter iter = deps.begin(); iter != deps.end(); iter++) {
        CItemData depItem = Find(iter->second)->second;
        // Set deleted for GUIRefreshEntry() which will remove from display
        depItem.SetStatus(CItemData::ES_DELETED);
        GUIRefreshEntry(depItem);
        
        DoDeleteEntry(depItem);
      }
    }

    int32 iKBShortcut;
    item.GetKBShortcut(iKBShortcut);

    if (iKBShortcut != 0)
      VERIFY(DelKBShortcut(iKBShortcut, item.GetUUID()));

    m_pwlist.erase(pos); // at last!

    if (item.NumberUnknownFields() > 0)
      DecrementNumRecordsWithUnknownFields();

    if (item.IsNormal() && item.IsPolicyNameSet()) {
      DecrementPasswordPolicy(item.GetPolicyName());
    }

    if (item.HasAttRef()) {
      CItemAtt &att = GetAtt(item.GetAttUUID());
      if (att.GetRefcount() == 1)
        RemoveAtt(item.GetAttUUID());
      else
        att.DecRefcount();
    }
  } // pos != m_pwlist.end()
}

void PWScore::DoReplaceEntry(const CItemData &old_ci, const CItemData &new_ci)
{
  // Assumes that old_uuid == new_uuid
  ASSERT(old_ci.GetUUID() == new_ci.GetUUID());
  m_pwlist[old_ci.GetUUID()] = new_ci;
  if (old_ci.GetEntryType() != new_ci.GetEntryType() || old_ci.GetStatus() != new_ci.GetStatus() ||
      old_ci.IsProtected() != new_ci.IsProtected())
    GUIRefreshEntry(new_ci);

  // Check if we need to update Expiry vector
  time_t tttXTime_old, tttXTime_new;
  old_ci.GetXTime(tttXTime_old);
  new_ci.GetXTime(tttXTime_new);

  if (tttXTime_old != tttXTime_new) {
    if (tttXTime_old != time_t(0))
      RemoveExpiryEntry(old_ci);
    if (tttXTime_new != time_t(0))
      AddExpiryEntry(new_ci);
  }

  if (old_ci.IsNormal() && old_ci.IsPolicyNameSet()) {
    DecrementPasswordPolicy(old_ci.GetPolicyName());
  }

  if (new_ci.IsNormal() && new_ci.IsPolicyNameSet()) {
    IncrementPasswordPolicy(new_ci.GetPolicyName());
  }

  int ioldKBShortcut, inewKBShortcut;
  old_ci.GetKBShortcut(ioldKBShortcut);
  new_ci.GetKBShortcut(inewKBShortcut);

  if (ioldKBShortcut != inewKBShortcut) {
    if (ioldKBShortcut != 0)
      VERIFY(DelKBShortcut(ioldKBShortcut, old_ci.GetUUID()));
    if (inewKBShortcut != 0)
      VERIFY(AddKBShortcut(inewKBShortcut, new_ci.GetUUID()));
  }
}

#if 0

void PWScore::DoDeleteAtt(const CItemAtt &att)
{
  /**
   * Note that we do NOT erase reference uuid in owner record(s)
   */

  // Make sure att's there
  auto pos = m_attlist.find(att.GetUUID());
  ASSERT(pos != m_attlist.end());

  m_attlist.erase(pos);
}
#endif

void PWScore::ClearDBData()
{
  const unsigned int BS = TwoFish::BLOCKSIZE;
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS - 1)) / BS) * BS);
    delete[] m_passkey;
    m_passkey = nullptr;
    m_passkey_len = 0;
  }
  m_passkey = nullptr;

  //Composed of ciphertext, so doesn't need to be overwritten
  m_pwlist.clear();
  m_attlist.clear();

  // Clear out out dependents mappings
  m_base2aliases_mmap.clear();
  m_base2shortcuts_mmap.clear();

  // Clear out unknown fields
  m_nRecordsWithUnknownFields = 0;
  m_UHFL.clear();

  // Clear out database filters
  m_MapDBFilters.clear();
  m_InitialMapDBFilters.clear();

  // Clear out policies
  m_MapPSWDPLC.clear();
  m_InitialMapPSWDPLC.clear();

  // Clear out Empty Groups
  m_vEmptyGroups.clear();
  m_InitialEmptyGroups.clear();

  // Reset DB pre-command state to clean
  m_DBCurrentState = CLEAN;

  // Clear changed nodes
  m_vModifiedNodes.clear();

  // Clear expired password entries
  m_ExpireCandidates.clear();

  // Clear entry keyboard shortcuts
  m_KBShortcutMap.clear();

  // Clear any unknown preferences from previous databases
  PWSprefs::GetInstance()->ClearUnknownPrefs();

  // OK now closed
  m_bIsOpen = false;
}

void PWScore::ReInit(bool bNewFile)
{
  PWS_LOGIT;

  // Now reset all values as if created from new
  if (bNewFile)
    m_ReadFileVersion = PWSfile::NEWFILE;
  else
    m_ReadFileVersion = PWSfile::UNKNOWN_VERSION;

  // Clear all internal variables EXCEPT command and DB state vectors
  ClearDBData();

  // Now clear out commands and DB pre-command states
  ClearCommands();
}

void PWScore::NewFile(const StringX &passkey)
{
  ClearDBData();
  SetPassKey(passkey);
  time(&m_hdr.m_whenpwdlastchanged); // update master password changed timestamp
  m_ReadFileVersion = PWSfile::VCURRENT;
}

// functor object type for for_each:
// Writes out all records to a PasswordSafe database of any version
struct RecordWriter {
  RecordWriter(PWSfile *pout, PWScore *pcore, PWSfile::VERSION version)
    : m_pout(pout), m_pcore(pcore), m_version(version) {}

  void operator()(std::pair<CUUID const, CItemData> &p)
  {
    if (p.second.IsAlias() && m_version < PWSfile::V30) {
      // Pre V30 does not support aliases.  Write as a normal record
      // with the base record's password
      CItemData ci = p.second;
      CItemData *pbase = m_pcore->GetBaseEntry(&(p.second));
      ci.SetPassword(pbase->GetPassword());
      m_pout->WriteRecord(ci);
      return;
    } else if (p.second.IsShortcut() && m_version < PWSfile::V30) {
      // Pre V30 does not support shortcuts at all - ignore completely
      return;
    }
    m_pout->WriteRecord(p.second);
    p.second.ClearStatus();
  }

private:
  RecordWriter& operator=(const RecordWriter&); // Do not implement
  PWSfile *m_pout;
  PWScore *m_pcore;
  const PWSfile::VERSION m_version;
};

int PWScore::WriteFile(const StringX &filename, PWSfile::VERSION version,
                       bool bUpdateSig)
{
  PWS_LOGIT_ARGS("bUpdateSig=%ls", bUpdateSig ? L"true" : L"false");

  int status;

  PWSfile *out = PWSfile::MakePWSfile(filename, GetPassKey(), version,
                                      PWSfile::Write, status);

  if (status != PWSfile::SUCCESS) {
    delete out;
    return status;
  }

  if (bUpdateSig) {
    // since we're writing a new file, the previous sig's
    // about to be invalidated but NOT if a user initiated Backup
    delete m_pFileSig;
    m_pFileSig = nullptr;
  }

  // If writing in a prior version format (ie. exporting) - save the header
  const PWSfileHeader saved_hdr = m_hdr;

  m_hdr.m_prefString = PWSprefs::GetInstance()->Store();
  m_hdr.m_whatlastsaved = m_AppNameAndVersion.c_str();
  m_hdr.m_RUEList = m_RUEList;

  out->SetHeader(m_hdr);
  out->SetUnknownHeaderFields(m_UHFL);
  out->SetNHashIters(GetHashIters());
  out->SetDBFilters(m_MapDBFilters);
  out->SetPasswordPolicies(m_MapPSWDPLC);
  out->SetEmptyGroups(m_vEmptyGroups);

  try { // exception thrown on write error
    pws_os::setenv("PWS_PK_CP_ACP", ""); // safety, in case someone tries to set this globally
    status = out->Open(GetPassKey());

    if (status != PWSfile::SUCCESS) {
      delete out;

      if (version < m_ReadFileVersion) // Exporting - restore saved header
        m_hdr = saved_hdr;

      return status;
    }

    RecordWriter write_record(out, this, version);
    for_each(m_pwlist.begin(), m_pwlist.end(), write_record);

    // Write attachments (only from V4)
    if (version >= PWSfile::V40)
      for_each(m_attlist.begin(), m_attlist.end(),
               [&](std::pair<CUUID const, CItemAtt> &p)
               {
                 p.second.Write(out);
               } );

    // Update header if V30 or later (no headers before V30)
    if (version >= PWSfile::V30) {
      m_hdr = out->GetHeader(); // update time saved, etc.
    }
  }

  catch (...) {
    out->Close();
    delete out;

    if (version < m_ReadFileVersion) // Exporting - restore saved header
      m_hdr = saved_hdr;

    return FAILURE;
  }

  out->Close();
  delete out;

  // Update info if we're saving or upgrading.
  if (version >= m_ReadFileVersion) {
    // Set/Reset everything as "unchanged"
    SetInitialValues();

    m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]
  } else { // Exporting to older version - restore saved header
    m_hdr = saved_hdr;
  }

  // Create new signature if required
  if (bUpdateSig)
    m_pFileSig = new PWSFileSig(filename.c_str());

  // If not exporting, set to clean
  if (version == m_ReadFileVersion) {
    // Set current state to CLEAN
    m_DBCurrentState = CLEAN;

    std::vector<DBStates>::iterator iter;

    if (m_redo_DBState_iter != m_vDBState.end()) {
      // Update command after of this one to be {before = CLEAN, after = DIRTY}
      m_redo_DBState_iter->before = CLEAN;
      m_redo_DBState_iter->after = DIRTY;

      // Update all additional commands after of the next one to be
      // {before = DIRTY, after = DIRTY}
      iter = m_redo_DBState_iter + 1;
      for (; iter != m_vDBState.end(); iter++) {
        iter->before = DIRTY;
        iter->after = DIRTY;
      }
    }

    if (m_undo_DBState_iter != m_vDBState.end()) {
      // Update command before this one to be {before = DIRTY, after = CLEAN}
      m_undo_DBState_iter->before = DIRTY;
      m_undo_DBState_iter->after = CLEAN;

      // Update all additional commands before the previous one to be
      // {before = DIRTY, after = DIRTY}
      iter = m_undo_DBState_iter;
      while (iter != m_vDBState.begin()) {
        iter--;
        iter->before = DIRTY;
        iter->after = DIRTY;
      }
    }
  }
  return SUCCESS;
}

// functor object type for for_each:
// Writes out subset of records to a PasswordSafe database at the current version
// Used by Export entry or Export Group
struct ExportRecordWriter {
  ExportRecordWriter(PWSfile *pout, PWScore *pcore, std::vector<pws_os::CUUID> &vuuidAddedBases,
    CReport *pRpt) :
    m_pout(pout), m_pcore(pcore), m_pRpt(pRpt), m_vuuidAddedBases(vuuidAddedBases) {}

  void operator()(CItemData &item)
  {
    StringX savePassword = item.GetPassword();
    StringX uuid_str(savePassword);
    CUUID base_uuid(CUUID::NullUUID());
    CUUID item_uuid = item.GetUUID();
    bool bAddToReport(true);

    if (item.IsAlias()) {
      base_uuid = item.GetBaseUUID();
      uuid_str = _T("[[");
      uuid_str += base_uuid;
      uuid_str += _T("]]");
    }
    else if (item.IsShortcut()) {
      base_uuid = item.GetBaseUUID();
      uuid_str = _T("[~");
      uuid_str += base_uuid;
      uuid_str += _T("~]");
    }
    else if (item.IsBase()) {
      bAddToReport = std::find(m_vuuidAddedBases.begin(), m_vuuidAddedBases.end(), item_uuid) == 
                       m_vuuidAddedBases.end();
    }

    item.SetPassword(uuid_str);
    m_pout->WriteRecord(item);
    item.SetPassword(savePassword);

    if (m_pRpt != nullptr && bAddToReport) {
      StringX sx_exported;
      Format(sx_exported, GROUPTITLEUSERINCHEVRONS,
        item.GetGroup().c_str(), item.GetTitle().c_str(), item.GetUser().c_str());
      m_pRpt->WriteLine(sx_exported.c_str(), true);
    }
  }

private:
  PWSfile *m_pout;
  PWScore *m_pcore;
  CReport *m_pRpt;
  std::vector<pws_os::CUUID> m_vuuidAddedBases;
};

int PWScore::WriteExportFile(const StringX &filename, OrderedItemList *pOIL,
                             PWScore *pINcore, PWSfile::VERSION version, 
                             std::vector<StringX> &vEmptyGroups,
                             bool bExportDBFilters,
                             std::vector<pws_os::CUUID> &vuuidAddedBases,
                             CReport *pRpt)
{
  // Writes out subset of database records (as supplied in OrderedItemList)
  // to a PasswordSafe database at the current version
  // Used by Export entry or Export Group
  int status;
  PWSfile *out = PWSfile::MakePWSfile(filename, GetPassKey(), version,
                                      PWSfile::Write, status);

  if (status != PWSfile::SUCCESS) {
    delete out;
    return status;
  }

  // m_hdr.m_prefString = PWSprefs::GetInstance()->Store(); - no need to expose this
  m_hdr.m_whatlastsaved = m_AppNameAndVersion.c_str();

  // Get current DB name and save in exported DB description
  std::wstring sx_dontcare, sx_file, sx_extn;
  pws_os::splitpath(pINcore->GetCurFile().c_str(), sx_dontcare, sx_dontcare, sx_file, sx_extn);
  Format(m_hdr.m_DB_Description, IDSC_EXPORTDESCRIPTION, (sx_file + sx_extn).c_str());

  // Set new header
  out->SetHeader(m_hdr);

  out->SetNHashIters(GetHashIters());

  // Write out empty groups
  if (!vEmptyGroups.empty()) {
    out->SetEmptyGroups(vEmptyGroups);
  }

  // Build a list of Named Password Polices used by exported entries
  std::vector<StringX> vPWPolicies;

  // As not exporting the whole database, only get referenced Password Policies
  PopulatePWPVector pwpv(&vPWPolicies);
  for_each(pOIL->begin(), pOIL->end(), pwpv);

  // Only include Named Policies in map that are being used by exported entries
  PSWDPolicyMap ExportMapPSWDPLC;
  PSWDPolicyMapCIter iter;
  for (iter = pINcore->m_MapPSWDPLC.begin(); iter != pINcore->m_MapPSWDPLC.end(); iter++) {
    if (std::find(vPWPolicies.begin(), vPWPolicies.end(), iter->first) != vPWPolicies.end()) {
      ExportMapPSWDPLC[iter->first] = iter->second;
    }
  }

  out->SetPasswordPolicies(ExportMapPSWDPLC); // Now give it the password policies to write out

  if (bExportDBFilters) {
    out->SetDBFilters(pINcore->m_MapDBFilters);
    if (pRpt != nullptr) {
      StringX sx_exportedfilters;
      LoadAString(sx_exportedfilters, IDSC_FILTERSEXPORTEDTODB);
      pRpt->WriteLine(sx_exportedfilters.c_str(), true);
      pRpt->WriteLine();
    }
  }
  
  try { // exception thrown on write error
    pws_os::setenv("PWS_PK_CP_ACP", ""); // safety, in case someone tries to set this globally
    status = out->Open(GetPassKey());

    if (status != PWSfile::SUCCESS) {
      delete out;
      return status;
    }

    ExportRecordWriter write_record(out, pINcore, vuuidAddedBases, pRpt);
    for_each(pOIL->begin(), pOIL->end(), write_record);

  }
  catch (...) {
    out->Close();
    delete out;
    return FAILURE;
  }
  out->Close();
  delete out;

  return SUCCESS;
}

void PWScore::ClearCommands()
{
  // ONLY do this at each DB Open (including new DB) & Close and application exit
  // Do NOT call this when clearing DB entries at DB lock as the user
  // will not be able to undo any changes after unlocking the DB
  // Should only be called from PWScore::ReInit

  // Clear commands
  while (!m_vpcommands.empty()) {
    delete m_vpcommands.back();
    m_vpcommands.pop_back();
  }
  m_undo_iter = m_redo_iter = m_vpcommands.end();

  // Clear DB states
  m_vDBState.clear();
  m_undo_DBState_iter = m_redo_DBState_iter = m_vDBState.end();
}

// On open and every time we save, record the "initial" state
void PWScore::SetInitialValues()
{
  m_InitialDBName = m_hdr.m_DB_Name;              // for detecting header changes
  m_InitialDBDesc = m_hdr.m_DB_Description;       // for detecting header changes
  m_InitialDBPreferences = m_hdr.m_prefString;    // for detecting DB preference changes
  m_InitialEmptyGroups = m_vEmptyGroups;          // for WasEmptyGroupsChanged
  m_InitialMapPSWDPLC = m_MapPSWDPLC;             // for HavePasswordPolicyNamesChanged
  m_InitialMapDBFilters = m_MapDBFilters;         // for HaveDBFiltersChanged

  // NOTE: These two are not tested for "Save Immediately" but only when the DB is closed
  m_InitialDisplayStatus = m_hdr.m_displaystatus; // for HasGroupDisplayChanged
  m_InitialRUEList = m_RUEList;                   // for detecting header changes

  // Clear changed nodes
  m_vModifiedNodes.clear();
}

int PWScore::Execute(Command *pcmd)
{
  /*
    NOTES:
    1. *ALL* USER ACTIONS (e.g., simple such as delete one entry or
      complex such as import a file, merge a DB or drag&drop) MUST call this
      member function EXACTLY ONCE, since this invokes "Save Immediately"
      if the preference for this is set.
      IT IS THE RESPONSIBILTY OF THE UI TO ENSURE THAT A SINGLE USER ACTION GENERATES
      ONLY ONE CALL THIS ROUTINE. OTHERWISE MULTIPLE SAVES AND, POTENTIALLY, INTERMEDIATE
      BACKUPS MAY BE GENERATED.

    2. ALL COMMANDS UPDATE THE DATABASE (except UpdateGUICommand, which is
      always used in combination with a command that does) AND SO WILL GENERATE
      A "Save Immediately" SAVE AND CREATE AN INTERMEDIATE BACKUP if
      the save immediate preference is set.
   */

   // If we have undone some previous commands, then this new command must
   // go on the end of the currently executed commands in the 
   // command chain and so we must delete any old commands
   // that have been undone first
  if (m_redo_iter != m_vpcommands.end()) {
    std::vector<Command *>::iterator cmd_Iter;

    for (cmd_Iter = m_redo_iter; cmd_Iter != m_vpcommands.end(); cmd_Iter++) {
      delete (*cmd_Iter);
    }

    // Now remove old commands past this one from vector
    m_vpcommands.erase(m_redo_iter, m_vpcommands.end());
    // Now remove old DB change states past this one from vector
    m_vDBState.erase(m_redo_DBState_iter, m_vDBState.end());
  }

  // Put this command on the end of the command chain
  m_vpcommands.push_back(pcmd);

  // And reset iterator to the end for undo/redo
  m_undo_iter = m_redo_iter = m_vpcommands.end();

  // Execute it
  int rc = pcmd->Execute();

  // Save current before & after DB states
  // Note: commands should always change something but check
  DBStates cmdDBStates;
  cmdDBStates.before = m_DBCurrentState;
  cmdDBStates.after = pcmd->WasDBChanged() ? DIRTY : CLEAN;
  m_vDBState.push_back(cmdDBStates);

  // Set current state
  m_DBCurrentState = cmdDBStates.after;

  // And reset iterator to the end for undo/redo
  m_undo_DBState_iter = m_redo_DBState_iter = m_vDBState.end();

  // Set undo iterator to this one
  m_undo_iter--;
  m_undo_DBState_iter--;

  // If user has set Save Immediately, then Execute() changes the DB and it should be
  // saved (with or without an intermediate backup)
  NotifyDBModified();

  // Then tell the UI update the GUI
  NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR, CUUID::NullUUID());
  return rc;
}

void PWScore::Undo()
{
  // Undo last executed command
  ASSERT(m_undo_iter != m_vpcommands.end());

  // Reset next command to redo (i.e., the one we just about to undo)
  m_redo_iter = m_undo_iter;
  m_redo_DBState_iter = m_undo_DBState_iter;

  // Undo it
  (*m_undo_iter)->Undo();

  // Reset command & DBstate iterator so that we know next command to undo
  if (m_undo_iter == m_vpcommands.begin()) {
    m_undo_iter = m_vpcommands.end();
    m_undo_DBState_iter = m_vDBState.end();
  } else {
    m_undo_iter--;
    m_undo_DBState_iter--;
  }

  // Need to reset current DB state based on the next command's before state
  m_DBCurrentState = m_redo_DBState_iter->before;

  // If user has set Save Immediately, then Undo changes the DB and it should be
  // saved (with or without an intermediate backup)
  NotifyDBModified();

  // Then tell the UI update the GUI
  NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR, CUUID::NullUUID());
}

void PWScore::Redo()
{
  // Redo last undone command
  ASSERT(m_redo_iter != m_vpcommands.end());

  // Reset next command to undo (i.e. the one we just about to redo)
  m_undo_iter = m_redo_iter;
  m_undo_DBState_iter = m_redo_DBState_iter;

  // Redo it
  (*m_redo_iter)->Redo();

  // Need to reset current DB state based on the command's after state
  m_DBCurrentState = m_redo_DBState_iter->after;

  // Reset iterator so that we know next command to redo
  if (m_redo_iter != m_vpcommands.end()) {
    m_redo_iter++;
    m_redo_DBState_iter++;
  }
  
  // If user has set Save Immediately, then Redo changes the DB and it should be
  // saved (with or without an intermediate backup)
  NotifyDBModified();

  // Then tell the UI update the GUI
  NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR, CUUID::NullUUID());
}

Command * PWScore::GetRedoCommand()
{
  ASSERT(m_redo_iter != m_vpcommands.end());
  return *m_redo_iter;
}

Command * PWScore::GetUndoCommand()
{
  ASSERT(m_undo_iter != m_vpcommands.end());
  return *m_undo_iter;
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

  if (!filename.empty()) {
    status = PWSfile::CheckPasskey(filename, passkey, m_ReadFileVersion);
    if (status == PWSfile::WRONG_PASSWORD) {
      // See if passkey was encoded incorrectly
      pws_os::setenv("PWS_PK_CP_ACP", "1");
      status = PWSfile::CheckPasskey(filename, passkey, m_ReadFileVersion);
      pws_os::setenv("PWS_PK_CP_ACP", ""); // no unsetenv() in Windows...
    }
  } else { // can happen if tries to export b4 save
    size_t t_passkey_len = passkey.length();
    if (t_passkey_len != m_passkey_len) // trivial test
      return WRONG_PASSWORD;
    size_t BlockLength = ((m_passkey_len + 7) / 8) * 8;
    auto *t_passkey = new unsigned char[BlockLength];
    auto plaintext = passkey.c_str();
    EncryptPassword(reinterpret_cast<const unsigned char *>(plaintext), t_passkey_len, t_passkey);
    if (memcmp(t_passkey, m_passkey, BlockLength) == 0)
      status = PWSfile::SUCCESS;
    else
      status = PWSfile::WRONG_PASSWORD;
    delete[] t_passkey;
  }

  return status;
}

#define MRE_FS _T("\xbb")

static void TestAndFixNullUUID(CItemData &ci_temp,
                               std::vector<st_GroupTitleUser> &vGTU_INVALID_UUID,
                               st_ValidateResults &st_vr)
{
  /*
   * If, for some reason, we're reading in an invalid UUID,
   * we will change the UUID before adding it to the list.
   *
   * To date, we know that databases of format 0x0200 and 0x0300 have a UUID
   * problem if records were duplicated.  Databases of format 0x0100 did not
   * have the duplicate function and it has been fixed in databases in format
   * 0x0301 and so not an issue in V1 (0x0100) or V3.03 (0x0301) or later
   *
   * But a Null CUUID is invalid even if another application using core.lib
   * does it and they could have got the version wrong - so fix it anyway
   */
  if (ci_temp.GetUUID() == CUUID::NullUUID()) {
    vGTU_INVALID_UUID.push_back(st_GroupTitleUser(ci_temp.GetGroup(),
                                                  ci_temp.GetTitle(), ci_temp.GetUser()));
    st_vr.num_invalid_UUIDs++;
    ci_temp.CreateUUID(); // replace invalid UUID
    ci_temp.SetStatus(CItemData::ES_MODIFIED);  // Show modified
  } // UUID invalid
}

static void TestAndFixDupUUID(CItemData &ci_temp, const PWScore &core,
                              std::vector<st_GroupTitleUser> &vGTU_DUPLICATE_UUID,
                              st_ValidateResults &st_vr)
{
  /*
   * If, for some reason, we're reading in a UUID that we already have
   * we will change the UUID, rather than overwrite an entry.
   * This is to protect the user from possible bugs that break
   * the uniqueness requirement of UUIDs.
   */
  if (core.Find(ci_temp.GetUUID()) != core.GetEntryEndIter()) {
    vGTU_DUPLICATE_UUID.push_back(st_GroupTitleUser(ci_temp.GetGroup(),
                                                    ci_temp.GetTitle(), ci_temp.GetUser()));
    st_vr.num_duplicate_UUIDs++;
    ci_temp.CreateUUID(); // replace duplicated UUID
    ci_temp.SetStatus(CItemData::ES_MODIFIED);  // Show modified
  } // UUID duplicate
}

static void ProcessPasswordPolicy(CItemData &ci_temp, PWScore &core)
{
  if (ci_temp.IsPasswordPolicySet() && ci_temp.IsPolicyNameSet()) {
    // Error: can't have both - clear Password Policy Name
    ci_temp.ClearField(CItemData::POLICYNAME);
  }

  if (ci_temp.IsPolicyNameSet()) {
    if (!core.IncrementPasswordPolicy(ci_temp.GetPolicyName())) {
      // Map name not present in database - clear it!
      ci_temp.ClearField(CItemData::POLICYNAME);
    }
  }
}

void PWScore::ProcessReadEntry(CItemData &ci_temp,
                               std::vector<st_GroupTitleUser> &vGTU_INVALID_UUID,
                               std::vector<st_GroupTitleUser> &vGTU_DUPLICATE_UUID,
                               st_ValidateResults &st_vr)
{
  TestAndFixNullUUID(ci_temp, vGTU_INVALID_UUID, st_vr);
  TestAndFixDupUUID(ci_temp, *this, vGTU_DUPLICATE_UUID, st_vr);
  ProcessPasswordPolicy(ci_temp, *this);

  int32 iKBShortcut;
  ci_temp.GetKBShortcut(iKBShortcut);
  if (iKBShortcut != 0) {
    // Entry can't have same shortcut as the Application's HotKey
    if (m_iAppHotKey == iKBShortcut) {
      ci_temp.SetKBShortcut(0);
    } else { // non-zero shortcut != app hotkey
      if (!ValidateKBShortcut(iKBShortcut)) {
        m_KBShortcutMap.insert(KBShortcutMapPair(iKBShortcut, ci_temp.GetUUID()));
      } else {
        ci_temp.SetKBShortcut(0);
      }
    }
  } // non-zero shortcut

  // Possibly expired?
  time_t tttXTime;
  ci_temp.GetXTime(tttXTime);
  if (tttXTime != time_t(0)) {
    m_ExpireCandidates.push_back(ExpPWEntry(ci_temp));
  }

  // Finally, add it to the list!
  m_pwlist.insert(std::make_pair(ci_temp.GetUUID(), ci_temp));
}

static void ReportReadErrors(CReport *pRpt,
                             std::vector<st_GroupTitleUser> &vGTU_INVALID_UUID,
                             std::vector<st_GroupTitleUser> &vGTU_DUPLICATE_UUID)
{
  if (pRpt == nullptr || (vGTU_INVALID_UUID.empty() && vGTU_DUPLICATE_UUID.empty()))
    return;

  // Here iff we've something to report and somewhere to report it
  stringT cs_Error;
  // Write out error heading
  pRpt->WriteLine();
  LoadAString(cs_Error, IDSC_VALIDATE_ERRORS);
  pRpt->WriteLine(cs_Error);

  // Report invalid UUIDs
  if (!vGTU_INVALID_UUID.empty()) {
    pRpt->WriteLine();
    LoadAString(cs_Error, IDSC_VALIDATE_BADUUID);
    pRpt->WriteLine(cs_Error);
  }
  std::sort(vGTU_INVALID_UUID.begin(), vGTU_INVALID_UUID.end(), GTUCompareV1);
  for (auto iv1 = vGTU_INVALID_UUID.begin(); iv1 != vGTU_INVALID_UUID.end(); iv1++) {
    Format(cs_Error, IDSC_VALIDATE_ENTRY,
           iv1->group.c_str(), iv1->title.c_str(), iv1->user.c_str(), _T(""));
    pRpt->WriteLine(cs_Error);
  }

  // Report Duplicate UUIDs
  if (!vGTU_DUPLICATE_UUID.empty()) {
    pRpt->WriteLine();
    LoadAString(cs_Error, IDSC_VALIDATE_DUPUUID);
    pRpt->WriteLine(cs_Error);
  }
  std::sort(vGTU_DUPLICATE_UUID.begin(), vGTU_DUPLICATE_UUID.end(), GTUCompareV1);
  for (auto iv2 = vGTU_DUPLICATE_UUID.begin(); iv2 < vGTU_DUPLICATE_UUID.end(); iv2++) {
    Format(cs_Error, IDSC_VALIDATE_ENTRY,
           iv2->group.c_str(), iv2->title.c_str(), iv2->user.c_str(), _T(""));
    pRpt->WriteLine(cs_Error);
  }
}

int PWScore::ReadFile(const StringX &a_filename, const StringX &a_passkey,
                      const bool bValidate, const size_t iMAXCHARS,
                      CReport *pRpt)
{
  PWS_LOGIT_ARGS("bValidate=%ls; iMAXCHARS=%d; pRpt=%p",
                 bValidate ? L"true" : L"false", iMAXCHARS,
                 pRpt);

  int status;
  st_ValidateResults st_vr;
  std::vector<st_GroupTitleUser> vGTU_INVALID_UUID, vGTU_DUPLICATE_UUID;

  // Clear any old expired password entries
  m_ExpireCandidates.clear();

  // Clear any old entry keyboard shortcuts
  m_KBShortcutMap.clear();

  PWSfile *in = PWSfile::MakePWSfile(a_filename, a_passkey, m_ReadFileVersion,
                                     PWSfile::Read, status, m_pAsker, m_pReporter);

  if (status != PWSfile::SUCCESS) {
    delete in;
    return status;
  }

  status = in->Open(a_passkey);
  if (status == PWSfile::WRONG_PASSWORD) {
    // See if passkey was encoded incorrectly
    pws_os::setenv("PWS_PK_CP_ACP", "1");
    status = in->Open(a_passkey);
    pws_os::setenv("PWS_PK_CP_ACP", ""); // no unsetenv() in Windows...
  }

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
    return status;
  }

  if (m_ReadFileVersion == PWSfile::UNKNOWN_VERSION) {
    delete in;
    return UNKNOWN_VERSION;
  }

  m_hdr = in->GetHeader();

  m_RUEList = m_hdr.m_RUEList;

  if (!m_isAuxCore) { // aux. core does not modify db prefs in pref singleton
    // Get pref string and tree display status & who saved when
    // all possibly empty!
    PWSprefs *prefs = PWSprefs::GetInstance();
    prefs->Load(m_hdr.m_prefString);

    // Ensure obsolete DB preferences are removed when opening a DB saved
    // by an earlier version
    m_hdr.m_prefString = prefs->Store();
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
  } // !m_isAuxCore

  ClearDBData(); // Before overwriting old data, but after opening the file...

  SetPassKey(a_passkey); // so user won't be prompted for saves

  CItemData ci_temp;
  bool go = true;

  m_hashIters = in->GetNHashIters();
  if (in->GetDBFilters() != nullptr) m_MapDBFilters = *in->GetDBFilters();
  if (in->GetPasswordPolicies() != nullptr) m_MapPSWDPLC = *in->GetPasswordPolicies();
  if (in->GetEmptyGroups() != nullptr) m_vEmptyGroups = *in->GetEmptyGroups();

  // Set initial values
  SetInitialValues();

  // We keep this vector sorted for comparison - other apps may not
  std::sort(m_InitialEmptyGroups.begin(), m_InitialEmptyGroups.end());

  if (pRpt != nullptr) {
    std::wstring cs_title;
    LoadAString(cs_title, IDSC_RPTVALIDATE);
    pRpt->StartReport(cs_title.c_str(), m_currfile.c_str());
  }

  do {
    ci_temp.Clear(); // Rather than creating a new one each time.
    status = in->ReadRecord(ci_temp);
    switch (status) {
      case PWSfile::FAILURE:
      {
        // Show a useful(?) error message - better than
        // silently losing data (but not by much)
        // Best if title intact. What to do if not?
        if (m_pReporter != nullptr) {
          stringT cs_msg, cs_caption;
          LoadAString(cs_caption, IDSC_READ_ERROR);
          Format(cs_msg, IDSC_ENCODING_PROBLEM, ci_temp.GetTitle().c_str());
          cs_msg = cs_caption + _S(": ") + cs_caption;
          (*m_pReporter)(cs_msg);
        }
      }
      // deliberate fall-through
      case PWSfile::SUCCESS:
        ProcessReadEntry(ci_temp, vGTU_INVALID_UUID, vGTU_DUPLICATE_UUID, st_vr);
        break;
      case PWSfile::WRONG_RECORD: {
        // See if this is a V4 attachment:
        CItemAtt att;
        status = att.Read(in);
        if (status == PWSfile::SUCCESS) {
          m_attlist.insert(std::make_pair(att.GetUUID(), att));
        } else {
          // XXX report problem!
        }
      }
        break;
      case PWSfile::END_OF_FILE:
        go = false;
        break;
      default:
        break;
    } // switch
  } while (go);

  ParseDependants();

  m_nRecordsWithUnknownFields = in->GetNumRecordsWithUnknownFields();
  in->GetUnknownHeaderFields(m_UHFL);
  int closeStatus = in->Close(); // in V3 & later this checks integrity
  delete in;

  ReportReadErrors(pRpt, vGTU_INVALID_UUID, vGTU_DUPLICATE_UUID);

  // Validate rest of things in the database (excluding duplicate UUIDs fixed above
  // as needed for m_pwlist - map uses UUID as its key)
  bool bValidateRC = !vGTU_INVALID_UUID.empty() || !vGTU_DUPLICATE_UUID.empty();

  // Only do the rest if user hasn't explicitly disabled the checks
  // NOTE: When a "other" core is involved (Compare, Merge etc.), we NEVER validate
  // the "other" core.
  if (bValidate)
    bValidateRC = Validate(iMAXCHARS, pRpt, st_vr);

  if (pRpt != nullptr)
    pRpt->EndReport();

  // Setup file signature for checking file integrity upon backup.
  // Goal is to prevent overwriting a good backup with a corrupt file.
  if (a_filename == m_currfile) {
    delete m_pFileSig;
    m_pFileSig = new PWSFileSig(a_filename.c_str());
  }

  // Make return code negative if validation errors
  if (closeStatus == SUCCESS && bValidateRC)
    closeStatus = OK_WITH_VALIDATION_ERRORS;

  // OK DB open
  m_bIsOpen = true;

  return closeStatus;
}

static void ManageIncBackupFiles(const stringT &cs_filenamebase,
                                 size_t maxnumincbackups, stringT &cs_newname)
{
  /**
   * make sure we've no more than maxnumincbackups backup files,
   * and return the base name of the next backup file
   * (sans the suffix, which will be added by caller)
   *
   * The current solution breaks when maxnumincbackups >= 999.
   * Best solution is to delete by modification time,
   * but that requires a bit too much for the cost/benefit.
   * So for now we're "good enough" - limiting maxnumincbackups to <= 998
   */

  if (maxnumincbackups >= 999) {
    pws_os::Trace(_T("Maxnumincbackups: truncating maxnumincbackups to 998"));
    maxnumincbackups = 998;
  }

  using std::vector;

  stringT cs_filenamemask(cs_filenamebase);
  vector<stringT> files;
  vector<unsigned int> file_nums;

  cs_filenamemask += _T("_???.ibak");

  pws_os::FindFiles(cs_filenamemask, files);

  for (vector<stringT>::iterator iter = files.begin();
       iter != files.end(); iter++) {
    stringT ibak_number_str = iter->substr(iter->length() - 8, 3);
    if (ibak_number_str.find_first_not_of(_T("0123456789")) != stringT::npos)
      continue;
    istringstreamT is(ibak_number_str);
    unsigned int n;
    is >> n;
    file_nums.push_back(n);
  }

  if (file_nums.empty()) {
    cs_newname = cs_filenamebase + _T("_001");
    return;
  }

  sort(file_nums.begin(), file_nums.end());

  // nnn is the number of the file in the returned value: cs_filebasename_nnn
  size_t nnn = file_nums.back();
  nnn++;
  if (nnn > 999) {
    // as long as there's a _999 file, we set n starting from 001
    nnn = 1;
    size_t x = file_nums.size() - maxnumincbackups;
    while (file_nums[x++] == nnn && x < file_nums.size())
      nnn++;
    // Now we need to determine who to delete.
    size_t next = 999 - (maxnumincbackups - nnn);
    unsigned int m = 1;
    for (x = 0; x < file_nums.size(); x++)
      if (file_nums[x] < next)
        file_nums[x] = (unsigned long)(next <= 999 ? next++ : m++);
  }

  Format(cs_newname, L"%ls_%03d", cs_filenamebase.c_str(), nnn);

  int i = 0;
  size_t num_found = file_nums.size();
  stringT excess_file;
  while (num_found >= maxnumincbackups) {
    nnn = file_nums[i];
    Format(excess_file, L"%ls_%03d.ibak", cs_filenamebase.c_str(), nnn);
    i++;
    num_found--;
    if (!pws_os::DeleteAFile(excess_file)) {
      pws_os::Trace(L"DeleteFile(%ls) failed", excess_file.c_str());
      continue;
    }
  }
}

bool PWScore::BackupCurFile(unsigned int maxNumIncBackups, int backupSuffix,
                            const stringT &userBackupPrefix,
                            const stringT &userBackupDir, stringT &bu_fname)
{
  stringT cs_temp;
  const stringT path(m_currfile.c_str());
  stringT drv, dir, name, ext;

  // Check if the file we're about to backup is unchanged since
  // we opened it, to avoid overwriting a good file with a bad one
  if (m_pFileSig != nullptr) {
    PWSFileSig curSig(m_currfile.c_str());
    bool passed = (curSig == *m_pFileSig);
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
                                                               PWSUtil::TMC_EXPORT_IMPORT);
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
  time(&m_hdr.m_whenpwdlastchanged); // update master password changed timestamp
  WriteCurFile(); // Save immediately!
}

// functor object type for find_if:
struct FieldsMatch {
  bool operator()(const std::pair<CUUID, CItemData> &p) {
    const CItemData &item = p.second;
    return (m_group == item.GetGroup() &&
            m_title == item.GetTitle() &&
            m_user  == item.GetUser());
  }
  FieldsMatch(const StringX &a_group, const StringX &a_title,
              const StringX &a_user) :
  m_group(a_group), m_title(a_title), m_user(a_user) {}

private:
  FieldsMatch& operator=(const FieldsMatch&) = delete;
  const StringX &m_group;
  const StringX &m_title;
  const StringX &m_user;
};

// Finds stuff based on group, title & user fields only
ItemListIter PWScore::Find(const StringX &a_group,const StringX &a_title,
                           const StringX &a_user)
{
  FieldsMatch fields_match(a_group, a_title, a_user);

  return find_if(m_pwlist.begin(), m_pwlist.end(), fields_match);
}

struct TitleMatch {
  bool operator()(const std::pair<CUUID, CItemData> &p) {
    const CItemData &item = p.second;
    return (m_title == item.GetTitle());
  }

  TitleMatch(const StringX &a_title) :
    m_title(a_title) {}

private:
  TitleMatch& operator=(const TitleMatch&) = delete;
  const StringX &m_title;
};

ItemListIter PWScore::GetUniqueBase(const StringX &a_title, bool &bMultiple)
{
  auto retval(m_pwlist.end());
  int num(0);
  TitleMatch TitleMatch(a_title);

  auto found(m_pwlist.begin());
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
  bool operator()(const std::pair<CUUID, CItemData> &p) {
    const CItemData &item = p.second;
    return ((m_gt == item.GetGroup() && m_tu == item.GetTitle()) ||
            (m_gt == item.GetTitle() && m_tu == item.GetUser()));
  }

  GroupTitle_TitleUserMatch(const StringX &a_grouptitle,
                            const StringX &a_titleuser) :
                            m_gt(a_grouptitle),  m_tu(a_titleuser) {}

private:
  GroupTitle_TitleUserMatch& operator=(const GroupTitle_TitleUserMatch&) = delete;
  const StringX &m_gt;
  const StringX &m_tu;
};

ItemListIter PWScore::GetUniqueBase(const StringX &grouptitle,
                                    const StringX &titleuser, bool &bMultiple)
{
  auto retval(m_pwlist.end());
  int num(0);
  GroupTitle_TitleUserMatch GroupTitle_TitleUserMatch(grouptitle, titleuser);

  auto found(m_pwlist.begin());
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
  auto ulen = static_cast<unsigned int>(len);

  const unsigned int BS = TwoFish::BLOCKSIZE;

  if (!pws_os::mcryptUnprotect(m_session_key, sizeof(m_session_key))) {
    pws_os::Trace(_T("pws_os::mcryptUnprotect failed"));
  }
  TwoFish tf(m_session_key, sizeof(m_session_key));
  if (!pws_os::mcryptProtect(m_session_key, sizeof(m_session_key))) {
    pws_os::Trace(_T("pws_os::mcryptProtect failed"));
  }
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
    tf.Encrypt(curblock, curblock);
    memcpy(ciphertext + x, curblock, BS);
  }
  trashMemory(curblock, sizeof(curblock));
}

void PWScore::SetPassKey(const StringX &new_passkey)
{
  // Only used when opening files and for new files
  const unsigned int BS = TwoFish::BLOCKSIZE;
  // if changing, clear old
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + (BS -1)) / BS) * BS);
    delete[] m_passkey;
  }

  m_passkey_len = new_passkey.length() * sizeof(TCHAR);

  size_t BlockLength = ((m_passkey_len + (BS - 1)) / BS) * BS;
  m_passkey = new unsigned char[BlockLength];
  auto plaintext = new_passkey.c_str();
  EncryptPassword(reinterpret_cast<const unsigned char *>(plaintext), m_passkey_len, m_passkey);
}

StringX PWScore::GetPassKey() const
{
  StringX retval(_T(""));
  if (m_passkey_len > 0) {
    const unsigned int BS = TwoFish::BLOCKSIZE;
    size_t BlockLength = ((m_passkey_len + (BS - 1)) / BS) * BS;
    if (!pws_os::mcryptUnprotect(m_session_key, sizeof(m_session_key))) {
      pws_os::Trace(_T("pws_os::mcryptUnprotect failed"));
    }
    TwoFish tf(m_session_key, sizeof(m_session_key));
    if (!pws_os::mcryptProtect(m_session_key, sizeof(m_session_key))) {
      pws_os::Trace(_T("pws_os::mcryptProtect failed"));
    }
    unsigned char curblock[BS];
    for (unsigned int x = 0; x < BlockLength; x += BS) {
      unsigned int i;
      for (i = 0; i < BS; i++) {
        curblock[i] = m_passkey[x + i];
      }

      tf.Decrypt(curblock, curblock);
      for (i = 0; i < BS; i += sizeof(TCHAR)) {
        if (x + i < m_passkey_len) {
          retval += *(reinterpret_cast<TCHAR*>(curblock + i));
        }
      }
    }
    trashMemory(curblock, sizeof(curblock));
  }
  return retval;
}

void PWScore::SetDisplayStatus(const std::vector<bool> &s)
{
  PWS_LOGIT;

  // DON'T set m_bDBChanged!
  // Application should use HasGroupDisplayChanged()
  // to determine if state has changed.
  // This allows app to silently save without nagging user
  m_hdr.m_displaystatus = s;
}

const std::vector<bool> &PWScore::GetDisplayStatus() const
{
  PWS_LOGIT;

  return m_hdr.m_displaystatus;
}

bool PWScore::HasGroupDisplayChanged() const
{
  // m_InitialDisplayStatus is set while reading and saving a file.
  // m_hdr.m_displaystatus may be changed via PWScore::SetDisplayStatus
  // Only for V3 and later
  return m_ReadFileVersion >= PWSfile::V30 &&
         (m_hdr.m_displaystatus != m_InitialDisplayStatus);
}

bool PWScore::HasRUEListChanged() const
{
  // m_InitialRUEList is set while reading and saving a file.
  // m_hdr.m_RUEList may be changed via PWScore::SetRUEList
  // Only for V3 and later
  return m_ReadFileVersion >= PWSfile::V30 &&
         (m_hdr.m_RUEList != m_InitialRUEList);
}

static void collectGroups(const StringX &sxg, std::set<stringT> &setGroups)
{
  StringX sxGroup(sxg);
  if (sxGroup.find(_T('.')) == StringX::npos) {
    // We have exactly one group
    setGroups.insert(sxGroup.c_str());
  } else {
    // We have > 1 group. Tokenize:
    StringX sxPath, token;
    std::vector<StringX> vTokens;
    size_t pos = 0;
    while ((pos = sxGroup.find(_T('.'))) != StringX::npos) {
      token = sxGroup.substr(0, pos);
      vTokens.push_back(token);
      sxGroup.erase(0, pos + 1);
    }
    vTokens.push_back(sxGroup); // rightmost subgroup, e.g., "b" in "a.b"
    ASSERT(vTokens.size() >= 2);
    sxPath = vTokens[0];

    setGroups.insert(sxPath.c_str());
    for (size_t i = 1; i < vTokens.size(); i++) {
      sxPath += StringX(_T(".")) + vTokens[i];
      setGroups.insert(sxPath.c_str());
    }
  } // > 1 group
}

// GetAllGroups - returns an array of all unique group prefix names
// e.g., "A", "A.B", "A.B.C"
void PWScore::GetAllGroups(std::vector<stringT> &vAllGroups, const bool bIncludeEmptyGroups) const
{
  // use the fact that set eliminates dups for us
  std::set<stringT> setGroups;

  // Start with groups that have elements
  for (auto iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    const CItemData &ci = iter->second;
    if (ci.IsGroupSet()) {
      // We have at least one group
      collectGroups(ci.GetGroup(), setGroups);
    } // IsGroupSet()
  } // for m_pwlist

  if (bIncludeEmptyGroups) {
    // Now add Empty groups in the same manner
    for (auto iter2 = m_vEmptyGroups.begin(); iter2 != m_vEmptyGroups.end(); iter2++) {
      ASSERT(!iter2->empty());
      collectGroups(*iter2, setGroups);
    }
  }

  vAllGroups.clear();
  // copy unique results from set to caller's vector
  copy(setGroups.begin(), setGroups.end(), back_inserter(vAllGroups));
}

// GetPolicyNames - returns an array of all password policy names
// They are in sort order as a map is always sorted by its key
void PWScore::GetPolicyNames(std::vector<stringT> &vNames) const
{
  vNames.clear();

  PSWDPolicyMapCIter citer;

  for (citer = m_MapPSWDPLC.begin(); citer != m_MapPSWDPLC.end(); citer++ ) {
    vNames.push_back(citer->first.c_str());
  }
}

bool PWScore::GetPolicyFromName(const StringX &sxPolicyName, PWPolicy &st_pp) const
{
  // - An empty policy name is never valid.
  ASSERT(!sxPolicyName.empty());

  // - The default policy is not stored in the map (but read from preferences)
  StringX sxDefPolicyStr;
  LoadAString(sxDefPolicyStr, IDSC_DEFAULT_POLICY);

  if (sxPolicyName == sxDefPolicyStr) {
    st_pp = PWSprefs::GetInstance()->GetDefaultPolicy();
    return true;
  } else {
    auto iter = m_MapPSWDPLC.find(sxPolicyName);
    if (iter != m_MapPSWDPLC.end()) {
      st_pp = iter->second;
      return true;
    } else {
      st_pp.Empty();
      return false;
    }
  }
}

class PolicyNameMatch
{
public:
  PolicyNameMatch(StringX &policyname) : m_policyname(policyname) {}

  bool operator()(const std::pair<StringX, StringX> &pr)
  {
    return (m_policyname == pr.second);
  }

private:
  StringX m_policyname;
};

void PWScore::MakePolicyUnique(std::map<StringX, StringX> &mapRenamedPolicies,
                               StringX &sxPolicyName, const StringX &sxDateTime,
                               const UINT IDS_MESSAGE)
{
  // 'mapRenamedPolicies' contains those policies already renamed. It will be
  // updated with any new name generated.
  // The map key is old name and the value is new name
  // 'sxPolicyName' is the current name and will be returned as the new name

  std::map<StringX, StringX>::iterator iter;
  // Have we done it already?
  iter = mapRenamedPolicies.find(sxPolicyName);
  if (iter != mapRenamedPolicies.end()) {
    // Yes - give them the new name
    sxPolicyName = iter->second;
    return;
  }

  StringX sxNewPolicyName;
  Format(sxNewPolicyName, IDS_MESSAGE, sxPolicyName.c_str(), sxDateTime.c_str());

  // Verify new policy name not already in this database
  if (m_MapPSWDPLC.find(sxNewPolicyName) != m_MapPSWDPLC.end())
    ASSERT(0);  // Already there - how can this be????

  // Now try to see if we have added this
  if (std::find_if(mapRenamedPolicies.begin(), mapRenamedPolicies.end(),
                   PolicyNameMatch(sxNewPolicyName)) == mapRenamedPolicies.end()) {
    // No. OK we have got a new unique policy name - save it
    mapRenamedPolicies[sxPolicyName] = sxNewPolicyName;
  }

  sxPolicyName = sxNewPolicyName;
  return;
}

// functor object type for for_each:
// Updates vector with entries using named password policy
struct AddEntry {
  AddEntry(const StringX sxPolicyName, std::vector<st_GroupTitleUser> &ventries)
    : m_sxPolicyName(sxPolicyName), m_pventries(&ventries) {}

  void operator()(std::pair<CUUID const, CItemData> &p)
  {
    if (p.second.GetPolicyName() == m_sxPolicyName) {
      st_GroupTitleUser st;
      st.group = p.second.GetGroup();
      st.title = p.second.GetTitle();
      st.user = p.second.GetUser();
      m_pventries->push_back(st);
      return;
    }
  }

private:
  AddEntry& operator=(const AddEntry&); // Do not implement
  StringX m_sxPolicyName;
  std::vector<st_GroupTitleUser> *m_pventries;
};

bool PWScore::GetEntriesUsingNamedPasswordPolicy(const StringX sxPolicyName,
              std::vector<st_GroupTitleUser> &ventries)
{
  AddEntry add_entry(sxPolicyName, ventries);
  std::for_each(m_pwlist.begin(), m_pwlist.end(), add_entry);

  // Sort them before displayed in the dialog later
  std::sort(ventries.begin(), ventries.end(), GTUCompareV1);

  return !ventries.empty();
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

  st_GroupTitleUser2(const st_GroupTitleUser2 &other)
    : group(other.group), title(other.title), user(other.user), newtitle(other.newtitle) {}

  st_GroupTitleUser2 &operator=(const st_GroupTitleUser2 &that) {
    if (this != &that) {
      group = that.group; title = that.title; user = that.user;
      newtitle = that.newtitle;
    }
    return *this;
  }
};

// For Validate only
struct st_AttTitle_Filename {
  StringX title;
  StringX filename;

  st_AttTitle_Filename() {}

  st_AttTitle_Filename(const StringX &t, const StringX &fn)
    : title(t), filename(fn) {}

  st_AttTitle_Filename(const st_AttTitle_Filename &other)
    : title(other.title), filename(other.filename) {}

  st_AttTitle_Filename &operator=(const st_AttTitle_Filename &that) {
    if (this != &that) {
      title = that.title; filename = that.filename;
    }
    return *this;
  }
};

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

void PWScore::ParseDependants()
{
  UUIDVector Possible_Aliases, Possible_Shortcuts;

  for (ItemListIter iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    const CItemData &ci = iter->second;
    // Get all possible Aliases/Shortcuts for future checking if base entries exist
    if (ci.IsAlias()) {
      Possible_Aliases.push_back(ci.GetUUID());
    } else if (ci.IsShortcut()) {
      Possible_Shortcuts.push_back(ci.GetUUID());
    }
    // Set refcount on attachments
    if (ci.HasAttRef()) {
      auto attIter = m_attlist.find(ci.GetAttUUID());
      if (attIter != m_attlist.end())
        attIter->second.IncRefcount();
      else
        pws_os::Trace(_T("dangling ATTREF")); // will be caught in validate
    }
  } // iter over m_pwlist

  if (!Possible_Aliases.empty()) {
    DoAddDependentEntries(Possible_Aliases, nullptr, CItemData::ET_ALIAS, CItemData::UUID);
  }

  if (!Possible_Shortcuts.empty()) {
    DoAddDependentEntries(Possible_Shortcuts, nullptr, CItemData::ET_SHORTCUT, CItemData::UUID);
  }
}

bool PWScore::Validate(const size_t iMAXCHARS, CReport *pRpt, st_ValidateResults &st_vr)
{
  /*
     1. Check PWH is valid
     2. Check that the 2 mandatory fields are present (Title & Password)
     3. Check group/title/user must be unique.
     4. Check that no text field has more than iMAXCHARS, that can displayed
        in the GUI's text control.
     5. Validate Empty Groups
     6. For attachments (V4):
     6.1 Check that each ATTREF in a data entry has a corresponding ItemAtt
     6.2 Check that each ItemAtt has a corresponding "owner" ItemData

     Note:
     m_pwlist is implemented as a map keyed on UUIDs, each entry is
     guaranteed to have a unique uuid. The uniqueness invariant
     should be enforced elsewhere.
     (ReadFile during Open and Import have already ensured UUIDs are unique
     and valid)
  */

  PWS_LOGIT_ARGS("iMAXCHARS=%d; pRpt=%p", iMAXCHARS, pRpt);

  int n = -1;
  size_t uimaxsize(0);

  stringT cs_Error;
  pws_os::Trace(_T("Start validation\n"));

  st_GroupTitleUser st_gtu;
  GTUSet setGTU;
  GTUSetPair pr_gtu;
  std::vector<st_GroupTitleUser> vGTU_UUID, vGTU_EmptyPassword, vGTU_PWH, vGTU_TEXT,
                                 vGTU_ALIASES, vGTU_SHORTCUTS;
  std::vector<st_GroupTitleUser2> vGTU_NONUNIQUE, vGTU_EmptyTitle;
  std::vector<st_GroupTitleUser> vGTU_MissingAtt;
  std::vector<st_AttTitle_Filename> vOrphanAtt;
  std::set<CUUID> sAtts;

  ItemListIter iter;

  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    CItemData &ci = iter->second;
    CItemData fixedItem(ci);
    bool bFixed(false);

    n++;

    // Fix GTU uniqueness - can't do this in a CItemData member function as it causes
    // circular includes:
    //  "ItemData.h" would need to include "coredefs.h", which needs to include "ItemData.h"!
    StringX sxgroup(ci.GetGroup()), sxtitle(ci.GetTitle()), sxuser(ci.GetUser());
    st_gtu.group = sxgroup;
    st_gtu.title = sxtitle;
    st_gtu.user = sxuser;

    if (sxtitle.empty()) {
      // This field is mandatory!
      // Change it and insert into a std::set which guarantees uniqueness
      int i = 0;
      StringX sxnewtitle(sxtitle);
      do {
        i++;
        Format(sxnewtitle, IDSC_MISSINGTITLE, i);
        st_gtu.title = sxnewtitle;
        pr_gtu =  setGTU.insert(st_gtu);
      } while (!pr_gtu.second);

      fixedItem.SetTitle(sxnewtitle);

      bFixed = true;
      vGTU_EmptyTitle.push_back(st_GroupTitleUser2(sxgroup, sxtitle, sxuser, sxnewtitle));
      st_vr.num_empty_titles++;
      sxtitle = sxnewtitle;
    } else {
      // Title was not empty
      // Insert into a std::set which guarantees uniqueness
      pr_gtu = setGTU.insert(st_gtu);
      if (!pr_gtu.second) {
        // Already have this group/title/user entry
        int i = 0;
        StringX s_copy, sxnewtitle(sxtitle);
        do {
          i++;
          Format(s_copy, IDSC_DUPLICATENUMBER, i);
          sxnewtitle = sxtitle + s_copy;
          st_gtu.title = sxnewtitle;
          pr_gtu =  setGTU.insert(st_gtu);
        } while (!pr_gtu.second);

        fixedItem.SetTitle(sxnewtitle);

        bFixed = true;
        vGTU_NONUNIQUE.push_back(st_GroupTitleUser2(sxgroup, sxtitle, sxuser, sxnewtitle));
        st_vr.num_duplicate_GTU_fixed++;
        sxtitle = sxnewtitle;
      }
    }

    // Test if Password is present as it is mandatory! was fixed
    if (ci.GetPassword().empty()) {
      StringX sxMissingPassword;
      LoadAString(sxMissingPassword, IDSC_MISSINGPASSWORD);
      fixedItem.SetPassword(sxMissingPassword);

      bFixed = true;
      vGTU_EmptyPassword.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      st_vr.num_empty_passwords++;
    }

    // Test if Password History was fixed
    if (!fixedItem.ValidatePWHistory()) {
      bFixed = true;
      vGTU_PWH.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      st_vr.num_PWH_fixed++;
    }

    // Note excessively sized text fields
    if (iMAXCHARS > 0) {
      bool bEntryHasBigField(false);
      for (auto uc = static_cast<unsigned char>(CItem::GROUP);
           uc < static_cast<unsigned char>(CItem::LAST_DATA); uc++) {
        if (CItemData::IsTextField(uc)) {
          StringX sxvalue = ci.GetFieldValue(static_cast<CItemData::FieldType>(uc));
          if (sxvalue.length() > iMAXCHARS && (!m_bIsReadOnly && !ci.IsProtected())) {
            bEntryHasBigField = true;
            //  We don't truncate the field, but if we did, then the the code would be:
            //  fixedItem.SetFieldValue((CItemData::FieldType)uc, sxvalue.substr(0, iMAXCHARS))
            break;
          }
        }
      }

      if (bEntryHasBigField) {
        uimaxsize = std::max(uimaxsize, ci.GetSize());
        vGTU_TEXT.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
        st_vr.num_excessivetxt_found++;
      }
    }

    // Attachment Reference check (6.1)
    if (ci.HasAttRef()) {
      sAtts.insert(ci.GetAttUUID());
      if (!HasAtt(ci.GetAttUUID())) {
        vGTU_MissingAtt.push_back(st_GroupTitleUser(ci.GetGroup(),
                                                    ci.GetTitle(),
                                                    ci.GetUser()));
        st_vr.num_missing_att++;
        // Fix the problem:
        fixedItem.ClearAttUUID();
        bFixed = true;
      }
    }

    // Empty group can't have entries!
    // This removes the empty group if it is an exact match to this entry's group
    std::vector<StringX>::iterator itEG;
    itEG = std::find(m_vEmptyGroups.begin(), m_vEmptyGroups.end(), sxgroup);
    if (itEG != m_vEmptyGroups.end()) {
      m_vEmptyGroups.erase(itEG);
    }

    // This remove the empty group if it contains this entry in one of its subgroups
    // Need to use reverse iterator so that can erase elements and still
    // iterate the vector but erase only takes a normal iterator!
    auto ritEG = m_vEmptyGroups.rbegin();
    while (ritEG != m_vEmptyGroups.rend()) {
      StringX sxEGDot = *ritEG + L".";
      ritEG++;
      if (sxgroup.length() > sxEGDot.length() &&
          _tcsncmp(sxEGDot.c_str(), sxgroup.c_str(), sxEGDot.length()) == 0) {
        ritEG = std::vector<StringX>::reverse_iterator(m_vEmptyGroups.erase(ritEG.base()));
      }
    }

    if (bFixed) {
      // Mark as modified
      fixedItem.SetStatus(CItemData::ES_MODIFIED);
      // We assume that this is run during file read. If not, then we
      // need to run using the Command mechanism for Undo/Redo.
      m_pwlist[fixedItem.GetUUID()] = fixedItem;
    }
  } // iteration over m_pwlist

  // Validate Empty Groups don't have empty sub-groups
  if (!m_vEmptyGroups.empty()) {
    std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());
    std::vector<size_t> viDelete;
    for (size_t ieg = 0; ieg < m_vEmptyGroups.size() - 1; ieg++) {
      StringX sxEG = m_vEmptyGroups[ieg] + L".";
      if (sxEG == m_vEmptyGroups[ieg + 1].substr(0, sxEG.length())) {
        // Can't be empty as has empty sub-group. Save to delete later
        viDelete.push_back(ieg);
      }
    }

    if (!viDelete.empty()) {
      // Remove non-empty groups
      std::vector<size_t>::reverse_iterator rit;
      for (rit = viDelete.rbegin(); rit != viDelete.rend(); rit++) {
        m_vEmptyGroups.erase(m_vEmptyGroups.begin() + *rit);
      }
    }
  }

  // Check for orphan attachments (6.2)
  for (auto att_iter = m_attlist.begin(); att_iter != m_attlist.end(); att_iter++) {
    if (sAtts.find(att_iter->first) == sAtts.end()) {
      st_AttTitle_Filename stATFN;
      stATFN.title = att_iter->second.GetTitle();
      stATFN.filename = att_iter->second.GetFileName();
      vOrphanAtt.push_back(stATFN);
      st_vr.num_orphan_att++;
      // NOT removing attachment for now. Add support for exporting orphans later.
    }
  }

#if 0 // XXX We've separated alias/shortcut processing from Validate - reconsider this!
  // See if we have any entries with passwords that imply they are an alias
  // but there is no equivalent base entry
  for (size_t ipa = 0; ipa < Possible_Aliases.size(); ipa++) {
    if (/* no base entry for ipa exists in m_pwlist */) {
      ItemListIter iter = m_pwlist.find(Possible_Aliases[ipa]);
      if (iter != m_pwlist.end()) {
        StringX sxgroup = iter->second.GetGroup();
        StringX sxtitle = iter->second.GetTitle();
        StringX sxuser = iter->second.GetUser();
        vGTU_ALIASES.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      }
      st_vr.num_alias_warnings++;
    }
  }

  // See if we have any entries with passwords that imply they are a shortcut
  // but there is no equivalent base entry
  for (size_t ips = 0; ips < Possible_Shortcuts.size(); ips++) {
    if (/* no base entry for ips exists in m_pwlist */) {
      ItemListIter iter = m_pwlist.find(Possible_Shortcuts[ips]);
      if (iter != m_pwlist.end()) {
        StringX sxgroup = iter->second.GetGroup();
        StringX sxtitle = iter->second.GetTitle();
        StringX sxuser = iter->second.GetUser();
        vGTU_SHORTCUTS.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      }
      st_vr.num_shortcuts_warnings++;
    }
  }
#endif

  if (st_vr.TotalIssues() != 0 && pRpt != nullptr) {
    // Only report problems if a. There are some and b. We have a report file
    if ((st_vr.num_invalid_UUIDs == 0 && st_vr.num_duplicate_UUIDs == 0)) {
      // As both zero, we didn't put error header in report - so do it now
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_ERRORS);
      pRpt->WriteLine(cs_Error);
    }

    if (!vGTU_EmptyTitle.empty()) {
      std::sort(vGTU_EmptyTitle.begin(), vGTU_EmptyTitle.end(), GTUCompareV2);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_EMPTYTITLE);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_EmptyTitle.size(); iv++) {
        st_GroupTitleUser2 &gtu2 = vGTU_EmptyTitle[iv];
        stringT cs_newtitle;
        Format(cs_newtitle, IDSC_VALIDATE_ENTRY2, gtu2.newtitle.c_str());
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu2.group.c_str(), gtu2.title.c_str(), gtu2.user.c_str(), cs_newtitle.c_str());
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_EmptyPassword.empty()) {
      StringX sxMissingPassword;
      LoadAString(sxMissingPassword, IDSC_MISSINGPASSWORD);
      std::sort(vGTU_EmptyPassword.begin(), vGTU_EmptyPassword.end(), GTUCompareV1);
      pRpt->WriteLine();
      Format(cs_Error, IDSC_VALIDATE_EMPTYPSWD, sxMissingPassword.c_str());
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_EmptyPassword.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_EmptyPassword[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_NONUNIQUE.empty()) {
      std::sort(vGTU_NONUNIQUE.begin(), vGTU_NONUNIQUE.end(), GTUCompareV2);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_DUPLICATES);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_NONUNIQUE.size(); iv++) {
        st_GroupTitleUser2 &gtu2 = vGTU_NONUNIQUE[iv];
        stringT cs_newtitle;
        Format(cs_newtitle, IDSC_VALIDATE_ENTRY2, gtu2.newtitle.c_str());
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu2.group.c_str(), gtu2.title.c_str(), gtu2.user.c_str(), cs_newtitle.c_str());
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_UUID.empty()) {
      std::sort(vGTU_UUID.begin(), vGTU_UUID.end(), GTUCompareV1);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_BADUUID);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_UUID.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_UUID[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_PWH.empty()) {
      std::sort(vGTU_PWH.begin(), vGTU_PWH.end(), GTUCompareV1);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_PWH);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_PWH.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_PWH[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if ((!vGTU_ALIASES.empty() || !vGTU_SHORTCUTS.empty() || !vGTU_TEXT.empty()) &&
        pRpt != nullptr) {
      // We have warnings
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_WARNINGS);
      pRpt->WriteLine(cs_Error);
    }

    if (!vGTU_ALIASES.empty()) {
      std::sort(vGTU_ALIASES.begin(), vGTU_ALIASES.end(), GTUCompareV1);
      pRpt->WriteLine();
      stringT sxAlias;
      LoadAString(sxAlias, IDSC_FALIAS);
      Format(cs_Error, IDSC_VALIDATE_DEPS, sxAlias.c_str());
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_ALIASES.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_ALIASES[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_SHORTCUTS.empty()) {
      std::sort(vGTU_SHORTCUTS.begin(), vGTU_SHORTCUTS.end(), GTUCompareV1);
      pRpt->WriteLine();
      stringT sxShortcut;
      LoadAString(sxShortcut, IDSC_FSHORTCUT);
      Format(cs_Error, IDSC_VALIDATE_DEPS,  sxShortcut.c_str());
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_SHORTCUTS.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_SHORTCUTS[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_TEXT.empty()) {
      std::sort(vGTU_TEXT.begin(), vGTU_TEXT.end(), GTUCompareV1);
      pRpt->WriteLine();
      int units(0);
      uimaxsize >>= 10;    // make bytes -> KB
      if (uimaxsize > 999) {
        uimaxsize >>= 10;  // make KB -> MB
        units++;
      }
      Format(cs_Error, IDSC_VALIDATE_TEXT, iMAXCHARS, uimaxsize,
             units == 0 ? L"KB" : L"MB");
      pRpt->WriteLine(cs_Error);

      for (size_t iv = 0; iv < vGTU_TEXT.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_TEXT[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    // Attachment-related issues
    if (st_vr.num_missing_att > 0) {
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_MISSING_ATT);
      pRpt->WriteLine(cs_Error);
      for_each(vGTU_MissingAtt.begin(), vGTU_MissingAtt.end(), [&](const st_GroupTitleUser &gtu) {
          Format(cs_Error, IDSC_VALIDATE_ENTRY,
                 gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
          pRpt->WriteLine(cs_Error);
      } );
    }

    if (st_vr.num_orphan_att > 0) {
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_ORPHAN_ATT);
      pRpt->WriteLine(cs_Error);
      stringT cs_NotSet;
      LoadAString(cs_NotSet, IDCS_VALIDATE_NOTSET);
      for_each(vOrphanAtt.begin(), vOrphanAtt.end(), [&](const st_AttTitle_Filename &stATFN) {
        stringT sTitle = stATFN.title.empty() ? cs_NotSet : stATFN.title.c_str();
        Format(cs_Error, IDSC_VALIDATE_ATTACHMENT, sTitle.c_str(), stATFN.filename.c_str());
        pRpt->WriteLine(cs_Error);
      } );
    }
  } // End of issues report handling

  pws_os::Trace(_T("End validation. %d entries processed\n"), n + 1);

  m_bUniqueGTUValidated = true;
  if (st_vr.TotalIssues() > 0) {
    m_DBCurrentState = DIRTY;
    return true;
  } else {
    return false;
  }
}

bool PWScore::ValidateKBShortcut(int32 &iKBShortcut)
{
  // Verify Entry Keyboard shortcut is valid
  // Note: to support cross-platforms, can't use "Virtual Key Codes"
  // as they differ between Windows, Linux & Mac.
  // However, the alphanumeric ASCII values are common
  // and so entry keyboard shortcuts are restricted to these values.

  if (iKBShortcut != 0) {
    static const TCHAR *tcValidKeys =
            _T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    static const WORD wValidModifiers = 0xffff ^ (
                            PWS_HOTKEYF_ALT   | PWS_HOTKEYF_CONTROL |
                            PWS_HOTKEYF_SHIFT | PWS_HOTKEYF_EXT |
                            PWS_HOTKEYF_META  | PWS_HOTKEYF_WIN |
                            PWS_HOTKEYF_CMD);

    WORD wVirtualKeyCode = iKBShortcut & 0xff;
    WORD wPWSModifiers = iKBShortcut >> 16;

    // If there are too many bits in the Modifiers or
    // Not a valid ASCII character (0-9, A-Z) - remove
    if ((wValidModifiers & wPWSModifiers) ||
          _tcschr(tcValidKeys, wVirtualKeyCode) == nullptr) {
      // Remove Entry Keyboard Shortcut
      iKBShortcut = 0;
      return true;  // Changed
    }
  }
  return false;  // Unchanged
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

bool PWScore::InitialiseGTU(GTUSet &setGTU, const StringX &sxPolicyName)
{
  setGTU.clear();

  if (sxPolicyName.empty())
    return false;

  // Populate the set of all group/title/user entries
  GTUSetPair pr_gtu;
  ItemListConstIter citer;

  for (citer = m_pwlist.begin(); citer != m_pwlist.end(); citer++) {
    const CItemData &ci = citer->second;
    if (ci.GetPolicyName() == sxPolicyName) {
      pr_gtu = setGTU.insert(st_GroupTitleUser(ci.GetGroup(), ci.GetTitle(), ci.GetUser()));
      if (!pr_gtu.second) {
        // Could happen if merging or synching a bad database!
        setGTU.clear();
        return false;
      }
    }
  }
  return true;
}

bool PWScore::InitialiseUUID(UUIDSet &setUUID)
{
  // Populate the set of all UUID entries
  UUIDSetPair pr_uuid;
  ItemListConstIter citer;

  setUUID.clear();
  for (citer = m_pwlist.begin(); citer != m_pwlist.end(); citer++) {
    pr_uuid = setUUID.insert(citer->second.GetUUID());
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

void PWScore::DoAddDependentEntry(const CUUID &base_uuid,
                                  const CUUID &entry_uuid,
                                  const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmmap = &m_base2shortcuts_mmap;
  } else {
    ASSERT(0);
    return;
  }

  auto biter = m_pwlist.find(base_uuid);
  auto diter = m_pwlist.find(entry_uuid);
  ASSERT(biter != m_pwlist.end());
  ASSERT(diter != m_pwlist.end());

  diter->second.SetBaseUUID(base_uuid);

  bool baseWasNormal = biter->second.IsNormal();
  if (type == CItemData::ET_ALIAS) {
    // Mark base entry as a base entry - must be a normal entry or already an alias base
    ASSERT(biter->second.IsNormal() || biter->second.IsAliasBase());
    biter->second.SetAliasBase();
    if (baseWasNormal) {
      // Allow fail as new entry might not yet be in the GUI
      GUIRefreshEntry(biter->second, true);
    }
  } else if (type == CItemData::ET_SHORTCUT) {
    // Mark base entry as a base entry - must be a normal entry or already a shortcut base
    ASSERT(biter->second.IsNormal() || biter->second.IsShortcutBase());
    biter->second.SetShortcutBase();
    if (baseWasNormal) {
      // Allow fail as new entry might not yet be in the GUI
      GUIRefreshEntry(biter->second, true);
    }
  }

  // Add to the base->type multimap
  pmmap->insert(ItemMMap_Pair(base_uuid, entry_uuid));
}

void PWScore::DoRemoveDependentEntry(const CUUID &base_uuid,
                                     const CUUID &entry_uuid,
                                     const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmmap = &m_base2shortcuts_mmap;
  } else {
    ASSERT(0);
    return;
  }

  // Remove from base -> entry multimap
  ItemMMapIter mmiter;
  ItemMMapIter mmlastElement;

  mmiter = pmmap->find(base_uuid);
  if (mmiter == pmmap->end())
    return;

  mmlastElement = pmmap->upper_bound(base_uuid);
  CUUID mmiter_uuid;

  for ( ; mmiter != mmlastElement; mmiter++) {
    mmiter_uuid = mmiter->second;
    if (entry_uuid == mmiter_uuid) {
      pmmap->erase(mmiter);
      break;
    }
  }

  // Reset base entry to normal if it has no more aliases/shortcuts
  if (pmmap->find(base_uuid) == pmmap->end()) {
    auto iter = m_pwlist.find(base_uuid);
    if (iter != m_pwlist.end()) {
      iter->second.SetNormal();

      // If base was being deleted, it might have been removed from the GUI
      // before we get here dealing with its last dependent
      GUIRefreshEntry(iter->second, true);
    }
  }
}

void PWScore::DoRemoveAllDependentEntries(const CUUID &base_uuid,
                                          const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmmap = &m_base2shortcuts_mmap;
  } else {
    ASSERT(0);
    return;
  }

  // Remove from entry -> base map for each entry
  ItemMMapIter itr;
  ItemMMapIter lastElement;

  itr = pmmap->find(base_uuid);
  if (itr == pmmap->end())
    return;

  lastElement = pmmap->upper_bound(base_uuid);

  // Remove from base -> entry multimap
  pmmap->erase(base_uuid);

  // Reset base entry to normal
  auto iter = m_pwlist.find(base_uuid);
  if (iter != m_pwlist.end())
    iter->second.SetNormal();
}

bool PWScore::DoMoveDependentEntries(const CUUID &from_baseuuid,
                                     const CUUID &to_baseuuid,
                                     const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmmap = &m_base2shortcuts_mmap;
  } else {
    ASSERT(0);
    return false;
  }

  ItemMMapIter from_itr;
  ItemMMapIter lastfromElement;

  from_itr = pmmap->find(from_baseuuid);
  if (from_itr == pmmap->end())
    return false;

  lastfromElement = pmmap->upper_bound(from_baseuuid);

  // Get a list of entries to be moved as doing it in-place
  // will cause a loop as modifying mmap while processing it!
  UUIDVector tlist;
  for (; from_itr != lastfromElement; from_itr++) {
    tlist.push_back(from_itr->second);
  }

  for (size_t idep = 0 ; idep < tlist.size(); idep++) {
    // Add to new base in base -> entry multimap
    pmmap->insert(ItemMMap_Pair(to_baseuuid, tlist[idep]));
  }

  // Now delete all old base entries
  pmmap->erase(from_baseuuid);
  return true;
}

int PWScore::DoAddDependentEntries(UUIDVector &dependentlist, CReport *pRpt,
                                   const CItemData::EntryType type, const int &iVia,
                                   ItemList *pmapDeletedItems,
                                   SaveTypePWMap *pmapSaveTypePW)
{
  // When called during validation of a database  - *pRpt is valid
  // When called during the opening of a database or during drag & drop
  //   - *pRpt is nullptr and no report generated

  // type is either CItemData::ET_ALIAS or CItemData::ET_SHORTCUT

  // If iVia == CItemData::UUID, the password was "[[uuidstr]]" or "[~uuidstr~]" of the
  //   associated base entry
  // If iVia == CItemData::PASSWORD, the password is expected to be in the full format
  // [g:t:u], where g and/or u may be empty.

  if (pmapDeletedItems != nullptr)
    pmapDeletedItems->clear();

  if (pmapSaveTypePW != nullptr)
    pmapSaveTypePW->clear();

  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmmap = &m_base2shortcuts_mmap;
  } else
    return -1;

  int num_warnings(0);
  st_SaveTypePW st_typepw;

  if (!dependentlist.empty()) {
    UUIDVectorIter paiter;
    ItemListIter iter;
    StringX sxPwdGroup, sxPwdTitle, sxPwdUser, tmp;
    CUUID base_uuid(CUUID::NullUUID());
    bool bwarnings(false);
    stringT strError;

    for (paiter = dependentlist.begin();
         paiter != dependentlist.end(); paiter++) {
      iter = m_pwlist.find(*paiter);
      if (iter == m_pwlist.end())
        return num_warnings;

      CItemData *pci_curitem = &iter->second;
      CUUID entry_uuid = pci_curitem->GetUUID();

      if (iVia == CItemData::UUID) {
        base_uuid = pci_curitem->GetBaseUUID();
        iter = m_pwlist.find(base_uuid);
      } else {
        tmp = pci_curitem->GetPassword();
        // Remove leading '[['/'[~' & trailing ']]'/'~]'
        tmp = tmp.substr(2, tmp.length() - 4);
        if (std::count(tmp.begin(), tmp.end(), _T(':')) == 2) {
          sxPwdGroup = tmp.substr(0, tmp.find_first_of(_T(':')));
          // Skip over 'group:'
          tmp = tmp.substr(sxPwdGroup.length() + 1);
          sxPwdTitle = tmp.substr(0, tmp.find_first_of(_T(':')));
          // Skip over 'title:'
          sxPwdUser = tmp.substr(sxPwdTitle.length() + 1);
          iter = Find(sxPwdGroup, sxPwdTitle, sxPwdUser);
          base_uuid = iter->second.GetUUID();
        } else {
          iter = m_pwlist.end();
        }
      }

      if (iter != m_pwlist.end()) {
        ASSERT(base_uuid != CUUID::NullUUID());
        pci_curitem->SetBaseUUID(base_uuid);
        if (type == CItemData::ET_SHORTCUT) {
          // Adding shortcuts -> Base must be normal or already a shortcut base
          if (!iter->second.IsNormal() && !iter->second.IsShortcutBase()) {
            // Bad news!
            if (pRpt != nullptr) {
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
            if (pmapDeletedItems != nullptr)
              pmapDeletedItems->insert(ItemList_Pair(*paiter, *pci_curitem));
            m_pwlist.erase(iter);
            continue;
          }
        }
        if (type == CItemData::ET_ALIAS) {
          // Adding Aliases -> Base must be normal or already a alias base
          if (!iter->second.IsNormal() && !iter->second.IsAliasBase()) {
            // Bad news!
            if (pRpt != nullptr) {
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
            if (pmapDeletedItems != nullptr)
              pmapDeletedItems->insert(ItemList_Pair(*paiter, *pci_curitem));
            m_pwlist.erase(iter);
            continue;
          }
          if (iter->second.IsAlias()) {
            // This is an alias too!  Not allowed!  Make new one point to original base
            // Note: this may be random as who knows the order of reading records?
            CUUID temp_uuid = iter->second.GetUUID();
            base_uuid = iter->second.GetBaseUUID(); // ??? used here ???
            if (pRpt != nullptr) {
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
            if (pmapSaveTypePW != nullptr) {
              st_typepw.et = iter->second.GetEntryType();
              st_typepw.sxpw = _T("");
              pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
            }
            pci_curitem->SetAlias();
            num_warnings++;
          }
        }
        base_uuid = iter->second.GetUUID();
        if (type == CItemData::ET_ALIAS) {
          if (pmapSaveTypePW != nullptr) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = _T("");
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          iter->second.SetAliasBase();
        } else
        if (type == CItemData::ET_SHORTCUT) {
          if (pmapSaveTypePW != nullptr) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = _T("");
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          iter->second.SetShortcutBase();
        }

        pmmap->insert(ItemMMap_Pair(base_uuid, entry_uuid));
        if (type == CItemData::ET_ALIAS) {
          if (pmapSaveTypePW != nullptr) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = pci_curitem->GetPassword();
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          pci_curitem->SetPassword(_T("[Alias]"));
          pci_curitem->SetAlias();
        } else
        if (type == CItemData::ET_SHORTCUT) {
          if (pmapSaveTypePW != nullptr) {
            st_typepw.et = iter->second.GetEntryType();
            st_typepw.sxpw = pci_curitem->GetPassword();
            pmapSaveTypePW->insert(SaveTypePWMap_Pair(*paiter, st_typepw));
          }
          pci_curitem->SetPassword(_T("[Shortcut]"));
          pci_curitem->SetShortcut();
        }
      } else {
        // Specified base does not exist!
        if (pRpt != nullptr) {
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
          if (pmapDeletedItems != nullptr)
            pmapDeletedItems->insert(ItemList_Pair(*paiter, *pci_curitem));
        } else {
          if (pmapSaveTypePW != nullptr) {
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

void PWScore::ResetAllAliasPasswords(const CUUID &base_uuid)
{
  // Alias ONLY - no shortcut version needed
  ItemMMapIter itr;
  ItemMMapIter lastElement;
  ItemListIter base_itr, alias_itr;
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
    CUUID alias_uuid = itr->second;
    alias_itr = m_pwlist.find(alias_uuid);
    if (alias_itr != m_pwlist.end()) {
      alias_itr->second.SetPassword(csBasePassword);
      alias_itr->second.SetNormal();
      GUIRefreshEntry(alias_itr->second);
    }
  }
  m_base2aliases_mmap.erase(base_uuid);
}

void PWScore::GetAllDependentEntries(const CUUID &base_uuid, UUIDVector &tlist,
                                     const CItemData::EntryType type)
{
  ItemMMapIter itr;
  ItemMMapIter lastElement;

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
    tlist.push_back(itr->second);
  }
}

bool PWScore::ParseBaseEntryPWD(const StringX &Password, BaseEntryParms &pl)
{
  // pl.ibasedata is:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but either no base entry found or 
  //      no unique entry found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and
  //  is only valid if n = -1 or -2.

  // Returns true if in a valid alias format, false if not

  pl.bMultipleEntriesFound = false;

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
        pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(':')));
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
        pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(':')));
        tmp = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
        pl.csPwdTitle = tmp.substr(0, tmp.find_first_of(_T(':')));    // Skip over 'title:'
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
        pl.base_uuid = iter->second.GetBaseUUID();
      } else {
        // This may not be a valid combination of source+target entries - sorted out by caller
        pl.base_uuid = iter->second.GetUUID();
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
  ASSERT(pAliasOrSC != nullptr);
  if (pAliasOrSC->IsDependent()) {
    const CUUID base_uuid = pAliasOrSC->GetBaseUUID();
    auto iter = Find(base_uuid);
    if (iter != GetEntryEndIter())
      return &iter->second;
    else
      pws_os::Trace(_T("PWScore::GetBaseEntry - Find(base_uuid) failed!\n"));
  }
  return nullptr;
}

/*
 *  Start UI Interface feedback routines
 */

void PWScore::NotifyDBModified()
{
  // This allows the core to provide feedback to the UI that the Database
  // has changed particularly to invalidate any current Find results and
  // to populate message during Vista and later shutdowns
  if (m_bNotifyDB) {
    for(auto& observer : m_Observers) {
      observer->DatabaseModified(HasDBChanged());
    }
  }
}

void PWScore::NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action ga,
                                     const CUUID &entry_uuid,
                                     CItemData::FieldType ft)
{
  // This allows the core to provide feedback to the UI that the GUI needs
  // updating due to a field having its value changed
  for(auto& observer : m_Observers) {
    observer->UpdateGUI(ga, entry_uuid, ft);
  }
}

void PWScore::NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action ga,
                                     const std::vector<StringX> &vGroups)
{
  // This allows the core to provide feedback to the UI that the GUI needs
  // updating due to a field having its value changed
  for(auto& observer : m_Observers) {
    observer->UpdateGUI(ga, vGroups);
  }
}

void PWScore::GUIRefreshEntry(const CItemData &ci, bool bAllowFail)
{
  // This allows the core to provide feedback to the UI that a particular
  // entry has been modified
  for(auto& observer : m_Observers) {
    observer->GUIRefreshEntry(ci, bAllowFail);
  }
}

void PWScore::UpdateWizard(const stringT &s)
{
  // This allows the core to provide feedback to the Compare, Merge, Synchronize,
  // Export (Text/XML) UI wizard as to the entry currently being processed.
  // The UI must be able to access the control in the wizard and the supplied
  // string gives the full 'group, title, user' of the entry.
  // It is expected that the UI will implement a pointer or other reference to
  // this control so that it can update the text displayed there (see MFC implementation).
  for(auto& observer : m_Observers) {
    observer->UpdateWizard(s);
  }
}

/*
 *  End UI Interface feedback routines
 */

bool PWScore::LockFile(const stringT &filename, stringT &locker)
{
  return pws_os::LockFile(filename, locker,
                          m_lockFileHandle);
}

bool PWScore::IsLockedFile(const stringT &filename) const
{
  return pws_os::IsLockedFile(filename);
}

void PWScore::UnlockFile(const stringT &filename)
{
  return pws_os::UnlockFile(filename, m_lockFileHandle);
}

void PWScore::SafeUnlockCurFile()
{
  const std::wstring filename(GetCurFile().c_str());

  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.empty() && !IsReadOnly() && IsLockedFile(filename))
    UnlockFile(filename);
}

bool PWScore::LockFile2(const stringT &filename, stringT &locker)
{
  return pws_os::LockFile(filename, locker,
                          m_lockFileHandle2);
}

void PWScore::UnlockFile2(const stringT &filename)
{
  return pws_os::UnlockFile(filename, m_lockFileHandle2);
}

bool PWScore::IsNodeModified(StringX &path) const
{
  if (!IsEmptyGroup(path)) {
    return std::find(m_vModifiedNodes.begin(),
      m_vModifiedNodes.end(), path) != m_vModifiedNodes.end();
  } else {
    return find(m_InitialEmptyGroups.begin(), m_InitialEmptyGroups.end(), path) ==
      m_InitialEmptyGroups.end();
  }
}

void PWScore::AddChangedNodes(StringX path)
{
  StringX nextpath(path);
  while (!nextpath.empty()) {
    if (std::find(m_vModifiedNodes.begin(), m_vModifiedNodes.end(), nextpath) ==
        m_vModifiedNodes.end())
      m_vModifiedNodes.push_back(nextpath);
    size_t i = nextpath.find_last_of(_T('.'));
    if (i == nextpath.npos)
      i = 0;
    nextpath = nextpath.substr(0, i);
  }
}

// functor objects for updating password history for each entry

struct HistoryUpdater {
  HistoryUpdater(int &num_altered,
                 SavePWHistoryMap &mapSavedHistory, bool bExcludeProtected)
  : m_num_altered(num_altered), m_mapSavedHistory(mapSavedHistory),
   m_bExcludeProtected(bExcludeProtected)
  {}
  virtual void operator() (CItemData &ci) = 0;
  virtual ~HistoryUpdater() {}
  HistoryUpdater& operator=(const HistoryUpdater&) = delete;

protected:
  int &m_num_altered;
  SavePWHistoryMap &m_mapSavedHistory;
  std::vector<BYTE> m_vSavedEntryStatus;
  bool m_bExcludeProtected;
};

struct HistoryUpdateResetOff : public HistoryUpdater {
  HistoryUpdateResetOff(int &num_altered,
                        SavePWHistoryMap &mapSavedHistory, bool bExcludeProtected)
 : HistoryUpdater(num_altered, mapSavedHistory, bExcludeProtected) {}

  void operator()(CItemData &ci) {
    if (!ci.IsProtected() ||
        (!m_bExcludeProtected && ci.IsProtected())) {
      StringX cs_tmp = ci.GetPWHistory();
      if (cs_tmp.length() >= 5 && cs_tmp[0] == L'1') {
        st_PWH_status st_pwhs;
        st_pwhs.pwh = cs_tmp;
        st_pwhs.es = ci.GetStatus();
        m_mapSavedHistory[ci.GetUUID()] = st_pwhs;
        cs_tmp[0] = L'0';
        ci.SetPWHistory(cs_tmp);
        ci.SetStatus(CItemData::ES_MODIFIED);
        m_num_altered++;
      }
    }
  }

private:
  HistoryUpdateResetOff& operator=(const HistoryUpdateResetOff&) = delete;
};

struct HistoryUpdateResetOn : public HistoryUpdater {
  HistoryUpdateResetOn(int &num_altered, int new_default_max,
                       SavePWHistoryMap &mapSavedHistory, bool bExcludeProtected)
    : HistoryUpdater(num_altered, mapSavedHistory, bExcludeProtected)
  {Format(m_text, L"1%02x00", new_default_max);}

  void operator()(CItemData &ci) {
    if (!ci.IsProtected() ||
        (!m_bExcludeProtected && ci.IsProtected())) {
      StringX cs_tmp = ci.GetPWHistory();
      st_PWH_status st_pwhs;
      st_pwhs.pwh = cs_tmp;
      st_pwhs.es = ci.GetStatus();
      if (cs_tmp.length() < 5) {
        m_mapSavedHistory[ci.GetUUID()] = st_pwhs;
        ci.SetPWHistory(m_text);
        m_num_altered++;
      } else {
        if (cs_tmp[0] == L'0') {
          m_mapSavedHistory[ci.GetUUID()] = st_pwhs;
          cs_tmp[0] = L'1';
          ci.SetPWHistory(cs_tmp);
          ci.SetStatus(CItemData::ES_MODIFIED);
          m_num_altered++;
        }
      }
    }
  }

private:
  HistoryUpdateResetOn& operator=(const HistoryUpdateResetOn&) = delete;
  StringX m_text;
};

struct HistoryUpdateSetMax : public HistoryUpdater {
  HistoryUpdateSetMax(int &num_altered, int new_default_max,
                      SavePWHistoryMap &mapSavedHistory, bool bExcludeProtected)
    : HistoryUpdater(num_altered, mapSavedHistory, bExcludeProtected),
    m_new_default_max(new_default_max)
  {Format(m_text, L"1%02x", new_default_max);}

  void operator()(CItemData &ci) {
    if (!ci.IsProtected() ||
        (!m_bExcludeProtected && ci.IsProtected())) {
      StringX cs_tmp = ci.GetPWHistory();

      size_t len = cs_tmp.length();
      if (len >= 5) {
        st_PWH_status st_pwhs;
        st_pwhs.pwh = cs_tmp;
        st_pwhs.es = ci.GetStatus();
        m_mapSavedHistory[ci.GetUUID()] = st_pwhs;
        int status, old_max, num_saved;
        const wchar_t *lpszPWHistory = cs_tmp.c_str();
        int iread = _stscanf_s(lpszPWHistory, _T("%01d%02x%02x"),
                               &status, &old_max, &num_saved);
        if (iread == 3 && status == 1 && num_saved <= m_new_default_max) {
          cs_tmp = m_text + cs_tmp.substr(3);
          ci.SetPWHistory(cs_tmp);
          ci.SetStatus(CItemData::ES_MODIFIED);
          m_num_altered++;
        }
      }
    }
  }

private:
  HistoryUpdateSetMax& operator=(const HistoryUpdateSetMax&) = delete;
  int m_new_default_max;
  StringX m_text;
};

struct HistoryUpdateClearAll : public HistoryUpdater {
  HistoryUpdateClearAll(int &num_altered,
                        SavePWHistoryMap &mapSavedHistory, bool bExcludeProtected)
  : HistoryUpdater(num_altered, mapSavedHistory, bExcludeProtected) {}

  void operator()(CItemData &ci) {
    if (!ci.IsProtected() ||
        (!m_bExcludeProtected && ci.IsProtected())) {
      StringX cs_tmp = ci.GetPWHistory();
      size_t len = cs_tmp.length();
      if (len != 0 && cs_tmp != _T("00000")) {
        st_PWH_status st_pwhs;
        st_pwhs.pwh = cs_tmp;
        st_pwhs.es = ci.GetStatus();
        m_mapSavedHistory[ci.GetUUID()] = st_pwhs;
        ci.SetPWHistory(L"");
        ci.SetStatus(CItemData::ES_MODIFIED);
        m_num_altered++;
      }
    }
  }

private:
  HistoryUpdateClearAll& operator=(const HistoryUpdateClearAll&) = delete;
};

int PWScore::DoUpdatePasswordHistory(int iAction, int new_default_max,
                                     SavePWHistoryMap &mapSavedHistory)
{
  int num_altered = 0;
  HistoryUpdater *updater = nullptr;
  bool bExcludeProtected(true);

  if (iAction < 0)
    bExcludeProtected = false;

  switch (iAction) {
    case PWHist::STOP_INCL_PROT:   // reset off - include protected entries
    case PWHist::STOP_EXCL_PROT:   // reset off - exclude protected entries
      updater = new HistoryUpdateResetOff(num_altered, mapSavedHistory, bExcludeProtected);
      break;
    case PWHist::START_INCL_PROT:   // reset on - include protected entries
    case PWHist::START_EXCL_PROT:   // reset on - exclude protected entries
      updater = new HistoryUpdateResetOn(num_altered, new_default_max, mapSavedHistory, bExcludeProtected);
      break;
    case PWHist::SETMAX_INCL_PROT:   // setmax   - include protected entries
    case PWHist::SETMAX_EXCL_PROT:   // setmax   - exclude protected entries
      updater = new  HistoryUpdateSetMax(num_altered, new_default_max, mapSavedHistory, bExcludeProtected);
      break;
    case PWHist::CLEAR_INCL_PROT:   // clearall - include protected entries
    case PWHist::CLEAR_EXCL_PROT:   // clearall - exclude protected entries
      updater = new HistoryUpdateClearAll(num_altered, mapSavedHistory, bExcludeProtected);
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
  * so reverted to slightly less elegant for loop.
  * Using polymorphism for the history updater
  * is an unrelated tweak.
  */

  if (updater != nullptr) {
    ItemListIter listPos;
    for (listPos = m_pwlist.begin(); listPos != m_pwlist.end(); listPos++) {
      CItemData &curitem = listPos->second;
      (*updater)(curitem);
    }
  }
  delete updater;
  return num_altered;
}

void PWScore::UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory)
{
  SavePWHistoryMap::iterator itr;

  for (itr = mapSavedHistory.begin(); itr != mapSavedHistory.end(); itr++) {
    auto listPos = m_pwlist.find(itr->first);
    if (listPos != m_pwlist.end()) {
      listPos->second.SetPWHistory(itr->second.pwh);
      listPos->second.SetStatus(itr->second.es);
    }
  }
}

int PWScore::DoRenameGroup(const StringX &sxOldPath, const StringX &sxNewPath,
                           MultiCommands * &pmulticmds)
{
  const StringX sxDot(L".");
  const wchar_t wcDot=L'.';
  StringX sxOldPath2 = sxOldPath + sxDot;
  const size_t len2 = sxOldPath2.length();
  ItemListIter iter;

  pmulticmds = MultiCommands::Create(this);

  // Nested Multicommand so set in a MultiCommand so that it doesn't save information again
  pmulticmds->SetNested();

  Command *pcmd;

  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    if (iter->second.GetGroup() == sxOldPath) {
      pcmd = UpdateEntryCommand::Create(this, iter->second,
                                        CItemData::GROUP, sxNewPath);
      pcmd->SetNoGUINotify();
      pmulticmds->Add(pcmd);
    }
    else if ((iter->second.GetGroup().length() > len2) && (iter->second.GetGroup().substr(0, len2) == sxOldPath2) &&
     (iter->second.GetGroup()[len2] != wcDot)) {
      // Need to check that next symbol is not a dot
      // to ensure not affecting another group
      // (group name could contain trailing dots, for example abc..def.g)
      // subgroup name will have len > len2 (old_name + dot + subgroup_name)
      StringX sxSubGroups = iter->second.GetGroup().substr(len2);

      pcmd = UpdateEntryCommand::Create(this, iter->second,
                                  CItemData::GROUP, sxNewPath + sxDot + sxSubGroups);
      pcmd->SetNoGUINotify();
      pmulticmds->Add(pcmd);
    }
  }

  return 0;
}

void PWScore::UndoRenameGroup(MultiCommands *pmulticmds)
{
  pmulticmds->Undo();
}

int PWScore::DoChangeHeader(const StringX &sxNewValue, const PWSfile::HeaderType ht)
{
  return SetHeaderItem(sxNewValue, ht);
}

void PWScore::UndoChangeHeader(const StringX &sxOldValue, const PWSfile::HeaderType ht)
{
  DoChangeHeader(sxOldValue, ht);
}

void PWScore::GetDBProperties(st_DBProperties &st_dbp)
{
  st_dbp.database = m_currfile;

  Format(st_dbp.databaseformat, L"%d.%02d",
                          m_hdr.m_nCurrentMajorVersion,
                          m_hdr.m_nCurrentMinorVersion);

  std::vector<std::wstring> vAllGroups;
  GetAllGroups(vAllGroups);
  Format(st_dbp.numgroups, L"%d", vAllGroups.size());
  Format(st_dbp.numemptygroups, L"%d", m_vEmptyGroups.size());
  Format(st_dbp.numentries, L"%d", m_pwlist.size());
  if (GetReadFileVersion() >= PWSfile::V40)
    Format(st_dbp.numattachments, L"%d", m_attlist.size());
  else
    st_dbp.numattachments = L"N/A";

  time_t twls = m_hdr.m_whenlastsaved;
  if (twls == 0) {
    LoadAString(st_dbp.whenlastsaved, IDSC_UNKNOWN);
  } else {
    st_dbp.whenlastsaved = PWSUtil::ConvertToDateTimeString(twls, PWSUtil::TMC_EXPORT_IMPORT);
  }

  time_t tpwdlc = m_hdr.m_whenpwdlastchanged;
  if (tpwdlc == 0) {
    LoadAString(st_dbp.whenpwdlastchanged, IDSC_UNKNOWN);
  } else {
    st_dbp.whenpwdlastchanged = PWSUtil::ConvertToDateTimeString(tpwdlc, PWSUtil::TMC_EXPORT_IMPORT);
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

  if(m_hdr.m_file_uuid == CUUID::NullUUID())
    st_dbp.file_uuid = _T("N/A");
  else {
    CUUID huuid(*m_hdr.m_file_uuid.GetARep(), true); // true for canonical format
    ostringstreamT os;
    os << std::uppercase << huuid;
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
      st_dbp.unknownfields += L")";
    } else {
      StringX wls;
      Format(wls, L"%d", num);
      st_dbp.unknownfields += wls;
      st_dbp.unknownfields += L")";
    }
  } else {
    LoadAString(st_dbp.unknownfields, IDSC_NONE);
  }

  st_dbp.db_name = m_hdr.m_DB_Name;
  st_dbp.db_description = m_hdr.m_DB_Description;
}

StringX PWScore::GetHeaderItem(PWSfile::HeaderType ht)
{
  switch (ht) {
  case PWSfile::HDR_DBNAME:
    return m_hdr.m_DB_Name;
  case PWSfile::HDR_DBDESC:
    return m_hdr.m_DB_Description;
  default:
    ASSERT(0);
    return StringX(_T(""));
  }
}

int PWScore::SetHeaderItem(const StringX &sxNewValue, PWSfile::HeaderType ht)
{
  int rc = PWScore::SUCCESS;
  switch (ht) {
  case PWSfile::HDR_DBNAME:
    m_hdr.m_DB_Name = sxNewValue;
    break;
  case PWSfile::HDR_DBDESC:
    m_hdr.m_DB_Description = sxNewValue;
    break;
  default:
    ASSERT(0);
    rc = PWScore::FAILURE;
  }

  return rc;
}

void PWScore::UpdateExpiryEntry(const CUUID &uuid, const CItemData::FieldType ft,
                                const StringX &value)
{
  ExpiredList::iterator iter;

  iter = std::find_if(m_ExpireCandidates.begin(), m_ExpireCandidates.end(),
                      std::bind2nd(std::equal_to<pws_os::CUUID>(), uuid));
  if (iter == m_ExpireCandidates.end())
    return;

  if (ft == CItemData::XTIME) {
    time_t t;
    if ((VerifyImportDateTimeString(value.c_str(), t) ||
         VerifyXMLDateTimeString(value.c_str(), t)    ||
         VerifyASCDateTimeString(value.c_str(), t))   &&
         (t != time_t(-1))) {  // checkerror despite all our verification!
      iter->expirytttXTime = t;
    } else {
      ASSERT(0);
    }
  } else {
    ASSERT(0);
  }
}

bool PWScore::ChangeMode(stringT &locker, int &iErrorCode)
{
  PWS_LOGIT;

  // We do not have to close or re-open the database as the database is closed after processing.
  /*
   So what do we need to do?

   If currently R/O, we need to lock the database.
   If currently R/W, we need to unlock the database.
  */
  iErrorCode = SUCCESS;
  locker = _T(""); // Important!

  if (m_bIsReadOnly) {
    // We know the file did exist but this will also determine if it is R-O
    bool isRO;
    if (pws_os::FileExists(m_currfile.c_str(), isRO) && isRO) {
      // OK - still exists but is R-O - can't change mode!
      // Need new return code but not this close to release - later
      iErrorCode = READ_FAIL;
      PWS_LOGIT_ARGS0("Failed: READ_FAIL");
      return false;
    }

    // OK, we have write access, let's lock it
    bool brc = pws_os::LockFile(m_currfile.c_str(), locker, m_lockFileHandle);
    if (!brc) {
      iErrorCode = CANT_GET_LOCK;
      PWS_LOGIT_ARGS0("Failed: CANT_GET_LOCK");
      return false;
    }

    // It was R-O, better check no-one has changed anything from in-memory copy
    // The one calculated when we read it in is 'm_pFileSig' (R-O - so we haven't changed it)
    // This is the new one
    PWSFileSig newFileSig = PWSFileSig(m_currfile.c_str());
    if (newFileSig.IsValid() && *m_pFileSig != newFileSig) {
      // Oops - someone else has changed this user will need to close and open properly.
      // Or the file signature is invalid e.g. file not there or fie size too small.
      // Tell them the bad news after unlocking file and not changing mode
      iErrorCode = DB_HAS_CHANGED;
    } else {
      // Other error - e.g. can't open file or it is too small.
      iErrorCode = newFileSig.GetErrorCode();
    }
    if (iErrorCode != 0) {
      pws_os::UnlockFile(m_currfile.c_str(), m_lockFileHandle);
      PWS_LOGIT_ARGS("Failed code: %d", iErrorCode);
      return false;
    }
  } else { // In R/W mode - switch to R-O
    // Unlock file
    pws_os::UnlockFile(m_currfile.c_str(), m_lockFileHandle);
  }

  // Swap Read/Write : Read/Only status
  m_bIsReadOnly = !m_bIsReadOnly;

  return true;
}

// Yubi support:
const unsigned char *PWScore::GetYubiSK() const
{
  return m_hdr.m_yubi_sk;
}

void PWScore::SetYubiSK(const unsigned char *sk)
{
  if (m_hdr.m_yubi_sk)
    trashMemory(m_hdr.m_yubi_sk, PWSfileHeader::YUBI_SK_LEN);
  delete[] m_hdr.m_yubi_sk;
  m_hdr.m_yubi_sk = nullptr;
  if (sk != nullptr) {
    m_hdr.m_yubi_sk = new unsigned char[PWSfileHeader::YUBI_SK_LEN];
    memcpy(m_hdr.m_yubi_sk, sk, PWSfileHeader::YUBI_SK_LEN);
  }
}

bool PWScore::SetPasswordPolicies(const PSWDPolicyMap &MapPSWDPLC)
{
  bool brc(false);
  if (m_MapPSWDPLC != MapPSWDPLC) {
    m_MapPSWDPLC = MapPSWDPLC;
    brc = true;
  }
  return brc;
}

bool PWScore::SetDBFilters(const PWSFilters &MapDBFilters)
{
  bool brc(false);
  if (m_MapDBFilters != MapDBFilters) {
    m_MapDBFilters = MapDBFilters;
    brc = true;
  }
  return brc;
}

bool PWScore::IncrementPasswordPolicy(const StringX &sxPolicyName)
{
  auto iter = m_MapPSWDPLC.find(sxPolicyName);
  if (iter == m_MapPSWDPLC.end()) {
    return false;
  } else {
    iter->second.usecount++;
    return true;
  }
}

bool PWScore::DecrementPasswordPolicy(const StringX &sxPolicyName)
{
  auto iter = m_MapPSWDPLC.find(sxPolicyName);
  if (iter == m_MapPSWDPLC.end() || iter->second.usecount == 0) {
    return false;
  } else {
    iter->second.usecount--;
    return true;
  }
}

bool PWScore::AddPolicy(const StringX &sxPolicyName, const PWPolicy &st_pp,
                        const bool bAllowReplace)
{
  bool bDoIt(false);
  auto iter = m_MapPSWDPLC.find(sxPolicyName);

  if (iter == m_MapPSWDPLC.end())
    bDoIt = true;
  else if (bAllowReplace) {
    bDoIt = true;
    m_MapPSWDPLC.erase(iter);
  }
  if (bDoIt) {
    m_MapPSWDPLC[sxPolicyName] = st_pp;
  }
  return bDoIt;
}

bool PWScore::SetEmptyGroups(const std::vector<StringX> &vEmptyGroups)
{
  bool brc(false);
  if (m_vEmptyGroups != vEmptyGroups) {
    m_vEmptyGroups = vEmptyGroups;

    // Now sort it for when we compare.
    std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());
    brc = true;
  }
  return brc;
}

bool PWScore::IsEmptyGroup(const StringX &sxEmptyGroup) const
{
  return find(m_vEmptyGroups.begin(), m_vEmptyGroups.end(), sxEmptyGroup) !=
              m_vEmptyGroups.end();
}

bool PWScore::AddEmptyGroup(const StringX &sxEmptyGroup)
{
  // Don't add root - can happen when deleting last entry in root
  // Put check here too just incase caused elsewhere
  if (sxEmptyGroup.empty())
    return false;

  std::vector<stringT> vAllNonEmptyGroups;
  GetAllGroups(vAllNonEmptyGroups, false);

  std::sort(vAllNonEmptyGroups.begin(), vAllNonEmptyGroups.end());
  stringT sEG = sxEmptyGroup.c_str();

  // Don't add if an entry with this group alreadly exists
  if (find(vAllNonEmptyGroups.begin(), vAllNonEmptyGroups.end(), sEG) !=
           vAllNonEmptyGroups.end())
    return false;

  // Only add if not already present
  if (find(m_vEmptyGroups.begin(), m_vEmptyGroups.end(), sxEmptyGroup) ==
           m_vEmptyGroups.end()) {
    // Add it
    m_vEmptyGroups.push_back(sxEmptyGroup);

    // Then sort it for when we compare.
    // Could use std::set but unnecessary complication/overhead
    std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());
    return true;
  } else
    return false;
}

bool PWScore::RemoveEmptyGroup(const StringX &sxEmptyGroup)
{
  std::vector<StringX>::iterator iter;
  iter = find(m_vEmptyGroups.begin(), m_vEmptyGroups.end(), sxEmptyGroup);

  if (iter != m_vEmptyGroups.end()) {
    m_vEmptyGroups.erase(iter);
    return true;
  } else
    return false;
}

bool PWScore::RenameEmptyGroup(const StringX &sxOldGroup, const StringX &sxNewGroup)
{
  bool bChanged(false);
  std::vector<StringX>::iterator iter;
  iter = find(m_vEmptyGroups.begin(), m_vEmptyGroups.end(), sxOldGroup);
  if (iter != m_vEmptyGroups.end()) {
    // Delete old name
    m_vEmptyGroups.erase(iter);
    // Add new name
    m_vEmptyGroups.push_back(sxNewGroup);
    // Sort it for when we compare.
    std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());
    bChanged = true;
  } else {
    ASSERT(0);
  }
  return bChanged;
}

bool PWScore::RenameEmptyGroupPaths(const StringX &sxOldPath, const StringX &sxNewPath)
{
  // Rename all empty group paths below this renamed group so that they 
  // stay within this new group tree
  bool bChanged(false);

  if (sxOldPath != sxNewPath) {
    const StringX sxOldPath2 = sxOldPath + L".";
    const size_t len = sxOldPath2.length();
    for (size_t ig = 0; ig < m_vEmptyGroups.size(); ig++) {
      if (m_vEmptyGroups[ig].length() > len && m_vEmptyGroups[ig].substr(0, len) == sxOldPath2) {
        m_vEmptyGroups[ig].replace(0, len - 1, sxNewPath);
        bChanged = true;
      }
    }

    // Now sort it for when we compare.
    std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());
  }
  return bChanged;
}

bool PWScore::AddKBShortcut(const int &iKBShortcut, const pws_os::CUUID &uuid)
{
  std::pair< std::map<int, pws_os::CUUID>::iterator, bool > pr;
  pr = m_KBShortcutMap.insert(KBShortcutMapPair(iKBShortcut, uuid));

  return pr.second;
}

bool PWScore::DelKBShortcut(const int32 &iKBShortcut, const pws_os::CUUID &uuid)
{
  auto iter = m_KBShortcutMap.find(iKBShortcut);

  if (iter == m_KBShortcutMap.end())
    return false;
  else {
    ASSERT(uuid == iter->second);
    if (uuid == iter->second) {
      m_KBShortcutMap.erase(iter);
    }
    return true;
  }
}

const pws_os::CUUID & PWScore::GetKBShortcut(const int32 &iKBShortcut)
{
  KBShortcutMapConstIter iter = m_KBShortcutMap.find(iKBShortcut);

  if (iter == m_KBShortcutMap.end())
    return CUUID::NullUUID();
  else
    return iter->second;
}

uint32 PWScore::GetHashIters() const
{
  return m_hashIters;
}

void PWScore::SetHashIters(uint32 value)
{
  m_hashIters = value;
}

void PWScore::RemoveAtt(const pws_os::CUUID &attuuid)
{
  // Should be a Command setting new CommandDBChange enum value
  ASSERT(HasAtt(attuuid));
  //m_stDBCS.bDBChanged = true; // Can't do this outside a Command
  m_attlist.erase(m_attlist.find(attuuid));
}

std::set<StringX> PWScore::GetAllMediaTypes() const
{
  // std::set<> has the properties we need here:
  // 1. Members are unique
  // 2. Iterator returns them sorted
  std::set<StringX> sMediaTypes;

  // Find media types of all our attachments, put them in a set:
  for (auto att_iter = m_attlist.begin(); att_iter != m_attlist.end(); att_iter++) {
    sMediaTypes.insert(att_iter->second.GetMediaType());
  }
  return sMediaTypes;
}
