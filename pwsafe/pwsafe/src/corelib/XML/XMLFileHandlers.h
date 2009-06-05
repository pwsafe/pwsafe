/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLFILEHANDLERS_H
#define __XMLFILEHANDLERS_H

// PWS includes
#include "../ItemData.h"
#include "../UUIDGen.h"
#include "../UnknownField.h"

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
  StringX run_command;
  StringX dca;
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

class PWScore;

class XMLFileHandlers
{
  // to allow access to protected members
#if   USE_XML_LIBRARY == EXPAT
  friend class EFileXMLProcessor;
#elif USE_XML_LIBRARY == MSXML
  friend class MFileXMLProcessor;
#elif USE_XML_LIBRARY == XERCES
  friend class XFileXMLProcessor;
#endif

public:
  XMLFileHandlers();
  virtual ~XMLFileHandlers();

  void SetVariables(PWScore *core, const bool &bValidation,
                    const stringT &ImportedPrefix, const TCHAR &delimiter,
                    UUIDList *possible_aliases, UUIDList *possible_shortcuts);

  bool getIfErrors() {return m_bErrors;}
  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMessage() {return m_strErrorMessage;}
  stringT getImportErrors() {return m_strImportErrors;}

  vdb_entries & getVDB_Entries() {return ventries;}
  stringT getDefaultAutotypeString() {return m_sDefaultAutotypeString;}
  stringT getDefaultUsername() {return m_sDefaultUsername;}
  TCHAR getDelimiter() {return m_delimiter;}
  int getNumEntries() {return m_numEntries;}
  int getNumIterations() {return m_nITER;}
  int getNumRecordsWithUnknownFields() {return m_nRecordsWithUnknownFields;}
  bool getDatabaseHeaderErrors() {return m_bDatabaseHeaderErrors;}
  bool getRecordHeaderErrors() {return m_bRecordHeaderErrors;}

protected:
  bool ProcessStartElement(const int icurrent_element);
  void ProcessEndElement(const int icurrent_element);
  void AddEntries();
  void AddDBUnknownFieldsPreferences(UnknownFieldList &uhfl);

  vdb_entries ventries;
  pw_entry *cur_entry;
  pwhistory_entry *cur_pwhistory_entry;

  StringX m_strElemContent;
  stringT m_strImportErrors;
  stringT m_strErrorMessage;
  int m_nITER;
  int m_numEntries;
  int m_nRecordsWithUnknownFields;
  int m_iErrorCode;
  TCHAR m_delimiter;
  bool m_bentrybeingprocessed;
  bool m_bValidation;
  bool m_bErrors, m_bRecordHeaderErrors, m_bDatabaseHeaderErrors;
  unsigned char m_ctype;

  UnknownFieldList m_ukhxl;  // For header unknown fields

private:
  // Local variables
  PWScore *m_xmlcore;
  UUIDList *m_possible_aliases;
  UUIDList *m_possible_shortcuts;

  int m_whichtime, m_ipwh;
  int m_fieldlen;
  bool m_bheader;
  unsigned char * m_pfield;

  // Preferences possibly stored in database
  // Note: boolean is integer to allow an 'not set' value of '-1'
  int prefsinXML[NUMPREFSINXML];
  stringT m_ImportedPrefix;
  stringT m_sDefaultAutotypeString;
  stringT m_sDefaultUsername;
};

#endif /* __XMLFILEHANDLERS_H */
