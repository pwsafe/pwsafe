/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_Basic.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordSafe.h"
#include "ThisMfcApp.h"    // For Help
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "Fonts.h"

#include "AddEdit_Basic.h"
#include "AddEdit_PropertySheet.h"
#include "ChangeAliasPswd.h"

#include "core/PWCharPool.h"
#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"
#include "core/core.h"
#include "core/command.h"

#include <shlwapi.h>
#include <fstream>

#include "Richedit.h"

using pws_os::CUUID;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static wchar_t PSSWDCHAR = L'*';

CSecString CAddEdit_Basic::HIDDEN_NOTES;

CString CAddEdit_Basic::CS_SHOW;
CString CAddEdit_Basic::CS_HIDE;

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic property page

IMPLEMENT_DYNAMIC(CAddEdit_Basic, CAddEdit_PropertyPage)

CAddEdit_Basic::CAddEdit_Basic(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent,
                          CAddEdit_Basic::IDD, CAddEdit_Basic::IDD_SHORT,
                          pAEMD),
  m_pToolTipCtrl(NULL), m_bInitdone(false), m_thread(NULL)
{
  if (CS_SHOW.IsEmpty()) { // one-time initializations
    HIDDEN_NOTES.LoadString(IDS_HIDDENNOTES);
#if defined(POCKET_PC)
    CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT1);
    CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT1);
#else
    CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT2);
    CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT2);
#endif
  }

  PWSprefs *prefs = PWSprefs::GetInstance();

  // Setup
  m_bWordWrap = prefs->GetPref(PWSprefs::NotesWordWrap);

  m_password = m_password2 = M_realpassword();
  m_notes = M_realnotes().Left(MAXTEXTCHARS);

  // Set up right-click Notes context menu additions
  std::vector<st_context_menu> vmenu_items;

  st_context_menu st_cm;
  std::wstring cs_menu_string;

  LoadAString(cs_menu_string, IDS_WORD_WRAP);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_EDIT_WORDWRAP;
  st_cm.flags = m_bWordWrap ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_SHOW_NOTES);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_EDIT_SHOWNOTES;
  st_cm.flags = m_isNotesHidden ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_NOTESZOOMIN);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_CALL_NOTESZOOMIN;
  st_cm.flags = 0;
  st_cm.lParam = 1;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_NOTESZOOMOUT);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_CALL_NOTESZOOMOUT;
  st_cm.flags = 0;
  st_cm.lParam = -1;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_EDITEXTERNALLY);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_CALL_EXTERNAL_EDITOR;
  st_cm.flags = 0;
  vmenu_items.push_back(st_cm);

  m_pex_notes = new CRichEditExtn(vmenu_items);
}

CAddEdit_Basic::~CAddEdit_Basic()
{
  delete m_pex_notes;
  delete m_pToolTipCtrl;
}

void CAddEdit_Basic::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_Basic)
  m_ex_password.DoDDX(pDX, m_password);
  m_ex_password2.DoDDX(pDX, m_password2);

  DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);

  DDX_CBString(pDX, IDC_GROUP, (CString&)M_group());
  DDX_Text(pDX, IDC_TITLE, (CString&)M_title());
  DDX_Text(pDX, IDC_USERNAME, (CString&)M_username());
  DDX_Text(pDX, IDC_URL, (CString&)M_URL());
  DDX_Text(pDX, IDC_EMAIL, (CString&)M_email());

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
  DDX_Control(pDX, IDC_NOTES, *m_pex_notes);
  DDX_Control(pDX, IDC_URL, m_ex_URL);
  DDX_Control(pDX, IDC_EMAIL, m_ex_email);
  DDX_Control(pDX, IDC_VIEWDEPENDENTS, m_ViewDependentsBtn);

  if (M_uicaller() != IDS_ADDENTRY) {
    DDX_Control(pDX, IDC_STATIC_GROUP, m_stc_group);
    DDX_Control(pDX, IDC_STATIC_TITLE, m_stc_title);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_stc_username);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stc_password);
    DDX_Control(pDX, IDC_STATIC_NOTES, m_stc_notes);
    DDX_Control(pDX, IDC_STATIC_URL, m_stc_URL);
    DDX_Control(pDX, IDC_STATIC_EMAIL, m_stc_email);
  }

  if (M_uicaller() == IDS_EDITENTRY && M_protected() != 0)
    DDX_Control(pDX, IDC_STATIC_PROTECTED, m_stc_protected);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Basic, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Basic)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowPassword)
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)
  ON_BN_CLICKED(IDC_LAUNCH, OnLaunch)
  ON_BN_CLICKED(IDC_SENDEMAIL, OnSendEmail)
  ON_BN_CLICKED(IDC_VIEWDEPENDENTS, OnViewDependents)

  ON_CBN_SELCHANGE(IDC_GROUP, OnGroupComboChanged)
  ON_CBN_EDITCHANGE(IDC_GROUP, OnGroupComboChanged)
  ON_EN_CHANGE(IDC_TITLE, OnChanged)
  ON_EN_CHANGE(IDC_USERNAME, OnChanged)
  ON_EN_CHANGE(IDC_PASSWORD2, OnChanged)
  ON_EN_CHANGE(IDC_NOTES, OnENChangeNotes)

  ON_EN_CHANGE(IDC_URL, OnENChangeURL)
  ON_EN_CHANGE(IDC_EMAIL, OnENChangeEmail)
  ON_EN_CHANGE(IDC_PASSWORD, OnENChangePassword)

  ON_EN_SETFOCUS(IDC_PASSWORD, OnENSetFocusPassword)
  ON_EN_SETFOCUS(IDC_PASSWORD2, OnENSetFocusPassword2)
  ON_EN_SETFOCUS(IDC_NOTES, OnENSetFocusNotes)
  ON_EN_KILLFOCUS(IDC_NOTES, OnENKillFocusNotes)

  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_GROUP, IDC_STATIC_EMAIL, OnSTCExClicked)

  ON_MESSAGE(PWS_MSG_CALL_EXTERNAL_EDITOR, OnCallExternalEditor)
  ON_MESSAGE(PWS_MSG_EXTERNAL_EDITOR_ENDED, OnExternalEditorEnded)
  ON_MESSAGE(PWS_MSG_EDIT_WORDWRAP, OnWordWrap)
  ON_MESSAGE(PWS_MSG_EDIT_SHOWNOTES, OnShowNotes)
  ON_MESSAGE(PWS_MSG_CALL_NOTESZOOMIN, OnZoomNotes)
  ON_MESSAGE(PWS_MSG_CALL_NOTESZOOMOUT, OnZoomNotes)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic message handlers

