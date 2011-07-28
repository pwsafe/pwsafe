/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * Interface / wrapper to YubiKey API
 */
#include <afxdisp.h>
#include <afxctl.h>
#include <afxwin.h>
#include "StdAfx.h"
#include "Yubi.h"

Yubi::Yubi(CCmdTarget *owner)
  : m_obj(0), m_owner(owner), m_isInit(false)
{
  m_owner->EnableAutomation();
}

void Yubi::Init()
{
  m_isInit = false;
	HRESULT hr = CoCreateInstance(CLSID_YubiClient, 0, CLSCTX_ALL,
                                IID_IYubiClient, reinterpret_cast<void **>(&m_obj));

	if (FAILED(hr)) {
#ifdef DEBUG
		_com_error er(hr);
		AfxMessageBox(er.ErrorMessage());
#endif
	} else {
		if (!AfxConnectionAdvise(m_obj, DIID__IYubiClientEvents,
                             m_owner->GetIDispatch(FALSE),
                             FALSE, &m_eventCookie)) {
#ifdef DEBUG
      AfxMessageBox(_T("Advise failed"));
#endif
      Destroy();
    } else {
      m_obj->enableNotifications = ycNOTIFICATION_ON;
      m_isInit = true;
    }
  }
}

void Yubi::Destroy()
{
  if (m_obj) {
    AfxConnectionUnadvise(m_obj, DIID__IYubiClientEvents,
                          m_owner->GetIDispatch(FALSE), FALSE, m_eventCookie);
    m_obj->Release();
    m_obj = 0;
  }
}

bool Yubi::isInserted() const
{
  return (m_obj != NULL && m_obj->GetisInserted() == ycRETCODE_OK);
}

void Yubi::RequestHMACSha1(void *data, int len)
{
  ASSERT(m_obj != NULL);
  ASSERT(data != NULL);
  bool setBufOK = true;
  try {
    _variant_t va;

    va.parray = SafeArrayCreateVectorEx(VT_UI1, 0, len, 0);
    BYTE HUGEP *pb;

    SafeArrayAccessData(va.parray, (void HUGEP **) &pb);
    memcpy(pb, data, len);
    SafeArrayUnaccessData(va.parray);
    va.vt = VT_ARRAY | VT_UI1;

    m_obj->dataBuffer = va;
  }
#ifdef DEBUG
  catch (_com_error e) {
    setBufOK = false;
    AfxMessageBox(e.ErrorMessage());
  }

  catch (...) {
    setBufOK = false;
    AfxMessageBox(_T("Other error"));
  }
#else
  catch (...) {
    setBufOK = false;
  }
#endif
  // 1 - for second yubikey configuration
  if (setBufOK) {
    ycRETCODE rc = m_obj->GethmacSha1(1, ycCALL_ASYNC);
  }
}

void Yubi::RetrieveHMACSha1(char *hash)
{
  ASSERT(m_obj != NULL);
  ASSERT(hash != NULL);
  try {
    variant_t va = m_obj->dataBuffer;

    switch (va.vt) {
    case VT_NULL:
      TRACE(_T("Yubi::RetrieveHMACSha1: av.vt = VT_NULL\n"));
      *hash = 0;
      break;

    case VT_UI2:
      TRACE(_T("Yubi::RetrieveHMACSha1: av.vt = VT_UI2\n"));
      break;

    case VT_UI4:
      TRACE(_T("Yubi::RetrieveHMACSha1: av.vt = VT_UI4\n"));
      break;

    case VT_BSTR:
      TRACE(_T("Yubi::RetrieveHMACSha1: av.vt = VT_BSTR: %s\n"),
            (LPCTSTR)(_bstr_t)va);
      break;

    case (VT_UI1 | VT_ARRAY):
      {
        if (SafeArrayGetDim(va.parray) != 1) {
          TRACE(_T("Invalid return dimension (should never get here)"));
          ASSERT(0);
          goto fail;
        }   

        LONG lbound = 0, hbound = -1;

        SafeArrayGetLBound(va.parray, 1, &lbound);
        SafeArrayGetUBound(va.parray, 1, &hbound);

        BYTE HUGEP *pb;
        HRESULT hr;

        hr = SafeArrayAccessData(va.parray, (void HUGEP **) &pb);
        if (FAILED(hr)) {
          TRACE(_T("Failed (1)"));
          ASSERT(0);
          goto fail;
        }
        if (hbound - lbound != 20) {
          TRACE(_T("Returned value not sizeof(SHA1): %d\n"), hbound-lbound);
        } else {
          memcpy(hash, pb, hbound - lbound);
        }
        SafeArrayUnaccessData(va.parray);
      }
      break;

    default:
      TRACE(_T("Invalid VARIANT return type %u (0x%04x)"), va.vt, va.vt);
      break;
    }
  fail:
    VariantClear(&va);

  }
#ifdef DEBUG
  catch (_com_error e) {
    AfxMessageBox(e.ErrorMessage());
  }

  catch (...) {
    AfxMessageBox(_T("Other error"));
  }
#else
  catch (...) {
  }
#endif
}
