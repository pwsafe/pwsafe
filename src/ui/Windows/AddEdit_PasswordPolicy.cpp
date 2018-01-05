/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_PasswordPolicy.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"

#include "DboxMain.h"

#include "AddEdit_PasswordPolicy.h"
#include "AddEdit_PropertySheet.h"
#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "core/core.h"
#include "core/PwsPlatform.h"
#include "core/ItemData.h"
#include "core/PWSprefs.h"
#include "core/PWCharPool.h"

#include <algorithm>

#include "resource.h"
#include "resource3.h"  // String resources

using pws_os::CUUID;

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

CAddEdit_PasswordPolicy::CAddEdit_PasswordPolicy(CWnd *pParent,
                                                 st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent,
                          CAddEdit_PasswordPolicy::IDD, CAddEdit_PasswordPolicy::IDD_SHORT,
                          pAEMD), m_bInitdone(false)
{
  m_policy_radibtn = M_ipolicy() != SPECIFIC_POLICY ? 0 : 1 ;

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
  DDX_Radio(pDX, IDC_USEPWPOLICYNAME, m_policy_radibtn);

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

  DDX_Control(pDX, IDC_OWNSYMBOLS, (CEdit&)m_symbols);
  
  DDX_Control(pDX, IDC_POLICYLIST, m_cbxPolicyNames);
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
  ON_BN_CLICKED(IDC_USEPWPOLICYNAME, OnSelectNamedPolicy)
  ON_BN_CLICKED(IDC_ENTRYPWPOLICY, OnSetSpecificPWPolicy)

  ON_EN_CHANGE(IDC_DEFPWLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINDIGITLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINLOWERLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINSYMBOLLENGTH, OnChanged)
  ON_EN_CHANGE(IDC_MINUPPERLENGTH, OnChanged)

  ON_EN_CHANGE(IDC_OWNSYMBOLS, OnOwnSymbolsChanged)
  
  ON_CBN_SELCHANGE(IDC_POLICYLIST, OnNamesComboChanged)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_RESET_SYMBOLS, &CAddEdit_PasswordPolicy::OnSymbolReset)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_PasswordPolicy message handlers

BOOL CAddEdit_PasswordPolicy::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

// Following is also in PasswordPolicDlg.cpp. Move to common mixin?
static void setupBuddy(CWnd *p, int spinid, int id, int &length, PWSprefs::IntPrefs iPref)
{
  const int minValue = PWSprefs::GetInstance()->GetPrefMinVal(iPref);
  const int maxValue = PWSprefs::GetInstance()->GetPrefMaxVal(iPref);

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)p->GetDlgItem(spinid);
  pspin->SetBuddy(p->GetDlgItem(id));
  pspin->SetRange32(minValue, maxValue);
  pspin->SetBase(10);
  pspin->SetPos(length);
}

