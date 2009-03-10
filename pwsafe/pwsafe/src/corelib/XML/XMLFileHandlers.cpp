/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Expat library V2.0.1 released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
*
* NOTE: EXPAT is a NON-validating XML Parser.  All conformity with the
* scheam must be performed in the handlers.  Also, the concept of pre-validation
* before importing is not available.
* As per XML parsing rules, any error stops the parsing immediately.
*/

#include "XMLDefs.h"

#ifdef USE_XML_LIBRARY

#include "XMLFileValidation.h"
#include "XMLFileHandlers.h"

// PWS includes
#include "../corelib.h"
#include "../ItemData.h"
#include "../util.h"
#include "../UUIDGen.h"
#include "../PWSprefs.h"
#include "../PWScore.h"
#include "../PWSfileV3.h"
#include "../VerifyFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

XMLFileHandlers::XMLFileHandlers()
{
  cur_entry = NULL;
  cur_pwhistory_entry = NULL;
  m_strElemContent.clear();

  m_sDefaultAutotypeString = _T("");
  m_sDefaultUsername = _T("");
  m_delimiter = _T('\0');
  m_strErrorMessage = _T("");
  m_iErrorCode = 0;

  m_bheader = false;
  m_bDatabaseHeaderErrors = false;
  m_bRecordHeaderErrors = false;
  m_bErrors = false;

  m_nITER = MIN_HASH_ITERATIONS;
  m_nRecordsWithUnknownFields = 0;

  // Following are user preferences stored in the database
  for (int i = 0; i < NUMPREFSINXML; i++) {
    prefsinXML[i] = -1;
  }
}

XMLFileHandlers::~XMLFileHandlers()
{
  m_ukhxl.clear();
}

void XMLFileHandlers::SetVariables(PWScore *core, const bool &bValidation,
                                   const stringT &ImportedPrefix, const TCHAR &delimiter,
                                   UUIDList *possible_aliases, UUIDList *possible_shortcuts)
{
  m_bValidation = bValidation;
  m_delimiter = delimiter;
  m_xmlcore = core;
  m_possible_aliases = possible_aliases;
  m_possible_shortcuts = possible_shortcuts;
  m_ImportedPrefix = ImportedPrefix;
}

