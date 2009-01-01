/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#ifndef __EFILEXMLPROCESSOR_H
#define __EFILEXMLPROCESSOR_H

// PWS includes
#include "EFileHandlers.h"

#include "../../UnknownField.h"
#include "../../UUIDGen.h"
#include "../../os/typedefs.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

// Expat includes
#include <expat.h>

typedef std::vector<CUUIDGen> UUIDList;

class PWScore;

class EFileXMLProcessor
{
public:
  EFileXMLProcessor(PWScore *core, UUIDList *possible_aliases, UUIDList *possible_shortcuts);
  ~EFileXMLProcessor();

  bool Process(const bool &bvalidation, const stringT &ImportedPrefix,
               const stringT &strXMLFileName, const stringT & /* XML Schema */,
               int &nITER, int &nRecordsWithUnknownFields, UnknownFieldList &uhfl);

  stringT getResultText() {return m_strResultText;}
  int getNumEntriesValidated() {return m_numEntriesValidated;}
  int getNnumEntriesImported() {return m_numEntriesImported;}
  bool getIfDatabaseHeaderErrors() {return m_bDatabaseHeaderErrors;}
  bool getIfRecordHeaderErrors() {return m_bRecordHeaderErrors;}

private:
  void AddEntries();

  PWScore *m_xmlcore;
  UUIDList *m_possible_aliases;
  UUIDList *m_possible_shortcuts;
  stringT m_strResultText;
  stringT m_ImportedPrefix;
  stringT m_strImportErrors;
  int m_numEntriesValidated, m_numEntriesImported;
  TCHAR m_delimiter;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
};

#endif /* __EFILEXMLPROCESSOR_H */
