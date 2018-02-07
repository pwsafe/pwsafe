/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __MFILTERSAX2HANDLERS_H
#define __MFILTERSAX2HANDLERS_H

// MFilterSAX2Handlers.h : header file
//

#include "../../PWSFilters.h"

class PWSFilters;

// MSXML includes
#include <msxml6.h>

//  -----------------------------------------------------------------------
class MFilterSAX2ErrorHandler: public ISAXErrorHandler
{
public:
  // Local variables and functions
  stringT m_strValidationResult;
  BOOL bErrorsFound;

  // Standard functions
  MFilterSAX2ErrorHandler();
  virtual ~MFilterSAX2ErrorHandler();

  virtual HRESULT STDMETHODCALLTYPE error(
                  /* [in] */ struct ISAXLocator * pLocator,
                  /* [in] */ const wchar_t * pwchErrorMessage,
                  /* [in] */ HRESULT hrErrorCode);

  //  This must be correctly implemented, if your handler must be a COM Object
  //  the current implementation is NOT thread-safe
  long __stdcall QueryInterface(const struct _GUID &,void ** );
  unsigned long __stdcall AddRef(void);
  unsigned long __stdcall Release(void);

  virtual HRESULT STDMETHODCALLTYPE fatalError(
                  /* [in] */ struct ISAXLocator * pLocator,
                  /* [in] */ const wchar_t * pwchErrorMessage,
                  /* [in] */ HRESULT hrErrorCode);

  virtual HRESULT STDMETHODCALLTYPE ignorableWarning(
                  /* [in] */ struct ISAXLocator * pLocator,
                  /* [in] */ const wchar_t * pwchErrorMessage,
                  /* [in] */HRESULT hrErrorCode);

private:
  // REQUIRED variable
  ULONG m_refCnt;
};

//  -----------------------------------------------------------------------
class MFilterSAX2ContentHandler: public ISAXContentHandler
{
public:
  // Local variables & function
  stringT m_strXMLErrors;
  PWSFilters *m_MapXMLFilters;  // So as not to confuse with UI & core
  FilterPool m_FPool;
  int m_type;

  // Standard functions
  MFilterSAX2ContentHandler();
  virtual ~MFilterSAX2ContentHandler();

  void SetVariables(Asker *pAsker, PWSFilters *mapfilters, const FilterPool fpool, 
                    const bool &bValidation)
  {m_pAsker = pAsker; m_MapXMLFilters = mapfilters, m_FPool = fpool; m_bValidation = bValidation;}
  void SetSchemaVersion(BSTR *schema_version)
  {m_pSchema_Version = schema_version;}

  //  This must be correctly implemented, if your handler must be a COM Object
  //  the current implementation is NOT thread-safe
  long __stdcall QueryInterface(const struct _GUID &,void ** );
  unsigned long __stdcall AddRef(void);
  unsigned long __stdcall Release(void);

  virtual HRESULT STDMETHODCALLTYPE putDocumentLocator(
                  /* [in] */ ISAXLocator __RPC_FAR *pLocator);

  virtual HRESULT STDMETHODCALLTYPE startDocument(void);

  virtual HRESULT STDMETHODCALLTYPE endDocument(void);

  virtual HRESULT STDMETHODCALLTYPE startPrefixMapping(
                  /* [in] */ const wchar_t __RPC_FAR *pwchPrefix,
                  /* [in] */ int cchPrefix,
                  /* [in] */ const wchar_t __RPC_FAR *pwchUri,
                  /* [in] */ int cchUri);

  virtual HRESULT STDMETHODCALLTYPE endPrefixMapping(
                  /* [in] */ const wchar_t __RPC_FAR *pwchPrefix,
                  /* [in] */ int cchPrefix);

  virtual HRESULT STDMETHODCALLTYPE startElement(
                  /* [in] */ const wchar_t __RPC_FAR *pwchNamespaceUri,
                  /* [in] */ int cchNamespaceUri,
                  /* [in] */ const wchar_t __RPC_FAR *pwchLocalName,
                  /* [in] */ int cchLocalName,
                  /* [in] */ const wchar_t __RPC_FAR *pwchRawName,
                  /* [in] */ int cchRawName,
                  /* [in] */ ISAXAttributes __RPC_FAR *pAttributes);

  virtual HRESULT STDMETHODCALLTYPE endElement(
                  /* [in] */ const wchar_t __RPC_FAR *pwchNamespaceUri,
                  /* [in] */ int cchNamespaceUri,
                  /* [in] */ const wchar_t __RPC_FAR *pwchLocalName,
                  /* [in] */ int cchLocalName,
                  /* [in] */ const wchar_t __RPC_FAR *pwchRawName,
                  /* [in] */ int cchRawName);

  virtual HRESULT STDMETHODCALLTYPE characters(
                  /* [in] */ const wchar_t __RPC_FAR *pwchChars,
                  /* [in] */ int cchChars);

  virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace(
                  /* [in] */ const wchar_t __RPC_FAR *pwchChars,
                  /* [in] */ int cchChars);

  virtual HRESULT STDMETHODCALLTYPE processingInstruction(
                  /* [in] */ const wchar_t __RPC_FAR *pwchTarget,
                  /* [in] */ int cchTarget,
                  /* [in] */ const wchar_t __RPC_FAR *pwchData,
                  /* [in] */ int cchData);

  virtual HRESULT STDMETHODCALLTYPE skippedEntity(
                  /* [in] */ const wchar_t __RPC_FAR *pwchName,
                  /* [in] */ int cchName);

private:
  // Local variables
  st_filters *cur_filter;
  st_FilterRow *cur_filterentry;
  Asker *m_pAsker;

  StringX m_sxElemContent;
  BSTR * m_pSchema_Version;

  int m_fieldlen;
  int m_iXMLVersion, m_iSchemaVersion;
  bool m_bEntryBeingProcessed;
  bool m_bValidation;
  unsigned char m_ctype;
  unsigned char * m_pfield;

  // REQUIRED variable
  ULONG m_refCnt;
};

#endif /* __MFILTERSAX2HANDLERS_H */
