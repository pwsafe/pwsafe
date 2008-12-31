/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of http.h
 */
#include <afx.h>
#include <Windows.h>
#include <atlhttp.h>
#include <string>
#include "../http.h"

#include <winsock2.h>
#include <svcguid.h>
#include <atlcoll.h>


#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>

#define SECURITY_WIN32
#include <security.h>
#include <sspi.h>


class CSecureEvtSyncSocket
{
public:
	CSecureEvtSyncSocket() throw();
	~CSecureEvtSyncSocket() { Close(); }
	operator SOCKET() { return m_socket; }
	void Close() throw();
	void Term() throw();
	bool Create(const addrinfo* pAI, WORD wFlags=0) throw();
	bool Create(short af, short st, short proto, WORD wFlags=0) throw();
	bool Connect(LPCTSTR szAddr, unsigned short nPort) throw();
	bool Connect(const addrinfo *pAI)throw();
	bool Connect(const SOCKADDR* psa,int len) throw();
	bool Write(const unsigned char *pBuffIn, DWORD *pdwSize) throw();
	bool Write(WSABUF *pBuffers, int nCount, DWORD *pdwSize) throw();
	bool Read(const unsigned char *pBuff, DWORD *pdwSize) throw();
	bool Init(SOCKET hSocket, void * /*pData=NULL*/) throw();
  DWORD SetSocketTimeout(DWORD dwNewTimeout) {
    DWORD dwOldTimeout = m_dwSocketTimeout;
    m_dwSocketTimeout = dwNewTimeout;
    return dwOldTimeout;
  }

	bool	SupportsScheme(ATL_URL_SCHEME	scheme) const
	{ 
		return ((scheme == ATL_URL_SCHEME_HTTP) ||
            (scheme == ATL_URL_SCHEME_HTTPS));
	}
  

protected:
	DWORD m_dwCreateFlags;
	WSAEVENT m_hEventRead;
	WSAEVENT m_hEventWrite;
	WSAEVENT m_hEventConnect;

	CComAutoCriticalSection m_csRead;
	CComAutoCriticalSection m_csWrite;
	SOCKET m_socket;
	bool m_bConnected;
	DWORD m_dwLastError;
	DWORD m_dwSocketTimeout;

	// overrides for HTTPS
	bool internalWrite(WSABUF *pBuffers, int nCount, DWORD *pdwSize) throw();
	bool internalRead(const unsigned char *pBuff, DWORD *pdwSize) throw();


protected:
	HCERTSTORE      m_hMyCertStore;
	SCHANNEL_CRED   m_SchannelCred;
	CredHandle		m_hClientCreds;
  bool m_hasClientCreds;
	CtxtHandle		m_hContext;
  bool m_hasSecurityContext;
	CString		m_strServerName;

	bool createCredentials();
	void getNewClientCredentials();
	void freeCredentials();
	bool performClientHandshake(SOCKET Socket, LPCTSTR pszServerName,
                              SecBuffer *pExtraData);

	bool clientHandshakeLoop(SOCKET Socket, BOOL fDoInitialRead,
                           SecBuffer *pExtraData);
	bool disconnectFromSecureServer();
  bool verifyServerCertificate(LPCTSTR pszServerName, DWORD dwCertFlags);

private:
	// HTTPS data
	LPCTSTR m_pszUserName;
	DWORD m_dwProtocol;
	ALG_ID m_aiKeyExch;
	bool m_bHttpsCommunication;

  static bool _securityInitialized;
	static SecurityFunctionTable	m_SecurityFunc;
	
	bool InitSecurityFunctionTable() {
		if (!_securityInitialized) {
			PSecurityFunctionTable pSecurityFunc;

			pSecurityFunc = InitSecurityInterface();

			if (pSecurityFunc != NULL) {
        memcpy_s(&m_SecurityFunc, sizeof(m_SecurityFunc),
                 pSecurityFunc, sizeof(m_SecurityFunc));
				_securityInitialized	=	true;
			}
		}
		return _securityInitialized;
	}

	static CString VerifyTrustErrorString(DWORD Status)
	{
		CString retval;

		switch (Status) {
    case CERT_E_EXPIRED:                retval = "Certificate expired"; break;
    case CERT_E_VALIDITYPERIODNESTING:  retval = "Certificate validity period nesting"; break;
    case CERT_E_ROLE:                   retval = "Certificate role error"; break;
    case CERT_E_PATHLENCONST:           retval = "Certificate path length constant"; break;
    case CERT_E_CRITICAL:               retval = "Certificate critical error"; break;
    case CERT_E_PURPOSE:                retval = "Certificate purpose error"; break;
    case CERT_E_ISSUERCHAINING:         retval = "Certificate issuer chaining error"; break;
    case CERT_E_MALFORMED:              retval = "Certificate malformed"; break;
    case CERT_E_UNTRUSTEDROOT:          retval = "Certificate untrusted root"; break;
    case CERT_E_CHAINING:               retval = "Certificate chaining error"; break;
    case TRUST_E_FAIL:                  retval = "Certificate trust failure"; break;
    case CERT_E_REVOKED:                retval = "Certificate revoked"; break;
    case CERT_E_UNTRUSTEDTESTROOT:      retval = "Certificate untrusted test root"; break;
    case CERT_E_REVOCATION_FAILURE:     retval = "Certificate revocation failure"; break;
    case CERT_E_CN_NO_MATCH:            retval = "Certificate CN no match"; break;
    case CERT_E_WRONG_USAGE:            retval = "Certificate wrong usage error"; break;
    default:                            retval = "Unknown certificate error"; break;
    }
    return retval;
	}
};

SecurityFunctionTable	CSecureEvtSyncSocket::m_SecurityFunc;
bool CSecureEvtSyncSocket::_securityInitialized = false;

/**
 * Simple http client wrapper implementation
 */

// XXX TBD - support possible proxy definitions
// ?? How to get system defaults ??
// m_client->SetProxy( m_proxy, m_proxy_port );
namespace pws_os {
  class HttpClientImpl
  {
  public:
    bool Navigate(const stringT &urlStr)
    { return client.Navigate(urlStr.c_str(), &navData); }

    // CAtlHttpClient client; for http, following for https
    CAtlHttpClientT<CSecureEvtSyncSocket> client;
    CAtlNavigateData navData;
  };

  HttpClient::HttpClient()
  {
    impl = new HttpClientImpl;
  }

  HttpClient::~HttpClient()
  {
    delete impl;
  }

