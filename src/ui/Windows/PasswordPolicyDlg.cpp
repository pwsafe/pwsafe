/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PasswordPolicyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "DboxMain.h"
#include "PasswordPolicyDlg.h"
#include "GeneralMsgBox.h"

#include "core/core.h"
#include "core/PWCharPool.h"
#include "core/PwsPlatform.h"
#include "core/PWSprefs.h"

#include "resource.h"
#include "resource3.h"  // String resources

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPasswordPolicyDlg property page

/*

  No need to colour all the controls blue as everything in this dialog
  affects the database - whether the default password policy in the preferences
  or the named password policies in the header.
  
*/

// nonHex* are irrelevant and disabled when hex is selected
const UINT CPasswordPolicyDlg::nonHex[CPasswordPolicyDlg::N_NOHEX] = {
  IDC_USELOWERCASE, IDC_USEUPPERCASE, IDC_USEDIGITS,
  IDC_USESYMBOLS, IDC_EASYVISION, IDC_PRONOUNCEABLE};

const UINT CPasswordPolicyDlg::LenTxts[CPasswordPolicyDlg::N_HEX_LENGTHS * 2] = {
  IDC_STATIC_LC1, IDC_STATIC_LC2, IDC_STATIC_UC1, IDC_STATIC_UC2,
  IDC_STATIC_DG1, IDC_STATIC_DG2, IDC_STATIC_SY1, IDC_STATIC_SY2};

const UINT CPasswordPolicyDlg::nonHexLengths[CPasswordPolicyDlg::N_HEX_LENGTHS] = {
  IDC_MINLOWERLENGTH, IDC_MINUPPERLENGTH, IDC_MINDIGITLENGTH, IDC_MINSYMBOLLENGTH};

const UINT CPasswordPolicyDlg::nonHexLengthSpins[CPasswordPolicyDlg::N_HEX_LENGTHS] = {
  IDC_SPINLOWERCASE, IDC_SPINUPPERCASE, IDC_SPINDIGITS, IDC_SPINSYMBOLS};

IMPLEMENT_DYNAMIC(CPasswordPolicyDlg, CPWDialog)

CPasswordPolicyDlg::CPasswordPolicyDlg(UINT uicaller, CWnd *pParent, bool bLongPPs,
                                       bool bReadOnly, PWPolicy &st_default_pp)
  : CPWDialog(bLongPPs ? CPasswordPolicyDlg::IDD : CPasswordPolicyDlg::IDD_SHORT, pParent),
  m_uicaller(uicaller), m_bReadOnly(bReadOnly), m_password(L""),
  m_UseNamedPolicy(FALSE), m_st_default_pp(st_default_pp), m_bLongPPs(bLongPPs), m_pCopyBtn(NULL),
  m_bCopyPasswordEnabled(false), m_bImageLoaded(FALSE), m_bDisabledImageLoaded(FALSE)
{
  m_PWUseLowercase = m_oldPWUseLowercase =
    (m_st_default_pp.flags & PWPolicy::UseLowercase) != 0;
  m_PWUseUppercase = m_oldPWUseUppercase =
    (m_st_default_pp.flags & PWPolicy::UseUppercase) != 0;
  m_PWUseDigits = m_oldPWUseDigits =
    (m_st_default_pp.flags & PWPolicy::UseDigits) != 0;
  m_PWUseSymbols = m_oldPWUseSymbols =
    (m_st_default_pp.flags & PWPolicy::UseSymbols) != 0;
  m_PWUseHexdigits = m_oldPWUseHexdigits =
    (m_st_default_pp.flags & PWPolicy::UseHexDigits) != 0;
  m_PWEasyVision = m_oldPWEasyVision =
    (m_st_default_pp.flags & PWPolicy::UseEasyVision) != 0;
  m_PWMakePronounceable = m_oldPWMakePronounceable =
    (m_st_default_pp.flags & PWPolicy::MakePronounceable) != 0;

  m_PWDefaultLength = m_oldPWDefaultLength = m_st_default_pp.length;
  m_PWDigitMinLength = m_oldPWDigitMinLength = m_st_default_pp.digitminlength;
  m_PWLowerMinLength = m_oldPWLowerMinLength = m_st_default_pp.lowerminlength;
  m_PWSymbolMinLength = m_oldPWSymbolMinLength = m_st_default_pp.symbolminlength;
  m_PWUpperMinLength = m_oldPWUpperMinLength = m_st_default_pp.upperminlength;

  if (!m_st_default_pp.symbols.empty())
    m_Symbols = m_oldSymbols = m_st_default_pp.symbols.c_str();
  else {
    // empty, set to appropriate default
    std::wstring st_symbols;
    if (m_PWEasyVision == TRUE)
      st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
    else if (m_PWMakePronounceable == TRUE)
      st_symbols = CPasswordCharPool::GetPronounceableSymbols();
    else
      st_symbols = CPasswordCharPool::GetDefaultSymbols();
    m_Symbols = m_oldSymbols = st_symbols.c_str();
  }

  // These must be in the same order as their respective Control IDs
  // in CPasswordPolicyDlg::nonHex
  m_pnonHex[0] = &m_PWUseLowercase;
  m_pnonHex[1] = &m_PWUseUppercase;
  m_pnonHex[2] = &m_PWUseDigits;
  m_pnonHex[3] = &m_PWUseSymbols;
  m_pnonHex[4] = &m_PWEasyVision;
  m_pnonHex[5] = &m_PWMakePronounceable;
}

CPasswordPolicyDlg::~CPasswordPolicyDlg()
{
  if (m_bImageLoaded)
    m_CopyPswdBitmap.Detach();

  if (m_bDisabledImageLoaded) 
    m_DisabledCopyPswdBitmap.Detach();
}

