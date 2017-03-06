/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_Additional.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"    // For Help
#include "DboxMain.h"

#include "AddEdit_Additional.h"
#include "AddEdit_PropertySheet.h"
#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "HKModifiers.h"

#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"

#include "core/core.h"
#include "resource3.h"

using pws_os::CUUID;

////////////////////////////////////////////////////////////////////////////
// CAddEdit_Additional property page

IMPLEMENT_DYNAMIC(CAddEdit_Additional, CAddEdit_PropertyPage)

CAddEdit_Additional::CAddEdit_Additional(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_Additional::IDD, CAddEdit_Additional::IDD_SHORT,
                          pAEMD),
  m_bClearPWHistory(false), m_bSortAscending(true),
  m_bInitdone(false), m_iSortedColumn(-1),
  m_bWarnUserKBShortcut(false), m_iOldHotKey(0),
  m_bAE_AppHotKeyEnabled(false), m_bAE_AutotypeHotKeyEnabled(false)
{
  if (M_MaxPWHistory() == 0)
    M_MaxPWHistory() = PWSprefs::GetInstance()->
                           GetPref(PWSprefs::NumPWHistoryDefault);

  // Save PWS HotKey info
  int32 iAppHotKeyValue = int32(PWSprefs::GetInstance()->GetPref(PWSprefs::HotKey));
  WORD wHKModifiers = iAppHotKeyValue >> 16;
  m_wAppVirtualKeyCode = iAppHotKeyValue & 0xff;
  m_wAppWindowsModifiers = ConvertModifersMFC2Windows(wHKModifiers);

  // Only set that the hotkeys are set if they are rather than what the user's
  // preferences indicate (they might have been disabled due to conflicts)
  DboxMain *dbox = GetMainDlg();
  m_bPWSAppHotKeyEnabled = dbox->IsAppHotKeyEnabled();
  m_bPWSAutotypeHotKeyEnabled = dbox->IsAutotypeHotKeyEnabled();

  if (m_bPWSAppHotKeyEnabled) {
    m_iAppHotKey = (m_wAppWindowsModifiers << 16) + m_wAppVirtualKeyCode;
  }

  if (m_bPWSAppHotKeyEnabled) {
    // If PWS Application has an active HotKey, disable it when user
    // is potentially editing this entry's HotKey
    BOOL brc = UnregisterHotKey(GetMainDlg()->m_hWnd, PWS_HOTKEY_ID);
    ASSERT(brc);
  }

  if (m_bPWSAutotypeHotKeyEnabled) {
    // If PWS Application has an active HotKey, disable it when user
    // is potentially editing this entry's HotKey
    BOOL brc = UnregisterHotKey(GetMainDlg()->m_hWnd, PWS_AT_HOTKEY_ID);
    ASSERT(brc);
  }

  m_KBShortcutCtrl.SetMyParent(this);
}

CAddEdit_Additional::~CAddEdit_Additional()
{
  if (m_bAE_AppHotKeyEnabled || m_bAE_AutotypeHotKeyEnabled ||
      m_wAppWindowsModifiers == 0 || m_wAppVirtualKeyCode == 0)
    return;

  // If PWS Application had an active HotKey before the user edited
  // this entry, put it back
  if (m_bPWSAppHotKeyEnabled && !m_bAE_AppHotKeyEnabled &&
    m_wAppWindowsModifiers != 0 && m_wAppVirtualKeyCode != 0) {
    m_bAE_AppHotKeyEnabled = RegisterHotKey(GetMainDlg()->m_hWnd, PWS_HOTKEY_ID,
      UINT(m_wAppWindowsModifiers), UINT(m_wAppVirtualKeyCode)) == TRUE;
  }

  // If PWS Application had the Autotype hotkey active before the user edited
  // this entry, put it back
  if (m_bPWSAutotypeHotKeyEnabled && !m_bAE_AutotypeHotKeyEnabled) {
    m_bAE_AutotypeHotKeyEnabled = RegisterHotKey(GetMainDlg()->m_hWnd, PWS_AT_HOTKEY_ID,
      UINT(AUTOTYPE_HOTKEY_MODIFIERS), UINT(AUTOTYPE_HOTKEY_KEYCODE)) == TRUE;
  }
}