  bool HttpClient::Navigate(const stringT &urlStr)
  {
    return impl->Navigate(urlStr);
  }
  
  std::string HttpClient::GetBody()
  {
    int bodyLen = impl->client.GetBodyLength();
    char *body = new char[bodyLen+1];
    memcpy( body, impl->client.GetBody(), bodyLen );
    body[bodyLen] = 0;

    std::string text(body);
    delete[] body;
    return text;
  }

  int HttpClient::GetStatus()
  {
    return impl->client.GetStatus();
  }
};

//-----------------------------------------------------------------
// Implementation of SSL support
//-----------------------------------------------------------------

#define HTTPS_IO_BUFFER_SIZE  0x10000

CSecureEvtSyncSocket::CSecureEvtSyncSocket()
  : m_dwCreateFlags(WSA_FLAG_OVERLAPPED), m_hEventRead(NULL),
    m_hEventWrite(NULL), m_hEventConnect(NULL),
    m_socket(INVALID_SOCKET), m_bConnected(false), m_dwLastError(0),
    m_dwSocketTimeout(ATL_SOCK_TIMEOUT),

    // HTTPS initialization
    m_pszUserName(NULL), m_dwProtocol(SP_PROT_NONE),
    m_aiKeyExch(0), m_hMyCertStore(NULL), m_hasSecurityContext(false),
    m_hasClientCreds(false), m_bHttpsCommunication(false)
{
	if (!g_HttpInit.Init()) {
    TRACE(_T("Failed to initialize global HttpInit object"));
  }

	if (!InitSecurityInterface()) {
    TRACE(_T("Failed to initialize security interface"));
  }
}

void CSecureEvtSyncSocket::Close()
{
	if (m_socket != INVALID_SOCKET) {
		if (m_bHttpsCommunication)
			disconnectFromSecureServer();
		m_bConnected = false;
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		Term();
	}
	if (m_bHttpsCommunication)
		freeCredentials();
}

void CSecureEvtSyncSocket::Term() 
{
	if (m_hEventRead) {
		WSACloseEvent(m_hEventRead);
		m_hEventRead = NULL;
	}
	if (m_hEventWrite) {
		WSACloseEvent(m_hEventWrite);
		m_hEventWrite = NULL;
	}
	if (m_hEventConnect) {
		WSACloseEvent(m_hEventConnect);
		m_hEventConnect = NULL;
	}
	m_socket = INVALID_SOCKET;
}

bool CSecureEvtSyncSocket::Create(const addrinfo* pAI, WORD wFlags)
{
	//return Create(PF_INET, SOCK_STREAM, IPPROTO_TCP, wFlags);
	return Create(short(pAI->ai_family), short(pAI->ai_socktype),
                short(pAI->ai_protocol), wFlags);
}

bool CSecureEvtSyncSocket::Create(short af, short st, short proto, WORD wFlags) 
{
	if (m_socket != INVALID_SOCKET) {
		m_dwLastError = WSAEALREADY;
		return false; // Must close this socket first
	}

	m_socket = WSASocket(af, st, proto, NULL, 0, wFlags | m_dwCreateFlags);
	if (m_socket == INVALID_SOCKET) {
		m_dwLastError = ::WSAGetLastError();
    return false;
	}
  return Init(m_socket, NULL);
}

bool CSecureEvtSyncSocket::Connect(LPCTSTR szAddr, unsigned short nPort) 
{
	if (m_bConnected)
		return true;

	m_bHttpsCommunication	=	(nPort	==	ATL_URL_DEFAULT_HTTPS_PORT);

	m_strServerName	=	szAddr;
	CSocketAddr address;
	if (SOCKET_ERROR == address.FindAddr(szAddr,nPort,
                                       AI_CANONNAME,PF_UNSPEC,SOCK_STREAM,IPPROTO_IP)) {
		m_dwLastError = WSAGetLastError();
    return false;
	}
  return Connect(address.GetAddrInfo());
}

bool CSecureEvtSyncSocket::Connect(const addrinfo *pAI)
{
	if (m_socket == INVALID_SOCKET && !Create(pAI))
		return false;

	return Connect((SOCKADDR*)pAI->ai_addr, (int)pAI->ai_addrlen);
}

// HTTPS modified
bool CSecureEvtSyncSocket::Connect(const SOCKADDR* psa, int len)
{
	if (m_bConnected)
		return true;

	bool retval = true;

	if (m_bHttpsCommunication)
		retval	=	createCredentials();

	if (!retval)
		return false;

	// if you try to connect the socket without
	// creating it first it's unreasonable to automatically
	// try the create for you.
	if (m_socket == INVALID_SOCKET) {
    if (m_bHttpsCommunication)
      freeCredentials();
    return false;
  }

	if (WSAConnect(m_socket, psa, len, NULL, NULL, NULL, NULL)) {
    DWORD dwLastError = WSAGetLastError();
    if (dwLastError != WSAEWOULDBLOCK) {
      m_dwLastError = dwLastError;
      retval = false;
    } else {
      dwLastError = WaitForSingleObject((HANDLE)m_hEventConnect, 10000);
      if (dwLastError == WAIT_OBJECT_0) {
        // make sure there were no connection errors.
        WSANETWORKEVENTS wse;
        ZeroMemory(&wse, sizeof(wse));
        WSAEnumNetworkEvents(m_socket, NULL, &wse);
        if (wse.iErrorCode[FD_CONNECT_BIT]!=0) {
          m_dwLastError = (DWORD)(wse.iErrorCode[FD_CONNECT_BIT]);
          if (m_bHttpsCommunication)
            freeCredentials();
          return false;
        }
      }
    }
  }

	m_bConnected = retval;
	if (!m_bConnected && m_bHttpsCommunication)
		freeCredentials();
	else if (m_bHttpsCommunication) {
    SecBuffer ExtraData;
    bool bOK =	performClientHandshake( m_socket, m_strServerName, &ExtraData);
		
    if (bOK)
      bOK	=	verifyServerCertificate(m_strServerName, 0);

    if (!bOK) {
      Close();
      m_bConnected	=	false;
      retval =	false;
    }
  }
	return retval;
}