BOOL CAddEdit_Basic::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSWORD2));

  // Need to get change notifcations
  m_pex_notes->SetEventMask(ENM_CHANGE | m_pex_notes->GetEventMask());

  if (M_uicaller() == IDS_EDITENTRY && M_protected() != 0) {
    GetDlgItem(IDC_STATIC_PROTECTED)->ShowWindow(SW_SHOW);
    m_stc_protected.SetColour(RGB(255,0,0));
  }

  if (M_uicaller() != IDS_ADDENTRY) {
    m_pToolTipCtrl = new CToolTipCtrl;
    if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
      pws_os::Trace(L"Unable To create CAddEdit_Basic Dialog ToolTip\n");
      delete m_pToolTipCtrl;
      m_pToolTipCtrl = NULL;
    } else {
      EnableToolTips();

      m_pToolTipCtrl->SetMaxTipWidth(300);

      CString cs_ToolTip;
      cs_ToolTip.LoadString(IDS_CLICKTOCOPY);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_GROUP), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_TITLE), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_USERNAME), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_PASSWORD), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_NOTES), cs_ToolTip);
      cs_ToolTip.LoadString(IDS_CLICKTOCOPY);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_URL), cs_ToolTip);
      cs_ToolTip.LoadString(IDS_CLICKTOCOPYPLUS1);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_EMAIL), cs_ToolTip);
      cs_ToolTip.LoadString(IDS_CLICKTOGOPLUS);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_LAUNCH), cs_ToolTip);
      cs_ToolTip.LoadString(IDS_CLICKTOSEND);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_SENDEMAIL), cs_ToolTip);

      if (M_uicaller() == IDS_EDITENTRY && M_protected() != 0) {
        cs_ToolTip.LoadString(IDS_UNPROTECT);
        m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_PROTECTED), cs_ToolTip);
      }

      m_pToolTipCtrl->Activate(TRUE);
    }

    m_stc_group.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_title.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_username.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_password.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_notes.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_URL.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_email.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  }

  m_ex_group.ChangeColour();
  GetDlgItem(IDC_LAUNCH)->EnableWindow(M_URL().IsEmpty() ? FALSE : TRUE);
  GetDlgItem(IDC_SENDEMAIL)->EnableWindow(M_email().IsEmpty() ? FALSE : TRUE);

  if (M_uicaller() == IDS_VIEWENTRY ||
      (M_uicaller() == IDS_EDITENTRY && M_protected() != 0)) {
    // Change 'OK' to 'Close' and disable 'Cancel'
    // "CancelToClose" disables the System Command SC_CLOSE
    // from clicking X on PropertySheet so have implemented
    // CAddEdit_PropertySheet::OnSysCommand to deal with it.
    CancelToClose();

    // Disable Group Combo
    GetDlgItem(IDC_GROUP)->EnableWindow(FALSE);

    // Disable normal Edit controls
    GetDlgItem(IDC_TITLE)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_USERNAME)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_PASSWORD)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_PASSWORD2)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_NOTES)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_URL)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_EMAIL)->SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable Button
    GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
  }

  m_pex_notes->EnableWindow(m_bWordWrap ? FALSE : TRUE);
  m_pex_notes->ShowWindow(m_bWordWrap ? SW_HIDE : SW_SHOW);

  // Populate the combo box
  m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)

  // The core function "GetUniqueGroups(vGroups)" returns the group list by
  // going through the entries in the database. This will not include empty
  // groups.  However, we already maintain this list in the UI to save the
  // display status, so use this instead.
  std::vector<std::wstring> vGroups;
  M_pDbx()->GetAllGroups(vGroups);

  for (std::vector<std::wstring>::iterator iter = vGroups.begin();
       iter != vGroups.end(); ++iter) {
    m_ex_group.AddString(iter->c_str());
  }

  size_t num_unkwn(0);
  if (M_uicaller() != IDS_ADDENTRY)
    num_unkwn = M_pci()->NumberUnknownFields();

  if (num_unkwn > 0) {
    CString cs_text;
    cs_text.Format(IDS_RECORDUNKNOWNFIELDS, num_unkwn);
    GetDlgItem(IDC_RECORDUNKNOWNFIELDS)->SetWindowText(cs_text);
    GetDlgItem(IDC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATICGROUPRUNKNFLDS)->ShowWindow(SW_SHOW);
  } else {
    GetDlgItem(IDC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_RECORDUNKNOWNFIELDS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATICGROUPRUNKNFLDS)->ShowWindow(SW_HIDE);
  }

  // Note shortcuts have their own dialog for edit.
  CString cs_text;
  if (M_original_entrytype() == CItemData::ET_ALIASBASE ||
      M_original_entrytype() == CItemData::ET_SHORTCUTBASE) {
    // Show button to allow users to view dependents
    cs_text.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ?
                       IDS_VIEWALIASESBTN : IDS_VIEWSHORTCUTSBTN);
    GetDlgItem(IDC_VIEWDEPENDENTS)->SetWindowText(cs_text);
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_SHOW);

    cs_text.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ?
                       IDS_ISANALIASBASE : IDS_ISASHORTCUTBASE);
    GetDlgItem(IDC_STATIC_ISANALIAS)->SetWindowText(cs_text);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_ALIAS) {
    // Update password to alias form
    // Show text stating that it is an alias
    M_realpassword() = M_oldRealPassword() = m_password = m_password2 = M_base();
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);

    cs_text.Format(IDS_ISANALIAS, M_base());
    GetDlgItem(IDC_STATIC_ISANALIAS)->SetWindowText(cs_text);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_NORMAL) {
    // Normal - do none of the above
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_HIDE);
  }

  PWSprefs *prefs = PWSprefs::GetInstance();
  if (prefs->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  if (prefs->GetPref(PWSprefs::ShowNotesDefault)) {
    ShowNotes();
  } else {
    HideNotes();
  }

  CHARFORMAT cf = {0};
  cf.cbSize = sizeof(cf);

  m_pex_notes->SetSel(0, -1);
  m_pex_notes->SendMessage(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
  m_iPointSize = cf.yHeight / 20;
  m_pex_notes->Clear();

  if (m_isNotesHidden) {
    m_pex_notes->EnableMenuItem(PWS_MSG_CALL_NOTESZOOMIN, false);
    m_pex_notes->EnableMenuItem(PWS_MSG_CALL_NOTESZOOMOUT, false);
  } else {
    // If at the limit - don't allow to be called again in that direction
    SetZoomMenu();
  }

  UpdateData(FALSE);
  m_bInitdone = true;
  return TRUE;
}

void CAddEdit_Basic::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/entering_pwd.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

HBRUSH CAddEdit_Basic::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CAddEdit_PropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC && M_uicaller() != IDS_ADDENTRY) {
    COLORREF *pcfOld;
    UINT nID = pWnd->GetDlgCtrlID();
    switch (nID) {
      case IDC_STATIC_GROUP:
        pcfOld = &m_group_cfOldColour;
        break;
      case IDC_STATIC_TITLE:
        pcfOld = &m_title_cfOldColour;
        break;
      case IDC_STATIC_USERNAME:
        pcfOld = &m_user_cfOldColour;
        break;
      case IDC_STATIC_PASSWORD:
        pcfOld = &m_pswd_cfOldColour;
        break;
      case IDC_STATIC_NOTES:
        pcfOld = &m_notes_cfOldColour;
        break;
      case IDC_STATIC_URL:
        pcfOld = &m_URL_cfOldColour;
        break;
      case IDC_STATIC_EMAIL:
        pcfOld = &m_email_cfOldColour;
        break;
      case IDC_STATIC_PROTECTED:
        pcfOld = &m_protected_cfOldColour;
        break;
      default:
        // Not one of ours - get out quick
        return hbr;
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

BOOL CAddEdit_Basic::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}

LRESULT CAddEdit_Basic::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      switch (M_uicaller()) {
        case IDS_EDITENTRY:
          if (M_group()        != M_pci()->GetGroup()      ||
              M_title()        != M_pci()->GetTitle()      ||
              M_username()     != M_pci()->GetUser()       ||
              M_realnotes()    != M_originalrealnotesTRC() ||
              M_URL()          != M_pci()->GetURL()        ||
              M_email()        != M_pci()->GetEmail()      ||
              M_symbols()      != M_pci()->GetSymbols()    ||
              M_realpassword() != M_oldRealPassword())
            return 1L;
          break;
        case IDS_ADDENTRY:
          if (!M_group().IsEmpty()        ||
              !M_title().IsEmpty()        ||
              !M_username().IsEmpty()     ||
              !M_realpassword().IsEmpty() ||
              !M_realnotes().IsEmpty()    ||
              !M_URL().IsEmpty()          ||
              !M_email().IsEmpty()        ||
              !M_symbols().IsEmpty()        )
            return 1L;
          break;
      }
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
      break;
  }
  return 0L;
}

