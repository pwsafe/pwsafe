/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsPasswordPolicy.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"

#include "core/PWCharPool.h"
#include "core/PwsPlatform.h"
#include "core/PWSprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "OptionsPasswordPolicy.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy property page

IMPLEMENT_DYNCREATE(COptionsPasswordPolicy, COptions_PropertyPage)

const UINT COptionsPasswordPolicy::nonHex[COptionsPasswordPolicy::N_NOHEX] = {
  IDC_USELOWERCASE, IDC_USEUPPERCASE, IDC_USEDIGITS,
  IDC_USESYMBOLS, IDC_EASYVISION, IDC_PRONOUNCEABLE};

const UINT COptionsPasswordPolicy::LenTxts[COptionsPasswordPolicy::N_HEX_LENGTHS * 2] = {
  IDC_STATIC_LC1, IDC_STATIC_LC2, IDC_STATIC_UC1, IDC_STATIC_UC2,
  IDC_STATIC_DG1, IDC_STATIC_DG2, IDC_STATIC_SY1, IDC_STATIC_SY2};

const UINT COptionsPasswordPolicy::nonHexLengths[COptionsPasswordPolicy::N_HEX_LENGTHS] = {
  IDC_MINLOWERLENGTH, IDC_MINUPPERLENGTH, IDC_MINDIGITLENGTH, IDC_MINSYMBOLLENGTH};

const UINT COptionsPasswordPolicy::nonHexLengthSpins[COptionsPasswordPolicy::N_HEX_LENGTHS] = {
  IDC_SPINLOWERCASE, IDC_SPINUPPERCASE, IDC_SPINDIGITS, IDC_SPINSYMBOLS};

COptionsPasswordPolicy::COptionsPasswordPolicy(bool bFromOptions)
  : COptions_PropertyPage(COptionsPasswordPolicy::IDD), m_bFromOptions(bFromOptions),
  m_pDbx(NULL), m_password(L"")
{
}

COptionsPasswordPolicy::~COptionsPasswordPolicy()
{
}

void COptionsPasswordPolicy::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsPasswordPolicy)
  DDX_Text(pDX, IDC_DEFPWLENGTH, m_pwdefaultlength);
  DDX_Text(pDX, IDC_MINDIGITLENGTH, m_pwdigitminlength);
  DDX_Text(pDX, IDC_MINLOWERLENGTH, m_pwlowerminlength);
  DDX_Text(pDX, IDC_MINSYMBOLLENGTH, m_pwsymbolminlength);
  DDX_Text(pDX, IDC_MINUPPERLENGTH, m_pwupperminlength);
  DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
  DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
  DDX_Check(pDX, IDC_USEDIGITS, m_pwusedigits);
  DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
  DDX_Check(pDX, IDC_EASYVISION, m_pweasyvision);
  DDX_Check(pDX, IDC_USEHEXDIGITS, m_pwusehexdigits);
  DDX_Check(pDX, IDC_PRONOUNCEABLE, m_pwmakepronounceable);
  
  DDX_Radio(pDX, IDC_USEDEFAULTSYMBOLS, m_useownsymbols);
  DDX_Control(pDX, IDC_OWNSYMBOLS, (CEdit&)m_symbols);

  // Because we can show the generated password when used from Mangage->Generate
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsPasswordPolicy, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsPasswordPolicy)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_USEHEXDIGITS, OnUseHexdigits)
  ON_BN_CLICKED(IDC_USELOWERCASE, OnUseLowerCase)
  ON_BN_CLICKED(IDC_USEUPPERCASE, OnUseUpperCase)
  ON_BN_CLICKED(IDC_USEDIGITS, OnUseDigits)
  ON_BN_CLICKED(IDC_USESYMBOLS, OnUseSymbols)
  ON_BN_CLICKED(IDC_EASYVISION, OnEasyVision)
  ON_BN_CLICKED(IDC_PRONOUNCEABLE, OnMakePronounceable)

  // Because we can show the generated password when used from Mangage->Generate
  ON_BN_CLICKED(IDC_RANDOM, OnRandom)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopyPassword)
  ON_EN_CHANGE(IDC_PASSWORD, OnENChangePassword)

  ON_BN_CLICKED(IDC_USEDEFAULTSYMBOLS, OnSymbols)
  ON_BN_CLICKED(IDC_USEOWNSYMBOLS, OnSymbols)

  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy message handlers

