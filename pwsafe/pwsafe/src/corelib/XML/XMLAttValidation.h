/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLATTVALIDATION_H
#define __XMLATTVALIDATION_H

#include "XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#ifdef USE_XML_LIBRARY
#include <map>
// PWS includes
#include "../StringX.h"

#if   USE_XML_LIBRARY == EXPAT
// Expat includes
#include <expat.h>
#elif USE_XML_LIBRARY == MSXML
// MSXML includes
// None
#elif USE_XML_LIBRARY == XERCES
// Xerces includes
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>
XERCES_CPP_NAMESPACE_USE
#endif

// Elements stack value
// XML fiLe Attachment Elements = 'XLA' prefix

enum XLA_PASSWORDSAFE {
  XLA_PASSWORDSAFE_ATTACHMENTS   = 0,
  XLA_ATTACHMENT,

  // Attachment
  XLA_GROUP,
  XLA_TITLE,
  XLA_USER,
  XLA_ATTACHMENT_UUID,
  XLA_ENTRY_UUID,
  XLA_FILENAME,
  XLA_PATH,
  XLA_DESCRIPTION,
  XLA_OSIZE,
  XLA_BSIZE,
  XLA_CSIZE,
  XLA_CRC,
  XLA_ODIGEST,
  XLA_CDIGEST,
  XLA_CTIME,
  XLA_ATIME,
  XLA_MTIME,
  XLA_DTIME,
  XLA_FLAGS,
  XLA_DATA80,
  XLA_DATA81,

  // Attachment flags
  XLA_FLAG_EXTRACTTOREMOVEABLE,
  XLA_FLAG_ERASEPROGAMEXISTS,
  XLA_FLAG_ERASEONDATABASECLOSE,

  // datetime fields
  XLA_DATE,
  XLA_TIME,

  // Last element
  XLA_LAST_ELEMENT
};

#define XLA_ELEMENTS (XLA_LAST_ELEMENT)

// Number of Integer/Boolean Preferences
#define NUMPREFSINXML (XLA_PREF_END - XLA_PREF_START + 1)

struct st_file_element_data {
  unsigned short int element_code /* XLA_PASSWORDSAFE */;
  unsigned short int element_entry_code /* XLA_PASSWORDSAFE  - entry values*/;
};


class XMLAttValidation
{
public:
  XMLAttValidation();
  ~XMLAttValidation();

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
    const TCHAR *name; st_file_element_data file_element_data;
  } m_file_elements[XLA_ELEMENTS];
};

#endif /* USE_XML_LIBRARY */
#endif /* __XMLATTVALIDATION_H */