BOOL CAddEdit_Basic::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  // Do tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  // if user hit Ctrl+A in Notes control, then SelectAllNotes
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'A' &&
      (GetKeyState(VK_CONTROL) & 0x8000)  == 0x8000 &&
      m_pex_notes->m_hWnd == ::GetFocus()) {
    SelectAllNotes();
    return TRUE;
  }

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_CONTROL &&
      !m_bLaunchPlus && GetDlgItem(IDC_LAUNCH)->IsWindowEnabled()) {
    CString cs_text(MAKEINTRESOURCE(IDS_LAUNCHPLUS));
    GetDlgItem(IDC_LAUNCH)->SetWindowText(cs_text);
    m_bLaunchPlus = true;
    return TRUE;
  }

  if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_CONTROL &&
      m_bLaunchPlus && GetDlgItem(IDC_LAUNCH)->IsWindowEnabled()) {
    CString cs_text(MAKEINTRESOURCE(IDS_LAUNCH));
    GetDlgItem(IDC_LAUNCH)->SetWindowText(cs_text);
    m_bLaunchPlus = false;
    return TRUE;
  }

  if (pMsg->message == WM_KEYDOWN && 
      (pMsg->wParam == VK_ADD || pMsg->wParam == VK_SUBTRACT) &&
      (GetKeyState(VK_CONTROL) & 0x8000) == 0x8000 &&
      m_pex_notes->m_hWnd == ::GetFocus()) {
    OnZoomNotes(0, pMsg->wParam == VK_ADD ? 1 : -1);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_Basic::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE; //CAddEdit_PropertyPage::OnApply();

  CWnd *pFocus(NULL);
  CGeneralMsgBox gmb;
  ItemListIter listindex;
  bool brc, b_msg_issued;

  UpdateData(TRUE);

  M_group().EmptyIfOnlyWhiteSpace();
  M_title().EmptyIfOnlyWhiteSpace();
  M_username().EmptyIfOnlyWhiteSpace();
  M_URL().EmptyIfOnlyWhiteSpace();
  M_email().EmptyIfOnlyWhiteSpace();
  M_symbols().EmptyIfOnlyWhiteSpace();

  m_notes.EmptyIfOnlyWhiteSpace();

  if (m_password.IsOnlyWhiteSpace()) {
    m_password.Empty();
    if (m_isPWHidden)
      m_password2.Empty();
  }

  if (!m_isPWHidden || m_password != HIDDEN_PASSWORD)
    M_realpassword() = m_password;

  if (!m_isNotesHidden)
    M_realnotes() = m_notes;

  UpdateData(FALSE);

  //Check that data is valid
  if (M_title().IsEmpty()) {
    gmb.AfxMessageBox(IDS_MUSTHAVETITLE);
    pFocus = &m_ex_title;
    goto error;
  }

  if (M_realpassword().IsEmpty()) {
    gmb.AfxMessageBox(IDS_MUSTHAVEPASSWORD);
    pFocus = &m_ex_password;
    goto error;
  }

  if (!M_group().IsEmpty() && M_group()[0] == '.') {
    gmb.AfxMessageBox(IDS_DOTINVALID);
    pFocus = &m_ex_group;
    goto error;
  }

  if (m_isPWHidden && (m_password.Compare(m_password2) != 0)) {
    gmb.AfxMessageBox(IDS_PASSWORDSNOTMATCH);
    UpdateData(FALSE);
    pFocus = &m_ex_password;
    goto error;
  }

  // If there is a matching entry in our list, tell the user to try again.
  listindex = M_pDbx()->Find(M_group(), M_title(), M_username());
  if (M_uicaller() == IDS_ADDENTRY) {
    // Add entry
    if (listindex != M_pDbx()->End()) {
      CSecString temp;
      if (M_group().IsEmpty())
        if (M_username().IsEmpty())
          temp.Format(IDS_ENTRYEXISTS3, M_title());
        else
          temp.Format(IDS_ENTRYEXISTS2, M_title(), M_username());
      else
        if (M_username().IsEmpty())
          temp.Format(IDS_ENTRYEXISTS1, M_group(), M_title());
        else
          temp.Format(IDS_ENTRYEXISTS, M_group(), M_title(), M_username());

      gmb.AfxMessageBox(temp);
      pFocus = &m_ex_title;
      goto error;
    }
  } else {
    // Edit entry
    if (listindex != M_pDbx()->End()) {
      const CItemData &listItem = M_pDbx()->GetEntryAt(listindex);
      if (listItem.GetUUID() != M_pci()->GetUUID()) {
        CSecString temp;
        temp.Format(IDS_ENTRYEXISTS, M_group(), M_title(), M_username());
        gmb.AfxMessageBox(temp);
        pFocus = &m_ex_title;
        goto error;
      }
    }
  }

  brc = CheckNewPassword(M_group(), M_title(), M_username(), M_realpassword(),
                         M_uicaller() != IDS_ADDENTRY, CItemData::ET_ALIAS,
                         M_base_uuid(), M_ibasedata(), b_msg_issued);

  if (!brc && M_ibasedata() != 0) {
    if (!b_msg_issued)
      gmb.AfxMessageBox(IDS_MUSTHAVETARGET, MB_OK);

    UpdateData(FALSE);
    pFocus = &m_ex_password;
    goto error;
  }
  //End check

  return CAddEdit_PropertyPage::OnApply();

error:
  // Are we the current page, if not activate this page
  if (m_ae_psh->GetActivePage() != (CAddEdit_PropertyPage *)this)
    m_ae_psh->SetActivePage(this);

  if (pFocus != NULL)
    pFocus->SetFocus();

  if (pFocus == &m_ex_title)
    m_ex_title.SetSel(MAKEWORD(-1, 0));

  return FALSE;
}

