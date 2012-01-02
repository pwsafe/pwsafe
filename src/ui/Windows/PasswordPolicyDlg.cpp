/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PasswordPolicyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "PasswordPolicyDlg.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"    // For Help

#include "core/PWCharPool.h"
#include "core/PwsPlatform.h"
#include "core/PWSprefs.h"

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
// CPasswordPolicyDlg property page

/*

  No need to colour all the controls blue as everything in this dialog
  affects the database - whether the default password policy in the preferences
  or the named password policies in the header.
  
*/

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
                                       bool bReadOnly, st_PSWDPolicy &st_default_pp)
  : CPWDialog(bLongPPs ? CPasswordPolicyDlg::IDD : CPasswordPolicyDlg::IDD_SHORT, pParent),
  m_uicaller(uicaller), m_bReadOnly(bReadOnly), m_pDbx(NULL), m_password(L""),
  m_UseNamedPolicy(FALSE), m_st_default_pp(st_default_pp)
{
  m_PWUseLowercase = m_oldPWUseLowercase =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyUseLowercase) != 0;
  m_PWUseUppercase = m_oldPWUseUppercase =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyUseUppercase) != 0;
  m_PWUseDigits = m_oldPWUseDigits =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyUseDigits) != 0;
  m_PWUseSymbols = m_oldPWUseSymbols =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyUseSymbols) != 0;
  m_PWUseHexdigits = m_oldPWUseHexdigits =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyUseHexDigits) != 0;
  m_PWEasyVision = m_oldPWEasyVision =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyUseEasyVision) != 0;
  m_PWMakePronounceable = m_oldPWMakePronounceable =
    (m_st_default_pp.pwp.flags & PWSprefs::PWPolicyMakePronounceable) != 0;

  m_PWDefaultLength = m_oldPWDefaultLength = m_st_default_pp.pwp.length;
  m_PWDigitMinLength = m_oldPWDigitMinLength = m_st_default_pp.pwp.digitminlength;
  m_PWLowerMinLength = m_oldPWLowerMinLength = m_st_default_pp.pwp.lowerminlength;
  m_PWSymbolMinLength = m_oldPWSymbolMinLength = m_st_default_pp.pwp.symbolminlength;
  m_PWUpperMinLength = m_oldPWUpperMinLength = m_st_default_pp.pwp.upperminlength;

  m_Symbols = m_oldSymbols = m_st_default_pp.symbols.c_str();
  m_UseOwnSymbols = m_oldUseOwnSymbols = m_Symbols.IsEmpty() ? DEFAULT_SYMBOLS : OWN_SYMBOLS;

  // These must be in the same order as their respective Control IDs
  // in CPasswordPolicyDlg::nonHex
  m_pnonHex[0] = &m_PWUseLowercase;
  m_pnonHex[1] = &m_PWUseUppercase;
  m_pnonHex[2] = &m_PWUseDigits;
  m_pnonHex[3] = &m_PWUseSymbols;
  m_pnonHex[4] = &m_PWEasyVision;
  m_pnonHex[5] = &m_PWMakePronounceable;

  CPasswordCharPool::GetDefaultSymbols(m_std_symbols);
  CPasswordCharPool::GetEasyVisionSymbols(m_easyvision_symbols);
  CPasswordCharPool::GetPronounceableSymbols(m_pronounceable_symbols);
}

CPasswordPolicyDlg::~CPasswordPolicyDlg()
{
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

  DDX_Radio(pDX, IDC_USEDEFAULTSYMBOLS, m_UseOwnSymbols);
  DDX_Control(pDX, IDC_OWNSYMBOLS, (CEdit&)m_SymbolsEdit);

  // Because we can show the generated password when used from Mangage->Generate
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);

  DDX_Control(pDX, IDC_POLICYNAME, m_PolicyNameEdit);
  DDX_Control(pDX, IDC_POLICYLIST, m_cbxPolicyNames);
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

  // Because we can show the generated password when used from Mangage->Generate
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopyPassword)
  ON_EN_CHANGE(IDC_PASSWORD, OnENChangePassword)

  ON_BN_CLICKED(IDC_USEDEFAULTSYMBOLS, OnSymbols)
  ON_BN_CLICKED(IDC_USEOWNSYMBOLS, OnSymbols)

  ON_BN_CLICKED(IDC_USENAMED_POLICY, OnUseNamedPolicy)

  ON_EN_KILLFOCUS(IDC_POLICYNAME, OnENChangePolicyName)

  ON_CBN_SELCHANGE(IDC_POLICYLIST, OnNamesComboChanged)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPasswordPolicyDlg message handlers

