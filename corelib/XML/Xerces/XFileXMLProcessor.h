/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.0.0 released on September 29, 2008
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
#include "../../UUIDGen.h"
#include "../../os/typedefs.h"

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

typedef std::vector<CUUIDGen> UUIDList;

class PWScore;

class XFileXMLProcessor
{
public:
  XFileXMLProcessor(PWScore *core, UUIDList *possible_aliases, UUIDList *possible_shortcuts);
  ~XFileXMLProcessor();

  bool Process(const bool &bvalidation, const stringT &ImportedPrefix, 
               const stringT &strXMLFileName, const stringT &strXSDFileName,
               int &nITER, int &nRecordsWithUnknownFields, UnknownFieldList &uhfl);

  stringT getResultText() {return m_strResultText;}
  int getNumEntriesValidated() {return m_numEntriesValidated;}
  int getNnumEntriesImported() {return m_numEntriesImported;}
  bool getIfDatabaseHeaderErrors() {return m_bDatabaseHeaderErrors;}
  bool getIfRecordHeaderErrors() {return m_bRecordHeaderErrors;}

private:
  PWScore *m_xmlcore;
  UUIDList *m_possible_aliases;
  UUIDList *m_possible_shortcuts;
  stringT m_strResultText;
  int m_numEntriesValidated, m_numEntriesImported;
  TCHAR m_delimiter;
  bool m_bValidation;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
};

#endif /* __XFILEXMLPROCESSOR_H */
