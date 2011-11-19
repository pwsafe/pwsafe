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
    m_YubiSK(_T("")), m_obj(0), m_isInit(false), m_generated(false)
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
    GetDlgItem(IDC_YUBI_SN)->SetWindowText(m_YubiSN);
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
  rc = yk.openKey();
  if (rc != YKLIB_OK) goto fail;
  rc = yk.readSerialBegin();
  if (rc != YKLIB_OK) goto fail;
  // Wait for response completion
  rc = yk.waitForCompletion(YKLIB_MAX_SERIAL_WAIT, buffer, sizeof(DWORD));
  if (rc != YKLIB_OK) goto fail;
  m_YubiSN = BinSK2HexStr(buffer, 4).c_str();
  rc = yk.closeKey();
  return; // good return
 fail:
  m_YubiSN = Yubi::RetCode2String(rc).c_str();
}

int CYubiCfgDlg::WriteYubiSK()
{
  CYkLib yk;
  BYTE buffer[128];
  YKLIB_RC rc;
  STATUS status;
  CONFIG config;

  memset(&status, 0, sizeof(status));
  memset(&config, 0, sizeof(config));
  config.fixedSize = 2;
  config.fixed[0] = 0x47; // ???
  config.fixed[1] = 0x11; // ???
  config.tktFlags = TKTFLAG_APPEND_CR; // ???
  config.extFlags = EXTFLAG_SERIAL_API_VISIBLE; // ???
  yk.setKey160(&config, m_yubi_sk_bin);
  rc = yk.openKey();
  if (rc != YKLIB_OK) goto fail;
  rc = yk.writeConfigBegin(0, &config, NULL);
  if (rc != YKLIB_OK) goto fail;
  // Wait for response completion
  rc = yk.waitForCompletion(YKLIB_MAX_WRITE_WAIT, buffer, sizeof(DWORD));
  if (rc != YKLIB_OK) goto fail;
  rc = yk.closeKey();
 fail:
  return rc;
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
  ReadYubiSN();
  UpdateData(FALSE);
}

void CYubiCfgDlg::yubiRemoved(void)
{
  m_YubiSN = m_YubiSK = _T("");
  UpdateData(FALSE);
  GetDlgItem(IDC_YUBI_SN)->EnableWindow(FALSE);
  GetDlgItem(IDC_YUBI_SK)->EnableWindow(FALSE);
  GetDlgItem(IDC_YUBI_GEN_BN)->EnableWindow(FALSE);
}

void CYubiCfgDlg::yubiCompleted(ycRETCODE )
{
}

void CYubiCfgDlg::yubiWait(WORD)
{
}

// CYubiCfgDlg message handlers


void CYubiCfgDlg::OnYubiGenBn()
{
  pws_os::GetRandomData(m_yubi_sk_bin, YUBI_SK_LEN);
  StringX str = BinSK2HexStr(m_yubi_sk_bin, YUBI_SK_LEN);
  m_YubiSK = str.c_str();
  m_generated = true; // tell OnBnClickedOk that we have work to do.
  UpdateData(FALSE);
}


void CYubiCfgDlg::OnBnClickedOk()
{
  UpdateData(TRUE);  
  StringX skStr = m_YubiSK;
  
  if (!skStr.empty()) {
    StringX oldSK = BinSK2HexStr(m_yubi_sk_bin, YUBI_SK_LEN);
    if (m_generated) {
      int rc;
      if ((rc = WriteYubiSK()) == YKLIB_OK) { // 1. Update SK on Yubi.
        // 2. If YubiKey update succeeds, update in core.
        m_core.SetYubiSK(m_yubi_sk_bin);
        // 3. Write DB ASAP!
        // XXX TBD
      } else {
        CString err = _T("Failed to update YubiKey: ");
        err += Yubi::RetCode2String(rc).c_str();
        GetDlgItem(IDC_YUBI_API)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_YUBI_API)->SetWindowText(err);
        return; // don't close dbox
      }
    }
  }
  CPWDialog::OnOK();
}
