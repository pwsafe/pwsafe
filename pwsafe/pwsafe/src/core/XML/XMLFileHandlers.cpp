/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#ifdef USE_XML_LIBRARY

#include "XMLFileValidation.h"
#include "XMLFileHandlers.h"

// PWS includes
#include "../core.h"
#include "../ItemData.h"
#include "../Util.h"
#include "../PWSprefs.h"
#include "../PWScore.h"
#include "../PWSfileV3.h"
#include "../VerifyFormat.h"
#include "../Command.h"
#include "os/UUID.h"

#include <algorithm>

extern const TCHAR *FORMATIMPORTED;

using namespace std;
using pws_os::CUUID;

XMLFileHandlers::XMLFileHandlers()
{
  cur_entry = NULL;
  cur_pwhistory_entry = NULL;
  cur_policy = NULL;
  m_strElemContent.clear();

  m_sDefaultAutotypeString = _T("");
  m_sDefaultUsername = _T("");
  m_sDefaultSymbols = _T("");
  m_delimiter = _T('\0');
  m_strErrorMessage = _T("");

  m_iErrorCode = 0;

  m_bheader = false;
  m_bDatabaseHeaderErrors = false;
  m_bRecordHeaderErrors = false;
  m_bErrors = false;

  m_nITER = MIN_HASH_ITERATIONS;
  
  m_sxXML_DateTime = PWSUtil::GetTimeStamp(true).c_str();

  // Following are user preferences stored in the database and for PolicyNames
  for (int i = 0; i < NUMPREFSINXML; i++) {
    prefsinXML[i] = -1;
  }
}

XMLFileHandlers::~XMLFileHandlers()
{
  m_ukhxl.clear();
}

void XMLFileHandlers::SetVariables(PWScore *pcore, const bool &bValidation,
                                   const stringT &ImportedPrefix, const TCHAR &delimiter,
                                   const bool &bImportPSWDsOnly,
                                   UUIDVector *pPossible_Aliases, UUIDVector *pPossible_Shortcuts,
                                   MultiCommands *pmulticmds, CReport *prpt)
{
  m_bValidation = bValidation;
  m_delimiter = delimiter;
  m_pXMLcore = pcore;
  m_pPossible_Aliases = pPossible_Aliases;
  m_pPossible_Shortcuts = pPossible_Shortcuts;
  m_ImportedPrefix = ImportedPrefix;
  m_bImportPSWDsOnly = bImportPSWDsOnly;
  m_pmulticmds = pmulticmds;
  m_prpt = prpt;
}

bool XMLFileHandlers::ProcessStartElement(const int icurrent_element)
{
  switch (icurrent_element) {
    case XLE_PASSWORDSAFE:
      m_numEntries = 0;
      m_numEntriesSkipped = 0;
      m_numEntriesRenamed = 0;
      m_numEntriesPWHErrors = 0;
      m_numNoPolicies = 0;
      m_numRenamedPolicies = 0;
      m_bEntryBeingProcessed = false;
      break;
    case XLE_ENTRY:
      m_bEntryBeingProcessed = true;
      if (m_bValidation)
        return false;

      cur_entry = new pw_entry;
      // Clear all fields
      cur_entry->id = 0;
      cur_entry->group = _T("");
      cur_entry->title = _T("");
      cur_entry->username = _T("");
      cur_entry->password = _T("");
      cur_entry->url = _T("");
      cur_entry->autotype = _T("");
      cur_entry->ctime = _T("");
      cur_entry->atime = _T("");
      cur_entry->xtime = _T("");
      cur_entry->xtime_interval = _T("");
      cur_entry->pmtime = _T("");
      cur_entry->rmtime = _T("");
      cur_entry->pwhistory = _T("");
      cur_entry->notes = _T("");
      cur_entry->uuid = _T("");
      cur_entry->pwp.Empty();
      cur_entry->run_command = _T("");
      cur_entry->dca = _T("");
      cur_entry->shiftdca = _T("");
      cur_entry->email = _T("");
      cur_entry->symbols = _T("");
      cur_entry->policyname = _T("");
      cur_entry->ucprotected = 0;
      cur_entry->entrytype = NORMAL;
      cur_entry->bforce_normal_entry = false;
      break;
    case XLE_HISTORY_ENTRY:
      if (m_bValidation)
        return false;

      ASSERT(cur_pwhistory_entry == NULL);
      cur_pwhistory_entry = new pwhistory_entry;
      cur_pwhistory_entry->changed = _T("");
      cur_pwhistory_entry->oldpassword = _T("");
      break;
    case XLE_CTIMEX:
    case XLE_ATIMEX:
    case XLE_XTIMEX:
    case XLE_PMTIMEX:
    case XLE_RMTIMEX:
    case XLE_CHANGEDX:
      break;
    case XLE_PASSWORDPOLICYNAMES:
      m_bInPolicyNames = true;
      break;
    case XLE_POLICY:
      m_bPolicyBeingProcessed = true;
      if (m_bValidation)
        return false;

      // Reset preferences in XML for PolicyNames (we reuse those for preferences)
      for (int i = 0; i < NUMPREFSINXML; i++) {
        prefsinXML[i] = -1;
      }
      m_sNamedPolicySymbols = _T("");

      cur_policy = new pw_policy;

      // Clear all fields
      cur_policy->name = _T("");
      cur_policy->st_pp.pwp.Empty();
      cur_policy->st_pp.symbols = _T("");
      break;
    case XLE_PWNAME:
    default:
      break;
  }
  return true;
}

