/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// XMLprefs.cpp : implementation file
//
#include "..\stdafx.h"
#include <atlcomcli.h>  // needed for VS7.1, not 8
#include "XMLprefs.h"
#include "xml_import.h"
#include "MyString.h"
#include "PWSfile.h"
#include "corelib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define DEBUG_XMLPREFS
#ifdef DEBUG_XMLPREFS
#include <stdio.h>
static FILE *f;
#define DOPEN() do {f = fopen("cxmlpref.log", "a+");} while (0)
#define DPRINT(x) do {fprintf x; fflush(f);} while (0)
#define DCLOSE() fclose(f)
#else
#define DOPEN()
#define DPRINT(x)
#define DCLOSE()
#endif
/////////////////////////////////////////////////////////////////////////////
// CXMLprefs

bool CXMLprefs::Lock()
{
	CMyString locker(_T(""));
    int tries = 10;
    do {
        m_bIsLocked = PWSfile::LockFile(m_csConfigFile, locker, false);
        if (!m_bIsLocked)
            Sleep(200);
    } while (!m_bIsLocked && --tries > 0);
    return m_bIsLocked;
}

void CXMLprefs::Unlock()
{
    PWSfile::UnlockFile(m_csConfigFile, false);
    m_bIsLocked = false;
}

bool CXMLprefs::Load()
{
	// Already loaded?
	if (m_bXMLLoaded) return true;
	//  Couldn't get it to work previously?
	if (m_MSXML_Version == -1) return false;

    bool alreadyLocked = m_bIsLocked;
    if (!alreadyLocked) {
        if (!Lock())
            return false;
    }

    // from here on all exits from function need to unlock file
	MSXML2::IXMLDOMParseErrorPtr pIParseError = NULL;
	CFile file;
	BOOL b_OK = FALSE;
	// initialize the XML parser
	switch (m_MSXML_Version) {
		case 0:
			// First time through!
			// Try 60
			if (FAILED(m_pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60),
                                                NULL, CLSCTX_ALL))) {
				// Try 40
				if (FAILED(m_pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument40),
                                                    NULL, CLSCTX_ALL))) {
					// Try 30
					if (FAILED(m_pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30),
						                                NULL, CLSCTX_ALL))) {
						AfxMessageBox(IDSC_NOXMLREADER, MB_OK);
						m_MSXML_Version = -1;
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
			b_OK = TRUE;
			break;
		case 60:
			if (SUCCEEDED(m_pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_ALL)))
				b_OK = TRUE;
			break;
		case 40:
			if (SUCCEEDED(m_pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument40), NULL, CLSCTX_ALL)))
				b_OK = TRUE;
			break;
		case 30:
			if (SUCCEEDED(m_pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30), NULL, CLSCTX_ALL)))
				b_OK = TRUE;
			break;
		default:
			// Should never get here
			ASSERT(0);
            b_OK = FALSE;
	}

	if (b_OK == FALSE) {
        goto exit;
    }

	// Ensure we preserve all white space!
	if (FAILED(m_pXMLDoc->put_preserveWhiteSpace(VARIANT_TRUE))) {
		UnloadXML();
		goto exit;
	}

	VARIANT_BOOL vbSuccessful;

	// see if the file exists
	if (!file.Open(m_csConfigFile, CFile::modeReadWrite)) {  // if not
		// create it - IDSC_XMLHEADER
		CComBSTR bstrXML;
		bstrXML.LoadString(IDSC_XMLHEADER);
		m_pXMLDoc->loadXML(bstrXML, &vbSuccessful);
	} else {  // if so
		file.Close();
		// load it
		m_pXMLDoc->load(CComVariant::CComVariant((LPCTSTR)m_csConfigFile),
                        &vbSuccessful);
	}

	m_bXMLLoaded = (vbSuccessful == VARIANT_TRUE);

	if (!m_bXMLLoaded) {
        // an XML load error occurred so display the reason
        m_pXMLDoc->get_parseError(&pIParseError);

        if (pIParseError) {
            long value, line, linepos;
            BSTR bstr = NULL;

            pIParseError->get_errorCode(&value);
            pIParseError->get_reason(&bstr);
            pIParseError->get_line(&line);
            pIParseError->get_linepos(&linepos);

            CString csMessage;
            csMessage.Format(IDSC_XMLFILEERROR, 
                             value, line, linepos, (char *)_bstr_t(bstr, TRUE));
            const CString cs_title(MAKEINTRESOURCE(IDSC_XMLLOADFAILURE));
            MessageBox(NULL, csMessage, cs_title, MB_OK);

            if (bstr) {
                SysFreeString(bstr);
                bstr = NULL;
            }

            pIParseError = NULL;
        }

        if (m_pXMLDoc != NULL) {
            m_pXMLDoc.Release();
            m_pXMLDoc = NULL;
        }
    } // load failed

  exit:
    // if we locked it, we should unlock it...
    if (!alreadyLocked)
        Unlock();
    return m_bXMLLoaded;
}

