/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine doesn't do anything as Xerces is a validating XML Parser.
* However, it is present to mimic Expat's version and contains similar data.
*
* Note: Xerces uses wchar_t even in non-Unicode mode.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileDefs.h"

// Xerces validation includes
#include "XFileValidator.h"

// PWS includes
#include "../../StringX.h"

#include <map>
//
//// Xerces includes
//#include <xercesc/util/PlatformUtils.hpp>
//#include <xercesc/sax2/SAX2XMLReader.hpp>
//
//#if defined(XERCES_NEW_IOSTREAMS)
//#include <fstream>
//#else
//#include <fstream.h>
//#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/*
*
* 1. Element Name
* 2. Element Code - non-zero if global
* 3. Element Entry Code - non-zero if within an entry
*
*/

const XFileValidator::st_file_elements XFileValidator::m_file_elements[XLE_ELEMENTS] = {
  {L"passwordsafe", {XLE_PASSWORDSAFE, 0}},
  {L"NumberHashIterations", {XLE_NUMBERHASHITERATIONS, 0}},
  {L"Preferences", {XLE_PREFERENCES, 0}},
  {L"unknownheaderfields", {XLE_UNKNOWNHEADERFIELDS, 0}},
  {L"entry", {XLE_ENTRY, XLE_ENTRY}},
  {L"DisplayExpandedAddEditDlg", {XLE_DISPLAYEXPANDEDADDEDITDLG, 0}},
  {L"MaintainDateTimeStamps", {XLE_MAINTAINDATETIMESTAMPS, 0}},
  {L"PWUseDigits", {XLE_PWUSEDIGITS, XLE_ENTRY_PWUSEDIGITS}},
  {L"PWUseEasyVision", {XLE_PWUSEEASYVISION, XLE_ENTRY_PWUSEEASYVISION}},
  {L"PWUseHexDigits", {XLE_PWUSEHEXDIGITS, XLE_ENTRY_PWUSEHEXDIGITS}},
  {L"PWUseLowercase", {XLE_PWUSELOWERCASE, XLE_ENTRY_PWUSELOWERCASE}},
  {L"PWUseSymbols", {XLE_PWUSESYMBOLS, XLE_ENTRY_PWUSESYMBOLS}},
  {L"PWUseUppercase", {XLE_PWUSEUPPERCASE, XLE_ENTRY_PWUSEUPPERCASE}},
  {L"PWMakePronounceable", {XLE_PWMAKEPRONOUNCEABLE, XLE_ENTRY_PWMAKEPRONOUNCEABLE}},
  {L"SaveImmediately", {XLE_SAVEIMMEDIATELY, 0}},
  {L"SavePasswordHistory", {XLE_SAVEPASSWORDHISTORY, 0}},
  {L"ShowNotesDefault", {XLE_SHOWNOTESDEFAULT, 0}},
  {L"ShowPWDefault", {XLE_SHOWPWDEFAULT, 0}},
  {L"ShowPasswordInTree", {XLE_SHOWPASSWORDINTREE, 0}},
  {L"ShowUsernameInTree", {XLE_SHOWUSERNAMEINTREE, 0}},
  {L"SortAscending", {XLE_SORTASCENDING, 0}},
  {L"UseDefaultUser", {XLE_USEDEFAULTUSER, 0}},
  {L"PWDefaultLength", {XLE_PWDEFAULTLENGTH, 0}},
  {L"IdleTimeout", {XLE_IDLETIMEOUT, 0}},
  {L"TreeDisplayStatusAtOpen", {XLE_TREEDISPLAYSTATUSATOPEN, 0}},
  {L"NumPWHistoryDefault", {XLE_NUMPWHISTORYDEFAULT, 0}},
  {L"PWLowercaseMinLength", {XLE_PWLOWERCASEMINLENGTH, XLE_ENTRY_PWLOWERCASEMINLENGTH}},
  {L"PWUppercaseMinLength", {XLE_PWUPPERCASEMINLENGTH, XLE_ENTRY_PWUPPERCASEMINLENGTH}},
  {L"PWDigitMinLength", {XLE_PWDIGITMINLENGTH, XLE_ENTRY_PWDIGITMINLENGTH}},
  {L"PWSymbolMinLength", {XLE_PWSYMBOLMINLENGTH, XLE_ENTRY_PWSYMBOLMINLENGTH}},
  {L"DefaultUsername", {XLE_DEFAULTUSERNAME, 0}},
  {L"DefaultAutotypeString", {XLE_DEFAULTAUTOTYPESTRING, 0}},
  {L"field", {XLE_HFIELD, XLE_RFIELD}},
  {L"group", {0, XLE_GROUP}},
  {L"title", {0, XLE_TITLE}},
  {L"username", {0, XLE_USERNAME}},
  {L"password", {0, XLE_PASSWORD}},
  {L"url", {0, XLE_URL}},
  {L"autotype", {0, XLE_AUTOTYPE}},
  {L"notes", {0, XLE_NOTES}},
  {L"uuid", {0, XLE_UUID}},
  {L"ctime", {0, XLE_CTIME}},
  {L"atime", {0, XLE_ATIME}},
  {L"ltime", {0, XLE_LTIME}},
  {L"xtime", {0, XLE_XTIME}},
  {L"xtime_interval", {0, XLE_XTIME_INTERVAL}},
  {L"pmtime", {0, XLE_PMTIME}},
  {L"rmtime", {0, XLE_RMTIME}},
  {L"pwhistory", {0, XLE_PWHISTORY}},
  {L"PasswordPolicy", {0, XLE_ENTRY_PASSWORDPOLICY}},
  {L"unknownrecordfields", {0, XLE_UNKNOWNRECORDFIELDS}},
  {L"status", {0, XLE_STATUS}},
  {L"max", {0, XLE_MAX}},
  {L"num", {0, XLE_NUM}},
  {L"history_entries", {0, XLE_HISTORY_ENTRIES}},
  {L"history_entry", {0, XLE_HISTORY_ENTRY}},
  {L"changed", {0, XLE_CHANGED}},
  {L"oldpassword", {0, XLE_OLDPASSWORD}},
  {L"PWLength", {0, XLE_ENTRY_PWLENGTH}},
  {L"date", {0, XLE_DATE}},
  {L"time", {0, XLE_TIME}}
};

XFileValidator::XFileValidator()
{
  for (int i = 0; i < XLE_ELEMENTS; i++) {
    m_element_map.insert(file_element_pair(stringT(m_file_elements[i].name),
                                           m_file_elements[i].file_element_data));
  }
}

XFileValidator::~XFileValidator()
{
  m_element_map.clear();
}

bool XFileValidator::GetElementInfo(const XMLCh *name, st_file_element_data &edata)
{
  const stringT strValue(name);

  if (strValue.length() == 0)
    return false;

  std::map<stringT, st_file_element_data> :: const_iterator e_iter;
  e_iter = m_element_map.find(strValue);
  if (e_iter != m_element_map.end()) {
    edata = e_iter->second;
    return true;
  } else {
    edata.element_code = XLE_LAST_ELEMENT;
    edata.element_entry_code = XLE_LAST_ELEMENT;
    return false;
  }
}

#endif /* USE_XML_LIBRARY == XERCES */