void CAddEdit_Additional::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_Additional)
  DDX_Text(pDX, IDC_AUTOTYPE, (CString&)M_autotype());
  DDX_Text(pDX, IDC_RUNCMD, (CString&)M_runcommand());

  DDX_Control(pDX, IDC_AUTOTYPE, m_ex_autotype);
  DDX_Control(pDX, IDC_RUNCMD, m_ex_runcommand);
  DDX_Control(pDX, IDC_DOUBLE_CLICK_ACTION, m_dblclk_cbox);
  DDX_Control(pDX, IDC_SHIFT_DOUBLE_CLICK_ACTION, m_shiftdblclk_cbox);

  DDX_Control(pDX, IDC_STATIC_AUTO, m_stc_autotype);
  DDX_Control(pDX, IDC_STATIC_RUNCMD, m_stc_runcommand);

  // Password History
  DDX_Control(pDX, IDC_PWHISTORY_LIST, m_PWHistListCtrl);
  DDX_Check(pDX, IDC_SAVE_PWHIST, M_SavePWHistory());
  DDX_Text(pDX, IDC_MAXPWHISTORY, M_MaxPWHistory());

  // Keyboard shortcut
  DDX_Control(pDX, IDC_ENTKBSHCTHOTKEY, m_KBShortcutCtrl);
  // Error/Warning messages for user defined keyboard shortcut
  DDX_Control(pDX, IDC_STATIC_SHCTWARNING, m_stc_warning);

  DDX_Control(pDX, IDC_AUTOTYPEHELP, m_Help1);
  DDX_Control(pDX, IDC_AUTOTYPEHELP2, m_Help2);
  DDX_Control(pDX, IDC_RUNCMDHELP, m_Help3);
  DDX_Control(pDX, IDC_ENTKBSHCTHOTKEYHELP, m_Help4);
  DDX_Control(pDX, IDC_PWHHELP, m_Help5);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Additional, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Additional)
  ON_WM_CTLCOLOR()
  ON_WM_CONTEXTMENU()

  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_AUTOTYPEHELP, OnAutotypeHelp)

  ON_EN_CHANGE(IDC_AUTOTYPE, OnChanged)
  ON_EN_CHANGE(IDC_MAXPWHISTORY, OnChanged)
  ON_EN_CHANGE(IDC_RUNCMD, OnRunCmdChanged)
  ON_EN_CHANGE(IDC_ENTKBSHCTHOTKEY, OnHotKeyChanged)

  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_AUTO, IDC_STATIC_RUNCMD, OnSTCExClicked)
  ON_CBN_SELCHANGE(IDC_DOUBLE_CLICK_ACTION, OnDCAComboChanged)
  ON_CBN_SELCHANGE(IDC_SHIFT_DOUBLE_CLICK_ACTION, OnShiftDCAComboChanged)

  // Password History
  ON_BN_CLICKED(IDC_CLEAR_PWHIST, OnClearPWHist)
  ON_BN_CLICKED(IDC_SAVE_PWHIST, OnCheckedSavePasswordHistory)
  ON_BN_CLICKED(IDC_PWH_COPY_ALL, OnPWHCopyAll)

  ON_NOTIFY(HDN_ITEMCLICK, 0, OnHeaderClicked)
  ON_NOTIFY(NM_CLICK, IDC_PWHISTORY_LIST, OnHistListClick)
  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Additional message handlers

BOOL CAddEdit_Additional::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  RelayToolTipEvent(pMsg);

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_Additional::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  // Get Add/Edit font
  Fonts *pFonts = Fonts::GetInstance();
  CFont *pFont = pFonts->GetAddEditFont();

  // Change font size of the autotype & run fields
  m_ex_autotype.SetFont(pFont);
  m_ex_runcommand.SetFont(pFont);

  m_stc_warning.SetColour(RGB(255, 0, 0));
  m_stc_warning.ShowWindow(SW_HIDE);

  CString cs_dats;
  StringX sx_dats = PWSprefs::GetInstance()->
                           GetPref(PWSprefs::DefaultAutotypeString);
  if (sx_dats.empty())
    cs_dats = DEFAULT_AUTOTYPE;
  else
    cs_dats.Format(IDS_DEFAULTAUTOTYPE, static_cast<LPCWSTR>(sx_dats.c_str()));

  GetDlgItem(IDC_DEFAULTAUTOTYPE)->SetWindowText(cs_dats);

  m_stc_autotype.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_runcommand.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);

  // These wil be zero for Add entry
  WORD wVirtualKeyCode = M_KBShortcut() & 0xff;
  WORD wPWSModifiers = M_KBShortcut() >> 16;

  // Translate from PWS to CHotKeyCtrl modifiers
  WORD wHKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);

  m_KBShortcutCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);

  // Save current values in case we have to revert
  m_wSavedVirtualKeyCode = wVirtualKeyCode;
  m_wSavedModifiers = wHKModifiers;

  // Check shortcut in case Options have changed
  CheckKeyboardShortcut();

  // We could do this to ensure user has at least Alt or Ctrl key
  // But it gets changed without the user knowing - so we do it elsewhere
  // instead and tell them.
  //m_KBShortcutCtrl.SetRules(HKCOMB_NONE | HKCOMB_S, HOTKEYF_ALT);

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    // Disable normal Edit controls
    GetDlgItem(IDC_AUTOTYPE)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_RUNCMD)->SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable HotKey
    m_KBShortcutCtrl.EnableWindow(FALSE);

    // Disable Combobox
    GetDlgItem(IDC_DOUBLE_CLICK_ACTION)->EnableWindow(FALSE);
    GetDlgItem(IDC_SHIFT_DOUBLE_CLICK_ACTION)->EnableWindow(FALSE);
  }

  // For some reason, MFC calls us twice when initializing.
  // SetupDCAComboBoxes is idempotent
  SetupDCAComboBoxes(&m_dblclk_cbox, false);
  SetupDCAComboBoxes(&m_shiftdblclk_cbox, true);

  short iDCA;
  if (M_DCA() < PWSprefs::minDCA || M_DCA() > PWSprefs::maxDCA)
    iDCA = (short)PWSprefs::GetInstance()->GetPref(PWSprefs::DoubleClickAction);
  else
    iDCA = M_DCA();
  m_dblclk_cbox.SetCurSel(m_DCA_to_Index[iDCA]);

  if (M_ShiftDCA() < PWSprefs::minDCA || M_ShiftDCA() > PWSprefs::maxDCA)
    iDCA = (short)PWSprefs::GetInstance()->GetPref(PWSprefs::ShiftDoubleClickAction);
  else
    iDCA = M_ShiftDCA();
  m_shiftdblclk_cbox.SetCurSel(m_DCA_to_Index[iDCA]);

  // Password History
  M_oldMaxPWHistory() = M_MaxPWHistory();

  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(M_SavePWHistory());

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXPWHISTORY));
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos((int)M_MaxPWHistory());

  if (M_uicaller() == IDS_ADDENTRY) {
    GetDlgItem(IDC_CLEAR_PWHIST)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PWHISTORY_LIST)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PWH_COPY_ALL)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_PWH_EDIT)->ShowWindow(SW_HIDE);
    UpdateData(FALSE);
    m_bInitdone = true;
    return TRUE;
  }

  // Initialise m_Help2 MUST be performed before calling UpdatePasswordHistory
  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_QUESTIONMARK);
    m_Help4.Init(IDB_QUESTIONMARK);
    m_Help5.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    CString cs_LabelText, cs_Tooltip;
    GetDlgItem(IDC_STATIC_AUTO)->GetWindowText(cs_LabelText);
    cs_LabelText.Remove(L'&');
    cs_LabelText.Remove(L':');
    cs_Tooltip.Format(IDS_CLICKTOCOPYEXPAND, static_cast<LPCWSTR>(cs_LabelText));
    AddTool(IDC_AUTOTYPEHELP2, cs_Tooltip);

    GetDlgItem(IDC_STATIC_RUNCMD)->GetWindowText(cs_LabelText);
    cs_LabelText.Remove(L'&');
    cs_LabelText.Remove(L':');
    cs_Tooltip.Format(IDS_CLICKTOCOPYEXPAND, static_cast<LPCWSTR>(cs_LabelText));
    AddTool(IDC_RUNCMDHELP, cs_Tooltip);

    if (M_runcommand().IsEmpty()) {
      m_Help3.EnableWindow(FALSE);
      m_Help3.ShowWindow(SW_HIDE);
    }

    AddTool(IDC_ENTKBSHCTHOTKEYHELP, IDS_ENTKBSHCTHOTKEYHELP);
    AddTool(IDC_AUTOTYPEHELP, IDS_AUTOTYPEHELP);
    AddTool(IDC_PWHHELP, IDS_PWHHELP);

    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
    m_Help4.EnableWindow(FALSE);
    m_Help4.ShowWindow(SW_HIDE);
    m_Help5.EnableWindow(FALSE);
    m_Help5.ShowWindow(SW_HIDE);
  }

  UpdatePasswordHistoryLC();

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVE_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);
  }

  UpdateData(FALSE);
  m_bInitdone = true;
  return TRUE;
}

