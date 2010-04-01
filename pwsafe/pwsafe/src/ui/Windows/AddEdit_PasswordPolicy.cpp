/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_PasswordPolicy.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"    // For Help
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "AddEdit_PasswordPolicy.h"
#include "AddEdit_PropertySheet.h"

#include "corelib/PwsPlatform.h"
#include "corelib/ItemData.h"
#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_PasswordPolicy property page

IMPLEMENT_DYNAMIC(CAddEdit_PasswordPolicy, CAddEdit_PropertyPage)

const UINT CAddEdit_PasswordPolicy::nonHex[CAddEdit_PasswordPolicy::N_NOHEX] = {
  IDC_USELOWERCASE, IDC_USEUPPERCASE, IDC_USEDIGITS,
  IDC_USESYMBOLS, IDC_EASYVISION, IDC_PRONOUNCEABLE};

const UINT CAddEdit_PasswordPolicy::LenTxts[CAddEdit_PasswordPolicy::N_HEX_LENGTHS * 2] = {
  IDC_STATIC_LC1, IDC_STATIC_LC2, IDC_STATIC_UC1, IDC_STATIC_UC2,
  IDC_STATIC_DG1, IDC_STATIC_DG2, IDC_STATIC_SY1, IDC_STATIC_SY2};

const UINT CAddEdit_PasswordPolicy::nonHexLengths[CAddEdit_PasswordPolicy::N_HEX_LENGTHS] = {
 IDC_MINLOWERLENGTH, IDC_MINUPPERLENGTH, IDC_MINDIGITLENGTH, IDC_MINSYMBOLLENGTH};

const UINT CAddEdit_PasswordPolicy::nonHexLengthSpins[CAddEdit_PasswordPolicy::N_HEX_LENGTHS] = {
  IDC_SPINLOWERCASE, IDC_SPINUPPERCASE, IDC_SPINDIGITS, IDC_SPINSYMBOLS};


CAddEdit_PasswordPolicy::CAddEdit_PasswordPolicy(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent,CAddEdit_PasswordPolicy::IDD, pAEMD), m_bInitdone(false)
{
  // We are given the Policy - set Dialog variables
  SetVariablesFromPolicy();

  // Save current status
  for (int i = 0; i < N_HEX_LENGTHS; i++) {
    m_save_visible[i] = true;
    BOOL bEnable(FALSE);
    switch (i) {
      case 0:  // IDC_MINLOWERLENGTH
        bEnable = m_pwuselowercase;
        break;
      case 1:  // IDC_MINUPPERLENGTH
        bEnable = m_pwuseuppercase;
        break;
      case 2:  // IDC_MINDIGITLENGTH
        bEnable = m_pwusedigits;
        break;
      case 3:  // IDC_MINSYMBOLLENGTH
        bEnable =  m_pwusesymbols;
        break;
      default:
        ASSERT(0);
    }
    m_save_enabled[i][0] = m_save_enabled[i][1] = bEnable;
  }
}

CAddEdit_PasswordPolicy::~CAddEdit_PasswordPolicy()
{
}

void CAddEdit_PasswordPolicy::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_PasswordPolicy)
  DDX_Radio(pDX, IDC_USEDEFAULTPWPOLICY, M_ipolicy());

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
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_PasswordPolicy, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_PasswordPolicy)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_USEHEXDIGITS, OnUseHexdigits)
  ON_BN_CLICKED(IDC_USELOWERCASE, OnUseLowerCase)
  ON_BN_CLICKED(IDC_USEUPPERCASE, OnUseUpperCase)
  ON_BN_CLICKED(IDC_USEDIGITS, OnUseDigits)
  ON_BN_CLICKED(IDC_USESYMBOLS, OnUseSymbols)
  ON_BN_CLICKED(IDC_EASYVISION, OnEasyVision)
  ON_BN_CLICKED(IDC_PRONOUNCEABLE, OnMakePronounceable)
  ON_BN_CLICKED(IDC_USEDEFAULTPWPOLICY, OnSetDefaultPWPolicy)
  ON_BN_CLICKED(IDC_ENTRYPWPOLICY, OnSetSpecificPWPolicy)
  ON_BN_CLICKED(IDC_RESETPWPOLICY, OnResetPolicy)

  ON_EN_CHANGE(IDC_DEFPWLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINDIGITLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINLOWERLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINSYMBOLLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINUPPERLENGTH, OnChanged)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_PasswordPolicy message handlers