#define CR_DATABASE_OPTIONS 0x800000

BOOL CPasswordPolicyDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  if (m_bReadOnly && m_uicaller != IDS_GENERATEPASSWORD) {
    // Change OK button test
    CString cs_close(MAKEINTRESOURCE(IDS_CLOSE));
    GetDlgItem(IDOK)->SetWindowText(cs_close);

    // Hide the Cancel button
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
  }

  // Verify ptr to DboxMain has been set up (call to SetPolicyData)
  ASSERT(m_pDbx != NULL);

  CString cs_title;
  switch (m_uicaller) {
    case IDS_OPTIONS:
      // Set correct window title
      cs_title.LoadString(IDS_EDIT_DEFAULT_POLICY);
      SetWindowText(cs_title);

      // These are only used in Manage -> Generate Password or Add/Edit Policy names
      GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
      GetDlgItem(IDC_GENERATEPASSWORD)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_COPYPASSWORD)->EnableWindow(FALSE);
      GetDlgItem(IDC_COPYPASSWORD)->ShowWindow(SW_HIDE);
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
      m_pDbx->GetPolicyNames(vNames);

      // Add Default
      CString cs_text(MAKEINTRESOURCE(IDS_DATABASE_DEFAULT));
      m_cbxPolicyNames.AddString(cs_text);

      for (std::vector<std::wstring>::iterator iter = vNames.begin();
           iter != vNames.end(); ++iter) {
        m_cbxPolicyNames.AddString(iter->c_str());
      }

      // Select Default
      m_cbxPolicyNames.SetCurSel(0);

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

      ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
      m_ex_password.SetSecure(false);

      // Remove password character so that the password is displayed
      m_ex_password.SetPasswordChar(0);
      break;
    }
    case IDS_PSWDPOLICY:
      // Set correct window title
      cs_title.LoadString(m_policyname.IsEmpty() ? IDS_ADD_NAMED_POLICY : IDS_EDIT_NAMED_POLICY);
      SetWindowText(cs_title);

      // These are only used in Manage -> Password Policy
      GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
      GetDlgItem(IDC_GENERATEPASSWORD)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_COPYPASSWORD)->EnableWindow(FALSE);
      GetDlgItem(IDC_COPYPASSWORD)->ShowWindow(SW_HIDE);
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
      // Max. length of policy name is 255 - only 2 hex digits used for length
      // in database header
      m_PolicyNameEdit.SetLimitText(255);
      break;
  }

  CSpinButtonCtrl *pspin  = (CSpinButtonCtrl *)GetDlgItem(IDC_PWLENSPIN);
  CSpinButtonCtrl *pspinD = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINDIGITS);
  CSpinButtonCtrl *pspinL = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINLOWERCASE);
  CSpinButtonCtrl *pspinS = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINSYMBOLS);
  CSpinButtonCtrl *pspinU = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINUPPERCASE);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
  pspin->SetRange(4, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_PWDefaultLength);

  pspinD->SetBuddy(GetDlgItem(IDC_MINDIGITLENGTH));
  pspinD->SetRange(0, 1024);
  pspinD->SetBase(10);
  pspinD->SetPos(m_PWDigitMinLength);

  pspinL->SetBuddy(GetDlgItem(IDC_MINLOWERLENGTH));
  pspinL->SetRange(0, 1024);
  pspinL->SetBase(10);
  pspinL->SetPos(m_PWLowerMinLength);

  pspinS->SetBuddy(GetDlgItem(IDC_MINSYMBOLLENGTH));
  pspinS->SetRange(0, 1024);
  pspinS->SetBase(10);
  pspinS->SetPos(m_PWSymbolMinLength);

  pspinU->SetBuddy(GetDlgItem(IDC_MINUPPERLENGTH));
  pspinU->SetRange(0, 1024);
  pspinU->SetBase(10);
  pspinU->SetPos(m_PWUpperMinLength);

  m_save[SAVE_LOWERCASE] = m_PWUseLowercase;
  m_save[SAVE_UPPERCASE] = m_PWUseUppercase;
  m_save[SAVE_DIGITS] = m_PWUseDigits;
  m_save[SAVE_SYMBOLS] = m_PWUseSymbols;
  m_save[SAVE_EASYVISION] = m_PWEasyVision;
  m_save[SAVE_PRONOUNCEABLE] = m_PWMakePronounceable;

  if (m_PWUseLowercase == TRUE && m_PWLowerMinLength == 0)
    m_PWLowerMinLength = 1;
  if (m_PWUseUppercase == TRUE && m_PWUpperMinLength == 0)
    m_PWUpperMinLength = 1;
  if (m_PWUseDigits == TRUE && m_PWDigitMinLength == 0)
    m_PWDigitMinLength = 1;
  if (m_PWUseSymbols == TRUE && m_PWSymbolMinLength == 0)
    m_PWSymbolMinLength = 1;

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

  m_SymbolsEdit.SetWindowText(m_Symbols);

  GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(m_PWUseSymbols);
  GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(m_PWUseSymbols);
  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((m_PWUseSymbols == TRUE && m_UseOwnSymbols == OWN_SYMBOLS) ? TRUE : FALSE);

  if (m_uicaller == IDS_GENERATEPASSWORD) {
    // Disable Specific policy controls as default is to use a named policy (database default)
    SetSpecificPolicyControls(FALSE);
  }

  // Set appropriate focus
  GetDlgItem(m_uicaller == IDS_GENERATEPASSWORD ? IDC_GENERATEPASSWORD : IDCANCEL)->SetFocus();
  return FALSE;
}

