/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file YubiKeyDlg.cpp
//

#include "stdafx.h"
#include "PasswordSafe.h" // for extern app.
#include "ThisMfcApp.h" // ditto
#include "YubiKeyDlg.h"
#include "corelib/Util.h"

// YubiKeyDlg dialog

IMPLEMENT_DYNAMIC(CYubiKeyDlg, CPWDialog)

CYubiKeyDlg::CYubiKeyDlg(CWnd* pParent,
                         const StringX &yPubID,
                         unsigned int yapiID,
                         const YubiApiKey_t &yapiKey)
: CPWDialog(CYubiKeyDlg::IDD, pParent), m_YKpubID(yPubID.c_str()),
  m_apiID(yapiID),
  m_YKinfo(_T("")), m_otp(_T("")), m_YKstatus(_T("")), m_apiKeyStr(_T(""))
  , m_ykEnabled(FALSE)
{
  memcpy(m_apiKey, yapiKey, sizeof(yapiKey));
  m_ykEnabled = !m_YKpubID.IsEmpty();
  if (m_ykEnabled) { // if we've a pub ID, we also have a key - show it
    m_apiKeyStr = PWSUtil::Base64Encode(m_apiKey, sizeof(m_apiKey)).c_str();
  }
}

CYubiKeyDlg::~CYubiKeyDlg()
{
}

BOOL CYubiKeyDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();
  if (!m_ykEnabled) {
    m_YKinfo = _T("To enable YubiKey support, please ")
      _T("click on the checkbox and fill in the\r\nfollowing fields")
      _T(" (You can get a Client ID and API Key from ")
      _T("<a href=\"https://api.yubico.com/yms/getapi.php\">")
      _T("\r\nhttps://api.yubico.com/yms/getapi.php</a>):");
  } else {
    m_YKinfo = _T("This database is associated with YubiKey \"");
    m_YKinfo += m_YKpubID;
    m_YKinfo += _T("\"\r\nTo disable YubiKey support, please clear the checkbox");
  }
  m_YKinfoCtrl.SetWindowText(m_YKinfo);

  GetDlgItem(IDC_YK_ID)->EnableWindow(m_ykEnabled);
  GetDlgItem(IDC_YK_KEY)->EnableWindow(m_ykEnabled);
  GetDlgItem(IDC_YK_OTP)->EnableWindow(m_ykEnabled);

  unsigned char hasKey = 0;
  for (size_t i = 0; i < sizeof(m_apiKey); i++) hasKey |= m_apiKey[i];
  if (hasKey != 0) {
    stringT keystr = PWSUtil::Base64Encode((const BYTE *)m_apiKey,
                                           sizeof(m_apiKey));
    m_apiKeyStr = keystr.c_str();
  }
  return TRUE;
}

void CYubiKeyDlg::DoDataExchange(CDataExchange* pDX)
{
   CPWDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_YK_INFO, m_YKinfoCtrl);
   DDX_Text(pDX, IDC_YK_ID, m_apiID);
   DDX_Text(pDX, IDC_YK_OTP, m_otp);
   DDV_MaxChars(pDX, m_otp, 44);
   DDX_Text(pDX, IDC_YK_STATUS, m_YKstatus);
   DDX_Text(pDX, IDC_YK_KEY, m_apiKeyStr);
   DDV_MaxChars(pDX, m_apiKeyStr, 28);
   if (pDX->m_bSaveAndValidate) { // dbox to data
      if (m_apiKeyStr.GetLength() == 28) {
         StringX keystr(m_apiKeyStr);
         size_t keylen;
         BYTE *key = (BYTE *)m_apiKey;
         PWSUtil::Base64Decode(keystr, key, keylen);
      }
   }
   DDX_Check(pDX, IDC_YK_ENABLED, m_ykEnabled);
   DDX_Control(pDX, IDC_YK_INFO, m_YKinfoCtrl);
}


BEGIN_MESSAGE_MAP(CYubiKeyDlg, CPWDialog)
   ON_BN_CLICKED(IDOK, &CYubiKeyDlg::OnOk)
  ON_BN_CLICKED(IDHELP, &CYubiKeyDlg::OnHelp)
  ON_BN_CLICKED(IDC_YK_ENABLED, &CYubiKeyDlg::OnBnClickedYkEnabled)
END_MESSAGE_MAP()


// CYubiKeyDlg message handlers

void CYubiKeyDlg::OnOk()
{
  // validate OTP, blablabla
  UpdateData(TRUE); // get data from control
  if (m_ykEnabled) { 
    // if required fields empty, complain
    if (m_apiID == 0 || m_apiKeyStr.IsEmpty()) {
      CString errmess(MAKEINTRESOURCE(IDS_YK_MISSING));
      m_YKstatus = errmess;
      UpdateData(FALSE);
      return; // don't close dbox
    }
    if (VerifyOTP(m_YKstatus)) { // an empty otp will make this fail
      m_YKpubID = m_otp.Left(12);
      CPWDialog::OnOK();
    } else { // verify failed, show why
      UpdateData(FALSE);
      return; // leaves dbox open
    } // VerifyOTP()
  } else { // !m_ykEnabled
    m_YKpubID = _T(""); // this tells the caller that we're
    //                     no longer interested in authentication
    CPWDialog::OnOK();
  }
}

bool CYubiKeyDlg::VerifyOTP(CString &error)
{
  YubiKeyAuthenticator yka(m_apiID, m_apiKey);

  if (!yka.VerifyOTP(LPCTSTR(m_otp))) {
    error = yka.GetError().c_str();
    return false;
  } else
    return true;
}

void CYubiKeyDlg::OnHelp() 
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/yubikey.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CYubiKeyDlg::OnBnClickedYkEnabled()
{
  UpdateData(TRUE); // get data from control
  if (!m_ykEnabled) { // if checkbox cleared, clear data
    m_apiID = 0;
    m_apiKeyStr = _T("");
    m_otp = _T("");
    UpdateData(FALSE);
  }
  GetDlgItem(IDC_YK_ID)->EnableWindow(m_ykEnabled);
  GetDlgItem(IDC_YK_KEY)->EnableWindow(m_ykEnabled);
  GetDlgItem(IDC_YK_OTP)->EnableWindow(m_ykEnabled);
}