void CPasswordPolicyDlg::DoDataExchange(CDataExchange* pDX)
{
    CPWDialog::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CPasswordPolicyDlg)
    DDX_Text(pDX, IDC_DEFPWLENGTH, m_PWDefaultLength);
    DDX_Text(pDX, IDC_MINDIGITLENGTH, m_PWDigitMinLength);
    DDX_Text(pDX, IDC_MINLOWERLENGTH, m_PWLowerMinLength);
    DDX_Text(pDX, IDC_MINSYMBOLLENGTH, m_PWSymbolMinLength);
    DDX_Text(pDX, IDC_MINUPPERLENGTH, m_PWUpperMinLength);
    DDX_Check(pDX, IDC_USELOWERCASE, m_PWUseLowercase);
    DDX_Check(pDX, IDC_USEUPPERCASE, m_PWUseUppercase);
    DDX_Check(pDX, IDC_USEDIGITS, m_PWUseDigits);
    DDX_Check(pDX, IDC_USESYMBOLS, m_PWUseSymbols);
    DDX_Check(pDX, IDC_EASYVISION, m_PWEasyVision);
    DDX_Check(pDX, IDC_USEHEXDIGITS, m_PWUseHexdigits);
    DDX_Check(pDX, IDC_PRONOUNCEABLE, m_PWMakePronounceable);
    DDX_Check(pDX, IDC_USENAMED_POLICY, m_UseNamedPolicy);

    DDX_Control(pDX, IDC_OWNSYMBOLS, (CEdit&)m_SymbolsEdit);

    // Because we can show the generated password when used from Manage->Generate
    DDX_Control(pDX, IDC_PASSWORD, m_ex_password);

    DDX_Control(pDX, IDC_POLICYNAME, m_PolicyNameEdit);
    DDX_Control(pDX, IDC_POLICYLIST, m_cbxPolicyNames);

    DDX_Control(pDX, IDC_STATIC_MESSAGE, m_stcMessage);
    DDX_Control(pDX, IDC_COPYPASSWORDHELP, m_Help1);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasswordPolicyDlg, CPWDialog)
  //{{AFX_MSG_MAP(CPasswordPolicyDlg)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_USEHEXDIGITS, OnUseHexdigits)
  ON_BN_CLICKED(IDC_USELOWERCASE, OnUseLowerCase)
  ON_BN_CLICKED(IDC_USEUPPERCASE, OnUseUpperCase)
  ON_BN_CLICKED(IDC_USEDIGITS, OnUseDigits)
  ON_BN_CLICKED(IDC_USESYMBOLS, OnUseSymbols)
  ON_BN_CLICKED(IDC_EASYVISION, OnEasyVision)
  ON_BN_CLICKED(IDC_PRONOUNCEABLE, OnMakePronounceable)
  ON_BN_CLICKED(IDC_USENAMED_POLICY, OnUseNamedPolicy)
  ON_BN_CLICKED(IDC_RESET_SYMBOLS, OnSymbolReset)

  // Because we can show the generated password when used from Manage->Generate
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopyPassword)
  ON_EN_CHANGE(IDC_PASSWORD, OnENChangePassword)

  ON_EN_KILLFOCUS(IDC_POLICYNAME, OnENChangePolicyName)
  ON_EN_KILLFOCUS(IDC_OWNSYMBOLS, OnENOwnSymbols)

  ON_CBN_SELCHANGE(IDC_POLICYLIST, OnNamesComboChanged)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPasswordPolicyDlg message handlers

// Following copied from AddEdit_PasswordPolicy.cpp. Move to common mixin?
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

