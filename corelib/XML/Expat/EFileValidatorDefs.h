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

#ifndef _EFILEVALIDATORDEFS_H_
#define _EFILEVALIDATORDEFS_H_

// Error codes
// Xml fiLe Parsing Error Codes = 'XLPEC' prefix

enum ErrorCodes {
  XLPEC_EXCEEDED_MAXOCCURS    = 1,
  XLPEC_UNEXPECTED_ELEMENT,
  XLPEC_MISSING_MANDATORY_FIELD,
  XLPEC_MISSING_ELEMENT,
  XLPEC_INVALID_DATA,
  XLPEC_UNKNOWN_FIELD,
  XLPEC_MISSING_DELIMITER_ATTRIBUTE
};

// XML Data Types
// Xml fiLe Datatypes = 'XLD' prefix
enum XLD_DATATYPES {
  // Not applicable - i.e. high level element with no actual value
  XLD_NA = 0,

  // Standard types xs:?????
  XLD_XS_BASE64BINARY,
  XLD_XS_DATE,
  XLD_XS_DATETIME,           // defined but the corresponding field is not used
  XLD_XS_HEXBINARY,
  XLD_XS_INT,
  XLD_XS_STRING,
  XLD_XS_TIME,

  // PWS types
  XLD_BOOLTYPE,
  XLD_CHARACTERTYPE,
  XLD_DATETIMESTAMPTYPE,     // defined but the corresponding field is not used
  XLD_DISPLAYSTATUSTYPE,
  XLD_EXPIRYDAYSTYPE,
  XLD_FIELDTYPE,
  XLD_FILEUUIDTYPE,          // defined but the corresponding field is not used
  XLD_NUMHASHTYPE,
  XLD_PASSWORDLENGTHTYPE,
  XLD_PASSWORDLENGTHTYPE2,
  XLD_PWHISTORYTYPE,
  XLD_TIMEOUTTYPE,
  XLD_UUIDTYPE
};

#endif /* _EFILEVALIDATORDEFS_H_ */