void CAddEdit_Additional::SetupDCAComboBoxes(CComboBox *pcbox, bool isShift)
{
  const struct {int res; int pref;} ResPref[] = {
    {IDSC_DCAAUTOTYPE,        PWSprefs::DoubleClickAutotype},
    {IDSC_DCABROWSE,          PWSprefs::DoubleClickBrowse},
    {IDSC_DCABROWSEPLUS,      PWSprefs::DoubleClickBrowsePlus},
    {IDSC_DCACOPYNOTES,       PWSprefs::DoubleClickCopyNotes},
    {IDSC_DCACOPYPASSWORD,    PWSprefs::DoubleClickCopyPassword},
    {IDSC_DCACOPYPASSWORDMIN, PWSprefs::DoubleClickCopyPasswordMinimize},
    {IDSC_DCACOPYUSERNAME,    PWSprefs::DoubleClickCopyUsername},
    {IDSC_DCAVIEWEDIT,        PWSprefs::DoubleClickViewEdit},
    {IDSC_DCARUN,             PWSprefs::DoubleClickRun},
    {IDSC_DCASENDEMAIL,       PWSprefs::DoubleClickSendEmail},
    {0,                       0},
  };
  if (pcbox->GetCount() == 0) { // Make sure we're idempotent
    int DefaultDCA = PWSprefs::GetInstance()->GetPref(isShift ?
                                                      PWSprefs::ShiftDoubleClickAction :
                                                      PWSprefs::DoubleClickAction);
    CString cs_text;
    int i;
    for (i = 0; ResPref[i].res != 0; i++) {
      cs_text.LoadString(ResPref[i].res);
      if (ResPref[i].pref == DefaultDCA) {
        const CString cs_default(MAKEINTRESOURCE(IDSC_DEFAULT));
        cs_text += cs_default;
      }
      int nIndex = pcbox->AddString(cs_text);
      pcbox->SetItemData(nIndex, ResPref[i].pref);
      m_DCA_to_Index[ResPref[i].pref] = nIndex;
    }
    // set up m_DCA_to_Index after populating pcbox, as order may be changed
    int N = pcbox->GetCount();
    for (i = 0; i < N; i++) {
      DWORD_PTR j = pcbox->GetItemData(i);
      m_DCA_to_Index[j] = i;
    }
  }
}
void CAddEdit_Additional::OnChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Additional::OnRunCmdChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);

  BOOL bRunCmdEmpty = M_runcommand().IsEmpty();
  m_Help3.EnableWindow(bRunCmdEmpty == TRUE ? FALSE : TRUE);
  m_Help3.ShowWindow(bRunCmdEmpty == TRUE ? SW_HIDE : SW_SHOW);

  m_ae_psh->SetChanged(true);
}

