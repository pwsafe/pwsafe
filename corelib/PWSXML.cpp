/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// PWSXML.cpp : implementation file
//

#include "PWSXML.h"
#include "SAXHandlers.h"
#include "ItemData.h"
#include "MyString.h"
#include "corelib.h"
#include "PWScore.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <atlcomcli.h>
#include "xml_import.h"
#include "UnknownField.h"
#include "PWSprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PWSXML::PWSXML(PWScore *core,UUIDList *possible_aliases)
  : m_xmlcore(core), m_MSXML_Version(60), m_delimiter(TCHAR('^')),
  m_possible_aliases(possible_aliases)
{
}

PWSXML::~PWSXML()
{
}

//	---------------------------------------------------------------------------
bool PWSXML::XMLProcess(const bool &bvalidation, const CString &ImportedPrefix,
						const CString &strXMLFileName, const CString &strXSDFileName,
            int &nITER, int &nRecordsWithUnknownFields, UnknownFieldList &uhfl)
{
	HRESULT hr, hr0, hr60, hr40, hr30;
	bool b_ok = false;
  bool b_into_empty;
	const CString cs_validation(MAKEINTRESOURCE(IDSC_XMLVALIDATION));
	const CString cs_import(MAKEINTRESOURCE(IDSC_XMLIMPORT));

	m_strResultText = _T("");
	m_bValidation = bvalidation;  // Validate or Import

	//	Create SAXReader object
	ISAXXMLReaderPtr pSAXReader = NULL;
	//	Get ready for XSD schema validation
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
    b_into_empty = m_xmlcore->GetNumEntries() == 0;
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

	//	Create ContentHandlerImpl object
	PWSSAXContentHandler* pCH = new PWSSAXContentHandler();
	if (m_bValidation)
		pCH->SetVariables(NULL, m_bValidation, ImportedPrefix, m_delimiter,
                      m_possible_aliases);
	else
		pCH->SetVariables(m_xmlcore, m_bValidation, ImportedPrefix, m_delimiter, 
                      m_possible_aliases);

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
			m_strResultText.LoadString(IDSC_CANTXMLVALIDATE);
			goto exit;
	}

	if (!FAILED(hr)) {  // Create SchemaCache
		//	Initialize the SchemaCache object with the XSD filename
		CComVariant cvXSDFileName;
		cvXSDFileName.vt = VT_BSTR;
		cvXSDFileName.bstrVal = strXSDFileName.AllocSysString();
		hr = pSchemaCache->add(L"", cvXSDFileName);

		//	Set the SAXReader/Schema Cache features and properties
		{
			/* Documentation is unclear as to what is in which release.
				Try them all - if they don't get set, the world will not end!
				Common Error codes:
					S_OK			Operation successful		0x00000000
					E_NOTIMPL		Not implemented				0x80004001
					E_NOINTERFACE	No such interface supported	0x80004002
					E_ABORT			Operation aborted			0x80004004
					E_FAIL			Unspecified failure			0x80004005
					E_INVALIDARG	Invalid argument			0x80070057
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

		//	Let's begin the parsing now
		wchar_t wcURL[MAX_PATH]={0};
#ifdef _UNICODE
#if _MSC_VER >= 1400
		_tcscpy_s(wcURL, MAX_PATH, strXMLFileName);
#else
		_tcscpy(wcURL, strXMLFileName);
#endif
#else
#if _MSC_VER >= 1400
		size_t numconverted;
		mbstowcs_s(&numconverted, wcURL, MAX_PATH, strXMLFileName, _tcslen(strXMLFileName));
#else
		mbstowcs(wcURL, strXMLFileName, _tcslen(strXMLFileName));
#endif
#endif
		hr = pSAXReader->parseURL(wcURL);

		if(!FAILED(hr)) {  // Check for parsing errors
			if(pEH->bErrorsFound == TRUE) {
				m_strResultText = pEH->m_strValidationResult;
			} else {
				if (m_bValidation) {
					m_numEntriesValidated = pCH->m_numEntries;
					m_delimiter = pCH->m_delimiter;
				} else {
					m_numEntriesImported = pCH->m_numEntries;
					m_strResultText = pCH->m_strImportErrors;  // Maybe import errors (PWHistory field processing)

          m_bRecordHeaderErrors = pCH->m_bRecordHeaderErrors;
          nRecordsWithUnknownFields = pCH->m_nRecordsWithUnknownFields;

          if (b_into_empty) {
            m_bDatabaseHeaderErrors = pCH->m_bDatabaseHeaderErrors;
            nITER = pCH->m_nITER;

            UnknownFieldList::const_iterator vi_IterUXFE;
            for (vi_IterUXFE = pCH->m_ukhxl.begin();
                 vi_IterUXFE != pCH->m_ukhxl.end();
                 vi_IterUXFE++) {
              UnknownFieldEntry ukxfe = *vi_IterUXFE;
              if (ukxfe.st_length > 0) {
                uhfl.push_back(ukxfe);
              }
            }
            PWSprefs *prefs = PWSprefs::GetInstance();
            if (pCH->m_bDisplayExpandedAddEditDlg != -1)
              prefs->SetPref(PWSprefs::DisplayExpandedAddEditDlg, pCH->m_bDisplayExpandedAddEditDlg == 1);
            if (pCH->m_bMaintainDateTimeStamps != -1)
              prefs->SetPref(PWSprefs::MaintainDateTimeStamps, pCH->m_bMaintainDateTimeStamps == 1);
            if (pCH->m_bPWUseDigits != -1)
              prefs->SetPref(PWSprefs::PWUseDigits, pCH->m_bPWUseDigits == 1);
            if (pCH->m_bPWUseEasyVision != -1)
              prefs->SetPref(PWSprefs::PWUseEasyVision, pCH->m_bPWUseEasyVision == 1);
            if (pCH->m_bPWUseHexDigits != -1)
              prefs->SetPref(PWSprefs::PWUseHexDigits, pCH->m_bPWUseHexDigits == 1);
            if (pCH->m_bPWUseLowercase != -1)
              prefs->SetPref(PWSprefs::PWUseLowercase, pCH->m_bPWUseLowercase == 1);
            if (pCH->m_bPWUseSymbols != -1)
              prefs->SetPref(PWSprefs::PWUseSymbols, pCH->m_bPWUseSymbols == 1);
            if (pCH->m_bPWUseUppercase != -1)
              prefs->SetPref(PWSprefs::PWUseUppercase, pCH->m_bPWUseUppercase == 1);
            if (pCH->m_bPWMakePronounceable != -1)
              prefs->SetPref(PWSprefs::PWMakePronounceable, pCH->m_bPWMakePronounceable == 1);
            if (pCH->m_bSaveImmediately != -1)
              prefs->SetPref(PWSprefs::SaveImmediately, pCH->m_bSaveImmediately == 1);
            if (pCH->m_bSavePasswordHistory != -1)
              prefs->SetPref(PWSprefs::SavePasswordHistory, pCH->m_bSavePasswordHistory == 1);
            if (pCH->m_bShowNotesDefault != -1)
              prefs->SetPref(PWSprefs::ShowNotesDefault, pCH->m_bShowNotesDefault == 1);
            if (pCH->m_bShowPasswordInTree != -1)
              prefs->SetPref(PWSprefs::ShowPasswordInTree, pCH->m_bShowPasswordInTree == 1);
            if (pCH->m_bShowPWDefault != -1)
              prefs->SetPref(PWSprefs::ShowPWDefault, pCH->m_bShowPWDefault == 1);
            if (pCH->m_bShowUsernameInTree != -1)
              prefs->SetPref(PWSprefs::ShowUsernameInTree, pCH->m_bShowUsernameInTree == 1);
            if (pCH->m_bSortAscending != -1)
              prefs->SetPref(PWSprefs::SortAscending, pCH->m_bSortAscending == 1);
            if (pCH->m_bUseDefaultUser != -1)
              prefs->SetPref(PWSprefs::UseDefaultUser, pCH->m_bUseDefaultUser == 1);
            if (pCH->m_iIdleTimeout != -1)
              prefs->SetPref(PWSprefs::IdleTimeout, pCH->m_iIdleTimeout);
            if (pCH->m_iNumPWHistoryDefault != -1)
              prefs->SetPref(PWSprefs::NumPWHistoryDefault, pCH->m_iNumPWHistoryDefault);
            if (pCH->m_iPWDefaultLength != -1)
              prefs->SetPref(PWSprefs::PWDefaultLength, pCH->m_iPWDefaultLength);
            if (pCH->m_iTreeDisplayStatusAtOpen != -1)
              prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, pCH->m_iTreeDisplayStatusAtOpen);
            if (!pCH->m_sDefaultAutotypeString.IsEmpty())
              prefs->SetPref(PWSprefs::DefaultAutotypeString, pCH->m_sDefaultAutotypeString);
            if (!pCH->m_sDefaultUsername.IsEmpty())
              prefs->SetPref(PWSprefs::DefaultUsername, pCH->m_sDefaultUsername);
          } else
            m_bDatabaseHeaderErrors = false;
        }

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
