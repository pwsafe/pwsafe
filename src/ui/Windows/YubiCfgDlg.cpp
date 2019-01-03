/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "YubiCfgDlg.h"
#include "PKBaseDlg.h" // for *YubiExists

#include "os/windows/yubi/YkLib.h"
#include "core/StringX.h"
#include "core/PWScore.h"

#include "os/rand.h"

using namespace std;

// CYubiCfgDlg dialog

static const wchar_t PSSWDCHAR = L'*';

CYubiCfgDlg::CYubiCfgDlg(CWnd* pParent, PWScore &core)
  : CPWDialog(CYubiCfgDlg::IDD, pParent), m_core(core),
    m_YubiSN(L""), m_YubiSK(L""), m_isSKHidden(true)
{
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer event
}

CYubiCfgDlg::~CYubiCfgDlg()
{
}

void CYubiCfgDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    m_ex_YubiSK.DoDDX(pDX, m_YubiSK);
    DDX_Text(pDX, IDC_YUBI_SN, m_YubiSN);
    DDX_Control(pDX, IDC_YUBI_SK, m_ex_YubiSK);
}

static StringX BinSN2Str(const unsigned char *snstr)
{
  unsigned int sn = 0;
  sn = (snstr[0] << 24) | (snstr[1] << 16);
  sn |= (snstr[2] << 8) | snstr[3];
  wostringstream os;
  os << sn;
  return StringX(os.str().c_str());
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
  if (CPWDialog::OnInitDialog() == TRUE) {
    SetTimer(1, 250, 0); // Setup a timer to poll the key every 250 ms
    HideSK();
    return TRUE;
  } else
    return FALSE;
}

void CYubiCfgDlg::ReadYubiSN()
{
  CYkLib yk;
  BYTE buffer[128];
  YKLIB_RC rc;
  STATUS status;
  CSingleLock singeLock(&m_mutex);

  memset(&status, 0, sizeof(status));
  singeLock.Lock();
  rc = yk.openKey();
  if (rc != YKLIB_OK) goto fail;
  rc = yk.readSerialBegin();
  if (rc != YKLIB_OK) goto fail;
  // Wait for response completion
  rc = yk.waitForCompletion(YKLIB_MAX_SERIAL_WAIT, buffer, sizeof(DWORD));
  if (rc != YKLIB_OK) goto fail;
  m_YubiSN = BinSN2Str(buffer).c_str();
  rc = yk.closeKey();
  return; // good return
 fail:
  m_YubiSN = L"Error reading YubiKey";
}

int CYubiCfgDlg::WriteYubiSK(const unsigned char *yubi_sk_bin)
{
  CYkLib yk;
  YKLIB_RC rc;
  STATUS status;
  CONFIG config;
  CSingleLock singeLock(&m_mutex);

  memset(&status, 0, sizeof(status));
  memset(&config, 0, sizeof(config));
  config.tktFlags = TKTFLAG_CHAL_RESP;
  config.cfgFlags = CFGFLAG_CHAL_HMAC | CFGFLAG_HMAC_LT64 | CFGFLAG_CHAL_BTN_TRIG;
  config.extFlags = EXTFLAG_SERIAL_API_VISIBLE;
  yk.setKey160(&config, yubi_sk_bin);
  singeLock.Lock();
  rc = yk.openKey();
  if (rc != YKLIB_OK) goto fail;
  rc = yk.writeConfigBegin(1, &config, NULL);
  if (rc != YKLIB_OK) goto fail;
  // Wait for response completion
  rc = yk.waitForCompletion(YKLIB_MAX_WRITE_WAIT);
  if (rc != YKLIB_OK) goto fail;
 fail:
  return rc;
}

BEGIN_MESSAGE_MAP(CYubiCfgDlg, CPWDialog)
    ON_BN_CLICKED(IDC_YUBI_GEN_BN, &CYubiCfgDlg::OnYubiGenBn)
    ON_BN_CLICKED(IDOK, &CYubiCfgDlg::OnBnClickedOk)
    ON_BN_CLICKED(ID_HELP, &CYubiCfgDlg::OnHelp)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_YUBI_SHOW_HIDE, &CYubiCfgDlg::OnBnClickedYubiShowHide)
END_MESSAGE_MAP()

void CYubiCfgDlg::yubiInserted(void)
{
  GetDlgItem(IDC_YUBI_SN)->EnableWindow(TRUE);
  GetDlgItem(IDC_YUBI_SK)->EnableWindow(TRUE);
  GetDlgItem(IDC_YUBI_GEN_BN)->EnableWindow(TRUE);
  GetDlgItem(IDOK)->EnableWindow(TRUE);
  if (m_core.GetYubiSK() != NULL) {
    HideSK();
    m_YubiSK = BinSK2HexStr(m_core.GetYubiSK(), YUBI_SK_LEN).c_str();
  } else 
    m_YubiSK = L"";
  ReadYubiSN();
  GetDlgItem(IDC_YUBI_SN)->SetWindowText(m_YubiSN);
  UpdateData(FALSE);
}