void CAddEdit_Additional::OnHotKeyChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);

  WORD wVirtualKeyCode, wHKModifiers, wPWSModifiers;
  m_KBShortcutCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);
  if (wVirtualKeyCode != 0 && wHKModifiers != 0) {
    // Translate from PWS to CHotKeyCtrl modifiers
    wPWSModifiers = ConvertModifersMFC2PWS(wHKModifiers);
    int32 iKBShortcut = (wPWSModifiers << 16) + wVirtualKeyCode;

    if (CheckKeyboardShortcut() == KBSHORTCUT_UNIQUE) {
      m_wSavedVirtualKeyCode = wVirtualKeyCode;
      m_wSavedModifiers = wHKModifiers;

      if (M_KBShortcut() != iKBShortcut) {
        m_ae_psh->SetChanged(true);
      }
    }
  
    if (wVirtualKeyCode == m_wAppVirtualKeyCode && wHKModifiers == m_wAppWindowsModifiers) {
      // Same as PWS Application HotKey - Restore last saved values
      m_KBShortcutCtrl.SetHotKey(m_wSavedVirtualKeyCode, m_wSavedModifiers);
      return;
    }

    if (wVirtualKeyCode == AUTOTYPE_HOTKEY_KEYCODE && wHKModifiers == AUTOTYPE_HOTKEY_MODIFIERS) {
      // Same as PWS Autotype HotKey - Restore last saved values
      m_KBShortcutCtrl.SetHotKey(m_wSavedVirtualKeyCode, m_wSavedModifiers);
      return;
    }
  }
}

void CAddEdit_Additional::OnHelp()
{
  ShowHelp(L"::/html/entering_pwd_add.html");
}

void CAddEdit_Additional::OnAutotypeHelp()
{
  ShowHelp(L"::/html/autotype.html");
}

HBRUSH CAddEdit_Additional::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CAddEdit_PropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC) {
    UINT nID = pWnd->GetDlgCtrlID();
    if (nID == IDC_STATIC_SHCTWARNING) {
      pDC->SetTextColor(RGB(255, 0, 0));
      pDC->SetBkMode(TRANSPARENT);
      return hbr;
    }

    COLORREF *pcfOld;
    switch (nID) {
      case IDC_STATIC_AUTO:
        pcfOld = &m_autotype_cfOldColour;
        break;
      case IDC_STATIC_RUNCMD:
        pcfOld = &m_runcmd_cfOldColour;
        break;
      default:
        // Not one of ours - get out quick
        return hbr;
        break;
    }

    int iFlashing = ((CStaticExtn *)pWnd)->IsFlashing();
    BOOL bHighlight = ((CStaticExtn *)pWnd)->IsHighlighted();
    BOOL bMouseInWindow = ((CStaticExtn *)pWnd)->IsMouseInWindow();

    if (iFlashing != 0) {
      pDC->SetBkMode(iFlashing == 1 || (iFlashing && bHighlight && bMouseInWindow) ?
                      OPAQUE : TRANSPARENT);
      COLORREF cfFlashColour = ((CStaticExtn *)pWnd)->GetFlashColour();
      *pcfOld = pDC->SetBkColor(iFlashing == 1 ? cfFlashColour : *pcfOld);
    } else if (bHighlight) {
      pDC->SetBkMode(bMouseInWindow ? OPAQUE : TRANSPARENT);
      COLORREF cfHighlightColour = ((CStaticExtn *)pWnd)->GetHighlightColour();
      *pcfOld = pDC->SetBkColor(bMouseInWindow ? cfHighlightColour : *pcfOld);
    } else if (((CStaticExtn *)pWnd)->GetColourState()) {
      COLORREF cfUser = ((CStaticExtn *)pWnd)->GetUserColour();
      pDC->SetTextColor(cfUser);
    }
  }

  // Let's get out of here
  return hbr;
}

BOOL CAddEdit_Additional::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  if (CheckKeyboardShortcut() < 0 || m_bWarnUserKBShortcut)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}

void CAddEdit_Additional::OnEntryHotKeyKillFocus()
{
  CheckKeyboardShortcut();
}

