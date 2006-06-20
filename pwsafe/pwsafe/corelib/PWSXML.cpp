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
}

PWSXML::~PWSXML()
{
}

//	---------------------------------------------------------------------------
bool PWSXML::XMLValidate(const CString &strXMLFileName, const CString &strXSDFileName)
{
	HRESULT hr0, hr;
	bool b_ok = false;

	m_strResultText = _T("");
	m_bValidation = true;
	TCHAR delim = _T('^');
	CString prefix = _T("");

	//	Create SAXReader object
	ISAXXMLReaderPtr pSAXReader = NULL;
	hr0 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader60));

	//	Create ContentHandlerImpl object
	PWSSAXContentHandler* pCH = new PWSSAXContentHandler();
	pCH->SetVariables(NULL, m_bValidation, prefix, delim);

	//	Create ErrorHandlerImpl object
	PWSSAXErrorHandler* pEH = new PWSSAXErrorHandler();

	if (!FAILED(hr0)) {  // Create SAXReader
		//	Set Content Handler
		hr = pSAXReader->putContentHandler(pCH);

		//	Set Error Handler
		hr = pSAXReader->putErrorHandler(pEH);

		//	Get ready for XSD schema validation
		IXMLDOMSchemaCollection2Ptr pSchemaCache = NULL;
		hr = pSchemaCache.CreateInstance(__uuidof(XMLSchemaCache60));

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
				if (FAILED(hr)) ASSERT(0);
				// Want to validate XML file
				hr = pSAXReader->putFeature(L"schema-validation", VARIANT_TRUE);
				if (FAILED(hr)) ASSERT(0);
				// Don't allow user to override validation by using DTDs
				hr = pSAXReader->putFeature(L"prohibit-dtd", VARIANT_TRUE);
				if (FAILED(hr)) ASSERT(0);
				// Ignore any schema specified in the XML file
				hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
				if (FAILED(hr)) ASSERT(0);
				// Ignore any schema embedded in the XML file
				hr = pSAXReader->putFeature(L"use-inline-schema", VARIANT_FALSE);
				if (FAILED(hr)) ASSERT(0);
				// Only use the one in PWSafe's installation directory!
				hr = pSAXReader->putProperty(L"schemas", _variant_t(pSchemaCache.GetInterfacePtr()));
				if (FAILED(hr)) ASSERT(0);
			}

			//	Let's begin the parsing now
			wchar_t wcURL[MAX_PATH]={0};
			mbstowcs(wcURL, strXMLFileName, _tcslen(strXMLFileName));
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
					m_strResultText.Format(_T("Parse Error %08X"), hr);
				}
			}  // End Check for parsing errors

		} else {
			m_strResultText.Format(_T("Create Schema Cache Error %08X"), hr);
		}  // End Create Schema Cache

	} else {
		m_strResultText.Format(_T("SAX Reader CreateInstance Error %08X."), hr0);
		m_strResultText += _T("\r\n\r\nProbably caused by MS MXL Core Services V6.0 (msxml6.dll), or later, not being installed.");
	}  // End Create SAXReader

	return b_ok;
}

//	---------------------------------------------------------------------------
bool PWSXML::XMLImport(const CString &ImportedPrefix, const CString &strXMLFileName)
{
	HRESULT hr0, hr;
	bool b_ok = false;

	m_strResultText = _T("");
	m_bValidation = false;

	//	Create SAXReader object
	ISAXXMLReaderPtr pSAXReader = NULL;
	hr0 = pSAXReader.CreateInstance(__uuidof(SAXXMLReader60));

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
		if (FAILED(hr)) ASSERT(0);
		// Don't allow user to override validation by using DTDs
		hr = pSAXReader->putFeature(L"prohibit-dtd", VARIANT_TRUE);
		if (FAILED(hr)) ASSERT(0);
		// Ignore any schema specified in the XML file
		hr = pSAXReader->putFeature(L"use-schema-location", VARIANT_FALSE);
		if (FAILED(hr)) ASSERT(0);
		// Ignore any schema embedded in the XML file
		hr = pSAXReader->putFeature(L"use-inline-schema", VARIANT_FALSE);
		if (FAILED(hr)) ASSERT(0);

		//	Let's begin the parsing now
		wchar_t wcURL[MAX_PATH]={0};
		mbstowcs(wcURL, strXMLFileName, _tcslen(strXMLFileName));
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
				m_strResultText.Format(_T("Parse Error %08Xt"), hr);
		}  // End Do Parse

	} else {
		m_strResultText.Format(_T("SAX Reader CreateInstance Error %08X."), hr0);
		m_strResultText += _T("\r\n\r\nProbably caused by MS MXL Core Services V6.0 (msxml6.dll), or later, not being installed.");
	}  // Create SAXReader

	return b_ok;
}
