// SAXHandlersBase.cpp : implementation file
//

#include "xml_import.h"
#include "SAXHandlersBase.h"

// Stop warnings about unused formal parameters!
#pragma warning(disable : 4100)

//	-------------------------------------------------------------------------
//	ErrorHandler methods
//	-------------------------------------------------------------------------
ErrorHandlerImplBase::ErrorHandlerImplBase()
{
	m_refCnt=0;
}

ErrorHandlerImplBase::~ErrorHandlerImplBase()
{
}

long __stdcall ErrorHandlerImplBase::QueryInterface(const struct _GUID &riid,void ** ppvObject)
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

unsigned long __stdcall ErrorHandlerImplBase::AddRef()
{
	 return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall ErrorHandlerImplBase::Release()
{
	--m_refCnt; // NOT thread-safe
   if (m_refCnt == 0) {
	  delete this;
	  return 0; // Can't return the member of a deleted object.
   }
   else return m_refCnt;
}

HRESULT STDMETHODCALLTYPE  ErrorHandlerImplBase::error (
		struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ErrorHandlerImplBase::fatalError (
		struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ErrorHandlerImplBase::ignorableWarning (
		struct ISAXLocator * pLocator,
        unsigned short * pwchErrorMessage,
        HRESULT hrErrorCode )
{
	return S_OK;
}

//	-------------------------------------------------------------------------
//	ContentHandler methods
//	-------------------------------------------------------------------------
ContentHandlerImplBase::ContentHandlerImplBase()
{
	m_refCnt=0;
}

ContentHandlerImplBase::~ContentHandlerImplBase()
{
}

long __stdcall ContentHandlerImplBase::QueryInterface(const struct _GUID &riid,void ** ppvObject)
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

unsigned long __stdcall ContentHandlerImplBase::AddRef()
{
	 return ++m_refCnt; // NOT thread-safe
}

unsigned long __stdcall ContentHandlerImplBase::Release()
{
	--m_refCnt; // NOT thread-safe
   if (m_refCnt == 0) {
	  delete this;
	  return 0; // Can't return the member of a deleted object.
   }
   else return m_refCnt;
}


HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::startDocument ( )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::endElement (
    unsigned short * pwchNamespaceUri,
    int cchNamespaceUri,
    unsigned short * pwchLocalName,
    int cchLocalName,
    unsigned short * pwchQName,
    int cchQName )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::putDocumentLocator (struct ISAXLocator * pLocator )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::endDocument ( )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::startPrefixMapping (
    unsigned short * pwchPrefix,
    int cchPrefix,
    unsigned short * pwchUri,
    int cchUri )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::endPrefixMapping (
    unsigned short * pwchPrefix,
    int cchPrefix )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::startElement (
    unsigned short * pwchNamespaceUri,
    int cchNamespaceUri,
    unsigned short * pwchLocalName,
    int cchLocalName,
    unsigned short * pwchQName,
    int cchQName,
    struct ISAXAttributes * pAttributes )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::characters (
    unsigned short * pwchChars,
    int cchChars )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::ignorableWhitespace (
    unsigned short * pwchChars,
    int cchChars )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::processingInstruction (
    unsigned short * pwchTarget,
    int cchTarget,
    unsigned short * pwchData,
    int cchData )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE  ContentHandlerImplBase::skippedEntity (
    unsigned short * pwchName,
    int cchName )
{
	return S_OK;
}
