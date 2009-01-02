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

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// PWS includes
#include "EFileHandlers.h"
#include "EFileValidator.h"

#include "../../corelib.h"
#include "../../PWSfileV3.h"
#include "../../UTF8Conv.h"

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
}

EFileHandlers::~EFileHandlers()
{
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

  const int icurrent_element = m_bentrybeingprocessed ? edata.element_entry_code : edata.element_code;
  XMLFileHandlers::ProcessStartElement(icurrent_element);

  switch (icurrent_element) {
    case XLE_PASSWORDSAFE:
      m_element_datatypes.push(XLD_NA);

      // Only interested in the delimiter attribute
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("delimiter")) == 0) {
#ifndef UNICODE
          CUTF8Conv utf8conv;
          int len = strlen(attrs[i + 1]);
          StringX str;
          bool utf8status = utf8conv.FromUTF8((unsigned char *)attrs[i + 1], len, str);
          if (utf8status)
            m_strElemContent = str.c_str();
          else
			      m_strElemContent = attrs[i + 1];
#else
          m_strElemContent = attrs[i + 1];
#endif
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
      break;
    case XLE_UNKNOWNHEADERFIELDS:
      m_element_datatypes.push(XLD_NA);
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
      break;
    case XLE_CTIME:
    case XLE_ATIME:
    case XLE_LTIME:
    case XLE_XTIME:
    case XLE_PMTIME:
    case XLE_RMTIME:
    case XLE_CHANGED:
      m_element_datatypes.push(XLD_NA);
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
    case XLE_ENTRY_PWUSEDIGITS:
    case XLE_ENTRY_PWUSEEASYVISION:
    case XLE_ENTRY_PWUSEHEXDIGITS:
    case XLE_ENTRY_PWUSELOWERCASE:
    case XLE_ENTRY_PWUSESYMBOLS:
    case XLE_ENTRY_PWUSEUPPERCASE:
    case XLE_ENTRY_PWMAKEPRONOUNCEABLE:
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
    case XLE_ENTRY_PWLOWERCASEMINLENGTH:
    case XLE_ENTRY_PWUPPERCASEMINLENGTH:
    case XLE_ENTRY_PWDIGITMINLENGTH:
    case XLE_ENTRY_PWSYMBOLMINLENGTH:
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
  const int icurrent_element = m_bentrybeingprocessed ? edata.element_entry_code : edata.element_code;
  XMLFileHandlers::ProcessEndElement(icurrent_element);

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

#endif /* USE_XML_LIBRARY == EXPAT */
