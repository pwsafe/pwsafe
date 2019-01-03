/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// MFilterSAX2Filters.cpp : implementation file
//

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == MSXML

#include "MFilterSAX2Handlers.h"
#include <msxml6.h>

#include "../../util.h"
#include "../../core.h"
#include "../../PWSfileV3.h"
#include "../../PWSFilters.h"
#include "../../PWSprefs.h"
#include "../../VerifyFormat.h"
#include "../../Match.h"

#include <map>
#include <algorithm>

typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
typedef std::vector<stringT>::const_iterator vciter;
typedef std::vector<stringT>::iterator viter;

// Stop warnings about unused formal parameters!
#pragma warning(disable : 4100)

//  -----------------------------------------------------------------------
//  MFilterSAX2ErrorHandler Methods
//  -----------------------------------------------------------------------
MFilterSAX2ErrorHandler::MFilterSAX2ErrorHandler()
  : bErrorsFound(FALSE), m_strValidationResult(_T(""))
{
  m_refCnt = 0;
}

MFilterSAX2ErrorHandler::~MFilterSAX2ErrorHandler()
{
}

long __stdcall MFilterSAX2ErrorHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
  *ppvObject = nullptr;
  if (riid == IID_IUnknown ||riid == __uuidof(ISAXContentHandler))
  {
    *ppvObject = static_cast<ISAXErrorHandler *>(this);
  }

  if (*ppvObject)
  {
    AddRef();
    return S_OK;
  }
  else return E_NOINTERFACE;
}

