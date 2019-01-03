/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MFILESAX2HANDLERS_H
#define __MFILESAX2HANDLERS_H

#include "../XMLFileValidation.h"
#include "../XMLFileHandlers.h"

#include "MFileValidator.h"

#include "../../StringX.h"
#include "../../UnknownField.h"

class PWScore;

// MSXML includes
#include <msxml6.h>

//  -----------------------------------------------------------------------
class MFileSAX2ErrorHandler: public ISAXErrorHandler
{
public:
  // Local variables and functions
  stringT m_strValidationResult;
  BOOL bErrorsFound;

  // Standard functions
  MFileSAX2ErrorHandler();
  virtual ~MFileSAX2ErrorHandler();

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
                  /* [in] */ HRESULT hrErrorCode);

private:
  // REQUIRED variable
  ULONG m_refCnt;
};

//  -----------------------------------------------------------------------
class MFileSAX2ContentHandler: public ISAXContentHandler, public XMLFileHandlers
{
public:
  // Standard functions
  MFileSAX2ContentHandler();
  virtual ~MFileSAX2ContentHandler();

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
  MFileValidator *m_pValidator;

  // REQUIRED variable
  ULONG m_refCnt;
};

#endif /*  __MFILESAX2HANDLERS_H */