void CAddEdit_Basic::OnShowPassword()
{
  UpdateData(TRUE);

  if (m_isPWHidden) {
    ShowPassword();
  } else {
    M_realpassword() = m_password; // save visible password
    HidePassword();
  }
  UpdateData(FALSE);
}

void CAddEdit_Basic::OnENSetFocusPassword()
{
  m_ex_password.SetSel(0, -1);
}

void CAddEdit_Basic::OnENSetFocusPassword2()
{
  m_ex_password2.SetSel(0, -1);
}

void CAddEdit_Basic::OnENChangePassword()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
  M_realpassword() = m_password;
}

void CAddEdit_Basic::ShowPassword()
{
  m_isPWHidden = false;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_HIDE);

  m_password = M_realpassword();
  m_ex_password.SetSecure(false);

  // Remove password character so that the password is displayed
  m_ex_password.SetPasswordChar(0);
  m_ex_password.Invalidate();

  // Don't need verification as the user can see the password entered
  m_password2.Empty();
  m_ex_password2.EnableWindow(FALSE);
  m_ex_password2.SetPasswordChar(0);
  m_ex_password2.Invalidate();
}

void CAddEdit_Basic::HidePassword()
{
  m_isPWHidden = true;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_SHOW);

  m_ex_password.SetSecure(true);

  // Set password character so that the password is not displayed
  m_ex_password.SetPasswordChar(PSSWDCHAR);
  m_ex_password.Invalidate();

  // Need verification as the user can not see the password entered
  m_password2 = m_password;
  m_ex_password2.SetSecureText(m_password2);
  m_ex_password2.EnableWindow(TRUE);
  m_ex_password2.SetPasswordChar(PSSWDCHAR);
  m_ex_password2.Invalidate();
}

