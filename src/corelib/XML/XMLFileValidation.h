/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLFILEVALIDATION_H
#define __XMLFILEVALIDATION_H

#ifdef USE_XML_LIBRARY

// PWS includes
#include "../StringX.h"

#include <map>

#if   USE_XML_LIBRARY == EXPAT
// Expat includes
#include <expat.h>
#elif USE_XML_LIBRARY == MSXMAL
// MSXML includes
// None
#elif USE_XML_LIBRARY == XERCES
// Xerces includes
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE

#endif

// Elements stack value
// Xml fiLe Elements = 'XLE' prefix

enum XLE_PASSWORDSAFE {
  XLE_PASSWORDSAFE            = 0,
  XLE_NUMBERHASHITERATIONS,
  XLE_PREFERENCES,
  XLE_UNKNOWNHEADERFIELDS,
  XLE_ENTRY,

  // Preferences
  //   Integer & Boolean
  XLE_PREF_START,
  XLE_DISPLAYEXPANDEDADDEDITDLG = XLE_PREF_START,
  XLE_IDLETIMEOUT,
  XLE_MAINTAINDATETIMESTAMPS,
  XLE_NUMPWHISTORYDEFAULT,
  XLE_PWDEFAULTLENGTH,
  XLE_PWDIGITMINLENGTH,
  XLE_PWLOWERCASEMINLENGTH,
  XLE_PWMAKEPRONOUNCEABLE,
  XLE_PWSYMBOLMINLENGTH,
  XLE_PWUPPERCASEMINLENGTH,
  XLE_PWUSEDIGITS,
  XLE_PWUSEEASYVISION,
  XLE_PWUSEHEXDIGITS,
  XLE_PWUSELOWERCASE,
  XLE_PWUSESYMBOLS,
  XLE_PWUSEUPPERCASE,
  XLE_SAVEIMMEDIATELY,
  XLE_SAVEPASSWORDHISTORY,
  XLE_SHOWNOTESDEFAULT,
  XLE_SHOWPASSWORDINTREE,
  XLE_SHOWPWDEFAULT,
  XLE_SHOWUSERNAMEINTREE,
  XLE_SORTASCENDING,
  XLE_TREEDISPLAYSTATUSATOPEN,
  XLE_USEDEFAULTUSER,
  XLE_PREF_END = XLE_USEDEFAULTUSER,
  //   String
  XLE_DEFAULTUSERNAME,
  XLE_DEFAULTAUTOTYPESTRING,

  // unknownheaderfields
  XLE_HFIELD,

  // entry
  XLE_GROUP,
  XLE_TITLE,
  XLE_USERNAME,
  XLE_PASSWORD,
  XLE_URL,
  XLE_AUTOTYPE,
  XLE_NOTES,
  XLE_UUID,
  XLE_CTIME,
  XLE_ATIME,
  XLE_LTIME,
  XLE_XTIME,
  XLE_XTIME_INTERVAL,
  XLE_PMTIME,
  XLE_RMTIME,
  XLE_PWHISTORY,
  XLE_RUNCOMMAND,
  XLE_ENTRY_PASSWORDPOLICY,
  XLE_UNKNOWNRECORDFIELDS,

  // pwhistory
  XLE_STATUS,
  XLE_MAX,
  XLE_NUM,
  XLE_HISTORY_ENTRIES,

  // history_entries
  XLE_HISTORY_ENTRY,

  // history_entry
  XLE_CHANGED,
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

  // unknownrecordfields
  XLE_RFIELD,

  // datetime fields
  XLE_DATE,
  XLE_TIME,

  // Last element
  XLE_LAST_ELEMENT
};

// Subtract duplicates (global/entry): header/record field(1), password policy fields(11)
#define XLE_ELEMENTS (XLE_LAST_ELEMENT - 1 - 11)

// Number of Integer/Boolean Preferences
#define NUMPREFSINXML (XLE_PREF_END - XLE_PREF_START + 1)

const struct st_file_element_data {
  unsigned short int element_code /* XLE_PASSWORDSAFE */;
  unsigned short int element_entry_code /* XLE_PASSWORDSAFE  - entry values*/;
};


class XMLFileValidation
{
public:
  XMLFileValidation();
  ~XMLFileValidation();

#if   USE_XML_LIBRARY == EXPAT
  bool GetElementInfo(const XML_Char *name, st_file_element_data &edata);
#elif USE_XML_LIBRARY == MSXML
  bool GetElementInfo(const wchar_t *name, st_file_element_data &edata);
#elif USE_XML_LIBRARY == XERCES
  bool GetElementInfo(const XMLCh *name, st_file_element_data &edata);
#endif

private:
  std::map<stringT, st_file_element_data> m_element_map;
  typedef std::pair<stringT, st_file_element_data> file_element_pair;

  static const struct st_file_elements {
    TCHAR *name; st_file_element_data file_element_data;
  } m_file_elements[XLE_ELEMENTS];
};

#endif /* USE_XML_LIBRARY */

#endif /* __XMLFILEVALIDATION_H */
