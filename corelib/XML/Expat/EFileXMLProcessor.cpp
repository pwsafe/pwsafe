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
*
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// PWS includes
#include "EFileXMLProcessor.h"
#include "EFileHandlers.h"
#include "ESecMemMgr.h"

#include "../../corelib.h"
#include "../../PWScore.h"
#include "../../UnknownField.h"

#include <sys/types.h>
#include <sys/stat.h>

// Expat includes
#include <expat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUFFSIZE 8192

// File Handler Wrappers as Expat is a C not a C++ interface
static EFileHandlers *pFileHandler(NULL);

void WstartFileElement(void *userdata, const XML_Char *name,
                  const XML_Char **attrs)
{
  ASSERT(pFileHandler);
  pFileHandler->startElement(userdata, name, attrs);
}

void WendFileElement(void *userdata, const XML_Char *name)
{
  ASSERT(pFileHandler);
  pFileHandler->endElement(userdata, name);
}

void WcharacterFileData(void *userdata, const XML_Char *s, int length)
{
  ASSERT(pFileHandler);
  pFileHandler->characterData(userdata, s, length);
}

// Secure Memory Wrappers as Expat is a C not a C++ interface
static ESecMemMgr *pSecMM(NULL);

void* WFile_malloc(size_t size)
{
  ASSERT(pSecMM);
  return pSecMM->malloc(size);
}

void* WFile_realloc(void *p, size_t size)
{
  ASSERT(pSecMM);
  return pSecMM->realloc(p, size);
}

void WFile_free(void *p)
{
  ASSERT(pSecMM);
  pSecMM->free(p);
}

EFileXMLProcessor::EFileXMLProcessor(PWScore *core,
                                     UUIDList *possible_aliases,
                                     UUIDList *possible_shortcuts)
  : m_xmlcore(core), m_delimiter(TCHAR('^')),
    m_possible_aliases(possible_aliases), m_possible_shortcuts(possible_shortcuts)
{
  pSecMM = new ESecMemMgr;
  pFileHandler = new EFileHandlers;
}

EFileXMLProcessor::~EFileXMLProcessor()
{
  if (pFileHandler) {
    delete pFileHandler;
    pFileHandler = NULL;
  }
  // Should be after freeing of handlers
  if (pSecMM) {
    delete pSecMM;
    pSecMM = NULL;
  }
}

// ---------------------------------------------------------------------------
bool EFileXMLProcessor::Process(const bool &bvalidation,
                                const stringT &ImportedPrefix,
                                const stringT &strXMLFileName,
                                const stringT & /* XML Schema File Name */,
                                int &nITER,
                                int &nRecordsWithUnknownFields,
                                UnknownFieldList &uhfl)
{
  // Open the file
  std::FILE *fd = NULL;
#if _MSC_VER >= 1400
  _tfopen_s(&fd, strXMLFileName.c_str(), _T("r"));
#else
  fd = _tfopen(strXMLFileName.c_str(), _T("r"));
#endif
  if (fd == NULL)
    return false;

  bool bEerrorOccurred = false;
  bool b_into_empty = m_xmlcore->GetNumEntries() == 0;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);
  m_strResultText = _T("");

  pFileHandler->SetVariables(bvalidation ? NULL : m_xmlcore, bvalidation, 
                             ImportedPrefix, m_delimiter,
                             bvalidation ? NULL : m_possible_aliases, 
                             bvalidation ? NULL : m_possible_shortcuts);

  // Tell Expat about our memory suites
  XML_Memory_Handling_Suite ms = {WFile_malloc, WFile_realloc, WFile_free};

  //  Create a parser object.
  XML_Parser pParser = XML_ParserCreate_MM(NULL, &ms, NULL);

  // Set non-default features
  XML_SetParamEntityParsing(pParser, XML_PARAM_ENTITY_PARSING_NEVER);

  // Create handler object and install it on the Parser, as the
  // document Handler.
  XML_SetElementHandler(pParser, WstartFileElement, WendFileElement);
  XML_SetCharacterDataHandler(pParser, WcharacterFileData);
  // Set userdata paraemter for handlers to be the parser itself
  XML_UseParserAsHandlerArg(pParser);

  void * buffer = pSecMM->malloc(BUFFSIZE);
  int done(0), numread;
  enum XML_Status status;
  while (done == 0) {
    numread = (int)fread(buffer, 1, BUFFSIZE, fd);
    done = feof(fd);

    status = XML_Parse(pParser, (char *)buffer, numread, done);
    if (status == XML_STATUS_ERROR || status == XML_STATUS_SUSPENDED) {
      bEerrorOccurred = true;
      break;
    }
  };
  pSecMM->free(buffer);
  fclose(fd);

  if (bvalidation)
    m_numEntriesValidated = pFileHandler->getNumEntries();
  else
    m_numEntriesImported = pFileHandler->getNumEntries();

  if (pFileHandler->getIfErrors() || bEerrorOccurred) {
    bEerrorOccurred = true;
    Format(m_strResultText, IDSC_EXPATPARSEERROR,
           XML_GetCurrentLineNumber(pParser),
           XML_GetCurrentColumnNumber(pParser),
        /* pFileHandler->getErrorCode(), */
           pFileHandler->getErrorMessage().c_str(),
           XML_ErrorString(XML_GetErrorCode(pParser)));
  } else {
    if (!bvalidation) {
      TCHAR delimiter = pFileHandler->getDelimiter();
      if (delimiter != _T('\0'))
        m_delimiter = delimiter;

      // Now add entries
      pFileHandler->AddEntries();

      // Maybe import errors (PWHistory field processing)
      m_strResultText = pFileHandler->getErrorMessage();

      m_bRecordHeaderErrors = pFileHandler->getRecordHeaderErrors();
      nRecordsWithUnknownFields = pFileHandler->getNumRecordsWithUnknownFields();

      if (b_into_empty) {
        m_bDatabaseHeaderErrors = pFileHandler->getDatabaseHeaderErrors();
        nITER = pFileHandler->getNumIterations();
        pFileHandler->AddDBUnknownFieldsPreferences(uhfl);
      } else
        m_bDatabaseHeaderErrors = false;
    }
  }

  // Free the parser
  XML_ParserFree(pParser);

  return !bEerrorOccurred;
}

#endif /* USE_XML_LIBRARY == EXPAT */
