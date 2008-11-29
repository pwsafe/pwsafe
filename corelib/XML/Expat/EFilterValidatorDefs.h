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
*
*/

#ifndef _EFILTERVALIDATORDEFS_H_
#define _EFILTERVALIDATORDEFS_H_

// Error codes
// Xml filTer Parsing Error Codes = 'XTPEC' prefix

enum ErrorCodes {
  XTPEC_EXCEEDED_MAXOCCURS       = 1,
  XTPEC_UNEXPECTED_ELEMENT,
  XTPEC_MISSING_MANDATORY_FIELD,
  XTPEC_MISSING_ELEMENT,
  XTPEC_INVALID_DATA,
  XTPEC_UNKNOWN_FIELD,
  XTPEC_FILTERS_VERSION_MISSING,
  XTPEC_FILTERNAME_MISSING,
  XTPEC_NON_UNIQUE_FILTER_NAME
};

// Elements stack value
// Xml filTer Elements = 'XTE' prefix

enum XTE_FILTERS {
  XTE_FILTERS                    = 0,
  XTE_FILTER,

  // filter
  XTE_FILTER_ENTRY,
  XTE_FILTER_GROUP,

  // filter group
  XTE_GROUP,
  XTE_GROUPTITLE,
  XTE_TITLE,
  XTE_USERNAME,
  XTE_NOTES,
  XTE_PASSWORD,
  XTE_CREATE_TIME,
  XTE_PASSWORD_MODIFIED_TIME,
  XTE_LAST_ACCESS_TIME,
  XTE_EXPIRY_TIME,
  XTE_RECORD_MODIFIED_TIME,
  XTE_URL,
  XTE_AUTOTYPE,
  XTE_PASSWORD_EXPIRY_INTERVAL,
  XTE_PASSWORD_HISTORY,
  XTE_HISTORY_PRESENT,
  XTE_HISTORY_ACTIVE,
  XTE_HISTORY_NUMBER,
  XTE_HISTORY_MAXIMUM,
  XTE_HISTORY_CHANGEDATE,
  XTE_HISTORY_PASSWORDS,
  XTE_PASSWORD_POLICY,
  XTE_POLICY_PRESENT,
  XTE_POLICY_LENGTH,
  XTE_POLICY_NUMBER_LOWERCASE,
  XTE_POLICY_NUMBER_UPPERCASE,
  XTE_POLICY_NUMBER_DIGITS,
  XTE_POLICY_NUMBER_SYMBOLS,
  XTE_POLICY_EASYVISION,
  XTE_POLICY_PRONOUNCEABLE,
  XTE_POLICY_HEXADECIMAL,
  XTE_ENTRYTYPE,
  XTE_UNKNOWNFIELDS,

  // others
  XTE_TEST,
  XTE_RULE,
  XTE_LOGIC,

  XTE_STRING,
  XTE_CASE,
  XTE_WARN,
  XTE_NUM1,
  XTE_NUM2,
  XTE_DATE1,
  XTE_DATE2,
  XTE_TYPE,

  // Last element
  XTE_LAST_ELEMENT
};

// Xml filTer Ruletypes = 'XTR' prefix
enum XTT_RULETYPES {
  XTR_NA                  = 0x8000,
  XTR_BOOLEANACTIVERULE   = 0x4000,
  XTR_BOOLEANPRESENTRULE  = 0x2000,
  XTR_BOOLEANSETRULE      = 0x1000,
  XTR_DATERULE            = 0x0800,
  XTR_ENTRYRULE           = 0x0400,
  XTR_INTEGERRULE         = 0x0200,
  XTR_PASSWORDRULE        = 0x0100,
  XTR_PASSWORDHISTORYRULE = 0x0080,
  XTR_PASSWORDPOLICYRULE  = 0x0040,
  XTR_STRINGRULE          = 0x0020,
  XTR_STRINGPRESENTRULE   = 0x0010
};

// XML Test Types
// Xml filTer Testtypes = 'XTT' prefix
enum XTT_TESTTYPES {
  XTT_NA            = 0,
  XTT_MATCHPASSWORD,
  XTT_MATCHSTRING,
  XTT_MATCHINTEGER,
  XTT_MATCHDATE,
  XTT_MATCHENTRY
};

// XML Data Types
// Xml filTer Datatypes = 'XTD' prefix
enum XTD_DATATYPES {
  // Not applicable - i.e. high level element with no actual value
  XTD_NA = 0,

  // Standard types xs:?????
  XTD_XS_DATE,
  XTD_XS_INT,
  XTD_XS_POSITIVEINTEGER,
  XTD_XS_STRING,

  // PWS filter types
  XTD_BOOLTYPE,
  XTD_ENTRYTYPE,
  XTD_FILEUUIDTYPE,          // defined but the corresponding field is not used
  XTD_LOGICTYPE,
  XTD_NONBLANKSTRINGTYPE,
  XTD_YESNOSTRINGTYPE
};

#endif /* _EFILTERVALIDATORDEFS_H_ */
