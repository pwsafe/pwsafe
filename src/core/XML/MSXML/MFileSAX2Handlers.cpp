/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// MFileSAX2Handlers.cpp : implementation file
//

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == MSXML

#include "MFileSAX2Handlers.h"

#include <msxml6.h>

#include "../../core.h"
#include "../../PWScore.h"
#include "../../util.h"
#include "../../PWSfileV3.h"

// Stop warnings about unused formal parameters!
#pragma warning(disable : 4100)

//  -----------------------------------------------------------------------
//  MFileSAX2ErrorHandler Methods
//  -----------------------------------------------------------------------
MFileSAX2ErrorHandler::MFileSAX2ErrorHandler()
  : bErrorsFound(FALSE), m_strValidationResult(_T(""))
{
  m_refCnt = 0;
}

MFileSAX2ErrorHandler::~MFileSAX2ErrorHandler()
{
}

long __stdcall MFileSAX2ErrorHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
  *ppvObject = nullptr;
  if (riid == IID_IUnknown || riid == __uuidof(ISAXContentHandler)) {
    *ppvObject = static_cast<ISAXErrorHandler *>(this);
  }

  if (*ppvObject) {
    AddRef();
    return S_OK;
  }
  else return E_NOINTERFACE;
}

unsigned long __stdcall MFileSAX2ErrorHandler::AddRef()
{
  return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall MFileSAX2ErrorHandler::Release()
{
  --m_refCnt; // NOT thread-safe
  if (m_refCnt == 0) {
    delete this;
    return 0; // Can't return the member of a deleted object.
  }
  else return m_refCnt;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ErrorHandler::error(struct ISAXLocator * pLocator,
                                                       const wchar_t * pwchErrorMessage,
                                                       HRESULT hrErrorCode )
{
  TCHAR szErrorMessage[MAX_PATH * 2] = {0};
  TCHAR szFormatString[MAX_PATH * 2] = {0};
  int iLineNumber, iCharacter;

  _tcscpy_s(szErrorMessage, MAX_PATH * 2, pwchErrorMessage);
  pLocator->getLineNumber(&iLineNumber);
  pLocator->getColumnNumber(&iCharacter);

  stringT cs_format;
  LoadAString(cs_format, IDSC_MSXMLSAXGENERROR);

  _stprintf_s(szFormatString, MAX_PATH * 2, cs_format.c_str(),
              hrErrorCode, iLineNumber, iCharacter, szErrorMessage);

  m_strValidationResult += szFormatString;

  bErrorsFound = TRUE;

  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ErrorHandler::fatalError(struct ISAXLocator * pLocator,
                                                            const wchar_t * pwchErrorMessage,
                                                            HRESULT hrErrorCode )
{
  return error(pLocator, pwchErrorMessage, hrErrorCode);
}

HRESULT STDMETHODCALLTYPE MFileSAX2ErrorHandler::ignorableWarning(struct ISAXLocator * pLocator,
                                                                  const wchar_t * pwchErrorMessage,
                                                                  HRESULT hrErrorCode )
{
  return S_OK;
}

//  -----------------------------------------------------------------------
//  MFileSAX2ContentHandler Methods
//  -----------------------------------------------------------------------
MFileSAX2ContentHandler::MFileSAX2ContentHandler()
{
  m_refCnt = 0;
  m_pValidator = new MFileValidator;
}

//  -----------------------------------------------------------------------
MFileSAX2ContentHandler::~MFileSAX2ContentHandler()
{
  delete m_pValidator;
}

long __stdcall MFileSAX2ContentHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
  *ppvObject = nullptr;
  if (riid == IID_IUnknown || riid == __uuidof(ISAXContentHandler)) {
    *ppvObject = static_cast<ISAXContentHandler *>(this);
  }

  if (*ppvObject) {
    AddRef();
    return S_OK;
  }
  else return E_NOINTERFACE;
}

unsigned long __stdcall MFileSAX2ContentHandler::AddRef()
{
  return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall MFileSAX2ContentHandler::Release()
{
  --m_refCnt; // NOT thread-safe
  if (m_refCnt == 0) {
    delete this;
    return 0; // Can't return the member of a deleted object.
  }
  else return m_refCnt;
}

//  -----------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE  MFileSAX2ContentHandler::startDocument()
{
  m_strXMLErrors = _T("");
  m_bEntryBeingProcessed = false;
  m_bPolicyBeingProcessed = false;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::putDocumentLocator(
                                 struct ISAXLocator * pLocator)
{
  return S_OK;
}

TCHAR * FileProcessAttributes(
  /* [in] */  ISAXAttributes __RPC_FAR *pAttributes,
  /* [in] */  TCHAR *lpName)
{
  // Note 1: Caller needs to free the value returned, which is created via '_tcsdup'.
  // Note 2: This ONLY processes the attributes to find ONE value.
  // Needs to be enhanced if we ever need more (which we do not currently)
  int iAttribs = 0;
  pAttributes->getLength(&iAttribs);
  for (int i = 0; i < iAttribs; i++) {
    TCHAR szQName[MAX_PATH + 1] = {0};
    TCHAR szValue[MAX_PATH + 1] = {0};
    const wchar_t *QName, *Value;
    int QName_length, Value_length;

    pAttributes->getQName(i, &QName, &QName_length);
    pAttributes->getValue(i, &Value, &Value_length);
    _tcsncpy_s(szQName, MAX_PATH + 1, QName, QName_length);
    _tcsncpy_s(szValue, MAX_PATH + 1, Value, Value_length);
    if (_tcscmp(szQName, lpName) == 0) {
      return _tcsdup(szValue);
    }
  }
  return nullptr;
}

//  ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::startElement(
  /* [in] */ const wchar_t __RPC_FAR *pwchNamespaceUri,
  /* [in] */ int cchNamespaceUri,
  /* [in] */ const wchar_t __RPC_FAR *pwchLocalName,
  /* [in] */ int cchLocalName,
  /* [in] */ const wchar_t __RPC_FAR *pwchRawName,
  /* [in] */ int cchRawName,
  /* [in] */ ISAXAttributes __RPC_FAR *pAttributes)
{
  wchar_t szCurElement[MAX_PATH + 1] = {0};

  wcsncpy_s(szCurElement, MAX_PATH + 1, pwchRawName, cchRawName);

  if (m_bValidation) {
    if (wcscmp(szCurElement, L"passwordsafe") == 0) {
      // Only interested in the delimiter
      TCHAR *lpValue = FileProcessAttributes(pAttributes, _T("delimiter"));
      if (lpValue != nullptr) {
        m_delimiter = lpValue[0];
        free(lpValue);
      }
    }
  }

  m_sxElemContent = _T("");

  st_file_element_data edata;
  m_pValidator->GetElementInfo(szCurElement, edata);
  const int icurrent_element = m_bEntryBeingProcessed ? edata.element_entry_code : edata.element_code;
  if (!XMLFileHandlers::ProcessStartElement(icurrent_element))
    return S_OK;

  switch (icurrent_element) {
    case XLE_ENTRY:
      {
        TCHAR *lpValue1 = FileProcessAttributes(pAttributes, _T("normal"));
        if (lpValue1 != nullptr) {
          m_cur_entry->bforce_normal_entry =
               _ttoi(lpValue1) == 1 || _tcscmp(lpValue1, _T("true")) == 0;
          free(lpValue1);
        }
        TCHAR *lpValue2 = FileProcessAttributes(pAttributes, _T("id"));
        if (lpValue2 != nullptr) {
          m_cur_entry->id = _ttoi(lpValue2);
          free(lpValue2);
        }
      }
      break;
    default:
      break;
  }
  return S_OK;
}

//  ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::characters(
  /* [in] */ const wchar_t __RPC_FAR *pwchChars,
  /* [in] */ int cchChars)
{
  if (m_bValidation)
    return S_OK;

  TCHAR* szData = new TCHAR[cchChars + 2];

  _tcsncpy_s(szData, cchChars + 2, pwchChars, cchChars);

  szData[cchChars] = 0;
  m_sxElemContent += szData;

  delete [] szData;
  szData = nullptr;

  return S_OK;
}

//  -----------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::endElement(
  const wchar_t * pwchNamespaceUri,
  int cchNamespaceUri,
  const wchar_t * pwchLocalName,
  int cchLocalName,
  const wchar_t * pwchQName,
  int cchQName)
{
  wchar_t szCurElement[MAX_PATH + 1] = {0};

  wcsncpy_s(szCurElement, MAX_PATH + 1, pwchQName, cchQName);

  if (m_bValidation) {
    if (wcscmp(szCurElement, L"entry") == 0)
      m_numEntries++;
    return S_OK;
  }

  StringX buffer(_T(""));

  st_file_element_data edata;
  m_pValidator->GetElementInfo(szCurElement, edata);

  // The rest is only processed in Import mode (not Validation mode)
  const int icurrent_element = m_bEntryBeingProcessed ? edata.element_entry_code : edata.element_code;
  XMLFileHandlers::ProcessEndElement(icurrent_element);

  return S_OK;
}

//  ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE  MFileSAX2ContentHandler::endDocument()
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::startPrefixMapping(
  const wchar_t * pwchPrefix,
  int cchPrefix,
  const wchar_t * pwchUri,
  int cchUri)
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::endPrefixMapping(
  const wchar_t * pwchPrefix,
  int cchPrefix)
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::ignorableWhitespace(
  const wchar_t * pwchChars,
  int cchChars)
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::processingInstruction(
  const wchar_t * pwchTarget,
  int cchTarget,
  const wchar_t * pwchData,
  int cchData)
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFileSAX2ContentHandler::skippedEntity(
  const wchar_t * pwchName,
  int cchName)
{
  return S_OK;
}

#endif /* USE_XML_LIBRARY == MSXML */