BOOL CPasswordPolicyDlg::PreTranslateMessage(MSG* pMsg)
{
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
      m_UseOwnSymbols       != m_oldUseOwnSymbols       ||
      (m_UseOwnSymbols      == OWN_SYMBOLS &&
       m_Symbols            != m_oldSymbols)) {
    // Confirm Cancel from User
    CGeneralMsgBox gmb;
    if (gmb.AfxMessageBox(IDS_AREYOUSURE_OPT,
                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
      return;
  }

  CPWDialog::OnCancel();
}

void CPasswordPolicyDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/password_policies.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CPasswordPolicyDlg::SetPolicyData(CString &cs_policyname,
                                       PSWDPolicyMap &MapPSWDPLC,
                                       DboxMain *pDbx)
{
  m_MapPSWDPLC = MapPSWDPLC;
  m_policyname = m_oldpolicyname = cs_policyname;

  // Save ptr to DboxMain
  ASSERT(pDbx != NULL);
  m_pDbx = pDbx;

  // Only called for Password Policies
  if (m_uicaller != IDS_PSWDPOLICY)
    return;

  m_iter = m_policyname.IsEmpty() ? m_MapPSWDPLC.end() :
                                    m_MapPSWDPLC.find(StringX((LPCWSTR)m_policyname));

  // Check the find worked above - if PolicyName not empty, it must be in the map!
  if (!m_policyname.IsEmpty())
    ASSERT(m_iter != m_MapPSWDPLC.end());

  // If New, use default values; otherwise use this policy's values
  st_PSWDPolicy xst_pp = m_policyname.IsEmpty() ? m_st_default_pp : m_iter->second;

  m_PWUseLowercase = m_oldPWUseLowercase =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyUseLowercase) ==
                       PWSprefs::PWPolicyUseLowercase;
  m_PWUseUppercase = m_oldPWUseUppercase =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyUseUppercase) ==
                       PWSprefs::PWPolicyUseUppercase;
  m_PWUseDigits = m_oldPWUseDigits =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyUseDigits) ==
                       PWSprefs::PWPolicyUseDigits;
  m_PWUseSymbols = m_oldPWUseSymbols =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyUseSymbols) ==
                       PWSprefs::PWPolicyUseSymbols;
  m_PWUseHexdigits = m_oldPWUseHexdigits =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyUseHexDigits) ==
                       PWSprefs::PWPolicyUseHexDigits;
  m_PWEasyVision = m_oldPWEasyVision =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyUseEasyVision) ==
                       PWSprefs::PWPolicyUseEasyVision;
  m_PWMakePronounceable = m_oldPWMakePronounceable =
    (xst_pp.pwp.flags & PWSprefs::PWPolicyMakePronounceable) ==
                       PWSprefs::PWPolicyMakePronounceable;
  m_PWDefaultLength = m_oldPWDefaultLength = xst_pp.pwp.length;
  m_PWDigitMinLength = m_oldPWDigitMinLength = xst_pp.pwp.digitminlength;
  m_PWLowerMinLength = m_oldPWLowerMinLength = xst_pp.pwp.lowerminlength;
  m_PWSymbolMinLength = m_oldPWSymbolMinLength = xst_pp.pwp.symbolminlength;
  m_PWUpperMinLength = m_oldPWUpperMinLength = xst_pp.pwp.upperminlength;

  CString cs_symbols = xst_pp.symbols.c_str();
  m_Symbols = m_oldSymbols = cs_symbols;
  m_UseOwnSymbols = m_oldUseOwnSymbols = cs_symbols.IsEmpty() ? DEFAULT_SYMBOLS : OWN_SYMBOLS;
}