BOOL CAddEdit_PasswordPolicy::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  // Get Add/Edit font
  Fonts *pFonts = Fonts::GetInstance();
  CFont *pFont = pFonts->GetAddEditFont();

  // Change font size of the user supplied symbol fields
  m_symbols.SetFont(pFont);

  // Populate the combo box
  m_cbxPolicyNames.ResetContent();
  m_cbxPolicyNames.ChangeColour();

  // Get all password policy names
  std::vector<std::wstring> vNames;
  GetMainDlg()->GetPolicyNames(vNames);

  // Add Default
  CString cs_text(MAKEINTRESOURCE(IDSC_DEFAULT_POLICY));
  m_cbxPolicyNames.AddString(cs_text);

  // Add the rest (if any)
  for (std::vector<std::wstring>::iterator iter = vNames.begin();
       iter != vNames.end(); ++iter) {
    m_cbxPolicyNames.AddString(iter->c_str());
  }

  int index(0);
  if (M_ipolicy() == NAMED_POLICY && !M_policyname().IsEmpty() && !vNames.empty()) {
    index = m_cbxPolicyNames.FindStringExact(-1, M_policyname());
    if (index == CB_ERR)
      index = 0;
  }
  m_cbxPolicyNames.SetCurSel(index);
  
  // Make sure user can't edit the list!
  COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
  m_cbxPolicyNames.GetComboBoxInfo(&info);
  ::SendMessage(info.hwndItem ,EM_SETREADONLY, TRUE, 0);

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    // Read-only
    m_cbxPolicyNames.EnableWindow(FALSE);
    m_cbxPolicyNames.ShowWindow(M_ipolicy() == SPECIFIC_POLICY ? SW_HIDE : SW_SHOW);
  } else {
    // If specific policy - disable named policy combobox
    m_cbxPolicyNames.EnableWindow(M_ipolicy() == SPECIFIC_POLICY ? FALSE : TRUE);
  }

  if (M_ipolicy() == NAMED_POLICY) {
    if (index != 0) {
      PWPolicy st_pp;
      GetMainDlg()->GetPolicyFromName(StringX((LPCWSTR)M_policyname()), st_pp);
      M_pwp() = st_pp;
      M_symbols() = st_pp.symbols;
    } else {
      M_policyname().Empty();
      M_pwp() = M_default_pwp();
      M_symbols() = M_default_symbols();
    }
    SetVariablesFromPolicy();
  }

  // Set up spin control relationships
  setupBuddy(this, IDC_PWLENSPIN, IDC_DEFPWLENGTH, m_pwdefaultlength, PWSprefs::PWDefaultLength);
  setupBuddy(this, IDC_SPINDIGITS, IDC_MINDIGITLENGTH, m_pwdigitminlength, PWSprefs::PWDigitMinLength);
  setupBuddy(this, IDC_SPINLOWERCASE, IDC_MINLOWERLENGTH, m_pwlowerminlength, PWSprefs::PWLowercaseMinLength);
  setupBuddy(this, IDC_SPINUPPERCASE, IDC_MINUPPERLENGTH, m_pwupperminlength, PWSprefs::PWUppercaseMinLength);
  setupBuddy(this, IDC_SPINSYMBOLS, IDC_MINSYMBOLLENGTH, m_pwsymbolminlength, PWSprefs::PWSymbolMinLength);

  // Disable controls based on m_ipolicy
  SetPolicyControls();
  
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    // Disable Buttons not already disabled in SetPolicyControls
    GetDlgItem(IDC_USEPWPOLICYNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRYPWPOLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_PWLEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_OR)->EnableWindow(FALSE);
  }

  if (M_original_entrytype() == CItemData::ET_ALIAS) {
    DisablePolicy();
    GetDlgItem(IDC_USEPWPOLICYNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRYPWPOLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_PWLEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_OR)->EnableWindow(FALSE);
  }

  // Setup symbols
  m_symbols.SetWindowText(M_symbols());

  m_bInitdone = true;
  return TRUE;  // return TRUE unless you set the focus to a control
}

void CAddEdit_PasswordPolicy::OnChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_PasswordPolicy::OnHelp()
{
  ShowHelp(L"::/html/entering_pwd_pp.html");
}

bool CAddEdit_PasswordPolicy::ValidatePolicy(CWnd *&pFocus)
{
  if (M_ipolicy() == NAMED_POLICY && !M_policyname().IsEmpty())
    return true;

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

  if ((m_pwdefaultlength < M_prefminPWLength()) || (m_pwdefaultlength > M_prefmaxPWLength())) {
    CString csText;
    csText.Format(IDS_DEFAULTPWLENGTH, M_prefminPWLength(), M_prefmaxPWLength());
    gmb.AfxMessageBox(csText);
    pFocus = GetDlgItem(IDC_DEFPWLENGTH);
    return false;
  }

  if (!(m_pwusehexdigits || m_pwmakepronounceable) &&
      ((m_pwusedigits ? m_pwdigitminlength : 0) +
       (m_pwuselowercase ? m_pwlowerminlength : 0) +
       (m_pwusesymbols ? m_pwsymbolminlength : 0) +
       (m_pwuseuppercase ? m_pwupperminlength : 0)) > m_pwdefaultlength) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTHTOOSMALL);
    pFocus = GetDlgItem(IDC_DEFPWLENGTH);
    return false;
  }

  return true;
}