LRESULT CAddEdit_Basic::OnShowNotes(WPARAM, LPARAM)
{
  UpdateData(TRUE);
  if (m_isNotesHidden) {
    ShowNotes();
  } else {
    HideNotes();
  }
  UpdateData(FALSE);

  m_pex_notes->UpdateState(PWS_MSG_EDIT_SHOWNOTES, m_isNotesHidden ? FALSE : TRUE);

  return 0L;
}

void CAddEdit_Basic::SetZoomMenu()
{
    m_pex_notes->EnableMenuItem(PWS_MSG_CALL_NOTESZOOMIN, m_iPointSize < 72);
    m_pex_notes->EnableMenuItem(PWS_MSG_CALL_NOTESZOOMOUT, m_iPointSize > 6);
}

LRESULT CAddEdit_Basic::OnZoomNotes(WPARAM, LPARAM lParam)
{
  // Zoom into and out of notes (by menu and also by keyboard)
  UpdateData(TRUE);

  ASSERT(lParam != 0);

  if ((lParam < 0 && m_iPointSize <= 6) || (lParam > 0 && m_iPointSize >= 72))
    return 0L;

  WPARAM wp_increment = (lParam > 0 ? 1 : -1) * 2;

  long nStart, nEnd;
  m_pex_notes->GetSel(nStart, nEnd);

  m_pex_notes->SetSel(0, -1);
  m_pex_notes->HideSelection(TRUE, FALSE);
  m_pex_notes->SendMessage(EM_SETFONTSIZE, wp_increment, 0);

  CHARFORMAT cf = {0};
  cf.cbSize = sizeof(cf);

  m_pex_notes->SendMessage(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
  m_iPointSize = cf.yHeight / 20;

  m_pex_notes->SetSel(nStart, nEnd);

  SetZoomMenu();
  return 0L;
}

void CAddEdit_Basic::ShowNotes()
{
  m_isNotesHidden = false;
  m_notes = M_realnotes();

  ((CEdit *)GetDlgItem(IDC_NOTES))->Invalidate();

  // If at the limit - don't allow to be called again in that direction
  SetZoomMenu();
 }

void CAddEdit_Basic::HideNotes()
{
  m_isNotesHidden = true;
  M_realnotes() = m_notes;

  m_notes = HIDDEN_NOTES;

  ((CEdit *)GetDlgItem(IDC_NOTES))->Invalidate();

  // Disable zoom of hidden text
  m_pex_notes->EnableMenuItem(PWS_MSG_CALL_NOTESZOOMIN, false);
  m_pex_notes->EnableMenuItem(PWS_MSG_CALL_NOTESZOOMOUT, false);
}

void CAddEdit_Basic::OnGeneratePassword()
{
  UpdateData(TRUE);

  if (QuerySiblings(PP_UPDATE_PWPOLICY, 0L) != 0L) {
    return;
  }

  INT_PTR rc(CChangeAliasPswd::CHANGEALIAS);
  if (M_original_entrytype() == CItemData::ET_ALIAS) {
    CChangeAliasPswd dlgchngepswd;
    CSecString cs_base = M_base();
    cs_base.Replace(L"[", L"\xab");
    cs_base.Replace(L":", L"\xbb \xab");
    cs_base.Replace(L"]", L"\xbb");
    dlgchngepswd.m_BaseEntry = (CString)cs_base;
    rc = dlgchngepswd.DoModal();
    switch (rc) {
      case IDCANCEL:
        return;
      case CChangeAliasPswd::CHANGEBASE:
        // Change Base
        break;
      case CChangeAliasPswd::CHANGEALIAS:
        // Change Alias
        break;
      default:
        ASSERT(0);
    }
  }

  StringX passwd;
  if (M_ipolicy() == NAMED_POLICY) {
    st_PSWDPolicy st_pp;
    M_pDbx()->GetPolicyFromName(M_policyname(), st_pp);
    M_pDbx()->MakeRandomPassword(passwd, st_pp.pwp, st_pp.symbols.c_str());
  } else {
    M_pDbx()->MakeRandomPassword(passwd, M_pwp(), M_symbols());
  }

  if (rc == CChangeAliasPswd::CHANGEBASE) {
    // Change Base
    ItemListIter iter = M_pDbx()->Find(M_base_uuid());
    ASSERT(iter != M_pcore()->GetEntryEndIter());
    CItemData *pci = &iter->second;
    Command *pcmd = UpdatePasswordCommand::Create(M_pcore(), *pci,
                                                  passwd);
    M_pDbx()->Execute(pcmd);
  } else {
    M_realpassword() = m_password = passwd.c_str();
    if (m_isPWHidden) {
      m_password2 = m_password;
    }
    m_ae_psh->SetChanged(true);
    UpdateData(FALSE);
  }

  QuerySiblings(PP_UPDATE_PWPOLICY, 0L);
}

void CAddEdit_Basic::OnGroupComboChanged()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic::OnChanged()
{
  if (!m_bInitdone || m_AEMD.uicaller != IDS_EDITENTRY)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic::OnENChangeNotes()
{
  if (!m_bInitdone || m_AEMD.uicaller != IDS_EDITENTRY)
    return;

  m_ae_psh->SetChanged(true);
  m_ae_psh->SetNotesChanged(true); // Needed if Notes field is long and will be truncated
  UpdateData(TRUE);
}

void CAddEdit_Basic::OnENChangeURL()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
  GetDlgItem(IDC_LAUNCH)->EnableWindow(M_URL().IsEmpty() ? FALSE : TRUE);
}

void CAddEdit_Basic::OnENChangeEmail()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
  GetDlgItem(IDC_SENDEMAIL)->EnableWindow(M_email().IsEmpty() ? FALSE : TRUE);
}

