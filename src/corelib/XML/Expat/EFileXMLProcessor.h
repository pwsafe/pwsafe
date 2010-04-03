/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "../../Command.h"
#include "os/typedefs.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

// Expat includes
#include <expat.h>

typedef std::vector<CUUIDGen> UUIDVector;

class PWScore;

class EFileXMLProcessor
{
public:
  EFileXMLProcessor(PWScore *pcore, UUIDVector *pPossible_Aliases, 
                    UUIDVector *pPossible_Shortcuts, MultiCommands *p_multicmds,
                    CReport *prpt);
  ~EFileXMLProcessor();

  bool Process(const bool &bvalidation, const stringT &ImportedPrefix,
               const stringT &strXMLFileName, const stringT & /* XML Schema */,
               const bool &bImportPSWDsOnly,
               int &nITER, int &nRecordsWithUnknownFields, UnknownFieldList &uhfl);

  stringT getXMLErrors() {return m_strXMLErrors;}
  stringT getPWHErrorList() {return m_strPWHErrorList;}
  stringT getRenameList() {return m_strRenameList;}
  stringT getSkippedList() {return m_strSkippedList;}

  int getNumEntriesValidated() {return m_numEntriesValidated;}
  int getNumEntriesImported() {return m_numEntriesImported;}
  int getNumEntriesSkipped() {return m_numEntriesSkipped;}
  int getNumEntriesRenamed() {return m_numEntriesRenamed;}
  int getNumEntriesPWHErrors() {return m_numEntriesPWHErrors;}

  bool getIfDatabaseHeaderErrors() {return m_bDatabaseHeaderErrors;}
  bool getIfRecordHeaderErrors() {return m_bRecordHeaderErrors;}

private:
  PWScore *m_pXMLcore;
  UUIDVector *m_pPossible_Aliases;
  UUIDVector *m_pPossible_Shortcuts;
  MultiCommands *m_pmulticmds;
  CReport *m_prpt;

  stringT m_strXMLErrors;
  stringT m_strRenameList, m_strSkippedList, m_strPWHErrorList;
  int m_numEntriesValidated, m_numEntriesImported, m_numEntriesSkipped;
  int m_numEntriesPWHErrors, m_numEntriesRenamed;
  TCHAR m_delimiter;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
};

#endif /* __EFILEXMLPROCESSOR_H */