bool CXMLprefs::Store()
{
   bool alreadyLocked = m_bIsLocked;
    if (!alreadyLocked) {
        if (!Lock())
            return false;
    }

	CString csConfigData;
	IStream *pIStream;
	STATSTG mStat;
	VARIANT_BOOL vbSuccessful;
	ULONG num;
    DOPEN();
    DPRINT((f, "Entered CXMLprefs::Store()\n"));
    DPRINT((f, "\tm_MSXML_Version = %d\n", m_MSXML_Version));
    DPRINT((f, "\tm_pXMLDoc = %p\n", m_pXMLDoc));
    ASSERT(m_pXMLDoc != NULL);
	// Get the string from the DOM
	m_pXMLDoc->QueryInterface(IID_IStream, (void **)&pIStream);
	pIStream->Stat(&mStat, 0);
	const int ilen = (int)mStat.cbSize.QuadPart + 1;
	TCHAR *lpszBuffer = csConfigData.GetBuffer(ilen);
	pIStream->Read(lpszBuffer, ilen, &num);
	csConfigData.ReleaseBuffer(num);
	pIStream->Release();
    DPRINT((f, "csConfigData=[%s]\n",csConfigData));
	// First remove all tabs, carriage returns and line-ends
	csConfigData.Remove(_T('\t'));
	csConfigData.Remove(_T('\r'));
	csConfigData.Remove(_T('\n'));

	// Define and then create the SAX reader and DOM writer.
	MSXML2::ISAXXMLReaderPtr pSAXReader = NULL;
	MSXML2::IMXWriterPtr pXMLWriter = NULL;
	BOOL b_R_OK, b_W_OK;

	b_R_OK = b_W_OK = FALSE;
	switch (m_MSXML_Version) {
		case 60:
			if (SUCCEEDED(pSAXReader.CreateInstance(__uuidof(MSXML2::SAXXMLReader60), 
                                                    NULL, CLSCTX_ALL)))
				b_R_OK = TRUE;
			if (SUCCEEDED(pXMLWriter.CreateInstance(__uuidof(MSXML2::MXXMLWriter60), 
                                                    NULL, CLSCTX_ALL)))
				b_W_OK = TRUE;
			break;
		case 40:
			if (SUCCEEDED(pSAXReader.CreateInstance(__uuidof(MSXML2::SAXXMLReader40), 
                                                    NULL, CLSCTX_ALL)))
				b_R_OK = TRUE;
			if (SUCCEEDED(pXMLWriter.CreateInstance(__uuidof(MSXML2::MXXMLWriter40), 
                                                    NULL, CLSCTX_ALL)))
				b_W_OK = TRUE;
			break;
		case 30:
			if (SUCCEEDED(pSAXReader.CreateInstance(__uuidof(MSXML2::SAXXMLReader30), 
                                                    NULL, CLSCTX_ALL)))
				b_R_OK = TRUE;
			if (SUCCEEDED(pXMLWriter.CreateInstance(__uuidof(MSXML2::MXXMLWriter30), 
                                                    NULL, CLSCTX_ALL)))
				b_W_OK = TRUE;
			break;
		default:
			// Should never get here
			ASSERT(0);
	}

	// Check created OK
	ASSERT(b_R_OK && b_W_OK);
    DPRINT((f, "\tb_R_OK = %s\n", b_R_OK ? "true" : "false"));
    DPRINT((f, "\tb_W_OK = %s\n", b_W_OK ? "true" : "false"));
    DPRINT((f, "\tpXMLWriter = %p\n",pXMLWriter));
    DPRINT((f, "\tpSAXReader = %p\n",pSAXReader));
	// Say we want it indented
	pXMLWriter->put_indent(VARIANT_TRUE);
	pXMLWriter->put_standalone(VARIANT_TRUE);
	pXMLWriter->put_encoding(CComBSTR(L"UTF-8"));

	// Create a reader ContentHandler and make it the writer
	MSXML2::ISAXContentHandlerPtr pCH = pXMLWriter;
	pSAXReader->putContentHandler(pCH);

	// Parse the current XML and then reload it once reformatted
	// But first convert from CString to VARIANT!
	VARIANT vDocString;
	vDocString.vt = VT_BSTR;
	vDocString.bstrVal = csConfigData.AllocSysString();

	pSAXReader->parse(vDocString);

	VARIANT vNewDocString;
	pXMLWriter->get_output(&vNewDocString);
	m_pXMLDoc->loadXML(vNewDocString.bstrVal, &vbSuccessful);

	ASSERT(vbSuccessful == VARIANT_TRUE);
    DPRINT((f, "\tvbSuccessful = %s\n",
            vbSuccessful == VARIANT_TRUE ? "OK" : "***FAILED***"));

	// Free memory
	SysFreeString(vDocString.bstrVal);

	// Now try to save!
    bool retval;
	if (SUCCEEDED(m_pXMLDoc->save(CComVariant::CComVariant(m_csConfigFile))))
		retval = true;
	else
		retval = false;

	// Now free reader & writer (content handler is done automatically)
	if (pXMLWriter != NULL) {
		pXMLWriter.Release();
		pXMLWriter = NULL;
		pCH = NULL;
	}
	if (pSAXReader != NULL) {
		pSAXReader.Release();
		pSAXReader = NULL;
	}

    // if we locked it, we should unlock it...
    if (!alreadyLocked)
        Unlock();
    DPRINT((f, "Leaving CXMLprefs::Store(), retval = %s\n",
            retval ? "true" : "false"));
    DCLOSE();
    return retval;
}


