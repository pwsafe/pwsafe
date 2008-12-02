/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine doesn't do anything as Xerces is a validating XML Parser.
* However, it is present to mimic Expat's version and contains similar data
* to streamline import processing.
*
* Note: Xerces uses wchar_t even in non-Unicode mode.
*/

#ifndef __XFILEVALIDATOR_H
#define __XFILEVALIDATOR_H

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileDefs.h"

// PWS includes
#include "../../StringX.h"

#include <vector>
#include <map>

const struct st_file_element_data {
  unsigned short int element_code /* XLE_PASSWORDSAFE */;
  unsigned short int element_entry_code /* XLE_PASSWORDSAFE  - entry values*/;
};

// Xerces includes
#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_USE

class XFileValidator
{
public:
  XFileValidator();
  ~XFileValidator();

  bool GetElementInfo(const XMLCh *name, st_file_element_data &edata);

private:
  std::map<wstringT, st_file_element_data> m_element_map;
  typedef std::pair<wstringT, st_file_element_data> file_element_pair;

  static const struct st_file_elements {
    wchar_t *name; st_file_element_data file_element_data;
  } m_file_elements[XLE_ELEMENTS];
};

#endif /* __XFILEVALIDATOR_H */