void CPasswordPolicyDlg::OnNamesComboChanged()
{
  UpdateData(TRUE);

  int index = m_cbxPolicyNames.GetCurSel();
  m_cbxPolicyNames.GetLBText(index, m_policyname);

  // We need to update the "values" of the controls based on the selected
  // Named Password Policy or the Database Default Policy

  m_iter = m_policyname.IsEmpty() ? m_MapPSWDPLC.end() :
                                    m_MapPSWDPLC.find(StringX((LPCWSTR)m_policyname));

  // Check the find worked above - if PolicyName not empty, it must be in the map!
  if (!m_policyname.IsEmpty())
    ASSERT(m_iter != m_MapPSWDPLC.end());

  // First disable them all
  SetSpecificPolicyControls(FALSE);

  // Now set values
  st_PSWDPolicy xst_pp = index == 0 ? m_st_default_pp : m_iter->second;

  m_PWUseLowercase = (xst_pp.pwp.flags & PWSprefs::PWPolicyUseLowercase) != 0;
  m_PWUseUppercase = (xst_pp.pwp.flags & PWSprefs::PWPolicyUseUppercase) != 0;
  m_PWUseDigits = (xst_pp.pwp.flags & PWSprefs::PWPolicyUseDigits) != 0;
  m_PWUseSymbols = (xst_pp.pwp.flags & PWSprefs::PWPolicyUseSymbols) != 0;
  m_PWUseHexdigits = (xst_pp.pwp.flags & PWSprefs::PWPolicyUseHexDigits) != 0;
  m_PWEasyVision = (xst_pp.pwp.flags & PWSprefs::PWPolicyUseEasyVision) != 0;
  m_PWMakePronounceable = (xst_pp.pwp.flags & PWSprefs::PWPolicyMakePronounceable) != 0;
  
  m_PWDefaultLength = xst_pp.pwp.length;
  m_PWDigitMinLength = xst_pp.pwp.digitminlength;
  m_PWLowerMinLength = xst_pp.pwp.lowerminlength;
  m_PWSymbolMinLength = xst_pp.pwp.symbolminlength;
  m_PWUpperMinLength = xst_pp.pwp.upperminlength;

  m_Symbols = m_oldSymbols = xst_pp.symbols.c_str();
  m_UseOwnSymbols = m_Symbols.IsEmpty() ? DEFAULT_SYMBOLS : OWN_SYMBOLS;

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

    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(FALSE);

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
      cs_value.Format(L"%d", m_savelen[i]);
      GetDlgItem(id)->SetWindowText(cs_value);
      GetDlgItem(id)->EnableWindow(m_save[i]);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i * 2])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i * 2 + 1])->EnableWindow(m_save[i]);
    }

    BOOL bEnable = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                    m_PWEasyVision == FALSE && m_PWMakePronounceable == FALSE) ?
                      TRUE : FALSE;
    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(bEnable);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((bEnable == TRUE &&
                                              m_UseOwnSymbols == OWN_SYMBOLS) ?
                                              TRUE : FALSE);

    m_PWLowerMinLength = m_savelen[SAVE_LOWERCASE];
    m_PWUpperMinLength = m_savelen[SAVE_UPPERCASE];
    m_PWDigitMinLength = m_savelen[SAVE_DIGITS];
    m_PWSymbolMinLength = m_savelen[SAVE_SYMBOLS];
  }
}