void CYubiCfgDlg::yubiRemoved(void)
{
  m_YubiSN = L"";
  m_YubiSK = CSecString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT));
  ShowSK();
  UpdateData(FALSE);
  GetDlgItem(IDC_YUBI_SN)->EnableWindow(FALSE);
  GetDlgItem(IDC_YUBI_SK)->EnableWindow(FALSE);
  GetDlgItem(IDC_YUBI_GEN_BN)->EnableWindow(FALSE);
  GetDlgItem(IDOK)->EnableWindow(FALSE);
}

// CYubiCfgDlg message handlers

void CYubiCfgDlg::OnYubiGenBn()
{
  unsigned char yubi_sk_bin[YUBI_SK_LEN];
  pws_os::GetRandomData(yubi_sk_bin, YUBI_SK_LEN);
  m_YubiSK = BinSK2HexStr(yubi_sk_bin, YUBI_SK_LEN).c_str();
  trashMemory(yubi_sk_bin, YUBI_SK_LEN);
  UpdateData(FALSE);
}

void CYubiCfgDlg::OnBnClickedOk()
{
  // Was OK button, now "Set Yubikey" so we don't close
  // after processing.
  UpdateData(TRUE);  
  StringX skStr = LPCWSTR(m_YubiSK);
  
  GetDlgItem(IDC_YUBI_API)->ShowWindow(SW_HIDE); // in case of retry
  if (!skStr.empty()) {
    unsigned char yubi_sk_bin[YUBI_SK_LEN];
    HexStr2BinSK(skStr, yubi_sk_bin, YUBI_SK_LEN);

    if (WriteYubiSK(yubi_sk_bin) == YKLIB_OK) {
      // 1. Update SK on Yubi.

      // 2. If YubiKey update succeeds, update in core.
      m_core.SetYubiSK(yubi_sk_bin);

      // 3. Write DB ASAP!
      int rc = m_core.WriteCurFile();
      if (rc == PWScore::SUCCESS)
        GetMainDlg()->BlockLogoffShutdown(false);

      trashMemory(yubi_sk_bin, YUBI_SK_LEN);
    } else {
      const CString err = L"Failed to update YubiKey";
      GetDlgItem(IDC_YUBI_API)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_YUBI_API)->SetWindowText(err);
    }
  }
}

bool CYubiCfgDlg::IsYubiInserted() const
{
  CSingleLock singeLock(&m_mutex);
  CYkLib yk;
  singeLock.Lock();
  return (yk.enumPorts() == 1);
}

void CYubiCfgDlg::OnTimer(UINT_PTR)
{
  // If an operation is pending, check if it has completed

  // No HMAC operation is pending - check if one and only one key is present
  bool inserted = IsYubiInserted();
  // call relevant callback if something's changed
  if (inserted != m_present) {
    m_present = inserted;
    if (m_present) {
      GetDlgItem(IDC_YUBI_API)->ShowWindow(SW_HIDE);
      CPKBaseDlg::SetYubiExists();
      yubiInserted();
    } else {
      yubiRemoved();
    }
  }
}

void CYubiCfgDlg::OnHelp() 
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/manage_menu.html#yubikey";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CYubiCfgDlg::ShowSK()
{
  m_isSKHidden = false;
  GetDlgItem(IDC_YUBI_SHOW_HIDE)->
    SetWindowText(CString(MAKEINTRESOURCE(IDS_HIDEPASSWORDTXT)));
  m_ex_YubiSK.SetSecure(false);

  // Remove password character so that the password is displayed
  m_ex_YubiSK.SetPasswordChar(0);
  m_ex_YubiSK.Invalidate();
}

void CYubiCfgDlg::HideSK()
{
  m_isSKHidden = true;
  GetDlgItem(IDC_YUBI_SHOW_HIDE)->
    SetWindowText(CString(MAKEINTRESOURCE(IDS_SHOWPASSWORDTXT)));
  m_ex_YubiSK.SetSecure(true);

  // Set password character so that the password is not displayed
  m_ex_YubiSK.SetPasswordChar(PSSWDCHAR);
  m_ex_YubiSK.Invalidate();
}

void CYubiCfgDlg::OnBnClickedYubiShowHide()
{
  UpdateData(TRUE);

  if (m_isSKHidden) {
    ShowSK();
  } else {
    HideSK();
  }
  UpdateData(FALSE);
}