int CAddEdit_Additional::CheckKeyboardShortcut()
{
  if (m_AEMD.uicaller == IDS_VIEWENTRY)
    return 0;

  int32 iKBShortcut;
  m_stc_warning.ShowWindow(SW_HIDE);
  
  WORD wVirtualKeyCode, wHKModifiers, wPWSModifiers;
  m_KBShortcutCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);
  
  // Translate from CHotKeyCtrl to PWS modifiers
  wPWSModifiers = ConvertModifersMFC2PWS(wHKModifiers);
  iKBShortcut = (wPWSModifiers << 16) + wVirtualKeyCode ;

  if (m_iOldHotKey != iKBShortcut)
    m_bWarnUserKBShortcut = false;

  if (iKBShortcut != 0) {
    CString cs_errmsg, cs_msg;
    
    // Luckily for valid alphanumeric characters, Windows Virtual Key Code
    // is the same as the ASCII character - no need for any conversion
    static const wchar_t *wcValidKeys = 
             L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (wcschr(wcValidKeys, wVirtualKeyCode) == NULL) {
      // Not alphanumeric
      cs_msg.LoadString(IDS_KBS_INVALID);
      m_stc_warning.SetWindowText(cs_msg);
      m_stc_warning.ShowWindow(SW_SHOW);

      // Reset keyboard shortcut
      wVirtualKeyCode = wHKModifiers = wPWSModifiers = 0;
      m_KBShortcutCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);
      return KBSHORTCUT_INVALID_CHARACTER;
    }

    if (m_bAE_AppHotKeyEnabled && m_iAppHotKey == iKBShortcut) {
      // Same as PWS application HotKey
      cs_msg.LoadString(IDS_KBS_SAMEASAPP);
      m_stc_warning.SetWindowText(cs_msg);
      m_stc_warning.ShowWindow(SW_SHOW);

      // Reset keyboard shortcut
      wVirtualKeyCode = wHKModifiers = wPWSModifiers = 0;
      m_KBShortcutCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);
      return KBSHORTCUT_IN_USE_BY_PWS;
    }

    if (m_bAE_AutotypeHotKeyEnabled &&
        wVirtualKeyCode == AUTOTYPE_HOTKEY_KEYCODE &&
        wHKModifiers == AUTOTYPE_HOTKEY_MODIFIERS) {
      // Same as PWS application HotKey2
      CString csHotKey(MAKEINTRESOURCE(IDS_AUTOTYPE_HOTKEY_VALUE));
      CString cs_Reset(MAKEINTRESOURCE(IDS_KBS_RESET));
      cs_msg.Format(IDS_AUTOTYPE_RESERVED, static_cast<LPCWSTR>(csHotKey),
        static_cast<LPCWSTR>(cs_Reset));
      m_stc_warning.SetWindowText(cs_msg);
      m_stc_warning.ShowWindow(SW_SHOW);

      //// Reset keyboard shortcut
      //wVirtualKeyCode = wHKModifiers = wPWSModifiers = 0;
      //m_KBShortcutCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);
      return KBSHORTCUT_IN_USE_BY_PWS;
    }

    if ((wPWSModifiers & PWS_HOTKEYF_ALT) == 0 &&
        (wPWSModifiers & PWS_HOTKEYF_CONTROL) == 0 &&
        !m_bWarnUserKBShortcut) {
      // Add Alt and/or Ctrl key and tell user but first check not already in use
      int iRC;
      pws_os::CUUID chk_uuid;

      WORD wValidModifierCombos[] = {
                   PWS_HOTKEYF_ALT,
                   PWS_HOTKEYF_ALT     | PWS_HOTKEYF_SHIFT,
                   PWS_HOTKEYF_CONTROL,
                   PWS_HOTKEYF_CONTROL | PWS_HOTKEYF_SHIFT,
                   PWS_HOTKEYF_ALT     | PWS_HOTKEYF_CONTROL,
                   PWS_HOTKEYF_ALT     | PWS_HOTKEYF_CONTROL | PWS_HOTKEYF_SHIFT};
      int iActions[] = {IDS_KBS_ADD_ALT, IDS_KBS_ADD_ALTSHIFT,
                        IDS_KBS_ADD_CTRL, IDS_KBS_ADD_CTRLSHIFT,
                        IDS_KBS_ADD_ALTCTRL, IDS_KBS_ADD_ALTCTRLSHIFT};
      
      // Try them in order
      int iChange, ierror(IDS_KBS_CANTADD);
      for (iChange = 0; iChange < sizeof(wValidModifierCombos) / sizeof(WORD); iChange++) {
        int iNewKBShortcut = ((wPWSModifiers | wValidModifierCombos[iChange]) << 16) + wVirtualKeyCode;
        chk_uuid = M_pcore()->GetKBShortcut(iNewKBShortcut);
        if (chk_uuid == CUUID::NullUUID() || chk_uuid == M_entry_uuid()) {
          ierror = iActions[iChange];
          break;
        }
      }

      if (ierror == IDS_KBS_CANTADD) {
        wVirtualKeyCode = wHKModifiers = wPWSModifiers = 0;
        iRC = KBSHORTCUT_CANT_MAKE_UNIQUE;
      } else {
        wPWSModifiers |= wValidModifierCombos[iChange];
        wHKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);
        iRC = KBSHORTCUT_MADE_UNIQUE;
      }

      cs_msg.LoadString(ierror);
      cs_errmsg.Format(IDS_KBS_INVALID, static_cast<LPCWSTR>(cs_msg));
      m_stc_warning.SetWindowText(cs_errmsg);
      m_stc_warning.ShowWindow(SW_SHOW);

      // Get new keyboard shortcut
      m_iOldHotKey = iKBShortcut = (wPWSModifiers << 16) + wVirtualKeyCode;
      ((CHotKeyCtrl *)GetDlgItem(IDC_ENTKBSHCTHOTKEY))->SetFocus();
      m_bWarnUserKBShortcut = true;
      m_KBShortcutCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);
      return iRC;
    }

    const CString cs_HotKey = m_KBShortcutCtrl.GetHotKeyName();
    pws_os::CUUID uuid = M_pcore()->GetKBShortcut(iKBShortcut);

    if (uuid != CUUID::NullUUID() && uuid != M_entry_uuid()) {
      // Tell user that it already exists as an entry keyboard shortcut
      ItemListIter iter = M_pcore()->Find(uuid);
      const StringX sxGroup = iter->second.GetGroup();
      const StringX sxTitle = iter->second.GetTitle();
      const StringX sxUser  = iter->second.GetUser();
      CString cs_Reset(MAKEINTRESOURCE(IDS_KBS_RESET));

      cs_errmsg.Format(IDS_KBS_INUSEBYENTRY, static_cast<LPCWSTR>(cs_HotKey),
                       static_cast<LPCWSTR>(sxGroup.c_str()),
                       static_cast<LPCWSTR>(sxTitle.c_str()),
                       static_cast<LPCWSTR>(sxUser.c_str()),
                       static_cast<LPCWSTR>(cs_Reset));
      m_stc_warning.SetWindowText(cs_errmsg);
      m_stc_warning.ShowWindow(SW_SHOW);

      ((CHotKeyCtrl *)GetDlgItem(IDC_ENTKBSHCTHOTKEY))->SetFocus();

      // Reset keyboard shortcut
      wVirtualKeyCode = wHKModifiers = wPWSModifiers = 0;
      m_KBShortcutCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);
      return KBSHORTCUT_IN_USE_BY_ENTRY;
    }
    
    StringX sxMenuItemName;
    unsigned char ucModifiers = wHKModifiers & 0xff;
    unsigned int nCID = GetMainDlg()->GetMenuShortcut(wVirtualKeyCode,
                                                      ucModifiers, sxMenuItemName);
    if (nCID != 0) {
      // Save this value
      m_iOldHotKey = iKBShortcut;

      // Warn user that it is already in use for a menu item
      // (on this instance for this user!)
      Remove(sxMenuItemName, L'&');
      cs_errmsg.Format(IDS_KBS_INUSEBYMENU, static_cast<LPCWSTR>(cs_HotKey),
                       static_cast<LPCWSTR>(sxMenuItemName.c_str()));
      m_stc_warning.SetWindowText(cs_errmsg);
      m_stc_warning.ShowWindow(SW_SHOW);
      // We have warned them - so now accept
      m_bWarnUserKBShortcut = !m_bWarnUserKBShortcut;
      return KBSHORTCUT_IN_USE_BY_MENU;
    }
    // We have warned them - so now accept (we are here, if hotkey was made unique by adding modifier)
    if (m_bWarnUserKBShortcut)
      m_bWarnUserKBShortcut = false;
  }
  return KBSHORTCUT_UNIQUE;
}