// get a int value
int CXMLprefs::Get(const CString &csBaseKeyName, const CString &csValueName, 
					   const int &iDefaultValue)
{
	/*
		Since XML is text based and we have no schema, just convert to a string and
		call the GetSettingString method.
	*/
	int iRetVal = iDefaultValue;
	CString csDefaultValue;

	csDefaultValue.Format(_T("%d"), iRetVal);

	iRetVal = _ttoi(Get(csBaseKeyName, csValueName, csDefaultValue));

	return iRetVal;
}

// get a string value
CString CXMLprefs::Get(const CString &csBaseKeyName, const CString &csValueName, 
                       const CString &csDefaultValue)
{
    ASSERT(m_bXMLLoaded); // shouldn't be called if load failed
    if (!m_bXMLLoaded) // just in case
        return csDefaultValue;

	int iNumKeys = 0;
	CString csValue = csDefaultValue;
	CString* pcsKeys = NULL;

	// Add the value to the base key separated by a '\'
	CString csKeyName(csBaseKeyName);
	csKeyName += _T("\\");
	csKeyName += csValueName;

	// Parse all keys from the base key name (keys separated by a '\')
	pcsKeys = ParseKeys(csKeyName, iNumKeys);

	// Traverse the xml using the keys parsed from the base key name to find the correct node
	if (pcsKeys) {
        MSXML2::IXMLDOMElementPtr rootElem = NULL;
        MSXML2::IXMLDOMNodePtr foundNode = NULL;

        m_pXMLDoc->get_documentElement(&rootElem);  // root node
        if (rootElem) {
            // returns the last node in the chain
            foundNode = FindNode(rootElem, pcsKeys, iNumKeys);
            if (foundNode) {
                // get the text of the node (will be the value we requested)
                BSTR bstr = NULL;
                foundNode->get_text(&bstr);
                csValue = (CString)bstr;
                if (bstr) {
                    SysFreeString(bstr);
                    bstr = NULL;
                }
                foundNode = NULL;
            }
            rootElem = NULL;
        }
		delete [] pcsKeys;
		pcsKeys = NULL;
	}

	return csValue;
}

