// SAXHandlersBase.h : header file
//

#pragma once

#include "xml_import.h"

//	-----------------------------------------------------------------------
class ErrorHandlerImplBase: public ISAXErrorHandler
{
public:
	ErrorHandlerImplBase();
	virtual ~ErrorHandlerImplBase();

	//	This must be correctly implemented, if your handler must be a COM Object
	//	the current implementation is NOT thread-safe
    long __stdcall QueryInterface(const struct _GUID &,void ** );
    unsigned long __stdcall AddRef(void);
    unsigned long __stdcall Release(void);

    virtual HRESULT STDMETHODCALLTYPE error (
        struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode );

    virtual HRESULT STDMETHODCALLTYPE fatalError (
        struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode );

    virtual HRESULT STDMETHODCALLTYPE ignorableWarning (
        struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode );

private:
	ULONG m_refCnt;

};

//	-----------------------------------------------------------------------
class ContentHandlerImplBase: public ISAXContentHandler
{
public:
	ContentHandlerImplBase();
	virtual ~ContentHandlerImplBase();

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
	ULONG m_refCnt;
};