void CPasswordPolicyDlg::do_easyorpronounceable(const int iSet)
{
  // Can't have minimum lengths!
  if ((m_PWEasyVision == TRUE  || m_PWMakePronounceable == TRUE) &&
      (m_PWDigitMinLength > 1  || m_PWLowerMinLength > 1 ||
       m_PWSymbolMinLength > 1 || m_PWUpperMinLength > 1)) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTSPECIFYMINNUMBER);
  }

  CString cs_value;
  int i;
  if (iSet != EVPR_NONE) {
    // Make EasyVision or Pronounceable
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->ShowWindow(SW_HIDE);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[2 * i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[2 * i + 1])->ShowWindow(SW_HIDE);
    }

    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(FALSE);

    m_savelen[SAVE_LOWERCASE] = m_PWLowerMinLength;
    m_savelen[SAVE_UPPERCASE] = m_PWUpperMinLength;
    m_savelen[SAVE_DIGITS] = m_PWDigitMinLength;
    m_savelen[SAVE_SYMBOLS] = m_PWSymbolMinLength;
  } else {
    // Unmake EasyVision or Pronounceable
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
    GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((bEnable == TRUE && m_UseOwnSymbols == OWN_SYMBOLS) ?
                                             TRUE : FALSE);

    m_PWLowerMinLength = m_savelen[SAVE_LOWERCASE];
    m_PWUpperMinLength = m_savelen[SAVE_UPPERCASE];
    m_PWDigitMinLength = m_savelen[SAVE_DIGITS];
    m_PWSymbolMinLength = m_savelen[SAVE_SYMBOLS];
  }
  
  stringT *pst_symbols;
  if (m_PWEasyVision == TRUE)
    pst_symbols = &m_easyvision_symbols;
  else if (m_PWMakePronounceable == TRUE)
    pst_symbols = &m_pronounceable_symbols;
  else
    pst_symbols = &m_std_symbols;

  GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->SetWindowText((*pst_symbols).c_str());
}

void CPasswordPolicyDlg::OnUseLowerCase()
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USELOWERCASE) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINLOWERLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINLOWERCASE)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_LC1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_LC2)->EnableWindow(bChecked);
  m_PWLowerMinLength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnUseUpperCase()
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USEUPPERCASE) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINUPPERLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINUPPERCASE)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_UC1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_UC2)->EnableWindow(bChecked);
  m_PWUpperMinLength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnUseDigits()
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USEDIGITS) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINDIGITLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINDIGITS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DG1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DG2)->EnableWindow(bChecked);
  m_PWDigitMinLength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnUseSymbols()
{
  UpdateData(TRUE);

  BOOL bChecked = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED &&
                   m_PWEasyVision == FALSE && m_PWMakePronounceable == FALSE) ? TRUE : FALSE;

  GetDlgItem(IDC_MINSYMBOLLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_SY1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_SY2)->EnableWindow(bChecked);

  GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DEFAULTSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow((bChecked == TRUE && m_UseOwnSymbols == OWN_SYMBOLS) ? TRUE : FALSE);

  m_PWSymbolMinLength = bChecked;  // Based on FALSE=0 & TRUE=1

  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnUseHexdigits()
{
  UpdateData(TRUE);

  do_hex(IsDlgButtonChecked(IDC_USEHEXDIGITS) == BST_CHECKED);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_hex" will be undone
}