LRESULT CAddEdit_Additional::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_SavePWHistory()   != M_oldSavePWHistory()  ||
          M_NumPWHistory()    != M_pwhistlist().size() ||
          (M_SavePWHistory()  == TRUE &&
           M_MaxPWHistory()   != M_oldMaxPWHistory()))
        return 1L;
      switch (M_uicaller()) {
        case IDS_EDITENTRY:
          if (M_autotype()    != M_pci()->GetAutotype()   ||
              M_runcommand()  != M_pci()->GetRunCommand() ||
              M_DCA()         != M_oldDCA()               ||
              M_ShiftDCA()    != M_oldShiftDCA()          ||
              M_KBShortcut()  != M_oldKBShortcut())
            return 1L;
          break;
        case IDS_ADDENTRY:
          if (!M_autotype().IsEmpty()     ||
              !M_runcommand().IsEmpty()   ||
              M_DCA()      != M_oldDCA()  ||
              M_ShiftDCA() != M_oldShiftDCA())
            return 1L;
          break;
      }
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
      break;
  }
  return 0L;
}

BOOL CAddEdit_Additional::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE; //CAddEdit_PropertyPage::OnApply();

  CWnd *pFocus(NULL);

  UpdateData(TRUE);
  M_autotype().EmptyIfOnlyWhiteSpace();
  M_runcommand().EmptyIfOnlyWhiteSpace();

  if (CheckKeyboardShortcut() < 0 || m_bWarnUserKBShortcut)
    return FALSE;

  WORD wVirtualKeyCode, wHKModifiers, wPWSModifiers;
  m_KBShortcutCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);
  
  // Translate from PWS to CHotKeyCtrl modifiers
  wPWSModifiers = ConvertModifersMFC2PWS(wHKModifiers);
  M_KBShortcut() = (wPWSModifiers << 16) + wVirtualKeyCode;

  if (M_runcommand().GetLength() > 0) {
    //Check Run Command parses - don't substitute
    std::wstring errmsg;
    size_t st_column;
    bool bAutotype(false);
    StringX sxAutotype(L"");
    bool bURLSpecial;
    PWSAuxParse::GetExpandedString(M_runcommand(), L"", NULL, NULL,
                                   bAutotype, sxAutotype, errmsg, st_column,
                                   bURLSpecial);
    if (errmsg.length() > 0) {
      CGeneralMsgBox gmb;
      CString cs_title(MAKEINTRESOURCE(IDS_RUNCOMMAND_ERROR));
      CString cs_temp(MAKEINTRESOURCE(IDS_RUN_IGNOREORFIX));
      CString cs_errmsg;
      cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, static_cast<LPCWSTR>(errmsg.c_str()));
      cs_errmsg += cs_temp;
      INT_PTR rc = gmb.MessageBox(cs_errmsg, cs_title,
                           MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
      if (rc == IDNO) {
        UpdateData(FALSE);
        // Are we the current page, if not activate this page
        if (m_ae_psh->GetActivePage() != (CAddEdit_PropertyPage *)this)
          m_ae_psh->SetActivePage(this);

        pFocus = &m_ex_runcommand;
        goto error;
      }
    }
  }

  /* Handle history header.
   *
   * Header is in the form fmmnn, where:
   * f = {0,1} if password history is on/off
   * mm = 2 digits max size of history list
   * nn = 2 digits current size of history list
   *
   * Special case: history empty and password history off - do nothing
   *
   */

  if (m_bClearPWHistory == TRUE) {
    M_pwhistlist().clear();
    M_PWHistory() = M_PWHistory().Left(5);
    m_bClearPWHistory = false;
  }

  if (M_SavePWHistory() == TRUE &&
      (M_MaxPWHistory() < 1 || M_MaxPWHistory() > 255)) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_DEFAULTNUMPWH);
    pFocus = GetDlgItem(IDC_MAXPWHISTORY);
    goto error;
  }

  if (!(M_PWHistory().IsEmpty() && M_SavePWHistory() == FALSE)) {
    wchar_t buffer[6];
    swprintf_s(buffer, 6, L"%1x%02x%02x",
              (M_SavePWHistory() == FALSE) ? 0 : 1,
              (unsigned int)M_MaxPWHistory(),
              (unsigned int)M_pwhistlist().size());
    if (M_PWHistory().GetLength() >= 5) {
      for (int i = 0; i < 5; i++) {
        M_PWHistory().SetAt(i, buffer[i]);
      }
    } else {
      M_PWHistory() = buffer;
    }
  }

  m_iOldHotKey = M_KBShortcut();
  return CAddEdit_PropertyPage::OnApply();

