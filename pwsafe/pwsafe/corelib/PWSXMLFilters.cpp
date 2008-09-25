/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWSXMLFilters.cpp : implementation file
//
#include "PWSXMLFilters.h"
#include "SAXFilters.h"
#include "corelib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <atlcomcli.h>
#include "xml_import.h"

#include <map>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PWSXMLFilters::PWSXMLFilters(PWSFilters &mapfilters, const FilterPool fpool)
  : m_MapFilters(mapfilters), m_FPool(fpool), m_MSXML_Version(60)
{
}

PWSXMLFilters::~PWSXMLFilters()
{
}

// ---------------------------------------------------------------------------
bool PWSXMLFilters::XMLFilterProcess(const bool &bvalidation,
                                     const CString &strXMLData,
                                     const CString &strXMLFileName, 
                                     const CString &strXSDFileName)
{
  HRESULT hr, hr0, hr60, hr40, hr30;
  bool b_ok = false;
  const CString cs_validation(MAKEINTRESOURCE(IDSC_XMLVALIDATION));
  const CString cs_import(MAKEINTRESOURCE(IDSC_XMLIMPORT));

  m_strResultText = _T("");
  m_bValidation = bvalidation;  // Validate or Import

  //  Create SAXReader object
  ISAXXMLReaderPtr pSAXReader = NULL;
  //  Get ready for XSD schema validation
  IXMLDOMSchemaCollection2Ptr pSchemaCache = NULL;

  if (m_bValidation) { //XMLValidate
    // Try 60
    hr60 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader60), NULL, CLSCTX_ALL);
    if (FAILED(hr60)) {
      // Try 40
      hr40 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader40), NULL, CLSCTX_ALL);
      if (FAILED(hr40)) {
        // Try 30
        hr30 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL);
        if (FAILED(hr30)) {
          m_strResultText.LoadString(IDSC_NOXMLREADER);
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
        hr0 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader60), NULL, CLSCTX_ALL);
        break;
      case 40:
        hr0 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader40), NULL, CLSCTX_ALL);
        break;
      case 30:
        hr0 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL);
        break;
      default:
        // Should never get here as validate would have sorted it and this doesn't get called if it fails
        ASSERT(0);
    }
  }

  //  Create ContentHandlerImpl object
  PWSSAXFilterContentHandler* pCH = new PWSSAXFilterContentHandler();
  //  Create ErrorHandlerImpl object
  PWSSAXFilterErrorHandler* pEH = new PWSSAXFilterErrorHandler();

  pCH->SetVariables(&m_MapFilters, m_FPool, m_bValidation);
 
  //  Set Content Handler
  hr = pSAXReader->putContentHandler(pCH);

  //  Set Error Handler
  hr = pSAXReader->putErrorHandler(pEH);

  switch (m_MSXML_Version) {
    case 60:
      hr = pSchemaCache.CreateInstance(__uuidof(XMLSchemaCache60));
      break;
    case 40:
      hr = pSchemaCache.CreateInstance(__uuidof(XMLSchemaCache40));
      break;
    case 30:
      hr = pSchemaCache.CreateInstance(__uuidof(XMLSchemaCache30));
      break;
    default:
      m_strResultText.LoadString(IDSC_CANTXMLVALIDATE);
      goto exit;
  }

  if (!FAILED(hr)) {  // Create SchemaCache
    //  Initialize the SchemaCache object with the XSD filename
    CComVariant cvXSDFileName;
    cvXSDFileName.vt = VT_BSTR;
    cvXSDFileName.bstrVal = strXSDFileName.AllocSysString();
    hr = pSchemaCache->add(L"", cvXSDFileName);
    if (hr != S_OK) {
      m_strResultText.LoadString(IDSC_INVALID_SCHEMA);
      return false;
    }
    hr = pSchemaCache->validate();
    if (hr != S_OK) {
      m_strResultText.LoadString(IDSC_INVALID_SCHEMA);
      return false;
    }

    // Check that we can get the Schema version
    BSTR bst_schema, bst_schema_version;
    bst_schema = L"";
    ISchema *pischema;
    hr = pSchemaCache->getSchema(bst_schema, &pischema);
    if (hr != S_OK) {
      m_strResultText.LoadString(IDSC_MISSING_SCHEMA_VER);
      return false;
    }
    hr = pischema->get_version(&bst_schema_version);
    if (hr != S_OK) {
      m_strResultText.LoadString(IDSC_INVALID_SCHEMA_VER);
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
      hr = pSAXReader->putFeature(L"exhaustive-errors", VARIANT_TRUE);
      // Don't allow user to override validation by using DTDs
      hr = pSAXReader->putFeature(L"prohibit-dtd", VARIANT_TRUE);
      // Don't allow user to override validation by using DTDs (2 features)
      hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-general-entities", VARIANT_FALSE);
      hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-parameter-entities", VARIANT_FALSE);
      // Want to validate XML file
      hr = pSAXReader->putFeature(L"schema-validation", VARIANT_TRUE);
      // Ignore any schema specified in the XML file
      hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
      // Ignore any schema in the XML file
      hr = pSAXReader->putFeature(L"use-inline-schema", VARIANT_FALSE);
      // Only use the XSD in PWSafe's installation directory!
      hr = pSAXReader->putProperty(L"schemas", _variant_t(pSchemaCache.GetInterfacePtr()));
    }

    //  Let's begin the parsing now
    if (!strXMLFileName.IsEmpty()) {
      wchar_t wcURL[MAX_PATH]={0};
#ifdef _UNICODE
#if _MSC_VER >= 1400
      _tcscpy_s(wcURL, MAX_PATH, strXMLFileName);
#else
      _tcscpy(wcURL, strXMLFileName);
#endif
#else
      mbstowcs(wcURL, strXMLFileName, _tcslen(strXMLFileName));
#endif
      hr = pSAXReader->parseURL(wcURL);
    } else {
      CComVariant cvXMLData;
      cvXMLData.vt = VT_BSTR;
      cvXMLData.bstrVal = strXMLData.AllocSysString();
      hr = pSAXReader->parse(cvXMLData);
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
        m_strResultText.Format(IDSC_XMLPARSEERROR, m_MSXML_Version, hr,
                               m_bValidation ? cs_validation : cs_import);
      }
    }  // End Check for parsing errors

  } else {
    m_strResultText.Format(IDSC_XMLBADCREATESCHEMA, m_MSXML_Version, hr,
                           m_bValidation ? cs_validation : cs_import);
  }  // End Create Schema Cache

exit:
  if (pSchemaCache != NULL)
    pSchemaCache.Release();

  if (pSAXReader != NULL)
    pSAXReader.Release();

  return b_ok;
}
