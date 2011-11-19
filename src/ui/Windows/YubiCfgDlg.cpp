/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// YubiCfgDlg.cpp : implementation file
//

#include <afxdisp.h>
#include <afxctl.h>
#include <afxwin.h>
#include <iomanip>
#include <sstream>

#include "stdafx.h"
#include "YubiCfgDlg.h"
#include "afxdialogex.h"

#include "Yubi.h"
#include "yubi/yklib.h"
#include "core/StringX.h"
#include "core/PWScore.h"

#include "os/rand.h"

using namespace std;

// CYubiCfgDlg dialog


CYubiCfgDlg::CYubiCfgDlg(CWnd* pParent, PWScore &core)
	: CPWDialog(CYubiCfgDlg::IDD, pParent), m_core(core), m_YubiSN(_T("")),
    m_YubiSK(_T("")), m_obj(0), m_isInit(false)
{
  EnableAutomation();
  memset(m_yubi_sk_bin, 0, YUBI_SK_LEN);
}

CYubiCfgDlg::~CYubiCfgDlg()
{
  trashMemory(m_yubi_sk_bin, YUBI_SK_LEN);
}

void CYubiCfgDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_YUBI_SN, m_YubiSN);
    DDX_Text(pDX, IDC_YUBI_SK, m_YubiSK);
}

static StringX BinSK2HexStr(const unsigned char *sk, int len)
{
  wostringstream os;
  os << setw(2);
  os << setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << hex << setw(2) << int(sk[i]);
    if (i != len - 1)
      os << " ";
  }
  return StringX(os.str().c_str());
}

static void HexStr2BinSK(const StringX &str, unsigned char *sk, int len)
{
  wistringstream is(str.c_str());
  is >> hex;
  int i = 0;
  int b;
  while ((is >> b ) && i < len) {
    sk[i++] = (unsigned char)b;
  }
}

BOOL CYubiCfgDlg::OnInitDialog()
{
  Init();
  if (m_isInit) {
    GetDlgItem(IDC_YUBI_API)->ShowWindow(SW_HIDE);
    if (m_core.GetYubiSK() != NULL) {
      m_YubiSK = BinSK2HexStr(m_core.GetYubiSK(), YUBI_SK_LEN).c_str();
    }
    ReadYubiSN();
  } else { // !m_isInit
    GetDlgItem(IDC_YUBI_API)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_YUBI_SN)->EnableWindow(FALSE);
    GetDlgItem(IDC_YUBI_SK)->EnableWindow(FALSE);
    GetDlgItem(IDC_YUBI_GEN_BN)->EnableWindow(FALSE);
  }
  return TRUE;
}

void CYubiCfgDlg::OnDestroy()
{
  Destroy();
}

void CYubiCfgDlg::Init()
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
                             GetIDispatch(FALSE),
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

void CYubiCfgDlg::Destroy()
{
  if (m_obj) {
    AfxConnectionUnadvise(m_obj, DIID__IYubiClientEvents,
                          GetIDispatch(FALSE), FALSE, m_eventCookie);
    m_obj->Release();
    m_obj = 0;
  }
}

void CYubiCfgDlg::ReadYubiSN()
{
  CYkLib yk;
  BYTE buffer[128];
  YKLIB_RC rc;
  STATUS status;

  memset(&status, 0, sizeof(status));
  rc = yk.readSerialBegin();
  if (rc == YKLIB_OK) {
    // Wait for response completion
    rc = yk.waitForCompletion(YKLIB_MAX_SERIAL_WAIT, buffer, sizeof(DWORD));

    if (rc == YKLIB_OK) {
      m_YubiSN = BinSK2HexStr(buffer, 4).c_str();
    } else
      m_YubiSN = Yubi::RetCode2String(rc).c_str();
  } else
    m_YubiSN = Yubi::RetCode2String(rc).c_str();
}

BEGIN_MESSAGE_MAP(CYubiCfgDlg, CPWDialog)
    ON_BN_CLICKED(IDC_YUBI_GEN_BN, &CYubiCfgDlg::OnYubiGenBn)
    ON_BN_CLICKED(IDOK, &CYubiCfgDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CYubiCfgDlg, CPWDialog)
	//{{AFX_DISPATCH_MAP(CPasskeyEntry)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CYubiCfgDlg, "deviceInserted", 1, yubiInserted, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CYubiCfgDlg, "deviceRemoved", 2, yubiRemoved, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CYubiCfgDlg, "operationCompleted", 3, yubiCompleted, VT_EMPTY, VTS_I2)
  DISP_FUNCTION_ID(CYubiCfgDlg, "userWait", 4, yubiWait, VT_EMPTY, VTS_I2)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CYubiCfgDlg, CPWDialog)
	INTERFACE_PART(CYubiCfgDlg, DIID__IYubiClientEvents, Dispatch)
