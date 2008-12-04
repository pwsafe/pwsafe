/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.0.0 released on September 29, 2008
*
* See http://xerces.apache.org/xerces-c/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
* To use the static version, the following pre-processor statement
* must be defined: XERCES_STATIC_LIBRARY
*
*/

/*
* NOTE: Xerces characters are ALWAYS in UTF-16 (may or may not be wchar_t
* depending on platform).
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFileSAX2Handlers.h"
#include "XFileValidator.h"

#include "../../corelib.h"
#include "../../PWScore.h"
#include "../../ItemData.h"
#include "../../util.h"
#include "../../UUIDGen.h"
#include "../../PWSfileV3.h"
#include "../../PWSprefs.h"
#include "../../VerifyFormat.h"

// Xerces includes
#include <xercesc/util/XMLString.hpp>

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

using namespace std;

XFileSAX2Handlers::XFileSAX2Handlers()
{
  m_pValidator = new XFileValidator;
  cur_entry = NULL;
  cur_pwhistory_entry = NULL;
  m_strElemContent.clear();

  m_sDefaultAutotypeString = _T("");
  m_sDefaultUsername = _T("");
  m_ImportedPrefix = _T("");
  m_delimiter = _T('^');

  m_bheader = false;
  m_bDatabaseHeaderErrors = false;
  m_bRecordHeaderErrors = false;
  m_bErrors = false;

  m_nITER = MIN_HASH_ITERATIONS;
  m_nRecordsWithUnknownFields = 0;
  m_numEntries = 0;

  // Following are user preferences stored in the database
  for (int i = 0; i < NUMPREFSINXML; i++) {
    prefsinXML[i] = -1;
  }
}

XFileSAX2Handlers::~XFileSAX2Handlers()
{
  m_ukhxl.clear();
  delete m_pValidator;
}

void XFileSAX2Handlers::SetVariables(PWScore *core, const bool &bValidation,
                                    const stringT &ImportedPrefix, const TCHAR &delimiter,
                                    UUIDList *possible_aliases, UUIDList *possible_shortcuts)
{
  m_bValidation = bValidation;
  m_ImportedPrefix = ImportedPrefix;
  m_delimiter = delimiter;
  m_xmlcore = core;
  m_possible_aliases = possible_aliases;
  m_possible_shortcuts = possible_shortcuts;
}

void XFileSAX2Handlers::startDocument( )
{
  m_strImportErrors = _T("");
  m_bentrybeingprocessed = false;
}

void XFileSAX2Handlers::startElement(const XMLCh* const /* uri */,
                                     const XMLCh* const /* localname */,
                                     const XMLCh* const qname,
                                     const Attributes& attrs)
{
  if (m_bValidation) {
    if (XMLString::equals(qname, L"passwordsafe")) {
      // Only interested in the delimiter attribute
      XMLCh *szValue = (XMLCh *)attrs.getValue(L"delimiter");
      if (szValue != NULL) {
#ifdef UNICODE
        m_delimiter = szValue[0];
#else
        char *szDelim = XMLString::transcode(szValue);
        m_delimiter = szDelim[0];
        XMLString::release(&szDelim);
#endif
      }
    }
    return;
  }

  // The rest is only processed in Import mode (not Validation mode)
  m_strElemContent = _T("");

  st_file_element_data edata;
  m_pValidator->GetElementInfo(qname, edata);
  int icurrent_element = m_bentrybeingprocessed ? edata.element_entry_code : edata.element_code;
  switch (icurrent_element) {
    case XLE_PASSWORDSAFE:
      m_bentrybeingprocessed = false;
      break;
    case XLE_UNKNOWNHEADERFIELDS:
      m_ukhxl.clear();
      m_bheader = true;
      break;
    case XLE_HFIELD:
    case XLE_RFIELD:
      {
        // Only interested in the ftype attribute
        XMLCh *szValue = (XMLCh *)attrs.getValue(L"ftype");
        if (szValue != NULL) {
          m_ctype = (unsigned char)_wtoi(szValue);
        }
      }
      break;
    case XLE_ENTRY:
      {
        m_bentrybeingprocessed = true;
        if (m_bValidation)
          return;

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
        cur_entry->entrytype = NORMAL;
        cur_entry->bforce_normal_entry = false;
        m_whichtime = -1;

        // Only interested in the normal attribute
        XMLCh *szValue = (XMLCh *)attrs.getValue(L"normal");
        if (szValue != NULL) {
          cur_entry->bforce_normal_entry =
               XMLString::equals(szValue, L"1") || XMLString::equals(szValue, L"true");
        }
      }
      break;
    case XLE_HISTORY_ENTRY:
      if (m_bValidation)
        return;

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
  return;
}

void XFileSAX2Handlers::characters(const XMLCh* const chars, const XMLSize_t length)
{
  if (m_bValidation)
    return;

  XMLCh *xmlchData = new XMLCh[length + 1];
  XMLString::copyNString(xmlchData, chars, length);
  xmlchData[length] = L'\0';
#ifdef UNICODE
  m_strElemContent += StringX(xmlchData);
#else
  char *szData = XMLString::transcode(xmlchData);
  m_strElemContent += StringX(szData);
  XMLString::release(&szData);
#endif
  delete [] xmlchData;
}

void XFileSAX2Handlers::ignorableWhitespace(const XMLCh* const chars,
                                           const XMLSize_t length)
{
  if (m_bValidation)
    return;

  XMLCh *xmlchData = new XMLCh[length + 1];
  XMLString::copyNString(xmlchData, chars, length);
  xmlchData[length] = L'\0';
#ifdef UNICODE
  m_strElemContent += StringX(xmlchData);
#else
  char *szData = XMLString::transcode(xmlchData);
  m_strElemContent += StringX(szData);
  XMLString::release(&szData);
#endif
  delete [] xmlchData;
}

void XFileSAX2Handlers::endElement(const XMLCh* const /* uri */,
                                   const XMLCh* const /* localname */,
                                   const XMLCh* const qname)
{
  if (m_bValidation) {
    if (XMLString::equals(qname, L"entry"))
      m_numEntries++;
    return;
  }

  StringX buffer(_T(""));

  st_file_element_data edata;
  m_pValidator->GetElementInfo(qname, edata);

  // The rest is only processed in Import mode (not Validation mode)
  int icurrent_element = m_bentrybeingprocessed ? edata.element_entry_code : edata.element_code;
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
      {
        uuid_array_t uuid_array;
        CItemData tempitem;
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
            i = 0;
            stringT s_import;
            do {
              i++;
              Format(s_import, IDSC_IMPORTNUMBER, i);
              Unique_Title = cur_entry->title + s_import.c_str();
              iter = m_xmlcore->Find(newgroup, Unique_Title, cur_entry->username);
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
            assert(0);
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
              /* #ifdef _DEBUG
              stringT cs_timestamp;
              cs_timestamp = PWSUtil::GetTimeStamp();
              TRACE(L"%s: Record %s, %s, %s has unknown field: %02x, length %d/0x%04x, value:\n",
              cs_timestamp, cur_entry->group, cur_entry->title, cur_entry->username,
              unkrfe.uc_Type, (int)unkrfe.st_length, (int)unkrfe.st_length);
              PWSDebug::HexDump(unkrfe.uc_pUField, (int)unkrfe.st_length, cs_timestamp);
              #endif /* DEBUG */
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
        cur_entry->uhrxl.clear();
        delete cur_entry;
        m_numEntries++;
      }
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
/* #ifdef _DEBUG
            stringT cs_timestamp;
            cs_timestamp = PWSUtil::GetTimeStamp();
            TRACE(_T("%s: Header has unknown field: %02x, length %d/0x%04x, value:\n",
            cs_timestamp, m_ctype, m_fieldlen, m_fieldlen);
            PWSDebug::HexDump(m_pfield, m_fieldlen, cs_timestamp);
#endif /* DEBUG */
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

void XFileSAX2Handlers::FormatError(const SAXParseException& e, const int type)
{
  TCHAR szFormatString[MAX_PATH * 2] = {0};
  int iLineNumber, iCharacter;

#ifdef UNICODE
  XMLCh *szErrorMessage = (XMLCh *)e.getMessage();
#else
  char *szErrorMessage = XMLString::transcode(e.getMessage());
#endif
  iLineNumber = (int)e.getLineNumber();
  iCharacter = (int)e.getColumnNumber();

  stringT cs_format, cs_errortype;
  LoadAString(cs_format, IDSC_XERCESSAXGENERROR);
  switch (type) {
    case SAX2_WARNING:
      LoadAString(cs_errortype, IDSC_SAX2WARNING);
      break;
    case SAX2_ERROR:
      LoadAString(cs_errortype, IDSC_SAX2ERROR);
      break;
    case SAX2_FATALERROR:
      LoadAString(cs_errortype, IDSC_SAX2FATALERROR);
      break;
    default:
      assert(0);
  }

#if (_MSC_VER >= 1400)
  _stprintf_s(szFormatString, MAX_PATH * 2, cs_format.c_str(),
              cs_errortype.c_str(), iLineNumber, iCharacter, szErrorMessage);
#else
  _stprintf(szFormatString, cs_format.c_str(), iLineNumber, iCharacter, szErrorMessage);
#endif

  m_strValidationResult += szFormatString;
#ifndef UNICODE
  XMLString::release(&szErrorMessage);
#endif
}

void XFileSAX2Handlers::error(const SAXParseException& e)
{
  FormatError(e, SAX2_ERROR);
  m_bErrors = true;
}

void XFileSAX2Handlers::fatalError(const SAXParseException& e)
{
  FormatError(e, SAX2_FATALERROR);
  m_bErrors = true;
}

void XFileSAX2Handlers::warning(const SAXParseException& e)
{
  FormatError(e, SAX2_WARNING);
}

#endif /* USE_XML_LIBRARY == XERCES */