bool CSecureEvtSyncSocket::internalWrite(WSABUF *pBuffers, int nCount, DWORD *pdwSize) 
{
	// if we aren't already connected we'll wait to see if the connect
	// event happens
	if (WAIT_OBJECT_0 != WaitForSingleObject((HANDLE)m_hEventConnect , m_dwSocketTimeout)) {
		m_dwLastError = WSAENOTCONN;
		return false; // not connected
	}

	// make sure we aren't already writing
	if (WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_hEventWrite, 0)) {
		m_dwLastError = WSAEINPROGRESS;
		return false; // another write on is blocking this socket
	}

	bool retval = true;
	*pdwSize = 0;
	WSAOVERLAPPED o;
	m_csWrite.Lock();
	o.hEvent = m_hEventWrite;
	WSAResetEvent(o.hEvent);
	if (WSASend(m_socket, pBuffers, nCount, pdwSize, 0, &o, 0)) {	
		DWORD dwLastError = WSAGetLastError();
		if (dwLastError != WSA_IO_PENDING) {
			m_dwLastError = dwLastError;
			retval = false;
		}
	}
	
	// wait for write to complete
	if (retval &&
      WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)m_hEventWrite, m_dwSocketTimeout)) {
		DWORD dwFlags = 0;
		if (WSAGetOverlappedResult(m_socket, &o, pdwSize, FALSE, &dwFlags))
			retval = true;
		else {
      m_dwLastError = ::GetLastError();
      retval = false;
    }
	}
	
	m_csWrite.Unlock();
	return retval;
}

bool CSecureEvtSyncSocket::Write(const unsigned char *pBuffIn, DWORD *pdwSize) 
{
	WSABUF buff;
	buff.buf = (char *)pBuffIn;
	buff.len = *pdwSize;
	return Write(&buff, 1, pdwSize);
}

bool CSecureEvtSyncSocket::internalRead(const unsigned char *pBuff, DWORD *pdwSize) 
{
	// if we aren't already connected we'll wait to see if the connect
	// event happens
	if (WAIT_OBJECT_0 != WaitForSingleObject((HANDLE)m_hEventConnect, m_dwSocketTimeout)) {
		m_dwLastError = WSAENOTCONN;
		return false; // not connected
	}

	if (WAIT_ABANDONED == WaitForSingleObject((HANDLE)m_hEventRead, 0)) {
    m_dwLastError = WSAEINPROGRESS;
    return false; // another write on is blocking this socket
  }

	bool retval = true;
	WSABUF buff;
	buff.buf = (char *)pBuff;
	buff.len = *pdwSize;
	*pdwSize = 0;
	DWORD dwFlags = 0;
	WSAOVERLAPPED o;
	ZeroMemory(&o, sizeof(o));

	// protect against re-entrency
	m_csRead.Lock();
	o.hEvent = m_hEventRead;
	WSAResetEvent(o.hEvent);
	if (WSARecv(m_socket, &buff, 1, pdwSize, &dwFlags, &o, 0)) {
    DWORD dwLastError = WSAGetLastError();
    if (dwLastError != WSA_IO_PENDING) {
      m_dwLastError = dwLastError;
      retval = false;
    }
  }

	// wait for the read to complete
	if (retval && WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)o.hEvent, m_dwSocketTimeout)) {
    dwFlags = 0;
    if (WSAGetOverlappedResult(m_socket, &o, pdwSize, FALSE, &dwFlags))
      retval = true;
    else {
      m_dwLastError = ::GetLastError();
      retval = false;
    }
  }

	m_csRead.Unlock();
	return retval;
}

bool CSecureEvtSyncSocket::Init(SOCKET hSocket, void * /*pData=NULL*/) 
{
	ASSERT(hSocket != INVALID_SOCKET);

	if (hSocket == INVALID_SOCKET) {
		m_dwLastError = WSAENOTSOCK;
		return false;
	}

	m_socket = hSocket;
	
	// Allocate Events. On error, any open event handles will be closed
	// in the destructor
	if ((NULL != (m_hEventRead = WSACreateEvent())) &&
      (NULL != (m_hEventWrite = WSACreateEvent())) &&
			(NULL != (m_hEventConnect = WSACreateEvent()))) {
    if (!WSASetEvent(m_hEventWrite) || !WSASetEvent(m_hEventRead)) {
      goto failed;
    }
    if ((SOCKET_ERROR != WSAEventSelect(m_socket, m_hEventRead, FD_READ)) &&
        (SOCKET_ERROR != WSAEventSelect(m_socket, m_hEventWrite, FD_WRITE)) &&
        (SOCKET_ERROR != WSAEventSelect(m_socket, m_hEventConnect, FD_CONNECT)))
          return true;
  }
 failed:
	m_dwLastError = ::GetLastError();
	return false;
}

// HTTPS stuff
bool CSecureEvtSyncSocket::createCredentials()
{
  DWORD           cSupportedAlgs = 0;
  ALG_ID          rgbSupportedAlgs[16];

  // Open the "MY" certificate store, which is where Internet Explorer
  // stores its client certificates.
  if (m_hMyCertStore == NULL) {
    m_hMyCertStore = CertOpenSystemStore(0, _T("MY"));

    if (!m_hMyCertStore) {
			TRACE("CSecureEvtSyncSocket::createCredentials 1 - SEC_E_NO_CREDENTIALS");
			return false;
    }
  }

  //
  // If a user name is specified, then attempt to find a client
  // certificate. Otherwise, just create a NULL credential.
  //
  PCCERT_CONTEXT pCertContext = NULL;
  if (m_pszUserName) {
    // Find client certificate. Note that we search for a 
    // certificate with a subject that matches the user name.

    pCertContext = CertFindCertificateInStore(m_hMyCertStore, 
                                              X509_ASN_ENCODING, 
                                              0,
                                              CERT_FIND_SUBJECT_STR_A,
                                              m_pszUserName,
                                              NULL);
    if (pCertContext == NULL) {
      TRACE("CSecureEvtSyncSocket::createCredentials 2 - SEC_E_NO_CREDENTIALS");
      return false;
    }
  }

  // Build Schannel credential structure. 
    
  ZeroMemory(&m_SchannelCred, sizeof(m_SchannelCred));

  m_SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
  if (pCertContext) {
    m_SchannelCred.cCreds     = 1;
    m_SchannelCred.paCred     = &pCertContext;
  }

  m_SchannelCred.grbitEnabledProtocols = m_dwProtocol;

  if (m_aiKeyExch) {
    rgbSupportedAlgs[cSupportedAlgs++] = m_aiKeyExch;
  }

  if (cSupportedAlgs) {
    m_SchannelCred.cSupportedAlgs    = cSupportedAlgs;
    m_SchannelCred.palgSupportedAlgs = rgbSupportedAlgs;
  }

  m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
  m_SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;

  //
  // Create an SSPI credential.
  //

  TimeStamp tsExpiry;
  SECURITY_STATUS Status = m_SecurityFunc.AcquireCredentialsHandle(NULL,
                                                                   UNISP_NAME,
                                                                   SECPKG_CRED_OUTBOUND,
                                                                   NULL,
                                                                   &m_SchannelCred,
                                                                   NULL,
                                                                   NULL,
                                                                   &m_hClientCreds,
                                                                   &tsExpiry);
  if (Status != SEC_E_OK) {
    TRACE("**** Error 0x%x returned by AcquireCredentialsHandle\n", Status);
    return false;
  }
  m_hasClientCreds = true;

  //
  // Free the certificate context. Schannel has already made its own copy.
  //
  if (pCertContext) {
    CertFreeCertificateContext(pCertContext);
  }
  return true;
}

