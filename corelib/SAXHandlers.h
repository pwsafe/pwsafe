/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __SAXHANDLERS_H
#define __SAXHANDLERS_H
// SAXHandlers.h : header file
//


#include "MyString.h"
#include "ItemData.h"
#include "UUIDGen.h"
#include "xml_import.h"

#include "UnknownField.h"

using namespace MSXML2;

// Local variables
enum {PASSWORDSAFE = 0, PW_ENTRY, PW_GROUP, PW_TITLE, PW_USERNAME, PW_PASSWORD, PW_URL,
		PW_AUTOTYPE, PW_NOTES, PW_CTIME, PW_ATIME, PW_LTIME, PW_PMTIME, PW_RMTIME,
		PW_HISTORY, PW_STATUS, PW_MAX, PW_NUM, PW_HISTORY_ENTRY,
		PW_CHANGED, PW_OLDPASSWORD, PW_DATE, PW_TIME, PW_UUID};

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
	CMyString uuid;
	UnknownFieldList uhrxl;  // Note: use header format for record unknown fields!
	bool alias;
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
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
	int m_nITER;
	UnknownFieldList m_ukhxl;  // For header unknown fields
  int m_nRecordsWithUnknownFields;

	void SetVariables(PWScore *core, const bool &bValidation,
					const CString &ImportedPrefix, const TCHAR &delimiter,
          UUIDList *possible_aliases);

  // Preferences posibly stored in database
  // Note: boolean is integer to allow an 'not set' value of '-1'
  int m_bDisplayExpandedAddEditDlg;
  int m_bMaintainDateTimeStamps;
  int m_bPWUseDigits;
  int m_bPWUseEasyVision;
  int m_bPWUseHexDigits;
  int m_bPWUseLowercase;
  int m_bPWUseSymbols;
  int m_bPWUseUppercase;
  int m_bSaveImmediately;
  int m_bSavePasswordHistory;
  int m_bShowNotesDefault;
  int m_bShowPasswordInTree;
  int m_bShowPWDefault;
  int m_bShowUsernameInTree;
  int m_bSortAscending;
  int m_bUseDefaultUser;
  int m_iIdleTimeout;
  int m_iNumPWHistoryDefault;
  int m_iPWDefaultLength;
  int m_iTreeDisplayStatusAtOpen;
  CString m_sDefaultAutotypeString;
  CString m_sDefaultUsername;

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
  UUIDList *m_possible_aliases;
	int m_whichtime, m_ipwh;
	bool m_bValidation;

  bool m_bheader;
	unsigned char m_ctype;
	unsigned char * m_pfield;
  int m_fieldlen;

	// REQUIRED variable
	ULONG m_refCnt;
};
#endif /*  __SAXHANDLERS_H */
