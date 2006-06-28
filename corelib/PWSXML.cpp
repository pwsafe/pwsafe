// PWSXML.cpp : implementation file
//

#include "PWSXML.h"
#include "SAXHandlers.h"
#include "ItemData.h"
#include "MyString.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <atlcomcli.h>
#include "xml_import.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PWSXML::PWSXML()
{
	m_MSXML_Version = 60;
	m_delimiter = _T('^');
}

PWSXML::~PWSXML()
{
}

void
PWSXML::SetCore(PWScore *core)
{
	m_core = core;
}

//	---------------------------------------------------------------------------
bool PWSXML::XMLValidate(const CString &strXMLFileName, const CString &strXSDFileName)
{
	HRESULT hr, hr60, hr40, hr30;
	bool b_ok = false;

	m_strResultText = _T("");
	m_bValidation = true;
	const CString prefix = _T("");
	CString msg;

	CoInitialize(NULL); 
	//	Create SAXReader object
	ISAXXMLReaderPtr pSAXReader = NULL;
	//	Get ready for XSD schema validation
	IXMLDOMSchemaCollection2Ptr pSchemaCache = NULL;

	// Try 60
	hr60 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader60), NULL, CLSCTX_ALL);
	if (FAILED(hr60)) {
		// Try 40
		hr40 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader40), NULL, CLSCTX_ALL);
		if (FAILED(hr40)) {
			// Try 30
			hr30 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL);
			if (FAILED(hr30)) {
				m_strResultText =_T("Unable to use a XML reader on your system.  Neither MS XML V3, V4 or V6 seems available.");
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

	//	Create ContentHandlerImpl object
	PWSSAXContentHandler* pCH = new PWSSAXContentHandler();
	pCH->SetVariables(NULL, m_bValidation, prefix, m_delimiter);

	//	Create ErrorHandlerImpl object
	PWSSAXErrorHandler* pEH = new PWSSAXErrorHandler();

	//	Set Content Handler
	hr = pSAXReader->putContentHandler(pCH);

	//	Set Error Handler
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
			m_strResultText = _T("Unable to validate a XML file using a XML Schema (XSD) on your system.");
			goto exit;
	}

	if (!FAILED(hr)) {  // Create SchemaCache
		//	Initialize the SchemaCache object with the XSD filename
		CComVariant cvXSDFileName;
		cvXSDFileName.vt = VT_BSTR;
		cvXSDFileName.bstrVal = strXSDFileName.AllocSysString();
		hr = pSchemaCache->add(L"", cvXSDFileName);

		//	Set the SAXReader XSD related properties
		{
			// Want all errors
			hr = pSAXReader->putFeature(L"exhaustive-errors", VARIANT_TRUE);
			// Want to validate XML file
			hr = pSAXReader->putFeature(L"schema-validation", VARIANT_TRUE);
			switch (m_MSXML_Version) {
				case 60:
					// Ignore any schema specified in the XML file
					hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
					// Don't allow user to override validation by using DTDs
					hr = pSAXReader->putFeature(L"prohibit-dtd", VARIANT_TRUE);
					// Ignore any schema embedded in the XML file
					hr = pSAXReader->putFeature(L"use-inline-schema", VARIANT_FALSE);
					break;
				case 40:
					// Ignore any schema specified in the XML file
					hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
					// Don't allow user to override validation by using DTDs - different way prior to V6
					hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-general-entities", VARIANT_FALSE);
					hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-parameter-entities", VARIANT_FALSE);
					// Ignore any schema embedded in the XML file is not supported prior to V6
					// .... L"use-inline-schema" ....
					break;
				case 30:
					// Ignore any schema specified in the XML file is not supported prior to V4
					// .... L"use-schema-location" ....
					// Don't allow user to override validation by using DTDs - different way prior to V6
					hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-general-entities", VARIANT_FALSE);
					hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-parameter-entities", VARIANT_FALSE);
					// Ignore any schema embedded in the XML file is not supported prior to V6
					// .... L"use-inline-schema" ....
					break;
				default:
					goto exit;
			}
				
			// Only use the XSD in PWSafe's installation directory!
			hr = pSAXReader->putProperty(L"schemas", _variant_t(pSchemaCache.GetInterfacePtr()));
		}

		//	Let's begin the parsing now
		wchar_t wcURL[MAX_PATH]={0};
#if _MSC_VER >= 1400
		size_t numconverted;
		mbstowcs_s(&numconverted, wcURL, MAX_PATH, strXMLFileName, _tcslen(strXMLFileName));
#else
		mbstowcs(wcURL, strXMLFileName, _tcslen(strXMLFileName));
#endif
		hr = pSAXReader->parseURL(wcURL);

		if(!FAILED(hr)) {  // Check for parsing errors
			if(pEH->bErrorsFound == TRUE) {
				m_strResultText = pEH->m_strValidationResult;
			} else {
				m_numEntriesValidated = pCH->m_numEntries;
				m_delimiter = pCH->m_delimiter;
				b_ok = true;
			}
		} else {
			if(pEH->bErrorsFound == TRUE) {
				m_strResultText = pEH->m_strValidationResult;
			} else {
				m_strResultText.Format(_T("SAX Parse%2d Error 0x%08X."), m_MSXML_Version, hr);
			}
		}  // End Check for parsing errors

	} else {
		m_strResultText.Format(_T("Create SchemaCache%2d Error 0x%08X."), m_MSXML_Version, hr);
	}  // End Create Schema Cache