void CSecureEvtSyncSocket::freeCredentials()
{
  // Free security context.
  if (m_hasSecurityContext) {
    m_SecurityFunc.DeleteSecurityContext(&m_hContext);
    m_hasSecurityContext = false;
  }

  // Free SSPI credentials handle.
  if (m_hasClientCreds) {
    m_SecurityFunc.FreeCredentialsHandle(&m_hClientCreds);
    m_hasClientCreds = false;
  }

  // Close "MY" certificate store.
  if (m_hMyCertStore != NULL) {
    CertCloseStore(m_hMyCertStore, 0);
    m_hMyCertStore = NULL;
  }
}



bool CSecureEvtSyncSocket::performClientHandshake(SOCKET Socket,
                                                  LPCTSTR pszServerName,
                                                  SecBuffer *pExtraData)
{
  SecBufferDesc   OutBuffer;
  SecBuffer       OutBuffers[1];
  DWORD dwSSPIFlags = (ISC_REQ_SEQUENCE_DETECT |
                       ISC_REQ_REPLAY_DETECT |
                       ISC_REQ_CONFIDENTIALITY |
                       ISC_RET_EXTENDED_ERROR |
                       ISC_REQ_ALLOCATE_MEMORY |
                       ISC_REQ_STREAM);

  //
  //  Initiate a ClientHello message and generate a token.
  //

  OutBuffers[0].pvBuffer   = NULL;
  OutBuffers[0].BufferType = SECBUFFER_TOKEN;
  OutBuffers[0].cbBuffer   = 0;

  OutBuffer.cBuffers = 1;
  OutBuffer.pBuffers = OutBuffers;
  OutBuffer.ulVersion = SECBUFFER_VERSION;

  DWORD dwSSPIOutFlags;
  TimeStamp tsExpiry;
  SECURITY_STATUS scRet = m_SecurityFunc.InitializeSecurityContext(&m_hClientCreds,
                                                                   0,
                                                                   (SEC_WCHAR *)pszServerName,
                                                                   dwSSPIFlags,
                                                                   0,
                                                                   SECURITY_NATIVE_DREP,
                                                                   NULL,
                                                                   0,
                                                                   &m_hContext,
                                                                   &OutBuffer,
                                                                   &dwSSPIOutFlags,
                                                                   &tsExpiry);

  if (scRet != SEC_I_CONTINUE_NEEDED) {
    CString	szError;
    szError.Format(_T("**** Error %d returned by InitializeSecurityContext (1)\n"), scRet);
    TRACE( (LPCTSTR)szError);
    return false;
  }

  m_hasSecurityContext = true;

  // Send response to server if there is one.
  if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
    DWORD cbData;
    WSABUF wsaBuff;
    wsaBuff.len	=  OutBuffers[0].cbBuffer;
    wsaBuff.buf = (char*)OutBuffers[0].pvBuffer;
    internalWrite( &wsaBuff, 1, &cbData);

    if (cbData == SOCKET_ERROR || cbData == 0) {
      CString		szError;
      szError.Format(_T("**** Error %d sending data to server (1)\n"),
                     WSAGetLastError());
      TRACE((LPCTSTR)szError);

      m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
      m_SecurityFunc.DeleteSecurityContext(&m_hContext);
      return false;
    }

    // Free output buffer.
    m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
    OutBuffers[0].pvBuffer = NULL;
  }
  return clientHandshakeLoop(Socket, TRUE, pExtraData);
}