BOOL CPasswordPolicyDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // If started with Tall and won't fit - return to be called again with Wide
  if (m_bLongPPs && !GetMainDlg()->LongPPs(this)) {
    EndDialog(-1);
    return TRUE;
  }
  
  m_pCopyBtn = (CButton *)GetDlgItem(IDC_COPYPASSWORD);

  m_stcMessage.SetWindowText(L"");

  if (m_bReadOnly && m_uicaller != IDS_GENERATEPASSWORD) {
    // Change OK button test
    CString cs_close(MAKEINTRESOURCE(IDS_CLOSE));
    GetDlgItem(IDOK)->SetWindowText(cs_close);

    // Hide the Cancel button
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
  }

  CString cs_title;
  switch (m_uicaller) {
  case IDS_OPTIONS:
    // Set correct window title
    cs_title.LoadString(IDS_EDIT_DEFAULT_POLICY);
    SetWindowText(cs_title);

    // These are only used in Manage -> Generate Password or Add/Edit Policy names
    m_pCopyBtn->EnableWindow(FALSE);
    m_pCopyBtn->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
    GetDlgItem(IDC_GENERATEPASSWORD)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
    GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_POLICYNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_POLICYNAME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_POLICYLIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_POLICYLIST)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_USENAMED_POLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_USENAMED_POLICY)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_NAMEDPOLICY)->ShowWindow(SW_HIDE);
    break;
  case IDS_GENERATEPASSWORD:
    {
      // Set correct window title
      cs_title.LoadString(IDS_GENERATEPASSWORD);
      SetWindowText(cs_title);

      // These are only used in Manage -> Add/Edit Policy names
      GetDlgItem(IDC_POLICYNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_POLICYNAME)->ShowWindow(SW_HIDE);

      // Used to generate passwords
      GetDlgItem(IDC_POLICYLIST)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_USENAMED_POLICY)->EnableWindow(TRUE);
      GetDlgItem(IDC_USENAMED_POLICY)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_STATIC_NAMEDPOLICY)->ShowWindow(SW_HIDE);

      // Populate the combo box
      m_cbxPolicyNames.ResetContent();

      // Get all password policy names
      std::vector<std::wstring> vNames;
      GetMainDlg()->GetPolicyNames(vNames);

      // Add Default
      const CString cs_default(MAKEINTRESOURCE(IDSC_DEFAULT_POLICY));
      m_cbxPolicyNames.AddString(cs_default);

      for (std::vector<std::wstring>::iterator iter = vNames.begin();
           iter != vNames.end(); ++iter) {
        m_cbxPolicyNames.AddString(iter->c_str());
      }

      // Select Default
      m_cbxPolicyNames.SelectString(-1, cs_default);

      // and check the box
      ((CButton *)GetDlgItem(IDC_USENAMED_POLICY))->SetCheck(BST_CHECKED);

      // Centre the OK button & change its text
      RECT rc, rcOK;
      CWnd *pOK = GetDlgItem(IDOK);

      GetClientRect(&rc);
      pOK->GetWindowRect(&rcOK);
      ScreenToClient(&rcOK);
      int top = rcOK.top;
      pOK->GetClientRect(&rcOK);
      int left = (rc.right - rcOK.right) / 2;
      pOK->MoveWindow(left, top, rcOK.right, rcOK.bottom);

      CString cs_close(MAKEINTRESOURCE(IDS_CLOSE));
      pOK->SetWindowText(cs_close);

      // Hide the Cancel & Help buttons
      GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
      GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
      GetDlgItem(ID_HELP)->EnableWindow(FALSE);
      GetDlgItem(ID_HELP)->ShowWindow(SW_HIDE);

      Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
      m_ex_password.SetSecure(false);

      // Remove password character so that the password is displayed
      m_ex_password.SetPasswordChar(0);

      // Load bitmaps
      UINT nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
        IDB_COPYPASSWORD_NEW : IDB_COPYPASSWORD_CLASSIC;
      m_bImageLoaded = m_CopyPswdBitmap.Attach(
                    ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                    MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                    (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

      ASSERT(m_bImageLoaded);
      if (m_bImageLoaded) {
        FixBitmapBackground(m_CopyPswdBitmap);
      }

      nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
        IDB_COPYPASSWORD_NEW_D : IDB_COPYPASSWORD_CLASSIC_D;

      m_bDisabledImageLoaded = m_DisabledCopyPswdBitmap.Attach(
        ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
          MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
          (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

      ASSERT(m_bDisabledImageLoaded);
      if (m_bDisabledImageLoaded) {
        FixBitmapBackground(m_DisabledCopyPswdBitmap);
        m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
      }
      break;
    }
  case IDS_PSWDPOLICY:
    // Set correct window title
    cs_title.LoadString(m_policyname.IsEmpty() ? IDS_ADD_NAMED_POLICY : IDS_EDIT_NAMED_POLICY);
    SetWindowText(cs_title);

    // These are only used in Manage -> Password Policy
    m_pCopyBtn->EnableWindow(FALSE);
    m_pCopyBtn->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
    GetDlgItem(IDC_GENERATEPASSWORD)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
    GetDlgItem(IDC_PASSWORD)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_POLICYLIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_POLICYLIST)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_USENAMED_POLICY)->EnableWindow(FALSE);
    GetDlgItem(IDC_USENAMED_POLICY)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_NAMEDPOLICY)->ShowWindow(SW_SHOW);

    if (!m_policyname.IsEmpty() && m_iter->second.usecount > 0) {
      // Cannot edit the policy 'Name' if it is present and use count > 0
      m_PolicyNameEdit.SetReadOnly(TRUE);
      m_PolicyNameEdit.EnableWindow(FALSE);
    }

    m_PolicyNameEdit.SetWindowText(m_policyname);
    // Max. length of policy name is 255 - only one byte used for length
    // in database header
    m_PolicyNameEdit.SetLimitText(255);
    break;
  }

  setupBuddy(this, IDC_PWLENSPIN, IDC_DEFPWLENGTH, m_PWDefaultLength, PWSprefs::PWDefaultLength);
  setupBuddy(this, IDC_SPINDIGITS, IDC_MINDIGITLENGTH, m_PWDigitMinLength, PWSprefs::PWDigitMinLength);
  setupBuddy(this, IDC_SPINLOWERCASE, IDC_MINLOWERLENGTH, m_PWLowerMinLength, PWSprefs::PWLowercaseMinLength);
  setupBuddy(this, IDC_SPINUPPERCASE, IDC_MINUPPERLENGTH, m_PWUpperMinLength, PWSprefs::PWUppercaseMinLength);
  setupBuddy(this, IDC_SPINSYMBOLS, IDC_MINSYMBOLLENGTH, m_PWSymbolMinLength, PWSprefs::PWSymbolMinLength);

  m_save[SAVE_LOWERCASE] = m_PWUseLowercase;
  m_save[SAVE_UPPERCASE] = m_PWUseUppercase;
  m_save[SAVE_DIGITS] = m_PWUseDigits;
  m_save[SAVE_SYMBOLS] = m_PWUseSymbols;
  m_save[SAVE_EASYVISION] = m_PWEasyVision;
  m_save[SAVE_PRONOUNCEABLE] = m_PWMakePronounceable;

  m_savelen[SAVE_LOWERCASE] = m_PWLowerMinLength;
  m_savelen[SAVE_UPPERCASE] = m_PWUpperMinLength;
  m_savelen[SAVE_DIGITS] = m_PWDigitMinLength;
  m_savelen[SAVE_SYMBOLS] = m_PWSymbolMinLength;

  // Set up the correct controls (enabled/disabled)
  do_hex(m_PWUseHexdigits == TRUE);

  int iSet = EVPR_NONE;
  if (m_PWEasyVision == TRUE)
    iSet = EVPR_EV;
  else
    if (m_PWMakePronounceable == TRUE)
      iSet = EVPR_PR;

  do_easyorpronounceable(iSet);

  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(m_PWUseSymbols);
  GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(m_PWUseSymbols);

  m_SymbolsEdit.SetWindowText(m_Symbols);

  if (m_uicaller == IDS_GENERATEPASSWORD) {
    // Disable Specific policy controls as default is to use a named policy (database default)
    SetSpecificPolicyControls(FALSE);

    // Set up Tooltips
    if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
      m_Help1.Init(IDB_QUESTIONMARK);

      AddTool(IDC_COPYPASSWORDHELP, IDS_CLICKTOCOPYGENPSWD);
      ActivateToolTip();
    } else {
      m_Help1.EnableWindow(FALSE);
      m_Help1.ShowWindow(SW_HIDE);
    }
  }

  // Set appropriate focus
  GotoDlgCtrl(GetDlgItem(m_uicaller == IDS_GENERATEPASSWORD ? IDC_GENERATEPASSWORD : IDCANCEL));
  return FALSE;
}