BOOL CAddEdit_PasswordPolicy::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_PasswordPolicy::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  // Set up spin control relationships
  CSpinButtonCtrl* pspin;

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWLENSPIN);
  pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
  pspin->SetRange(4, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwdefaultlength);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINDIGITS);
  pspin->SetBuddy(GetDlgItem(IDC_MINDIGITLENGTH));
  pspin->SetRange(0, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwdigitminlength);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINLOWERCASE);
  pspin->SetBuddy(GetDlgItem(IDC_MINLOWERLENGTH));
  pspin->SetRange(0, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwlowerminlength);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINSYMBOLS);
  pspin->SetBuddy(GetDlgItem(IDC_MINSYMBOLLENGTH));
  pspin->SetRange(0, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwsymbolminlength);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINUPPERCASE);
  pspin->SetBuddy(GetDlgItem(IDC_MINUPPERLENGTH));
  pspin->SetRange(0, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwupperminlength);

  // Disable controls based on m_ipolicy
  SetPolicyControls();

  UINT uiChk = (M_ipolicy() == CAddEdit_PropertySheet::DEFAULT_POLICY) ?
                IDC_USEDEFAULTPWPOLICY : IDC_ENTRYPWPOLICY;
  ((CButton *)GetDlgItem(uiChk))->SetCheck(BST_CHECKED);

  if (M_uicaller() == IDS_VIEWENTRY) {
    // Disable Buttons not already disabled in SetPolicyControls
    GetDlgItem(IDC_USEDEFAULTPWPOLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRYPWPOLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_PWLEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_OR)->EnableWindow(FALSE);
  }

  if (M_original_entrytype() == CItemData::ET_ALIAS) {
    DisablePolicy();
    GetDlgItem(IDC_USEDEFAULTPWPOLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRYPWPOLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_OR)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_PWLEN)->EnableWindow(FALSE);
  }
  m_bInitdone = true;
  return TRUE;
}

void CAddEdit_PasswordPolicy::OnChanged()
{
  if (!m_bInitdone || m_AEMD.uicaller != IDS_EDITENTRY)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_PasswordPolicy::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/entering_pwd_pp.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

bool CAddEdit_PasswordPolicy::ValidatePolicy(CWnd *&pFocus)
{
  CGeneralMsgBox gmb;
  pFocus = NULL; // caller should set focus to this if non-null
  // Check that options, as set, are valid.

  // Hex is tricky: if user selected it, then other checkboxes are
  // disabled and ignored. But it's possible for the user to set
  // hex and others, so we warn only in the latter case.
  if (m_pwusehexdigits &&
      (GetDlgItem(nonHex[0])->IsWindowEnabled() == TRUE) &&
      (m_pwuselowercase || m_pwuseuppercase || m_pwusedigits ||
       m_pwusesymbols   || m_pweasyvision   || m_pwmakepronounceable)) {
    gmb.AfxMessageBox(IDS_HEXMUTUALLYEXCL);
    pFocus = GetDlgItem(IDC_USEHEXDIGITS);
    return false;
  }
  if (m_pwusehexdigits && (m_pwdefaultlength % 2 != 0)) {
    gmb.AfxMessageBox(IDS_HEXMUSTBEEVEN);
    pFocus = GetDlgItem(IDC_DEFPWLENGTH);
    return false;
  }

  if (!m_pwuselowercase && !m_pwuseuppercase &&
      !m_pwusedigits    && !m_pwusesymbols   && !m_pwusehexdigits) {
    gmb.AfxMessageBox(IDS_MUSTHAVEONEOPTION);
    return false;
  }

  if ((m_pwdefaultlength < 4) || (m_pwdefaultlength > 1024)) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTH);
    pFocus = GetDlgItem(IDC_DEFPWLENGTH);
    return false;
  }

  if (!(m_pwusehexdigits || m_pweasyvision || m_pwmakepronounceable) &&
      (m_pwdigitminlength + m_pwlowerminlength +
       m_pwsymbolminlength + m_pwupperminlength) > m_pwdefaultlength) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTHTOOSMALL);
    pFocus = GetDlgItem(IDC_DEFPWLENGTH);
    return false;
  }

  if ((m_pwusehexdigits || m_pweasyvision || m_pwmakepronounceable))
    m_pwdigitminlength = m_pwlowerminlength =
      m_pwsymbolminlength = m_pwupperminlength = 1;
  return true;
}

