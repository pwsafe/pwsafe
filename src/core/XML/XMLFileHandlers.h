/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLFILEHANDLERS_H
#define __XMLFILEHANDLERS_H

// PWS includes
#include "../ItemData.h"
#include "../UnknownField.h"
#include "../Command.h"
#include "../Report.h"
#include "os/UUID.h"

#include "XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

// Entry types
enum {NORMAL = 0, ALIAS, SHORTCUT};

// New imported entry
struct pw_entry {
  int id;
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
  StringX shiftdca;
  StringX email;
  StringX symbols;
  StringX policyname;
  StringX kbshortcut;
  unsigned char ucprotected;
  PWPolicy pwp;
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
#if USE_XML_LIBRARY == MSXML
  friend class MFileXMLProcessor;
#elif USE_XML_LIBRARY == XERCES
  friend class XFileXMLProcessor;
#endif

public:
  XMLFileHandlers();
  virtual ~XMLFileHandlers();

  void SetVariables(PWScore *pcore, const bool &bValidation,
                    const stringT &ImportedPrefix, const TCHAR &delimiter,
                    const bool &bImportPSWDsOnly,
                    UUIDVector *pPossible_Aliases, UUIDVector *pPossible_Shortcuts,
                    MultiCommands *pmulticmds, CReport *prpt);

  bool getIfErrors() const {return m_bErrors;}
  int getErrorCode() const {return m_iErrorCode;}
  stringT getErrorMessage() const {return m_strErrorMessage;}
  stringT getXMLErrors() const {return m_strXMLErrors;}
  stringT getSkippedList() const {return m_strSkippedList;}
  stringT getPWHErrorList() const {return m_strPWHErrorList;}
  stringT getRenameList() const {return m_strRenameList;}

  vdb_entries & getVDB_Entries() {return m_ventries;}

  TCHAR getDelimiter() const {return m_delimiter;}

  int getNumEntries() const {return m_numEntries;}
  int getNumSkipped() const {return m_numEntriesSkipped;}
  int getNumRenamed() const {return m_numEntriesRenamed;}
  int getNumPWHErrors() const {return m_numEntriesPWHErrors;}
  int getNumNoPolicies() const {return m_numNoPolicies;}
  int getNumRenamedPolicies() const {return m_numRenamedPolicies;}
  int getNumShortcutsRemoved() const {return m_numShortcutsRemoved;}
  int getNumEmptyGroupsImported() const {return m_numEmptyGroupsImported;}
  bool getDatabaseHeaderErrors() const {return m_bDatabaseHeaderErrors;}
  bool getRecordHeaderErrors() const {return m_bRecordHeaderErrors;}

protected:
  bool ProcessStartElement(const int icurrent_element);
  void ProcessEndElement(const int icurrent_element);
  void AddXMLEntries();
  void AddDBPreferences();

  PWPolicy currentDB_default_pwp, importDB_default_pwp;

  vdb_entries m_ventries;
  pw_entry *m_cur_entry;

  StringX m_sxElemContent;
  stringT m_strErrorMessage;
  stringT m_strXMLErrors;
  stringT m_strPWHErrorList;
  stringT m_strRenameList;
  stringT m_strSkippedList;

  int m_numEntries;
  int m_numEntriesSkipped;
  int m_numEntriesRenamed;
  int m_numEntriesPWHErrors;
  int m_numNoPolicies;
  int m_numRenamedPolicies;
  int m_numShortcutsRemoved;
  int m_numEmptyGroupsImported;
  int m_iErrorCode;
  TCHAR m_delimiter;

  bool m_bEntryBeingProcessed;
  bool m_bPolicyBeingProcessed;
  bool m_bValidation;
  bool m_bInPolicyNames, m_bInEmptyGroups;
  bool m_bErrors, m_bRecordHeaderErrors, m_bDatabaseHeaderErrors;
  bool m_bImportPSWDsOnly;
  unsigned char m_ctype;

  UnknownFieldList m_ukhxl;  // For header unknown fields

private:
  // Local variables
  PWScore *m_pXMLcore;
  UUIDVector *m_pPossible_Aliases;
  UUIDVector *m_pPossible_Shortcuts;
  MultiCommands *m_pmulticmds;
  CReport *m_prpt;

  int m_ipwh;
  int m_fieldlen;
  bool m_bheader;
  unsigned char *m_pfield;

  // Preferences possibly stored in database
  stringT m_ImportedPrefix;

  StringX m_PolicyName;
  PWPolicy m_Named_pwp;
  PSWDPolicyMap m_MapPSWDPLC;
  std::map<StringX, StringX> m_mapRenamedPolicies;
  std::vector<StringX> m_vEmptyGroups;
  StringX m_sxXML_DateTime;
  pwhistory_entry *m_cur_pwhistory_entry;
};

#endif /* __XMLFILEHANDLERS_H */