// set a int value
int CXMLprefs::Set(const CString &csBaseKeyName, const CString &csValueName,
					   const int &iValue)
{
	/*
		Since XML is text based and we have no schema, just convert to a string and
		call the SetSettingString method.
	*/
	int iRetVal = 0;
	CString csValue = _T("");

	csValue.Format(_T("%d"), iValue);

	iRetVal = Set(csBaseKeyName, csValueName, csValue);

	return iRetVal;
}

// set a string value
int CXMLprefs::Set(const CString &csBaseKeyName, const CString &csValueName, 
                   const CString &csValue)
{
    ASSERT(m_bXMLLoaded); // shouldn't be called if load failed
    if (!m_bXMLLoaded) // just in case
        return XML_LOAD_FAILED;

	int iRetVal = XML_SUCCESS;
	int iNumKeys = 0;
	CString* pcsKeys = NULL;

	// Add the value to the base key separated by a '\'
	CString csKeyName(csBaseKeyName);
	csKeyName += _T("\\");
	csKeyName += csValueName;

	// Parse all keys from the base key name (keys separated by a '\')
	pcsKeys = ParseKeys(csKeyName, iNumKeys);

	// Traverse the xml using the keys parsed from the base key name to find the correct node
	if (pcsKeys) {
        MSXML2::IXMLDOMElementPtr rootElem = NULL;
        MSXML2::IXMLDOMNodePtr foundNode = NULL;

        m_pXMLDoc->get_documentElement(&rootElem);  // root node

        if (rootElem) {
            // returns the last node in the chain
            foundNode = FindNode(rootElem, pcsKeys, iNumKeys, TRUE);

            if (foundNode) {
                // set the text of the node (will be the value we sent)
                if (!SUCCEEDED(foundNode->put_text(_bstr_t(csValue)))) {
                    iRetVal = XML_PUT_TEXT_FAILED;
                }
                foundNode = NULL;
            } else
                iRetVal = XML_NODE_NOT_FOUND;

            rootElem = NULL;
		}
		else
			iRetVal = XML_LOAD_FAILED;

		delete [] pcsKeys;
		pcsKeys = NULL;
	}

	return iRetVal;
}

// delete a key or chain of keys
BOOL CXMLprefs::DeleteSetting(const CString &csBaseKeyName, const CString &csValueName)
{
    ASSERT(m_bXMLLoaded); // shouldn't be called if load failed
    if (!m_bXMLLoaded) // just in case
        return FALSE;

	BOOL bRetVal = FALSE;
	int iNumKeys = 0;
	CString* pcsKeys = NULL;
	CString csKeyName(csBaseKeyName);

	if (!csValueName.IsEmpty()) {
		csKeyName += _T("\\");
		csKeyName += csValueName;
	}

	// Parse all keys from the base key name (keys separated by a '\')
	pcsKeys = ParseKeys(csKeyName, iNumKeys);

	// Traverse the xml using the keys parsed from the base key name to find the correct node.
	if (pcsKeys) {
        MSXML2::IXMLDOMElementPtr rootElem = NULL;
        MSXML2::IXMLDOMNodePtr foundNode = NULL;

        m_pXMLDoc->get_documentElement(&rootElem);  // root node
        if (rootElem) {
            // returns the last node in the chain
            foundNode = FindNode(rootElem, pcsKeys, iNumKeys);
            if (foundNode) {
                // get the parent of the found node and use removeChild to delete the found node
                MSXML2::IXMLDOMNodePtr parentNode = NULL;
                foundNode->get_parentNode(&parentNode);
                if (parentNode) {
                    if (SUCCEEDED(parentNode->removeChild(foundNode, NULL))) {
                        bRetVal = TRUE;
                    }
                    parentNode = NULL;
                }
                foundNode = NULL;
            }
            rootElem = NULL;
		}
		delete [] pcsKeys;
		pcsKeys = NULL;
	}
	return bRetVal;
}

