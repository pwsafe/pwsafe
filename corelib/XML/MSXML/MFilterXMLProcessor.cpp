/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// MFilterXMLProcessor.cpp : implementation file
//

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == MSXML

#include "MFilterXMLProcessor.h"
#include "MFilterSAX2Handlers.h"
#include <msxml6.h>

#include "../../corelib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <comutil.h>

#include <map>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MFilterXMLProcessor::MFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool,
                                         Asker *pAsker)
  : m_MSXML_Version(60), m_MapFilters(mapfilters), m_FPool(fpool), m_pAsker(pAsker)
{
}

MFilterXMLProcessor::~MFilterXMLProcessor()
{
}

bool MFilterXMLProcessor::Process(const bool &bvalidation,
                                  const StringX &strXMLData,
                                  const stringT &strXMLFileName,
                                  const stringT &strXSDFileName)
{
  HRESULT hr, hr0, hr60, hr40, hr30;
  bool b_ok = false;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);

  m_strResultText = _T("");
  m_bValidation = bvalidation;  // Validate or Import

  //  Create SAXReader object
  ISAXXMLReader* pSAX2Reader = NULL;
  //  Get ready for XSD schema validation
  IXMLDOMSchemaCollection2* pSchemaCache = NULL;

  if (m_bValidation) { //XMLValidate
    // Try 60
    hr60 = CoCreateInstance(__uuidof(SAXXMLReader60), NULL, CLSCTX_ALL,
                            __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
    if (FAILED(hr60)) {
      // Try 40
      hr40 = CoCreateInstance(__uuidof(SAXXMLReader40), NULL, CLSCTX_ALL,
                              __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
      if (FAILED(hr40)) {
        // Try 30
        hr30 = CoCreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL,
                                __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
        if (FAILED(hr30)) {
          LoadAString(m_strResultText, IDSC_NOMSXMLREADER);
          goto exit;
        } else {
          m_MSXML_Version = 30;
        }
      } else {
        m_MSXML_Version = 40;
      }
    } else {
      m_MSXML_Version = 60;
    }
  } else {  // XMLImport
    switch (m_MSXML_Version) {
      case 60:
        hr0 = CoCreateInstance(__uuidof(SAXXMLReader60), NULL, CLSCTX_ALL,
                               __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
        break;
      case 40:
        hr0 = CoCreateInstance(__uuidof(SAXXMLReader40), NULL, CLSCTX_ALL,
                               __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
        break;
      case 30:
        hr0 = CoCreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL,
                               __uuidof(ISAXXMLReader), (void **)&pSAX2Reader);
        break;
      default:
        // Should never get here as validate would have sorted it and this doesn't get called if it fails
        ASSERT(0);
    }
  }

  //  Create ContentHandlerImpl object
  MFilterSAX2ContentHandler* pCH = new MFilterSAX2ContentHandler;
  //  Create ErrorHandlerImpl object
  MFilterSAX2ErrorHandler* pEH = new MFilterSAX2ErrorHandler;

  pCH->SetVariables(m_pAsker, &m_MapFilters, m_FPool, m_bValidation);

  //  Set Content Handler
  hr = pSAX2Reader->putContentHandler(pCH);

  //  Set Error Handler
  hr = pSAX2Reader->putErrorHandler(pEH);

  switch (m_MSXML_Version) {
    case 60:
      hr = CoCreateInstance(__uuidof(XMLSchemaCache60), NULL, CLSCTX_ALL,
                            __uuidof(IXMLDOMSchemaCollection2), (void **)&pSchemaCache);
      break;
    case 40:
      hr = CoCreateInstance(__uuidof(XMLSchemaCache40), NULL, CLSCTX_ALL,
                            __uuidof(IXMLDOMSchemaCollection2), (void **)&pSchemaCache);
      break;
    case 30:
      hr = CoCreateInstance(__uuidof(XMLSchemaCache30), NULL, CLSCTX_ALL,
                            __uuidof(IXMLDOMSchemaCollection2), (void **)&pSchemaCache);
      break;
    default:
      LoadAString(m_strResultText, IDSC_CANTXMLVALIDATE);
      goto exit;
  }

  if (!FAILED(hr)) {  // Create SchemaCache
    //  Initialize the SchemaCache object with the XSD filename
    CComVariant cvXSDFileName;
    cvXSDFileName.vt = VT_BSTR;
    CString cs_XSDfname(strXSDFileName.c_str());
    cvXSDFileName.bstrVal = cs_XSDfname.AllocSysString();
    hr = pSchemaCache->add(L"", cvXSDFileName);
    if (hr != S_OK) {
      LoadAString(m_strResultText, IDSC_INVALID_SCHEMA);
      return false;
    }
    hr = pSchemaCache->validate();
    if (hr != S_OK) {
      LoadAString(m_strResultText, IDSC_INVALID_SCHEMA);
      return false;
    }

    // Check that we can get the Schema version
    BSTR bst_schema, bst_schema_version;
    bst_schema = L"";
    ISchema *pischema;
    hr = pSchemaCache->getSchema(bst_schema, &pischema);
    if (hr != S_OK) {
      LoadAString(m_strResultText, IDSC_MISSING_SCHEMA_VER);
      return false;
    }
    hr = pischema->get_version(&bst_schema_version);
    if (hr != S_OK) {
      LoadAString(m_strResultText, IDSC_INVALID_SCHEMA_VER);
      return false;
    }

    pCH->SetSchemaVersion(&bst_schema_version);

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
      hr = pSAX2Reader->putFeature(L"http://xml.org/sax/features/external-general-entities", VARIANT_FALSE);
      hr = pSAX2Reader->putFeature(L"http://xml.org/sax/features/external-parameter-entities", VARIANT_FALSE);
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
    if (!strXMLFileName.empty()) {
      wchar_t wcURL[MAX_PATH]={0};
#ifdef _UNICODE
#if _MSC_VER >= 1400
      _tcscpy_s(wcURL, MAX_PATH, strXMLFileName.c_str());
#else
      _tcscpy(wcURL, strXMLFileName.c_str());
#endif
#else
      mbstowcs(wcURL, strXMLFileName.c_str(), strXMLFileName.length());
#endif
      hr = pSAX2Reader->parseURL(wcURL);
    } else {
      CComVariant cvXMLData;
      cvXMLData.vt = VT_BSTR;
      CString cs_XMLData(strXMLData.c_str());
      cvXMLData.bstrVal = cs_XMLData.AllocSysString();
      hr = pSAX2Reader->parse(cvXMLData);
    }

    if(!FAILED(hr)) {  // Check for parsing errors
      if(pEH->bErrorsFound == TRUE) {
        m_strResultText = pEH->m_strValidationResult;
      } else {
        b_ok = true;
      }
    } else {
      if(pEH->bErrorsFound == TRUE) {
        m_strResultText = pEH->m_strValidationResult;
      } else {
        Format(m_strResultText, IDSC_MSXMLPARSEERROR, m_MSXML_Version, hr,
               m_bValidation ? cs_validation.c_str() : cs_import.c_str());
      }
    }  // End Check for parsing errors

  } else {
    Format(m_strResultText, IDSC_MSXMLBADCREATESCHEMA, m_MSXML_Version, hr,
           m_bValidation ? cs_validation.c_str() : cs_import.c_str());
  }  // End Create Schema Cache

exit:
  if (pSchemaCache != NULL)
    pSchemaCache->Release();

  if (pSAX2Reader != NULL)
    pSAX2Reader->Release();

  return b_ok;
}

#endif /* USE_XML_LIBRARY == MSXML */