BOOL COptionsPasswordPolicy::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsPasswordPolicy::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/password_policies.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

BOOL COptionsPasswordPolicy::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  if (m_bFromOptions) {
    // These are only used in Manage -> Generate Password
    GetDlgItem(IDC_RANDOM)->EnableWindow(FALSE);
    GetDlgItem(IDC_RANDOM)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_COPYPASSWORD)->EnableWindow(FALSE);
    GetDlgItem(IDC_COPYPASSWORD)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
    GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_HIDE);
  } else {
    // Centre the OK button & change its text
    RECT rc, rcOK;
    CWnd *pOK = m_options_psh->GetDlgItem(IDOK);

    m_options_psh->GetClientRect(&rc);
    pOK->GetWindowRect(&rcOK);
    m_options_psh->ScreenToClient(&rcOK);
    int top = rcOK.top;
    pOK->GetClientRect(&rcOK);
    int left = (rc.right - rcOK.right) / 2;
    pOK->MoveWindow(left, top, rcOK.right, rcOK.bottom);

    CString cs_close(MAKEINTRESOURCE(IDS_CLOSE));
    pOK->SetWindowText(cs_close);

    // Hide the Cancel button
    m_options_psh->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    m_options_psh->GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

    ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
    m_ex_password.SetSecure(false);
    // Remove password character so that the password is displayed
    m_ex_password.SetPasswordChar(0);
  }

  CSpinButtonCtrl *pspin  = (CSpinButtonCtrl *)GetDlgItem(IDC_PWLENSPIN);
  CSpinButtonCtrl *pspinD = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINDIGITS);
  CSpinButtonCtrl *pspinL = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINLOWERCASE);
  CSpinButtonCtrl *pspinS = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINSYMBOLS);
  CSpinButtonCtrl *pspinU = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINUPPERCASE);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
  pspin->SetRange(4, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwdefaultlength);

  pspinD->SetBuddy(GetDlgItem(IDC_MINDIGITLENGTH));
  pspinD->SetRange(0, 1024);
  pspinD->SetBase(10);
  pspinD->SetPos(m_pwdigitminlength);

  pspinL->SetBuddy(GetDlgItem(IDC_MINLOWERLENGTH));
  pspinL->SetRange(0, 1024);
  pspinL->SetBase(10);
  pspinL->SetPos(m_pwlowerminlength);

  pspinS->SetBuddy(GetDlgItem(IDC_MINSYMBOLLENGTH));
  pspinS->SetRange(0, 1024);
  pspinS->SetBase(10);
  pspinS->SetPos(m_pwsymbolminlength);

  pspinU->SetBuddy(GetDlgItem(IDC_MINUPPERLENGTH));
  pspinU->SetRange(0, 1024);
  pspinU->SetBase(10);
  pspinU->SetPos(m_pwupperminlength);

  m_save[SAVE_LOWERCASE] = m_pwuselowercase;
  m_save[SAVE_UPPERCASE] = m_pwuseuppercase;
  m_save[SAVE_DIGITS] = m_pwusedigits;
  m_save[SAVE_SYMBOLS] = m_pwusesymbols;
  m_save[SAVE_EASYVISION] = m_pweasyvision;
  m_save[SAVE_PRONOUNCEABLE] = m_pwmakepronounceable;

  if (m_pwuselowercase == TRUE && m_pwlowerminlength == 0)
    m_pwlowerminlength = 1;
  if (m_pwuseuppercase == TRUE && m_pwupperminlength == 0)
    m_pwupperminlength = 1;
  if (m_pwusedigits == TRUE && m_pwdigitminlength == 0)
    m_pwdigitminlength = 1;
  if (m_pwusesymbols == TRUE && m_pwsymbolminlength == 0)
    m_pwsymbolminlength = 1;

  m_savelen[SAVE_LOWERCASE] = m_pwlowerminlength;
  m_savelen[SAVE_UPPERCASE] = m_pwupperminlength;
  m_savelen[SAVE_DIGITS] = m_pwdigitminlength;
  m_savelen[SAVE_SYMBOLS] = m_pwsymbolminlength;

  m_savepwdefaultlength = m_pwdefaultlength;
  m_savepwuselowercase = m_pwuselowercase;
  m_savepwuseuppercase = m_pwuseuppercase;
  m_savepwusedigits = m_pwusedigits;
  m_savepwusesymbols = m_pwusesymbols;
  m_savepweasyvision = m_pweasyvision;
  m_savepwusehexdigits = m_pwusehexdigits;
  m_savepwmakepronounceable = m_pwmakepronounceable;
  m_savepwdigitminlength = m_pwdigitminlength;
  m_savepwlowerminlength = m_pwlowerminlength;
  m_savepwsymbolminlength = m_pwsymbolminlength;
  m_savepwupperminlength = m_pwupperminlength;

  // Set up the correct controls (enabled/disabled)
  do_hex(m_pwusehexdigits == TRUE);
  do_easyorpronounceable(m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE);

  // Setup symbols
  m_saveuseownsymbols = m_useownsymbols;
  m_cs_savesymbols = m_cs_symbols;

  stringT st_symbols;
  CPasswordCharPool::GetDefaultSymbols(st_symbols);

  GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->SetWindowText(st_symbols.c_str());
  m_symbols.SetWindowText(m_cs_symbols);

  GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(m_pwusesymbols);
  GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(m_pwusesymbols);
  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((m_pwusesymbols == TRUE && m_useownsymbols == OWN_SYMBOLS) ? TRUE : FALSE);

  return TRUE;
}

