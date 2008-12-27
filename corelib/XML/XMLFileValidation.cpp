/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* Common data fields for all File XML import implementations
* This file is included by the pre-processor into the appropriate File
* XML validation routine [E|M|X]FileValidator.cpp
*/

#include "XMLDefs.h"

#ifdef USE_XML_LIBRARY

#include "XMLFileValidation.h"

/*
* Data format:
*   1. Element Name
*   2. Element Code - non-zero if at the database level
*   3. Element Entry Code - non-zero if within an entry
*
* These are entered into a std::map.  The name is the key field and
* the other 2 fields comprise the associated data vias a structure.
*/

const XMLFileValidation::st_file_elements XMLFileValidation::m_file_elements[XLE_ELEMENTS] = {
  {_T("passwordsafe"), {XLE_PASSWORDSAFE, 0}},
  {_T("NumberHashIterations"), {XLE_NUMBERHASHITERATIONS, 0}},
  {_T("Preferences"), {XLE_PREFERENCES, 0}},
  {_T("unknownheaderfields"), {XLE_UNKNOWNHEADERFIELDS, 0}},
  {_T("entry"), {XLE_ENTRY, XLE_ENTRY}},                          // Note: entry must be in both!
  {_T("DisplayExpandedAddEditDlg"), {XLE_DISPLAYEXPANDEDADDEDITDLG, 0}},
  {_T("MaintainDateTimeStamps"), {XLE_MAINTAINDATETIMESTAMPS, 0}},
  {_T("PWUseDigits"), {XLE_PWUSEDIGITS, XLE_ENTRY_PWUSEDIGITS}},
  {_T("PWUseEasyVision"), {XLE_PWUSEEASYVISION, XLE_ENTRY_PWUSEEASYVISION}},
  {_T("PWUseHexDigits"), {XLE_PWUSEHEXDIGITS, XLE_ENTRY_PWUSEHEXDIGITS}},
  {_T("PWUseLowercase"), {XLE_PWUSELOWERCASE, XLE_ENTRY_PWUSELOWERCASE}},
  {_T("PWUseSymbols"), {XLE_PWUSESYMBOLS, XLE_ENTRY_PWUSESYMBOLS}},
  {_T("PWUseUppercase"), {XLE_PWUSEUPPERCASE, XLE_ENTRY_PWUSEUPPERCASE}},
  {_T("PWMakePronounceable"), {XLE_PWMAKEPRONOUNCEABLE, XLE_ENTRY_PWMAKEPRONOUNCEABLE}},
  {_T("SaveImmediately"), {XLE_SAVEIMMEDIATELY, 0}},
  {_T("SavePasswordHistory"), {XLE_SAVEPASSWORDHISTORY, 0}},
  {_T("ShowNotesDefault"), {XLE_SHOWNOTESDEFAULT, 0}},
  {_T("ShowPWDefault"), {XLE_SHOWPWDEFAULT, 0}},
  {_T("ShowPasswordInTree"), {XLE_SHOWPASSWORDINTREE, 0}},
  {_T("ShowUsernameInTree"), {XLE_SHOWUSERNAMEINTREE, 0}},
  {_T("SortAscending"), {XLE_SORTASCENDING, 0}},
  {_T("UseDefaultUser"), {XLE_USEDEFAULTUSER, 0}},
  {_T("PWDefaultLength"), {XLE_PWDEFAULTLENGTH, 0}},
  {_T("IdleTimeout"), {XLE_IDLETIMEOUT, 0}},
  {_T("TreeDisplayStatusAtOpen"), {XLE_TREEDISPLAYSTATUSATOPEN, 0}},
  {_T("NumPWHistoryDefault"), {XLE_NUMPWHISTORYDEFAULT, 0}},
  {_T("PWLowercaseMinLength"), {XLE_PWLOWERCASEMINLENGTH, XLE_ENTRY_PWLOWERCASEMINLENGTH}},
  {_T("PWUppercaseMinLength"), {XLE_PWUPPERCASEMINLENGTH, XLE_ENTRY_PWUPPERCASEMINLENGTH}},
  {_T("PWDigitMinLength"), {XLE_PWDIGITMINLENGTH, XLE_ENTRY_PWDIGITMINLENGTH}},
  {_T("PWSymbolMinLength"), {XLE_PWSYMBOLMINLENGTH, XLE_ENTRY_PWSYMBOLMINLENGTH}},
  {_T("DefaultUsername"), {XLE_DEFAULTUSERNAME, 0}},
  {_T("DefaultAutotypeString"), {XLE_DEFAULTAUTOTYPESTRING, 0}},
  {_T("field"), {XLE_HFIELD, XLE_RFIELD}},
  {_T("group"), {0, XLE_GROUP}},
  {_T("title"), {0, XLE_TITLE}},
  {_T("username"), {0, XLE_USERNAME}},
  {_T("password"), {0, XLE_PASSWORD}},
  {_T("url"), {0, XLE_URL}},
  {_T("autotype"), {0, XLE_AUTOTYPE}},
  {_T("notes"), {0, XLE_NOTES}},
  {_T("uuid"), {0, XLE_UUID}},
  {_T("ctime"), {0, XLE_CTIME}},
  {_T("atime"), {0, XLE_ATIME}},
  {_T("ltime"), {0, XLE_LTIME}},
  {_T("xtime"), {0, XLE_XTIME}},
  {_T("xtime_interval"), {0, XLE_XTIME_INTERVAL}},
  {_T("pmtime"), {0, XLE_PMTIME}},
  {_T("rmtime"), {0, XLE_RMTIME}},
  {_T("pwhistory"), {0, XLE_PWHISTORY}},
  {_T("PasswordPolicy"), {0, XLE_ENTRY_PASSWORDPOLICY}},
  {_T("unknownrecordfields"), {0, XLE_UNKNOWNRECORDFIELDS}},
  {_T("status"), {0, XLE_STATUS}},
  {_T("max"), {0, XLE_MAX}},
  {_T("num"), {0, XLE_NUM}},
  {_T("history_entries"), {0, XLE_HISTORY_ENTRIES}},
  {_T("history_entry"), {0, XLE_HISTORY_ENTRY}},
  {_T("changed"), {0, XLE_CHANGED}},
  {_T("oldpassword"), {0, XLE_OLDPASSWORD}},
  {_T("PWLength"), {0, XLE_ENTRY_PWLENGTH}},
  {_T("date"), {0, XLE_DATE}},
  {_T("time"), {0, XLE_TIME}}
};

