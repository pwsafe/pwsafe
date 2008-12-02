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
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.  This is done in the XFileSAX2Handlers routines:
* 'startelement' for attributes and 'characters' & 'ignorableWhitespace'
* for element data.
*/

#ifndef __MFILEVALIDATOR_H
#define __MFILEVALIDATOR_H

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

class MFileValidator
{
public:
  MFileValidator();
  ~MFileValidator();

  bool GetElementInfo(const wchar_t *name, int numchars, st_file_element_data &edata);

private:
  std::map<stringT, st_file_element_data> m_element_map;
  typedef std::pair<stringT, st_file_element_data> file_element_pair;

  static const struct st_file_elements {
    TCHAR *name; st_file_element_data file_element_data;
  } m_file_elements[XLE_ELEMENTS];
};

#endif /* __MFILEVALIDATOR_H */