error:
  // Are we the current page, if not activate this page
  if (m_ae_psh->GetActivePage() != (CAddEdit_PropertyPage *)this)
    m_ae_psh->SetActivePage(this);

  if (pFocus != NULL)
    pFocus->SetFocus();

  if (pFocus == GetDlgItem(IDC_MAXPWHISTORY))
    ((CEdit *)pFocus)->SetSel(MAKEWORD(-1, 0));

  return FALSE;
}

void CAddEdit_Additional::OnDCAComboChanged()
{
  m_ae_psh->SetChanged(true);

  int nIndex = m_dblclk_cbox.GetCurSel();
  M_DCA() = (short)m_dblclk_cbox.GetItemData(nIndex);
}

void CAddEdit_Additional::OnShiftDCAComboChanged()
{
  m_ae_psh->SetChanged(true);

  int nIndex = m_shiftdblclk_cbox.GetCurSel();
  M_ShiftDCA() = (short)m_shiftdblclk_cbox.GetItemData(nIndex);
}

void CAddEdit_Additional::OnSTCExClicked(UINT nID)
{
  UpdateData(TRUE);
  StringX sxData;
  int iaction(0);
  std::vector<size_t> vactionverboffsets;

  // NOTE: These values must be contiguous in "resource.h"
  switch (nID) {
    case IDC_STATIC_AUTO:
      m_stc_autotype.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      // If Ctrl pressed - just copy un-substituted Autotype string
      // else substitute
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
        if (M_autotype().IsEmpty()) {
          sxData = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultAutotypeString);
          // If still empty, take this default
          if (sxData.empty()) {
            sxData = DEFAULT_AUTOTYPE;
          }
        } else
          sxData = (LPCWSTR)M_autotype();
      } else {
        CSecString sPassword(M_realpassword()), sLastPassword(M_lastpassword());
        if (m_AEMD.pci->IsAlias()) {
          CItemData *pciA = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
          ASSERT(pciA != NULL);
          sPassword = pciA->GetPassword();
          sLastPassword = pciA->GetPreviousPassword();
        }

        sxData = PWSAuxParse::GetAutotypeString(M_autotype(),
                                                M_group(),
                                                M_title(),
                                                M_username(),
                                                sPassword,
                                                sLastPassword,
                                                M_notes(),
                                                M_URL(),
                                                M_email(),
                                                vactionverboffsets);

        // Replace any special code that we can - i.e. only \{\t} and \{ }
        StringX sxTabCode(L"\\{\t}"), sxSpaceCode(L"\\{ }");
        StringX sxTab(L"\t"), sxSpace(L" ");
        Replace(sxData, sxTabCode, sxTab);
        Replace(sxData, sxSpaceCode, sxSpace);
      }
      iaction = CItemData::AUTOTYPE;
      break;
    case IDC_STATIC_RUNCMD:
      m_stc_runcommand.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      // If Ctrl pressed - just copy un-substituted Run Command
      // else substitute
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 || M_runcommand().IsEmpty()) {
        sxData = (LPCWSTR)M_runcommand();
      } else {
        std::wstring errmsg;
        size_t st_column;
        bool bURLSpecial;

        CItemData *pbci = NULL;
        if (M_pci()->IsAlias()) {
          pbci = M_pcore()->GetBaseEntry(M_pci());
        }

        sxData = PWSAuxParse::GetExpandedString(M_runcommand(),
                                                M_currentDB(),
                                                M_pci(), pbci,
                                                GetMainDlg()->m_bDoAutotype,
                                                GetMainDlg()->m_sxAutotype,
                                                errmsg, st_column, bURLSpecial);
        if (errmsg.length() > 0) {
          CGeneralMsgBox gmb;
          CString cs_title(MAKEINTRESOURCE(IDS_RUNCOMMAND_ERROR));
          CString cs_errmsg;
          cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, static_cast<LPCWSTR>(errmsg.c_str()));
          gmb.MessageBox(cs_errmsg, cs_title, MB_ICONERROR);
        }
      }
      iaction = CItemData::RUNCMD;
      break;
    default:
      ASSERT(0);
  }

  if (iaction != 0) {
    GetMainDlg()->SetClipboardData(sxData);
    GetMainDlg()->UpdateLastClipboardAction(iaction);
  }
}

void CAddEdit_Additional::OnCheckedSavePasswordHistory()
{
  M_SavePWHistory() = ((CButton*)GetDlgItem(IDC_SAVE_PWHIST))->GetCheck() == BST_CHECKED ?
                           TRUE : FALSE;
  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(M_SavePWHistory());
  GetDlgItem(IDC_PWHSPIN)->EnableWindow(M_SavePWHistory());

  Invalidate();
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Additional::OnClearPWHist()
{
  m_bClearPWHistory = true;
  m_PWHistListCtrl.DeleteAllItems();
  M_pwhistlist().clear();
  m_ae_psh->SetChanged(true);

  // Update control states
  m_PWHistListCtrl.EnableWindow(FALSE);
  GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);
  GetDlgItem(IDC_PWH_COPY_ALL)->EnableWindow(FALSE);

  // Help no longer needed
  m_Help5.EnableWindow(FALSE);
  m_Help5.ShowWindow(SW_HIDE);
}

void CAddEdit_Additional::OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNotifyStruct;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = true;

    m_iSortedColumn = phdn->iItem;
    m_PWHistListCtrl.SortItems(PWHistCompareFunc, (LPARAM)this);

    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_PWHistListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= (m_bSortAscending ? HDF_SORTUP : HDF_SORTDOWN);
    m_PWHistListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pLResult = 0;
}

