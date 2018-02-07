/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
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

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFilterXMLProcessor.h"
#include "XSecMemMgr.h"

#include "../../StringX.h"
#include "../../core.h"
#include "./XMLChConverter.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <algorithm>
#include <stdexcept>

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

XFilterXMLProcessor::XFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool,
                                         Asker *pAsker)
  : m_pAsker(pAsker), m_MapXMLFilters(mapfilters), m_FPool(fpool)
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
  USES_XMLCH_STR

  bool bErrorOccurred = false;
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
    m_strXMLErrors = stringT(_X2ST(toCatch.getMessage()));
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
  // we need const_cast here, because _W2X return const wchar_t* when
  // WCHAR_INCOMPATIBLE_XMLCH isn't set
  pSAX2Parser->setProperty(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
                      const_cast<XMLCh*>(_W2X(strXSDFileName.c_str())));
  pSAX2Parser->setProperty(XMLUni::fgXercesScannerName,
                      const_cast<XMLCh*>(XMLUni::fgSGXMLScanner));
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

  pSAX2Handler->SetVariables(m_pAsker, &m_MapXMLFilters, m_FPool, m_bValidation);

  // instantiate converter out of if/else to be sure that string will be valid
  // till the end of pSAX2Parser, that may capture pointer to string from MemBufInputSource
  CUTF8Conv conv;
  try
  {
    // Let's begin the parsing now
    if (!strXMLFileName.empty()) {
      pSAX2Parser->parse(_W2X(strXMLFileName.c_str()));
    } else {
      const char *szID = "database_filters";
      // Xerces use encoding from XML (we have set it to utf-8), but transcode() on Windows convert to one-byte cpXXXX,
      // so we need to manually convert from wchar to UTF-8
      const unsigned char* buffer=nullptr;
      size_t len;
      if (!conv.ToUTF8(strXMLData, buffer, len)) {
        throw std::runtime_error("Can't convert data to UTF-8");
      }
      //2nd parameter must be number of bytes, so we use a length for char* representation
      MemBufInputSource* memBufIS = new MemBufInputSource(
                    reinterpret_cast<const XMLByte *>(buffer),
                    strlen(reinterpret_cast<const char*>(buffer)),
                    szID, false);
      pSAX2Parser->parse(*memBufIS);
      delete memBufIS;
    }
  }
  catch (const OutOfMemoryException&)
  {
    LoadAString(strResultText, IDCS_XERCESOUTOFMEMORY);
    bErrorOccurred = true;
  }
  catch (const XMLException& e)
  {
    strResultText = stringT(_X2ST(e.getMessage()));
    bErrorOccurred = true;
  }

  catch (...)
  {
    LoadAString(strResultText, IDCS_XERCESEXCEPTION);
    bErrorOccurred = true;
  }

  if (pSAX2Handler->getIfErrors() || bErrorOccurred) {
    bErrorOccurred = true;
    if (pSAX2Handler->getIfErrors())
      strResultText = pSAX2Handler->getValidationResult();
    Format(m_strXMLErrors, IDSC_XERCESPARSEERROR,
           m_bValidation ? cs_validation.c_str() : cs_import.c_str(),
           strResultText.c_str());
  } else {
    m_strXMLErrors = strResultText;
  }

  //  Delete the pSAX2Parser itself.  Must be done prior to calling Terminate, below.
  delete pSAX2Parser;
  delete pSAX2Handler;

  USES_XMLCH_STR_END

  // And call the termination method
  XMLPlatformUtils::Terminate();

  return !bErrorOccurred;
}

#endif /* USE_XML_LIBRARY == XERCES */