END_INTERFACE_MAP()

void CYubiCfgDlg::yubiInserted(void)
{
  GetDlgItem(IDC_YUBI_SN)->EnableWindow(TRUE);
  GetDlgItem(IDC_YUBI_SK)->EnableWindow(TRUE);
  GetDlgItem(IDC_YUBI_GEN_BN)->EnableWindow(TRUE);
}

void CYubiCfgDlg::yubiRemoved(void)
{
  GetDlgItem(IDC_YUBI_SN)->EnableWindow(FALSE);
  GetDlgItem(IDC_YUBI_SK)->EnableWindow(FALSE);
  GetDlgItem(IDC_YUBI_GEN_BN)->EnableWindow(FALSE);
}

void CYubiCfgDlg::yubiCompleted(ycRETCODE rc)
{
#if 0
  m_yubi_status.ShowWindow(SW_SHOW);
  m_yubi_timeout.ShowWindow(SW_HIDE);
  switch (rc) {
  case ycRETCODE_OK:
    m_yubi_timeout.SetPos(0);
    m_yubi_status.SetWindowText(_T(""));
    TRACE(_T("yubiCompleted(ycRETCODE_OK)"));
    // Get hmac, process it, synthesize OK event
    m_yubi->RetrieveHMACSha1(m_passkey);
    // The returned hash is the passkey
    ProcessPhrase();
    // If we returned from above, reset status:
    m_yubi_status.SetWindowText(_T("Click, then activate your YubiKey"));
    break;
  case ycRETCODE_NO_DEVICE:
    // device removed while waiting?
    TRACE(_T("yubiCompleted(ycRETCODE_NO_DEVICE)\n"));
    m_yubi_status.SetWindowText(_T("Error: YubiKey removed"));
    break;
  case ycRETCODE_TIMEOUT:
    // waited, no user input
    TRACE(_T("yubiCompleted(ycRETCODE_TIMEOUT)\n"));
    m_yubi_status.SetWindowText(_T("YubiKey timed out"));
    break;
  case ycRETCODE_MORE_THAN_ONE:
    TRACE(_T("yubiCompleted(ycRETCODE_MORE_THAN_ONE)\n"));
    m_yubi_status.SetWindowText(_T("More than one YubiKey detected"));
    break;
  case ycRETCODE_REENTRANT_CALL:
    TRACE(_T("yubiCompleted(ycRETCODE_REENTRANT_CALL)\n"));
    m_yubi_status.SetWindowText(_T("Internal error: Reentrant call"));
    break;
  case ycRETCODE_FAILED:
    TRACE(_T("yubiCompleted(ycRETCODE_FAILED)\n"));
    if (m_waited) {
      m_yubi_status.SetWindowText(_T("YubiKey timed out: Click to try again"));
      m_waited = false;
    } else
      m_yubi_status.SetWindowText(_T("YubiKey returned an error. Unconfigured?"));
    break;
  default:
    // Generic error message
    TRACE(_T("yubiCompleted(%d)\n"), rc);
    m_yubi_status.SetWindowText(_T("Internal error: Unknown return code"));
    break;
  }
#endif
}

void CYubiCfgDlg::yubiWait(WORD seconds)
{
}

// CYubiCfgDlg message handlers


void CYubiCfgDlg::OnYubiGenBn()
{
  pws_os::GetRandomData(m_yubi_sk_bin, YUBI_SK_LEN);
  StringX str = BinSK2HexStr(m_yubi_sk_bin, YUBI_SK_LEN);
  m_YubiSK = str.c_str();
  UpdateData(FALSE);
}


void CYubiCfgDlg::OnBnClickedOk()
{
  UpdateData(TRUE);  
  StringX skStr = m_YubiSK;
  
  if (!skStr.empty()) {
    StringX oldSK = BinSK2HexStr(m_yubi_sk_bin, YUBI_SK_LEN);
    if (skStr != oldSK) {
      // 1. Update SK on Yubi.
      // 2. If that succeeds, update in core.
      // 3. Write DB ASAP!
    }
  }
  CPWDialog::OnOK();
}
