/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "XMLAttValidation.h"

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
* the other 2 fields comprise the associated data vias a structure.
*/

const XMLAttValidation::st_file_elements XMLAttValidation::m_file_elements[XLA_ELEMENTS] = {
  {_T("passwordsafe_attachments"), {XLA_PASSWORDSAFE_ATTACHMENTS, 0}},
  {_T("attachment"), {XLA_ATTACHMENT, XLA_ATTACHMENT}},  // Note: attachment must be in both!
  {_T("group"), {0, XLA_GROUP}},
  {_T("title"), {0, XLA_TITLE}},
  {_T("username"), {0, XLA_USER}},
  {_T("attachment_uuid"), {0, XLA_ATTACHMENT_UUID}},
  {_T("entry_uuid"), {0, XLA_ENTRY_UUID}},
  {_T("filename"), {0, XLA_FILENAME}},
  {_T("path"), {0, XLA_PATH}},
  {_T("description"), {0, XLA_DESCRIPTION}},
  {_T("osize"), {0, XLA_OSIZE}},
  {_T("bsize"), {0, XLA_BSIZE}},
  {_T("csize"), {0, XLA_CSIZE}},
  {_T("crc"), {0, XLA_CRC}},
  {_T("odigest"), {0, XLA_ODIGEST}},
  {_T("cdigest"), {0, XLA_CDIGEST}},
  {_T("ctime"), {0, XLA_CTIME}},
  {_T("atime"), {0, XLA_ATIME}},
  {_T("mtime"), {0, XLA_MTIME}},
  {_T("dtime"), {0, XLA_DTIME}},
  {_T("flags"), {0, XLA_FLAGS}},
  {_T("data80"), {0, XLA_DATA80}},
  {_T("data81"), {0, XLA_DATA81}},
  {_T("extracttoremoveable"), {0, XLA_FLAG_EXTRACTTOREMOVEABLE}},
  {_T("eraseprogamexists"), {0, XLA_FLAG_ERASEPROGAMEXISTS}},
  {_T("eraseondatabaseclose"), {0, XLA_FLAG_ERASEONDATABASECLOSE}},
  {_T("date"), {0, XLA_DATE}},
  {_T("time"), {0, XLA_TIME}}
};

XMLAttValidation::XMLAttValidation()
{
  for (int i = 0; i < XLA_ELEMENTS; i++) {
    m_element_map.insert(file_element_pair(stringT(m_file_elements[i].name),
                                           m_file_elements[i].file_element_data));
  }
}

XMLAttValidation::~XMLAttValidation()
{
  m_element_map.clear();
}

#if   USE_XML_LIBRARY == EXPAT
bool XMLAttValidation::GetElementInfo(const XML_Char *name, st_file_element_data &edata)
#elif USE_XML_LIBRARY == MSXML
bool XMLAttValidation::GetElementInfo(const wchar_t *name, st_file_element_data &edata)
#elif USE_XML_LIBRARY == XERCES
bool XMLAttValidation::GetElementInfo(const XMLCh *name, st_file_element_data &edata)
#endif
{
#if USE_XML_LIBRARY == XERCES
  USES_XMLCH_STR
#endif

#ifdef UNICODE
#if USE_XML_LIBRARY == XERCES
  const stringT strValue(_X2ST(name));
#else
  const stringT strValue(name);
#endif
#else   // NON-UNICODE
#if   USE_XML_LIBRARY == EXPAT
  const stringT strValue(name);
#elif USE_XML_LIBRARY == MSXML
#if (_MSC_VER >= 1400)
  size_t numchars = wcslen(name);
  char* szData = new char[numchars + 2];
  size_t num_converted;
  wcstombs_s(&num_converted, szData, numchars + 2, name, numchars);
#else
  wcstombs(szData, name, numchars);
#endif  // MSXML NON-UNICODE _MSC_VER
  const stringT strValue(szData);
  delete szData;
#elif USE_XML_LIBRARY == XERCES
  char *szData = XMLString::transcode(name);
  const stringT strValue(szData);
  XMLString::release(&szData);
#endif  // EXPAT, MSXML or XERCES
#endif  // NON-UNICODE

  if (strValue.length() == 0)
    return false;

  std::map<stringT, st_file_element_data> :: const_iterator e_iter;
  e_iter = m_element_map.find(strValue);
  if (e_iter != m_element_map.end()) {
    edata = e_iter->second;
    return true;
  } else {
    edata.element_code = XLA_LAST_ELEMENT;
    edata.element_entry_code = XLA_LAST_ELEMENT;
    return false;
  }
}

#endif /* USE_XML_LIBRARY */
