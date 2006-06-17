// SAXHandlers.h : header file
//

#pragma once

#include "SAXHandlersBase.h"
#include "MyString.h"
#include "ItemData.h"

// Local variables
enum {PASSWORDSAFE = 0, PW_ENTRY, PW_GROUP, PW_TITLE, PW_USERNAME, PW_PASSWORD, PW_URL,
		PW_AUTOTYPE, PW_NOTES, PW_CTIME, PW_ATIME, PW_LTIME, PW_PMTIME, PW_RMTIME,
		PW_HISTORY, PW_STATUS, PW_MAX, PW_NUM, PW_HISTORY_ENTRY,
		PW_CHANGED, PW_OLDPASSWORD, PW_DATE, PW_TIME};

struct pw_entry {
	CMyString group;
	CMyString title;
	CMyString username;
	CMyString password;
	CMyString url;
	CMyString autotype;
	CMyString ctime;
	CMyString atime;
	CMyString ltime;
	CMyString pmtime;
	CMyString rmtime;
	CMyString changed;
	CMyString pwhistory;
	CMyString notes;
};

//	-----------------------------------------------------------------------
class ErrorHandlerImpl: public ErrorHandlerImplBase
{
public:
	// Local variables and functions
	CString m_strValidationResult;
	BOOL bErrorsFound;

	// Standard functions
	ErrorHandlerImpl();
	virtual ~ErrorHandlerImpl();

	virtual HRESULT STDMETHODCALLTYPE error (
		struct ISAXLocator * pLocator,
		unsigned short * pwchErrorMessage,
		HRESULT hrErrorCode );
};

//	-----------------------------------------------------------------------
class ContentHandlerImpl: public ContentHandlerImplBase
{
public:
	// Local variables & function
	int m_numEntries;
	TCHAR m_delimiter;
	void SetVariables(void* core, const bool &bValidation,
					const CString &ImportedPrefix, const TCHAR &delimiter);

	// Standard functions
	ContentHandlerImpl();
	virtual ~ContentHandlerImpl();

	virtual HRESULT STDMETHODCALLTYPE startDocument(void);

	virtual HRESULT STDMETHODCALLTYPE startElement(
			/* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
			/* [in] */ int cchNamespaceUri,
			/* [in] */ wchar_t __RPC_FAR *pwchLocalName,
			/* [in] */ int cchLocalName,
			/* [in] */ wchar_t __RPC_FAR *pwchRawName,
			/* [in] */ int cchRawName,
			/* [in] */ ISAXAttributes __RPC_FAR *pAttributes);

	virtual HRESULT STDMETHODCALLTYPE endElement(
			/* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
			/* [in] */ int cchNamespaceUri,
			/* [in] */ wchar_t __RPC_FAR *pwchLocalName,
			/* [in] */ int cchLocalName,
			/* [in] */ wchar_t __RPC_FAR *pwchRawName,
			/* [in] */ int cchRawName);

	virtual HRESULT STDMETHODCALLTYPE characters(
			/* [in] */ wchar_t __RPC_FAR *pwchChars,
			/* [in] */ int cchChars);

	virtual HRESULT STDMETHODCALLTYPE endDocument(void);

private:
	// Local variables
	pw_entry *cur_entry;

	CItemData m_tempitem;

	CMyString m_strElemContent;
	CString m_ImportedPrefix;
	void* m_core;
	int m_whichtime, m_ipwh;
	bool m_bValidation;
};