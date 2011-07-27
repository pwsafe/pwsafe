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
  : m_owner(owner), m_obj(0)
{
  m_owner->EnableAutomation();
}

void Yubi::Init()
{
	HRESULT hr = CoCreateInstance(CLSID_YubiClient, 0, CLSCTX_ALL,
                                IID_IYubiClient, reinterpret_cast<void **>(&m_obj));

	if (FAILED(hr)) {
		_com_error er(hr);
		AfxMessageBox(er.ErrorMessage());
	} else {
		if (!AfxConnectionAdvise(m_obj, DIID__IYubiClientEvents,
                             m_owner->GetIDispatch(FALSE),
                             FALSE, &m_eventCookie))
      AfxMessageBox(_T("Advise failed"));
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
