/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine validates XML when using the Expat library V2.0.1
* released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* NOTE: EXPAT is a NON-validating XML Parser.  All conformity with the
* schema must be performed here in lieu of schema schecking.
*
* As per XML parsing rules, any error stops the parsing immediately.
*/

#ifndef __EFILEVALIDATOR_H
#define __EFILEVALIDATOR_H

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileDefs.h"

// Expat validation includes
#include "EFileValidatorDefs.h"

// PWS includes
#include "../../StringX.h"

#include <vector>
#include <map>

// Expat includes
#include <expat.h>

const struct st_file_element_data {
  unsigned short int element_code /* XLE_PASSWORDSAFE */;
  unsigned short int element_entry_code /* XLE_PASSWORDSAFE  - entry values*/;
  short int element_maxoccurs;
};

class EFileValidator
{
public:
  EFileValidator();
  ~EFileValidator();

  bool startElement(stringT &startElement);
  bool endElement(stringT &endElement, StringX &strElemContent, int &datatype);
  bool VerifyXMLDataType(const StringX &strElemContent, const int &datatype);
  bool GetElementInfo(const XML_Char *name, st_file_element_data &edata);

  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMsg() {return m_sErrorMsg;}

private:
  bool VerifyXMLDate(const StringX &strElemContent);
  bool VerifyXMLTime(const StringX &strElemContent);
  StringX Trim(const StringX &s, const TCHAR *set = NULL);

  std::map<stringT, st_file_element_data> m_element_map;
  typedef std::pair<stringT, st_file_element_data> file_element_pair;

  std::vector<int> m_elementstack;

  stringT m_sErrorMsg;
  int m_ielement_occurs[XLE_LAST_ELEMENT];
  int m_iprevious_element;
  int m_idatetime_element;
  int m_iErrorCode;
  bool m_b_inheader, m_b_inentry;

  static const struct st_file_elements {
    TCHAR *name; st_file_element_data file_element_data;
  } m_file_elements[XLE_ELEMENTS];
};

#endif /* __EFILEVALIDATOR_H */
