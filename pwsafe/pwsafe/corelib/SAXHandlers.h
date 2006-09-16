// SAXHandlers.h : header file
//

#pragma once

#include "MyString.h"
#include "ItemData.h"
#include "xml_import.h"
using namespace MSXML2;

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
class PWSSAXErrorHandler: public ISAXErrorHandler
{
public:
	// Local variables and functions
	CString m_strValidationResult;
	BOOL bErrorsFound;

	// Standard functions
	PWSSAXErrorHandler();
	virtual ~PWSSAXErrorHandler();

	virtual HRESULT STDMETHODCALLTYPE error (
		struct ISAXLocator * pLocator,
		unsigned short * pwchErrorMessage,
		HRESULT hrErrorCode );

	//	This must be correctly implemented, if your handler must be a COM Object
	//	the current implementation is NOT thread-safe
    long __stdcall QueryInterface(const struct _GUID &,void ** );
    unsigned long __stdcall AddRef(void);
    unsigned long __stdcall Release(void);

    virtual HRESULT STDMETHODCALLTYPE fatalError (
        struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode );

    virtual HRESULT STDMETHODCALLTYPE ignorableWarning (
        struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode );

private:
	// REQUIRED variable
	ULONG m_refCnt;
};

//	-----------------------------------------------------------------------
class PWSSAXContentHandler: public MSXML2::ISAXContentHandler
{
public:
	// Local variables & function
	CString m_strImportErrors;
	int m_numEntries;
	TCHAR m_delimiter;
	void SetVariables(PWScore *core, const bool &bValidation,
					const CString &ImportedPrefix, const TCHAR &delimiter);

	// Standard functions
	PWSSAXContentHandler();
	virtual ~PWSSAXContentHandler();

	//	This must be correctly implemented, if your handler must be a COM Object
	//	the current implementation is NOT thread-safe
    long __stdcall QueryInterface(const struct _GUID &,void ** );
    unsigned long __stdcall AddRef(void);
    unsigned long __stdcall Release(void);

    virtual HRESULT STDMETHODCALLTYPE putDocumentLocator(
            /* [in] */ ISAXLocator __RPC_FAR *pLocator);

    virtual HRESULT STDMETHODCALLTYPE startDocument( void);

    virtual HRESULT STDMETHODCALLTYPE endDocument( void);

    virtual HRESULT STDMETHODCALLTYPE startPrefixMapping(
            /* [in] */ wchar_t __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix,
            /* [in] */ wchar_t __RPC_FAR *pwchUri,
            /* [in] */ int cchUri);

    virtual HRESULT STDMETHODCALLTYPE endPrefixMapping(
            /* [in] */ wchar_t __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix);

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

    virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace(
            /* [in] */ wchar_t __RPC_FAR *pwchChars,
            /* [in] */ int cchChars);

    virtual HRESULT STDMETHODCALLTYPE processingInstruction(
            /* [in] */ wchar_t __RPC_FAR *pwchTarget,
            /* [in] */ int cchTarget,
            /* [in] */ wchar_t __RPC_FAR *pwchData,
            /* [in] */ int cchData);

    virtual HRESULT STDMETHODCALLTYPE skippedEntity(
            /* [in] */ wchar_t __RPC_FAR *pwchName,
            /* [in] */ int cchName);

private:
	// Local variables
	pw_entry *cur_entry;

	CMyString m_strElemContent;
	CString m_ImportedPrefix;
	PWScore *m_xmlcore;
	int m_whichtime, m_ipwh;
	bool m_bValidation;

	// REQUIRED variable
	ULONG m_refCnt;
};