void COptionsPasswordPolicy::do_hex(const bool bHex)
{
  CString cs_value;
  int i;
  if (bHex) {
    // true means enable hex, save state for possible re-enable
    // Disable non-hex controls
    for (i = 0; i < N_NOHEX; i++) {
      UINT id = nonHex[i];
      m_save[i] = ((CButton*)GetDlgItem(id))->GetCheck();
      ((CButton*)GetDlgItem(id))->SetCheck(BST_UNCHECKED);
      GetDlgItem(id)->EnableWindow(FALSE);
    }

    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      GetDlgItem(id)->SetWindowText(L"0");
      GetDlgItem(id)->EnableWindow(FALSE);

      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(FALSE);
    }

    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(FALSE);

    m_savelen[SAVE_LOWERCASE] = m_pwlowerminlength;
    m_savelen[SAVE_UPPERCASE] = m_pwupperminlength;
    m_savelen[SAVE_DIGITS] = m_pwdigitminlength;
    m_savelen[SAVE_SYMBOLS] = m_pwsymbolminlength;
  } else {
    // non-hex, restore state
    // Enable non-hex controls and restore checked state
    for (i = 0; i < N_NOHEX; i++) {
      UINT id = nonHex[i];
      GetDlgItem(id)->EnableWindow(TRUE);
      ((CButton*)GetDlgItem(id))->SetCheck(m_save[i]);
    }

    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      cs_value.Format(L"%d", m_savelen[i]);
      GetDlgItem(id)->SetWindowText(cs_value);
      GetDlgItem(id)->EnableWindow(m_save[i]);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(m_save[i]);
    }

    BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;
    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((bEnable == TRUE && m_useownsymbols == OWN_SYMBOLS) ?
                                             TRUE : FALSE);

    m_pwlowerminlength = m_savelen[SAVE_LOWERCASE];
    m_pwupperminlength = m_savelen[SAVE_UPPERCASE];
    m_pwdigitminlength = m_savelen[SAVE_DIGITS];
    m_pwsymbolminlength = m_savelen[SAVE_SYMBOLS];
  }
}

void COptionsPasswordPolicy::do_easyorpronounceable(const bool bSet)
{
  // Can't have minimum lengths!
  if ((m_pweasyvision == TRUE  || m_pwmakepronounceable == TRUE) &&
      (m_pwdigitminlength > 1  || m_pwlowerminlength > 1 || 
       m_pwsymbolminlength > 1 || m_pwupperminlength > 1)) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTSPECIFYMINNUMBER);
  }

  CString cs_value;
  int i;
  if (bSet) {
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->ShowWindow(SW_HIDE);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[2*i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[2*i + 1])->ShowWindow(SW_HIDE);
    }

    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow((IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED) ? TRUE : FALSE);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(FALSE);

    m_savelen[SAVE_LOWERCASE] = m_pwlowerminlength;
    m_savelen[SAVE_UPPERCASE] = m_pwupperminlength;
    m_savelen[SAVE_DIGITS] = m_pwdigitminlength;
    m_savelen[SAVE_SYMBOLS] = m_pwsymbolminlength;
  } else {
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->ShowWindow(SW_SHOW);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(SW_SHOW);
      GetDlgItem(LenTxts[2 * i])->ShowWindow(SW_SHOW);
      GetDlgItem(LenTxts[2 * i + 1])->ShowWindow(SW_SHOW);
    }

    BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED) ? TRUE : FALSE;
    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((bEnable == TRUE && m_useownsymbols == OWN_SYMBOLS) ?
                                             TRUE : FALSE);

    m_pwlowerminlength = m_savelen[SAVE_LOWERCASE];
    m_pwupperminlength = m_savelen[SAVE_UPPERCASE];
    m_pwdigitminlength = m_savelen[SAVE_DIGITS];
    m_pwsymbolminlength = m_savelen[SAVE_SYMBOLS];
  }
}