BOOL CPasswordPolicyDlg::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  // Don't even look like it was pressed if it should be disabled
  if (pMsg->message == WM_LBUTTONDOWN && pMsg->hwnd == m_pCopyBtn->GetSafeHwnd() &&
      !m_bCopyPasswordEnabled) {
    return TRUE;
  }

  // Don't even process double click - looks bad
  if (pMsg->message == WM_LBUTTONDBLCLK && pMsg->hwnd == m_pCopyBtn->GetSafeHwnd()) {
    return TRUE;
  }


  if (pMsg->message == WM_KEYDOWN && 
      (pMsg->wParam == VK_F1 || pMsg->wParam == VK_ESCAPE)) {
    PostMessage(WM_COMMAND, MAKELONG(pMsg->wParam == VK_F1 ? ID_HELP : IDCANCEL, BN_CLICKED), NULL);
    return TRUE;
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CPasswordPolicyDlg::OnOK()
{
  if (m_bReadOnly) {
    CPWDialog::OnCancel();
    return;
  }

  // If in edit (database default or named policy) and it fails validation - don't exit
  if (m_uicaller != IDS_GENERATEPASSWORD && !UpdatePasswordPolicy())
    return;

  CPWDialog::OnOK();
}

void CPasswordPolicyDlg::OnCancel()
{
  // If user is just generating passwords, nothing to save.
  if (m_uicaller == IDS_GENERATEPASSWORD)
    goto exit;
  
  
  // Get new symbols (if any)
  m_SymbolsEdit.GetWindowText(m_Symbols);

  // Check if any changes made
  if (m_PWDefaultLength     != m_oldPWDefaultLength     ||
      m_PWUseLowercase      != m_oldPWUseLowercase      ||
      (m_oldPWUseLowercase  == TRUE &&
       m_PWLowerMinLength   != m_oldPWLowerMinLength)   ||
      m_PWUseUppercase      != m_oldPWUseUppercase      ||
      (m_oldPWUseUppercase  == TRUE &&
       m_PWUpperMinLength   != m_oldPWUpperMinLength)   ||
      m_PWUseDigits         != m_oldPWUseDigits         ||
      (m_oldPWUseDigits     == TRUE &&
       m_PWDigitMinLength   != m_oldPWDigitMinLength)   ||
      m_PWUseSymbols        != m_oldPWUseSymbols        ||
      (m_oldPWUseSymbols    == TRUE &&
       m_PWSymbolMinLength  != m_oldPWSymbolMinLength)  ||
      m_PWEasyVision        != m_oldPWEasyVision        ||
      m_PWUseHexdigits      != m_oldPWUseHexdigits      ||
      m_PWMakePronounceable != m_oldPWMakePronounceable ||
      m_Symbols             != m_oldSymbols) {
    // Confirm Cancel from User
    CGeneralMsgBox gmb;
    if (gmb.AfxMessageBox(IDS_AREYOUSURE_OPT,
                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
      return;
  }

exit:
  CPWDialog::OnCancel();
}

void CPasswordPolicyDlg::OnHelp()
{
  ShowHelp(L"::/html/password_policies.html");
}

void CPasswordPolicyDlg::SetPolicyData(CString &cs_policyname,
                                       PSWDPolicyMap &MapPSWDPLC)
{
  m_MapPSWDPLC = MapPSWDPLC;
  m_policyname = m_oldpolicyname = cs_policyname;

  // Only called for Password Policies
  if (m_uicaller != IDS_PSWDPOLICY)
    return;

  m_iter = m_policyname.IsEmpty() ? m_MapPSWDPLC.end() :
                                    m_MapPSWDPLC.find(StringX((LPCWSTR)m_policyname));

  // Check the find() worked above - if PolicyName not empty, it must be in the map!
  if (!m_policyname.IsEmpty())
    ASSERT(m_iter != m_MapPSWDPLC.end());

  // If New, use default values; otherwise use this policy's values
  PWPolicy xst_pp = m_policyname.IsEmpty() ? m_st_default_pp : m_iter->second;

  m_PWUseLowercase = m_oldPWUseLowercase =
    (xst_pp.flags & PWPolicy::UseLowercase) ==
                       PWPolicy::UseLowercase;
  m_PWUseUppercase = m_oldPWUseUppercase =
    (xst_pp.flags & PWPolicy::UseUppercase) ==
                       PWPolicy::UseUppercase;
  m_PWUseDigits = m_oldPWUseDigits =
    (xst_pp.flags & PWPolicy::UseDigits) ==
                       PWPolicy::UseDigits;
  m_PWUseSymbols = m_oldPWUseSymbols =
    (xst_pp.flags & PWPolicy::UseSymbols) ==
                       PWPolicy::UseSymbols;
  m_PWUseHexdigits = m_oldPWUseHexdigits =
    (xst_pp.flags & PWPolicy::UseHexDigits) ==
                       PWPolicy::UseHexDigits;
  m_PWEasyVision = m_oldPWEasyVision =
    (xst_pp.flags & PWPolicy::UseEasyVision) ==
                       PWPolicy::UseEasyVision;
  m_PWMakePronounceable = m_oldPWMakePronounceable =
    (xst_pp.flags & PWPolicy::MakePronounceable) ==
                       PWPolicy::MakePronounceable;
  m_PWDefaultLength = m_oldPWDefaultLength = xst_pp.length;
  m_PWDigitMinLength = m_oldPWDigitMinLength = xst_pp.digitminlength;
  m_PWLowerMinLength = m_oldPWLowerMinLength = xst_pp.lowerminlength;
  m_PWSymbolMinLength = m_oldPWSymbolMinLength = xst_pp.symbolminlength;
  m_PWUpperMinLength = m_oldPWUpperMinLength = xst_pp.upperminlength;

  CString cs_symbols = xst_pp.symbols.c_str();
  if (m_PWUseSymbols &&!cs_symbols.IsEmpty())
    m_Symbols = m_oldSymbols = cs_symbols;
}

void CPasswordPolicyDlg::OnNamesComboChanged()
{
  UpdateData(TRUE);

  int index = m_cbxPolicyNames.GetCurSel();
  m_cbxPolicyNames.GetLBText(index, m_policyname);

  // We need to update the controls' settings based on the selected
  // Named Password Policy or the Database Default Policy

  PWPolicy xst_pp;
  const CString defpol(MAKEINTRESOURCE(IDSC_DEFAULT_POLICY));

  if (m_policyname == defpol || m_policyname.IsEmpty()) {
    // m_policyname shouldn't be empty here, but JIC...
    xst_pp = m_st_default_pp;
  } else { // ! default policy, must be in map
    m_iter = m_MapPSWDPLC.find(StringX((LPCWSTR)m_policyname));
    ASSERT(m_iter != m_MapPSWDPLC.end());
    if (m_iter == m_MapPSWDPLC.end()) // if the impossible happens, at least don't crash
      return;
    xst_pp = m_iter->second;
  }

  // First disable them all
  SetSpecificPolicyControls(FALSE);

  // Now set values
  m_PWUseLowercase = (xst_pp.flags & PWPolicy::UseLowercase) != 0;
  m_PWUseUppercase = (xst_pp.flags & PWPolicy::UseUppercase) != 0;
  m_PWUseDigits = (xst_pp.flags & PWPolicy::UseDigits) != 0;
  m_PWUseSymbols = (xst_pp.flags & PWPolicy::UseSymbols) != 0;
  m_PWUseHexdigits = (xst_pp.flags & PWPolicy::UseHexDigits) != 0;
  m_PWEasyVision = (xst_pp.flags & PWPolicy::UseEasyVision) != 0;
  m_PWMakePronounceable = (xst_pp.flags & PWPolicy::MakePronounceable) != 0;
  
  m_PWDefaultLength = xst_pp.length;
  m_PWDigitMinLength = xst_pp.digitminlength;
  m_PWLowerMinLength = xst_pp.lowerminlength;
  m_PWSymbolMinLength = xst_pp.symbolminlength;
  m_PWUpperMinLength = xst_pp.upperminlength;

  m_Symbols = m_oldSymbols = xst_pp.symbols.c_str();
  m_SymbolsEdit.SetWindowText(m_Symbols);

  UpdateData(FALSE);
}

void CPasswordPolicyDlg::do_hex(const bool bHex)
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
      (*m_pnonHex[i]) = 0; // zero variable - otherwise Hexcheck wont work
    }

    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      GetDlgItem(id)->SetWindowText(L"0");
      GetDlgItem(id)->EnableWindow(FALSE);

      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(FALSE);
    }

    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(FALSE);

    m_savelen[SAVE_LOWERCASE] = m_PWLowerMinLength;
    m_savelen[SAVE_UPPERCASE] = m_PWUpperMinLength;
    m_savelen[SAVE_DIGITS] = m_PWDigitMinLength;
    m_savelen[SAVE_SYMBOLS] = m_PWSymbolMinLength;
  } else {
    // non-hex, restore state
    // Enable non-hex controls and restore checked state
    for (i = 0; i < N_NOHEX; i++) {
      UINT id = nonHex[i];
      GetDlgItem(id)->EnableWindow(TRUE);
      ((CButton*)GetDlgItem(id))->SetCheck(m_save[i]);
      (*m_pnonHex[i]) = m_save[i]; // Restore variable
    }

    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      cs_value.Format(L"%u", m_savelen[i]);
      GetDlgItem(id)->SetWindowText(cs_value);
      GetDlgItem(id)->EnableWindow(m_save[i]);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(m_save[i]);
    }

    BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                    m_PWEasyVision == FALSE && m_PWMakePronounceable == FALSE) ?
                      TRUE : FALSE;
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(bEnable);

    m_PWLowerMinLength = m_savelen[SAVE_LOWERCASE];
    m_PWUpperMinLength = m_savelen[SAVE_UPPERCASE];
    m_PWDigitMinLength = m_savelen[SAVE_DIGITS];
    m_PWSymbolMinLength = m_savelen[SAVE_SYMBOLS];
  }
}

