/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// PWS includes
#include "EFileHandlers.h"
#include "EFileValidator.h"

#include "../../corelib.h"
#include "../../PWScore.h"
#include "../../ItemData.h"
#include "../../util.h"
#include "../../UUIDGen.h"
#include "../../PWSfileV3.h"
#include "../../PWSprefs.h"

// Expat includes
#include <expat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

EFileHandlers::EFileHandlers()
{
  m_pValidator = new EFileValidator;
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

EFileHandlers::~EFileHandlers()
{
  m_ukhxl.clear();
  delete m_pValidator;
}

void XMLCALL EFileHandlers::startElement(void *userdata, const XML_Char *name,
                                         const XML_Char **attrs)
{
  bool battr_found(false);
  m_strElemContent = _T("");

  if (m_bValidation) {
    stringT element_name(name);
    if (!m_pValidator->startElement(element_name)) {
      m_bErrors = true;
      m_iErrorCode = m_pValidator->getErrorCode();
      m_strErrorMessage = m_pValidator->getErrorMsg();
      goto start_errors;
    }
  }

  st_file_element_data edata;
  if (!m_pValidator->GetElementInfo(name, edata))
    goto start_errors;

  int icurrent_element = m_bentrybeingprocessed ? edata.element_entry_code : edata.element_code;
  switch (icurrent_element) {
    case XLE_PASSWORDSAFE:
      m_element_datatypes.push(XLD_NA);
      m_strErrorMessage = _T("");
      m_bentrybeingprocessed = false;

      // Only interested in the delimiter attribute
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("delimiter")) == 0) {
          m_strElemContent = attrs[i + 1];
          battr_found = true;
          break;
        }
      }

      if (!battr_found) {
        // error - it is required!
        m_iErrorCode = XLPEC_MISSING_DELIMITER_ATTRIBUTE;
        LoadAString(m_strErrorMessage, IDSC_EXPATNODELIMITER);
        goto start_errors;
      }

      if (!m_pValidator->VerifyXMLDataType(m_strElemContent, XLD_CHARACTERTYPE)) {
        // Invalid value - single character required
        m_iErrorCode = XLPEC_INVALID_DATA;
        LoadAString(m_strErrorMessage, IDSC_EXPATBADDELIMITER);
        goto start_errors;
      }
      m_delimiter = m_strElemContent[0];
      m_strElemContent = _T("");
      break;
    case XLE_UNKNOWNHEADERFIELDS:
      m_element_datatypes.push(XLD_NA);
      m_ukhxl.clear();
      m_bheader = true;
      break;
    case XLE_HFIELD:
    case XLE_RFIELD:
      m_element_datatypes.push(XLD_XS_BASE64BINARY);
      // Only interested in the ftype attribute
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("ftype")) == 0) {
          m_ctype = (unsigned char)_ttoi(attrs[i + 1]);
          battr_found = true;
          break;
        }
      }
      if (!battr_found) {
        // error - it is required!
        m_iErrorCode = XLPEC_MISSING_ELEMENT;
        LoadAString(m_strErrorMessage, IDSC_EXPATFTYPEMISSING);
        goto start_errors;
      }
      break;
    case XLE_ENTRY:
      m_element_datatypes.push(XLD_NA);
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
      cur_entry->changed = _T("");
      cur_entry->pwhistory = _T("");
      cur_entry->notes = _T("");
      cur_entry->uuid = _T("");
      cur_entry->pwp.Empty();
      cur_entry->entrytype = NORMAL;
      cur_entry->bforce_normal_entry = false;
      m_bentrybeingprocessed = true;
      m_whichtime = -1;

      // Only interested in the normal attribute
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("normal")) == 0) {
          cur_entry->bforce_normal_entry =
            (_tcscmp(attrs[i + 1], _T("1")) == 0) ||
            (_tcscmp(attrs[i + 1], _T("true")) == 0);
          break;
        }
      }
      break;
    case XLE_HISTORY_ENTRIES:
    case XLE_PREFERENCES:
    case XLE_ENTRY_PASSWORDPOLICY:
      m_element_datatypes.push(XLD_NA);
      break;
    case XLE_HISTORY_ENTRY:
      m_element_datatypes.push(XLD_NA);
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
      m_element_datatypes.push(XLD_NA);
      m_whichtime = icurrent_element;
      break;
    case XLE_GROUP:
    case XLE_TITLE:
    case XLE_USERNAME:
    case XLE_PASSWORD:
    case XLE_URL:
    case XLE_AUTOTYPE:
    case XLE_NOTES:
    case XLE_OLDPASSWORD:
    case XLE_PWHISTORY:
    case XLE_DEFAULTUSERNAME:
    case XLE_DEFAULTAUTOTYPESTRING:
      m_element_datatypes.push(XLD_XS_STRING);
      break;
    case XLE_XTIME_INTERVAL:
      m_element_datatypes.push(XLD_EXPIRYDAYSTYPE);
      break;
    case XLE_UUID:
      m_element_datatypes.push(XLD_UUIDTYPE);
      break;
    case XLE_DATE:
      m_element_datatypes.push(XLD_XS_DATE);
      break;
    case XLE_TIME:
      m_element_datatypes.push(XLD_XS_TIME);
      break;
    case XLE_NUMBERHASHITERATIONS:
      m_element_datatypes.push(XLD_NUMHASHTYPE);
      break;
    case XLE_DISPLAYEXPANDEDADDEDITDLG:
    case XLE_MAINTAINDATETIMESTAMPS:
    case XLE_PWMAKEPRONOUNCEABLE:
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
    case XLE_STATUS:
      m_element_datatypes.push(XLD_BOOLTYPE);
      break;
    case XLE_PWDEFAULTLENGTH:
    case XLE_ENTRY_PWLENGTH:
      m_element_datatypes.push(XLD_PASSWORDLENGTHTYPE);
      break;
    case XLE_IDLETIMEOUT:
      m_element_datatypes.push(XLD_TIMEOUTTYPE);
      break;
    case XLE_TREEDISPLAYSTATUSATOPEN:
      m_element_datatypes.push(XLD_DISPLAYSTATUSTYPE);
      break;
    case XLE_NUMPWHISTORYDEFAULT:
      m_element_datatypes.push(XLD_PWHISTORYTYPE);
      break;
    case XLE_PWDIGITMINLENGTH:
    case XLE_PWLOWERCASEMINLENGTH:
    case XLE_PWSYMBOLMINLENGTH:
    case XLE_PWUPPERCASEMINLENGTH:
      m_element_datatypes.push(XLD_PASSWORDLENGTHTYPE2);
      break;
    case XLE_MAX:
    case XLE_NUM:
      m_element_datatypes.push(XLD_PWHISTORYTYPE);
      break;
    default:
      ASSERT(0);
  }
  return;

