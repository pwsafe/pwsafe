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
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// Expat validation includes
#include "EFilterValidator.h"
#include "EFilterValidatorDefs.h"

// PWS includes
#include "../../StringX.h"
#include "../../StringXStream.h"
#include "../../VerifyFormat.h"
#include "../../PWSFilters.h"
#include "../../corelib.h"

#include <algorithm>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/*
 * st_filter_elements:
 * 1. Element Name, as expected in the XML file
 * 2. Element Code, corresponding unique code
 * 3. Rule Type, associated rule type for the element
 * 4. MaxOccurs, associated maximum allowed in XML file or group
 * 5. Filter Type, (Invalid, Normal, History, Policy)
 * 6. Match Type, e.g. string, integer etc.
 * 7. FieldType, corresponding code within filter
 */

const EFilterValidator::st_filter_elements EFilterValidator::m_filter_elements[XTE_LAST_ELEMENT] = {
  {_T("filters"),
    {XTE_FILTERS, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("filter"),
    {XTE_FILTER, XTR_NA, -1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("filter_entry"),
    {XTE_FILTER_ENTRY, XTR_NA, -1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("filter_group"),
    {XTE_FILTER_GROUP, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("group"),
    {XTE_GROUP, XTR_STRINGPRESENTRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_GROUP}},
  {_T("grouptitle"),
    {XTE_GROUPTITLE, XTR_STRINGRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_GROUPTITLE}},
  {_T("title"),
    {XTE_TITLE, XTR_STRINGRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_TITLE}},
  {_T("user"),
    {XTE_USERNAME, XTR_STRINGPRESENTRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_USER}},
  {_T("notes"),
    {XTE_NOTES, XTR_STRINGPRESENTRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_NOTES}},
  {_T("password"),
    {XTE_PASSWORD, XTR_PASSWORDRULE, 1, DFTYPE_MAIN, PWSMatch::MT_PASSWORD, FT_PASSWORD}},
  {_T("create_time"),
    {XTE_CREATE_TIME, XTR_DATERULE, 1, DFTYPE_MAIN, PWSMatch::MT_DATE, FT_CTIME}},
  {_T("password_modified_time"),
    {XTE_PASSWORD_MODIFIED_TIME, XTR_DATERULE, 1, DFTYPE_MAIN, PWSMatch::MT_DATE, FT_PMTIME}},
  {_T("last_access_time"),
    {XTE_LAST_ACCESS_TIME, XTR_DATERULE, 1, DFTYPE_MAIN, PWSMatch::MT_DATE, FT_ATIME}},
  {_T("expiry_time"),
    {XTE_EXPIRY_TIME, XTR_DATERULE, 1, DFTYPE_MAIN, PWSMatch::MT_DATE, FT_XTIME}},
  {_T("record_modified_time"),
    {XTE_RECORD_MODIFIED_TIME, XTR_DATERULE, 1, DFTYPE_MAIN, PWSMatch::MT_DATE, FT_RMTIME}},
  {_T("url"),
    {XTE_URL, XTR_STRINGPRESENTRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_URL}},
  {_T("autotype"),
    {XTE_AUTOTYPE, XTR_STRINGPRESENTRULE, 1, DFTYPE_MAIN, PWSMatch::MT_STRING, FT_AUTOTYPE}},
  {_T("password_expiry_interval"),
    {XTE_PASSWORD_EXPIRY_INTERVAL, XTR_INTEGERRULE, 1, DFTYPE_MAIN, PWSMatch::MT_INTEGER, FT_XTIME_INT}},
  {_T("password_history"),
    {XTE_PASSWORD_HISTORY, XTR_PASSWORDHISTORYRULE, 1, DFTYPE_MAIN, PWSMatch::MT_PWHIST, FT_PWHIST}},
  {_T("history_present"),
    {XTE_HISTORY_PRESENT, XTR_BOOLEANPRESENTRULE, 1, DFTYPE_PWHISTORY, PWSMatch::MT_BOOL, HT_PRESENT}},
  {_T("history_active"),
    {XTE_HISTORY_ACTIVE, XTR_BOOLEANACTIVERULE, 1, DFTYPE_PWHISTORY, PWSMatch::MT_BOOL, HT_ACTIVE}},
  {_T("history_number"),
    {XTE_HISTORY_NUMBER, XTR_INTEGERRULE, 1, DFTYPE_PWHISTORY, PWSMatch::MT_INTEGER, HT_NUM}},
  {_T("history_maximum"),
    {XTE_HISTORY_MAXIMUM, XTR_INTEGERRULE, 1, DFTYPE_PWHISTORY, PWSMatch::MT_INTEGER, HT_MAX}},
  {_T("history_changedate"),
    {XTE_HISTORY_CHANGEDATE, XTR_DATERULE, 1, DFTYPE_PWHISTORY, PWSMatch::MT_DATE, HT_CHANGEDATE}},
  {_T("history_passwords"),
    {XTE_HISTORY_PASSWORDS, XTR_NA, 1, DFTYPE_PWHISTORY, PWSMatch::MT_PASSWORD, HT_PASSWORDS}},
  {_T("password_policy"),
    {XTE_PASSWORD_POLICY, XTR_NA, 1, DFTYPE_MAIN, PWSMatch::MT_POLICY, FT_POLICY}},
  {_T("policy_present"),
    {XTE_POLICY_PRESENT, XTR_NA, 1, DFTYPE_PWPOLICY, PWSMatch::MT_BOOL, PT_PRESENT}},
  {_T("policy_length"),
    {XTE_POLICY_LENGTH, XTR_INTEGERRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_INTEGER, PT_LENGTH}},
  {_T("policy_number_lowercase"),
    {XTE_POLICY_NUMBER_LOWERCASE, XTR_INTEGERRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_INTEGER, PT_LOWERCASE}},
  {_T("policy_number_uppercase"),
    {XTE_POLICY_NUMBER_UPPERCASE, XTR_INTEGERRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_INTEGER, PT_UPPERCASE}},
  {_T("policy_number_digits"),
    {XTE_POLICY_NUMBER_DIGITS, XTR_INTEGERRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_INTEGER, PT_DIGITS}},
  {_T("policy_number_symbols"),
    {XTE_POLICY_NUMBER_SYMBOLS, XTR_INTEGERRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_INTEGER, PT_SYMBOLS}},
  {_T("policy_easyvision"),
    {XTE_POLICY_EASYVISION, XTR_BOOLEANSETRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_BOOL, PT_EASYVISION}},
  {_T("policy_pronounceable"),
    {XTE_POLICY_PRONOUNCEABLE, XTR_BOOLEANSETRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_BOOL, PT_PRONOUNCEABLE}},
  {_T("policy_hexadecimal"),
    {XTE_POLICY_HEXADECIMAL, XTR_BOOLEANSETRULE, 1, DFTYPE_PWPOLICY, PWSMatch::MT_BOOL, PT_HEXADECIMAL}},
  {_T("entrytype"),
    {XTE_ENTRYTYPE, XTR_ENTRYRULE, 1, DFTYPE_MAIN, PWSMatch::MT_ENTRYTYPE, FT_ENTRYTYPE}},
  {_T("unknownfields"),
    {XTE_UNKNOWNFIELDS, XTR_BOOLEANPRESENTRULE, 1, DFTYPE_MAIN, PWSMatch::MT_INVALID, FT_UNKNOWNFIELDS}},
  {_T("test"),
    {XTE_TEST, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("rule"),
    {XTE_RULE, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("logic"),
    {XTE_LOGIC, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("string"),
    {XTE_STRING, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("case"),
    {XTE_CASE, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("warn"),
    {XTE_WARN, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("num1"),
    {XTE_NUM1, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("num2"),
    {XTE_NUM2, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("date1"),
    {XTE_DATE1, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("date2"),
    {XTE_DATE2, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
  {_T("type"),
    {XTE_TYPE, XTR_NA, 1, DFTYPE_INVALID, PWSMatch::MT_INVALID, FT_INVALID}},
};

const EFilterValidator::st_filter_rules EFilterValidator::m_filter_rulecodes[PWSMatch::MR_LAST] = {
  {_T("NA"), {PWSMatch::MR_INVALID, XTR_NA} },
  {_T("EQ"), {PWSMatch::MR_EQUALS, XTR_DATERULE | XTR_INTEGERRULE | XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("NE"), {PWSMatch::MR_NOTEQUAL, XTR_DATERULE | XTR_INTEGERRULE | XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("AC"), {PWSMatch::MR_ACTIVE, XTR_BOOLEANACTIVERULE} },
  {_T("IA"), {PWSMatch::MR_INACTIVE, XTR_BOOLEANACTIVERULE} },
  {_T("PR"), {PWSMatch::MR_PRESENT, XTR_BOOLEANPRESENTRULE | XTR_DATERULE | XTR_INTEGERRULE | XTR_STRINGPRESENTRULE} },
  {_T("NP"), {PWSMatch::MR_NOTPRESENT, XTR_BOOLEANPRESENTRULE | XTR_DATERULE | XTR_INTEGERRULE | XTR_STRINGPRESENTRULE} },
  {_T("SE"), {PWSMatch::MR_SET, XTR_BOOLEANSETRULE} },
  {_T("NS"), {PWSMatch::MR_NOTSET, XTR_BOOLEANSETRULE} },
  {_T("IS"), {PWSMatch::MR_IS, XTR_ENTRYRULE} },
  {_T("NI"), {PWSMatch::MR_ISNOT, XTR_ENTRYRULE} },
  {_T("BE"), {PWSMatch::MR_BEGINS, XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("NB"), {PWSMatch::MR_NOTBEGIN, XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("EN"), {PWSMatch::MR_ENDS, XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("ND"), {PWSMatch::MR_NOTEND, XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("CO"), {PWSMatch::MR_CONTAINS, XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("NC"), {PWSMatch::MR_NOTCONTAIN, XTR_PASSWORDRULE | XTR_STRINGRULE | XTR_STRINGPRESENTRULE} },
  {_T("BT"), {PWSMatch::MR_BETWEEN, XTR_DATERULE | XTR_INTEGERRULE} },
  {_T("LT"), {PWSMatch::MR_LT, XTR_INTEGERRULE} },
  {_T("LE"), {PWSMatch::MR_LE, XTR_INTEGERRULE} },
  {_T("GT"), {PWSMatch::MR_GT, XTR_INTEGERRULE} },
  {_T("GE"), {PWSMatch::MR_GE, XTR_INTEGERRULE} },
  {_T("BF"), {PWSMatch::MR_BEFORE, XTR_DATERULE} },
  {_T("AF"), {PWSMatch::MR_AFTER, XTR_DATERULE} },
  {_T("EX"), {PWSMatch::MR_EXPIRED, XTR_PASSWORDRULE} },
  {_T("WX"), {PWSMatch::MR_WILLEXPIRE, XTR_PASSWORDRULE} }
};

EFilterValidator::EFilterValidator()
{
  m_sErrorMsg = _T("");
  m_group_element_code = XTE_INVALID;
  m_datetime_element_code = XTE_INVALID;
  m_rule_code = XTR_NA;
  m_iErrorCode = 0;
  m_bfiltergroup = false;

  // Populate Element Map - provides, amongst other things,
  // the equivalent of a 'switch' statement on a string value
  for (int i = 0; i < XTE_LAST_ELEMENT; i++) {
    m_element_map.insert(Filter_Element_Pair(stringT(m_filter_elements[i].name),
                                      m_filter_elements[i].filter_element_data));
  }

  // Populate Rules Map - provides, amongst other things, 
  // the equivalent of a 'switch' statement on a string value
  for (int i = 0; i < PWSMatch::MR_LAST; i++) {
    m_rulecode_map.insert(Filter_Rules_Pair(stringT(m_filter_rulecodes[i].name),
                                      m_filter_rulecodes[i].filter_rulecode_data));
  }

  // Element count variable to ensure that user doesn't specify too many (MaxOccurs)
  for (int i = 0; i < XTE_LAST_ELEMENT; i++) {
    m_ielement_occurs[i] = 0;
  }
}

EFilterValidator::~EFilterValidator()
{
  // Clear stack variables (no clear function)
  // Should be empty in normal processing.  Might not be if error aborted parsing.
  while (m_element_code_stack.size() > 0) {
    m_element_code_stack.pop();
  }
  while (m_element_datatype_stack.size() > 0) {
    m_element_datatype_stack.pop();
  }
  // Clear map variables
  m_element_map.clear();
  m_rulecode_map.clear();
}

bool EFilterValidator::startElement(stringT & strStartElement)
{
  if (strStartElement == _T("filters")) {
    if (!m_element_code_stack.empty() || m_ielement_occurs[XTE_FILTERS] > 0) {
      return false;
    } else {
      m_element_code_stack.push(XTE_FILTERS);
      m_element_datatype_stack.push(XTD_NA);
      return true;
    }
  }

  if (m_element_code_stack.empty())
    return false;

  cFilter_Element_iter e_iter;
  e_iter = m_element_map.find(strStartElement);
  if (e_iter == m_element_map.end()) {
    m_iErrorCode = XTPEC_UNKNOWN_FIELD;
    Format(m_sErrorMsg, IDSC_EXPATUNKNELEMENT, strStartElement.c_str());
    return false;
  }

  if (!VerifyStartElement(e_iter)) {
    return false;
  }

  XTD_DataTypes data_type(XTD_NA);
  switch (e_iter->second.element_code) {
    case XTE_FILTER_ENTRY:
      // Reset elements found as this is a new filter entry
      m_bfiltergroup = false;
      m_group_element_code = XTE_INVALID;
      m_datetime_element_code = XTE_INVALID;
      m_rule_code = XTR_NA;
      for (int i = XTE_GROUP; i <= XTE_TYPE; i++) {
        m_ielement_occurs[i] = 0;
      }
      break;
    case XTE_CREATE_TIME:
    case XTE_LAST_ACCESS_TIME:
    case XTE_EXPIRY_TIME:
    case XTE_PASSWORD_MODIFIED_TIME:
    case XTE_RECORD_MODIFIED_TIME:
    case XTE_HISTORY_CHANGEDATE:
      m_datetime_element_code = e_iter->second.element_code;
      break;
    case XTE_LOGIC:
      data_type = XTD_LOGICTYPE;
      break;
    case XTE_STRING:
      data_type = XTD_XS_STRING;
      break;
    case XTE_CASE:
      data_type = XTD_BOOLTYPE;
      break;
    case XTE_WARN:
    case XTE_NUM1:
    case XTE_NUM2:
      data_type = XTD_XS_INT;
      break;
    case XTE_DATE1:
    case XTE_DATE2:
      data_type = XTD_XS_DATE;
      break;
    case XTE_TYPE:
      data_type = XTD_ENTRYTYPE;
      break;
    default:
      break;
  }

  if (e_iter->second.element_code >= XTE_GROUP &&
      e_iter->second.element_code <= XTE_UNKNOWNFIELDS) {
    if (m_bfiltergroup) {
      m_group_element_code = e_iter->second.element_code;
      m_rule_code = e_iter->second.rule_code;
    }
  }

  if (m_iErrorCode != 0) {
    switch (m_iErrorCode) {
      case XTPEC_UNEXPECTED_ELEMENT:
        Format(m_sErrorMsg, IDSC_EXPATUNEXPECTED, strStartElement);
        break;
      default:
      /*
      case XTPEC_MISSING_MANDATORY_FIELD:
      case XTPEC_EXCEEDED_MAXOCCURS:
      case XTPEC_MISSING_ELEMENT:
      case XTPEC_INVALID_DATA:
      case XTPEC_UNKNOWN_FIELD:
      */
        break;
    }
    return false;
  }

  m_element_code_stack.push(e_iter->second.element_code);
  m_element_datatype_stack.push(data_type);
  return true;
}

bool EFilterValidator::endElement(stringT &strEndElement,
                                  StringX &strValue)
{
  if (strEndElement == _T("filters"))
    return true;

  bool bGoodData(false);
  if (strEndElement == _T("rule")) {
    bGoodData = VerifyXMLRule(strValue, m_rule_code);
  } else {
    XTD_DataTypes data_type = m_element_datatype_stack.top();
    bGoodData = VerifyXMLDataType(strValue, data_type);
  }

  if (!bGoodData) {
    m_iErrorCode = XTPEC_INVALID_DATA;
    Format(m_sErrorMsg, IDSC_EXPATBADDATA, strValue.c_str(), strEndElement.c_str());
    return false;
  }

  XTE_Codes &icurrent_element = m_element_code_stack.top();
  stringT cs_missing_element(_T(""));
  switch (icurrent_element) {
    case XTE_FILTER_ENTRY:
      m_bfiltergroup = false;
      break;
    case XTE_TEST:
      switch (m_rule_code) {
        case XTR_DATERULE:
          if (m_ielement_occurs[XTE_DATE1] != 1 ||
              m_ielement_occurs[XTE_DATE2] != 1) {
            m_iErrorCode = XTPEC_MISSING_ELEMENT;
            cs_missing_element = m_ielement_occurs[XTE_DATE1] != 1 ? _T("date1") : _T("date2");
          }
          break;
        case XTR_ENTRYRULE:
          if (m_ielement_occurs[XTE_TYPE] != 1) {
            m_iErrorCode = XTPEC_MISSING_ELEMENT;
            cs_missing_element =  _T("type");
          }
          break;
        case XTR_INTEGERRULE:
          if (m_ielement_occurs[XTE_NUM1] != 1 ||
              m_ielement_occurs[XTE_NUM2] != 1) {
            m_iErrorCode = XTPEC_MISSING_ELEMENT;
            cs_missing_element = m_ielement_occurs[XTE_NUM1] != 1 ? _T("num1") : _T("num2");
          }
          break;
        case XTR_PASSWORDRULE:
          if (m_ielement_occurs[XTE_STRING] != 1 ||
              m_ielement_occurs[XTE_CASE] != 1 ||
              m_ielement_occurs[XTE_WARN] != 1) {
            m_iErrorCode = XTPEC_MISSING_ELEMENT;
          }
          if (m_ielement_occurs[XTE_WARN] != 1)
            cs_missing_element = _T("warn");
          else
            cs_missing_element = m_ielement_occurs[XTE_STRING] != 1 ? _T("string") : _T("case");
          break;
        case XTR_STRINGRULE:
        case XTR_STRINGPRESENTRULE:
        if (m_ielement_occurs[XTE_STRING] != 1 ||
              m_ielement_occurs[XTE_CASE] != 1) {
            m_iErrorCode = XTPEC_MISSING_ELEMENT;
            cs_missing_element = m_ielement_occurs[XTE_STRING] != 1 ? _T("string") : _T("case");
          }
          break;
        case XTR_PASSWORDHISTORYRULE:
        case XTR_PASSWORDPOLICYRULE:
        default:
          break;
      }
      if (m_iErrorCode != 0) {
        Format(m_sErrorMsg, IDSC_EXPATMISSINGTELM, cs_missing_element.c_str());
        return false;
      }
      break;
    default:
      break;
  }

  m_element_code_stack.pop();
  m_element_datatype_stack.pop();
  return true;
}

bool EFilterValidator::VerifyStartElement(cFilter_Element_iter e_iter)
{
  // Check we haven't reached maximum (iMaxOccurs == -1 means unbounded)
  if (e_iter->second.element_maxoccurs != -1 &&
      m_ielement_occurs[e_iter->second.element_code] >= e_iter->second.element_maxoccurs) {
    TCHAR buffer[10];
#if _MSC_VER >= 1400
    _itot_s(e_iter->second.element_maxoccurs, buffer, 10, 10);
#else
    _itot(e_iter->second.element_maxoccurs, buffer, 10);
#endif
    m_iErrorCode = XTPEC_EXCEEDED_MAXOCCURS;
    Format(m_sErrorMsg, IDSC_EXPATEXCEEDMAXOCCURS, e_iter->first.c_str(), buffer);
    return false;
  }

  if (e_iter->second.element_code < XTE_FILTER_ENTRY)
    return true;

  XTE_Codes current_element_code(e_iter->second.element_code);
  XTE_Codes previous_element_code = m_element_code_stack.top();
  if (previous_element_code >= XTE_GROUP &&
      previous_element_code <= XTE_UNKNOWNFIELDS) {
    previous_element_code = XTE_FILTER_GROUP;
  }
  XTE_Codes pcode;
  int nrule;

  switch (e_iter->second.element_code) {
    case XTE_GROUP:
    case XTE_GROUPTITLE:
    case XTE_TITLE:
    case XTE_USERNAME:
    case XTE_NOTES:
    case XTE_PASSWORD:
    case XTE_CREATE_TIME:
    case XTE_PASSWORD_MODIFIED_TIME:
    case XTE_LAST_ACCESS_TIME:
    case XTE_EXPIRY_TIME:
    case XTE_RECORD_MODIFIED_TIME:
    case XTE_URL:
    case XTE_AUTOTYPE:
    case XTE_PASSWORD_EXPIRY_INTERVAL:
    case XTE_PASSWORD_HISTORY:
    case XTE_HISTORY_PRESENT:
    case XTE_HISTORY_ACTIVE:
    case XTE_HISTORY_NUMBER:
    case XTE_HISTORY_MAXIMUM:
    case XTE_HISTORY_CHANGEDATE:
    case XTE_HISTORY_PASSWORDS:
    case XTE_PASSWORD_POLICY:
    case XTE_POLICY_PRESENT:
    case XTE_POLICY_LENGTH:
    case XTE_POLICY_NUMBER_LOWERCASE:
    case XTE_POLICY_NUMBER_UPPERCASE:
    case XTE_POLICY_NUMBER_DIGITS:
    case XTE_POLICY_NUMBER_SYMBOLS:
    case XTE_POLICY_EASYVISION:
    case XTE_POLICY_PRONOUNCEABLE:
    case XTE_POLICY_HEXADECIMAL:
    case XTE_ENTRYTYPE:
    case XTE_UNKNOWNFIELDS:
      if (m_bfiltergroup) {
        // Can't have more than one field in each filter_entry.
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
        LoadAString(m_sErrorMsg, IDSC_EXPATUNEXPFILTERFLD);
        return false;
      } else {
        m_bfiltergroup = true;
        current_element_code = XTE_FILTER_GROUP;
      }
      break;
    case XTE_RULE:
      if (previous_element_code != XTE_FILTER_GROUP ||
          m_ielement_occurs[XTE_RULE] != 0 ||
          m_ielement_occurs[XTE_LOGIC] != 0 ||
          m_ielement_occurs[XTE_TEST] != 0 ||
          m_ielement_occurs[XTE_STRING] != 0 ||
          m_ielement_occurs[XTE_CASE] != 0 ||
          m_ielement_occurs[XTE_WARN] != 0 ||
          m_ielement_occurs[XTE_NUM1] != 0 ||
          m_ielement_occurs[XTE_NUM2] != 0 ||
          m_ielement_occurs[XTE_DATE1] != 0 ||
          m_ielement_occurs[XTE_DATE2] != 0 ||
          m_ielement_occurs[XTE_TYPE] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_LOGIC:
      if ((m_rule_code & (XTR_PASSWORDHISTORYRULE | XTR_PASSWORDPOLICYRULE)) != 0) {
        pcode = XTE_FILTER_GROUP;
        nrule = 0;
      } else {
        pcode = XTE_RULE;
        nrule = 1;
      }
      if (previous_element_code != pcode ||
          m_ielement_occurs[XTE_RULE] != nrule ||
          m_ielement_occurs[XTE_LOGIC] != 0 ||
          m_ielement_occurs[XTE_TEST] != 0 ||
          m_ielement_occurs[XTE_STRING] != 0 ||
          m_ielement_occurs[XTE_CASE] != 0 ||
          m_ielement_occurs[XTE_WARN] != 0 ||
          m_ielement_occurs[XTE_NUM1] != 0 ||
          m_ielement_occurs[XTE_NUM2] != 0 ||
          m_ielement_occurs[XTE_DATE1] != 0 ||
          m_ielement_occurs[XTE_DATE2] != 0 ||
          m_ielement_occurs[XTE_TYPE] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_TEST:
      if (previous_element_code != XTE_LOGIC ||
          m_ielement_occurs[XTE_RULE] != 1 ||
          m_ielement_occurs[XTE_LOGIC] != 1 ||
          m_ielement_occurs[XTE_TEST] != 0 ||
          m_ielement_occurs[XTE_STRING] != 0 ||
          m_ielement_occurs[XTE_CASE] != 0 ||
          m_ielement_occurs[XTE_WARN] != 0 ||
          m_ielement_occurs[XTE_NUM1] != 0 ||
          m_ielement_occurs[XTE_NUM2] != 0 ||
          m_ielement_occurs[XTE_DATE1] != 0 ||
          m_ielement_occurs[XTE_DATE2] != 0 ||
          m_ielement_occurs[XTE_TYPE] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_STRING:
      if (previous_element_code != XTE_TEST ||
          (m_rule_code & (XTR_STRINGRULE | XTR_STRINGPRESENTRULE | XTR_PASSWORDRULE)) == 0 ||
          m_ielement_occurs[XTE_STRING] != 0 ||
          m_ielement_occurs[XTE_CASE] != 0 ||
          m_ielement_occurs[XTE_WARN] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_CASE:
      if (previous_element_code != XTE_STRING ||
          (m_rule_code & (XTR_STRINGRULE | XTR_STRINGPRESENTRULE | XTR_PASSWORDRULE)) == 0 ||
          m_ielement_occurs[XTE_STRING] != 1 ||
          m_ielement_occurs[XTE_CASE] != 0 ||
          m_ielement_occurs[XTE_WARN] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_WARN:
      if (previous_element_code != XTE_CASE ||
          (m_rule_code & XTR_PASSWORDRULE) == 0 ||
          m_ielement_occurs[XTE_STRING] != 1 ||
          m_ielement_occurs[XTE_CASE] != 1 ||
          m_ielement_occurs[XTE_WARN] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_NUM1:
      if (previous_element_code != XTE_TEST ||
          (m_rule_code & XTR_INTEGERRULE) == 0 ||
          m_ielement_occurs[XTE_NUM1] != 0 ||
          m_ielement_occurs[XTE_NUM2] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_NUM2:
      if (previous_element_code != XTE_NUM1 ||
          (m_rule_code & XTR_INTEGERRULE) == 0 ||
          m_ielement_occurs[XTE_NUM1] != 1 ||
          m_ielement_occurs[XTE_NUM2] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_DATE1:
      if (previous_element_code != XTE_TEST ||
          (m_rule_code & XTR_DATERULE) == 0 ||
          m_ielement_occurs[XTE_DATE1] != 0 ||
          m_ielement_occurs[XTE_DATE2] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_DATE2:
      if (previous_element_code != XTE_DATE1 ||
          (m_rule_code & XTR_DATERULE) == 0 ||
          m_ielement_occurs[XTE_DATE1] != 1 ||
          m_ielement_occurs[XTE_DATE2] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    case XTE_TYPE:
      if (previous_element_code != XTE_TEST ||
          (m_rule_code & XTR_ENTRYRULE) == 0 ||
          m_ielement_occurs[XTE_TYPE] != 0) {
        m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
      }
      break;
    default:
      break;
  }

  if ((m_rule_code & XTR_BOOLEANONLY) != 0 &&
       m_ielement_occurs[XTE_TEST] != 0) {
    // Verify no Test element if a boolean only type filter
    m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
  }

  if ((m_rule_code & (XTR_PASSWORDHISTORYRULE | XTR_PASSWORDPOLICYRULE)) != 0 &&
      (m_ielement_occurs[XTE_RULE] != 0 || m_ielement_occurs[XTE_TEST] != 0)) {
    // Verify no Rule or Test element if a Password History/Policy type filter
    m_iErrorCode = XTPEC_UNEXPECTED_ELEMENT;
  }

  if (m_iErrorCode != 0) {
    if (m_sErrorMsg.empty()) {
      switch (m_iErrorCode) {
        case XTPEC_UNEXPECTED_ELEMENT:
          Format(m_sErrorMsg, IDSC_EXPATUNEXPECTED, e_iter->first.c_str());
          break;
        default:
          break;
      }
    }
    return false;
  }

  m_element_code_stack.push(current_element_code);
  ASSERT(e_iter->second.element_code >= 0 && 
         e_iter->second.element_code < XTE_LAST_ELEMENT);
  m_ielement_occurs[e_iter->second.element_code]++;
  return true;
}

bool EFilterValidator::VerifyXMLRule(const StringX &strElemContent, const XTR_Codes &rule_code)
{
  stringT strValue = stringT(strElemContent.c_str());
  m_matchrule = PWSMatch::MR_INVALID;

  if (strValue.length() == 0)
    return false;

  cFilter_Rules_iter r_iter;
  r_iter = m_rulecode_map.find(strValue.c_str());
  if (r_iter == m_rulecode_map.end()) {
    return false;
  }

  m_matchrule = r_iter->second.mr;
  return ((r_iter->second.irule_code & rule_code) == rule_code);
}

PWSMatch::MatchRule EFilterValidator::GetMatchRule(const TCHAR *cs_rule)
{
  const stringT strValue(cs_rule);

  if (strValue.length() == 0)
    return PWSMatch::MR_INVALID;

  cFilter_Rules_iter r_iter;
  r_iter = m_rulecode_map.find(strValue.c_str());
  if (r_iter == m_rulecode_map.end()) {
    return PWSMatch::MR_INVALID;
  } else {
    return r_iter->second.mr;
  }
}

bool EFilterValidator::GetElementInfo(const XML_Char *name, st_filter_element_data &edata)
{
  const stringT strValue(name);

  if (strValue.length() == 0)
    return false;

  cFilter_Element_iter e_iter;
  e_iter = m_element_map.find(strValue);
  if (e_iter != m_element_map.end()) {
    edata = e_iter->second;
    return (e_iter->second.filter_type != DFTYPE_INVALID);
  } else {
    edata.element_code = XTE_LAST_ELEMENT;
    edata.rule_code = XTR_NA;
    edata.element_maxoccurs = 0;
    edata.filter_type = DFTYPE_INVALID;
    edata.mt = PWSMatch::MT_INVALID;
    edata.ft = FT_INVALID;
    return false;
  }
}

bool EFilterValidator::VerifyXMLDataType(const StringX &strElemContent, const XTD_DataTypes &datatype)
{
  static const TCHAR *digits(_T("0123456789"));

  switch (datatype) {
    case XTD_XS_DATE:
      return VerifyXMLDate(strElemContent);
    case XTD_XS_INT:
      return (strElemContent.find_first_not_of(digits) == StringX::npos);
    case XTD_BOOLTYPE:
      return (strElemContent == _T("0") || strElemContent == _T("1"));
    case XTD_ENTRYTYPE:
      return (strElemContent == _T("normal") ||
              strElemContent == _T("alias") ||
              strElemContent == _T("shortcut") ||
              strElemContent == _T("aliasbase") ||
              strElemContent == _T("shortcutbase"));
    case XTD_LOGICTYPE:
      return (strElemContent == _T("and") ||
              strElemContent == _T("or"));
    case XTD_NONBLANKSTRINGTYPE:
      return (strElemContent.length() == 1);
    case XTD_YESNOSTRINGTYPE:
      return (strElemContent == _T("yes") || strElemContent == _T("no"));
    case XTD_XS_STRING:            // All elements are strings!
    case XTD_FILEUUIDTYPE:         // defined but fields using this datatype are not used
    case XTD_NA:                   // N/A - element doesn't have a value in its own right
    default:
      return true;
  }
}

bool EFilterValidator::VerifyXMLDate(const StringX &strValue)
{
  // yyyy-mm-dd
  if (strValue == _T("1970-01-01"))  // Special case
    return true;

  const int ndigits = 8;
  const int idigits[ndigits] = {0, 1, 2, 3, 5, 6, 8, 9};
  int yyyy, mm, dd;

  if (strValue.length() != 10)
    return false;

  // Validate strValue
  if (strValue[4] != _T('-') ||
      strValue[7] != _T('-'))
    return false;

  for (int i = 0; i < ndigits; i++) {
    if (!iswdigit(strValue[idigits[i]]))
      return false;
  }

  iStringXStream is(strValue);
  TCHAR dummy;
  is >> yyyy >> dummy >> mm >> dummy >> dd;

  return verifyDTvalues(yyyy, mm, dd, 0, 0, 0);
}

StringX EFilterValidator::Trim(const StringX &s, const TCHAR *set)
{
  // This version does NOT change the input arguments!
  const TCHAR *tset = (set == NULL) ? _T(" \t\r\n") : set;
  StringX retval(_T(""));

  StringX::size_type b = s.find_first_not_of(tset);
  if (b != StringX::npos) {
    StringX::size_type e = s.find_last_not_of(tset);
    StringX trimmed(s.begin() + b, s.end() - (s.length() - e) + 1);
    retval = trimmed;
  }
  return retval;
}

#endif /* USE_XML_LIBRARY == EXPAT */