void CPasswordPolicyDlg::do_easyorpronounceable(const int iSet)
{
  // Can't have at-least for Pronounceables!
  if ((m_PWMakePronounceable == TRUE) &&
      (m_PWDigitMinLength > 1  || m_PWLowerMinLength > 1 ||
       m_PWSymbolMinLength > 1 || m_PWUpperMinLength > 1)) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTSPECIFYMINNUMBER);
  }

  CString cs_value;
  int i;
  if (iSet == EVPR_PR) {
    // Don't support at-least for Pronounceables
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->EnableWindow(FALSE);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[2 * i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[2 * i + 1])->EnableWindow(FALSE);
    }

    m_savelen[SAVE_LOWERCASE] = m_PWLowerMinLength;
    m_savelen[SAVE_UPPERCASE] = m_PWUpperMinLength;
    m_savelen[SAVE_DIGITS] = m_PWDigitMinLength;
    m_savelen[SAVE_SYMBOLS] = m_PWSymbolMinLength;
  } else { // we're good for at-least
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->EnableWindow(TRUE);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(TRUE);
      GetDlgItem(LenTxts[2 * i])->EnableWindow(TRUE);
      GetDlgItem(LenTxts[2 * i + 1])->EnableWindow(TRUE);
    }
  }
  if (iSet != EVPR_NONE) {
    BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED) ? TRUE : FALSE;
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(TRUE);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(bEnable);
  }
}