bool CSecureEvtSyncSocket::clientHandshakeLoop(SOCKET /* Socket */,
                                               BOOL fDoInitialRead,
                                               SecBuffer *pExtraData)
{
  SecBufferDesc InBuffer;
  SecBuffer InBuffers[2];
  SecBufferDesc OutBuffer;
  SecBuffer OutBuffers[1];
	bool retval = true;

  DWORD dwSSPIFlags = (ISC_REQ_SEQUENCE_DETECT |
                       ISC_REQ_REPLAY_DETECT |
                       ISC_REQ_CONFIDENTIALITY |
                       ISC_RET_EXTENDED_ERROR |
                       ISC_REQ_ALLOCATE_MEMORY |
                       ISC_REQ_STREAM);

  //
  // Allocate data buffer.
  //
  PUCHAR IoBuffer = (PUCHAR)LocalAlloc(LMEM_FIXED, HTTPS_IO_BUFFER_SIZE);
  if (IoBuffer == NULL) {
    TRACE("**** Out of memory (1)\n");
    return false;
  }


  // 
  // Loop until the handshake is finished or an error occurs.
  //

  DWORD cbIoBuffer = 0;
  BOOL fDoRead = fDoInitialRead;
  SECURITY_STATUS scRet = SEC_I_CONTINUE_NEEDED;

  while(scRet == SEC_I_CONTINUE_NEEDED        ||
        scRet == SEC_E_INCOMPLETE_MESSAGE     ||
        scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
    DWORD cbData;
    //
    // Read data from server.
    //
    if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
      if (fDoRead) {
        cbData = HTTPS_IO_BUFFER_SIZE - cbIoBuffer;
        internalRead( (const unsigned char*)IoBuffer + cbIoBuffer, &cbData);
        if (cbData == SOCKET_ERROR) {
          CString		szError;
          szError.Format(_T("**** Error %d reading data from server\n"), WSAGetLastError());
          TRACE((LPCTSTR)szError);
          scRet = SEC_E_INTERNAL_ERROR;
          retval	=	false;
          break;
        } else if (cbData == 0) {
          TRACE("**** Server unexpectedly disconnected\n");
          scRet = SEC_E_INTERNAL_ERROR;
          retval = false;
          break;
        }
        cbIoBuffer += cbData;
      } else {
        fDoRead = TRUE;
      }
    }
    if (!retval)
      return retval;
    //
    // Set up the input buffers. Buffer 0 is used to pass in data
    // received from the server. Schannel will consume some or all
    // of this. Leftover data (if any) will be placed in buffer 1 and
    // given a buffer type of SECBUFFER_EXTRA.
    //

    InBuffers[0].pvBuffer   = IoBuffer;
    InBuffers[0].cbBuffer   = cbIoBuffer;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InBuffers[1].pvBuffer   = NULL;
    InBuffers[1].cbBuffer   = 0;
    InBuffers[1].BufferType = SECBUFFER_EMPTY;

    InBuffer.cBuffers       = 2;
    InBuffer.pBuffers       = InBuffers;
    InBuffer.ulVersion      = SECBUFFER_VERSION;

    //
    // Set up the output buffers. These are initialized to NULL
    // so as to make it less likely we'll attempt to free random
    // garbage later.
    //

    OutBuffers[0].pvBuffer  = NULL;
    OutBuffers[0].BufferType= SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer  = 0;

    OutBuffer.cBuffers      = 1;
    OutBuffer.pBuffers      = OutBuffers;
    OutBuffer.ulVersion     = SECBUFFER_VERSION;

    //
    // Call InitializeSecurityContext.
    //

    DWORD dwSSPIOutFlags;
    TimeStamp tsExpiry;
    scRet = m_SecurityFunc.InitializeSecurityContext(&m_hClientCreds,
                                                     &m_hContext,
                                                     NULL,
                                                     dwSSPIFlags,
                                                     0,
                                                     SECURITY_NATIVE_DREP,
                                                     &InBuffer,
                                                     0,
                                                     NULL,
                                                     &OutBuffer,
                                                     &dwSSPIOutFlags,
                                                     &tsExpiry);

    //
    // If InitializeSecurityContext was successful (or if the error was 
    // one of the special extended ones), send the contends of the output
    // buffer to the server.
    //

    if (scRet == SEC_E_OK                ||
        scRet == SEC_I_CONTINUE_NEEDED   ||
        FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR)) {
      if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
        WSABUF	wsaBuff;
        wsaBuff.len	=  OutBuffers[0].cbBuffer;
        wsaBuff.buf = (char*)OutBuffers[0].pvBuffer;
        internalWrite( &wsaBuff, 1, &cbData);
                
        if (cbData == SOCKET_ERROR || cbData == 0) {
          CString		szError;
          szError.Format(_T("**** Error %d sending data to server (2)\n"), 
                         WSAGetLastError());
          TRACE((LPCTSTR)szError);
          m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
          m_SecurityFunc.DeleteSecurityContext(&m_hContext);
          m_hasSecurityContext = false;
          // return SEC_E_INTERNAL_ERROR;
          return false;
        }

        // Free output buffer.
        m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
        OutBuffers[0].pvBuffer = NULL;
      }
    }

    //
    // If InitializeSecurityContext returned SEC_E_INCOMPLETE_MESSAGE,
    // then we need to read more data from the server and try again.
    //

    if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
      continue;
    }


    //
    // If InitializeSecurityContext returned SEC_E_OK, then the 
    // handshake completed successfully.
    //

    if (scRet == SEC_E_OK) {
      //
      // If the "extra" buffer contains data, this is encrypted application
      // protocol layer stuff. It needs to be saved. The application layer
      // will later decrypt it with DecryptMessage.
      //
      if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
        pExtraData->pvBuffer = LocalAlloc(LMEM_FIXED, 
                                          InBuffers[1].cbBuffer);
        if (pExtraData->pvBuffer == NULL) {
          TRACE("**** Out of memory (2)\n");
          return false;
          //return SEC_E_INTERNAL_ERROR;
        }

        memmove_s(pExtraData->pvBuffer, InBuffers[1].cbBuffer,
                  IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                  InBuffers[1].cbBuffer);

        pExtraData->cbBuffer   = InBuffers[1].cbBuffer;
        pExtraData->BufferType = SECBUFFER_TOKEN;
      } else {
        pExtraData->pvBuffer   = NULL;
        pExtraData->cbBuffer   = 0;
        pExtraData->BufferType = SECBUFFER_EMPTY;
      }
      //
      // Bail out to quit
      //
      break;
    }

    //
    // Check for fatal error.
    //

    if (FAILED(scRet)) {
      CString szError;
      szError.Format(_T("**** Error 0x%x returned by InitializeSecurityContext (2)\n"), scRet);
      TRACE((LPCTSTR)szError);
      retval	=	false;
      break;
    }


    //
    // If InitializeSecurityContext returned SEC_I_INCOMPLETE_CREDENTIALS,
    // then the server just requested client authentication. 
    //

    if (scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
      //
      // Display trusted issuers info. 
      //

      getNewClientCredentials();

      // As this is currently written, Schannel will send a "no 
      // certificate" alert to the server in place of a certificate. 
      // The server might be cool with this, or it might drop the 
      // connection.
      // 
      // TODO : This would be a good time to prompt the user to select
      // a client certificate and obtain a new credential handle.

      // Go around again.
      fDoRead = FALSE;
      scRet = SEC_I_CONTINUE_NEEDED;
      continue;
    }

    //
    // Copy any leftover data from the "extra" buffer, and go around
    // again.
    //

    if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
      memmove_s(IoBuffer, HTTPS_IO_BUFFER_SIZE,
                IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                InBuffers[1].cbBuffer);

      cbIoBuffer = InBuffers[1].cbBuffer;
    } else {
      cbIoBuffer = 0;
    }
  }

  // Delete the security context in the case of a fatal error.
  if (FAILED(scRet)) {
    retval	=	false;
    m_SecurityFunc.DeleteSecurityContext(&m_hContext);
    m_hasSecurityContext = false;
  }

  LocalFree(IoBuffer);

  return retval;
}