BOOL CAddEdit_PasswordPolicy::OnKillActive()
{
  CWnd *pFocus(NULL);
  if (ValidatePolicy(pFocus)) {
    SetPolicyFromVariables();
  } else {
    if (pFocus != NULL)
      pFocus->SetFocus();
    return FALSE;
  }
 
  return CAddEdit_PropertyPage::OnKillActive();
}

LRESULT CAddEdit_PasswordPolicy::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  SetPolicyFromVariables();

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_ipolicy() != M_oldipolicy() ||
         (M_ipolicy() == CAddEdit_PropertySheet::SPECIFIC_POLICY &&
          M_pwp()     != M_oldpwp()))
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
    case PP_UPDATE_PWPOLICY:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL CAddEdit_PasswordPolicy::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY)
    return CAddEdit_PropertyPage::OnApply();

  UpdateData(TRUE);
  CWnd *pFocus(NULL);

  if (M_ipolicy() == CAddEdit_PropertySheet::DEFAULT_POLICY) {
    M_pwp().Empty();
    M_oldipolicy() = M_ipolicy();
    M_oldpwp() = M_pwp();
    return CAddEdit_PropertyPage::OnApply();
  }

  if (ValidatePolicy(pFocus)) {
    SetPolicyFromVariables();
    M_oldipolicy() = M_ipolicy();
    M_oldpwp()= M_pwp();
    return CAddEdit_PropertyPage::OnApply();
  } else {
    // Are we the current page? If not activate this page
    if (m_ae_psh->GetActivePage() != (CAddEdit_PropertyPage *)this)
      m_ae_psh->SetActivePage(this);

    if (pFocus != NULL)
      pFocus->SetFocus();
    return FALSE;
  }
}

void CAddEdit_PasswordPolicy::do_hex(const bool bHex)
{
  // Based on the fact that:
  // BST_UNCHECKED = 0 - which is also FALSE
  // BST_CHECKED   = 1 - which is also TRUE
  CString cs_value;
  int i;
  if (bHex) {
    // true means enable hex, save state for possible re-enable
    // Disable non-hex controls
    for (i = 0; i < N_NOHEX; i++) {
      GetDlgItem(nonHex[i])->EnableWindow(FALSE);
    }
    // Disable lengths
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      m_save_enabled[i][1] = GetDlgItem(id)->IsWindowEnabled();
      GetDlgItem(id)->EnableWindow(FALSE);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(FALSE);
    }
  } else {
    // non-hex, restore state
    // Enable non-hex controls and restore checked state
    for (i = 0; i < N_NOHEX; i++) {
      GetDlgItem(nonHex[i])->EnableWindow(TRUE);
    }
    // Restore lengths
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      GetDlgItem(id)->EnableWindow(m_save_enabled[i][1]);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save_enabled[i][1]);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(m_save_enabled[i][1]);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(m_save_enabled[i][1]);
    }
  }
}

void CAddEdit_PasswordPolicy::do_easyorpronounceable(const bool bSet)
{
  // Can't have any minimum lengths set!
  if ((m_pweasyvision == TRUE  || m_pwmakepronounceable == TRUE) &&
      (m_pwdigitminlength  > 1 || m_pwlowerminlength > 1 ||
       m_pwsymbolminlength > 1 || m_pwupperminlength > 1)) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTSPECIFYMINNUMBER);
  }

  CString cs_value;
  int i;
  if (bSet) {
    // Hide lengths
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      m_save_enabled[i][0] = GetDlgItem(nonHexLengths[i])->IsWindowEnabled();
      m_save_visible[i] = GetDlgItem(nonHexLengths[i])->IsWindowVisible();
      GetDlgItem(nonHexLengths[i])->EnableWindow(FALSE);
      GetDlgItem(nonHexLengths[i])->ShowWindow(SW_HIDE);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2 + 1])->ShowWindow(SW_HIDE);
    }
  } else {
    // Show lengths
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      const int iShow = m_save_visible[i] == TRUE ? SW_SHOW : SW_HIDE;
      GetDlgItem(nonHexLengths[i])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(nonHexLengths[i])->ShowWindow(iShow);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(iShow);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(LenTxts[i * 2])->ShowWindow(iShow);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(LenTxts[i * 2 + 1])->ShowWindow(iShow);
    }
  }
}

