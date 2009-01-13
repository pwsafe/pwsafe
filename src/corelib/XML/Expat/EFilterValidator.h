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
#include "../../PWSFilters.h"

#include <stack>
#include <map>

// Expat includes
#include <expat.h>

const struct st_filter_element_data {
  XTE_Codes element_code;
  XTR_Codes rule_code;
  short int element_maxoccurs;
  FilterType filter_type; 
  PWSMatch::MatchType mt;
  FieldType ft;
};

const struct st_filter_rulecodes {
  PWSMatch::MatchRule mr;
  int irule_code;
};

typedef std::map<const stringT, const st_filter_element_data> Filter_Element_Map;
typedef std::map<const stringT, const st_filter_element_data> :: const_iterator cFilter_Element_iter;
typedef std::pair<const stringT, const st_filter_element_data> Filter_Element_Pair;

typedef std::map<const stringT, const st_filter_rulecodes> Filter_Rulecodes_Map;
typedef std::map<const stringT, const st_filter_rulecodes> :: const_iterator cFilter_Rules_iter;
typedef std::pair<const stringT, const st_filter_rulecodes> Filter_Rules_Pair;

class EFilterValidator
{
public:
  EFilterValidator();
  ~EFilterValidator();

  bool startElement(stringT &strStartElement);
  bool endElement(stringT &strEndElement, StringX &strElemContent);

  bool VerifyXMLDataType(const StringX &strElemContent, const XTD_DataTypes &datatype);
  bool GetElementInfo(const XML_Char *name, st_filter_element_data &edata);
  PWSMatch::MatchRule GetMatchRule(const TCHAR *cs_rule);

  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMsg() {return m_sErrorMsg;}

private:
  bool VerifyStartElement(cFilter_Element_iter e_iter);
  bool VerifyXMLRule(const StringX &strElemContent, const XTR_Codes &rule_code);
  bool VerifyXMLDate(const StringX &strElemContent);
  StringX Trim(const StringX &s, const TCHAR *set = NULL);

  Filter_Element_Map m_element_map;
  Filter_Rulecodes_Map m_rulecode_map;

  std::stack<XTE_Codes> m_element_code_stack;
  std::stack<XTD_DataTypes> m_element_datatype_stack;

  stringT m_sErrorMsg;
  int m_iErrorCode;

  int m_ielement_occurs[XTE_LAST_ELEMENT];
  XTE_Codes m_group_element_code;
  XTE_Codes m_datetime_element_code;
  XTR_Codes m_rule_code;
  bool m_bfiltergroup;
  PWSMatch::MatchRule m_matchrule;

  static const struct st_filter_elements {
    TCHAR *name; st_filter_element_data filter_element_data;
  } m_filter_elements[XTE_LAST_ELEMENT];
  static const struct st_filter_rules {
    TCHAR *name; st_filter_rulecodes filter_rulecode_data;
  } m_filter_rulecodes[PWSMatch::MR_LAST];
};

#endif /* __EFILTERVALIDATOR_H */
