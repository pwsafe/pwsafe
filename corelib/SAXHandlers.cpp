// SAXHandlers.cpp : implementation file
//

#include "PWScore.h"
#include "ItemData.h"
#include "MyString.h"
#include "util.h"
#include "SAXHandlers.h"
#include "UUIDGen.h"
#include "xml_import.h"

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

	wcstombs(szErrorMessage, pwchErrorMessage, MAX_PATH);
	pLocator->getLineNumber(&iLineNumber);
	pLocator->getColumnNumber(&iCharacter);

	sprintf(szFormatString, "Error (%08X): line %d character %d %s",
		hrErrorCode, iLineNumber, iCharacter, szErrorMessage);

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
}

//	-----------------------------------------------------------------------
PWSSAXContentHandler::~PWSSAXContentHandler()
{
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
	wcstombs(szCurElement, pwchRawName, cchRawName);

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
				wcstombs(szQName, QName, QName_length);
				pAttributes->getValue(i, &Value, &Value_length);
				wcstombs(szValue, Value, Value_length);
				if (_tcscmp(szQName, _T("delimiter")) == 0)
					m_delimiter = szValue[0];
			}
		}
	}

	if (m_bValidation)
		return S_OK;

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
#if _MSC_VER >= 1400
	size_t num_converted;
	wcstombs_s(&num_converted, szData, cchChars+2, pwchChars, cchChars);
#else
	wcstombs(szData, pwchChars, cchChars);
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
    int cchQName )
{
	TCHAR szCurElement[MAX_PATH+1] = {0};
	wcstombs(szCurElement, pwchQName, cchQName);

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
#if _MSC_VER >= 1400
			int nscanned = sscanf_s(cur_entry->uuid, "%32x", uuid_array);
#else
			int nscanned = sscanf(cur_entry->uuid, "%32x", uuid_array);
#endif
			if (nscanned != 1)
				tempitem.CreateUUID();
			else
				tempitem.SetUUID(uuid_array);
		}
		CMyString newgroup(m_ImportedPrefix.IsEmpty() ? "" : m_ImportedPrefix + ".");
		tempitem.SetGroup(newgroup + cur_entry->group);
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
		buffer.Format(_T("\nError in Password History for entry: \xbb%s\xbb%s\xbb%s\xbb: "),
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

		m_xmlcore->AddEntryToTail(tempitem);
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
		int i = atoi(m_strElemContent);
		buffer.Format("%01x", i);
		cur_entry->pwhistory = CMyString(buffer);
	}

	if (_tcscmp(szCurElement, _T("max")) == 0) {
		CString buffer;
		int i = atoi(m_strElemContent);
		buffer.Format("%02x", i);
		cur_entry->pwhistory += CMyString(buffer);
	}

	if (_tcscmp(szCurElement, _T("num")) == 0) {
		CString buffer;
		int i = atoi(m_strElemContent);
		buffer.Format("%02x", i);
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
		buffer.Format(" %04x %s", m_strElemContent.GetLength(), m_strElemContent);
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