void CAddEdit_Basic::OnSTCExClicked(UINT nID)
{
  UpdateData(TRUE);

  StringX cs_data;
  int iaction(0);

  // NOTE: These values must be contiguous in "resource.h"
  switch (nID) {
    case IDC_STATIC_GROUP:
      m_stc_group.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = StringX(M_group());
      iaction = CItemData::GROUP;
      break;
    case IDC_STATIC_TITLE:
      m_stc_title.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = StringX(M_title());
      iaction = CItemData::TITLE;
      break;
    case IDC_STATIC_USERNAME:
      m_stc_username.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = StringX(M_username());
      iaction = CItemData::USER;
      break;
    case IDC_STATIC_PASSWORD:
      m_stc_password.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = StringX(M_realpassword());
      iaction = CItemData::PASSWORD;
      break;
    case IDC_STATIC_NOTES:
      m_stc_notes.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = StringX(M_realnotes());
      iaction = CItemData::NOTES;
      break;
    case IDC_STATIC_URL:
      cs_data = StringX(M_URL());
      m_stc_URL.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      iaction = CItemData::URL;
      break;
    case IDC_STATIC_EMAIL:
      cs_data = StringX(M_email());
      // If Ctrl pressed - also copy to URL field with the 'mailto:' prefix
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 && !M_email().IsEmpty()) {
        M_URL() = L"mailto:" + cs_data;
        UpdateData(FALSE);
      }
      m_stc_email.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      iaction = CItemData::EMAIL;
      break;
    default:
      ASSERT(0);
  }
  M_pDbx()->SetClipboardData(cs_data);
  M_pDbx()->UpdateLastClipboardAction(iaction);
}

void CAddEdit_Basic::SelectAllNotes()
{
  // Here from PreTranslateMessage iff User pressed Ctrl+A
  // in Notes control
  ((CEdit *)GetDlgItem(IDC_NOTES))->SetSel(0, -1, TRUE);
}

LRESULT CAddEdit_Basic::OnWordWrap(WPARAM, LPARAM)
{
  m_bWordWrap = !m_bWordWrap;
  m_pex_notes->SetTargetDevice(NULL, m_bWordWrap ? 0 : 1);

  m_pex_notes->UpdateState(PWS_MSG_EDIT_WORDWRAP, m_bWordWrap);

  return 0L;
}

void CAddEdit_Basic::OnENSetFocusNotes()
{
  UpdateData(TRUE);
  if (m_isNotesHidden) {
    ShowNotes();
  }
  UpdateData(FALSE);
}

void CAddEdit_Basic::OnENKillFocusNotes()
{
  UpdateData(TRUE);
  if (m_isNotesHidden) {
    HideNotes();
  } else
    M_realnotes() = m_notes;

  UpdateData(FALSE);
}

void CAddEdit_Basic::OnLaunch()
{
  UpdateData(TRUE);
  StringX sx_url = StringX(M_URL());
  std::vector<size_t> vactionverboffsets;
  StringX sx_autotype = PWSAuxParse::GetAutoTypeString(M_autotype(),
                                                       M_group(),
                                                       M_title(),
                                                       M_username(),
                                                       M_realpassword(),
                                                       M_realnotes(),
                                                       vactionverboffsets);

  const bool bDoAutoType = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  M_pDbx()->LaunchBrowser(sx_url.c_str(), sx_autotype, vactionverboffsets, bDoAutoType);
  M_pDbx()->UpdateLastClipboardAction(CItemData::URL);

  if (bDoAutoType) {
    // Reset button
    BYTE KeyState[256];
    ASSERT(GetKeyboardState(&KeyState[0]));
    KeyState[VK_CONTROL] = 0x80;
    ASSERT(SetKeyboardState(&KeyState[0]));
    CString cs_text(MAKEINTRESOURCE(IDS_LAUNCH));
    GetDlgItem(IDC_LAUNCH)->SetWindowText(cs_text);
  }
  m_bLaunchPlus = false;
}

void CAddEdit_Basic::OnSendEmail()
{
  UpdateData(TRUE);
  StringX sx_email = StringX(M_email());

  M_pDbx()->SendEmail(sx_email.c_str());
  M_pDbx()->UpdateLastClipboardAction(CItemData::EMAIL);
}

LRESULT CAddEdit_Basic::OnCallExternalEditor(WPARAM, LPARAM)
{
  // Warn the user about sensitive data lying around
  CGeneralMsgBox gmb;
  INT_PTR rc = gmb.AfxMessageBox(IDS_EXTERNAL_EDITOR_WARNING,
                         MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2);
  if (rc != IDYES)
    return 0L;

  m_bOKSave = GetParent()->GetDlgItem(IDOK)->EnableWindow(FALSE);
  m_bOKCancel = GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

  const CString cs_text(MAKEINTRESOURCE(IDS_NOTES_IN_EXTERNAL_EDITOR));
  CWnd *pwnotes = GetDlgItem(IDC_NOTES);
  pwnotes->EnableWindow(FALSE);
  pwnotes->SetWindowText(cs_text);

  m_thread = CExtThread::BeginThread(ExternalEditorThread, this);
  return 0L;
}