BOOL CAddEdit_PasswordPolicy::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

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
      if (M_ipolicy()     != M_oldipolicy()     ||
          (M_ipolicy()    == SPECIFIC_POLICY &&
           M_pwp()        != M_oldpwp())        ||
          (M_ipolicy()    == NAMED_POLICY &&
           M_policyname() != M_oldpolicyname()) ||
          M_iownsymbols() != M_ioldownsymbols() ||
          (M_ipolicy()    != NAMED_POLICY &&
           M_symbols()    != M_oldsymbols())      )
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
    case PP_UPDATE_PWPOLICY:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
      break;
  }
  return 0L;
}

BOOL CAddEdit_PasswordPolicy::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE; //CAddEdit_PropertyPage::OnApply();

  UpdateData(TRUE);
  CWnd *pFocus(NULL);

  if (M_ipolicy() == DEFAULT_POLICY) {
    return CAddEdit_PropertyPage::OnApply();
  }

  if (ValidatePolicy(pFocus)) {
    SetPolicyFromVariables();
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

    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(FALSE);
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

    BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                   m_pweasyvision == FALSE && m_pwmakepronounceable == FALSE) ? TRUE : FALSE;
    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(bEnable == TRUE);
  }
}

void CAddEdit_PasswordPolicy::do_easyorpronounceable(const bool bSet)
{
  // Can't have any minimum lengths set!
  if ((m_pwmakepronounceable == TRUE) &&
      (m_pwdigitminlength  > 1 || m_pwlowerminlength > 1 ||
       m_pwsymbolminlength > 1 || m_pwupperminlength > 1)) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTSPECIFYMINNUMBER);
  }

  CString cs_value;
  int i;
  if (bSet) {
    // Disable lengths
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      m_save_enabled[i][0] = GetDlgItem(nonHexLengths[i])->IsWindowEnabled();
      m_save_visible[i] = GetDlgItem(nonHexLengths[i])->IsWindowVisible();
      GetDlgItem(nonHexLengths[i])->EnableWindow(FALSE);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(FALSE);
    }

  } else {
    // Show lengths
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(m_save_enabled[i][0]);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(m_save_enabled[i][0]);
    }
  }
  OnSymbolReset();
}

void CAddEdit_PasswordPolicy::do_useX(UseX x)
{
  const struct { int cb; int edit; int spin; int left; int right;} controls[] = {
    {IDC_USELOWERCASE, IDC_MINLOWERLENGTH,  IDC_SPINLOWERCASE, IDC_STATIC_LC1, IDC_STATIC_LC2},
    {IDC_USEUPPERCASE, IDC_MINUPPERLENGTH,  IDC_SPINUPPERCASE, IDC_STATIC_UC1, IDC_STATIC_UC2},
    {IDC_USEDIGITS,    IDC_MINDIGITLENGTH,  IDC_SPINDIGITS,    IDC_STATIC_DG1, IDC_STATIC_DG2},
    {IDC_USESYMBOLS,   IDC_MINSYMBOLLENGTH, IDC_SPINSYMBOLS,   IDC_STATIC_SY1, IDC_STATIC_SY2},
  };

  UpdateData(TRUE);

  m_ae_psh->SetChanged(true);

  BOOL bEnable = (IsDlgButtonChecked(controls[x].cb) == BST_CHECKED &&
                  m_pwmakepronounceable == FALSE) ? TRUE : FALSE;

  GetDlgItem(controls[x].edit)->EnableWindow(bEnable);
  GetDlgItem(controls[x].spin)->EnableWindow(bEnable);
  GetDlgItem(controls[x].left)->EnableWindow(bEnable);
  GetDlgItem(controls[x].right)->EnableWindow(bEnable);
  if (x == USESYM) {
    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(bEnable == TRUE);
  }

  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::OnUseLowerCase()
{
  do_useX(USELOWER);
}

void CAddEdit_PasswordPolicy::OnUseUpperCase()
{
  do_useX(USEUPPER);
}

void CAddEdit_PasswordPolicy::OnUseDigits()
{
  do_useX(USEDIGITS);
}

void CAddEdit_PasswordPolicy::OnUseSymbols()
{
  do_useX(USESYM);
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

  OnSymbolReset();
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

void CAddEdit_PasswordPolicy::OnOwnSymbolsChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);

  bool bIsSymbolsChanged = false;

  CString cs_symbols;
  m_symbols.GetWindowText(cs_symbols);

  std::wstring oldstr = (LPCWSTR)M_symbols();
  std::wstring newstr = cs_symbols;

  // First check lengths the same
  if (newstr.length() != oldstr.length()) {
    bIsSymbolsChanged = true;
  } else {
    // We do not care if same string in different order:
    //  so sort new string
    std::sort(newstr.begin(), newstr.end());

    //  then sort old string
    std::sort(oldstr.begin(), oldstr.end());

    // Then compare
    bIsSymbolsChanged = oldstr.compare(newstr) != 0;
  }
  m_ae_psh->SetSymbolsChanged(bIsSymbolsChanged);
}