void XMLFileHandlers::ProcessEndElement(const int icurrent_element)
{
  StringX buffer(_T(""));
  int i;

  switch (icurrent_element) {
    case XLE_NUMBERHASHITERATIONS:
      i = _ttoi(m_strElemContent.c_str());
      if (i > MIN_HASH_ITERATIONS) {
        m_nITER = i;
      }
      break;
    case XLE_ENTRY:
      m_ventries.push_back(cur_entry);
      m_numEntries++;
      break;
    case XLE_DISPLAYEXPANDEDADDEDITDLG:
      // Obsoleted in 3.18
      return;
    case XLE_LOCKDBONIDLETIMEOUT:
    case XLE_IDLETIMEOUT:
    case XLE_MAINTAINDATETIMESTAMPS:
    case XLE_NUMPWHISTORYDEFAULT:
    case XLE_PWDEFAULTLENGTH:
    case XLE_PWDIGITMINLENGTH:
    case XLE_PWLOWERCASEMINLENGTH:
    case XLE_PWMAKEPRONOUNCEABLE:
    case XLE_PWSYMBOLMINLENGTH:
    case XLE_PWUPPERCASEMINLENGTH:
    case XLE_PWUSEDIGITS:
    case XLE_PWUSEEASYVISION:
    case XLE_PWUSEHEXDIGITS:
    case XLE_PWUSELOWERCASE:
    case XLE_PWUSESYMBOLS:
    case XLE_PWUSEUPPERCASE:
    case XLE_SAVEIMMEDIATELY:
    case XLE_SAVEPASSWORDHISTORY:
    case XLE_SHOWNOTESDEFAULT:
    case XLE_SHOWPASSWORDINTREE:
    case XLE_SHOWPWDEFAULT:
    case XLE_SHOWUSERNAMEINTREE:
    case XLE_SORTASCENDING:
    case XLE_USEDEFAULTUSER:
      prefsinXML[icurrent_element - XLE_PREF_START] = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_PASSWORDPOLICYNAMES:
      m_bInPolicyNames = false;
      break;
    case XLE_TREEDISPLAYSTATUSATOPEN:
      if (m_strElemContent == _T("AllCollapsed"))
        prefsinXML[XLE_TREEDISPLAYSTATUSATOPEN - XLE_PREF_START] = PWSprefs::AllCollapsed;
      else if (m_strElemContent == _T("AllExpanded"))
        prefsinXML[XLE_TREEDISPLAYSTATUSATOPEN - XLE_PREF_START] = PWSprefs::AllExpanded;
      else if (m_strElemContent == _T("AsPerLastSave"))
        prefsinXML[XLE_TREEDISPLAYSTATUSATOPEN - XLE_PREF_START] = PWSprefs::AsPerLastSave;
      break;
    case XLE_DEFAULTUSERNAME:
      m_sDefaultUsername = m_strElemContent.c_str();
      break;
    case XLE_DEFAULTAUTOTYPESTRING:
      m_sDefaultAutotypeString = m_strElemContent.c_str();
      break;
    case XLE_DEFAULTSYMBOLS:
      m_sDefaultSymbols = m_strElemContent.c_str();
      break;
    // MUST be in the same order as enum beginning STR_GROUP...
    case XLE_GROUP:
      cur_entry->group = m_strElemContent;
      break;
    case XLE_TITLE:
      cur_entry->title = m_strElemContent;
      break;
    case XLE_USERNAME:
      cur_entry->username = m_strElemContent;
      break;
    case XLE_URL:
      cur_entry->url = m_strElemContent;
      break;
    case XLE_AUTOTYPE:
      cur_entry->autotype = m_strElemContent;
      break;
    case XLE_NOTES:
      cur_entry->notes = m_strElemContent;
      break;
    case XLE_UUID:
      cur_entry->uuid = m_strElemContent;
      break;
    case XLE_PASSWORD:
      cur_entry->password = m_strElemContent;
      if (Replace(m_strElemContent, _T(':'), _T(';')) <= 2) {
        if (m_strElemContent.substr(0, 2) == _T("[[") &&
            m_strElemContent.substr(m_strElemContent.length() - 2) == _T("]]")) {
            cur_entry->entrytype = ALIAS;
        }
        if (m_strElemContent.substr(0, 2) == _T("[~") &&
            m_strElemContent.substr(m_strElemContent.length() - 2) == _T("~]")) {
            cur_entry->entrytype = SHORTCUT;
        }
      }
      break;
    case XLE_CTIMEX:
      cur_entry->ctime = m_strElemContent;
      break;
    case XLE_ATIMEX:
      cur_entry->atime = m_strElemContent;
      break;
    case XLE_XTIMEX:
      cur_entry->xtime = m_strElemContent;
      break;
    case XLE_PMTIMEX:
      cur_entry->pmtime = m_strElemContent;
      break;
    case XLE_RMTIMEX:
      cur_entry->rmtime = m_strElemContent;
      break;
    case XLE_XTIME_INTERVAL:
      cur_entry->xtime_interval = Trim(m_strElemContent);
      break;
    case XLE_RUNCOMMAND:
      cur_entry->run_command = m_strElemContent;
      break;
    case XLE_DCA:
      cur_entry->dca = Trim(m_strElemContent);
      break;
    case XLE_SHIFTDCA:
      cur_entry->shiftdca = Trim(m_strElemContent);
      break;
    case XLE_EMAIL:
      cur_entry->email = m_strElemContent;
      break;
    case XLE_PROTECTED:
      if (m_strElemContent == _T("1"))
        cur_entry->ucprotected = 1;
      break;
    case XLE_SYMBOLS:
      if (m_bEntryBeingProcessed)
        cur_entry->symbols = m_strElemContent;
      else
        cur_policy->st_pp.symbols = m_strElemContent;
      break;
    case XLE_ENTRY_PASSWORDPOLICYNAME:
      cur_entry->policyname = m_strElemContent;
      break;
    case XLE_STATUS:
      i = _ttoi(m_strElemContent.c_str());
      Format(buffer, _T("%01x"), i);
      cur_entry->pwhistory = buffer;
      break;
    case XLE_MAX:
      i = _ttoi(m_strElemContent.c_str());
      Format(buffer, _T("%02x"), i);
      cur_entry->pwhistory += buffer;
      break;
    case XLE_NUM:
      i = _ttoi(m_strElemContent.c_str());
      Format(buffer, _T("%02x"), i);
      cur_entry->pwhistory += buffer;
      break;
    case XLE_HISTORY_ENTRY:
      ASSERT(cur_pwhistory_entry != NULL);
      Format(buffer, _T("\xff%s\xff%04x\xff%s"),
             cur_pwhistory_entry->changed.c_str(),
             cur_pwhistory_entry->oldpassword.length(),
             cur_pwhistory_entry->oldpassword.c_str());
      cur_entry->pwhistory += buffer;
      delete cur_pwhistory_entry;
      cur_pwhistory_entry = NULL;
      break;
    case XLE_CHANGEDX:
      cur_pwhistory_entry->changed = m_strElemContent;
      break;
    case XLE_OLDPASSWORD:
      ASSERT(cur_pwhistory_entry != NULL);
      cur_pwhistory_entry->oldpassword = m_strElemContent;
      break;
    case XLE_ENTRY_PWLENGTH:
      cur_entry->pwp.length = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_ENTRY_PWUSEDIGITS:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseDigits;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseDigits;
      break;
    case XLE_ENTRY_PWUSEEASYVISION:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseEasyVision;
      break;
    case XLE_ENTRY_PWUSEHEXDIGITS:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseHexDigits;
      break;
    case XLE_ENTRY_PWUSELOWERCASE:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseLowercase;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseLowercase;
      break;
    case XLE_ENTRY_PWUSESYMBOLS:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseSymbols;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseSymbols;
      break;
    case XLE_ENTRY_PWUSEUPPERCASE:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseUppercase;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseUppercase;
      break;
    case XLE_ENTRY_PWMAKEPRONOUNCEABLE:
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyMakePronounceable;
      break;
    case XLE_ENTRY_PWDIGITMINLENGTH:
      cur_entry->pwp.digitminlength = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_ENTRY_PWLOWERCASEMINLENGTH:
      cur_entry->pwp.lowerminlength = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_ENTRY_PWSYMBOLMINLENGTH:
      cur_entry->pwp.symbolminlength = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_ENTRY_PWUPPERCASEMINLENGTH:
      cur_entry->pwp.upperminlength = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_PASSWORDSAFE:
    case XLE_PREFERENCES:
    case XLE_PWHISTORY:
    case XLE_HISTORY_ENTRIES:
    case XLE_ENTRY_PASSWORDPOLICY:
    default:
      break;
    case XLE_POLICY:
    {
      // Deal with Named Policies in the XML file
      int ivalue;
      PWSprefs *prefs = PWSprefs::GetInstance();
      if ((ivalue = prefsinXML[XLE_PWDEFAULTLENGTH - XLE_PREF_START]) != -1)
        cur_policy->st_pp.pwp.length = ivalue;
      else
        cur_policy->st_pp.pwp.length =
                              prefs->GetPrefDefVal(PWSprefs::PWDefaultLength);

      if (prefsinXML[XLE_PWUSEDIGITS - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyUseDigits;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyUseDigits;

      if (prefsinXML[XLE_PWUSEEASYVISION - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyUseEasyVision;

      if (prefsinXML[XLE_PWUSEHEXDIGITS - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyUseHexDigits;

      if (prefsinXML[XLE_PWUSELOWERCASE - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyUseLowercase;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyUseLowercase;

      if (prefsinXML[XLE_PWUSESYMBOLS - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyUseSymbols;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyUseSymbols;

      if (prefsinXML[XLE_PWUSEUPPERCASE - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyUseUppercase;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyUseUppercase;

      if (prefsinXML[XLE_PWMAKEPRONOUNCEABLE - XLE_PREF_START]  == 1)
        cur_policy->st_pp.pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
      else
        cur_policy->st_pp.pwp.flags &= ~PWSprefs::PWPolicyMakePronounceable;

      if ((ivalue = prefsinXML[XLE_PWDIGITMINLENGTH - XLE_PREF_START]) != -1)
        cur_policy->st_pp.pwp.digitminlength = ivalue;
      else
        cur_policy->st_pp.pwp.digitminlength =
                              prefs->GetPrefDefVal(PWSprefs::PWDigitMinLength);

      if ((ivalue = prefsinXML[XLE_PWLOWERCASEMINLENGTH - XLE_PREF_START]) != -1)
        cur_policy->st_pp.pwp.lowerminlength = ivalue;
      else
        cur_policy->st_pp.pwp.lowerminlength =
                              prefs->GetPrefDefVal(PWSprefs::PWLowercaseMinLength);

      if ((ivalue = prefsinXML[XLE_PWSYMBOLMINLENGTH - XLE_PREF_START]) != -1)
        cur_policy->st_pp.pwp.symbolminlength = ivalue;
      else
        cur_policy->st_pp.pwp.symbolminlength =
                              prefs->GetPrefDefVal(PWSprefs::PWSymbolMinLength);

      if ((ivalue = prefsinXML[XLE_PWUPPERCASEMINLENGTH - XLE_PREF_START]) != -1)
        cur_policy->st_pp.pwp.upperminlength = ivalue;
      else
        cur_policy->st_pp.pwp.upperminlength = 
                              prefs->GetPrefDefVal(PWSprefs::PWUppercaseMinLength);

      cur_policy->st_pp.symbols = m_sDefaultSymbols.c_str();

      st_PSWDPolicy currentDB_st_pp;
      if (m_pXMLcore->GetPolicyFromName(cur_policy->name, currentDB_st_pp)) {
        // It already exists in current database
        if (currentDB_st_pp != cur_policy->st_pp) {
          // They are not the same
          m_pXMLcore->MakePolicyUnique(m_mapRenamedPolicies, cur_policy->name,
                              m_sxXML_DateTime, IDSC_IMPORTPOLICY);
          // Now renamed add it
          m_MapPSWDPLC[cur_policy->name] = cur_policy->st_pp;
          m_numRenamedPolicies++;
        }
      } else {
        // Doesn't exist - add
        m_MapPSWDPLC[cur_policy->name] = cur_policy->st_pp;
      }
      delete cur_policy;
      m_bPolicyBeingProcessed = false;
      break;
    }
    case XLE_PWNAME:
      cur_policy->name = m_strElemContent.c_str();
  }
}

void XMLFileHandlers::AddXMLEntries()
{
  // First add any Policy Names imported that are not already in the database
  // This must be done prior to importing entries that may reference them
  if (!m_MapPSWDPLC.empty()) {
    Command *pcmd = DBPolicyNamesCommand::Create(m_pXMLcore, m_MapPSWDPLC,
                            DBPolicyNamesCommand::ADDNEW);
    m_pmulticmds->Add(pcmd);
  }
  
  StringX sxEntriesWithNewNamedPolicies;
  vdb_entries::iterator entry_iter;
  CItemData ci_temp;
  bool bMaintainDateTimeStamps = PWSprefs::GetInstance()->
              GetPref(PWSprefs::MaintainDateTimeStamps);
  bool bIntoEmpty = m_pXMLcore->GetNumEntries() == 0;

  // Initialize sets
  GTUSet setGTU;
  m_pXMLcore->InitialiseGTU(setGTU);
  UUIDSet setUUID;
  m_pXMLcore->InitialiseUUID(setUUID);

  Command *pcmd1 = UpdateGUICommand::Create(m_pXMLcore,
                                            UpdateGUICommand::WN_UNDO,
                                            UpdateGUICommand::GUI_UNDO_IMPORT);
  m_pmulticmds->Add(pcmd1);

  for (entry_iter = m_ventries.begin(); entry_iter != m_ventries.end(); entry_iter++) {
    bool bNoPolicy(false);
    pw_entry *cur_entry = *entry_iter;
    StringX sxtitle(cur_entry->title);
    StringX sxMissingPolicyName;
    EmptyIfOnlyWhiteSpace(sxtitle);
    // Title and Password are mandatory fields!
    if (sxtitle.empty() || cur_entry->password.empty()) {
      stringT cs_error, cs_temp, cs_id, cs_tp, cs_t(_T("")), cs_p(_T(""));
      int num = 0;
      if (sxtitle.empty()) {
        num++;
        cs_t = CItemData::EngFieldName(CItemData::TITLE);
      }
      if (cur_entry->password.empty()) {
        num++;
        cs_p = CItemData::EngFieldName(CItemData::PASSWORD);
      }

      Format(cs_tp, _T("%s%s%s"), cs_t.c_str(), num == 2 ? _T(" & ") : _T(""), cs_p.c_str());
      stringT::iterator new_end = std::remove(cs_tp.begin(), cs_tp.end(), TCHAR('\t'));
      cs_tp.erase(new_end, cs_tp.end());

      LoadAString(cs_id, IDSC_IMPORT_ENTRY_ID);
      Format(cs_temp, IDSC_IMPORTENTRY, cs_id.c_str(), cur_entry->id, 
             cur_entry->group.c_str(), cur_entry->title.c_str(), cur_entry->username.c_str());
      Format(cs_error, IDSC_IMPORTRECSKIPPED, cs_temp.c_str(), cs_tp.c_str());
      m_strSkippedList += cs_error;
      m_numEntriesSkipped++;
      m_numEntries--;
      delete cur_entry;
      continue;
    }
 
    if (m_bImportPSWDsOnly) {
      ItemListIter iter = m_pXMLcore->Find(cur_entry->group, cur_entry->title, cur_entry->username);
      if (iter == m_pXMLcore->GetEntryEndIter()) {
        stringT cs_error, cs_id, cs_temp;
        LoadAString(cs_id, IDSC_IMPORT_ENTRY_ID);
        Format(cs_temp, IDSC_IMPORTENTRY, cs_id.c_str(), cur_entry->id, 
               cur_entry->group.c_str(), cur_entry->title.c_str(), cur_entry->username.c_str());
        Format(cs_error, IDSC_IMPORTRECNOTFOUND, cs_temp.c_str());

        m_strSkippedList += cs_error;
        m_numEntriesSkipped++;
        m_numEntries--;
      } else {
        CItemData *pci = &iter->second;
        Command *pcmd = UpdatePasswordCommand::Create(m_pXMLcore, *pci,
                                                      cur_entry->password);
        pcmd->SetNoGUINotify();
        m_pmulticmds->Add(pcmd);
        if (bMaintainDateTimeStamps) {
          pci->SetATime();
        }
      }
      delete cur_entry;
      continue;
    }

    uuid_array_t ua;
    ci_temp.Clear();
    bool bNewUUID(true);
    if (!cur_entry->uuid.empty()) {
      stringT temp = cur_entry->uuid.c_str();
      // Verify it is the correct length (should be or the schema is wrong!)
      if (temp.length() == sizeof(uuid_array_t) * 2) {
        unsigned int x(0);
        for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
          stringstreamT ss;
          ss.str(temp.substr(i * 2, 2));
          ss >> hex >> x;
          ua[i] = static_cast<unsigned char>(x);
        }
        const CUUID uuid(ua);
        if (uuid != CUUID::NullUUID()) {
          UUIDSetPair pr_uuid = setUUID.insert(uuid);
          if (pr_uuid.second) {
            ci_temp.SetUUID(uuid);
            bNewUUID = false;
          }
        }
      }
    }

    if (bNewUUID) {
      // Need to create new UUID (missing or duplicate in DB or import file)
      // and add to set
      CUUID uuid;
      setUUID.insert(uuid);
      ci_temp.SetUUID(uuid);
    }

    StringX sxnewgroup, sxnewtitle(cur_entry->title);
    if (!m_ImportedPrefix.empty()) {
      sxnewgroup = m_ImportedPrefix.c_str();
      sxnewgroup += _T(".");
    }
    sxnewgroup += cur_entry->group;
    EmptyIfOnlyWhiteSpace(sxnewgroup);
    EmptyIfOnlyWhiteSpace(sxnewtitle);

    bool conflict = !m_pXMLcore->MakeEntryUnique(setGTU,
                                                 sxnewgroup, sxnewtitle,
                                                 cur_entry->username,
                                                 IDSC_IMPORTNUMBER);

    if (conflict) {
      stringT cs_header, cs_error;
      if (cur_entry->group.empty())
        LoadAString(cs_header, IDSC_IMPORTCONFLICTSX2);
      else
        Format(cs_header, IDSC_IMPORTCONFLICTSX1, cur_entry->group.c_str());
      
      Format(cs_error, IDSC_IMPORTCONFLICTS0, cs_header.c_str(),
               cur_entry->title.c_str(), cur_entry->username.c_str(), sxnewtitle.c_str());
      m_strRenameList += cs_error;
      m_numEntriesRenamed++;
    }

    ci_temp.SetGroup(sxnewgroup);

    if (!sxnewtitle.empty())
      ci_temp.SetTitle(sxnewtitle, m_delimiter);

    EmptyIfOnlyWhiteSpace(cur_entry->username);
    if (!cur_entry->username.empty())
      ci_temp.SetUser(cur_entry->username);

    if (!cur_entry->password.empty())
      ci_temp.SetPassword(cur_entry->password);

    EmptyIfOnlyWhiteSpace(cur_entry->url);
    if (!cur_entry->url.empty())
      ci_temp.SetURL(cur_entry->url);

    EmptyIfOnlyWhiteSpace(cur_entry->autotype);
    if (!cur_entry->autotype.empty())
      ci_temp.SetAutoType(cur_entry->autotype);

    if (!cur_entry->ctime.empty())
      ci_temp.SetCTime(cur_entry->ctime.c_str());

    if (!cur_entry->pmtime.empty())
      ci_temp.SetPMTime(cur_entry->pmtime.c_str());

    if (!cur_entry->atime.empty())
      ci_temp.SetATime(cur_entry->atime.c_str());

    if (!cur_entry->xtime.empty())
      ci_temp.SetXTime(cur_entry->xtime.c_str());

    if (!cur_entry->xtime_interval.empty()) {
      int numdays = _ttoi(cur_entry->xtime_interval.c_str());
      if (numdays > 0 && numdays <= 3650)
        ci_temp.SetXTimeInt(numdays);
    }

    if (!cur_entry->rmtime.empty())
      ci_temp.SetRMTime(cur_entry->rmtime.c_str());

    if (!cur_entry->run_command.empty())
      ci_temp.SetRunCommand(cur_entry->run_command);

    if (cur_entry->pwp.flags != 0)
      ci_temp.SetPWPolicy(cur_entry->pwp);

    if (!cur_entry->dca.empty())
      ci_temp.SetDCA(cur_entry->dca.c_str());

    if (!cur_entry->shiftdca.empty())
      ci_temp.SetShiftDCA(cur_entry->shiftdca.c_str());

    if (!cur_entry->email.empty())
      ci_temp.SetEmail(cur_entry->email);

    if (cur_entry->ucprotected)
      ci_temp.SetProtected(cur_entry->ucprotected != 0);

    if (!cur_entry->symbols.empty())
      ci_temp.SetSymbols(cur_entry->symbols);

    if (!cur_entry->policyname.empty()) {
      // First check if it is one we renamed because it exists in the current
      // database but with different settings
      std::map<StringX, StringX>::iterator iter;
      iter = m_mapRenamedPolicies.find(cur_entry->policyname);
      if (iter != m_mapRenamedPolicies.end()) {
        // Yes we did, so use renamed version
        cur_entry->policyname = iter->second;
        StringX sxChanged = L"\r\n\xab" + cur_entry->group + L"\xbb " +
	                        L"\xab" + cur_entry->title + L"\xbb " +
	                        L"\xab" + cur_entry->username + L"\xbb";
        sxEntriesWithNewNamedPolicies += sxChanged;
      } else {
        // No we didn't, verify current database has it
        st_PSWDPolicy currentDB_st_pp;
        if (!m_pXMLcore->GetPolicyFromName(cur_entry->policyname, currentDB_st_pp)) {
          // Not here - make a note and clear the name
          // As we have no information about it's settings we can't even give
          // this entry a specific policy
          sxMissingPolicyName = cur_entry->policyname;
          cur_entry->policyname = _T("");
          m_numNoPolicies++;
          bNoPolicy = true;
        }
      }
      ci_temp.SetPolicyName(cur_entry->policyname);
    }

    StringX newPWHistory;
    stringT strPWHErrorList;

    switch (VerifyXMLImportPWHistoryString(cur_entry->pwhistory,
                                           newPWHistory, strPWHErrorList)) {
      case PWH_OK:
        ci_temp.SetPWHistory(newPWHistory.c_str());
        break;
      case PWH_IGNORE:
        break;
      case PWH_INVALID_HDR:
      case PWH_INVALID_STATUS:
      case PWH_INVALID_NUM:
      case PWH_INVALID_DATETIME:
      case PWH_PSWD_LENGTH_NOTHEX:
      case PWH_INVALID_PSWD_LENGTH: 
      case PWH_INVALID_FIELD_LENGTH:
      {
        stringT buffer;
        Format(buffer, IDSC_SAXERRORPWH, cur_entry->group.c_str(),
               cur_entry->title.c_str(),
               cur_entry->username.c_str());
        m_strPWHErrorList += buffer;
        m_strPWHErrorList += strPWHErrorList;
        m_numEntriesPWHErrors++;
        break;
      }
      default:
        ASSERT(0);
    }

    EmptyIfOnlyWhiteSpace(cur_entry->notes);
    if (!cur_entry->notes.empty())
      ci_temp.SetNotes(cur_entry->notes, m_delimiter);

    // If a potential alias, add to the vector for later verification and processing
    if (cur_entry->entrytype == ALIAS && !cur_entry->bforce_normal_entry) {
      m_pPossible_Aliases->push_back(ci_temp.GetUUID());
    }
    if (cur_entry->entrytype == SHORTCUT && !cur_entry->bforce_normal_entry) {
      m_pPossible_Shortcuts->push_back(ci_temp.GetUUID());
    }

    if (!bIntoEmpty) {
      ci_temp.SetStatus(CItemData::ES_ADDED);
    }

    StringX sx_imported;
    Format(sx_imported, FORMATIMPORTED,
                        cur_entry->group.c_str(), cur_entry->title.c_str(), cur_entry->username.c_str());
    m_prpt->WriteLine(sx_imported.c_str());

    if (bNoPolicy) {
      Format(sx_imported, IDSC_MISSINGPOLICYNAME, sxMissingPolicyName.c_str());
      m_prpt->WriteLine(sx_imported.c_str());
    }

    m_pXMLcore->GUISetupDisplayInfo(ci_temp);
    Command *pcmd = AddEntryCommand::Create(m_pXMLcore, ci_temp);
    pcmd->SetNoGUINotify();
    m_pmulticmds->Add(pcmd);
    delete cur_entry;
  }

  Command *pcmdA = AddDependentEntriesCommand::Create(m_pXMLcore, *m_pPossible_Aliases, m_prpt, 
                                                      CItemData::ET_ALIAS,
                                                      CItemData::PASSWORD);
  pcmdA->SetNoGUINotify();
  m_pmulticmds->Add(pcmdA);
  Command *pcmdS = AddDependentEntriesCommand::Create(m_pXMLcore, *m_pPossible_Shortcuts, m_prpt, 
                                                      CItemData::ET_SHORTCUT,
                                                      CItemData::PASSWORD);
  pcmdS->SetNoGUINotify();
  m_pmulticmds->Add(pcmdS);

  Command *pcmd2 = UpdateGUICommand::Create(m_pXMLcore,
                                            UpdateGUICommand::WN_EXECUTE_REDO,
                                            UpdateGUICommand::GUI_REDO_IMPORT);
  m_pmulticmds->Add(pcmd2);
  
  if (!sxEntriesWithNewNamedPolicies.empty()) {
    StringX sxRenamedPolicies;
    Format(sxRenamedPolicies, IDSC_ENTRIES_POLICIES, m_sxXML_DateTime.c_str(),
                sxEntriesWithNewNamedPolicies.c_str());
    m_prpt->WriteLine();
    m_prpt->WriteLine(sxRenamedPolicies.c_str());
    m_prpt->WriteLine();
  }
}

void XMLFileHandlers::AddDBPreferences()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  int ivalue;
  // Integer/Boolean preferences
  /* Obsolete in 3.18
  if ((ivalue = prefsinXML[XLE_DISPLAYEXPANDEDADDEDITDLG - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::DisplayExpandedAddEditDlg, ivalue == 1);
  */
  if ((ivalue = prefsinXML[XLE_LOCKDBONIDLETIMEOUT - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::LockDBOnIdleTimeout, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_IDLETIMEOUT - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::IdleTimeout, ivalue);
  if ((ivalue = prefsinXML[XLE_MAINTAINDATETIMESTAMPS - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::MaintainDateTimeStamps, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_NUMPWHISTORYDEFAULT - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::NumPWHistoryDefault, ivalue);
  if ((ivalue = prefsinXML[XLE_PWDIGITMINLENGTH - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWDigitMinLength, ivalue);
  if ((ivalue = prefsinXML[XLE_PWLOWERCASEMINLENGTH - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWLowercaseMinLength, ivalue);
  if ((ivalue = prefsinXML[XLE_PWMAKEPRONOUNCEABLE - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWMakePronounceable, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_PWSYMBOLMINLENGTH - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWSymbolMinLength, ivalue);
  if ((ivalue = prefsinXML[XLE_PWUPPERCASEMINLENGTH - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUppercaseMinLength, ivalue);
  if ((ivalue = prefsinXML[XLE_PWDEFAULTLENGTH - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWDefaultLength, ivalue);
  if ((ivalue = prefsinXML[XLE_PWUSEDIGITS - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUseDigits, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_PWUSEEASYVISION - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUseEasyVision, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_PWUSEHEXDIGITS - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUseHexDigits, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_PWUSELOWERCASE - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUseLowercase, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_PWUSESYMBOLS - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUseSymbols, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_PWUSEUPPERCASE - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::PWUseUppercase, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SAVEIMMEDIATELY - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::SaveImmediately, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SAVEPASSWORDHISTORY - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::SavePasswordHistory, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SHOWNOTESDEFAULT - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::ShowNotesDefault, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SHOWPASSWORDINTREE - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::ShowPasswordInTree, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SHOWPWDEFAULT - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::ShowPWDefault, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SHOWUSERNAMEINTREE - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::ShowUsernameInTree, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_SORTASCENDING - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::SortAscending, ivalue == 1);
  if ((ivalue = prefsinXML[XLE_TREEDISPLAYSTATUSATOPEN - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, ivalue);
  if ((ivalue = prefsinXML[XLE_USEDEFAULTUSER - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::UseDefaultUser, ivalue == 1);

  // String preferences
  if (!m_sDefaultUsername.empty())
    prefs->SetPref(PWSprefs::DefaultUsername,
                   m_sDefaultUsername.c_str());
  if (!m_sDefaultAutotypeString.empty())
    prefs->SetPref(PWSprefs::DefaultAutotypeString,
                   m_sDefaultAutotypeString.c_str());
  if (!m_sDefaultSymbols.empty())
    prefs->SetPref(PWSprefs::DefaultSymbols,
                   m_sDefaultSymbols.c_str());
}

#endif /* USE_XML_LIBRARY */