bool XMLFileHandlers::ProcessStartElement(const int icurrent_element)
{
  switch (icurrent_element) {
    case XLE_PASSWORDSAFE:
      m_numEntries = 0;
      m_bentrybeingprocessed = false;
      break;
    case XLE_UNKNOWNHEADERFIELDS:
      m_ukhxl.clear();
      m_bheader = true;
      break;
    case XLE_ENTRY:
      m_bentrybeingprocessed = true;
      if (m_bValidation)
        return false;

      cur_entry = new pw_entry;
      // Clear all fields
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
      cur_entry->execute_string = _T("");
      cur_entry->entrytype = NORMAL;
      cur_entry->bforce_normal_entry = false;
      m_whichtime = -1;
      break;
    case XLE_HISTORY_ENTRY:
      if (m_bValidation)
        return false;

      ASSERT(cur_pwhistory_entry == NULL);
      cur_pwhistory_entry = new pwhistory_entry;
      cur_pwhistory_entry->changed = _T("");
      cur_pwhistory_entry->oldpassword = _T("");
      break;
    case XLE_CTIME:
    case XLE_ATIME:
    case XLE_LTIME:
    case XLE_XTIME:
    case XLE_PMTIME:
    case XLE_RMTIME:
    case XLE_CHANGED:
      m_whichtime = icurrent_element;
      break;
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
    case XLE_UNKNOWNHEADERFIELDS:
      m_bheader = false;
      break;
    case XLE_ENTRY:
      ventries.push_back(cur_entry);
      m_numEntries++;
      break;
    case XLE_DISPLAYEXPANDEDADDEDITDLG:
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
    case XLE_HFIELD:
    case XLE_RFIELD:
      {
        // _stscanf_s always outputs to an "int" using %x even though
        // target is only 1.  Read into larger buffer to prevent data being
        // overwritten and then copy to where we want it!
        const int length = m_strElemContent.length();
        // UNK_HEX_REP will represent unknown values
        // as hexadecimal, rather than base64 encoding.
        // Easier to debug.
#ifndef UNK_HEX_REP
        m_pfield = new unsigned char[(length / 3) * 4 + 4];
        size_t out_len;
        PWSUtil::Base64Decode(m_strElemContent, m_pfield, out_len);
        m_fieldlen = (int)out_len;
#else
        m_fieldlen = length / 2;
        m_pfield = new unsigned char[m_fieldlen + sizeof(int)];
        int nscanned = 0;
        TCHAR *lpsz_string = m_strElemContent.GetBuffer(length);
        for (int i = 0; i < m_fieldlen; i++) {
#if _MSC_VER >= 1400
          nscanned += _stscanf_s(lpsz_string, _T("%02x"), &m_pfield[i]);
#else
          nscanned += _stscanf(lpsz_string, _T("%02x"), &m_pfield[i]);
#endif
          lpsz_string += 2;
        }
        m_strElemContent.ReleaseBuffer();
#endif
        // We will use header field entry and add into proper record field
        // when we create the complete record entry
        UnknownFieldEntry ukxfe(m_ctype, m_fieldlen, m_pfield);
        if (m_bheader) {
          if (m_ctype >= PWSfileV3::HDR_LAST) {
            m_ukhxl.push_back(ukxfe);
#ifdef NOTDEF // _DEBUG
            stringT cs_timestamp;
            cs_timestamp = PWSUtil::GetTimeStamp();
            pws_os::Trace(_T("%s: Header has unknown field: %02x, length %d/0x%04x, value:\n",
            cs_timestamp, m_ctype, m_fieldlen, m_fieldlen);
            pws_os::HexDump(m_pfield, m_fieldlen, cs_timestamp);
#endif /* _DEBUG */
          } else {
            m_bDatabaseHeaderErrors = true;
          }
        } else {
          if (m_ctype >= CItemData::LAST) {
            cur_entry->uhrxl.push_back(ukxfe);
          } else {
            m_bRecordHeaderErrors = true;
          }
        }
        trashMemory(m_pfield, m_fieldlen);
        delete[] m_pfield;
        m_pfield = NULL;
      }
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
    case XLE_CTIME:
      Replace(cur_entry->ctime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLE_ATIME:
      Replace(cur_entry->atime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLE_LTIME:
    case XLE_XTIME:
      Replace(cur_entry->xtime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLE_PMTIME:
      Replace(cur_entry->pmtime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLE_RMTIME:
      Replace(cur_entry->rmtime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLE_XTIME_INTERVAL:
      cur_entry->xtime_interval = Trim(m_strElemContent);
      break;
    case XLE_EXECUTE:
      cur_entry->execute_string = m_strElemContent;
      break;
    case XLE_UNKNOWNRECORDFIELDS:
      if (!cur_entry->uhrxl.empty())
        m_nRecordsWithUnknownFields++;
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
      Format(buffer, _T(" %s %04x %s"),
             cur_pwhistory_entry->changed.c_str(),
             cur_pwhistory_entry->oldpassword.length(),
             cur_pwhistory_entry->oldpassword.c_str());
      cur_entry->pwhistory += buffer;
      delete cur_pwhistory_entry;
      cur_pwhistory_entry = NULL;
      break;
    case XLE_CHANGED:
      ASSERT(cur_pwhistory_entry != NULL);
      Replace(cur_pwhistory_entry->changed, _T('-'), _T('/'));
      Trim(cur_pwhistory_entry->changed);
      if (cur_pwhistory_entry->changed.empty()) {
        //                                 1234567890123456789
        cur_pwhistory_entry->changed = _T("1970-01-01 00:00:00");
      }
      m_whichtime = -1;
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
    case XLE_DATE:
      switch (m_whichtime) {
        case XLE_CTIME:
          cur_entry->ctime = m_strElemContent;
          break;
        case XLE_ATIME:
          cur_entry->atime = m_strElemContent;
          break;
        case XLE_LTIME:
        case XLE_XTIME:
          cur_entry->xtime = m_strElemContent;
          break;
        case XLE_PMTIME:
          cur_entry->pmtime = m_strElemContent;
          break;
        case XLE_RMTIME:
          cur_entry->rmtime = m_strElemContent;
          break;
        case XLE_CHANGED:
          ASSERT(cur_pwhistory_entry != NULL);
          cur_pwhistory_entry->changed = m_strElemContent;
          break;
        default:
          ASSERT(0);
      }
      break;
    case XLE_TIME:
      switch (m_whichtime) {
        case XLE_CTIME:
          cur_entry->ctime += _T(" ") + m_strElemContent;
          break;
        case XLE_ATIME:
          cur_entry->atime += _T(" ") + m_strElemContent;
          break;
        case XLE_LTIME:
        case XLE_XTIME:
          cur_entry->xtime += _T(" ") + m_strElemContent;
          break;
        case XLE_PMTIME:
          cur_entry->pmtime += _T(" ") + m_strElemContent;
          break;
        case XLE_RMTIME:
          cur_entry->rmtime += _T(" ") + m_strElemContent;
          break;
        case XLE_CHANGED:
          ASSERT(cur_pwhistory_entry != NULL);
          cur_pwhistory_entry->changed += _T(" ") + m_strElemContent;
          break;
        default:
          ASSERT(0);
      }
      break;
    case XLE_PASSWORDSAFE:
    case XLE_PREFERENCES:
    case XLE_PWHISTORY:
    case XLE_HISTORY_ENTRIES:
    case XLE_ENTRY_PASSWORDPOLICY:
    default:
      break;
  }
}

void XMLFileHandlers::AddEntries()
{
  vdb_entries::iterator entry_iter;
  CItemData tempitem;

  for (entry_iter = ventries.begin(); entry_iter != ventries.end(); entry_iter++) {
    pw_entry *cur_entry = *entry_iter;
    uuid_array_t uuid_array;
    tempitem.Clear();
    if (cur_entry->uuid.empty())
      tempitem.CreateUUID();
    else {
      // _stscanf_s always outputs to an "int" using %x even though
      // target is only 1.  Read into larger buffer to prevent data being
      // overwritten and then copy to where we want it!
      unsigned char temp_uuid_array[sizeof(uuid_array_t) + sizeof(int)];
      int nscanned = 0;
      const TCHAR *lpszuuid = cur_entry->uuid.c_str();
      for (unsigned i = 0; i < sizeof(uuid_array_t); i++) {
#if _MSC_VER >= 1400
        nscanned += _stscanf_s(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#else
        nscanned += _stscanf(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#endif
        lpszuuid += 2;
      }
      memcpy(uuid_array, temp_uuid_array, sizeof(uuid_array_t));
      if (nscanned != sizeof(uuid_array_t) ||
        m_xmlcore->Find(uuid_array) != m_xmlcore->GetEntryEndIter())
        tempitem.CreateUUID();
      else {
        tempitem.SetUUID(uuid_array);
      }
    }
    StringX newgroup;
    if (!m_ImportedPrefix.empty()) {
      newgroup = m_ImportedPrefix.c_str(); newgroup += _T(".");
    }
    EmptyIfOnlyWhiteSpace(cur_entry->group);
    newgroup += cur_entry->group;
    if (m_xmlcore->Find(newgroup, cur_entry->title,
                        cur_entry->username) != m_xmlcore->GetEntryEndIter()) {
        // Find a unique "Title"
        StringX Unique_Title;
        ItemListConstIter iter;
        int i = 0;
        stringT s_import;
        do {
          i++;
          Format(s_import, IDSC_IMPORTNUMBER, i);
          Unique_Title = cur_entry->title + s_import.c_str();
          iter = m_xmlcore->Find(newgroup, Unique_Title,
                                 cur_entry->username);
        } while (iter != m_xmlcore->GetEntryEndIter());
        cur_entry->title = Unique_Title;
    }
    tempitem.SetGroup(newgroup);
    EmptyIfOnlyWhiteSpace(cur_entry->title);
    if (!cur_entry->title.empty())
      tempitem.SetTitle(cur_entry->title, m_delimiter);
    EmptyIfOnlyWhiteSpace(cur_entry->username);
    if (!cur_entry->username.empty())
      tempitem.SetUser(cur_entry->username);
    if (!cur_entry->password.empty())
      tempitem.SetPassword(cur_entry->password);
    EmptyIfOnlyWhiteSpace(cur_entry->url);
    if (!cur_entry->url.empty())
      tempitem.SetURL(cur_entry->url);
    EmptyIfOnlyWhiteSpace(cur_entry->autotype);
    if (!cur_entry->autotype.empty())
      tempitem.SetAutoType(cur_entry->autotype);
    if (!cur_entry->ctime.empty())
      tempitem.SetCTime(cur_entry->ctime.c_str());
    if (!cur_entry->pmtime.empty())
      tempitem.SetPMTime(cur_entry->pmtime.c_str());
    if (!cur_entry->atime.empty())
      tempitem.SetATime(cur_entry->atime.c_str());
    if (!cur_entry->xtime.empty())
      tempitem.SetXTime(cur_entry->xtime.c_str());
    if (!cur_entry->xtime_interval.empty()) {
      int numdays = _ttoi(cur_entry->xtime_interval.c_str());
      if (numdays > 0 && numdays <= 3650)
        tempitem.SetXTimeInt(numdays);
    }
    if (!cur_entry->rmtime.empty())
      tempitem.SetRMTime(cur_entry->rmtime.c_str());
    if (!cur_entry->execute_string.empty())
      tempitem.SetExecuteString(cur_entry->execute_string);
    if (cur_entry->pwp.flags != 0) {
      tempitem.SetPWPolicy(cur_entry->pwp);
    }

    StringX newPWHistory;
    stringT strPWHErrors;
    switch (VerifyImportPWHistoryString(cur_entry->pwhistory,
                                        newPWHistory, strPWHErrors)) {
      case PWH_OK:
        tempitem.SetPWHistory(newPWHistory.c_str());
        break;
      case PWH_IGNORE:
        break;
      case PWH_INVALID_HDR:
      case PWH_INVALID_STATUS:
      case PWH_INVALID_NUM:
      case PWH_INVALID_DATETIME:
      case PWH_INVALID_PSWD_LENGTH:
      case PWH_TOO_SHORT:
      case PWH_TOO_LONG:
      case PWH_INVALID_CHARACTER:
      {
        stringT buffer;
        Format(buffer, IDSC_SAXERRORPWH, cur_entry->group.c_str(),
               cur_entry->title.c_str(),
               cur_entry->username.c_str());
        m_strImportErrors += buffer;
        m_strImportErrors += strPWHErrors;
        break;
      }
      default:
        ASSERT(0);
    }

    EmptyIfOnlyWhiteSpace(cur_entry->notes);
    if (!cur_entry->notes.empty())
      tempitem.SetNotes(cur_entry->notes, m_delimiter);

    if (!cur_entry->uhrxl.empty()) {
      UnknownFieldList::const_iterator vi_IterUXRFE;
      for (vi_IterUXRFE = cur_entry->uhrxl.begin();
        vi_IterUXRFE != cur_entry->uhrxl.end();
        vi_IterUXRFE++) {
          UnknownFieldEntry unkrfe = *vi_IterUXRFE;
#ifdef NOTDEF // _DEBUG
          stringT cs_timestamp;
          cs_timestamp = PWSUtil::GetTimeStamp();
          pws_os::Trace(_T("%s: Record %s, %s, %s has unknown field: %02x, length %d/0x%04x, value:\n",
          cs_timestamp, cur_entry->group, cur_entry->title, cur_entry->username,
          unkrfe.uc_Type, (int)unkrfe.st_length, (int)unkrfe.st_length);
          pws_os::HexDump(unkrfe.uc_pUField, (int)unkrfe.st_length, cs_timestamp);
#endif /* _DEBUG */
          tempitem.SetUnknownField(unkrfe.uc_Type, (int)unkrfe.st_length, unkrfe.uc_pUField);
      }
    }

    // If a potential alias, add to the vector for later verification and processing
    if (cur_entry->entrytype == ALIAS && !cur_entry->bforce_normal_entry) {
      tempitem.GetUUID(uuid_array);
      m_possible_aliases->push_back(uuid_array);
    }
    if (cur_entry->entrytype == SHORTCUT && !cur_entry->bforce_normal_entry) {
      tempitem.GetUUID(uuid_array);
      m_possible_shortcuts->push_back(uuid_array);
    }

    m_xmlcore->AddEntry(tempitem);
    delete cur_entry;
  }
  ventries.clear();
}

void XMLFileHandlers::AddDBUnknownFieldsPreferences(UnknownFieldList &uhfl)
{
  UnknownFieldList::const_iterator vi_IterUXFE;
  for (vi_IterUXFE = m_ukhxl.begin();
       vi_IterUXFE != m_ukhxl.end();
       vi_IterUXFE++) {
    UnknownFieldEntry ukxfe = *vi_IterUXFE;
    if (ukxfe.st_length > 0) {
      uhfl.push_back(ukxfe);
    }
  }

  PWSprefs *prefs = PWSprefs::GetInstance();
  int ivalue;
  // Integer/Boolean preferences
  if ((ivalue = prefsinXML[XLE_DISPLAYEXPANDEDADDEDITDLG - XLE_PREF_START]) != -1)
    prefs->SetPref(PWSprefs::DisplayExpandedAddEditDlg, ivalue == 1);
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
  if (!m_sDefaultAutotypeString.empty())
    prefs->SetPref(PWSprefs::DefaultAutotypeString,
                   m_sDefaultAutotypeString.c_str());
  if (!m_sDefaultUsername.empty())
    prefs->SetPref(PWSprefs::DefaultUsername,
                   m_sDefaultUsername.c_str());
}

#endif /* USE_XML_LIBRARY */