void CSecureEvtSyncSocket::getNewClientCredentials()
{
  //
  // Read list of trusted issuers from schannel.
  //

  SecPkgContext_IssuerListInfoEx	IssuerListInfo;
  SECURITY_STATUS Status = m_SecurityFunc.QueryContextAttributes(&m_hContext,
                                                                 SECPKG_ATTR_ISSUER_LIST_EX,
                                                                 (PVOID)&IssuerListInfo);
  if (Status != SEC_E_OK) {
    CString	szError;
		szError.Format(_T("Error 0x%x querying issuer list info\n"), Status);
		TRACE((LPCTSTR)szError);
    return;
  }

  //
  // Enumerate the client certificates.
  //

  CERT_CHAIN_FIND_BY_ISSUER_PARA	FindByIssuerPara;
  ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));

  FindByIssuerPara.cbSize = sizeof(FindByIssuerPara);
  FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
  FindByIssuerPara.dwKeySpec = 0;
  FindByIssuerPara.cIssuer   = IssuerListInfo.cIssuers;
  FindByIssuerPara.rgIssuer  = IssuerListInfo.aIssuers;

  PCCERT_CHAIN_CONTEXT pChainContext = NULL;

  while(TRUE) {
    // Find a certificate chain.
    pChainContext = CertFindChainInStore(m_hMyCertStore,
                                         X509_ASN_ENCODING,
                                         0,
                                         CERT_CHAIN_FIND_BY_ISSUER,
                                         &FindByIssuerPara,
                                         pChainContext);
    if (pChainContext == NULL) {
      CString	szError;
      szError.Format(_T("Error 0x%x finding cert chain\n"), GetLastError());
      TRACE((LPCTSTR)szError);
      break;
    }

    // Get pointer to leaf certificate context.
    PCCERT_CONTEXT pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;

    // Create schannel credential.
    m_SchannelCred.cCreds = 1;
    m_SchannelCred.paCred = &pCertContext;

    CredHandle hCreds;
    TimeStamp tsExpiry;
    Status = m_SecurityFunc.AcquireCredentialsHandle(NULL,
                                                     UNISP_NAME,
                                                     SECPKG_CRED_OUTBOUND,
                                                     NULL,
                                                     &m_SchannelCred,
                                                     NULL,
                                                     NULL,
                                                     &hCreds,
                                                     &tsExpiry);
    if (Status != SEC_E_OK) {
      CString	szError;
      szError.Format(_T("**** Error 0x%x returned by AcquireCredentialsHandle\n"), Status);
      TRACE((LPCTSTR)szError);
      continue;
    }

    // Destroy the old credentials.
    m_SecurityFunc.FreeCredentialsHandle(&m_hClientCreds);

    m_hClientCreds = hCreds;
    break;
  }
}

bool	CSecureEvtSyncSocket::verifyServerCertificate(LPCTSTR pszServerName, DWORD dwCertFlags)
{
  PCCERT_CHAIN_CONTEXT pChainContext = NULL;
  PCCERT_CONTEXT pServerCert = NULL;
  DWORD Status = m_SecurityFunc.QueryContextAttributes(&m_hContext,
                                                       SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                                       (PVOID)&pServerCert);
  if (Status != SEC_E_OK) {
    CString szError;
    szError.Format(_T("Error 0x%x querying remote certificate\n"), Status);
    TRACE( szError);
    return false;
  }

  if (pServerCert == NULL) {
    //SEC_E_WRONG_PRINCIPAL;
    return false;
  }


  //
  // Convert server name to unicode.
  //

  if (pszServerName == NULL || _tcslen(pszServerName) == 0) {
    // SEC_E_WRONG_PRINCIPAL;
    return false;
  }

  PWSTR pwszServerName;
  DWORD cchServerName;

#ifndef UNICODE
  cchServerName	= MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, NULL, 0);
  pwszServerName	= (PWSTR)LocalAlloc(LMEM_FIXED, cchServerName * sizeof(WCHAR));
  if (pwszServerName == NULL) {
    // SEC_E_INSUFFICIENT_MEMORY;
    return false;
  }
#endif
	bool	retval	=	true;
#ifndef UNICODE
	cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, pwszServerName, cchServerName);
#else
  pwszServerName = PWSTR(pszServerName);
  cchServerName = _tcslen(pszServerName);
#endif
  if (cchServerName == 0) {
    //SEC_E_WRONG_PRINCIPAL;
    retval = false;
    goto cleanup;
  }


  //
  // Build certificate chain.
  //

  CERT_CHAIN_PARA ChainPara;
  ZeroMemory(&ChainPara, sizeof(ChainPara));
  ChainPara.cbSize = sizeof(ChainPara);

  if (!CertGetCertificateChain(NULL,
                               pServerCert,
                               NULL,
                               pServerCert->hCertStore,
                               &ChainPara,
                               0,
                               NULL,
                               &pChainContext)) {
    CString		szError;
    Status = GetLastError();
    szError.Format(_T("Error 0x%x returned by CertGetCertificateChain!\n"), Status);
    TRACE((LPCTSTR)szError);
    goto cleanup;
  }


  //
  // Validate certificate chain.
  // 

  HTTPSPolicyCallbackData  polHttps;
  ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
  polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
  polHttps.dwAuthType         = AUTHTYPE_SERVER;
  polHttps.fdwChecks          = dwCertFlags;
  polHttps.pwszServerName     = pwszServerName;

  CERT_CHAIN_POLICY_PARA PolicyPara;
  memset(&PolicyPara, 0, sizeof(PolicyPara));
  PolicyPara.cbSize            = sizeof(PolicyPara);
  PolicyPara.pvExtraPolicyPara = &polHttps;

  CERT_CHAIN_POLICY_STATUS PolicyStatus;
  memset(&PolicyStatus, 0, sizeof(PolicyStatus));
  PolicyStatus.cbSize = sizeof(PolicyStatus);

  if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL,
                                        pChainContext,
                                        &PolicyPara,
                                        &PolicyStatus)) {
    CString		szError;
    Status = GetLastError();
    szError.Format(_T("Error 0x%x returned by CertVerifyCertificateChainPolicy!\n"),
                   Status);
    TRACE((LPCTSTR)szError);
    goto cleanup;
  }


  // XXX HACK to test!!!
	if (PolicyStatus.dwError && PolicyStatus.dwError != CERT_E_CN_NO_MATCH) {
    TRACE(_T("%s"), VerifyTrustErrorString(PolicyStatus.dwError));
    retval	=	false;
    goto cleanup;
  }
  Status = SEC_E_OK;

 cleanup:
  if (pChainContext) {
    CertFreeCertificateChain(pChainContext);
  }
#ifndef UNICODE
	if (pwszServerName) {
    ::LocalFree(pwszServerName);
  }
#endif
	return retval;
}

