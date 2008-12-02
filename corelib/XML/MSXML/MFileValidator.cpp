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
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.  This is done in the XFileSAX2Handlers routines:
* 'startelement' for attributes and 'characters' & 'ignorableWhitespace'
* for element data.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == MSXML

// XML File Import constants - used by Expat, Xerces and MSXML
#include "../XMLFileDefs.h"

// MSXML validation includes
#include "MFileValidator.h"

// PWS includes
#include "../../StringX.h"

#include <map>

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

const MFileValidator::st_file_elements MFileValidator::m_file_elements[XLE_ELEMENTS] = {
  {_T("passwordsafe"), {XLE_PASSWORDSAFE, 0}},
  {_T("NumberHashIterations"), {XLE_NUMBERHASHITERATIONS, 0}},
  {_T("Preferences"), {XLE_PREFERENCES, 0}},
  {_T("unknownheaderfields"), {XLE_UNKNOWNHEADERFIELDS, 0}},
  {_T("entry"), {XLE_ENTRY, XLE_ENTRY}},
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

MFileValidator::MFileValidator()
{
  for (int i = 0; i < XLE_ELEMENTS; i++) {
    m_element_map.insert(file_element_pair(stringT(m_file_elements[i].name),
                                           m_file_elements[i].file_element_data));
  }
}

MFileValidator::~MFileValidator()
{
  m_element_map.clear();
}

bool MFileValidator::GetElementInfo(const wchar_t *name, int numchars, st_file_element_data &edata)
{
#ifdef _UNICODE
  const stringT strValue(name);
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
