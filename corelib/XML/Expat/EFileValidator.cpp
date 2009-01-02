/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine validates XML when using the Expat library V2.0.1
* released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* NOTE: EXPAT is a NON-validating XML Parser.  All conformity with the
* schema must be performed here in lieu of schema schecking.
*
* As per XML parsing rules, any error stops the parsing immediately.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileValidation.h"

// Expat validation includes
#include "EFileValidator.h"
#include "EFileValidatorDefs.h"

// PWS includes
#include "../../StringX.h"
#include "../../StringXStream.h"
#include "../../VerifyFormat.h"
#include "../../corelib.h"

#include <algorithm>
#include <vector>
#include <map>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

EFileValidator::EFileValidator()
{
  m_elementstack.clear();
  m_sErrorMsg.clear();
  m_iprevious_element = 0;
  m_b_inheader = false;
  m_b_inentry = false;
  m_iErrorCode = 0;

  // Element count variable to ensure that user doesn't specify too many (MaxOccurs)
  for (int i = 0; i < XLE_LAST_ELEMENT; i++) {
    m_ielement_occurs[i] = 0;
  }
}

EFileValidator::~EFileValidator()
{
  m_elementstack.clear();
}

bool EFileValidator::startElement(stringT & strStartElement)
{
  m_iErrorCode = 0;
  if (strStartElement == _T("passwordsafe")) {
    if (!m_elementstack.empty() || m_ielement_occurs[XLE_PASSWORDSAFE] > 0) {
      m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      return false;
    } else {
      m_elementstack.push_back(XLE_PASSWORDSAFE);
      return true;
    }
  }

  if (m_elementstack.empty())
    return false;

  st_file_element_data edata;
  if (!GetElementInfo(strStartElement.c_str(), edata)) {
    m_iErrorCode = XLPEC_UNKNOWN_FIELD;
    Format(m_sErrorMsg, IDSC_EXPATUNKNELEMENT, strStartElement.c_str());
    return false;
  }

  m_iprevious_element = m_elementstack.back();
  int icurrent_element = m_b_inentry ? edata.element_entry_code :
                                       edata.element_code;
  if (icurrent_element == XLE_ENTRY) {
    for (int i = XLE_GROUP; i <= XLE_RFIELD; i++) {
      m_ielement_occurs[i] = 0;
    }
  }

  // Check MaxOccurs
  int maxoccurs;
  if (icurrent_element == XLE_ENTRY ||
      icurrent_element == XLE_HFIELD ||
      icurrent_element == XLE_RFIELD)
    maxoccurs = -1;
  else 
  if (icurrent_element == XLE_HISTORY_ENTRY)
    maxoccurs = 255;
  else
    maxoccurs = 1;
  
  if (maxoccurs != -1 && m_ielement_occurs[icurrent_element] >= maxoccurs) {
    m_iErrorCode = XLPEC_EXCEEDED_MAXOCCURS;
    TCHAR buffer[10];
#if _MSC_VER >= 1400
    _itot_s(maxoccurs, buffer, 10, 10);
#else
    _itot(maxoccurs, buffer, 10);
#endif
    Format(m_sErrorMsg, IDSC_EXPATEXCEEDMAXOCCURS, strStartElement.c_str(), buffer);
    return false;
  } else {
    m_ielement_occurs[icurrent_element]++;
  }

  // Special processing
  switch (icurrent_element) {
    case XLE_ENTRY_PASSWORDPOLICY:
      for (int i = XLE_ENTRY_PWLENGTH; i <= XLE_ENTRY_PWSYMBOLMINLENGTH; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XLE_PREFERENCES:
      for (int i = XLE_DISPLAYEXPANDEDADDEDITDLG; i <= XLE_DEFAULTAUTOTYPESTRING; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XLE_ENTRY:
      m_b_inentry = true;
      // Reset counts
      for (int i = XLE_GROUP; i <= XLE_UNKNOWNRECORDFIELDS; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XLE_CTIME:
    case XLE_ATIME:
    case XLE_LTIME:
    case XLE_XTIME:
    case XLE_PMTIME:
    case XLE_RMTIME:
      m_idatetime_element = icurrent_element;
      for (int i = XLE_DATE; i <= XLE_TIME; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XLE_PWHISTORY:
      for (int i = XLE_STATUS; i <= XLE_HISTORY_ENTRIES; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XLE_HISTORY_ENTRY:
      for (int i = XLE_CHANGED; i <= XLE_OLDPASSWORD; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XLE_CHANGED:
      m_idatetime_element = XLE_CHANGED;
      for (int i = XLE_DATE; i <= XLE_TIME; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    default:
      break;
  }

  switch (icurrent_element) {
    // Main Passwordsafe elements - xs:sequence (in schema defined order)
    case XLE_PASSWORDSAFE:
      break;
    case XLE_NUMBERHASHITERATIONS:
      if (m_iprevious_element != XLE_PASSWORDSAFE ||
          m_ielement_occurs[XLE_PREFERENCES] != 0 ||
          m_ielement_occurs[XLE_UNKNOWNHEADERFIELDS] != 0 ||
          m_ielement_occurs[XLE_ENTRY] != 0) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
        break;
      }
      break;
    case XLE_PREFERENCES:
      if (m_iprevious_element != XLE_PASSWORDSAFE ||
          m_ielement_occurs[XLE_UNKNOWNHEADERFIELDS] != 0 ||
          m_ielement_occurs[XLE_ENTRY] != 0) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XLE_UNKNOWNHEADERFIELDS:
      if (m_iprevious_element != XLE_PASSWORDSAFE ||
          m_ielement_occurs[XLE_ENTRY] != 0) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XLE_ENTRY:
      if (m_iprevious_element != XLE_PASSWORDSAFE) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // Preferences elements - xs:all (any order)
    case XLE_DISPLAYEXPANDEDADDEDITDLG:
    case XLE_MAINTAINDATETIMESTAMPS:
    case XLE_PWUSEDIGITS:
    case XLE_PWUSEEASYVISION:
    case XLE_PWUSEHEXDIGITS:
    case XLE_PWUSELOWERCASE:
    case XLE_PWUSESYMBOLS:
    case XLE_PWUSEUPPERCASE:
    case XLE_PWMAKEPRONOUNCEABLE:
    case XLE_SAVEIMMEDIATELY:
    case XLE_SAVEPASSWORDHISTORY:
    case XLE_SHOWNOTESDEFAULT:
    case XLE_SHOWPWDEFAULT:
    case XLE_SHOWPASSWORDINTREE:
    case XLE_SHOWUSERNAMEINTREE:
    case XLE_SORTASCENDING:
    case XLE_USEDEFAULTUSER:
    case XLE_PWDEFAULTLENGTH:
    case XLE_IDLETIMEOUT:
    case XLE_TREEDISPLAYSTATUSATOPEN:
    case XLE_NUMPWHISTORYDEFAULT:
    case XLE_PWDIGITMINLENGTH:
    case XLE_PWLOWERCASEMINLENGTH:
    case XLE_PWSYMBOLMINLENGTH:
    case XLE_PWUPPERCASEMINLENGTH:
    case XLE_DEFAULTUSERNAME:
    case XLE_DEFAULTAUTOTYPESTRING:
      if (m_iprevious_element != XLE_PREFERENCES) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // unknownheaderfields - xs:all - multiple entries
    case XLE_HFIELD:
      if (m_iprevious_element != XLE_UNKNOWNHEADERFIELDS) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // entry - xs:all (in any order)
    case XLE_GROUP:
    case XLE_TITLE:
    case XLE_USERNAME:
    case XLE_PASSWORD:
    case XLE_URL:
    case XLE_AUTOTYPE:
    case XLE_NOTES:
    case XLE_UUID:
    case XLE_CTIME:
    case XLE_ATIME:
    case XLE_LTIME:
    case XLE_XTIME:
    case XLE_XTIME_INTERVAL:
    case XLE_PMTIME:
    case XLE_RMTIME:
    case XLE_PWHISTORY:
    case XLE_ENTRY_PASSWORDPOLICY:
    case XLE_UNKNOWNRECORDFIELDS:
      if (m_iprevious_element != XLE_ENTRY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // pwhistory - xs:sequence (in schema defined order)
    case XLE_STATUS:
      if (m_iprevious_element != XLE_PWHISTORY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XLE_MAX:
      if (m_iprevious_element != XLE_PWHISTORY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
        break;
      }
    case XLE_NUM:
      if (m_iprevious_element != XLE_PWHISTORY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
        break;
      }
      break;
    case XLE_HISTORY_ENTRIES:
      if (m_iprevious_element != XLE_PWHISTORY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
        break;
      }
      break;

    // history_entries
    case XLE_HISTORY_ENTRY:
      if (m_iprevious_element != XLE_HISTORY_ENTRIES) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // history_entry - xs:all (in any order)
    case XLE_CHANGED:
      if (m_iprevious_element != XLE_HISTORY_ENTRY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XLE_OLDPASSWORD:
      if (m_iprevious_element != XLE_HISTORY_ENTRY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // entry_PasswordPolicy - xs:all (in any order)
    case XLE_ENTRY_PWLENGTH:
    case XLE_ENTRY_PWUSEDIGITS:
    case XLE_ENTRY_PWUSEEASYVISION:
    case XLE_ENTRY_PWUSEHEXDIGITS:
    case XLE_ENTRY_PWUSELOWERCASE:
    case XLE_ENTRY_PWUSESYMBOLS:
    case XLE_ENTRY_PWUSEUPPERCASE:
    case XLE_ENTRY_PWMAKEPRONOUNCEABLE:
    case XLE_ENTRY_PWLOWERCASEMINLENGTH:
    case XLE_ENTRY_PWUPPERCASEMINLENGTH:
    case XLE_ENTRY_PWDIGITMINLENGTH:
    case XLE_ENTRY_PWSYMBOLMINLENGTH:
      if (m_iprevious_element != XLE_ENTRY_PASSWORDPOLICY) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // unknownrecordfields - xs:all - multiple entries
    case XLE_RFIELD:
      if (m_iprevious_element != XLE_UNKNOWNRECORDFIELDS) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
      }
      break;

    // date time fields - xs:sequence (in schema defined order)
    case XLE_DATE:
      // Only allowed within a 'datetime' field
      switch (m_iprevious_element) {
        case XLE_CTIME:
        case XLE_ATIME:
        case XLE_LTIME:
        case XLE_XTIME:
        case XLE_PMTIME:
        case XLE_RMTIME:
        case XLE_CHANGED:
          break;
        default:
          m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
          break;
      }
      break;
    case XLE_TIME:
      // Only allowed within a 'datetime' field
      switch (m_iprevious_element) {
        case XLE_CTIME:
        case XLE_ATIME:
        case XLE_LTIME:
        case XLE_XTIME:
        case XLE_PMTIME:
        case XLE_RMTIME:
        case XLE_CHANGED:
          break;
        default:
          m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
          break;
      }
      // and if the 'date' field has been processed
      if (m_ielement_occurs[XLE_DATE] != 1) {
        m_iErrorCode = XLPEC_UNEXPECTED_ELEMENT;
        break;
      }
      break;
    default:
      ASSERT(0);
      break;
  }

  if (m_iErrorCode != 0) {
    switch (m_iErrorCode) {
      case XLPEC_UNEXPECTED_ELEMENT:
        Format(m_sErrorMsg, IDSC_EXPATUNEXPECTED, strStartElement.c_str());
        break;
      default:
      /*
      case XLPEC_MISSING_MANDATORY_FIELD:
      case XLPEC_EXCEEDED_MAXOCCURS:
      case XLPEC_MISSING_ELEMENT:
      case XLPEC_INVALID_DATA:
      case XLPEC_UNKNOWN_FIELD:
      */
        break;
    }
    return false;
  }

  m_elementstack.push_back(icurrent_element);
  return true;
}


bool EFileValidator::endElement(stringT & endElement, StringX &strValue, int &datatype)
{
  if (endElement == _T("entry")) {
    if (m_ielement_occurs[XLE_TITLE] == 0 || m_ielement_occurs[XLE_PASSWORD] == 0) {
      // missing title and/or password mandatory fields
      m_iErrorCode = XLPEC_MISSING_MANDATORY_FIELD;
      LoadAString(m_sErrorMsg, IDSC_EXPATMISSINGTG);
      return false;
    }
    m_b_inentry = false;
  }

  if (!VerifyXMLDataType(strValue, datatype)) {
    m_iErrorCode = XLPEC_INVALID_DATA;
    Format(m_sErrorMsg, IDSC_EXPATINVALIDDATA, endElement.c_str());
    return false;
  }

  int &icurrent_element = m_elementstack.back();
  switch (icurrent_element) {
    case XLE_CTIME:
    case XLE_ATIME:
    case XLE_LTIME:
    case XLE_XTIME:
    case XLE_PMTIME:
    case XLE_RMTIME:
    case XLE_CHANGED:
      if (m_ielement_occurs[XLE_DATE] != 1 || m_ielement_occurs[XLE_TIME] != 1) {
        m_iErrorCode = XLPEC_MISSING_ELEMENT;
        LoadAString(m_sErrorMsg, IDSC_EXPATMISSINGDT);
        return false;
      }
      break;
    case XLE_PWHISTORY:
      if (m_ielement_occurs[XLE_STATUS] != 1 ||
          m_ielement_occurs[XLE_MAX] != 1 ||
          m_ielement_occurs[XLE_NUM] != 1 ) {
        m_iErrorCode = XLPEC_MISSING_ELEMENT;
        LoadAString(m_sErrorMsg, IDSC_EXPATMISSINGPWH);
        return false;
      }
      break;
    default:
      break;
  }

  m_elementstack.pop_back();
  return true;
}

bool EFileValidator::VerifyXMLDataType(const StringX &strElemContent, const int &datatype)
{
  static const TCHAR *digits(_T("0123456789"));
  static const TCHAR *hexBinary(_T("0123456789abcdefABCDEF"));
  static const TCHAR *base64Binary(_T("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+/"));
  // date = "yyyy-mm-dd"
  // time = "hh:mm:ss"

  const StringX strValue = Trim(strElemContent);
  int i = (int)strValue.length();

  if (i == 0)
    return false;

  switch (datatype) {
    case XLD_XS_BASE64BINARY:
      if (i % 4 != 0)
        return false;
      // Note: a base64Binary string ends with either '....', '...=' or '..=='
      // An equal sign cannot appear anywhere else, so exclude from check as appropriate.
      if (strValue.substr(i - 2, 2) == _T("=="))
        i -= 2;
      else if (strValue[i - 1] == _T('='))
        i--;
      return (strValue.find_first_not_of(base64Binary, 0, i) == StringX::npos);
    case XLD_XS_DATE:
      return VerifyXMLDate(strValue);
    case XLD_XS_HEXBINARY:
      if (i % 2 != 0)
        return false;
      return (strValue.find_first_not_of(hexBinary) == StringX::npos);
    case XLD_XS_INT:
      return (strValue.find_first_not_of(digits) == StringX::npos);
    case XLD_XS_TIME:
      return VerifyXMLTime(strValue);
    case XLD_BOOLTYPE:
      return (strValue == _T("0") || strValue == _T("1"));
    case XLD_CHARACTERTYPE:
      return (i == 1);
    case XLD_DISPLAYSTATUSTYPE:
      return (strValue == _T("AllCollapsed") ||
              strValue == _T("AllExpanded") ||
              strValue == _T("AsPerLastSave"));
    case XLD_EXPIRYDAYSTYPE:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 1 && i <= 3650);
    case XLD_FIELDTYPE:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 18 && i <= 255);
    case XLD_NUMHASHTYPE:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 2048);
    case XLD_PASSWORDLENGTHTYPE:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 4 && i <= 1024);
    case XLD_PASSWORDLENGTHTYPE2:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 0 && i <= 1024);
    case XLD_PWHISTORYTYPE:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 0 && i <= 255);
    case XLD_TIMEOUTTYPE:
      if (strValue.find_first_not_of(digits) != StringX::npos)
        return false;
      i = _ttoi(strValue.c_str());
      return (i >= 1 && i <= 120);
    case XLD_UUIDTYPE:
      if (i != 32)
        return false;
      return (strValue.find_first_not_of(hexBinary) == StringX::npos);
    case XLD_XS_DATETIME:          // defined but fields using this datatype are not used
    case XLD_XS_STRING:            // All elements are strings!
    case XLD_DATETIMESTAMPTYPE:    // defined but fields using this datatype are not used
    case XLD_FILEUUIDTYPE:         // defined but fields using this datatype are not used
    case XLD_NA:                   // N/A - element doesn't have a value in its own right
    default:
      return true;
  }
}

bool EFileValidator::VerifyXMLDate(const StringX &strValue)
{
  // yyyy-mm-dd
  if (strValue == _T("1970-01-01"))  // Special case
    return true;

  const int ndigits = 8;
  const int idigits[ndigits] = {0, 1, 2, 3, 5, 6, 8, 9};
  int yyyy, mm, dd;

  if (strValue.length() != 10)
    return false;

  // Validate strValue
  if (strValue[4] != _T('-') ||
      strValue[7] != _T('-'))
    return false;

  for (int i = 0; i < ndigits; i++) {
    if (!iswdigit(strValue[idigits[i]]))
      return false;
  }

  iStringXStream is(strValue);
  TCHAR dummy;
  is >> yyyy >> dummy >> mm >> dummy >> dd;

  return verifyDTvalues(yyyy, mm, dd, 0, 0, 0);
}


bool EFileValidator::VerifyXMLTime(const StringX &strValue)
{
  // hh:mm:ss
  const int ndigits = 6;
  const int idigits[ndigits] = {0, 1, 3, 4, 6, 7};
  int hh, min, ss;

  if (strValue.length() != 8)
    return false;

  // Validate strValue
  if (strValue[2] != _T(':') ||
      strValue[5] != _T(':'))
    return false;

  for (int i = 0; i < ndigits; i++) {
    if (!iswdigit(strValue[idigits[i]]))
      return false;
  }

  iStringXStream is(strValue);
  TCHAR dummy;
  is >> hh >> dummy >> min >> dummy >> ss;

  return verifyDTvalues(1970, 01, 01, hh, min, ss);
}

StringX EFileValidator::Trim(const StringX &s, const TCHAR *set)
{
  // This version does NOT change the input arguments!
  const TCHAR *tset = (set == NULL) ? _T(" \t\r\n") : set;
  StringX retval(_T(""));

  StringX::size_type b = s.find_first_not_of(tset);
  if (b != StringX::npos) {
    StringX::size_type e = s.find_last_not_of(tset);
    StringX trimmed(s.begin() + b, s.end() - (s.length() - e) + 1);
    retval = trimmed;
  }
  return retval;
}

#endif