// Following from AddEdit_PasswordPolicy - TBD - move to common mixin class
void CPasswordPolicyDlg::do_useX(UseX x)
{
  const struct { int cb; int edit; int spin; int left; int right;} controls[] = {
    {IDC_USELOWERCASE, IDC_MINLOWERLENGTH,  IDC_SPINLOWERCASE, IDC_STATIC_LC1, IDC_STATIC_LC2},
    {IDC_USEUPPERCASE, IDC_MINUPPERLENGTH,  IDC_SPINUPPERCASE, IDC_STATIC_UC1, IDC_STATIC_UC2},
    {IDC_USEDIGITS,    IDC_MINDIGITLENGTH,  IDC_SPINDIGITS,    IDC_STATIC_DG1, IDC_STATIC_DG2},
    {IDC_USESYMBOLS,   IDC_MINSYMBOLLENGTH, IDC_SPINSYMBOLS,   IDC_STATIC_SY1, IDC_STATIC_SY2},
  };

  UnselectNamedPolicy();
  UpdateData(TRUE);

  BOOL bChecked = (IsDlgButtonChecked(controls[x].cb) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(controls[x].edit)->EnableWindow(bChecked);
  GetDlgItem(controls[x].spin)->EnableWindow(bChecked);
  GetDlgItem(controls[x].left)->EnableWindow(bChecked);
  GetDlgItem(controls[x].right)->EnableWindow(bChecked);
  if (x == USESYM) {
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(bChecked);
    GetDlgItem(IDC_RESET_SYMBOLS)->EnableWindow(bChecked);
  }

  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnUseLowerCase()
{
  do_useX(USELOWER);
}

void CPasswordPolicyDlg::OnUseUpperCase()
{
  do_useX(USEUPPER);
}

void CPasswordPolicyDlg::OnUseDigits()
{
  do_useX(USEDIGITS);
}

void CPasswordPolicyDlg::OnUseSymbols()
{
  do_useX(USESYM);
  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnUseHexdigits()
{
  UnselectNamedPolicy();
  UpdateData(TRUE);

  do_hex(IsDlgButtonChecked(IDC_USEHEXDIGITS) == BST_CHECKED);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_hex" will be undone
}

void CPasswordPolicyDlg::OnEasyVision()
{
  UnselectNamedPolicy();
  UpdateData(TRUE);

  if (m_PWEasyVision && m_PWMakePronounceable) {
    CGeneralMsgBox gmb;
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_PWEasyVision = FALSE;
    return;
  }

  const bool bChecked = (IsDlgButtonChecked(IDC_EASYVISION) == BST_CHECKED);

  do_easyorpronounceable(bChecked ? EVPR_EV : EVPR_NONE);  
  do_reset_symbols(false);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

void CPasswordPolicyDlg::OnMakePronounceable()
{
  UnselectNamedPolicy();
  UpdateData(TRUE);

  if (m_PWEasyVision && m_PWMakePronounceable) {
    CGeneralMsgBox gmb;
    ((CButton*)GetDlgItem(IDC_PRONOUNCEABLE))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_PWMakePronounceable = FALSE;
    return;
  }

  const bool bChecked = (IsDlgButtonChecked(IDC_PRONOUNCEABLE) == BST_CHECKED);

  if (m_PWUseLowercase == FALSE && m_PWUseUppercase == FALSE) {
    CGeneralMsgBox gmb;
    ((CButton*)GetDlgItem(IDC_PRONOUNCEABLE))->SetCheck(FALSE);
    gmb.AfxMessageBox(IDS_PR_MUSTHAVECHARACTERS);
    m_PWMakePronounceable = FALSE;
    return;
  }

  do_easyorpronounceable(bChecked ? EVPR_PR : EVPR_NONE);  
  do_reset_symbols(false);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

BOOL CPasswordPolicyDlg::Validate()
{
  CGeneralMsgBox gmb;
  // Check that options, as set, are valid.
  if (m_PWUseHexdigits &&
     (m_PWUseLowercase || m_PWUseUppercase || m_PWUseDigits ||
      m_PWUseSymbols || m_PWEasyVision || m_PWMakePronounceable)) {
    gmb.AfxMessageBox(IDS_HEXMUTUALLYEXCL);
    return FALSE;
  }

  if (m_PWUseHexdigits) {
    if (m_PWDefaultLength % 2 != 0) {
      gmb.AfxMessageBox(IDS_HEXMUSTBEEVEN);
      return FALSE;
    }
  } else
  if (!m_PWUseLowercase && !m_PWUseUppercase &&
      !m_PWUseDigits && !m_PWUseSymbols) {
    gmb.AfxMessageBox(IDS_MUSTHAVEONEOPTION);
    return FALSE;
  }

  int minPWL = PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::PWDefaultLength);
  int maxPWL = PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::PWDefaultLength);
  if ((m_PWDefaultLength < minPWL) || (m_PWDefaultLength > maxPWL)) {
    CString errmess;
    errmess.Format(IDS_DEFAULTPWLENGTH, minPWL, maxPWL);
    gmb.AfxMessageBox(errmess);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if (!(m_PWUseHexdigits || m_PWEasyVision || m_PWMakePronounceable) &&
      ((m_PWUseDigits ? m_PWDigitMinLength : 0) +
       (m_PWUseLowercase ? m_PWLowerMinLength : 0) +
       (m_PWUseSymbols ? m_PWSymbolMinLength : 0) +
       (m_PWUseUppercase ? m_PWUpperMinLength : 0)) > m_PWDefaultLength) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTHTOOSMALL);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if (m_uicaller == IDS_PSWDPOLICY) {
    // Only need to check if policy name is empty or it has been changed
    // from the old name and the new name is already defined
    // Note: If editing default password policy: m_uicaller == I.DS_OPTIONS
    UINT uiMsg = 0;
    if (m_policyname.IsEmpty())
      uiMsg = IDS_INVALIDPOLICYNAME1;
    else
      if ((m_policyname != m_oldpolicyname &&
         (m_MapPSWDPLC.find((LPCWSTR)m_policyname) != m_MapPSWDPLC.end())))
        uiMsg = IDS_INVALIDPOLICYNAME2;
    
    if (uiMsg != 0) {
      gmb.AfxMessageBox(uiMsg);
      ((CEdit*)GetDlgItem(IDC_POLICYNAME))->SetFocus();
      return FALSE;
    }
  }

  //End check
  return TRUE;
}

void CPasswordPolicyDlg::OnGeneratePassword()
{
  UpdateData(TRUE);

  PWPolicy st_pp;

  if (m_UseNamedPolicy == BST_UNCHECKED) {
    // Use specific policy but validate it first
    if (Validate() == FALSE)
      return;

    if (m_PWUseLowercase == TRUE)
      st_pp.flags |= PWPolicy::UseLowercase;
    if (m_PWUseUppercase == TRUE)
      st_pp.flags |= PWPolicy::UseUppercase;
    if (m_PWUseDigits == TRUE)
      st_pp.flags |= PWPolicy::UseDigits;
    if (m_PWUseSymbols == TRUE)
      st_pp.flags |= PWPolicy::UseSymbols;
    if (m_PWUseHexdigits == TRUE)
      st_pp.flags |= PWPolicy::UseHexDigits;
    if (m_PWEasyVision == TRUE)
      st_pp.flags |= PWPolicy::UseEasyVision;
    if (m_PWMakePronounceable == TRUE)
      st_pp.flags |= PWPolicy::MakePronounceable;
    st_pp.length = m_PWDefaultLength;
    st_pp.digitminlength = m_PWDigitMinLength;
    st_pp.lowerminlength = m_PWLowerMinLength;
    st_pp.symbolminlength = m_PWSymbolMinLength;
    st_pp.upperminlength = m_PWUpperMinLength;

    m_SymbolsEdit.GetWindowText(m_Symbols);
    st_pp.symbols = LPCWSTR(m_Symbols);
  } else {
    // Use named policy
    if (m_PolicyNameEdit.IsWindowVisible()) {
      m_PolicyNameEdit.GetWindowText((CString &)m_policyname);
    } else {
      int index = m_cbxPolicyNames.GetCurSel();
      m_cbxPolicyNames.GetLBText(index, m_policyname);
    }
    StringX sxPolicyName(m_policyname);
    GetMainDlg()->GetPolicyFromName(sxPolicyName, st_pp);
  }

  StringX passwd;
  GetMainDlg()->MakeRandomPassword(passwd, st_pp);
  m_password = passwd.c_str();
  m_ex_password.SetWindowText(m_password);
  m_ex_password.Invalidate();

  m_bCopyPasswordEnabled = m_password.GetLength() > 0;

  // Enable/Disable Copy to Clipboard
  m_pCopyBtn->SetBitmap(m_bCopyPasswordEnabled ? m_CopyPswdBitmap : m_DisabledCopyPswdBitmap);

  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnCopyPassword()
{
  if (!m_bCopyPasswordEnabled)
    return;

  UpdateData(TRUE);

  GetMainDlg()->SetClipboardData(m_password);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);

  CString csMessage(MAKEINTRESOURCE(IDS_PASSWORDCOPIED));
  m_stcMessage.SetWindowText(csMessage);
}

void CPasswordPolicyDlg::OnENChangePassword()
{
  UpdateData(TRUE);

  m_ex_password.GetWindowText((CString &)m_password);

  m_bCopyPasswordEnabled = m_password.GetLength() > 0;

  // Enable/Disable Copy to Clipboard
  m_pCopyBtn->SetBitmap(m_bCopyPasswordEnabled ? m_CopyPswdBitmap : m_DisabledCopyPswdBitmap);

  m_stcMessage.SetWindowText(L"");
}

void CPasswordPolicyDlg::OnENChangePolicyName()
{
  UpdateData(TRUE);

  m_PolicyNameEdit.GetWindowText((CString &)m_policyname);
}

void CPasswordPolicyDlg::OnENOwnSymbols()
{
  UpdateData(TRUE);

  CString cs_symbols;
  m_SymbolsEdit.GetWindowText(cs_symbols);

  // Check if user about to leave own symbols empty
  if (cs_symbols.GetLength() == 0) {
    // Tell user
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_OWNSYMBOLSMISSING, MB_ICONEXCLAMATION);

    // Select Default symbols - then uncheck use symbols
    ((CButton *)GetDlgItem(IDC_USESYMBOLS))->SetCheck(BST_UNCHECKED);

    // Setup variables etc.
    OnUseSymbols();
  }
}

void CPasswordPolicyDlg::OnUseNamedPolicy()
{
  BOOL bEnable = ((CButton *)GetDlgItem(IDC_USENAMED_POLICY))->GetCheck() == BST_CHECKED ?
                           TRUE : FALSE;

  m_cbxPolicyNames.EnableWindow(bEnable);
  SetSpecificPolicyControls(1 - bEnable);  // Assumes TRUE == 1; FALSE == 0
}

void CPasswordPolicyDlg::SetSpecificPolicyControls(const BOOL bEnable)
{
  if (bEnable == FALSE) {
    // Disable all specific controls after saving info
    m_save[SAVE_LOWERCASE] = m_PWUseLowercase;
    m_save[SAVE_UPPERCASE] = m_PWUseUppercase;
    m_save[SAVE_DIGITS] = m_PWUseDigits;
    m_save[SAVE_SYMBOLS] = m_PWUseSymbols;
    m_save[SAVE_EASYVISION] = m_PWEasyVision;
    m_save[SAVE_PRONOUNCEABLE] = m_PWMakePronounceable;
    m_savelen[SAVE_LOWERCASE] = m_PWLowerMinLength;
    m_savelen[SAVE_UPPERCASE] = m_PWUpperMinLength;
    m_savelen[SAVE_DIGITS] = m_PWDigitMinLength;
    m_savelen[SAVE_SYMBOLS] = m_PWSymbolMinLength;

    for (int i = 0; i < N_HEX_LENGTHS * 2; i++) {
      GetDlgItem(LenTxts[i])->EnableWindow(FALSE);
    }

    for (int i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->EnableWindow(FALSE);
    }

    for (int i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
    }

    for (int i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHex[i])->EnableWindow(FALSE);
    }

    GetDlgItem(IDC_EASYVISION)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
    GetDlgItem(IDC_PRONOUNCEABLE)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFPWLENGTH)->EnableWindow(FALSE);

    // Disable Symbols
    m_SymbolsEdit.EnableWindow(FALSE);
  } else {
    // Restore previous values
    m_PWUseLowercase = m_save[SAVE_LOWERCASE];
    m_PWUseUppercase = m_save[SAVE_UPPERCASE];
    m_PWUseDigits = m_save[SAVE_DIGITS];
    m_PWUseSymbols = m_save[SAVE_SYMBOLS];
    m_PWEasyVision = m_save[SAVE_EASYVISION];
    m_PWMakePronounceable = m_save[SAVE_PRONOUNCEABLE];
    m_PWLowerMinLength = m_savelen[SAVE_LOWERCASE];
    m_PWUpperMinLength = m_savelen[SAVE_UPPERCASE];
    m_PWDigitMinLength = m_savelen[SAVE_DIGITS];
    m_PWSymbolMinLength = m_savelen[SAVE_SYMBOLS];

    // Enable lengths
    GetDlgItem(IDC_STATIC_PSWDLENGTH)->EnableWindow(TRUE);
    GetDlgItem(IDC_DEFPWLENGTH)->EnableWindow(TRUE);

    // Set up the correct controls (enabled/disabled)
    do_hex(m_PWUseHexdigits == TRUE);
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);

    int iSet = EVPR_NONE;
    if (m_PWEasyVision == TRUE)
      iSet = EVPR_EV;
    else
    if (m_PWMakePronounceable == TRUE)
      iSet = EVPR_PR;
    do_easyorpronounceable(iSet);
  }
}