exit:
	if (pSchemaCache != NULL)
		pSchemaCache.Release();

	if (pSAXReader != NULL)
		pSAXReader.Release();

	CoUninitialize();
	return b_ok;
}

//	---------------------------------------------------------------------------
bool PWSXML::XMLImport(const CString &ImportedPrefix, const CString &strXMLFileName)
{
	HRESULT hr0 = 0, hr;
	bool b_ok = false;

	m_strResultText = _T("");
	m_bValidation = false;

	CoInitialize(NULL);
	//	Create SAXReader object
	ISAXXMLReaderPtr pSAXReader = NULL;
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
			// Should never get here as XMLValidate would have sorted it and this doesn't get called if it fails
			ASSERT(0);
	}

	//	Create ContentHandlerImpl object
	PWSSAXContentHandler* pCH = new PWSSAXContentHandler();
	pCH->SetVariables(m_core, m_bValidation, ImportedPrefix, m_delimiter);

	//	Create ErrorHandlerImpl object
	PWSSAXErrorHandler* pEH = new PWSSAXErrorHandler();

	if (!FAILED(hr0)) {  // Create SAXReader
		//	Set Content Handler
		hr = pSAXReader->putContentHandler(pCH);

		//	Set Error Handler
		hr = pSAXReader->putErrorHandler(pEH);

		// Want all errors
		hr = pSAXReader->putFeature(L"exhaustive-errors", VARIANT_TRUE);
		// Want to validate XML file
		hr = pSAXReader->putFeature(L"schema-validation", VARIANT_TRUE);
		switch (m_MSXML_Version) {
			case 60:
				// Ignore any schema specified in the XML file
				hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
				// Don't allow user to override validation by using DTDs
				hr = pSAXReader->putFeature(L"prohibit-dtd", VARIANT_TRUE);
				// Ignore any schema embedded in the XML file
				hr = pSAXReader->putFeature(L"use-inline-schema", VARIANT_FALSE);
				break;
			case 40:
				// Ignore any schema specified in the XML file
				hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
				// Don't allow user to override validation by using DTDs - different way prior to V6
				hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-general-entities", VARIANT_FALSE);
				hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-parameter-entities", VARIANT_FALSE);
				// Ignore any schema embedded in the XML file is not supported prior to V6
				// .... L"use-inline-schema" ....
				break;
			case 30:
				// Ignore any schema specified in the XML file is not supported prior to V4
				// .... L"use-schema-location" ....
				// Don't allow user to override validation by using DTDs - different way prior to V6
				hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-general-entities", VARIANT_FALSE);
				hr = pSAXReader->putFeature(L"http://xml.org/sax/features/external-parameter-entities", VARIANT_FALSE);
				// Ignore any schema embedded in the XML file is not supported prior to V6
				// .... L"use-inline-schema" ....
				break;
			default:
				// Should never get here as XMLValidate would have sorted it and this doesn't get called if it fails
				ASSERT(0);
		}

		//	Let's begin the parsing now
		wchar_t wcURL[MAX_PATH]={0};
#if _MSC_VER >= 1400
		size_t numconverted;
		mbstowcs_s(&numconverted, wcURL, MAX_PATH, strXMLFileName, _tcslen(strXMLFileName));
#else
		mbstowcs(wcURL, strXMLFileName, _tcslen(strXMLFileName));
#endif
		hr = pSAXReader->parseURL(wcURL);

		if(!FAILED(hr)) {  // Do Parse
			if(pEH->bErrorsFound == TRUE) {  // Check for errors - shouldn't be as it passed validation!
				m_strResultText = pEH->m_strValidationResult;
			} else {
				m_numEntriesImported = pCH->m_numEntries;
				m_strResultText = pCH->m_strImportErrors;  // Maybe import errors (PWHistory field processing)
				b_ok = true;
			}  // End Check for errors
		} else {
				m_strResultText.Format(_T("SAX Parse%2d Error 0x%08X."), m_MSXML_Version, hr);
		}  // End Do Parse

	} else {
		m_strResultText.Format(_T("SAXReader%2d CreateInstance Error 0x%08X."), m_MSXML_Version, hr0);
	}  // Create SAXReader

	if (pSAXReader != NULL)
		pSAXReader.Release();

	CoUninitialize();
	return b_ok;
}
