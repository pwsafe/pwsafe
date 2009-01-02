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

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFilterXMLProcessor.h"
#include "XSecMemMgr.h"

#include "../../StringX.h"
#include "../../corelib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <algorithm>

// Xerces includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/XMLGrammarDescription.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <fstream>
#else
#include <fstream.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

XFilterXMLProcessor::XFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool,
                                         Asker *pAsker)
  : m_MapFilters(mapfilters), m_FPool(fpool), m_pAsker(pAsker)
{
}

XFilterXMLProcessor::~XFilterXMLProcessor()
{
}

bool XFilterXMLProcessor::Process(const bool &bvalidation,
                                  const StringX &strXMLData,
                                  const stringT &strXMLFileName,
                                  const stringT &strXSDFileName)
{
  bool bEerrorOccurred = false;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);
  stringT strResultText(_T(""));
  m_bValidation = bvalidation;  // Validate or Import

  XSecMemMgr sec_mm;

  // Initialize the XML4C2 system
  try
  {
    XMLPlatformUtils::Initialize(XMLUni::fgXercescDefaultLocale, 0, 0, &sec_mm);
  }
  catch (const XMLException& toCatch)
  {
#ifdef UNICODE
    m_strResultText = stringT(toCatch.getMessage());
#else
    char *szData = XMLString::transcode(toCatch.getMessage());
    strResultText = stringT(szData);
    XMLString::release(&szData);
#endif
    return false;
  }

  //  Create a SAX2 parser object.
  SAX2XMLReader* pSAX2Parser = XMLReaderFactory::createXMLReader(&sec_mm);

  // Set non-default features
  pSAX2Parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, true);
  pSAX2Parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
  pSAX2Parser->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
  pSAX2Parser->setFeature(XMLUni::fgXercesLoadExternalDTD, false);
  pSAX2Parser->setFeature(XMLUni::fgXercesSkipDTDValidation, true);

  // Set properties
  pSAX2Parser->setProperty(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
                      (void *)strXSDFileName.c_str());
  pSAX2Parser->setProperty(XMLUni::fgXercesScannerName,
                      (void *)XMLUni::fgSGXMLScanner);
  pSAX2Parser->setInputBufferSize(4096);

  // Create SAX handler object and install it on the pSAX2Parser, as the
  // document and error pSAX2Handler.
  XFilterSAX2Handlers * pSAX2Handler = new XFilterSAX2Handlers;
  pSAX2Parser->setContentHandler(pSAX2Handler);
  pSAX2Parser->setErrorHandler(pSAX2Handler);

  // Workaround/bypass until Xerces supports retrieving version from the
  // <xs:schema ...> statement!
  // Set 'dummy' schema version to arbitrary value > 1
  pSAX2Handler->SetSchemaVersion(99);

  pSAX2Handler->SetVariables(m_pAsker, &m_MapFilters, m_FPool, m_bValidation);

  try
  {
    // Let's begin the parsing now
    if (!strXMLFileName.empty()) {
      pSAX2Parser->parse(strXMLFileName.c_str());
    } else {
      const char *szID = "database_filters";
#ifdef UNICODE
      const char *buffer = XMLString::transcode(strXMLData.c_str());
#else
      const char *buffer = strXMLData.c_str();
#endif
      MemBufInputSource* memBufIS = new MemBufInputSource(
                    (const XMLByte*)buffer,
                    strXMLData.length(),
                    szID, false);
      pSAX2Parser->parse(*memBufIS);
      delete memBufIS;
#ifdef UNICODE
      XMLString::release((char **)&buffer);
#endif
    }
  }
  catch (const OutOfMemoryException&)
  {
    LoadAString(strResultText, IDCS_XERCESOUTOFMEMORY);
    bEerrorOccurred = true;
  }
  catch (const XMLException& e)
  {
#ifdef UNICODE
    strResultText = stringT(e.getMessage());
#else
    char *szData = XMLString::transcode(e.getMessage());
    strResultText = stringT(szData);
    XMLString::release(&szData);
#endif
    bEerrorOccurred = true;
  }

  catch (...)
  {
    LoadAString(strResultText, IDCS_XERCESEXCEPTION);
    bEerrorOccurred = true;
  }

  if (pSAX2Handler->getIfErrors() || bEerrorOccurred) {
    bEerrorOccurred = true;
    strResultText = pSAX2Handler->getValidationResult();
    Format(m_strResultText, IDSC_XERCESPARSEERROR, 
           m_bValidation ? cs_validation.c_str() : cs_import.c_str()), 
           strResultText.c_str();
  } else {
    m_strResultText = strResultText;
  }

  //  Delete the pSAX2Parser itself.  Must be done prior to calling Terminate, below.
  delete pSAX2Parser;
  delete pSAX2Handler;

  // And call the termination method
  XMLPlatformUtils::Terminate();

  return !bEerrorOccurred;
}

#endif /* USE_XML_LIBRARY == XERCES */