// Overrides for Read, Write
bool CSecureEvtSyncSocket::Write(WSABUF *pBuffers, int nCount, DWORD *pdwSize) 
{
	bool retval	=	true;
	if (!m_bHttpsCommunication)
		return internalWrite( pBuffers, nCount, pdwSize);
	
	SecPkgContext_StreamSizes		Sizes;
  SECURITY_STATUS					scRet;
  DWORD							dwIoBufferLength;
	WSABUF							*arInternalBuffers = NULL;
	int								iIndex;

  //
  // Read stream encryption properties.
  //

  scRet = m_SecurityFunc.QueryContextAttributes(&m_hContext,
                                                SECPKG_ATTR_STREAM_SIZES,
                                                &Sizes);
  if (scRet != SEC_E_OK) {
    CString		szError;
    szError.Format(_T("**** Error 0x%x reading SECPKG_ATTR_STREAM_SIZES\n"), scRet);
    TRACE((LPCTSTR)szError);
    return false;
  }
	arInternalBuffers	=	new WSABUF[nCount];
	memset(arInternalBuffers, 0, nCount*sizeof(WSABUF));

	dwIoBufferLength =	Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;
	

	*pdwSize	=	0;
	for (iIndex = 0; iIndex < nCount; iIndex ++) {
    PBYTE			pbIoBuffer = NULL;
    DWORD			dwMessageSize	=	pBuffers[iIndex].len;
    SecBufferDesc	Message;
    SecBuffer		Buffers[4];

    *pdwSize += dwMessageSize;
		
    pbIoBuffer =	new BYTE[dwIoBufferLength];

    ASSERT(pbIoBuffer != NULL);
    ASSERT( dwMessageSize < Sizes.cbMaximumMessage);

    if (NULL == pbIoBuffer || dwMessageSize > Sizes.cbMaximumMessage) {
      retval = false;
      if (pbIoBuffer)delete[] pbIoBuffer;
      break;
    }

    memcpy_s( pbIoBuffer + Sizes.cbHeader,
              sizeof(pbIoBuffer) - Sizes.cbHeader, pBuffers[iIndex].buf, dwMessageSize);

    Buffers[0].pvBuffer     = pbIoBuffer;
    Buffers[0].cbBuffer     = Sizes.cbHeader;
    Buffers[0].BufferType   = SECBUFFER_STREAM_HEADER;

    Buffers[1].pvBuffer     = pbIoBuffer	+	Sizes.cbHeader;
    Buffers[1].cbBuffer     = dwMessageSize;
    Buffers[1].BufferType   = SECBUFFER_DATA;

    Buffers[2].pvBuffer     = pbIoBuffer	+	Sizes.cbHeader + dwMessageSize;
    Buffers[2].cbBuffer     = Sizes.cbTrailer;
    Buffers[2].BufferType   = SECBUFFER_STREAM_TRAILER;

    Buffers[3].BufferType   = SECBUFFER_EMPTY;

    Message.ulVersion       = SECBUFFER_VERSION;
    Message.cBuffers        = 4;
    Message.pBuffers        = Buffers;

    scRet = m_SecurityFunc.EncryptMessage(&m_hContext, 0,
                                          &Message, 0);

    if (FAILED(scRet)) {
      CString		szError;
      szError.Format(_T("**** Error 0x%x returned by EncryptMessage\n"), scRet);
      TRACE((LPCTSTR)szError);
      retval	=	false;
    }

    arInternalBuffers[iIndex].len	=	Buffers[0].cbBuffer+Buffers[1].cbBuffer+Buffers[2].cbBuffer;
    arInternalBuffers[iIndex].buf	=	(PCHAR)pbIoBuffer;
  }

	DWORD dwWrittenEnc;
	if (retval)
		retval	=	internalWrite( arInternalBuffers, nCount, &dwWrittenEnc);

	for(iIndex = 0; iIndex < nCount; iIndex ++) {
    delete[] arInternalBuffers[iIndex].buf;
  }
	delete[]	arInternalBuffers;

	if (!retval)
		*pdwSize	=	0;
	return retval;
}

