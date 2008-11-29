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
*
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

  if (_tcscmp(name, _T("passwordsafe")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_strErrorMessage = _T("");
    m_bentrybeingprocessed = false;

    // Only interested in the delimiter attribute
    bool bdelim(false);
    StringX wsDelim;
    for (int i = 0; attrs[i]; i += 2) {
      if (_tcscmp(attrs[i], _T("delimiter")) == 0) {
        wsDelim = attrs[i + 1];
        bdelim = true;
        break;
      }
    }

    if (!bdelim) {
      // error - it is required!
      m_iErrorCode = XLPEC_MISSING_DELIMITER_ATTRIBUTE;
      m_strErrorMessage = stringT(_T("Mandatory delimiter attribute missing."));
      goto start_errors;
    }

    if (!m_pValidator->VerifyXMLDataType(wsDelim, XLD_CHARACTERTYPE)) {
      // Invalid value - single character required
      m_iErrorCode = XLPEC_INVALID_DATA;
      m_strErrorMessage = stringT(_T("Mandatory delimiter attribute invalid."));
      goto start_errors;
    }
    m_delimiter = wsDelim[0];
    return;
  }

  if (_tcscmp(name, _T("unknownheaderfields")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_ukhxl.clear();
    m_bheader = true;
  }

  else if (_tcscmp(name, _T("field")) == 0) {
    m_element_datatypes.push(XLD_XS_BASE64BINARY);
    // Only interested in the ftype attribute
    bool bfield(false);
    for (int i = 0; attrs[i]; i += 2) {
      if (_tcscmp(attrs[i], _T("ftype")) == 0) {
        m_ctype = (unsigned char)_ttoi(attrs[i + 1]);
        bfield = true;
        break;
      }
    }
    if (!bfield) {
      // error - it is required!
      m_iErrorCode = XLPEC_MISSING_ELEMENT;
      m_strErrorMessage = stringT(_T("Mandatory ftype attribute missing."));
      goto start_errors;
    }
  }

  else if (_tcscmp(name, _T("entry")) == 0) {
    m_element_datatypes.push(XLD_NA);
    if (m_bValidation) return;
    cur_entry = new pw_entry;
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
    m_bentrybeingprocessed = true;
    m_whichtime = -1;
    // Only interested in the normal attribute
    bool bnormal(false);
    for (int i = 0; attrs[i]; i += 2) {
      if (_tcscmp(attrs[i], _T("normal")) == 0) {
        cur_entry->bforce_normal_entry =
          (_tcscmp(attrs[i + 1], _T("1")) == 0) ||
          (_tcscmp(attrs[i + 1], _T("true")) == 0);
        bnormal = true;
        break;
      }
    }
  }

  else if (_tcscmp(name, _T("history_entries")) == 0) {
    m_element_datatypes.push(XLD_NA);
  }

  else if (_tcscmp(name, _T("history_entry")) == 0) {
    m_element_datatypes.push(XLD_NA);
    if (m_bValidation) return;
    ASSERT(cur_pwhistory_entry == NULL);
    cur_pwhistory_entry = new pwhistory_entry;
    cur_pwhistory_entry->changed = _T("");
    cur_pwhistory_entry->oldpassword = _T("");
  }

  else if (_tcscmp(name, _T("ctime")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_whichtime = PW_CTIME;
  }

  else if (_tcscmp(name, _T("atime")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_whichtime = PW_ATIME;
  }

  // 'ltime' depreciated but must still be handled for a while!
  else if (_tcscmp(name, _T("ltime")) == 0 ||
           _tcscmp(name, _T("xtime")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_whichtime = PW_XTIME;
  }

  else if (_tcscmp(name, _T("pmtime")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_whichtime = PW_PMTIME;
  }

  else if (_tcscmp(name, _T("rmtime")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_whichtime = PW_RMTIME;
  }

  else if (_tcscmp(name, _T("changed")) == 0) {
    m_element_datatypes.push(XLD_NA);
    m_whichtime = PW_CHANGED;
  }

  else if (_tcscmp(name, _T("group")) == 0 ||
           _tcscmp(name, _T("title")) == 0 ||
           _tcscmp(name, _T("username")) == 0 ||
           _tcscmp(name, _T("password")) == 0 ||
           _tcscmp(name, _T("url")) == 0 ||
           _tcscmp(name, _T("autotype")) == 0 ||
           _tcscmp(name, _T("oldpassword")) == 0 ||
           _tcscmp(name, _T("pwhistory")) == 0 ||
           _tcscmp(name, _T("notes")) == 0 ||
           _tcscmp(name, _T("DefaultUsername")) == 0 ||
           _tcscmp(name, _T("DefaultAutotypeString")) == 0)
    m_element_datatypes.push(XLD_XS_STRING);

  else if (_tcscmp(name, _T("xtime_interval")) == 0)
    m_element_datatypes.push(XLD_EXPIRYDAYSTYPE);

  else if (_tcscmp(name, _T("uuid")) == 0)
    m_element_datatypes.push(XLD_UUIDTYPE);

  else if (_tcscmp(name, _T("date")) == 0)
    m_element_datatypes.push(XLD_XS_DATE);

  else if (_tcscmp(name, _T("time")) == 0)
    m_element_datatypes.push(XLD_XS_TIME);

  else if (_tcscmp(name, _T("NumberHashIterations")) == 0)
    m_element_datatypes.push(XLD_NUMHASHTYPE);

  else if (_tcscmp(name, _T("Preferences")) == 0)
    m_element_datatypes.push(XLD_NA);

  else if (_tcscmp(name, _T("PasswordPolicy")) == 0)
    m_element_datatypes.push(XLD_NA);

  else if (_tcscmp(name, _T("DisplayExpandedAddEditDlg")) == 0 ||
           _tcscmp(name, _T("MaintainDateTimeStamps")) == 0 ||
           _tcscmp(name, _T("PWUseDigits")) == 0 ||
           _tcscmp(name, _T("PWUseEasyVision")) == 0 ||
           _tcscmp(name, _T("PWUseHexDigits")) == 0 ||
           _tcscmp(name, _T("PWUseLowercase")) == 0 ||
           _tcscmp(name, _T("PWUseSymbols")) == 0 ||
           _tcscmp(name, _T("PWUseUppercase")) == 0 ||
           _tcscmp(name, _T("PWMakePronounceable")) == 0 ||
           _tcscmp(name, _T("SaveImmediately")) == 0 ||
           _tcscmp(name, _T("SavePasswordHistory")) == 0 ||
           _tcscmp(name, _T("ShowNotesDefault")) == 0 ||
           _tcscmp(name, _T("ShowPWDefault")) == 0 ||
           _tcscmp(name, _T("ShowPasswordInTree")) == 0 ||
           _tcscmp(name, _T("ShowUsernameInTree")) == 0 ||
           _tcscmp(name, _T("SortAscending")) == 0 ||
           _tcscmp(name, _T("UseDefaultUser")) == 0 ||
           _tcscmp(name, _T("status")) == 0)
    m_element_datatypes.push(XLD_BOOLTYPE);

  else if (_tcscmp(name, _T("PWDefaultLength")) == 0 ||
           _tcscmp(name, _T("PWLength")) == 0)
    m_element_datatypes.push(XLD_PASSWORDLENGTHTYPE);

  else if (_tcscmp(name, _T("IdleTimeout")) == 0)
    m_element_datatypes.push(XLD_TIMEOUTTYPE);

  else if (_tcscmp(name, _T("TreeDisplayStatusAtOpen")) == 0)
    m_element_datatypes.push(XLD_DISPLAYSTATUSTYPE);

  else if (_tcscmp(name, _T("NumPWHistoryDefault")) == 0)
    m_element_datatypes.push(XLD_PWHISTORYTYPE);

  else if (_tcscmp(name, _T("PWDigitMinLength")) == 0 ||
           _tcscmp(name, _T("PWLowercaseMinLength")) == 0 ||
           _tcscmp(name, _T("PWSymbolMinLength")) == 0 ||
           _tcscmp(name, _T("PWUppercaseMinLength")) == 0)
    m_element_datatypes.push(XLD_PASSWORDLENGTHTYPE2);

  else if (_tcscmp(name, _T("max")) == 0 ||
           _tcscmp(name, _T("num")) == 0)
    m_element_datatypes.push(XLD_PWHISTORYTYPE);

  else
    ASSERT(0);

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
  xmlchData[length] = L'\0';
  m_strElemContent += StringX(xmlchData);
  delete [] xmlchData;
}

void XMLCALL EFileHandlers::endElement(void * userdata, const XML_Char *name)
{
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

  // The rest is only processed in Import mode (not Validation mode)
  if (_tcscmp(name, _T("entry")) == 0) {
    m_numEntries++;
    ventries.push_back(cur_entry);
    return;
  }

  else if (_tcscmp(name, _T("group")) == 0) {
    cur_entry->group = m_strElemContent;
  }

  else if (_tcscmp(name, _T("title")) == 0) {
    cur_entry->title = m_strElemContent;
  }

  else if (_tcscmp(name, _T("username")) == 0) {
    cur_entry->username = m_strElemContent;
  }

  else if (_tcscmp(name, _T("password")) == 0) {
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
  }

  else if (_tcscmp(name, _T("url")) == 0) {
    cur_entry->url = m_strElemContent;
  }

  else if (_tcscmp(name, _T("autotype")) == 0) {
    cur_entry->autotype = m_strElemContent;
  }

  else if (_tcscmp(name, _T("notes")) == 0) {
    cur_entry->notes = m_strElemContent;
  }

  else if (_tcscmp(name, _T("uuid")) == 0) {
    cur_entry->uuid = m_strElemContent;
  }

  else if (_tcscmp(name, _T("status")) == 0) {
    StringX buffer;
    int i = _ttoi(m_strElemContent.c_str());
    Format(buffer, _T("%01x"), i);
    cur_entry->pwhistory = buffer;
  }

  else if (_tcscmp(name, _T("max")) == 0) {
    StringX buffer;
    int i = _ttoi(m_strElemContent.c_str());
    Format(buffer, _T("%02x"), i);
    cur_entry->pwhistory += buffer;
  }

  else if (_tcscmp(name, _T("num")) == 0) {
    StringX buffer;
    int i = _ttoi(m_strElemContent.c_str());
    Format(buffer, _T("%02x"), i);
    cur_entry->pwhistory += buffer;
  }

  else if (_tcscmp(name, _T("ctime")) == 0) {
    Replace(cur_entry->ctime, _T('-'), _T('/'));
    m_whichtime = -1;
  }

  else if (_tcscmp(name, _T("pmtime")) == 0) {
    Replace(cur_entry->pmtime, _T('-'), _T('/'));
    m_whichtime = -1;
  }

  else if (_tcscmp(name, _T("atime")) == 0) {
    Replace(cur_entry->atime, _T('-'), _T('/'));
    m_whichtime = -1;
  }

  else if (_tcscmp(name, _T("xtime")) == 0) {
    Replace(cur_entry->xtime, _T('-'), _T('/'));
    m_whichtime = -1;
  }

  else if (_tcscmp(name, _T("rmtime")) == 0) {
    Replace(cur_entry->rmtime, _T('-'), _T('/'));
    m_whichtime = -1;
  }

  else if (_tcscmp(name, _T("changed")) == 0) {
    ASSERT(cur_pwhistory_entry != NULL);
    Replace(cur_pwhistory_entry->changed, _T('-'), _T('/'));
    Trim(cur_pwhistory_entry->changed);
    if (cur_pwhistory_entry->changed.empty()) {
      //                               1234567890123456789
      cur_pwhistory_entry->changed = _T("1970-01-01 00:00:00");
    }
    m_whichtime = -1;
  }

  else if (_tcscmp(name, _T("oldpassword")) == 0) {
    ASSERT(cur_pwhistory_entry != NULL);
    cur_pwhistory_entry->oldpassword = m_strElemContent;
  }

  else if (_tcscmp(name, _T("history_entries")) == 0) {
  }

  else if (_tcscmp(name, _T("history_entry")) == 0) {
    ASSERT(cur_pwhistory_entry != NULL);
    StringX buffer;
    Format(buffer, _T(" %s %04x %s"),
           cur_pwhistory_entry->changed.c_str(),
           cur_pwhistory_entry->oldpassword.length(),
           cur_pwhistory_entry->oldpassword.c_str());
    cur_entry->pwhistory += buffer;
    delete cur_pwhistory_entry;
    cur_pwhistory_entry = NULL;
  }

  else if (_tcscmp(name, _T("date")) == 0 && !m_strElemContent.empty()) {
    switch (m_whichtime) {
      case PW_CTIME:
        cur_entry->ctime = m_strElemContent;
        break;
      case PW_PMTIME:
        cur_entry->pmtime = m_strElemContent;
        break;
      case PW_ATIME:
        cur_entry->atime = m_strElemContent;
        break;
      case PW_XTIME:
        cur_entry->xtime = m_strElemContent;
        break;
      case PW_RMTIME:
        cur_entry->rmtime = m_strElemContent;
        break;
      case PW_CHANGED:
        ASSERT(cur_pwhistory_entry != NULL);
        cur_pwhistory_entry->changed = m_strElemContent;
        break;
      default:
        ASSERT(0);
    }
  }

  else if (_tcscmp(name, _T("time")) == 0 && !m_strElemContent.empty()) {
    switch (m_whichtime) {
      case PW_CTIME:
        cur_entry->ctime += _T(" ") + m_strElemContent;
        break;
      case PW_PMTIME:
        cur_entry->pmtime += _T(" ") + m_strElemContent;
        break;
      case PW_ATIME:
        cur_entry->atime += _T(" ") + m_strElemContent;
        break;
      case PW_XTIME:
        cur_entry->xtime += _T(" ") + m_strElemContent;
        break;
      case PW_RMTIME:
        cur_entry->rmtime += _T(" ") + m_strElemContent;
        break;
      case PW_CHANGED:
        ASSERT(cur_pwhistory_entry != NULL);
        cur_pwhistory_entry->changed += _T(" ") + m_strElemContent;
        break;
      default:
        ASSERT(0);
    }
  }

  else if (_tcscmp(name, _T("xtime_interval")) == 0 && !m_strElemContent.empty()) {
    cur_entry->xtime_interval = Trim(m_strElemContent);
  }

  else if (_tcscmp(name, _T("unknownheaderfields")) == 0) {
    m_bheader = false;
  }

  else if (_tcscmp(name, _T("unknownrecordfields")) == 0) {
    if (!cur_entry->uhrxl.empty())
      m_nRecordsWithUnknownFields++;
  }

  else if (_tcscmp(name, _T("field")) == 0) {
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

  else if (_tcscmp(name, _T("NumberHashIterations")) == 0) {
    int i = _ttoi(m_strElemContent.c_str());
    if (i > MIN_HASH_ITERATIONS) {
      m_nITER = i;
    }
  }

  else if (_tcscmp(name, _T("DisplayExpandedAddEditDlg")) == 0) {
    prefsinXML[XLP_BDISPLAYEXPANDEDADDEDITDLG] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("MaintainDateTimeStamps")) == 0) {
    prefsinXML[XLP_BMAINTAINDATETIMESTAMPS] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUseDigits")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseDigits;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseDigits;
    } else
      prefsinXML[XLP_BPWUSEDIGITS] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUseEasyVision")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseEasyVision;
    } else
      prefsinXML[XLP_BPWUSEEASYVISION] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUseHexDigits")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseHexDigits;
    } else
      prefsinXML[XLP_BPWUSEHEXDIGITS] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUseLowercase")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseLowercase;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseLowercase;
    } else
      prefsinXML[XLP_BPWUSELOWERCASE] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUseSymbols")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseSymbols;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseSymbols;
    } else
      prefsinXML[XLP_BPWUSESYMBOLS] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUseUppercase")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyUseUppercase;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyUseUppercase;
    } else
      prefsinXML[XLP_BPWUSEUPPERCASE] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWMakePronounceable")) == 0) {
    if (m_bentrybeingprocessed) {
      if (m_strElemContent == _T("1"))
        cur_entry->pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
      else
        cur_entry->pwp.flags &= ~PWSprefs::PWPolicyMakePronounceable;
    } else
      prefsinXML[XLP_BPWMAKEPRONOUNCEABLE] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("SaveImmediately")) == 0) {
    prefsinXML[XLP_BSAVEIMMEDIATELY] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("SavePasswordHistory")) == 0) {
    prefsinXML[XLP_BSAVEPASSWORDHISTORY] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("ShowNotesDefault")) == 0) {
    prefsinXML[XLP_BSHOWNOTESDEFAULT] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("ShowPWDefault")) == 0) {
    prefsinXML[XLP_BSHOWPWDEFAULT] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("ShowPasswordInTree")) == 0) {
    prefsinXML[XLP_BSHOWPASSWORDINTREE] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("ShowUsernameInTree")) == 0) {
    prefsinXML[XLP_BSHOWUSERNAMEINTREE] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("SortAscending")) == 0) {
    prefsinXML[XLP_BSORTASCENDING] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("UseDefaultUser")) == 0) {
    prefsinXML[XLP_BUSEDEFAULTUSER] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWDefaultLength")) == 0) {
    prefsinXML[XLP_IPWDEFAULTLENGTH] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("IdleTimeout")) == 0) {
    prefsinXML[XLP_IIDLETIMEOUT] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("TreeDisplayStatusAtOpen")) == 0) {
    if (m_strElemContent == _T("AllCollapsed"))
      prefsinXML[XLP_ITREEDISPLAYSTATUSATOPEN] = PWSprefs::AllCollapsed;
    else if (m_strElemContent == _T("AllExpanded"))
      prefsinXML[XLP_ITREEDISPLAYSTATUSATOPEN] = PWSprefs::AllExpanded;
    else if (m_strElemContent == _T("AsPerLastSave"))
      prefsinXML[XLP_ITREEDISPLAYSTATUSATOPEN] = PWSprefs::AsPerLastSave;
  }

  else if (_tcscmp(name, _T("NumPWHistoryDefault")) == 0) {
    prefsinXML[XLP_INUMPWHISTORYDEFAULT] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("DefaultUsername")) == 0) {
    m_sDefaultUsername = m_strElemContent.c_str();
  }

  else if (_tcscmp(name, _T("DefaultAutotypeString")) == 0) {
    m_sDefaultAutotypeString = m_strElemContent.c_str();
  }

  else if (_tcscmp(name, _T("PWLength")) == 0) {
    cur_entry->pwp.length = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWDigitMinLength")) == 0) {
    if (m_bentrybeingprocessed)
      cur_entry->pwp.digitminlength = _ttoi(m_strElemContent.c_str());
    else
      prefsinXML[XLP_IPWDIGITMINLENGTH] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWLowercaseMinLength")) == 0) {
    if (m_bentrybeingprocessed)
      cur_entry->pwp.lowerminlength = _ttoi(m_strElemContent.c_str());
    else
      prefsinXML[XLP_IPWLOWERCASEMINLENGTH] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWSymbolMinLength")) == 0) {
    if (m_bentrybeingprocessed)
      cur_entry->pwp.symbolminlength = _ttoi(m_strElemContent.c_str());
    else
      prefsinXML[XLP_IPWSYMBOLMINLENGTH] = _ttoi(m_strElemContent.c_str());
  }

  else if (_tcscmp(name, _T("PWUppercaseMinLength")) == 0) {
    if (m_bentrybeingprocessed)
      cur_entry->pwp.upperminlength = _ttoi(m_strElemContent.c_str());
    else
      prefsinXML[XLP_IPWUPPERCASEMINLENGTH] = _ttoi(m_strElemContent.c_str());
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