start_errors:
  // Non validating parser, so we have to tidy up now
  vdb_entries::iterator entry_iter;

  for (entry_iter = ventries.begin(); entry_iter != ventries.end(); entry_iter++) {
    pw_entry *cur_entry = *entry_iter;
    delete cur_entry;
  }

  ventries.clear();
  if (cur_entry) {
    cur_entry->uhrxl.clear();
    delete cur_entry;
    cur_entry = NULL;
  }
  XML_StopParser((XML_Parser)userdata, XML_FALSE);
}

void XMLCALL EFileHandlers::characterData(void * /* userdata */, const XML_Char *s,
                                          int length)
{
  XML_Char *xmlchData = new XML_Char[length + 1];
#if _MSC_VER >= 1400
  _tcsncpy_s(xmlchData, length + 1, s, length);
#else
  _tcsncpy(xmlchData, s, length);
#endif
  xmlchData[length] = TCHAR('\0');
  m_strElemContent += StringX(xmlchData);
  delete [] xmlchData;
}

void XMLCALL EFileHandlers::endElement(void * userdata, const XML_Char *name)
{
  StringX buffer(_T(""));

  if (m_bValidation) {
    int &element_datatype = m_element_datatypes.top();
    stringT element_name(name);
    if (!m_pValidator->endElement(element_name, m_strElemContent, element_datatype)) {
      m_bErrors = true;
      m_iErrorCode = m_pValidator->getErrorCode();
      m_strErrorMessage = m_pValidator->getErrorMsg();
      goto end_errors;
    }
  }

  m_element_datatypes.pop();

  if (m_bValidation) {
    if (_tcscmp(name, _T("entry")) == 0)
      m_numEntries++;
    return;
  }

  st_file_element_data edata;
  if (!m_pValidator->GetElementInfo(name, edata))
    goto end_errors;

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
      m_numEntries++;
      ventries.push_back(cur_entry);
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
    case XLE_PWHISTORY:
      cur_entry->pwhistory = m_strElemContent;
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
    case XLE_ENTRY_PWUPPERCASEMINLENGTH:
      cur_entry->pwp.symbolminlength = _ttoi(m_strElemContent.c_str());
      break;
    case XLE_ENTRY_PWSYMBOLMINLENGTH:
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
    case XLE_HISTORY_ENTRIES:
    case XLE_ENTRY_PASSWORDPOLICY:
    default:
      break;
  }

  return;

end_errors:
  // Non validating parser, so we have to tidy up now
  vdb_entries::iterator entry_iter;

  for (entry_iter = ventries.begin(); entry_iter != ventries.end(); entry_iter++) {
    pw_entry *cur_entry = *entry_iter;
    delete cur_entry;
  }

  ventries.clear();
  if (cur_entry) {
    cur_entry->uhrxl.clear();
    delete cur_entry;
    cur_entry = NULL;
  }
  XML_StopParser((XML_Parser)userdata, XML_FALSE);
}

#endif /* USE_XML_LIBRARY == EXPATS */