void CAddEdit_PasswordPolicy::OnSelectNamedPolicy()
{
  m_ae_psh->SetChanged(true);

  m_cbxPolicyNames.EnableWindow(TRUE);
  m_policy_radibtn = 0;

  int index = m_cbxPolicyNames.GetCurSel();
  M_ipolicy() = index == 0 ? DEFAULT_POLICY : NAMED_POLICY;
  if (index != 0 && index != CB_ERR) {
    CString cs_text;
    m_cbxPolicyNames.GetLBText(index, cs_text);
    M_policyname() = (LPCWSTR)cs_text;

    PWPolicy st_pp;
    GetMainDlg()->GetPolicyFromName(StringX((LPCWSTR)cs_text), st_pp);
    M_pwp() = st_pp;
    M_symbols() = st_pp.symbols;

    m_cbxPolicyNames.EnableWindow(TRUE);
  } else {
    M_policyname().Empty();
    M_pwp() = M_default_pwp();
    M_symbols() = M_default_symbols();
  }

  SetVariablesFromPolicy();
  SetPolicyControls();
}

void CAddEdit_PasswordPolicy::OnSetSpecificPWPolicy()
{
  m_ae_psh->SetChanged(true);

  m_cbxPolicyNames.EnableWindow(FALSE);
  m_policy_radibtn = 1;

  M_ipolicy() = SPECIFIC_POLICY;
  M_policyname().Empty();

  SetPolicyControls();

  m_cbxPolicyNames.EnableWindow(FALSE);
}