// Parse all keys from the base key name.
CString* CXMLprefs::ParseKeys(const CString &csFullKeyPath, int &iNumKeys)
{
	CString* pcsKeys = NULL;

	// replace spaces with _ since xml doesn't like them
	CString csFKP(csFullKeyPath);
	csFKP.Replace(_T(' '), _T('_'));

	if (csFKP.GetAt(csFKP.GetLength() - 1) == _T('\\'))
		csFKP.TrimRight(_T('\\'));  // remove slashes on the end

	CString csTemp(csFKP);

	iNumKeys = csTemp.Remove(_T('\\')) + 1;  // get a count of slashes

	pcsKeys = new CString[iNumKeys];  // create storage for the keys

	if (pcsKeys) {
		int iFind = 0, iLastFind = 0, iCount = -1;

		// get all of the keys in the chain
		while (iFind != -1) {
			iFind = csFKP.Find(_T("\\"), iLastFind);
			if (iFind > -1) {
				iCount++;
				pcsKeys[iCount] = csFKP.Mid(iLastFind, iFind - iLastFind);
				iLastFind = iFind + 1;
			} else {
				// make sure we don't just discard the last key in the chain
				if (iLastFind < csFKP.GetLength())  {
					iCount++;
					pcsKeys[iCount] = csFKP.Right(csFKP.GetLength() - iLastFind);
				}
			}
		}
	}
	return pcsKeys;
}

void CXMLprefs::UnloadXML()
{
	if (!m_bXMLLoaded)
		return;

	if (m_pXMLDoc != NULL) {
		m_pXMLDoc.Release();
		m_pXMLDoc = NULL;
	}
	m_bXMLLoaded = false;
}



// find a node given a chain of key names
MSXML2::IXMLDOMNodePtr CXMLprefs::FindNode(MSXML2::IXMLDOMNodePtr parentNode,
									CString* pcsKeys, int iNumKeys,
									bool bAddNodes /*= false*/)
{
	MSXML2::IXMLDOMNodePtr foundNode = NULL;
	MSXML2::IXMLDOMElementPtr rootElem = NULL, tempElem = NULL;

	m_pXMLDoc->get_documentElement(&rootElem);  // root element

	for (int i=0; i<iNumKeys; i++) {
		// find the node named X directly under the parent
		HRESULT hr = parentNode->selectSingleNode(_bstr_t(pcsKeys[i]), &foundNode);

		if (FAILED(hr) || foundNode == NULL) {
			// if its not found...
			if (bAddNodes)  {  // create the node and append to parent (Set only)
				m_pXMLDoc->createElement(_bstr_t(pcsKeys[i]), &tempElem);
				if (tempElem)  {
					parentNode->appendChild(tempElem, &foundNode);
				}

				// since we are traversing the nodes, we need to set the parentNode to our foundNode
				parentNode = NULL;
				parentNode = foundNode;
				foundNode = NULL;
			} else {
				foundNode = NULL;
				parentNode = NULL;
				break;
			}
		} else {
			// since we are traversing the nodes, we need to set the parentNode to our foundNode
			parentNode = NULL;
			parentNode = foundNode;
			foundNode = NULL;
		}
	}
	rootElem = NULL;
	return parentNode;
}
