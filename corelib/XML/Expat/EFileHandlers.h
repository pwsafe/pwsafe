/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Expat library V2.0.1 released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
*
* NOTE: EXPAT is a NON-validating XML Parser.  All conformity with the
* scheam must be performed in the handlers.  Also, the concept of pre-validation
* before importing is not available.
* As per XML parsing rules, any error stops the parsing immediately.
*/

#ifndef __EFILEHANDLERS_H
#define __EFILEHANDLERS_H

#include "EFileValidator.h"

// PWS includes
#include "../../StringX.h"
#include "../../ItemData.h"
#include "../../UUIDGen.h"
#include "../../UnknownField.h"
#include "../../PWScore.h"

#include <stack>

// Expat includes
#include <expat.h>

// Entry types
enum {NORMAL = 0, ALIAS, SHORTCUT};

// New imported entry
struct pw_entry {
  StringX group;
  StringX title;
  StringX username;
  StringX password;
  StringX url;
  StringX autotype;
  StringX ctime;
  StringX atime;
  StringX xtime;
  StringX xtime_interval;
  StringX pmtime;
  StringX rmtime;
  StringX pwhistory;
  StringX notes;
  StringX uuid;
  PWPolicy pwp;
  UnknownFieldList uhrxl;  // Note: use header format for record unknown fields!
  int entrytype;
  bool bforce_normal_entry;
};

struct pwhistory_entry {
  StringX changed;
  StringX oldpassword;
};

typedef std::vector<pw_entry *> vdb_entries;

class EFileHandlers
{
public:
  EFileHandlers();
  virtual ~EFileHandlers();

  // -----------------------------------------------------------------------
  //  Handlers for the ContentHandler interface
  // -----------------------------------------------------------------------
  void XMLCALL startElement(void *userdata, const XML_Char *name,
                            const XML_Char **attrs);
  void XMLCALL endElement(void *userdata, const XML_Char *name);
  void XMLCALL characterData(void *userdata, const XML_Char *s,
                             int length);
  void SetMode(const bool bValidation)
  {m_bValidation = bValidation; m_numEntries = 0;}

  bool getIfErrors() {return m_bErrors;}
  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMessage() {return m_strErrorMessage;}

  inline int getXMLPref(int num)
  {
    ASSERT(num >= 0 && num < NUMPREFSINXML);
    return prefsinXML[num];
  }

  vdb_entries & getVDB_Entries() {return ventries;}
  stringT getDefaultAutotypeString() {return m_sDefaultAutotypeString;}
  stringT getDefaultUsername() {return m_sDefaultUsername;}
  TCHAR getDelimiter() {return m_delimiter;}
  int getNumEntries() {return m_numEntries;}
  int getNumIterations() {return m_nITER;}
  int getNumRecordsWithUnknownFields() {return m_nRecordsWithUnknownFields;}
  bool getDatabaseHeaderErrors() {return m_bDatabaseHeaderErrors;}
  bool getRecordHeaderErrors() {return m_bRecordHeaderErrors;}

  UnknownFieldList m_ukhxl;  // For header unknown fields

private:
  // Local variables
  EFileValidator *m_pValidator;

  vdb_entries ventries;
  pw_entry *cur_entry;
  pwhistory_entry *cur_pwhistory_entry;
  std::stack<int> m_element_datatypes;

  StringX m_strElemContent;
  int m_whichtime, m_ipwh;
  bool m_bentrybeingprocessed;
  bool m_bheader;
  unsigned char m_ctype;
  unsigned char * m_pfield;
  int m_fieldlen;

  // Preferences possibly stored in database
  // Note: boolean is integer to allow an 'not set' value of '-1'
  int prefsinXML[NUMPREFSINXML];
  stringT m_sDefaultAutotypeString;
  stringT m_sDefaultUsername;
  int m_nITER;
  int m_numEntries;

  stringT m_strErrorMessage;
  TCHAR m_delimiter;
  int m_nRecordsWithUnknownFields;
  int m_iErrorCode;
  bool m_bValidation;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
  bool m_bErrors;
};

#endif /* __EFILEHANDLERS_H */