void CAddEdit_PasswordPolicy::SetPolicyControls()
{
  BOOL bEnableSpecificPolicy(FALSE);
  bool bEnableLengths(false);
  int iShowLengths(SW_SHOW);

  if (M_uicaller() == IDS_ADDENTRY || 
     (M_uicaller() == IDS_EDITENTRY && M_protected() == 0)) {
    bEnableSpecificPolicy = (M_ipolicy() == SPECIFIC_POLICY) ? TRUE : FALSE;
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

  // Deal with symbols...
  // Default policy - all disabled like the rest.
  // Specific policy - enable if Use Symbols set.
  BOOL bEnableSymControls = (m_pwusesymbols == TRUE && bEnableSpecificPolicy == TRUE) ? TRUE: FALSE;

  GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(bEnableSymControls);
  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(bEnableSymControls);

  // populate m_symbols
  if (M_symbols().IsEmpty())
    OnSymbolReset();

  UpdateData(FALSE);
}

void CAddEdit_PasswordPolicy::SetPolicyFromVariables()
{
  switch (M_ipolicy()) {
    case DEFAULT_POLICY:
      M_pwp() = M_default_pwp();
      M_iownsymbols() = DEFAULT_SYMBOLS;
      break;
    case NAMED_POLICY:
      break;
    case SPECIFIC_POLICY:
      M_pwp().Empty();
      // Since in Hex, the checkboxes for non-hex characters can still be
      // checked but the checkbox is disabled, we have to check both
      if (m_pwuselowercase == TRUE &&
          (GetDlgItem(IDC_USELOWERCASE)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::UseLowercase;
      if (m_pwuseuppercase == TRUE &&
          (GetDlgItem(IDC_USEUPPERCASE)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::UseUppercase;
      if (m_pwusedigits == TRUE &&
          (GetDlgItem(IDC_USEDIGITS)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::UseDigits;
      if (m_pwusesymbols == TRUE &&
          (GetDlgItem(IDC_USESYMBOLS)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::UseSymbols;
      if (m_pwusehexdigits == TRUE &&
          (GetDlgItem(IDC_USEHEXDIGITS)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::UseHexDigits;
      if (m_pweasyvision == TRUE &&
          (GetDlgItem(IDC_EASYVISION)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::UseEasyVision;
      if (m_pwmakepronounceable == TRUE &&
          (GetDlgItem(IDC_PRONOUNCEABLE)->IsWindowEnabled() == TRUE))
        M_pwp().flags |= PWPolicy::MakePronounceable;

      M_pwp().length = (int)m_pwdefaultlength;
      M_pwp().digitminlength = (int)m_pwdigitminlength;
      M_pwp().lowerminlength = (int)m_pwlowerminlength;
      M_pwp().symbolminlength = (int)m_pwsymbolminlength;
      M_pwp().upperminlength = (int)m_pwupperminlength;

      CString cs_symbols;
      m_symbols.GetWindowText(cs_symbols);
      if (m_pwusesymbols == TRUE && !cs_symbols.IsEmpty()) {
        M_iownsymbols() = OWN_SYMBOLS;
        M_symbols() = CSecString(cs_symbols);
      } else {
        M_iownsymbols() = DEFAULT_SYMBOLS;
      }
      break;
  }
}

void CAddEdit_PasswordPolicy::SetVariablesFromPolicy()
{
  m_pwuselowercase = (M_pwp().flags & PWPolicy::UseLowercase) ? TRUE : FALSE;
  m_pwuseuppercase = (M_pwp().flags & PWPolicy::UseUppercase) ? TRUE : FALSE;
  m_pwusedigits = (M_pwp().flags & PWPolicy::UseDigits) ? TRUE : FALSE;
  m_pwusesymbols = (M_pwp().flags & PWPolicy::UseSymbols) ? TRUE : FALSE;
  m_pwusehexdigits = (M_pwp().flags & PWPolicy::UseHexDigits) ? TRUE : FALSE;
  m_pweasyvision = (M_pwp().flags & PWPolicy::UseEasyVision) ? TRUE : FALSE;
  m_pwmakepronounceable =  (M_pwp().flags & PWPolicy::MakePronounceable) ? TRUE : FALSE;
  m_pwdefaultlength = M_pwp().length;
  m_pwdigitminlength = M_pwp().digitminlength;
  m_pwlowerminlength = M_pwp().lowerminlength;
  m_pwsymbolminlength = M_pwp().symbolminlength;
  m_pwupperminlength = M_pwp().upperminlength;
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
}

void CAddEdit_PasswordPolicy::OnNamesComboChanged()
{
  UpdateData(TRUE);

  CSecString cs_policyname(L"");

  int index = m_cbxPolicyNames.GetCurSel();
  if (index == CB_ERR)
    index = 0;

  M_ipolicy() = index == 0 ? DEFAULT_POLICY : NAMED_POLICY;

  if (index != 0) {
    m_cbxPolicyNames.GetLBText(index, cs_policyname);
  }

  if (M_policyname() != cs_policyname) {
    M_policyname() = cs_policyname;
    m_ae_psh->SetChanged(true);
  }

  if (index != 0) {
    PWPolicy st_pp;
    GetMainDlg()->GetPolicyFromName(StringX((LPCWSTR)cs_policyname), st_pp);
    M_pwp() = st_pp;
    M_symbols() = st_pp.symbols;

    m_cbxPolicyNames.EnableWindow(TRUE);
  } else {
    M_policyname().Empty();
    M_pwp() = M_default_pwp();
    M_symbols() = M_default_symbols();
  }

  SetVariablesFromPolicy();
  SetPolicyControls();
}

void CAddEdit_PasswordPolicy::OnSymbolReset()
{
  std::wstring symbols;
  if (m_pweasyvision)
    symbols = CPasswordCharPool::GetEasyVisionSymbols();
  else if (m_pwmakepronounceable)
    symbols = CPasswordCharPool::GetPronounceableSymbols();
  else
    symbols = CPasswordCharPool::GetDefaultSymbols();
  m_symbols.SetValidSym(symbols);
  m_symbols.SetWindowText(symbols.c_str());
}
