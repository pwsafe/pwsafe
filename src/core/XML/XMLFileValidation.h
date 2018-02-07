/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLFILEVALIDATION_H
#define __XMLFILEVALIDATION_H

#include "XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#ifdef USE_XML_LIBRARY
#include <map>
// PWS includes
#include "../StringX.h"

#if USE_XML_LIBRARY == MSXML
// MSXML includes
// None
#elif USE_XML_LIBRARY == XERCES
// Xerces includes
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>
XERCES_CPP_NAMESPACE_USE
#endif

// Elements stack value
// XML fiLe Elements = 'XLE' prefix

enum XLE_PASSWORDSAFE {
  XLE_PASSWORDSAFE            = 0,
  XLE_PREFERENCES,
  XLE_PASSWORDPOLICYNAMES,
  XLE_POLICY,
  XLE_PWNAME,
  XLE_EMPTYGROUPS,
  XLE_EGNAME,
  XLE_ENTRY,

  //  Boolean Preferences
  XLE_PREF_START,
  XLE_PREF_SHOWPWDEFAULT = XLE_PREF_START,
  XLE_PREF_SHOWPASSWORDINTREE,
  XLE_PREF_SORTASCENDING,  // Obsolete in 3.40
  XLE_PREF_USEDEFAULTUSER,
  XLE_PREF_SAVEIMMEDIATELY,
  XLE_PREF_PWUSELOWERCASE,
  XLE_PREF_PWUSEUPPERCASE,
  XLE_PREF_PWUSEDIGITS,
  XLE_PREF_PWUSESYMBOLS,
  XLE_PREF_PWUSEHEXDIGITS,
  XLE_PREF_PWUSEEASYVISION,
  XLE_PREF_MAINTAINDATETIMESTAMPS,
  XLE_PREF_SAVEPASSWORDHISTORY,
  XLE_PREF_SHOWNOTESDEFAULT,
  XLE_PREF_SHOWUSERNAMEINTREE,
  XLE_PREF_PWMAKEPRONOUNCEABLE,
  XLE_PREF_LOCKDBONIDLETIMEOUT,
  XLE_PREF_COPYPASSWORDWHENBROWSETOURL,

  //  Integer Preferences
  XLE_PREF_PWDEFAULTLENGTH,
  XLE_PREF_IDLETIMEOUT,
  XLE_PREF_TREEDISPLAYSTATUSATOPEN,
  XLE_PREF_NUMPWHISTORYDEFAULT,
  XLE_PREF_PWDIGITMINLENGTH,
  XLE_PREF_PWLOWERCASEMINLENGTH,
  XLE_PREF_PWSYMBOLMINLENGTH,
  XLE_PREF_PWUPPERCASEMINLENGTH,

  //  String Preferences
  XLE_PREF_DEFAULTUSERNAME,
  XLE_PREF_DEFAULTAUTOTYPESTRING,
  XLE_PREF_DEFAULTSYMBOLS,

  // entry
  XLE_GROUP,
  XLE_TITLE,
  XLE_USERNAME,
  XLE_PASSWORD,
  XLE_URL,
  XLE_AUTOTYPE,
  XLE_NOTES,
  XLE_UUID,
  XLE_CTIMEX,   // Using standard XML format
  XLE_ATIMEX,   // Using standard XML format
  XLE_XTIMEX,   // Using standard XML format
  XLE_PMTIMEX,  // Using standard XML format
  XLE_RMTIMEX,  // Using standard XML format
  XLE_XTIME_INTERVAL,
  XLE_PWHISTORY,
  XLE_RUNCOMMAND,
  XLE_DCA,
  XLE_SHIFTDCA,
  XLE_EMAIL,
  XLE_PROTECTED,
  XLE_SYMBOLS,
  XLE_KBSHORTCUT,
  XLE_ENTRY_PASSWORDPOLICY,
  XLE_ENTRY_PASSWORDPOLICYNAME,

  // pwhistory
  XLE_STATUS,
  XLE_MAX,
  XLE_NUM,
  XLE_HISTORY_ENTRIES,

  // history_entries
  XLE_HISTORY_ENTRY,

  // history_entry
  XLE_CHANGEDX,  // Using standard XML format
  XLE_OLDPASSWORD,

  // entry_PasswordPolicy
  XLE_ENTRY_PWLENGTH,
  XLE_ENTRY_PWUSEDIGITS,
  XLE_ENTRY_PWUSEEASYVISION,
  XLE_ENTRY_PWUSEHEXDIGITS,
  XLE_ENTRY_PWUSELOWERCASE,
  XLE_ENTRY_PWUSESYMBOLS,
  XLE_ENTRY_PWUSEUPPERCASE,
  XLE_ENTRY_PWMAKEPRONOUNCEABLE,
  XLE_ENTRY_PWDIGITMINLENGTH,
  XLE_ENTRY_PWLOWERCASEMINLENGTH,
  XLE_ENTRY_PWUPPERCASEMINLENGTH,
  XLE_ENTRY_PWSYMBOLMINLENGTH,

  // Last element
  XLE_LAST_ELEMENT
};

// Subtract duplicates (global/entry): password policy fields(11)
#define XLE_ELEMENTS (XLE_LAST_ELEMENT - (XLE_ENTRY_PWSYMBOLMINLENGTH - XLE_ENTRY_PWLENGTH + 1))

struct st_file_element_data {
  unsigned short int element_code /* XLE_PASSWORDSAFE */;
  unsigned short int element_entry_code /* XLE_PASSWORDSAFE  - entry values*/;
};

class XMLFileValidation
{
public:
  XMLFileValidation();
  ~XMLFileValidation();

#if USE_XML_LIBRARY == MSXML
  bool GetElementInfo(const wchar_t *name, st_file_element_data &edata);
#elif USE_XML_LIBRARY == XERCES
  bool GetElementInfo(const XMLCh *name, st_file_element_data &edata);
#endif

private:
  std::map<stringT, st_file_element_data> m_element_map;
  typedef std::pair<stringT, st_file_element_data> file_element_pair;

  static const struct st_file_elements {
    const TCHAR *name; st_file_element_data file_element_data;
  } m_file_elements[XLE_ELEMENTS];
};

#endif /* USE_XML_LIBRARY */
#endif /* __XMLFILEVALIDATION_H */
