/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#include "XMLDefs.h"   // Required if testing "USE_XML_LIBRARY"

#ifdef USE_XML_LIBRARY

#include "XMLFileValidation.h"

#if USE_XML_LIBRARY == XERCES
#include "./Xerces/XMLChConverter.h"
#endif

/*
* Data format:
*   1. Element Name
*   2. Element Code - non-zero if at the database level
*   3. Element Entry Code - non-zero if within an entry
*
* These are entered into a std::map.  The name is the key field and
* the other 2 fields comprise the associated data via a structure.
*/

const XMLFileValidation::st_file_elements XMLFileValidation::m_file_elements[XLE_ELEMENTS] = {
  {_T("passwordsafe"), {XLE_PASSWORDSAFE, 0}},
  {_T("Preferences"), {XLE_PREFERENCES, 0}},
  {_T("entry"), {XLE_ENTRY, XLE_ENTRY}},                          // Note: entry must be in both!
  {_T("MaintainDateTimeStamps"), {XLE_PREF_MAINTAINDATETIMESTAMPS, 0}},
  {_T("PWUseDigits"), {XLE_PREF_PWUSEDIGITS, XLE_ENTRY_PWUSEDIGITS}},
  {_T("PWUseEasyVision"), {XLE_PREF_PWUSEEASYVISION, XLE_ENTRY_PWUSEEASYVISION}},
  {_T("PWUseHexDigits"), {XLE_PREF_PWUSEHEXDIGITS, XLE_ENTRY_PWUSEHEXDIGITS}},
  {_T("PWUseLowercase"), {XLE_PREF_PWUSELOWERCASE, XLE_ENTRY_PWUSELOWERCASE}},
  {_T("PWUseSymbols"), {XLE_PREF_PWUSESYMBOLS, XLE_ENTRY_PWUSESYMBOLS}},
  {_T("PWUseUppercase"), {XLE_PREF_PWUSEUPPERCASE, XLE_ENTRY_PWUSEUPPERCASE}},
  {_T("PWMakePronounceable"), {XLE_PREF_PWMAKEPRONOUNCEABLE, XLE_ENTRY_PWMAKEPRONOUNCEABLE}},
  {_T("SaveImmediately"), {XLE_PREF_SAVEIMMEDIATELY, 0}},
  {_T("SavePasswordHistory"), {XLE_PREF_SAVEPASSWORDHISTORY, 0}},
  {_T("ShowNotesDefault"), {XLE_PREF_SHOWNOTESDEFAULT, 0}},
  {_T("ShowPWDefault"), {XLE_PREF_SHOWPWDEFAULT, 0}},
  {_T("ShowPasswordInTree"), {XLE_PREF_SHOWPASSWORDINTREE, 0}},
  {_T("ShowUsernameInTree"), {XLE_PREF_SHOWUSERNAMEINTREE, 0}},
  {_T("SortAscending"), {XLE_PREF_SORTASCENDING, 0}},
  {_T("UseDefaultUser"), {XLE_PREF_USEDEFAULTUSER, 0}},
  {_T("PWDefaultLength"), {XLE_PREF_PWDEFAULTLENGTH, 0}},
  {_T("LockDBOnIdleTimeout"), {XLE_PREF_LOCKDBONIDLETIMEOUT, 0}},
  {_T("IdleTimeout"), {XLE_PREF_IDLETIMEOUT, 0}},
  {_T("TreeDisplayStatusAtOpen"), {XLE_PREF_TREEDISPLAYSTATUSATOPEN, 0}},
  {_T("NumPWHistoryDefault"), {XLE_PREF_NUMPWHISTORYDEFAULT, 0}},
  {_T("PWLowercaseMinLength"), {XLE_PREF_PWLOWERCASEMINLENGTH, XLE_ENTRY_PWLOWERCASEMINLENGTH}},
  {_T("PWUppercaseMinLength"), {XLE_PREF_PWUPPERCASEMINLENGTH, XLE_ENTRY_PWUPPERCASEMINLENGTH}},
  {_T("PWDigitMinLength"), {XLE_PREF_PWDIGITMINLENGTH, XLE_ENTRY_PWDIGITMINLENGTH}},
  {_T("PWSymbolMinLength"), {XLE_PREF_PWSYMBOLMINLENGTH, XLE_ENTRY_PWSYMBOLMINLENGTH}},
  {_T("DefaultUsername"), {XLE_PREF_DEFAULTUSERNAME, 0}},
  {_T("DefaultAutotypeString"), {XLE_PREF_DEFAULTAUTOTYPESTRING, 0}},
  {_T("DefaultSymbols"), {XLE_PREF_DEFAULTSYMBOLS, 0}},
  {_T("group"), {0, XLE_GROUP}},
  {_T("title"), {0, XLE_TITLE}},
  {_T("username"), {0, XLE_USERNAME}},
  {_T("password"), {0, XLE_PASSWORD}},
  {_T("url"), {0, XLE_URL}},
  {_T("autotype"), {0, XLE_AUTOTYPE}},
  {_T("runcommand"), {0, XLE_RUNCOMMAND}},
  {_T("dca"), {0, XLE_DCA}},
  {_T("shiftdca"), {0, XLE_SHIFTDCA}},
  {_T("email"), {0, XLE_EMAIL}},
  {_T("protected"), {0, XLE_PROTECTED}},
  {_T("notes"), {0, XLE_NOTES}},
  {_T("uuid"), {0, XLE_UUID}},
  {_T("ctimex"), {0, XLE_CTIMEX}},
  {_T("atimex"), {0, XLE_ATIMEX}},
  {_T("xtimex"), {0, XLE_XTIMEX}},
  {_T("pmtimex"), {0, XLE_PMTIMEX}},
  {_T("rmtimex"), {0, XLE_RMTIMEX}},
  {_T("xtime_interval"), {0, XLE_XTIME_INTERVAL}},
  {_T("pwhistory"), {0, XLE_PWHISTORY}},
  {_T("PasswordPolicy"), {0, XLE_ENTRY_PASSWORDPOLICY}},
  {_T("symbols"), {0, XLE_SYMBOLS}},
  {_T("kbshortcut"), {0, XLE_KBSHORTCUT}},
  {_T("status"), {0, XLE_STATUS}},
  {_T("max"), {0, XLE_MAX}},
  {_T("num"), {0, XLE_NUM}},
  {_T("history_entries"), {0, XLE_HISTORY_ENTRIES}},
  {_T("history_entry"), {0, XLE_HISTORY_ENTRY}},
  {_T("changedx"), {0, XLE_CHANGEDX}},
  {_T("oldpassword"), {0, XLE_OLDPASSWORD}},
  {_T("PWLength"), {0, XLE_ENTRY_PWLENGTH}},
  {_T("NamedPasswordPolicies"), {0, XLE_PASSWORDPOLICYNAMES}},
  {_T("Policy"), {XLE_POLICY, 0}},
  {_T("PWName"), {XLE_PWNAME, 0}},
  {_T("PasswordPolicyName"), {0, XLE_ENTRY_PASSWORDPOLICYNAME}},
  {_T("EmptyGroups"), {XLE_EMPTYGROUPS, 0}},
  {_T("EGName"), {XLE_EGNAME, 0}},
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

#if USE_XML_LIBRARY == MSXML
bool XMLFileValidation::GetElementInfo(const wchar_t *name, st_file_element_data &edata)
#elif USE_XML_LIBRARY == XERCES
bool XMLFileValidation::GetElementInfo(const XMLCh *name, st_file_element_data &edata)
#endif
{
#if USE_XML_LIBRARY == XERCES
  const stringT strValue(_X2ST(name));
#else
  const stringT strValue(name);
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