bool CPasswordPolicyDlg::UpdatePasswordPolicy()
{
  UpdateData(TRUE);

  // Validate
  if (Validate() == FALSE)
    return false;

  // Get new symbols (if any)
  m_SymbolsEdit.GetWindowText(m_Symbols);

  PWPolicy st_pp;
  st_pp.flags = 0;
  if (m_PWUseLowercase == TRUE)
    st_pp.flags |= PWPolicy::UseLowercase;
  if (m_PWUseUppercase == TRUE)
    st_pp.flags |= PWPolicy::UseUppercase;
  if (m_PWUseDigits == TRUE)
    st_pp.flags |= PWPolicy::UseDigits;
  if (m_PWUseSymbols == TRUE)
    st_pp.flags |= PWPolicy::UseSymbols;
  if (m_PWUseHexdigits == TRUE)
    st_pp.flags |= PWPolicy::UseHexDigits;
  if (m_PWEasyVision == TRUE)
    st_pp.flags |= PWPolicy::UseEasyVision;
  if (m_PWMakePronounceable == TRUE)
    st_pp.flags |= PWPolicy::MakePronounceable;

  st_pp.length = m_PWDefaultLength;
  st_pp.digitminlength = m_PWDigitMinLength;
  st_pp.lowerminlength = m_PWLowerMinLength;
  st_pp.symbolminlength = m_PWSymbolMinLength;
  st_pp.upperminlength = m_PWUpperMinLength;

  st_pp.symbols = m_Symbols;

  if (m_uicaller == IDS_OPTIONS) {
    // Update database default
    m_st_default_pp = st_pp;
  } else {
    ASSERT(!m_policyname.IsEmpty());
    // Update a named policy
    StringX sxPolicyName(m_policyname), sx_OldPolicyName(m_oldpolicyname);
    if (!sx_OldPolicyName.empty()) {
      // Edit of an old entry
      PSWDPolicyMapIter iter = m_MapPSWDPLC.find(sx_OldPolicyName);

      // Get and reset use count
      if (iter != m_MapPSWDPLC.end())
        st_pp.usecount = iter->second.usecount;

      if (m_policyname != m_oldpolicyname) {
        // Delete old policy changing name
        m_MapPSWDPLC.erase(iter);
      }
    }

    // Insert the new name
    m_MapPSWDPLC[sxPolicyName] = st_pp;
  }
  return true;
}

void CPasswordPolicyDlg::UnselectNamedPolicy()
{
  ((CButton *)GetDlgItem(IDC_USENAMED_POLICY))->SetCheck(BST_UNCHECKED);
  m_cbxPolicyNames.EnableWindow(FALSE);
}

void CPasswordPolicyDlg::OnSymbolReset()
{
  do_reset_symbols(true);
}

void CPasswordPolicyDlg::do_reset_symbols(bool restore_defaults)
{
  std::wstring st_symbols;
  if (m_PWEasyVision == TRUE)
    st_symbols = CPasswordCharPool::GetEasyVisionSymbols();
  else if (m_PWMakePronounceable == TRUE)
    st_symbols = CPasswordCharPool::GetPronounceableSymbols();
  else {
    if (restore_defaults)
      CPasswordCharPool::ResetDefaultSymbols(); // restore the preference!
    st_symbols = CPasswordCharPool::GetDefaultSymbols();
  }

  m_SymbolsEdit.SetValidSym(st_symbols);
  m_SymbolsEdit.SetWindowText(st_symbols.c_str());
  m_Symbols = st_symbols.c_str();
}
