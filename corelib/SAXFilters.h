/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __SAXFILTERS_H
#define __SAXFILTERS_H
// SAXFilters.h : header file
//

#include "xml_import.h"
#include "PWSFilters.h"

class PWSFilters;
using namespace MSXML2;

// Local variables

#define PWS_XML_FILTER_VERSION 1

enum {FI_NORMAL, FI_HISTORY, FI_POLICY};
//  -----------------------------------------------------------------------
class PWSSAXFilterErrorHandler: public ISAXErrorHandler
{
public:
  // Local variables and functions
  CString m_strValidationResult;
  BOOL bErrorsFound;

  // Standard functions
  PWSSAXFilterErrorHandler();
  virtual ~PWSSAXFilterErrorHandler();

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
class PWSSAXFilterContentHandler: public MSXML2::ISAXContentHandler
{
public:
  // Local variables & function
  CString m_strImportErrors;
  PWSFilters *m_MapFilters;
  FilterPool m_FPool;
  int m_type;

  // Standard functions
  PWSSAXFilterContentHandler();
  virtual ~PWSSAXFilterContentHandler();

  void SetVariables(PWSFilters *mapfilters, const FilterPool fpool, const bool &bValidation)
  {m_MapFilters = mapfilters, m_FPool = fpool; m_bValidation = bValidation;}
  void SetSchemaVersion(BSTR *schema_version)
  {m_pSchema_Version = schema_version;}

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
  st_filters *cur_filter;
  st_FilterRow *cur_filterentry;

  StringX m_strElemContent;
  bool m_bValidation;
  BSTR * m_pSchema_Version;

  unsigned char m_ctype;
  unsigned char * m_pfield;
  int m_fieldlen;
  int m_iXMLVersion, m_iSchemaVersion;
  bool m_bentrybeingprocessed;

  // REQUIRED variable
  ULONG m_refCnt;
};
#endif /*  __SAXFILTERS_H */