XMLFileValidation::XMLFileValidation()
{
  for (int i = 0; i < XLE_ELEMENTS; i++) {
    m_element_map.insert(file_element_pair(stringT(m_file_elements[i].name),
                                           m_file_elements[i].file_element_data));
  }
}

XMLFileValidation::~XMLFileValidation()
{
  m_element_map.clear();
}

#if   USE_XML_LIBRARY == EXPAT
bool XMLFileValidation::GetElementInfo(const XML_Char *name, st_file_element_data &edata)
{
  const stringT strValue(name);
#elif USE_XML_LIBRARY == MSXML
bool XMLFileValidation::GetElementInfo(const wchar_t *name, int numchars, st_file_element_data &edata)
{
#ifdef _UNICODE
  const stringT strValue(name);
  numchars = numchars;  // stop warning : unreferenced formal parameter
#else
#if _MSC_VER >= 1400
  TCHAR* szData = new TCHAR[numchars + 2];
  size_t num_converted;
  wcstombs_s(&num_converted, szData, numchars + 2, name, numchars);
#else
  wcstombs(szData, name, numchars);
#endif
  const stringT strValue(szData);
  delete szData;
#endif
#elif USE_XML_LIBRARY == XERCES
bool XMLFileValidation::GetElementInfo(const XMLCh *name, st_file_element_data &edata)
{
#ifdef UNICODE
  const stringT strValue(name);
#else
  char *szData = XMLString::transcode(name);
  const stringT strValue(szData);
  XMLString::release(&szData);
#endif
#endif

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

#endif /* USE_XML_LIBRARY */