bool CSecureEvtSyncSocket::Read(const unsigned char *pBuff, DWORD *pdwSize) 
{
	bool retval	=	true;
	if (!m_bHttpsCommunication)
		return internalRead(pBuff, pdwSize);

  //
  // Read stream encryption properties.
  //
	SecPkgContext_StreamSizes Sizes;
  SECURITY_STATUS scRet = m_SecurityFunc.QueryContextAttributes(&m_hContext,
                                                                SECPKG_ATTR_STREAM_SIZES,
                                                                &Sizes);
  if (scRet != SEC_E_OK) {
    CString		szError;
    szError.Format(_T("**** Error 0x%x reading SECPKG_ATTR_STREAM_SIZES\n"), scRet);
    TRACE((LPCTSTR)szError);
    return false;
  }

	// This asserts that the size of the answer is smaller than maximum encrypted message size and ATL_READ_BUFF_SIZE
	DWORD dwIoBufferLength =	Sizes.cbHeader + min(ATL_READ_BUFF_SIZE, Sizes.cbMaximumMessage) +   Sizes.cbTrailer;
	unsigned char	*pReadBuff = new unsigned char[dwIoBufferLength];
	DWORD dwIoBuffer = 0;
	DWORD dwActualReadSize = 0;

	while (1) {
    SecBufferDesc	Message;
    SecBuffer		Buffers[4];
        
    //
    // Read some data.
    //
    if (0 == dwIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
      DWORD	dwRead	=	dwIoBufferLength - dwIoBuffer;
      bool	bReadRet;
      bReadRet	=	internalRead( (unsigned char*)pReadBuff + dwIoBuffer, &dwRead);
      if (!bReadRet) {
        CString	szError;
        szError.Format(_T("**** Error %d reading data from server\n"), WSAGetLastError());
        scRet = SEC_E_INTERNAL_ERROR;
        retval	=	false;
        break;
      } else if (dwRead == 0) {
        // Server disconnected.
        retval	=	false;
        break;
      } else {
        dwIoBuffer += dwRead;
      }
    }

    // 
    // Attempt to decrypt the received data.
    //

    Buffers[0].pvBuffer     = pReadBuff;
    Buffers[0].cbBuffer     = dwIoBuffer;
    Buffers[0].BufferType   = SECBUFFER_DATA;

    Buffers[1].BufferType   = SECBUFFER_EMPTY;
    Buffers[2].BufferType   = SECBUFFER_EMPTY;
    Buffers[3].BufferType   = SECBUFFER_EMPTY;

    Message.ulVersion       = SECBUFFER_VERSION;
    Message.cBuffers        = 4;
    Message.pBuffers        = Buffers;

    scRet = m_SecurityFunc.DecryptMessage(&m_hContext,
                                          &Message, 0, NULL);

    if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
      // The input buffer contains only a fragment of an
      // encrypted record. Loop around and read some more
      // data.
      continue;
    }

    // Server signalled end of session
    if (scRet == SEC_I_CONTEXT_EXPIRED) {
      retval	=	false;
      break;
    }

    if (scRet != SEC_E_OK && 
        scRet != SEC_I_RENEGOTIATE && 
        scRet != SEC_I_CONTEXT_EXPIRED) {
      CString	szError;
      szError.Format(_T("**** Error 0x%x returned by DecryptMessage\n"), scRet);
      TRACE( szError);
      retval	=	false;
      break;
    }

    // Locate data and (optional) extra buffers.
    SecBuffer *     pDataBuffer;
    SecBuffer *     pExtraBuffer;

    pDataBuffer  = NULL;
    pExtraBuffer = NULL;
    for(int i = 1; i < 4; i++) {
      if (pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA) {
        pDataBuffer = &Buffers[i];
      }
      if (pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA) {
        pExtraBuffer = &Buffers[i];
      }
    }

    // Display or otherwise process the decrypted data.
    if (pDataBuffer) {
      ASSERT(pDataBuffer->cbBuffer < (*pdwSize - dwActualReadSize));

      if (pDataBuffer->cbBuffer >= (*pdwSize - dwActualReadSize)) {
        retval	=	false;
        break;
      }

      memcpy_s((void*)(pBuff + dwActualReadSize), pDataBuffer->cbBuffer,
               (PBYTE)pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);
      dwActualReadSize += pDataBuffer->cbBuffer;
    }

    // Move any "extra" data to the input buffer.
    if (pExtraBuffer) {
      memmove_s(pReadBuff, dwIoBufferLength, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
      dwIoBuffer = pExtraBuffer->cbBuffer;
    } else { // That's all, no extra buffer, everything is read, so
      break;
    }

    if (scRet == SEC_I_RENEGOTIATE) {
      SecBuffer ExtraBuffer;
      // The server wants to perform another handshake
      // sequence.

      TRACE("Server requested renegotiate!\n");
      scRet = clientHandshakeLoop(m_socket, 
                                  FALSE, 
                                  &ExtraBuffer);
      if (scRet != SEC_E_OK) {
        retval	=	false;
        break;
      }
      // Move any "extra" data to the input buffer.
      if (ExtraBuffer.pvBuffer) {
        memmove_s(pReadBuff, dwIoBufferLength, ExtraBuffer.pvBuffer, ExtraBuffer.cbBuffer);
        dwIoBuffer = ExtraBuffer.cbBuffer;
      }
    }
  }


	delete[] pReadBuff;

	if (retval)
    *pdwSize	=	dwActualReadSize;
	return retval;
}


bool CSecureEvtSyncSocket::disconnectFromSecureServer()
{
  SecBufferDesc   OutBuffer;
  SecBuffer       OutBuffers[1];
  DWORD           dwSSPIOutFlags;
  TimeStamp       tsExpiry;

  //
  // Notify schannel that we are about to close the connection.
  //

  DWORD dwType = SCHANNEL_SHUTDOWN;

  OutBuffers[0].pvBuffer   = &dwType;
  OutBuffers[0].BufferType = SECBUFFER_TOKEN;
  OutBuffers[0].cbBuffer   = sizeof(dwType);

  OutBuffer.cBuffers  = 1;
  OutBuffer.pBuffers  = OutBuffers;
  OutBuffer.ulVersion = SECBUFFER_VERSION;

  DWORD Status = m_SecurityFunc.ApplyControlToken(&m_hContext, &OutBuffer);

  if (FAILED(Status)) {
    CString	szError;
    szError.Format(_T("**** Error 0x%x returned by ApplyControlToken\n"), Status);
    TRACE(szError);
    goto cleanup;
  }

  //
  // Build an SSL close notify message.
  //

  DWORD dwSSPIFlags = (ISC_REQ_SEQUENCE_DETECT |
                       ISC_REQ_REPLAY_DETECT |
                       ISC_REQ_CONFIDENTIALITY |
                       ISC_RET_EXTENDED_ERROR |
                       ISC_REQ_ALLOCATE_MEMORY |
                       ISC_REQ_STREAM);

  OutBuffers[0].pvBuffer   = NULL;
  OutBuffers[0].BufferType = SECBUFFER_TOKEN;
  OutBuffers[0].cbBuffer   = 0;

  OutBuffer.cBuffers  = 1;
  OutBuffer.pBuffers  = OutBuffers;
  OutBuffer.ulVersion = SECBUFFER_VERSION;

  Status = m_SecurityFunc.InitializeSecurityContext(&m_hClientCreds,
                                                    &m_hContext,
                                                    NULL,
                                                    dwSSPIFlags,
                                                    0,
                                                    SECURITY_NATIVE_DREP,
                                                    NULL,
                                                    0,
                                                    &m_hContext,
                                                    &OutBuffer,
                                                    &dwSSPIOutFlags,
                                                    &tsExpiry);

  if (FAILED(Status)) {
    CString szError;
    szError.Format(_T("**** Error 0x%x returned by InitializeSecurityContext\n"), Status);
    TRACE(szError);
    goto cleanup;
  }

  char *pbMessage = (char*)OutBuffers[0].pvBuffer;
  DWORD cbMessage = OutBuffers[0].cbBuffer;

  //
  // Send the close notify message to the server.
  //
  if (pbMessage != NULL && cbMessage != 0) {
    DWORD cbData;
    WSABUF wsaBuff;
    wsaBuff.len	=  cbMessage;
    wsaBuff.buf = pbMessage;
    internalWrite( &wsaBuff, 1, &cbData);
        
    // cbData = send(m_socket, pbMessage, cbMessage, 0);
    if (cbData == SOCKET_ERROR || cbData == 0) {
      Status = WSAGetLastError();
      CString	szError;
      szError.Format(_T("**** Error %d sending close notify\n"), Status);
      TRACE(szError);
      goto cleanup;
    }

    TRACE("Sending Close Notify\n");

    // Free output buffer.
    m_SecurityFunc.FreeContextBuffer(pbMessage);
  }

 cleanup:
  // Free the security context.
  m_SecurityFunc.DeleteSecurityContext(&m_hContext);
  m_hasSecurityContext = false;
  return true;
}
