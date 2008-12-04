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
* As per XML parsing rules, any error stops the parsing immediately.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// PWS includes
#include "EFilterXMLProcessor.h"
#include "EFilterHandlers.h"
#include "ESecMemMgr.h"

#include "../../corelib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <algorithm>

// Expat includes
#include <expat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUFFSIZE 8192

// File Handler Wrappers as Expat is a C not a C++ interface
static EFilterHandlers* pFilterHandler(NULL);

void WstartFilterElement(void *userdata, const XML_Char *name,
                  const XML_Char **attrs)
{
  ASSERT(pFilterHandler);
  pFilterHandler->startElement(userdata, name, attrs);
}

void WendFilterElement(void *userdata, const XML_Char *name)
{
  ASSERT(pFilterHandler);
  pFilterHandler->endElement(userdata, name);
}

void WcharacterFilterData(void *userdata, const XML_Char *s, int length)
{
  ASSERT(pFilterHandler);
  pFilterHandler->characterData(userdata, s, length);
}

// Secure Memory Wrappers as Expat is a C not a C++ interface
ESecMemMgr* pSecMM;

void* WFilter_malloc(size_t size)
{
  ASSERT(pSecMM);
  return pSecMM->malloc(size);
}

void* WFilter_realloc(void *p, size_t size)
{
  ASSERT(pSecMM);
  return pSecMM->realloc(p, size);
}

void WFilter_free(void *p)
{
  ASSERT(pSecMM);
  pSecMM->free(p);
}

EFilterXMLProcessor::EFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool,
                                         Asker *pAsker)
  : m_MapFilters(mapfilters), m_FPool(fpool), m_pAsker(pAsker)
{
  pSecMM = new ESecMemMgr;
  pFilterHandler = new EFilterHandlers;
}

EFilterXMLProcessor::~EFilterXMLProcessor()
{
  if (pFilterHandler) {
    delete pFilterHandler;
    pFilterHandler = NULL;
  }
  // Should be after freeing of handlers
  if (pSecMM) {
    delete pSecMM;
    pSecMM = NULL;
  }
}

bool EFilterXMLProcessor::Process(const bool &bvalidation,
                                  const StringX &strXMLData,
                                  const stringT &strXMLFileName,
                                  const stringT & /* XML Schema File Name */)
{
  bool bEerrorOccurred = false;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);
  m_strResultText = _T("");
  m_bValidation = bvalidation;  // Validate or Import

  XML_Memory_Handling_Suite ms = {WFilter_malloc, WFilter_realloc, WFilter_free};

  //  Create a parser object.
  XML_Parser pParser = XML_ParserCreate_MM(NULL, &ms, NULL);

  // Set non-default features
  XML_SetParamEntityParsing(pParser, XML_PARAM_ENTITY_PARSING_NEVER);

  // Create handler object and install it on the Parser, as the
  // document Handler.
  XML_SetElementHandler(pParser, WstartFilterElement, WendFilterElement);
  XML_SetCharacterDataHandler(pParser, WcharacterFilterData);
  XML_UseParserAsHandlerArg(pParser);

  pFilterHandler->SetVariables(m_pAsker, &m_MapFilters, m_FPool, m_bValidation);

  enum XML_Status status;
  if (!strXMLFileName.empty()) {
    // Parse the file
    std::FILE *fd = NULL;
#if _MSC_VER >= 1400
    _tfopen_s(&fd, strXMLFileName.c_str(), _T("r"));
#else
    fd = _tfopen(strXMLFileName.c_str(), _T("r"));
#endif
    if (fd == NULL)
      return false;

    void *buffer = pSecMM->malloc(BUFFSIZE);
    int done(0), numread;
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
  } else {
    // Parse in memory 'file'
    StringX sbuffer(strXMLData);
#ifdef UNICODE
    StringX::size_type ipos;
    ipos = sbuffer.find(_T("UTF-8"));
    if (ipos != stringT::npos)
      sbuffer.replace(ipos, 5, _T("UTF-16"));
#endif
    char *buffer = (char *)&sbuffer.at(0);
    status = XML_Parse(pParser, buffer,
                       _tcslen(sbuffer.c_str()) * sizeof(TCHAR), 1);
  }

  if (pFilterHandler->getIfErrors() || bEerrorOccurred) {
    bEerrorOccurred = true;
    Format(m_strResultText, IDSC_EXPATPARSEERROR,
           XML_GetCurrentLineNumber(pParser),
           XML_GetCurrentColumnNumber(pParser),
        /* pFileHandler->getErrorCode(), */
           pFilterHandler->getErrorMessage().c_str(),
           XML_ErrorString(XML_GetErrorCode(pParser)));
  } else {
    //
  }

  // Free the parser
  XML_ParserFree(pParser);

  return !bEerrorOccurred;
}

#endif /* USE_XML_LIBRARY == EXPAT */
