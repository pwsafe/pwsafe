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
#include "../XMLFileValidation.h"

// Expat validation includes
#include "EFileValidatorDefs.h"

// PWS includes
#include "../../StringX.h"

#include <vector>
#include <map>

// Expat includes
#include <expat.h>

class EFileValidator : public XMLFileValidation
{
public:
  EFileValidator();
  ~EFileValidator();

  bool startElement(stringT &startElement);
  bool endElement(stringT &endElement, StringX &strElemContent, int &datatype);
  bool VerifyXMLDataType(const StringX &strElemContent, const int &datatype);

  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMsg() {return m_sErrorMsg;}

private:
  bool VerifyXMLDate(const StringX &strElemContent);
  bool VerifyXMLTime(const StringX &strElemContent);
  StringX Trim(const StringX &s, const TCHAR *set = NULL);

  std::vector<int> m_elementstack;

  stringT m_sErrorMsg;
  int m_ielement_occurs[XLE_LAST_ELEMENT];
  int m_iprevious_element;
  int m_idatetime_element;
  int m_iErrorCode;
  bool m_b_inheader, m_b_inentry;
};

#endif /* __EFILEVALIDATOR_H */
