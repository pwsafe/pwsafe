/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFileXMLProcessor.h"
#include "XFileSAX2Handlers.h"
#include "XSecMemMgr.h"

#include "../../ItemData.h"
#include "../../core.h"
#include "../../PWScore.h"
#include "../../UnknownField.h"
#include "../../PWSprefs.h"

#include <sys/types.h>
#include <sys/stat.h>

// Xerces includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/XMLGrammarDescription.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <fstream>
#else
#include <fstream.h>
#endif

#include "./XMLChConverter.h"

XFileXMLProcessor::XFileXMLProcessor(PWScore *pcore,
                                     UUIDVector *pPossible_Aliases,
                                     UUIDVector *pPossible_Shortcuts,
                                     MultiCommands *p_multicmds,
                                     CReport *prpt)
  : m_pXMLcore(pcore),
    m_pPossible_Aliases(pPossible_Aliases), m_pPossible_Shortcuts(pPossible_Shortcuts),
    m_pmulticmds(p_multicmds), m_prpt(prpt), m_delimiter(TCHAR('^'))
{
}

XFileXMLProcessor::~XFileXMLProcessor()
{
}

// ---------------------------------------------------------------------------
bool XFileXMLProcessor::Process(const bool &bvalidation, const stringT &ImportedPrefix,
                                const stringT &strXMLFileName, const stringT &strXSDFileName,
                                const bool &bImportPSWDsOnly)
{
  USES_XMLCH_STR

  bool bErrorOccurred = false;
  bool b_into_empty = false;
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
    strResultText = stringT(_X2ST(toCatch.getMessage()));
    return false;
  }

  const XMLCh* xmlfilename = _W2X(strXMLFileName.c_str());
  const XMLCh* schemafilename = _W2X(strXSDFileName.c_str());

  //  Create a SAX2 parser object.
  SAX2XMLReader* pSAX2Parser = XMLReaderFactory::createXMLReader(&sec_mm);

  // Set non-default features
  pSAX2Parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, true);
  pSAX2Parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
  pSAX2Parser->setFeature(XMLUni::fgXercesDynamic, false);
  pSAX2Parser->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
  pSAX2Parser->setFeature(XMLUni::fgXercesLoadExternalDTD, false);
  pSAX2Parser->setFeature(XMLUni::fgXercesSkipDTDValidation, true);

  // Set properties
  pSAX2Parser->setProperty(XMLUni::fgXercesScannerName,
                           const_cast<void*>(reinterpret_cast<const void*>(XMLUni::fgSGXMLScanner)));
  pSAX2Parser->setInputBufferSize(4096);

  // Set schema file name (also via property)
  pSAX2Parser->setProperty(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
                           const_cast<void*>(reinterpret_cast<const void*>(schemafilename)));

  // Create SAX handler object and install it on the pSAX2Parser, as the
  // document and error pSAX2Handler.
  XFileSAX2Handlers * pSAX2Handler = new XFileSAX2Handlers;
  pSAX2Parser->setContentHandler(pSAX2Handler);
  pSAX2Parser->setErrorHandler(pSAX2Handler);

  pSAX2Handler->SetVariables(m_bValidation ? nullptr : m_pXMLcore, m_bValidation,
                             ImportedPrefix, m_delimiter, bImportPSWDsOnly,
                             m_bValidation ? nullptr : m_pPossible_Aliases,
                             m_bValidation ? nullptr : m_pPossible_Shortcuts,
                             m_pmulticmds, m_prpt);
  if (!m_bValidation) {
    b_into_empty = m_pXMLcore->GetNumEntries() == 0;
  }

  try {
    // Let's begin the parsing now
    pSAX2Parser->parse(xmlfilename);
  }
  catch (const OutOfMemoryException&) {
    LoadAString(strResultText, IDCS_XERCESOUTOFMEMORY);
    bErrorOccurred = true;
  }
  catch (const XMLException& e) {
    strResultText = stringT(_X2ST(e.getMessage()));
    bErrorOccurred = true;
  }

  catch (...) {
    LoadAString(strResultText, IDCS_XERCESEXCEPTION);
    bErrorOccurred = true;
  }

  if (pSAX2Handler->getIfErrors() || bErrorOccurred) {
    bErrorOccurred = true;
    strResultText = pSAX2Handler->getValidationResult();
    Format(m_strXMLErrors, IDSC_XERCESPARSEERROR,
           m_bValidation ? cs_validation.c_str() : cs_import.c_str(),
           strResultText.c_str());
  } else {
    if (m_bValidation) {
      m_strXMLErrors = pSAX2Handler->getValidationResult();
      m_numEntriesValidated = pSAX2Handler->getNumEntries();
      m_delimiter = pSAX2Handler->getDelimiter();
    } else {
      pSAX2Handler->AddXMLEntries();

      // Get numbers (may have been modified by AddXMLEntries)
      m_numEntriesImported = pSAX2Handler->getNumEntries();
      m_numEntriesSkipped = pSAX2Handler->getNumSkipped();
      m_numEntriesRenamed = pSAX2Handler->getNumRenamed();
      m_numEntriesPWHErrors = pSAX2Handler->getNumPWHErrors();
      m_numNoPolicies = pSAX2Handler->getNumNoPolicies();
      m_numRenamedPolicies = pSAX2Handler->getNumRenamedPolicies();
      m_numShortcutsRemoved = pSAX2Handler->getNumShortcutsRemoved();
      m_numEmptyGroupsImported = pSAX2Handler->getNumEmptyGroupsImported();

      // Get lists
      m_strXMLErrors = pSAX2Handler->getXMLErrors();
      m_strSkippedList = pSAX2Handler->getSkippedList();
      m_strPWHErrorList = pSAX2Handler->getPWHErrorList();
      m_strRenameList = pSAX2Handler->getRenameList();

      if (b_into_empty) {
        pSAX2Handler->AddDBPreferences();
      }
    }
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