UINT CAddEdit_Basic::ExternalEditorThread(LPVOID me) // static method!
{
  CAddEdit_Basic *self = (CAddEdit_Basic *)me;
  CGeneralMsgBox gmb;

  wchar_t szExecName[MAX_PATH + 1];
  wchar_t lpPathBuffer[4096];
  DWORD dwBufSize(4096);

  // Get the temp path
  GetTempPath(dwBufSize,          // length of the buffer
              lpPathBuffer);      // buffer for path

  // Create a temporary file.
  GetTempFileName(lpPathBuffer,          // directory for temp files
                  L"NTE",                // temp file name prefix
                  0,                     // create unique name
                  self->m_szTempName);   // buffer for name

  // Open it and put the Notes field in it
  std::wofstream ofs(self->m_szTempName);
  if (ofs.bad())
    return 16;

  ofs << LPCWSTR(self->M_realnotes()) << std::endl;
  ofs.flush();
  ofs.close();

  StringX sxEditor = PWSprefs::GetInstance()->GetPref(PWSprefs::AltNotesEditor);
  if (sxEditor.empty()) {
    // Find out the users default editor for "txt" files
    DWORD dwSize(MAX_PATH);
    HRESULT stat = ::AssocQueryString(0, ASSOCSTR_EXECUTABLE, L".txt", L"Open",
                                      szExecName, &dwSize);
    if (int(stat) != S_OK) {
#ifdef _DEBUG
      gmb.AfxMessageBox(L"oops");
#endif
      return 16;
    }
    sxEditor = szExecName;
  }

  // Create an Edit process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

  DWORD dwCreationFlags(0);
  dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;

  CString cs_CommandLine;

  // Make the command line = "<program>" "file"
  cs_CommandLine.Format(L"\"%s\" \"%s\"", sxEditor.c_str(), self->m_szTempName);
  int ilen = cs_CommandLine.GetLength();
  LPWSTR pszCommandLine = cs_CommandLine.GetBuffer(ilen);

  if (!CreateProcess(NULL, pszCommandLine, NULL, NULL, FALSE, dwCreationFlags,
                     NULL, lpPathBuffer, &si, &pi)) {
    pws_os::Trace(L"CreateProcess failed (%d).\n", GetLastError());
    // Delete temporary file
    _wremove(self->m_szTempName);
    SecureZeroMemory(self->m_szTempName, sizeof(self->m_szTempName));
    return 0;
  }

  pws_os::Trace(L"%d\n", sizeof(self->m_szTempName));
  WaitForInputIdle(pi.hProcess, INFINITE);

  // Wait until child process exits.
  WaitForSingleObject(pi.hProcess, INFINITE);

  // Close process and thread handles.
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  cs_CommandLine.ReleaseBuffer();

  self->PostMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 0, 0);
  return 0;
}

LRESULT CAddEdit_Basic::OnExternalEditorEnded(WPARAM, LPARAM)
{
  GetDlgItem(IDC_NOTES)->EnableWindow(TRUE);

  // Now get what the user saved in this file and put it back into Notes field
  std::wifstream ifs(m_szTempName);
  if (ifs.bad())
    return 16;

  M_realnotes().Empty();
  std::wstring linebuf, note;

  // Get first line
  getline(ifs, note, L'\n');

  // Now get the rest (if any)
  while (!ifs.eof()) {
    getline(ifs, linebuf, L'\n');
    note += L"\r\n";
    note += linebuf;
  }

  ifs.close();

  if (note.length() > MAXTEXTCHARS) {
    note = note.substr(0, MAXTEXTCHARS);

    CGeneralMsgBox gmb;
    CString cs_text, cs_title(MAKEINTRESOURCE(IDS_WARNINGTEXTLENGTH));
    cs_text.Format(IDS_TRUNCATETEXT, MAXTEXTCHARS);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }

  // Set real notes field, and
  // we are still displaying the old text, so replace that too
  M_realnotes() = m_notes = note.c_str();

  UpdateData(FALSE);
  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();

  // Delete temporary file
  _wremove(m_szTempName);
  SecureZeroMemory(m_szTempName, sizeof(m_szTempName));

  // Restore Sheet buttons
  GetParent()->GetDlgItem(IDOK)->EnableWindow(m_bOKSave == 0 ? TRUE : FALSE);
  GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(m_bOKCancel == 0 ? TRUE : FALSE);

  OnENChangeNotes();
  return 0L;
}

void CAddEdit_Basic::OnViewDependents()
{
  CString cs_msg, cs_type;
  CGeneralMsgBox gmb;

  if (M_original_entrytype() == CItemData::ET_ALIASBASE)
    cs_type.LoadString(M_num_dependents() == 1 ? IDSC_ALIAS : IDSC_ALIASES);
  else
    cs_type.LoadString(M_num_dependents() == 1 ? IDSC_SHORTCUT : IDSC_SHORTCUTS);

  cs_msg.Format(IDS_VIEWDEPENDENTS, M_num_dependents(), cs_type, M_dependents());
  gmb.MessageBox(cs_msg, AfxGetAppName(), MB_OK);
}

