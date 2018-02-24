/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// MFileXMLProcessor.cpp : implementation file
//

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == MSXML

#include "MFileXMLProcessor.h"
#include "MFileSAX2Handlers.h"
#include <msxml6.h>

#include "../../core.h"
#include "../../PWScore.h"
#include "../../Report.h"
#include "../../UnknownField.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <comutil.h>

MFileXMLProcessor::MFileXMLProcessor(PWScore *pcore,
                                     UUIDVector *pPossible_Aliases,
                                     UUIDVector *pPossible_Shortcuts,
                                     MultiCommands *p_multicmds,
                                     CReport *prpt)
  : m_pXMLcore(pcore), m_delimiter(TCHAR('^')),
  m_pPossible_Aliases(pPossible_Aliases), m_pPossible_Shortcuts(pPossible_Shortcuts),
  m_pmulticmds(p_multicmds), m_prpt(prpt)
{
}

MFileXMLProcessor::~MFileXMLProcessor()
{
}

// ---------------------------------------------------------------------------
bool MFileXMLProcessor::Process(const bool &bvalidation, const stringT &ImportedPrefix,
                                const stringT &strXMLFileName, const stringT &strXSDFileName,
                                const bool &bImportPSWDsOnly)
{
  HRESULT hr, hr0, hr60;
  bool b_ok = false;
  bool b_into_empty;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);

  m_strXMLErrors = _T("");
  m_bValidation = bvalidation;  // Validate or Import

  //  Create SAXReader object
  ISAXXMLReader *pSAX2Reader = nullptr;
  //  Get ready for XSD schema validation
  IXMLDOMSchemaCollection2 *pSchemaCache = nullptr;

  if (m_bValidation) { //XMLValidate
    hr60 = CoCreateInstance(__uuidof(SAXXMLReader60), nullptr, CLSCTX_ALL,
                            __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
    if (FAILED(hr60)) {
      LoadAString(m_strXMLErrors, IDSC_NOMSXMLREADER);
      goto exit;
    }
  } else {  // XMLImport
    b_into_empty = m_pXMLcore->GetNumEntries() == 0;
    hr0 = CoCreateInstance(__uuidof(SAXXMLReader60), nullptr, CLSCTX_ALL,
                           __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
  }

  //  Create ContentHandlerImpl object
  MFileSAX2ContentHandler *pCH = new MFileSAX2ContentHandler();
  pCH->SetVariables(m_bValidation ? nullptr : m_pXMLcore, m_bValidation, 
                    ImportedPrefix, m_delimiter, bImportPSWDsOnly,
                    m_bValidation ? nullptr : m_pPossible_Aliases, 
                    m_bValidation ? nullptr : m_pPossible_Shortcuts,
                    m_pmulticmds, m_prpt);

  //  Create ErrorHandlerImpl object
  MFileSAX2ErrorHandler *pEH = new MFileSAX2ErrorHandler();

  //  Set Content Handler
  hr = pSAX2Reader->putContentHandler(pCH);

  //  Set Error Handler
  hr = pSAX2Reader->putErrorHandler(pEH);

  hr = CoCreateInstance(__uuidof(XMLSchemaCache60), nullptr, CLSCTX_ALL,
                        __uuidof(IXMLDOMSchemaCollection2), (void **)&pSchemaCache);

  if (!FAILED(hr)) {  // Create SchemaCache
    //  Initialize the SchemaCache object with the XSD filename
    CComVariant cvXSDFileName = strXSDFileName.c_str();
    hr = pSchemaCache->add(L"", cvXSDFileName);

    //  Set the SAXReader/Schema Cache features and properties
    {
      /* Documentation is unclear as to what is in which release.
      Try them all - if they don't get set, the world will not end!
      Common Error codes:
      S_OK          Operation successful         0x00000000
      E_NOTIMPL     Not implemented              0x80004001
      E_NOINTERFACE No such interface supported  0x80004002
      E_ABORT       Operation aborted            0x80004004
      E_FAIL        Unspecified failure          0x80004005
      E_INVALIDARG Invalid argument              0x80070057
      Normally not supported on a back level MSXMLn.DLL
      */

      // Want all validation errors
      hr = pSAX2Reader->putFeature(L"exhaustive-errors", VARIANT_TRUE);
      // Don't allow user to override validation by using DTDs
      hr = pSAX2Reader->putFeature(L"prohibit-dtd", VARIANT_TRUE);
      // Don't allow user to override validation by using DTDs (2 features)
      hr = pSAX2Reader->putFeature(L"http://xml.org/sax/features/external-general-entities",
                                   VARIANT_FALSE);
      hr = pSAX2Reader->putFeature(L"http://xml.org/sax/features/external-parameter-entities",
                                   VARIANT_FALSE);
      // Want to validate XML file
      hr = pSAX2Reader->putFeature(L"schema-validation", VARIANT_TRUE);
      // Ignore any schema specified in the XML file
      hr = pSAX2Reader->putFeature(L"use-schema-location", VARIANT_FALSE);
      // Ignore any schema in the XML file
      hr = pSAX2Reader->putFeature(L"use-inline-schema", VARIANT_FALSE);
      // Only use the XSD in PWSafe's installation directory!
      hr = pSAX2Reader->putProperty(L"schemas", _variant_t(pSchemaCache));
    }

    //  Let's begin the parsing now
    wchar_t wcURL[MAX_PATH] = {0};
    _tcscpy_s(wcURL, MAX_PATH, strXMLFileName.c_str());
    hr = pSAX2Reader->parseURL(wcURL);

    if (!FAILED(hr)) {  // Check for parsing errors
      if (pEH->bErrorsFound == TRUE) {
        m_strXMLErrors = pEH->m_strValidationResult;
      } else {
        if (m_bValidation) {
          m_numEntriesValidated = pCH->m_numEntries;
          m_delimiter = pCH->m_delimiter;
        } else {
          // Now add entries
          pCH->AddXMLEntries();

          // Get numbers (may have been modified by AddXMLEntries
          m_numEntriesImported = pCH->m_numEntries;
          m_numEntriesSkipped = pCH->getNumSkipped();
          m_numEntriesRenamed = pCH->getNumRenamed();
          m_numEntriesPWHErrors = pCH->getNumPWHErrors();
          m_numNoPolicies = pCH->getNumNoPolicies();
          m_numRenamedPolicies = pCH->getNumRenamedPolicies();
          m_numShortcutsRemoved = pCH->getNumShortcutsRemoved();
          m_numEmptyGroupsImported = pCH->getNumEmptyGroupsImported();

          // Get lists
          m_strXMLErrors = pCH->getXMLErrors();
          m_strSkippedList = pCH->getSkippedList();
          m_strPWHErrorList = pCH->getPWHErrorList();
          m_strRenameList = pCH->getRenameList();

          if (b_into_empty) {
            pCH->AddDBPreferences();
          }
        }

        b_ok = true;
      }
    } else {
      if (pEH->bErrorsFound == TRUE) {
        m_strXMLErrors = pEH->m_strValidationResult;
      } else {
        Format(m_strXMLErrors, IDSC_MSXMLPARSEERROR, hr,
               m_bValidation ? cs_validation.c_str() : cs_import.c_str());
      }
    }  // End Check for parsing errors

  } else {
    Format(m_strXMLErrors, IDSC_MSXMLBADCREATESCHEMA, hr,
           m_bValidation ? cs_validation.c_str() : cs_import.c_str());
  }  // End Create Schema Cache

exit:
  if (pSchemaCache != nullptr)
    pSchemaCache->Release();

  if (pSAX2Reader != nullptr)
    pSAX2Reader->Release();

  return b_ok;
}

#endif /* USE_XML_LIBRARY == MSXML */