int CALLBACK CAddEdit_Additional::PWHistCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                                    LPARAM closure)
{
  CAddEdit_Additional *self = (CAddEdit_Additional *)closure;
  int nSortColumn = self->m_iSortedColumn;
  size_t Lpos = (size_t)lParam1;
  size_t Rpos = (size_t)lParam2;
  const PWHistEntry pLHS = self->M_pwhistlist()[Lpos];
  const PWHistEntry pRHS = self->M_pwhistlist()[Rpos];
  CSecString password1, changedate1;
  CSecString password2, changedate2;
  time_t t1, t2;

  int iResult(0);
  switch(nSortColumn) {
    case 0:
      t1 = pLHS.changetttdate;
      t2 = pRHS.changetttdate;
      if (t1 != t2)
        iResult = ((long) t1 < (long) t2) ? -1 : 1;
      break;
    case 1:
      password1 = pLHS.password;
      password2 = pRHS.password;
      iResult = ((CString)password1).Compare(password2);
      break;
    default:
      ASSERT(FALSE);
  }

  if (!self->m_bSortAscending && iResult != 0)
    iResult *= -1;

  return iResult;
}

void CAddEdit_Additional::OnPWHCopyAll()
{
  CSecString HistStr;
  PWHistList::iterator iter;

  for (iter = M_pwhistlist().begin(); iter != M_pwhistlist().end(); iter++) {
    const PWHistEntry &ent = *iter;
    HistStr += ent.changedate;
    HistStr += L"\t";
    HistStr += ent.password;
    HistStr += L"\r\n";
  }

  GetMainDlg()->SetClipboardData(HistStr);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::RESERVED);
}

void CAddEdit_Additional::OnHistListClick(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
  int selectedRow = pNMItemActivate->iItem;
  if (selectedRow >= 0) {
    int indx = (int)m_PWHistListCtrl.GetItemData(selectedRow);
    const StringX histpasswd = M_pwhistlist()[indx].password;
    GetMainDlg()->SetClipboardData(histpasswd);

    // Note use of CItemData::RESERVED for indicating in the
    // Status bar that an old password has been copied
    GetMainDlg()->UpdateLastClipboardAction(CItemData::RESERVED); 
  }
  *pResult = 0;
}

void CAddEdit_Additional::UpdatePasswordHistoryLC()
{
  // Set up PWH CListCtrl
  CString cs_text;

  m_PWHistListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_PWHistListCtrl.UpdateRowHeight(false);

  cs_text.LoadString(IDS_SETDATETIME);
  m_PWHistListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_PASSWORD);
  m_PWHistListCtrl.InsertColumn(1, cs_text);

  // Update Password History
  m_PWHistListCtrl.SetRedraw(FALSE);
  m_PWHistListCtrl.DeleteAllItems();

  PWHistList::iterator iter;
  DWORD nIdx;
  for (iter = M_pwhistlist().begin(), nIdx = 0;
       iter != M_pwhistlist().end(); iter++, nIdx++) {
    int nPos = 0;
    const PWHistEntry pwhentry = *iter;
    if (pwhentry.changetttdate != 0) {
      const StringX locTime = PWSUtil::ConvertToDateTimeString(pwhentry.changetttdate,
                                                               PWSUtil::TMC_LOCALE);
      nPos = m_PWHistListCtrl.InsertItem(nPos, locTime.c_str());
    } else {
      cs_text.LoadString(IDS_UNKNOWN);
      cs_text.Trim();
      nPos = m_PWHistListCtrl.InsertItem(nPos, cs_text);
    }
    m_PWHistListCtrl.SetItemText(nPos, 1, pwhentry.password.c_str());
    m_PWHistListCtrl.SetItemData(nPos, nIdx);
  }

  for (int i = 0; i < 2; i++) {
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, std::max(nColumnWidth, nHeaderWidth));
  }

  m_PWHistListCtrl.SetRedraw(TRUE);

  // Update controls state
  const BOOL bEntriesPresent = m_PWHistListCtrl.GetItemCount() != 0 ? TRUE : FALSE;

  // Don't enable change of PWH if an alias as passwords are now the base's.
  if (M_original_entrytype() == CItemData::ET_ALIAS) {
    GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVE_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_OLDPW1)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(bEntriesPresent);
  }

  m_PWHistListCtrl.EnableWindow(bEntriesPresent);
  GetDlgItem(IDC_PWH_COPY_ALL)->EnableWindow(bEntriesPresent);
  GetDlgItem(IDC_STATIC_PWH_ADD)->ShowWindow(SW_HIDE);

  // Help no longer needed
  m_Help5.EnableWindow(bEntriesPresent ? TRUE : FALSE);
  m_Help5.ShowWindow(bEntriesPresent ? SW_SHOW : SW_HIDE);
}

void CAddEdit_Additional::OnContextMenu(CWnd *, CPoint point)
{
  CRect rect;

  m_KBShortcutCtrl.GetWindowRect(&rect);
  if (rect.PtInRect(point)) {
    WORD wVirtualKeyCode, wHKModifiers;
    m_KBShortcutCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);

    if (wVirtualKeyCode == 0 && wHKModifiers == 0)
      return;

    CMenu PopupMenu;
    PopupMenu.LoadMenu(IDR_POPRESETSHORTCUT);
    CMenu *pContextMenu = PopupMenu.GetSubMenu(0);
    pContextMenu->RemoveMenu(ID_MENUITEM_REMOVESHORTCUT, MF_BYCOMMAND);

    int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
                                           point.x, point.y, this);

    if (nID == ID_MENUITEM_RESETSHORTCUT) {
      // Reset HotKey
      m_KBShortcutCtrl.SetHotKey(0, 0);

      // Clear any warnings
      m_stc_warning.SetWindowText(L"");
      m_stc_warning.ShowWindow(SW_HIDE);
    }
  }
}