unsigned long __stdcall MFilterSAX2ErrorHandler::AddRef()
{
  return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall MFilterSAX2ErrorHandler::Release()
{
  --m_refCnt; // NOT thread-safe
  if (m_refCnt == 0) {
    delete this;
    return 0; // Can't return the member of a deleted object.
  }
  else return m_refCnt;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ErrorHandler::error(struct ISAXLocator * pLocator,
                                                         const wchar_t * pwchErrorMessage,
                                                         HRESULT hrErrorCode )
{
  TCHAR szErrorMessage[MAX_PATH*2] = {0};
  TCHAR szFormatString[MAX_PATH*2] = {0};
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

HRESULT STDMETHODCALLTYPE MFilterSAX2ErrorHandler::fatalError(struct ISAXLocator * pLocator,
                                                          const wchar_t * pwchErrorMessage,
                                                          HRESULT hrErrorCode )
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ErrorHandler::ignorableWarning(struct ISAXLocator * pLocator,
                                                                const wchar_t * pwchErrorMessage,
                                                                HRESULT hrErrorCode )
{
  return S_OK;
}

//  -----------------------------------------------------------------------
//  MFilterSAX2ContentHandler Methods
//  -----------------------------------------------------------------------
MFilterSAX2ContentHandler::MFilterSAX2ContentHandler()
{
  m_refCnt = 0;
  m_sxElemContent = _T("");
  m_pSchema_Version = nullptr;
  m_iXMLVersion = -1;
  m_iSchemaVersion = -1;
  m_pAsker = nullptr;
}

//  -----------------------------------------------------------------------
MFilterSAX2ContentHandler::~MFilterSAX2ContentHandler()
{
}

long __stdcall MFilterSAX2ContentHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
  *ppvObject = nullptr;
  if (riid == IID_IUnknown ||riid == __uuidof(ISAXContentHandler)) {
    *ppvObject = static_cast<ISAXContentHandler *>(this);
  }

  if (*ppvObject) {
    AddRef();
    return S_OK;
  }
  else return E_NOINTERFACE;
}

unsigned long __stdcall MFilterSAX2ContentHandler::AddRef()
{
  return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall MFilterSAX2ContentHandler::Release()
{
  --m_refCnt; // NOT thread-safe
  if (m_refCnt == 0) {
    delete this;
    return 0; // Can't return the member of a deleted object.
  }
  else return m_refCnt;
}

//  -----------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::startDocument ( )
{
  m_strXMLErrors = _T("");
  m_bEntryBeingProcessed = false;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::putDocumentLocator (struct ISAXLocator * pLocator )
{
  return S_OK;
}

//  ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::startElement(
  /* [in] */ const wchar_t __RPC_FAR *pwchNamespaceUri,
  /* [in] */ int cchNamespaceUri,
  /* [in] */ const wchar_t __RPC_FAR *pwchLocalName,
  /* [in] */ int cchLocalName,
  /* [in] */ const wchar_t __RPC_FAR *pwchRawName,
  /* [in] */ int cchRawName,
  /* [in] */ ISAXAttributes __RPC_FAR *pAttributes)
{
  TCHAR szCurElement[MAX_PATH + 1] = {0};

  _tcsncpy_s(szCurElement, MAX_PATH + 1, pwchRawName, cchRawName);

  if (m_bValidation && _tcscmp(szCurElement, _T("filters")) == 0) {
    int iAttribs = 0;
    if (m_pSchema_Version == nullptr) {
      LoadAString(m_strXMLErrors, IDSC_MISSING_SCHEMA_VER);
      return E_FAIL;
    }

    m_iSchemaVersion = _wtoi(*m_pSchema_Version);
    if (m_iSchemaVersion <= 0) {
      LoadAString(m_strXMLErrors, IDSC_INVALID_SCHEMA_VER);
      return E_FAIL;
    }

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
      if (QName_length == 7 && memcmp(szQName, _T("version"), 7 * sizeof(TCHAR)) == 0) {
        m_iXMLVersion = _ttoi(szValue);
      }
    }
  }

  if (m_bValidation || _tcscmp(szCurElement, _T("filters")) == 0)
    return S_OK;

  bool bfilter = (_tcscmp(szCurElement, _T("filter")) == 0);
  bool bfilter_entry = (_tcscmp(szCurElement, _T("filter_entry")) == 0);

  if (bfilter) {
    cur_filter = new st_filters;
  }

  if (bfilter_entry) {
    cur_filterentry = new st_FilterRow;
    cur_filterentry->Empty();
    cur_filterentry->bFilterActive = true;
    m_bEntryBeingProcessed = true;
  }

  if (bfilter || bfilter_entry) {
    // Process the attributes we need.
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

      if (bfilter && QName_length == 10 && memcmp(szQName, _T("filtername"), 10 * sizeof(TCHAR)) == 0)
        cur_filter->fname = szValue;

      if (bfilter_entry && QName_length == 6 && memcmp(szQName, _T("active"), 6 * sizeof(TCHAR)) == 0) {
        if (Value_length == 2 && memcmp(szValue, _T("no"), 2 * sizeof(TCHAR)) == 0)
          cur_filterentry->bFilterActive = false;
      }
    }
  }

  m_sxElemContent = _T("");

  return S_OK;
}

//  ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::characters(
  /* [in] */ const wchar_t __RPC_FAR *pwchChars,
  /* [in] */ int cchChars)
{
  if (m_bValidation)
    return S_OK;

  TCHAR* szData = new TCHAR[cchChars + 2];

  _tcsncpy_s(szData, cchChars + 2, pwchChars, cchChars);

  szData[cchChars]=0;
  m_sxElemContent += szData;

  delete [] szData;
  szData = nullptr;

  return S_OK;
}

