/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __SAXHANDLERS_H
#define __SAXHANDLERS_H
// SAXHandlers.h : header file
//


#include "StringX.h"
#include "ItemData.h"
#include "UUIDGen.h"
#include "xml_import.h"

#include "UnknownField.h"

using namespace MSXML2;

// Local variables
enum {PASSWORDSAFE = 0, PW_ENTRY, PW_GROUP, PW_TITLE, PW_USERNAME, PW_PASSWORD, PW_URL,
      PW_AUTOTYPE, PW_NOTES, PW_CTIME, PW_ATIME, PW_XTIME, PW_PMTIME, PW_RMTIME,
      PW_HISTORY, PW_STATUS, PW_MAX, PW_NUM, PW_HISTORY_ENTRY,
      PW_CHANGED, PW_OLDPASSWORD, PW_DATE, PW_TIME, PW_UUID};

enum {NORMAL = 0, ALIAS, SHORTCUT};

struct pw_entry {
  StringX group;
  StringX title;
  StringX username;
  StringX password;
  StringX url;
  StringX autotype;
  StringX ctime;
  StringX atime;
  StringX xtime;
  StringX xtime_interval;
  StringX pmtime;
  StringX rmtime;
  StringX changed;
  StringX pwhistory;
  StringX notes;
  StringX uuid;
  PWPolicy pwp;
  UnknownFieldList uhrxl;  // Note: use header format for record unknown fields!
  int entrytype;
};

//  -----------------------------------------------------------------------
class PWSSAXErrorHandler: public ISAXErrorHandler
{
public:
  // Local variables and functions
  stringT m_strValidationResult;
  BOOL bErrorsFound;

  // Standard functions
  PWSSAXErrorHandler();
  virtual ~PWSSAXErrorHandler();

  virtual HRESULT STDMETHODCALLTYPE error (
  struct ISAXLocator * pLocator,
    unsigned short * pwchErrorMessage,
    HRESULT hrErrorCode );

  //  This must be correctly implemented, if your handler must be a COM Object
  //  the current implementation is NOT thread-safe
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

//  -----------------------------------------------------------------------
class PWSSAXContentHandler: public MSXML2::ISAXContentHandler
{
public:
  // Local variables & function
  stringT m_strImportErrors;
  int m_numEntries;
  TCHAR m_delimiter;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
  int m_nITER;
  UnknownFieldList m_ukhxl;  // For header unknown fields
  int m_nRecordsWithUnknownFields;

  void SetVariables(PWScore *core, const bool &bValidation,
    const stringT &ImportedPrefix, const TCHAR &delimiter,
    UUIDList *possible_aliases, UUIDList *possible_shortcuts);

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
  int m_bPWMakePronounceable;
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
  int m_iPWDigitMinLength;
  int m_iPWLowercaseMinLength;
  int m_iPWSymbolMinLength;
  int m_iPWUppercaseMinLength;
  stringT m_sDefaultAutotypeString;
  stringT m_sDefaultUsername;

  // Standard functions
  PWSSAXContentHandler();
  virtual ~PWSSAXContentHandler();

  //  This must be correctly implemented, if your handler must be a COM Object
  //  the current implementation is NOT thread-safe
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

  StringX m_strElemContent;
  stringT m_ImportedPrefix;
  PWScore *m_xmlcore;
  UUIDList *m_possible_aliases;
  UUIDList *m_possible_shortcuts;
  int m_whichtime, m_ipwh;
  bool m_bValidation;

  bool m_bheader;
  unsigned char m_ctype;
  unsigned char * m_pfield;
  int m_fieldlen;
  bool m_bentrybeingprocessed;

  // REQUIRED variable
  ULONG m_refCnt;
};
#endif /*  __SAXHANDLERS_H */