void COptionsPasswordPolicy::OnUseLowerCase() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USELOWERCASE) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINLOWERLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINLOWERCASE)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_LC1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_LC2)->EnableWindow(bChecked);
  m_pwlowerminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUseUpperCase() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USEUPPERCASE) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINUPPERLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINUPPERCASE)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_UC1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_UC2)->EnableWindow(bChecked);
  m_pwupperminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUseDigits() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USEDIGITS) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINDIGITLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINDIGITS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DG1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DG2)->EnableWindow(bChecked);
  m_pwdigitminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUseSymbols() 
{
  UpdateData(TRUE);

  BOOL bChecked = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;

  GetDlgItem(IDC_MINSYMBOLLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_SY1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_SY2)->EnableWindow(bChecked);

  GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow((IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED) ? TRUE : FALSE);
  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((bChecked == TRUE && m_useownsymbols == OWN_SYMBOLS) ? TRUE : FALSE);

  m_pwsymbolminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUseHexdigits() 
{
  UpdateData(TRUE);
  do_hex(IsDlgButtonChecked(IDC_USEHEXDIGITS) == BST_CHECKED);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_hex" will be undone
}

void COptionsPasswordPolicy::OnEasyVision() 
{
  UpdateData(TRUE);

  if (m_pweasyvision && m_pwmakepronounceable) {
    CGeneralMsgBox gmb;
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_pweasyvision = FALSE;
  }

  const bool bChecked = (IsDlgButtonChecked(IDC_EASYVISION) == BST_CHECKED);

  do_easyorpronounceable(bChecked);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

void COptionsPasswordPolicy::OnMakePronounceable() 
{
  UpdateData(TRUE);

  if (m_pweasyvision && m_pwmakepronounceable) {
    CGeneralMsgBox gmb;
    ((CButton*)GetDlgItem(IDC_PRONOUNCEABLE))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_pwmakepronounceable = FALSE;
  }

  const bool bChecked = (IsDlgButtonChecked(IDC_PRONOUNCEABLE) == BST_CHECKED);

  do_easyorpronounceable(bChecked);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

void COptionsPasswordPolicy::OnSymbols()
{
  m_useownsymbols = ((CButton *)GetDlgItem(IDC_USEDEFAULTSYMBOLS))->GetCheck() == BST_CHECKED ?
                        DEFAULT_SYMBOLS : OWN_SYMBOLS;

  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(m_useownsymbols == DEFAULT_SYMBOLS ? FALSE : TRUE);
  if (m_useownsymbols == OWN_SYMBOLS)
    GetDlgItem(IDC_OWNSYMBOLS)->SetFocus();
}

BOOL COptionsPasswordPolicy::Validate()
{
  CGeneralMsgBox gmb;
  // Check that options, as set, are valid.
  if (m_pwusehexdigits &&
     (m_pwuselowercase || m_pwuseuppercase || m_pwusedigits ||
      m_pwusesymbols || m_pweasyvision || m_pwmakepronounceable)) {
    gmb.AfxMessageBox(IDS_HEXMUTUALLYEXCL);
    return FALSE;
  }

  if (m_pwusehexdigits) {
    if (m_pwdefaultlength % 2 != 0) {
      gmb.AfxMessageBox(IDS_HEXMUSTBEEVEN);
      return FALSE;
    }
  } else
  if (!m_pwuselowercase && !m_pwuseuppercase &&
      !m_pwusedigits && !m_pwusesymbols) {
    gmb.AfxMessageBox(IDS_MUSTHAVEONEOPTION);
    return FALSE;
  }

  if ((m_pwdefaultlength < 4) || (m_pwdefaultlength > 1024)) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTH);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if (!(m_pwusehexdigits || m_pweasyvision || m_pwmakepronounceable) &&
      (m_pwdigitminlength + m_pwlowerminlength + 
       m_pwsymbolminlength + m_pwupperminlength) > m_pwdefaultlength) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTHTOOSMALL);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if ((m_pwusehexdigits || m_pweasyvision || m_pwmakepronounceable))
    m_pwdigitminlength = m_pwlowerminlength = 
       m_pwsymbolminlength = m_pwupperminlength = 1;
  //End check
  return TRUE;
}

BOOL COptionsPasswordPolicy::OnKillActive()
{
  if (Validate() == FALSE)
    return FALSE;

  return COptions_PropertyPage::OnKillActive();
}

LRESULT COptionsPasswordPolicy::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  if (!m_bFromOptions)
    return 0L;

  if (UpdateData(TRUE) == FALSE)
    return 1L;

  if (wParam == PP_UPDATE_VARIABLES && Validate() == FALSE)
    return 1L;

  // Get symbol string from Edit control
  m_symbols.GetWindowText(m_cs_symbols);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (m_savepwdefaultlength     != m_pwdefaultlength     ||
          m_savepwuselowercase      != m_pwuselowercase      ||
          (m_pwuselowercase         == TRUE &&
           m_savepwlowerminlength   != m_pwlowerminlength)   ||
          m_savepwuseuppercase      != m_pwuseuppercase      ||
          (m_pwuseuppercase         == TRUE &&
           m_savepwupperminlength   != m_pwupperminlength)   ||
          m_savepwusedigits         != m_pwusedigits         ||
          (m_pwusedigits            == TRUE &&
           m_savepwdigitminlength   != m_pwdigitminlength)   ||
          m_savepwusesymbols        != m_pwusesymbols        ||
          (m_pwusesymbols           == TRUE &&
           m_savepwsymbolminlength  != m_pwsymbolminlength)  ||
          m_savepweasyvision        != m_pweasyvision        ||
          m_savepwusehexdigits      != m_pwusehexdigits      ||
          m_savepwmakepronounceable != m_pwmakepronounceable ||
          m_saveuseownsymbols       != m_useownsymbols    ||
          (m_useownsymbols          == OWN_SYMBOLS &&
          m_cs_savesymbols          != m_cs_symbols))
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

void COptionsPasswordPolicy::OnRandom()
{
  UpdateData(TRUE);

  // Use common validation
  if (Validate() == FALSE)
    return;

  PWPolicy pwp;

  pwp.Empty();
  if (m_pwuselowercase == TRUE)
    pwp.flags |= PWSprefs::PWPolicyUseLowercase;
  if (m_pwuseuppercase == TRUE)
    pwp.flags |= PWSprefs::PWPolicyUseUppercase;
  if (m_pwusedigits == TRUE)
    pwp.flags |= PWSprefs::PWPolicyUseDigits;
  if (m_pwusesymbols == TRUE)
    pwp.flags |= PWSprefs::PWPolicyUseSymbols;
  if (m_pwusehexdigits == TRUE)
    pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
  if (m_pweasyvision == TRUE)
    pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
  if (m_pwmakepronounceable == TRUE)
    pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
  pwp.length = m_pwdefaultlength;
  pwp.digitminlength = m_pwdigitminlength;
  pwp.lowerminlength = m_pwlowerminlength;
  pwp.symbolminlength = m_pwsymbolminlength;
  pwp.upperminlength = m_pwupperminlength;
  
  StringX passwd;
  stringT st_symbols;
  if (m_useownsymbols == OWN_SYMBOLS) {
    m_symbols.GetWindowText(m_cs_symbols);
    st_symbols = LPCWSTR(m_cs_symbols);
  } else
    CPasswordCharPool::GetDefaultSymbols(st_symbols);

  m_pDbx->MakeRandomPassword(passwd, pwp, st_symbols.c_str(), false);
  m_password = passwd.c_str();
  m_ex_password.SetWindowText(m_password);
  m_ex_password.Invalidate();
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnCopyPassword()
{
  UpdateData(TRUE);

  m_pDbx->SetClipboardData(m_password);
  m_pDbx->UpdateLastClipboardAction(CItemData::PASSWORD);
}

void COptionsPasswordPolicy::OnENChangePassword()
{
  UpdateData(TRUE);

  m_ex_password.GetWindowText((CString &)m_password);
}