bool CAddEdit_Basic::CheckNewPassword(const StringX &group, const StringX &title,
                                      const StringX &user, const StringX &password,
                                      const bool bIsEdit, const CItemData::EntryType InputType, 
                                      pws_os::CUUID &base_uuid, int &ibasedata, bool &b_msg_issued)
{
  // b_msg_issued - whether this routine issued a message
  b_msg_issued = false;
  CGeneralMsgBox gmb;

  // Called from Add and Edit entry
  // Returns false if not a special alias or shortcut password
  BaseEntryParms pl;
  pl.InputType = InputType;

  bool brc = M_pcore()->ParseBaseEntryPWD(password, pl);

  // Copy data back before possibly returning
  ibasedata = pl.ibasedata;
  base_uuid = pl.base_uuid;
  if (!brc)    
    return false;

  // if we ever return 'false', this routine will have issued a message to the user
  b_msg_issued = true;

  if (bIsEdit && 
    (pl.csPwdGroup == group && pl.csPwdTitle == title && pl.csPwdUser == user)) {
    // In Edit, check user isn't changing entry to point to itself (circular/self reference)
    // Can't happen during Add as already checked entry does not exist so if accepted the
    // password would be treated as an unusal "normal" password
    gmb.AfxMessageBox(IDS_ALIASCANTREFERTOITSELF, MB_OK);
    return false;
  }

  // ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and is only valid if n = -1 or -2.

  if (pl.ibasedata < 0) {
    if (InputType == CItemData::ET_SHORTCUT) {
      // Target must exist (unlike for aliases where it could be an unusual password)
      if (pl.bMultipleEntriesFound)
        gmb.AfxMessageBox(IDS_MULTIPLETARGETSFOUND, MB_OK);
      else
        gmb.AfxMessageBox(IDS_TARGETNOTFOUND, MB_OK);
      return false;
    }

    CString cs_msg;
    const CString cs_msgA(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDA));
    const CString cs_msgZ(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDZ));
    INT_PTR rc(IDNO);
    switch (pl.ibasedata) {
      case -1: // [t] - must be title as this is the only mandatory field
        if (pl.bMultipleEntriesFound)
          cs_msg.Format(IDS_ALIASNOTFOUND0A,
                        pl.csPwdTitle.c_str());  // multiple entries exist with title=x
        else
          cs_msg.Format(IDS_ALIASNOTFOUND0B,
                        pl.csPwdTitle.c_str());  // no entry exists with title=x
        rc = gmb.AfxMessageBox(cs_msgA + cs_msg + cs_msgZ,
                               NULL, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -2: // [g,t], [t:u]
        // In this case the 2 fields from the password are in Group & Title
        if (pl.bMultipleEntriesFound)
          cs_msg.Format(IDS_ALIASNOTFOUND1A, 
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str(),
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str());
        else
          cs_msg.Format(IDS_ALIASNOTFOUND1B, 
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str(),
                        pl.csPwdGroup.c_str(),
                        pl.csPwdTitle.c_str());
        rc = gmb.AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, 
                               NULL, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -3: // [g:t:u], [g:t:], [:t:u], [:t:] (title cannot be empty)
      {
        const bool bGE = pl.csPwdGroup.empty();
        const bool bTE = pl.csPwdTitle.empty();
        const bool bUE = pl.csPwdUser.empty();
        if (bTE) {
          // Title is mandatory for all entries!
          gmb.AfxMessageBox(IDS_BASEHASNOTITLE, MB_OK);
          rc = IDNO;
          break;
        } else if (!bGE && !bUE)  // [x:y:z]
          cs_msg.Format(IDS_ALIASNOTFOUND2A, 
                        pl.csPwdGroup.c_str(), 
                        pl.csPwdTitle.c_str(), 
                        pl.csPwdUser.c_str());
        else if (!bGE && bUE)     // [x:y:]
          cs_msg.Format(IDS_ALIASNOTFOUND2B, 
                        pl.csPwdGroup.c_str(), 
                        pl.csPwdTitle.c_str());
        else if (bGE && !bUE)     // [:y:z]
          cs_msg.Format(IDS_ALIASNOTFOUND2C, 
                        pl.csPwdTitle.c_str(), 
                        pl.csPwdUser.c_str());
        else if (bGE && bUE)      // [:y:]
          cs_msg.Format(IDS_ALIASNOTFOUND0B, 
                        pl.csPwdTitle.c_str());

        rc = gmb.AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, 
                               NULL, MB_YESNO | MB_DEFBUTTON2);
        break;
      }
      default:
        // Never happens
        ASSERT(0);
    }
    if (rc == IDNO)
      return false;
  }

  if (pl.ibasedata > 0) {
    if (pl.TargetType == CItemData::ET_ALIAS) {
      // If user tried to point to an alias -> change to point to the 'real' base
      CString cs_msg;
      cs_msg.Format(IDS_BASEISALIAS, 
                    pl.csPwdGroup.c_str(),
                    pl.csPwdTitle.c_str(),
                    pl.csPwdUser.c_str());
      if (gmb.AfxMessageBox(cs_msg, NULL, MB_YESNO | MB_DEFBUTTON2) == IDNO) {
        return false;
      }
    } else {
      if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
        // An alias can only point to a normal entry or an alias base entry
        CString cs_msg;
        cs_msg.Format(IDS_ABASEINVALID, 
                      pl.csPwdGroup.c_str(),
                      pl.csPwdTitle.c_str(), 
                      pl.csPwdUser.c_str());
        gmb.AfxMessageBox(cs_msg, NULL, MB_OK);
        return false;
      } else {
        return true;
      }
    }
  }
  return true; // All OK
}
