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

// Elements stack value
// Xml fiLe Elements = 'XLE' prefix

enum XLE_PASSWORDSAFE {
  XLE_PASSWORDSAFE            = 1,
  XLE_NUMBERHASHITERATIONS,
  XLE_PREFERENCES,
  XLE_UNKNOWNHEADERFIELDS,
  XLE_ENTRY,

  // Preferences
  XLE_DISPLAYEXPANDEDADDEDITDLG,
  XLE_MAINTAINDATETIMESTAMPS,
  XLE_PWUSEDIGITS,
  XLE_PWUSEEASYVISION,
  XLE_PWUSEHEXDIGITS,
  XLE_PWUSELOWERCASE,
  XLE_PWUSESYMBOLS,
  XLE_PWUSEUPPERCASE,
  XLE_PWMAKEPRONOUNCEABLE,
  XLE_SAVEIMMEDIATELY,
  XLE_SAVEPASSWORDHISTORY,
  XLE_SHOWNOTESDEFAULT,
  XLE_SHOWPWDEFAULT,
  XLE_SHOWPASSWORDINTREE,
  XLE_SHOWUSERNAMEINTREE,
  XLE_SORTASCENDING,
  XLE_USEDEFAULTUSER,
  XLE_PWDEFAULTLENGTH,
  XLE_IDLETIMEOUT,
  XLE_TREEDISPLAYSTATUSATOPEN,
  XLE_NUMPWHISTORYDEFAULT,
  XLE_PWDIGITMINLENGTH,
  XLE_PWLOWERCASEMINLENGTH,
  XLE_PWSYMBOLMINLENGTH,
  XLE_PWUPPERCASEMINLENGTH,
  XLE_DEFAULTUSERNAME,
  XLE_DEFAULTAUTOTYPESTRING,

  // unknownheaderfields
  XLE_HFIELD,

  // entry
  XLE_GROUP,
  XLE_TITLE,
  XLE_USERNAME,
  XLE_PASSWORD,
  XLE_URL,
  XLE_AUTOTYPE,
  XLE_NOTES,
  XLE_UUID,
  XLE_CTIME,
  XLE_ATIME,
  XLE_LTIME,
  XLE_XTIME,
  XLE_XTIME_INTERVAL,
  XLE_PMTIME,
  XLE_RMTIME,
  XLE_PWHISTORY,
  XLE_ENTRY_PASSWORDPOLICY,
  XLE_UNKNOWNRECORDFIELDS,

  // pwhistory
  XLE_STATUS,
  XLE_MAX,
  XLE_NUM,
  XLE_HISTORY_ENTRIES,

  // history_entries
  XLE_HISTORY_ENTRY,

  // history_entry
  XLE_CHANGED,
  XLE_OLDPASSWORD,

  // entry_PasswordPolicy
  XLE_ENTRY_PWLENGTH,
  XLE_ENTRY_PWUSEDIGITS,
  XLE_ENTRY_PWUSEEASYVISION,
  XLE_ENTRY_PWUSEHEXDIGITS,
  XLE_ENTRY_PWUSELOWERCASE,
  XLE_ENTRY_PWUSESYMBOLS,
  XLE_ENTRY_PWUSEUPPERCASE,
  XLE_ENTRY_PWMAKEPRONOUNCEABLE,
  XLE_ENTRY_PWLOWERCASEMINLENGTH,
  XLE_ENTRY_PWUPPERCASEMINLENGTH,
  XLE_ENTRY_PWDIGITMINLENGTH,
  XLE_ENTRY_PWSYMBOLMINLENGTH,

  // unknownrecordfields
  XLE_RFIELD,

  // datetime fields
  XLE_DATE,
  XLE_TIME,

  // Last element
  XLE_LAST_ELEMENT
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