void CPasswordPolicyDlg::OnEasyVision()
{
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
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

void CPasswordPolicyDlg::OnMakePronounceable()
{
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
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

void CPasswordPolicyDlg::OnSymbols()
{
  m_UseOwnSymbols = ((CButton *)GetDlgItem(IDC_USEDEFAULTSYMBOLS))->GetCheck() == BST_CHECKED ?
                        DEFAULT_SYMBOLS : OWN_SYMBOLS;

  GetDlgItem(IDC_OWNSYMBOLS)->EnableWindow(m_UseOwnSymbols == DEFAULT_SYMBOLS ? FALSE : TRUE);
  if (m_UseOwnSymbols == OWN_SYMBOLS)
    GetDlgItem(IDC_OWNSYMBOLS)->SetFocus();
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

  if ((m_PWDefaultLength < 4) || (m_PWDefaultLength > 1024)) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTH);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if (!(m_PWUseHexdigits || m_PWEasyVision || m_PWMakePronounceable) &&
      (m_PWDigitMinLength + m_PWLowerMinLength +
       m_PWSymbolMinLength + m_PWUpperMinLength) > m_PWDefaultLength) {
    gmb.AfxMessageBox(IDS_DEFAULTPWLENGTHTOOSMALL);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if ((m_PWUseHexdigits || m_PWEasyVision || m_PWMakePronounceable))
    m_PWDigitMinLength = m_PWLowerMinLength =
       m_PWSymbolMinLength = m_PWUpperMinLength = 1;

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

  stringT st_symbols;

  st_PSWDPolicy st_pp;

  if (m_UseNamedPolicy == BST_UNCHECKED) {
    // Use specific policy but validate it first
    if (Validate() == FALSE)
      return;

    if (m_PWUseLowercase == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseLowercase;
    if (m_PWUseUppercase == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseUppercase;
    if (m_PWUseDigits == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseDigits;
    if (m_PWUseSymbols == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseSymbols;
    if (m_PWUseHexdigits == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
    if (m_PWEasyVision == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
    if (m_PWMakePronounceable == TRUE)
      st_pp.pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
    st_pp.pwp.length = m_PWDefaultLength;
    st_pp.pwp.digitminlength = m_PWDigitMinLength;
    st_pp.pwp.lowerminlength = m_PWLowerMinLength;
    st_pp.pwp.symbolminlength = m_PWSymbolMinLength;
    st_pp.pwp.upperminlength = m_PWUpperMinLength;

    if (m_UseOwnSymbols == OWN_SYMBOLS) {
      m_SymbolsEdit.GetWindowText(m_Symbols);
      st_symbols = LPCWSTR(m_Symbols);
    } else
      CPasswordCharPool::GetDefaultSymbols(st_symbols);

    st_pp.symbols = st_symbols.c_str();
  } else {
    // Use named policy
    StringX sxPolicyName(m_policyname);
    m_pDbx->GetPolicyFromName(sxPolicyName, st_pp);

    if (st_pp.symbols.empty()) {
      CPasswordCharPool::GetDefaultSymbols(st_symbols);
      st_pp.symbols = st_symbols.c_str();
    }
  }

  StringX passwd;
  m_pDbx->MakeRandomPassword(passwd, st_pp.pwp, st_pp.symbols.c_str(), false);
  m_password = passwd.c_str();
  m_ex_password.SetWindowText(m_password);
  m_ex_password.Invalidate();

  UpdateData(FALSE);
}

void CPasswordPolicyDlg::OnCopyPassword()
{
  UpdateData(TRUE);

  m_pDbx->SetClipboardData(m_password);
  m_pDbx->UpdateLastClipboardAction(CItemData::PASSWORD);
}

void CPasswordPolicyDlg::OnENChangePassword()
{
  UpdateData(TRUE);

  m_ex_password.GetWindowText((CString &)m_password);
}

void CPasswordPolicyDlg::OnENChangePolicyName()
{
  UpdateData(TRUE);

  m_PolicyNameEdit.GetWindowText((CString &)m_policyname);
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

    // Disable lengths
    GetDlgItem(IDC_STATIC_PSWDLENGTH)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFPWLENGTH)->EnableWindow(FALSE);

    // Disable Symbols
    GetDlgItem(IDC_USEDEFAULTSYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEOWNSYMBOLS)->EnableWindow(FALSE);
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

  st_PSWDPolicy st_pp;
  st_pp.pwp.flags = 0;
  if (m_PWUseLowercase == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyUseLowercase;
  if (m_PWUseUppercase == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyUseUppercase;
  if (m_PWUseDigits == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyUseDigits;
  if (m_PWUseSymbols == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyUseSymbols;
  if (m_PWUseHexdigits == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
  if (m_PWEasyVision == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
  if (m_PWMakePronounceable == TRUE)
    st_pp.pwp.flags |= PWSprefs::PWPolicyMakePronounceable;

  st_pp.pwp.length = m_PWDefaultLength;
  st_pp.pwp.digitminlength = m_PWDigitMinLength;
  st_pp.pwp.lowerminlength = m_PWLowerMinLength;
  st_pp.pwp.symbolminlength = m_PWSymbolMinLength;
  st_pp.pwp.upperminlength = m_PWUpperMinLength;

  st_pp.symbols = (m_PWUseSymbols == TRUE && m_UseOwnSymbols == OWN_SYMBOLS) ?
                         m_Symbols : L"";

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