void CAddEdit_PasswordPolicy::OnUseLowerCase()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  BOOL bEnable = (IsDlgButtonChecked(IDC_USELOWERCASE) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;
  int iShow = (m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) ? SW_HIDE : SW_SHOW;

  GetDlgItem(IDC_MINLOWERLENGTH)->EnableWindow(bEnable);
  GetDlgItem(IDC_MINLOWERLENGTH)->ShowWindow(iShow);
  GetDlgItem(IDC_SPINLOWERCASE)->EnableWindow(bEnable);
  GetDlgItem(IDC_SPINLOWERCASE)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_LC1)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_LC1)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_LC2)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_LC2)->ShowWindow(iShow);
  m_pwlowerminlength = IsDlgButtonChecked(IDC_USELOWERCASE);  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnUseUpperCase()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  BOOL bEnable = (IsDlgButtonChecked(IDC_USEUPPERCASE) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;
  int iShow = (m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) ? SW_HIDE : SW_SHOW;

  GetDlgItem(IDC_MINUPPERLENGTH)->EnableWindow(bEnable);
  GetDlgItem(IDC_MINUPPERLENGTH)->ShowWindow(iShow);
  GetDlgItem(IDC_SPINUPPERCASE)->EnableWindow(bEnable);
  GetDlgItem(IDC_SPINUPPERCASE)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_UC1)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_UC1)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_UC2)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_UC2)->ShowWindow(iShow);
  m_pwupperminlength = IsDlgButtonChecked(IDC_USEUPPERCASE);  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnUseDigits()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  BOOL bEnable = (IsDlgButtonChecked(IDC_USEDIGITS) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;
  int iShow = (m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) ? SW_HIDE : SW_SHOW;

  GetDlgItem(IDC_MINDIGITLENGTH)->EnableWindow(bEnable);
  GetDlgItem(IDC_MINDIGITLENGTH)->ShowWindow(iShow);
  GetDlgItem(IDC_SPINDIGITS)->EnableWindow(bEnable);
  GetDlgItem(IDC_SPINDIGITS)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_DG1)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_DG1)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_DG2)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_DG2)->ShowWindow(iShow);
  m_pwdigitminlength = IsDlgButtonChecked(IDC_USEDIGITS);  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnUseSymbols()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;
  int iShow = (m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) ? SW_HIDE : SW_SHOW;

  GetDlgItem(IDC_MINSYMBOLLENGTH)->EnableWindow(bEnable);
  GetDlgItem(IDC_MINSYMBOLLENGTH)->ShowWindow(iShow);
  GetDlgItem(IDC_SPINSYMBOLS)->EnableWindow(bEnable);
  GetDlgItem(IDC_SPINSYMBOLS)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_SY1)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_SY1)->ShowWindow(iShow);
  GetDlgItem(IDC_STATIC_SY2)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_SY2)->ShowWindow(iShow);
  m_pwsymbolminlength = IsDlgButtonChecked(IDC_USESYMBOLS);  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnUseHexdigits()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  do_hex(IsDlgButtonChecked(IDC_USEHEXDIGITS) == BST_CHECKED);
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnEasyVision()
{
  CGeneralMsgBox gmb;
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  if (m_pweasyvision && m_pwmakepronounceable) {
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_pweasyvision = FALSE;
  }

  do_easyorpronounceable(IsDlgButtonChecked(IDC_EASYVISION) == BST_CHECKED);
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnMakePronounceable()
{
  CGeneralMsgBox gmb;
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);

  if (m_pweasyvision && m_pwmakepronounceable) {
    ((CButton*)GetDlgItem(IDC_PRONOUNCEABLE))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_pwmakepronounceable = FALSE;
  }

  do_easyorpronounceable(IsDlgButtonChecked(IDC_PRONOUNCEABLE) == BST_CHECKED);
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnResetPolicy()
{
  m_ae_psh->SetChanged(true);
  M_pwp() = M_default_pwp();

  SetVariablesFromPolicy();

  // Now update dialog
  UpdateData(FALSE);

  // Save current status
  for (int i = 0; i < N_HEX_LENGTHS; i++) {
    m_save_visible[i] = true;
    BOOL bEnable(FALSE);
    switch (i) {
      case 0:  // IDC_MINLOWERLENGTH
        bEnable = m_pwuselowercase;
        break;
      case 1:  // IDC_MINUPPERLENGTH
        bEnable = m_pwuseuppercase;
        break;
      case 2:  // IDC_MINDIGITLENGTH
        bEnable = m_pwusedigits;
        break;
      case 3:  // IDC_MINSYMBOLLENGTH
        bEnable =  m_pwusesymbols;
        break;
      default:
        ASSERT(0);
    }
    m_save_enabled[i][0] = m_save_enabled[i][1] = bEnable;
  }

  SetPolicyControls();
}

void CAddEdit_PasswordPolicy::OnSetDefaultPWPolicy()
{
  m_ae_psh->SetChanged(true);
  M_ipolicy() = CAddEdit_PropertySheet::DEFAULT_POLICY;

  SetPolicyControls();
}

void CAddEdit_PasswordPolicy::OnSetSpecificPWPolicy()
{
  m_ae_psh->SetChanged(true);
  M_ipolicy() = CAddEdit_PropertySheet::SPECIFIC_POLICY;

  SetPolicyControls();
}

void CAddEdit_PasswordPolicy::SetPolicyControls()
{
  BOOL bEnableSpecificPolicy(FALSE);
  bool bEnableLengths(false);
  int iShowLengths(SW_SHOW);

  if (M_uicaller() != IDS_VIEWENTRY) {
    bEnableSpecificPolicy = (M_ipolicy() == CAddEdit_PropertySheet::DEFAULT_POLICY) ? FALSE : TRUE;
    bEnableLengths = ((bEnableSpecificPolicy == TRUE) &&
                      (m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE &&
                       m_pwusehexdigits == FALSE));
    iShowLengths = (m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) ? 
                       SW_HIDE : SW_SHOW;
  }

  GetDlgItem(IDC_DEFPWLENGTH)->EnableWindow(bEnableSpecificPolicy);
  GetDlgItem(IDC_PWLENSPIN)->EnableWindow(bEnableSpecificPolicy);

  // Deal with lengths
  for (int i = 0; i < N_HEX_LENGTHS; i++) {
    BOOL bEnable(FALSE);
    switch (i) {
      case 0:  // IDC_MINLOWERLENGTH
        bEnable = bEnableLengths ? m_pwuselowercase : FALSE;
        break;
      case 1:  // IDC_MINUPPERLENGTH
        bEnable = bEnableLengths ? m_pwuseuppercase : FALSE;
        break;
      case 2:  // IDC_MINDIGITLENGTH
        bEnable = bEnableLengths ? m_pwusedigits : FALSE;
        break;
      case 3:  // IDC_MINSYMBOLLENGTH
        bEnable = bEnableLengths ? m_pwusesymbols : FALSE;
        break;
      default:
        ASSERT(0);
    }
    GetDlgItem(nonHexLengths[i])->EnableWindow(bEnable);
    GetDlgItem(nonHexLengths[i])->ShowWindow(iShowLengths);
    GetDlgItem(nonHexLengthSpins[i])->EnableWindow(bEnable);
    GetDlgItem(nonHexLengthSpins[i])->ShowWindow(iShowLengths);
    GetDlgItem(LenTxts[i * 2])->EnableWindow(bEnable);
    GetDlgItem(LenTxts[i * 2])->ShowWindow(iShowLengths);
    GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(bEnable);
    GetDlgItem(LenTxts[i * 2 + 1])->ShowWindow(iShowLengths);
  }

  // Deal with non-hex checkboxes
  for (int i = 0; i < N_NOHEX; i++) {
    GetDlgItem(nonHex[i])->EnableWindow(bEnableSpecificPolicy);
  }
  // Deal with hex checkbox
  GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(bEnableSpecificPolicy);

  GetDlgItem(IDC_RESETPWPOLICY)->EnableWindow(bEnableSpecificPolicy);
  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::SetPolicyFromVariables()
{
  if (M_ipolicy() == CAddEdit_PropertySheet::DEFAULT_POLICY) {
    M_pwp() = M_default_pwp();
  } else {
    M_pwp().Empty();
    if (m_pwuselowercase == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyUseLowercase;
    if (m_pwuseuppercase == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyUseUppercase;
    if (m_pwusedigits == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyUseDigits;
    if (m_pwusesymbols == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyUseSymbols;
    if (m_pwusehexdigits == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyUseHexDigits;
    if (m_pweasyvision == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyUseEasyVision;
    if (m_pwmakepronounceable == TRUE)
      M_pwp().flags |= PWSprefs::PWPolicyMakePronounceable;
    M_pwp().length = m_pwdefaultlength;
    M_pwp().digitminlength = m_pwdigitminlength;
    M_pwp().lowerminlength = m_pwlowerminlength;
    M_pwp().symbolminlength = m_pwsymbolminlength;
    M_pwp().upperminlength = m_pwupperminlength;
  }
}

void CAddEdit_PasswordPolicy::SetVariablesFromPolicy()
{
  m_pwuselowercase = (M_pwp().flags & PWSprefs::PWPolicyUseLowercase) ? TRUE : FALSE;
  m_pwuseuppercase = (M_pwp().flags & PWSprefs::PWPolicyUseUppercase) ? TRUE : FALSE;
  m_pwusedigits = (M_pwp().flags & PWSprefs::PWPolicyUseDigits) ? TRUE : FALSE;
  m_pwusesymbols = (M_pwp().flags & PWSprefs::PWPolicyUseSymbols) ? TRUE : FALSE;
  m_pwusehexdigits = (M_pwp().flags & PWSprefs::PWPolicyUseHexDigits) ? TRUE : FALSE;
  m_pweasyvision = (M_pwp().flags & PWSprefs::PWPolicyUseEasyVision) ? TRUE : FALSE;
  m_pwmakepronounceable =  (M_pwp().flags & PWSprefs::PWPolicyMakePronounceable) ? TRUE : FALSE;
  m_pwdefaultlength = M_pwp().length;
  m_pwdigitminlength = M_pwp().digitminlength;
  m_pwlowerminlength = M_pwp().lowerminlength;
  m_pwsymbolminlength = M_pwp().symbolminlength;
  m_pwupperminlength = M_pwp().upperminlength;

  if (m_pwuselowercase == TRUE && m_pwlowerminlength == 0)
    m_pwlowerminlength = 1;
  if (m_pwuseuppercase == TRUE && m_pwupperminlength == 0)
    m_pwupperminlength = 1;
  if (m_pwusedigits == TRUE && m_pwdigitminlength == 0)
    m_pwdigitminlength = 1;
  if (m_pwusesymbols == TRUE && m_pwsymbolminlength == 0)
    m_pwsymbolminlength = 1;
}

void CAddEdit_PasswordPolicy::DisablePolicy()
{
  int iShowLengths = (m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) ? 
                       SW_HIDE : SW_SHOW;

  GetDlgItem(IDC_DEFPWLENGTH)->EnableWindow(FALSE);
  GetDlgItem(IDC_PWLENSPIN)->EnableWindow(FALSE);

  // Deal with lengths
  for (int i = 0; i < N_HEX_LENGTHS; i++) {
    GetDlgItem(nonHexLengths[i])->EnableWindow(FALSE);
    GetDlgItem(nonHexLengths[i])->ShowWindow(iShowLengths);
    GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
    GetDlgItem(nonHexLengthSpins[i])->ShowWindow(iShowLengths);
    GetDlgItem(LenTxts[i * 2])->EnableWindow(FALSE);
    GetDlgItem(LenTxts[i * 2])->ShowWindow(iShowLengths);
    GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(FALSE);
    GetDlgItem(LenTxts[i * 2 + 1])->ShowWindow(iShowLengths);
  }

  // Deal with non-hex checkboxes
  for (int i = 0; i < N_NOHEX; i++) {
    GetDlgItem(nonHex[i])->EnableWindow(FALSE);
  }
  // Deal with hex checkbox
  GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);

  GetDlgItem(IDC_RESETPWPOLICY)->EnableWindow(FALSE);
}
