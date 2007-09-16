/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// SAXHandlers.cpp : implementation file
//

#include "../stdafx.h"
#include "corelib.h"
#include "PWScore.h"
#include "ItemData.h"
#include "MyString.h"
#include "util.h"
#include "SAXHandlers.h"
#include "UUIDGen.h"
#include "xml_import.h"
#include "corelib.h"
#include "PWSfileV3.h"
#include "PWSprefs.h"

// Stop warnings about unused formal parameters!
#pragma warning(disable : 4100)

//	-----------------------------------------------------------------------
//	PWSSAXErrorHandler Methods
//	-----------------------------------------------------------------------
PWSSAXErrorHandler::PWSSAXErrorHandler():
	bErrorsFound(FALSE),
	m_strValidationResult("")
{
	m_refCnt = 0;
}

PWSSAXErrorHandler::~PWSSAXErrorHandler()
{
}

long __stdcall PWSSAXErrorHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
	*ppvObject = NULL;
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

unsigned long __stdcall PWSSAXErrorHandler::AddRef()
{
	 return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall PWSSAXErrorHandler::Release()
{
	--m_refCnt; // NOT thread-safe
   if (m_refCnt == 0) {
	  delete this;
	  return 0; // Can't return the member of a deleted object.
   }
   else return m_refCnt;
}

HRESULT STDMETHODCALLTYPE PWSSAXErrorHandler::error (
        struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode )
{
	TCHAR szErrorMessage[MAX_PATH*2] = {0};
	TCHAR szFormatString[MAX_PATH*2] = {0};
	int iLineNumber, iCharacter;

#ifdef _UNICODE
#if (_MSC_VER >= 1400)
	_tcscpy_s(szErrorMessage, MAX_PATH * 2, pwchErrorMessage);
#else
	_tcscpy(szErrorMessage, pwchErrorMessage);
#endif
#else
#if (_MSC_VER >= 1400)
	size_t num_converted;
	wcstombs_s(&num_converted, szErrorMessage, MAX_PATH*2, pwchErrorMessage, MAX_PATH);
#else
	wcstombs(szErrorMessage, pwchErrorMessage, MAX_PATH);
#endif
#endif
	pLocator->getLineNumber(&iLineNumber);
	pLocator->getColumnNumber(&iCharacter);

	const CString cs_format(MAKEINTRESOURCE(IDSC_SAXGENERROR));

#if (_MSC_VER >= 1400)
	_stprintf_s(szFormatString, MAX_PATH*2, cs_format,
		hrErrorCode, iLineNumber, iCharacter, szErrorMessage);
#else
	_stprintf(szFormatString, cs_format,
		hrErrorCode, iLineNumber, iCharacter, szErrorMessage);
#endif

	m_strValidationResult += szFormatString;

	bErrorsFound = TRUE;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXErrorHandler::fatalError (
		struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXErrorHandler::ignorableWarning (
		struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode )
{
	return S_OK;
}

//	-----------------------------------------------------------------------
//	PWSSAXContentHandler Methods
//	-----------------------------------------------------------------------
PWSSAXContentHandler::PWSSAXContentHandler()
{
	m_refCnt = 0;
	m_strElemContent.Empty();
	m_numEntries = 0;
	m_ImportedPrefix = _T("");
	m_delimiter = _T('^');
  m_bheader = false;
  m_bDatabaseHeaderErrors = false;
  m_bRecordHeaderErrors = false;
  m_nITER = 0;
  m_nRecordsWithUnknownFields = 0;

  m_bDisplayExpandedAddEditDlg = -1;
  m_bMaintainDateTimeStamps = -1;
  m_bPWUseDigits = -1;
  m_bPWUseEasyVision = -1;
  m_bPWUseHexDigits = -1;
  m_bPWUseLowercase = -1;
  m_bPWUseSymbols = -1;
  m_bPWUseUppercase = -1;
  m_bSaveImmediately = -1;
  m_bSavePasswordHistory = -1;
  m_bShowNotesDefault = -1;
  m_bShowPasswordInTree = -1;
  m_bShowPWDefault = -1;
  m_bShowUsernameInTree = -1;
  m_bSortAscending = -1;
  m_bUseDefaultUser = -1;
  m_iIdleTimeout = -1;
  m_iNumPWHistoryDefault = -1;
  m_iPWDefaultLength = -1;
  m_iTreeDisplayStatusAtOpen = -1;
  m_sDefaultAutotypeString = _T("");
  m_sDefaultUsername = _T("");
}

//	-----------------------------------------------------------------------
PWSSAXContentHandler::~PWSSAXContentHandler()
{
  m_ukhxl.clear();
}

void PWSSAXContentHandler::SetVariables(PWScore *core, const bool &bValidation,
									  const CString &ImportedPrefix, const TCHAR &delimiter)
{
	m_bValidation = bValidation;
	m_ImportedPrefix = ImportedPrefix;
	m_delimiter = delimiter;
	m_xmlcore = core;
}

long __stdcall PWSSAXContentHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
	*ppvObject = NULL;
	if (riid == IID_IUnknown ||riid == __uuidof(ISAXContentHandler)) {
		*ppvObject = static_cast<ISAXContentHandler *>(this);
	}

	if (*ppvObject) {
		AddRef();
		return S_OK;
	}
	else return E_NOINTERFACE;
}

unsigned long __stdcall PWSSAXContentHandler::AddRef()
{
	 return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall PWSSAXContentHandler::Release()
{
	--m_refCnt; // NOT thread-safe
   if (m_refCnt == 0) {
	  delete this;
	  return 0; // Can't return the member of a deleted object.
   }
   else return m_refCnt;
}

//	-----------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::startDocument ( )
{
	m_strImportErrors = _T("");
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::putDocumentLocator (struct ISAXLocator * pLocator )
{
	return S_OK;
}

//	---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE PWSSAXContentHandler::startElement(
    /* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
    /* [in] */ int cchNamespaceUri,
    /* [in] */ wchar_t __RPC_FAR *pwchLocalName,
    /* [in] */ int cchLocalName,
    /* [in] */ wchar_t __RPC_FAR *pwchRawName,
    /* [in] */ int cchRawName,
    /* [in] */ ISAXAttributes __RPC_FAR *pAttributes)
{
	TCHAR szCurElement[MAX_PATH+1] = {0};

#ifdef _UNICODE
#if (_MSC_VER >= 1400)
  _tcsncpy_s(szCurElement, MAX_PATH+1, pwchRawName, cchRawName);
#else
	_tcsncpy(szCurElement, pwchRawName, cchRawName);
#endif
#else
#if (_MSC_VER >= 1400)
	size_t num_converted;
	wcstombs_s(&num_converted, szCurElement, MAX_PATH+1, pwchRawName, cchRawName);
#else
	wcstombs(szCurElement, pwchRawName, cchRawName);
#endif
#endif

	if (_tcscmp(szCurElement, _T("passwordsafe")) == 0) {
		if (m_bValidation) {
			int iAttribs = 0;
			pAttributes->getLength(&iAttribs);
			for (int i = 0; i < iAttribs; i++) {
				TCHAR szQName[MAX_PATH + 1] = {0};
				TCHAR szValue[MAX_PATH + 1] = {0};
				wchar_t *QName, *Value;
				int QName_length, Value_length;

				pAttributes->getQName(i, &QName, &QName_length);
				pAttributes->getValue(i, &Value, &Value_length);
#ifdef _UNICODE
#if (_MSC_VER >= 1400)
				_tcsncpy_s(szQName, MAX_PATH + 1, QName, QName_length);
				_tcsncpy_s(szValue, MAX_PATH + 1, Value, Value_length);
#else
				_tcsncpy(szQName, QName, QName_length);
				_tcsncpy(szValue, Value, Value_length);
#endif
#else
#if (_MSC_VER >= 1400)
				wcstombs_s(&num_converted, szQName, MAX_PATH+1, QName, QName_length);
				wcstombs_s(&num_converted, szValue, MAX_PATH+1, Value, Value_length);
#else
				wcstombs(szQName, QName, QName_length);
				wcstombs(szValue, Value, Value_length);
#endif
#endif
				if (_tcscmp(szQName, _T("delimiter")) == 0)
					m_delimiter = szValue[0];

        // We do not save or copy the imported file_uuid_array
        //   szQName == _T("Database_uuid")
			}
		}
	}

	if (m_bValidation)
		return S_OK;

  if (_tcscmp(szCurElement, _T("unknownheaderfields")) == 0) {
		m_ukhxl.clear();
    m_bheader = true;
  }

  if (_tcscmp(szCurElement, _T("field")) == 0) {
    int iAttribs = 0;
		pAttributes->getLength(&iAttribs);
		for (int i = 0; i < iAttribs; i++) {
			TCHAR szQName[MAX_PATH + 1] = {0};
			TCHAR szValue[MAX_PATH + 1] = {0};
			wchar_t *QName, *Value;
			int QName_length, Value_length;

			pAttributes->getQName(i, &QName, &QName_length);
			pAttributes->getValue(i, &Value, &Value_length);
#ifdef _UNICODE
#if (_MSC_VER >= 1400)
			_tcsncpy_s(szQName, MAX_PATH + 1, QName, QName_length);
			_tcsncpy_s(szValue, MAX_PATH + 1, Value, Value_length);
#else
			_tcsncpy(szQName, QName, QName_length);
			_tcsncpy(szValue, Value, Value_length);
#endif
#else
#if (_MSC_VER >= 1400)
			wcstombs_s(&num_converted, szQName, MAX_PATH+1, QName, QName_length);
			wcstombs_s(&num_converted, szValue, MAX_PATH+1, Value, Value_length);
#else
			wcstombs(szQName, QName, QName_length);
			wcstombs(szValue, Value, Value_length);
#endif
#endif
			if (_tcscmp(szQName, _T("ftype")) == 0)
				m_ctype = (unsigned char)_ttoi(szValue);
		}
  }

	if (_tcscmp(szCurElement, _T("entry")) == 0) {
		cur_entry = new pw_entry;
		cur_entry->group = _T("");
		cur_entry->title = _T("");
		cur_entry->username = _T("");
		cur_entry->password = _T("");
		cur_entry->url = _T("");
		cur_entry->autotype = _T("");
		cur_entry->ctime = _T("");
		cur_entry->atime = _T("");
		cur_entry->ltime = _T("");
		cur_entry->pmtime = _T("");
		cur_entry->rmtime = _T("");
		cur_entry->changed = _T("");
		cur_entry->pwhistory = _T("");
		cur_entry->notes = _T("");
		cur_entry->uuid = _T("");
	}

	if (_tcscmp(szCurElement, _T("ctime")) == 0)
		m_whichtime = PW_CTIME;

	if (_tcscmp(szCurElement, _T("atime")) == 0)
		m_whichtime = PW_ATIME;

	if (_tcscmp(szCurElement, _T("ltime")) == 0)
		m_whichtime = PW_LTIME;

	if (_tcscmp(szCurElement, _T("pmtime")) == 0)
		m_whichtime = PW_PMTIME;

	if (_tcscmp(szCurElement, _T("rmtime")) == 0)
		m_whichtime = PW_RMTIME;

	if (_tcscmp(szCurElement, _T("changed")) == 0)
		m_whichtime = PW_CHANGED;

	m_strElemContent = _T("");

	return S_OK;
}

//	---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE PWSSAXContentHandler::characters(
            /* [in] */ wchar_t __RPC_FAR *pwchChars,
            /* [in] */ int cchChars)
{
	if (m_bValidation)
		return S_OK;

	TCHAR* szData = new TCHAR[cchChars+2];

#ifdef _UNICODE
#if (_MSC_VER >= 1400)
	_tcsncpy_s(szData, cchChars+2, pwchChars, cchChars);
#else
	_tcsncpy(szData, pwchChars, cchChars);
#endif
#else
#if _MSC_VER >= 1400
	size_t num_converted;
	wcstombs_s(&num_converted, szData, cchChars+2, pwchChars, cchChars);
#else
	wcstombs(szData, pwchChars, cchChars);
#endif
#endif

	szData[cchChars]=0;
	m_strElemContent += szData;

	delete [] szData;
	szData = NULL;

	return S_OK;
}

//	-----------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::endElement (
                                                             unsigned short * pwchNamespaceUri,
                                                             int cchNamespaceUri,
                                                             unsigned short * pwchLocalName,
                                                             int cchLocalName,
                                                             unsigned short * pwchQName,
                                                             int cchQName)
{
	TCHAR szCurElement[MAX_PATH+1] = {0};

#ifdef _UNICODE
#if (_MSC_VER >= 1400)
	_tcsncpy_s(szCurElement, MAX_PATH+1, pwchQName, cchQName);
#else
	_tcsncpy(szCurElement, pwchQName, cchQName);
#endif
#else
#if (_MSC_VER >= 1400)
	size_t num_converted;
	wcstombs_s(&num_converted, szCurElement, MAX_PATH+1, pwchQName, cchQName);
#else
	wcstombs(szCurElement, pwchQName, cchQName);
#endif
#endif

	if (m_bValidation) {
		if (_tcscmp(szCurElement, _T("entry")) == 0)
			m_numEntries++;

		return S_OK;
	}

	if (_tcscmp(szCurElement, _T("entry")) == 0) {
		CItemData tempitem;
		tempitem.Clear();
		if (cur_entry->uuid.IsEmpty())
			tempitem.CreateUUID();
		else {
			uuid_array_t uuid_array;
      // _stscanf_s always outputs to an "int" using %x even though
      // target is only 1.  Read into larger buffer to prevent data being
      // overwritten and then copy to where we want it!
      unsigned char temp_uuid_array[sizeof(uuid_array_t) + sizeof(int)];
			int nscanned = 0;
			TCHAR *lpszuuid = cur_entry->uuid.GetBuffer(sizeof(uuid_array_t) * 2);
 			for (unsigned i = 0; i < sizeof(uuid_array_t); i++) {
#if _MSC_VER >= 1400
        nscanned += _stscanf_s(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#else
        nscanned += _stscanf(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#endif
        lpszuuid += 2;
      }
      cur_entry->uuid.ReleaseBuffer(sizeof(uuid_array_t) * 2);
      memcpy(uuid_array, temp_uuid_array, sizeof(uuid_array_t));
			if (nscanned != sizeof(uuid_array_t) ||
          m_xmlcore->Find(uuid_array) != m_xmlcore->GetEntryEndIter())
				tempitem.CreateUUID();
			else {
				tempitem.SetUUID(uuid_array);
      }
		}
		CMyString newgroup(m_ImportedPrefix.IsEmpty() ? _T("") : m_ImportedPrefix + _T("."));
		newgroup += cur_entry->group;
		if (m_xmlcore->Find(newgroup, cur_entry->title, cur_entry->username) != 
        m_xmlcore->GetEntryEndIter()) {
      // Find a unique "Title"
      CMyString Unique_Title;
      ItemListConstIter iter;
      int i = 0;
      CString s_import;
      do {
        i++;
        s_import.Format(IDSC_IMPORTNUMBER, i);
        Unique_Title = cur_entry->title + CMyString(s_import);
        iter = m_xmlcore->Find(newgroup, Unique_Title, cur_entry->username);
      } while (iter != m_xmlcore->GetEntryEndIter());
      cur_entry->title = Unique_Title;
    }
		tempitem.SetGroup(newgroup);
		if (cur_entry->title.GetLength() != 0)
			tempitem.SetTitle(cur_entry->title, m_delimiter);
		if (cur_entry->username.GetLength() != 0)
			tempitem.SetUser(cur_entry->username);
		if (cur_entry->password.GetLength() != 0)
			tempitem.SetPassword(cur_entry->password);
		if (cur_entry->url.GetLength() != 0)
			tempitem.SetURL(cur_entry->url);
		if (cur_entry->autotype.GetLength() != 0)
			tempitem.SetAutoType(cur_entry->autotype);
		if (cur_entry->ctime.GetLength() != 0)
			tempitem.SetCTime(cur_entry->ctime);
		if (cur_entry->pmtime.GetLength() != 0)
			tempitem.SetPMTime(cur_entry->pmtime);
		if (cur_entry->atime.GetLength() != 0)
			tempitem.SetATime(cur_entry->atime);
		if (cur_entry->ltime.GetLength() != 0)
			tempitem.SetLTime(cur_entry->ltime);
		if (cur_entry->rmtime.GetLength() != 0)
			tempitem.SetRMTime(cur_entry->rmtime);
		CMyString newPWHistory;
		CString strPWHErrors, buffer;
		buffer.Format(IDSC_SAXERRORPWH,
                  cur_entry->group, cur_entry->title, cur_entry->username);
		switch (PWSUtil::VerifyImportPWHistoryString(cur_entry->pwhistory, newPWHistory, strPWHErrors)) {
    case PWH_OK:
      tempitem.SetPWHistory(newPWHistory);
      buffer.Empty();
      break;
    case PWH_IGNORE:
      buffer.Empty();
      break;
    case PWH_INVALID_HDR:
    case PWH_INVALID_STATUS:
    case PWH_INVALID_NUM:
    case PWH_INVALID_DATETIME:
    case PWH_INVALID_PSWD_LENGTH:
    case PWH_TOO_SHORT:
    case PWH_TOO_LONG:
    case PWH_INVALID_CHARACTER:
      buffer += strPWHErrors;
      break;
    default:
      ASSERT(0);
		}
		m_strImportErrors += buffer;
		if (cur_entry->notes.GetLength() != 0)
			tempitem.SetNotes(cur_entry->notes, m_delimiter);

    if (!cur_entry->uhrxl.empty()) {
      UnknownFieldList::const_iterator vi_IterUXRFE;
      for (vi_IterUXRFE = cur_entry->uhrxl.begin();
           vi_IterUXRFE != cur_entry->uhrxl.end();
           vi_IterUXRFE++) {
        UnknownFieldEntry unkrfe = *vi_IterUXRFE;
#ifdef _DEBUG
        CString cs_timestamp;
        cs_timestamp = PWSUtil::GetTimeStamp();
        TRACE(_T("%s: Record %s, %s, %s has unknown field: %02x, length %d/0x%04x, value:\n"),
              cs_timestamp, cur_entry->group, cur_entry->title, cur_entry->username, 
          unkrfe.uc_Type, (int)unkrfe.st_length, (int)unkrfe.st_length);
        PWSUtil::HexDump(unkrfe.uc_pUField, (int)unkrfe.st_length, cs_timestamp);
#endif
        tempitem.SetUnknownField(unkrfe.uc_Type, (int)unkrfe.st_length, unkrfe.uc_pUField);
      }
    }

		m_xmlcore->AddEntry(tempitem);
    cur_entry->uhrxl.clear();
		delete cur_entry;
		m_numEntries++;
	}

	if (_tcscmp(szCurElement, _T("group")) == 0) {
		cur_entry->group = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("title")) == 0) {
		cur_entry->title = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("username")) == 0) {
		cur_entry->username = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("password")) == 0) {
		cur_entry->password = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("url")) == 0) {
		cur_entry->url = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("autotype")) == 0) {
		cur_entry->autotype = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("notes")) == 0) {
		cur_entry->notes = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("uuid")) == 0) {
		cur_entry->uuid = m_strElemContent;
	}

	if (_tcscmp(szCurElement, _T("status")) == 0) {
		CString buffer;
		int i = _ttoi(m_strElemContent);
		buffer.Format(_T("%01x"), i);
		cur_entry->pwhistory = CMyString(buffer);
	}

	if (_tcscmp(szCurElement, _T("max")) == 0) {
		CString buffer;
		int i = _ttoi(m_strElemContent);
		buffer.Format(_T("%02x"), i);
		cur_entry->pwhistory += CMyString(buffer);
	}

	if (_tcscmp(szCurElement, _T("num")) == 0) {
		CString buffer;
		int i = _ttoi(m_strElemContent);
		buffer.Format(_T("%02x"), i);
		cur_entry->pwhistory += CMyString(buffer);
	}

	if (_tcscmp(szCurElement, _T("ctime")) == 0) {
		cur_entry->ctime.Replace(_T('-'), _T('/'));
		m_whichtime = -1;
	}

	if (_tcscmp(szCurElement, _T("pmtime")) == 0) {
		cur_entry->pmtime.Replace(_T('-'), _T('/'));
		m_whichtime = -1;
	}

	if (_tcscmp(szCurElement, _T("atime")) == 0) {
		cur_entry->atime.Replace(_T('-'), _T('/'));
		m_whichtime = -1;
	}

	if (_tcscmp(szCurElement, _T("ltime")) == 0) {
		cur_entry->ltime.Replace(_T('-'), _T('/'));
		m_whichtime = -1;
	}

	if (_tcscmp(szCurElement, _T("rmtime")) == 0) {
		cur_entry->rmtime.Replace(_T('-'), _T('/'));
		m_whichtime = -1;
	}

	if (_tcscmp(szCurElement, _T("changed")) == 0) {
		cur_entry->changed.Replace(_T('-'), _T('/'));
		m_whichtime = -1;
	}

	if (_tcscmp(szCurElement, _T("oldpassword")) == 0) {
		cur_entry->changed.TrimLeft();
		cur_entry->changed.TrimRight();
		if (cur_entry->changed.IsEmpty()) {
			//                       1234567890123456789
			cur_entry->changed = _T("1970-01-01 00:00:00");
		}
		cur_entry->pwhistory += _T(" ") + cur_entry->changed;
		//cur_entry->changed.Empty();
		CString buffer;
		buffer.Format(_T(" %04x %s"), m_strElemContent.GetLength(), m_strElemContent);
		cur_entry->pwhistory += CMyString(buffer);
		buffer.Empty();
	}

	if (_tcscmp(szCurElement, _T("date")) == 0 && !m_strElemContent.IsEmpty()) {
		switch (m_whichtime) {
    case PW_CTIME:
      cur_entry->ctime = m_strElemContent;
      break;
    case PW_PMTIME:
      cur_entry->pmtime = m_strElemContent;
      break;
    case PW_ATIME:
      cur_entry->atime = m_strElemContent;
      break;
    case PW_LTIME:
      cur_entry->ltime = m_strElemContent;
      break;
    case PW_RMTIME:
      cur_entry->rmtime = m_strElemContent;
      break;
    case PW_CHANGED:
      cur_entry->changed = m_strElemContent;
      break;
    default:
      ASSERT(0);
		}
	}

	if (_tcscmp(szCurElement, _T("time")) == 0 && !m_strElemContent.IsEmpty()) {
		switch (m_whichtime) {
    case PW_CTIME:
      cur_entry->ctime += _T(" ") + m_strElemContent;
      break;
    case PW_PMTIME:
      cur_entry->pmtime += _T(" ") + m_strElemContent;
      break;
    case PW_ATIME:
      cur_entry->atime += _T(" ") + m_strElemContent;
      break;
    case PW_LTIME:
      cur_entry->ltime += _T(" ") + m_strElemContent;
      break;
    case PW_RMTIME:
      cur_entry->rmtime += _T(" ") + m_strElemContent;
      break;
    case PW_CHANGED:
      cur_entry->changed += _T(" ") + m_strElemContent;
      break;
    default:
      ASSERT(0);
		}
	}

  if (_tcscmp(szCurElement, _T("unknownheaderfields")) == 0)
    m_bheader = false;

  if (_tcscmp(szCurElement, _T("unknownrecordfields")) == 0) {
    if (!cur_entry->uhrxl.empty())
      m_nRecordsWithUnknownFields++;
  }

  if (_tcscmp(szCurElement, _T("field")) == 0) {
    // _stscanf_s always outputs to an "int" using %x even though
    // target is only 1.  Read into larger buffer to prevent data being
    // overwritten and then copy to where we want it!
    const int length = m_strElemContent.GetLength();
    // UNK_HEX_REP will represent unknown values
    // as hexadecimal, rather than base64 encoding.
    // Easier to debug.
#ifndef UNK_HEX_REP
    m_pfield = new unsigned char[(length / 3) * 4 + 4];
    size_t out_len;
    PWSUtil::Base64Decode(m_strElemContent, m_pfield, out_len);
    m_fieldlen = (int)out_len;
#else
    m_fieldlen = length / 2;
    m_pfield = new unsigned char[m_fieldlen + sizeof(int)];
    int nscanned = 0;
    TCHAR *lpsz_string = m_strElemContent.GetBuffer(length);
    for (int i = 0; i < m_fieldlen; i++) {
#if _MSC_VER >= 1400
      nscanned += _stscanf_s(lpsz_string, _T("%02x"), &m_pfield[i]);
#else
      nscanned += _stscanf(lpsz_string, _T("%02x"), &m_pfield[i]);
#endif
      lpsz_string += 2;
    }
    m_strElemContent.ReleaseBuffer();
#endif
    // We will use header field entry and add into proper record field
    // when we create the complete record entry
    UnknownFieldEntry ukxfe(m_ctype, m_fieldlen, m_pfield);
    if (m_bheader) {
      if (m_ctype >= PWSfileV3::HDR_LAST) {
        m_ukhxl.push_back(ukxfe);
#ifdef _DEBUG
        CString cs_timestamp;
        cs_timestamp = PWSUtil::GetTimeStamp();
        TRACE(_T("%s: Header has unknown field: %02x, length %d/0x%04x, value:\n"),
          cs_timestamp, m_ctype, m_fieldlen, m_fieldlen);
        PWSUtil::HexDump(m_pfield, m_fieldlen, cs_timestamp);
#endif
      } else {
        m_bDatabaseHeaderErrors = true;
      }
    } else {
      if (m_ctype >= CItemData::LAST) {
        cur_entry->uhrxl.push_back(ukxfe);
      } else {
        m_bRecordHeaderErrors = true;
      }
    }
    trashMemory(m_pfield, m_fieldlen);
    delete[] m_pfield;
    m_pfield = NULL;
  }

	if (_tcscmp(szCurElement, _T("NumberHashIterations")) == 0) { 
	  int i = _ttoi(m_strElemContent);
    if (i > MIN_HASH_ITERATIONS) {
		  m_nITER = i;
		}
  }

  if (_tcscmp(szCurElement, _T("DisplayExpandedAddEditDlg")) == 0)
    m_bDisplayExpandedAddEditDlg = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("MaintainDateTimeStamps")) == 0)
    m_bMaintainDateTimeStamps = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("PWUseDigits")) == 0)
    m_bPWUseDigits = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("PWUseEasyVision")) == 0)
    m_bPWUseEasyVision = _ttoi(m_strElemContent);
  
  if (_tcscmp(szCurElement, _T("PWUseHexDigits")) == 0)
    m_bPWUseHexDigits = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("PWUseLowercase")) == 0)
    m_bPWUseLowercase = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("PWUseSymbols")) == 0)
    m_bPWUseSymbols = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("PWUseUppercase")) == 0)
    m_bPWUseUppercase = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("SaveImmediately")) == 0)
    m_bSaveImmediately = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("SavePasswordHistory")) == 0)
    m_bSavePasswordHistory = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("ShowNotesDefault")) == 0)
    m_bShowNotesDefault = _ttoi(m_strElemContent);
  
  if (_tcscmp(szCurElement, _T("ShowPWDefault")) == 0)
    m_bShowPWDefault = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("ShowPasswordInTree")) == 0)
    m_bShowPasswordInTree = _ttoi(m_strElemContent);
  
  if (_tcscmp(szCurElement, _T("ShowUsernameInTree")) == 0)
    m_bShowUsernameInTree = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("SortAscending")) == 0)
    m_bSortAscending = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("UseDefaultUser")) == 0)
    m_bUseDefaultUser = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("PWLengthDefault")) == 0)
    m_iPWDefaultLength = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("IdleTimeout")) == 0)
    m_iIdleTimeout = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("TreeDisplayStatusAtOpen")) == 0) {
    if (m_strElemContent == _T("AllCollapsed"))
      m_iTreeDisplayStatusAtOpen = PWSprefs::AllCollapsed;
    else if (m_strElemContent == _T("AllExpanded"))
      m_iTreeDisplayStatusAtOpen = PWSprefs::AllExpanded;
    else if (m_strElemContent == _T("AsPerLastSave"))
      m_iTreeDisplayStatusAtOpen = PWSprefs::AsPerLastSave;
  }

  if (_tcscmp(szCurElement, _T("NumPWHistoryDefault")) == 0)
    m_iNumPWHistoryDefault = _ttoi(m_strElemContent);

  if (_tcscmp(szCurElement, _T("DefaultUsername")) == 0)
    m_sDefaultUsername = CString(m_strElemContent);

  if (_tcscmp(szCurElement, _T("DefaultAutotypeString")) == 0)
    m_sDefaultAutotypeString = CString(m_strElemContent);

	return S_OK;
}

//	---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::endDocument ( )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::startPrefixMapping (
    unsigned short * pwchPrefix,
    int cchPrefix,
    unsigned short * pwchUri,
    int cchUri )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::endPrefixMapping (
    unsigned short * pwchPrefix,
    int cchPrefix )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::ignorableWhitespace (
    unsigned short * pwchChars,
    int cchChars )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::processingInstruction (
    unsigned short * pwchTarget,
    int cchTarget,
    unsigned short * pwchData,
    int cchData )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  PWSSAXContentHandler::skippedEntity (
    unsigned short * pwchName,
    int cchName )
{
	return S_OK;
}
