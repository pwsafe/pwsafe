/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes File XML using the STANDARD and UNMODIFIED
* Xerces library V3.1.1 released on April 27, 2010
*
* See http://xerces.apache.org/xerces-c/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
* To use the static version, the following pre-processor statement
* must be defined: XERCES_STATIC_LIBRARY
*
*/

/*
* NOTE: Xerces characters are ALWAYS in UTF-16 (may or may not be wchar_t 
* depending on platform).
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.
*/

#ifndef __XFILEXMLPROCESSOR_H
#define __XFILEXMLPROCESSOR_H

// PWS includes
#include "XFileSAX2Handlers.h"

#include "../../UnknownField.h"
#include "os/typedefs.h"
#include "os/UUID.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

// Xerces includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

class PWScore;

class XFileXMLProcessor
{
public:
  XFileXMLProcessor(PWScore *pcore, UUIDVector *pPossible_Aliases,
                    UUIDVector *pPossible_Shortcuts, MultiCommands *p_multicmds,
                    CReport *prpt);
  ~XFileXMLProcessor();

  bool Process(const bool &bvalidation, const stringT &ImportedPrefix, 
               const stringT &strXMLFileName, const stringT &strXSDFileName,
               const bool &bImportPSWDsOnly);

  stringT getXMLErrors() {return m_strXMLErrors;}
  stringT getRenameList() {return m_strRenameList;}
  stringT getPWHErrorList() {return m_strPWHErrorList;}
  stringT getSkippedList() {return m_strSkippedList;}

  int getNumEntriesValidated() {return m_numEntriesValidated;}
  int getNumEntriesImported() {return m_numEntriesImported;}
  int getNumEntriesSkipped() {return m_numEntriesSkipped;}
  int getNumEntriesRenamed() {return m_numEntriesRenamed;}
  int getNumEntriesPWHErrors() {return m_numEntriesPWHErrors;}
  int getNumNoPolicies() {return m_numNoPolicies;}
  int getNumRenamedPolicies() const {return m_numRenamedPolicies;}
  int getNumShortcutsRemoved() const {return m_numShortcutsRemoved;}
  int getNumEmptyGroupsImported() const {return m_numEmptyGroupsImported;}

private:
  PWScore *m_pXMLcore;
  UUIDVector *m_pPossible_Aliases;
  UUIDVector *m_pPossible_Shortcuts;
  MultiCommands *m_pmulticmds;
  CReport *m_prpt;

  stringT m_strXMLErrors, m_strSkippedList, m_strRenameList, m_strPWHErrorList;
  int m_numEntriesValidated, m_numEntriesImported, m_numEntriesSkipped;
  int m_numEntriesPWHErrors, m_numEntriesRenamed;
  int m_numRenamedPolicies, m_numNoPolicies;
  int m_numShortcutsRemoved, m_numEmptyGroupsImported;
  TCHAR m_delimiter;
  bool m_bValidation;
};

#endif /* __XFILEXMLPROCESSOR_H */
