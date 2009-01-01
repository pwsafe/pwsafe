/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
*
*/

#ifndef __EFILTERVALIDATOR_H
#define __EFILTERVALIDATOR_H

// Expat validation includes
#include "EFilterValidatorDefs.h"

// PWS includes
#include "../../StringX.h"
#include "../../Match.h"

#include <vector>
#include <map>

// Expat includes
#include <expat.h>

const struct st_filter_element_data {
  unsigned short int element_code /* XTE_FILTERS */;
  unsigned short int element_entrytype /* XTN_ENTRYTYPES */;
  short int element_maxoccurs;
  unsigned short int type;
  PWSMatch::MatchType mt;
  unsigned short int ft /* FieldType */;
};

const struct st_filter_testtypes {
  PWSMatch::MatchRule mr;
  unsigned short int element_entrytype /* XTN_ENTRYTYPES */;
};

class EFilterValidator
{
public:
  EFilterValidator();
  ~EFilterValidator();

  bool startElement(stringT &strStartElement);
  bool endElement(stringT &strEndElement, StringX &strElemContent, int &datatype);

  bool VerifyXMLDataType(const StringX &strElemContent, const int &datatype);
  bool GetElementInfo(const XML_Char *name, st_filter_element_data &edata);
  PWSMatch::MatchRule GetMatchRule(const TCHAR *cs_rule);

  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMsg() {return m_sErrorMsg;}

private:
  bool VerifyStartElement(const st_filter_element_data &filter_element_data);
  bool VerifyXMLRule(const StringX &strElemContent, const int &datatype);
  bool VerifyXMLDate(const StringX &strElemContent);
  StringX Trim(const StringX &s, const TCHAR *set = NULL);

  std::map<stringT, st_filter_element_data> m_element_map;
  std::map<stringT, st_filter_testtypes> m_testtypes_map;
  typedef std::pair<stringT, st_filter_element_data> filter_element_pair;
  typedef std::pair<stringT, st_filter_testtypes> filter_rules_pair;

  std::vector<int> m_elementstack;
  std::vector<int> m_elementtype;

  stringT m_sErrorMsg;
  int m_ielement_occurs[XTE_LAST_ELEMENT];
  int m_igroup_element;
  int m_idatetime_element;
  int m_irule_type;
  int m_iErrorCode;
  bool m_bfiltergroup;

  static const struct st_filter_elements {
    TCHAR *name; st_filter_element_data filter_element_data;
  } m_filter_elements[XTE_LAST_ELEMENT];
  static const struct st_filter_rules {
    TCHAR *name; st_filter_testtypes filter_testtypes;
  } m_filter_rules[PWSMatch::MR_LAST];
};

#endif /* __EFILTERVALIDATOR_H */