//  -----------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::endElement (
  const wchar_t * pwchNamespaceUri,
  int cchNamespaceUri,
  const wchar_t * pwchLocalName,
  int cchLocalName,
  const wchar_t * pwchQName,
  int cchQName)
{
  TCHAR szCurElement[MAX_PATH + 1] = {0};

  _tcsncpy_s(szCurElement, MAX_PATH + 1, pwchQName, cchQName);

  if (m_bValidation && _tcscmp(szCurElement, _T("filters")) == 0) {
    // Check that the XML file version is present and that
    // a. it is less than or equal to the Filter schema version
    // b. it is less than or equal to the version supported by this PWS
    if (m_iXMLVersion < 0) {
      LoadAString(m_strXMLErrors, IDSC_MISSING_XML_VER);
      return E_FAIL;
    }
    if (m_iXMLVersion > m_iSchemaVersion) {
      Format(m_strXMLErrors,
             IDSC_INVALID_XML_VER1, m_iXMLVersion, m_iSchemaVersion);
      return E_FAIL;
    }
    if (m_iXMLVersion > PWS_XML_FILTER_VERSION) {
      Format(m_strXMLErrors,
             IDSC_INVALID_XML_VER2, m_iXMLVersion, PWS_XML_FILTER_VERSION);
      return E_FAIL;
    }
  }

  if (m_bValidation) {
    return S_OK;
  }

  if (_tcscmp(szCurElement, _T("filter")) == 0) {
    INT_PTR rc = IDYES;
    st_Filterkey fk;
    fk.fpool = m_FPool;
    fk.cs_filtername = cur_filter->fname;
    if (m_MapXMLFilters->find(fk) != m_MapXMLFilters->end()) {
      stringT question;
      Format(question, IDSC_FILTEREXISTS, cur_filter->fname.c_str());
      if (m_pAsker == nullptr || !(*m_pAsker)(question)) {
        m_MapXMLFilters->erase(fk);
      }
    }
    if (rc == IDYES) {
      m_MapXMLFilters->insert(PWSFilters::Pair(fk, *cur_filter));
    }
    delete cur_filter;
    return S_OK;
  }

  else if (_tcscmp(szCurElement, _T("filter_entry")) == 0) {
    if (cur_filterentry->mtype  == PWSMatch::MT_DATE &&
        cur_filterentry->rule   != PWSMatch::MR_PRESENT &&
        cur_filterentry->rule   != PWSMatch::MR_NOTPRESENT &&
        cur_filterentry->fdate1 == (time_t)0 &&
        cur_filterentry->fdate2 == (time_t)0)
      cur_filterentry->fdatetype = 1; // Relative Date
    if (m_type == DFTYPE_MAIN) {
      cur_filter->num_Mactive++;
      cur_filter->vMfldata.push_back(*cur_filterentry);
    } else if (m_type == DFTYPE_PWHISTORY) {
      cur_filter->num_Hactive++;
      cur_filter->vHfldata.push_back(*cur_filterentry);
    } else if (m_type == DFTYPE_PWPOLICY) {
      cur_filter->num_Pactive++;
      cur_filter->vPfldata.push_back(*cur_filterentry);
    } else if (m_type == DFTYPE_ATTACHMENT) {
      cur_filter->num_Aactive++;
      cur_filter->vAfldata.push_back(*cur_filterentry);
    }
    delete cur_filterentry;
  }

  else if (_tcscmp(szCurElement, _T("grouptitle")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_GROUPTITLE;
  }

  else if (_tcscmp(szCurElement, _T("group")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_GROUP;
  }

  else if (_tcscmp(szCurElement, _T("title")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_TITLE;
  }

  else if (_tcscmp(szCurElement, _T("user")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_USER;
  }

  else if (_tcscmp(szCurElement, _T("password")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_PASSWORD;
    cur_filterentry->ftype = FT_PASSWORD;
  }

  else if (_tcscmp(szCurElement, _T("notes")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_NOTES;
  }

  else if (_tcscmp(szCurElement, _T("url")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_URL;
  }

  else if (_tcscmp(szCurElement, _T("autotype")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_AUTOTYPE;
  }

  else if (_tcscmp(szCurElement, _T("runcommand")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_RUNCMD;
  }

  else if (_tcscmp(szCurElement, _T("DCA")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DCA;
    cur_filterentry->ftype = FT_DCA;
  }

  else if (_tcscmp(szCurElement, _T("ShiftDCA")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_SHIFTDCA;
    cur_filterentry->ftype = FT_SHIFTDCA;
  }

  else if (_tcscmp(szCurElement, _T("email")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_EMAIL;
  }

  else if (_tcscmp(szCurElement, _T("protected")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = FT_PROTECTED;
  }

  else if (_tcscmp(szCurElement, _T("kbshortcut")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = FT_KBSHORTCUT;
  }

  else if (_tcscmp(szCurElement, _T("symbols")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_SYMBOLS;
    cur_filterentry->fstring = PWSUtil::DeDupString(cur_filterentry->fstring);
  }

  else if (_tcscmp(szCurElement, _T("policy_name")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_POLICYNAME;
  }

  else if (_tcscmp(szCurElement, _T("create_time")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_CTIME;
  }

  else if (_tcscmp(szCurElement, _T("password_modified_time")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_PMTIME;
  }

  else if (_tcscmp(szCurElement, _T("last_access_time")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_ATIME;
  }

  else if (_tcscmp(szCurElement, _T("expiry_time")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_XTIME;
  }

  else if (_tcscmp(szCurElement, _T("record_modified_time")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_RMTIME;
  }

  else if (_tcscmp(szCurElement, _T("password_expiry_interval")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = FT_XTIME_INT;
  }

  else if (_tcscmp(szCurElement, _T("password_length")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = FT_PASSWORDLEN;
  }

  else if (_tcscmp(szCurElement, _T("entrytype")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYTYPE;
    cur_filterentry->ftype = FT_ENTRYTYPE;
  }

  else if (_tcscmp(szCurElement, _T("entrystatus")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYSTATUS;
    cur_filterentry->ftype = FT_ENTRYSTATUS;
  }

  else if (_tcscmp(szCurElement, _T("entrysize")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYSIZE;
    cur_filterentry->ftype = FT_ENTRYSIZE;
  }

  else if (_tcscmp(szCurElement, _T("unknownfields")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->ftype = FT_UNKNOWNFIELDS;
  }

  else if (_tcscmp(szCurElement, _T("password_history")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_PWHIST;
    cur_filterentry->ftype = FT_PWHIST;
  }

  else if (_tcscmp(szCurElement, _T("history_present")) == 0) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = HT_PRESENT;
  }

  else if (_tcscmp(szCurElement, _T("history_active")) == 0) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = HT_ACTIVE;
  }

  else if (_tcscmp(szCurElement, _T("history_number")) == 0) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = HT_NUM;
  }

  else if (_tcscmp(szCurElement, _T("history_maximum")) == 0) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = HT_MAX;
  }

  else if (_tcscmp(szCurElement, _T("history_changedate")) == 0) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = HT_CHANGEDATE;
  }

  else if (_tcscmp(szCurElement, _T("history_passwords")) == 0) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_PASSWORD;
    cur_filterentry->ftype = HT_PASSWORDS;
  }

  else if (_tcscmp(szCurElement, _T("password_policy")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_POLICY;
    cur_filterentry->ftype = FT_POLICY;
  }

  else if (_tcscmp(szCurElement, _T("policy_present")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_PRESENT;
  }

  else if (_tcscmp(szCurElement, _T("policy_length")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_LENGTH;
  }

  else if (_tcscmp(szCurElement, _T("policy_number_lowercase")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_LOWERCASE;
  }

  else if (_tcscmp(szCurElement, _T("policy_number_uppercase")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_UPPERCASE;
  }

  else if (_tcscmp(szCurElement, _T("policy_number_digits")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_DIGITS;
  }

  else if (_tcscmp(szCurElement, _T("policy_number_symbols")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_SYMBOLS;
  }

  else if (_tcscmp(szCurElement, _T("policy_easyvision")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_EASYVISION;
  }

  else if (_tcscmp(szCurElement, _T("policy_pronounceable")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_PRONOUNCEABLE;
  }

  else if (_tcscmp(szCurElement, _T("policy_hexadecimal")) == 0) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_HEXADECIMAL;
  }

  else if (_tcscmp(szCurElement, _T("attachment")) == 0) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ATTACHMENT;
    cur_filterentry->ftype = FT_ATTACHMENT;
  }

  else if (_tcscmp(szCurElement, _T("attachment_present")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = AT_PRESENT;
  }

  else if (_tcscmp(szCurElement, _T("attachment_title")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_TITLE;
  }

  else if (_tcscmp(szCurElement, _T("attachment_mediatype")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_MEDIATYPE;
  }

  else if (_tcscmp(szCurElement, _T("attachment_filename")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_FILENAME;
  }

  else if (_tcscmp(szCurElement, _T("attachment_filepath")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_FILEPATH;
  }

  else if (_tcscmp(szCurElement, _T("attachment_ctime")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_CTIME;
  }

  else if (_tcscmp(szCurElement, _T("attachment_filectime")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_FILECTIME;
  }

  else if (_tcscmp(szCurElement, _T("attachment_filemtime")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_FILEMTIME;
  }

  else if (_tcscmp(szCurElement, _T("attachment_fileatime")) == 0) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_FILEATIME;
  }

  else if (_tcscmp(szCurElement, _T("rule")) == 0) {
    ToUpper(m_sxElemContent);
    cur_filterentry->rule = PWSMatch::GetRule(m_sxElemContent);
  }

  else if (_tcscmp(szCurElement, _T("logic")) == 0) {
    if (m_sxElemContent == _T("or"))
      cur_filterentry->ltype = LC_OR;
    else
      cur_filterentry->ltype = LC_AND;
  }

  else if (_tcscmp(szCurElement, _T("string")) == 0) {
    cur_filterentry->fstring = m_sxElemContent;
  }

  else if (_tcscmp(szCurElement, _T("case")) == 0) {
    cur_filterentry->fcase = _ttoi(m_sxElemContent.c_str()) != 0;
  }

  else if (_tcscmp(szCurElement, _T("warn")) == 0) {
    cur_filterentry->fnum1 = _ttoi(m_sxElemContent.c_str());
  }

  else if (_tcscmp(szCurElement, _T("num1")) == 0) {
    cur_filterentry->fnum1 = _ttoi(m_sxElemContent.c_str());
  }

  else if (_tcscmp(szCurElement, _T("num2")) == 0) {
    cur_filterentry->fnum2 = _ttoi(m_sxElemContent.c_str());
  }

  else if (_tcscmp(szCurElement, _T("unit")) == 0) {
    cur_filterentry->funit = _ttoi(m_sxElemContent.c_str());
  }

  else if (_tcscmp(szCurElement, _T("date1")) == 0) {
    time_t t(0);
    if (VerifyXMLDateString(m_sxElemContent.c_str(), t) &&
        (t != (time_t)-1))
      cur_filterentry->fdate1 = t;
    else
      cur_filterentry->fdate1 = (time_t)0;
  }

  else if (_tcscmp(szCurElement, _T("date2")) == 0) {
    time_t t(0);
    if (VerifyXMLDateString(m_sxElemContent.c_str(), t) &&
        (t != (time_t)-1))
      cur_filterentry->fdate2 = t;
    else
      cur_filterentry->fdate1 = (time_t)0;
  }

  else if (_tcscmp(szCurElement, _T("dca")) == 0) {
    cur_filterentry->fdca = (short)_ttoi(m_sxElemContent.c_str());
  }

  else if (_tcscmp(szCurElement, _T("shiftdca")) == 0) {
    cur_filterentry->fdca = (short)_ttoi(m_sxElemContent.c_str());
  }

  else if (_tcscmp(szCurElement, _T("type")) == 0) {
    if (m_sxElemContent == _T("normal"))
      cur_filterentry->etype = CItemData::ET_NORMAL;
    else if (m_sxElemContent == _T("alias"))
      cur_filterentry->etype = CItemData::ET_ALIAS;
    else if (m_sxElemContent == _T("shortcut"))
      cur_filterentry->etype = CItemData::ET_SHORTCUT;
    else if (m_sxElemContent == _T("aliasbase"))
      cur_filterentry->etype = CItemData::ET_ALIASBASE;
    else if (m_sxElemContent == _T("shortcutbase"))
      cur_filterentry->etype = CItemData::ET_SHORTCUTBASE;
    else
      cur_filterentry->etype = CItemData::ET_INVALID;
  }

  else if (_tcscmp(szCurElement, _T("status")) == 0) {
    if (m_sxElemContent == _T("clean"))
      cur_filterentry->estatus = CItemData::ES_CLEAN;
    else if (m_sxElemContent == _T("added"))
      cur_filterentry->estatus = CItemData::ES_ADDED;
    else if (m_sxElemContent == _T("modified"))
      cur_filterentry->estatus = CItemData::ES_MODIFIED;
    else
      cur_filterentry->estatus = CItemData::ES_INVALID;
  } else if (!(_tcscmp(szCurElement, _T("test")) == 0 ||
               _tcscmp(szCurElement, _T("filters")) == 0))
    ASSERT(0);

  return S_OK;
}

//  ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::endDocument ( )
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::startPrefixMapping (
  const wchar_t * pwchPrefix,
  int cchPrefix,
  const wchar_t * pwchUri,
  int cchUri )
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::endPrefixMapping (
  const wchar_t * pwchPrefix,
  int cchPrefix )
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::ignorableWhitespace (
  const wchar_t * pwchChars,
  int cchChars )
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::processingInstruction (
  const wchar_t * pwchTarget,
  int cchTarget,
  const wchar_t * pwchData,
  int cchData )
{
  return S_OK;
}

HRESULT STDMETHODCALLTYPE MFilterSAX2ContentHandler::skippedEntity (
  const wchar_t * pwchName,
  int cchName )
{
  return S_OK;
}

#endif /* USE_XML_LIBRARY == MSXML */
