/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_Basic.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordSafe.h"

#include "DboxMain.h"

#include "AddEdit_Basic.h"
#include "AddEdit_PropertySheet.h"

#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"
#include "core/core.h"
#include "core/command.h"

#include "os/file.h"

#include "os/debug.h"

#include <shlwapi.h>
#include <fstream>
#include <limits>

#include "Richedit.h"

using pws_os::CUUID;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static wchar_t PSSWDCHAR = L'*';

HANDLE CAddEdit_Basic::ghEvents[2];

CString CAddEdit_Basic::CS_SHOW;
CString CAddEdit_Basic::CS_HIDE;
CString CAddEdit_Basic::CS_EXTERNAL_EDITOR;
CString CAddEdit_Basic::CS_HIDDEN_NOTES;

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic property page

IMPLEMENT_DYNAMIC(CAddEdit_Basic, CAddEdit_PropertyPage)

CAddEdit_Basic::CAddEdit_Basic(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent,
    CAddEdit_Basic::IDD, CAddEdit_Basic::IDD_SHORT,
    pAEMD),
  m_bInitdone(false), m_thread(NULL), m_isNotesHidden(false),
  m_bUsingNotesExternalEditor(false)
{
  if (CS_SHOW.IsEmpty()) { // one-time initializations
    CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT);
    CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT);
    CS_HIDDEN_NOTES.LoadString(IDS_HIDDENNOTES);
    CS_EXTERNAL_EDITOR.LoadString(IDS_NOTES_IN_EXTERNAL_EDITOR);
  }

  // Clear external editor events
  ghEvents[0] = ghEvents[1] = NULL;

  PWSprefs *prefs = PWSprefs::GetInstance();

  // Setup
  m_bWordWrap = prefs->GetPref(PWSprefs::NotesWordWrap);

  m_password = m_password2 = M_realpassword();

  if ((!M_pcore()->IsReadOnly() && M_protected() == 0) && 
      M_notes().GetLength() > MAXTEXTCHARS) {
    // Only truncate if in Edit mode
    M_notes() = M_notes().Left(MAXTEXTCHARS);

    CGeneralMsgBox gmb;
    CString cs_text, cs_title(MAKEINTRESOURCE(IDS_WARNINGTEXTLENGTH));
    cs_text.Format(IDS_TRUNCATETEXT, MAXTEXTCHARS);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }

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

  m_ex_notes.SetContextMenu(vmenu_items);
}

void CAddEdit_Basic::DoDataExchange(CDataExchange *pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_Basic)
  m_ex_password.DoDDX(pDX, m_password);
  m_ex_password2.DoDDX(pDX, m_password2);

  DDX_CBString(pDX, IDC_GROUP, (CString&)M_group());
  DDX_Text(pDX, IDC_TITLE, (CString&)M_title());
  DDX_Text(pDX, IDC_USERNAME, (CString&)M_username());
  DDX_Text(pDX, IDC_URL, (CString&)M_URL());
  DDX_Text(pDX, IDC_EMAIL, (CString&)M_email());
  DDX_Text(pDX, IDC_NOTES, (CString&)M_notes());

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
  DDX_Control(pDX, IDC_NOTES, m_ex_notes);
  DDX_Control(pDX, IDC_HIDDEN_NOTES, m_ex_hidden_notes);
  DDX_Control(pDX, IDC_URL, m_ex_URL);
  DDX_Control(pDX, IDC_EMAIL, m_ex_email);
  DDX_Control(pDX, IDC_MYBASE, m_ex_base);
  DDX_Control(pDX, IDC_LISTDEPENDENTS, m_cmbDependents);

  DDX_Control(pDX, IDC_STATIC_GROUP, m_stc_group);
  DDX_Control(pDX, IDC_STATIC_TITLE, m_stc_title);
  DDX_Control(pDX, IDC_STATIC_USERNAME, m_stc_username);
  DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stc_password);
  DDX_Control(pDX, IDC_STATIC_NOTES, m_stc_notes);
  DDX_Control(pDX, IDC_STATIC_URL, m_stc_URL);
  DDX_Control(pDX, IDC_STATIC_EMAIL, m_stc_email);
  DDX_Control(pDX, IDC_STATIC_ISANALIAS, m_stc_isdependent);
  DDX_Control(pDX, IDC_STATIC_DEPENDENT, m_stc_dependent);

  DDX_Control(pDX, IDC_SMARTLABELHELP, m_Help1);
  DDX_Control(pDX, IDC_PASSWORDHELP, m_Help2);
  DDX_Control(pDX, IDC_PASSWORDHELP2, m_Help3);
  DDX_Control(pDX, IDC_NOTESHELP, m_Help4);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Basic, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Basic)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowPassword)
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopyPassword)
  ON_BN_CLICKED(IDC_LAUNCH, OnLaunch)
  ON_BN_CLICKED(IDC_SENDEMAIL, OnSendEmail)

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
  ON_EN_KILLFOCUS(IDC_NOTES, OnENKillFocusNotes)

  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_GROUP, IDC_STATIC_EMAIL, OnSTCExClicked)

  ON_MESSAGE(PWS_MSG_CALL_EXTERNAL_EDITOR, OnCallExternalEditor)
  ON_MESSAGE(PWS_MSG_EXTERNAL_EDITOR_ENDED, OnExternalEditorEnded)
  ON_MESSAGE(PWS_MSG_EDIT_WORDWRAP, OnWordWrap)
  ON_MESSAGE(PWS_MSG_CALL_NOTESZOOMIN, OnZoomNotes)
  ON_MESSAGE(PWS_MSG_CALL_NOTESZOOMOUT, OnZoomNotes)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  ON_NOTIFY(PSN_KILLACTIVE, 0, OnPageKillActive)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic message handlers

BOOL CAddEdit_Basic::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  Fonts *pFonts = Fonts::GetInstance();
  pFonts->ApplyPasswordFont(&m_ex_password);
  pFonts->ApplyPasswordFont(&m_ex_password2);

  // Get Add/Edit font
  CFont *pFont = pFonts->GetAddEditFont();

  // Change font size of the group, title, username, URL & email fields
  m_ex_group.SetFont(pFont);
  m_ex_title.SetFont(pFont);
  m_ex_username.SetFont(pFont);
  m_ex_URL.SetFont(pFont);
  m_ex_email.SetFont(pFont);
  m_ex_base.SetFont(pFont);

  // Need to get change notifications
  m_ex_notes.SetEventMask(ENM_CHANGE | m_ex_notes.GetEventMask());

  // Set plain text - not that it seems to do much!
  m_ex_notes.SetTextMode(TM_PLAINTEXT);

  PWSprefs *prefs = PWSprefs::GetInstance();

  // Set Notes font!
  if (prefs->GetPref(PWSprefs::NotesFont).empty()) {
    m_ex_notes.SetFont(pFonts->GetAddEditFont());
    m_ex_hidden_notes.SetFont(pFonts->GetAddEditFont());
  } else {
    m_ex_notes.SetFont(pFonts->GetNotesFont());
    m_ex_hidden_notes.SetFont(pFonts->GetNotesFont());
  }

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_REDEXCLAMATION);
    m_Help4.Init(IDB_QUESTIONMARK);

    // Disable help for alias password for the moment
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_SMARTLABELHELP, IDS_SMARTLABELHELP);
    AddTool(IDC_PASSWORDHELP, IDS_PASSWORDHELP);
    AddTool(IDC_PASSWORDHELP2, IDS_PASSWORDHELP2);
    AddTool(IDC_NOTESHELP, IDS_NOTESHELP);

    // Old style tooltips
    AddTool(IDC_STATIC_EMAIL, IDS_CLICKTOCOPYPLUS1);
    AddTool(IDC_LAUNCH, IDS_CLICKTOGOPLUS);
    AddTool(IDC_SENDEMAIL, IDS_CLICKTOSEND);

    if (M_uicaller() == IDS_EDITENTRY && M_protected() != 0) {
      AddTool(IDC_STATIC_TUTORIAL, IDS_UNPROTECT);
    }

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
  }

  m_stc_group.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_title.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_username.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_password.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_notes.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_URL.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  m_stc_email.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);

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
    m_ex_group.EnableWindow(FALSE);

    // Disable normal Edit controls
    m_ex_title.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_username.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_password.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_password2.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_notes.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_hidden_notes.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_URL.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_email.SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable Button
    GetDlgItem(IDC_GENERATEPASSWORD)->EnableWindow(FALSE);
  }

  // Populate the combo box
  m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)

  std::vector<std::wstring> vGroups;
  GetMainDlg()->GetAllGroups(vGroups);

  for (std::vector<std::wstring>::iterator iter = vGroups.begin();
       iter != vGroups.end(); ++iter) {
    m_ex_group.AddString(iter->c_str());
  }

  // Make sure Group combobox is wide enough
  SetGroupComboBoxWidth();

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
    // Show ComboBox to allow users to view dependents
    SetUpDependentsCombo();
    m_cmbDependents.ShowWindow(SW_SHOW);
    m_cmbDependents.SetCurSel(0);

    cs_text.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ?
                       IDS_ISANALIASBASE : IDS_ISASHORTCUTBASE);
    m_stc_isdependent.SetWindowText(cs_text);

    m_stc_isdependent.ShowWindow(SW_SHOW);
    m_ex_base.ShowWindow(SW_HIDE);
    m_stc_dependent.ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_ALIAS) {
    // Update password to alias form
    // Show text stating that it is an alias
    m_password = m_password2 = M_base();
    m_cmbDependents.ShowWindow(SW_HIDE);

    m_stc_isdependent.ShowWindow(SW_SHOW);
    m_ex_base.SetWindowText(M_base());
    m_ex_base.ShowWindow(SW_SHOW);
    m_stc_dependent.ShowWindow(SW_HIDE);

    // Swap Help buttons
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);

    m_Help3.EnableWindow(TRUE);
    m_Help3.ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_NORMAL) {
    // Normal - do none of the above
    m_cmbDependents.ShowWindow(SW_HIDE);
    m_stc_isdependent.ShowWindow(SW_HIDE);
    m_ex_base.ShowWindow(SW_HIDE);
    m_stc_dependent.ShowWindow(SW_HIDE);
  }

  if (prefs->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  // Set "Hidden Notes" text
  m_ex_hidden_notes.SetWindowText(CS_HIDDEN_NOTES);
  // Make read only
  m_ex_hidden_notes.SendMessage(EM_SETREADONLY, TRUE, 0);

  if (prefs->GetPref(PWSprefs::ShowNotesDefault)) {
    ShowNotes(true);
  } else {
    HideNotes(true);
  }

  // Get current font size
  CHARFORMAT2 cf;
  memset(&cf, 0, sizeof(cf));
  cf.cbSize = sizeof(cf);
  m_ex_notes.GetDefaultCharFormat(cf);
  ASSERT((cf.dwMask & CFM_SIZE) == CFM_SIZE);
  m_iPointSize = cf.yHeight / 20;

  // Load copy password bitmap
  UINT nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
    IDB_COPYPASSWORD_NEW : IDB_COPYPASSWORD_CLASSIC;
  BOOL brc = m_CopyPswdBitmap.Attach(
                    ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                    MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                    (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

  ASSERT(brc);
  
  if (brc) {
    FixBitmapBackground(m_CopyPswdBitmap);
    CButton *pBtn = (CButton *)GetDlgItem(IDC_COPYPASSWORD);
    ASSERT(pBtn != NULL);
    if (pBtn != NULL)
      pBtn->SetBitmap(m_CopyPswdBitmap);
  }

  // Set initial Word Wrap
  m_ex_notes.SetTargetDevice(NULL, m_bWordWrap ? 0 : 1);
  m_ex_notes.UpdateState(PWS_MSG_EDIT_WORDWRAP, m_bWordWrap);

  UpdateData(FALSE);
  m_bInitdone = true;
  return TRUE;  // return TRUE unless you set the focus to a control
}

void CAddEdit_Basic::OnHelp()
{
  ShowHelp(L"::/html/entering_pwd.html");
}

HBRUSH CAddEdit_Basic::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CAddEdit_PropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC) {
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

void CAddEdit_Basic::OnPageKillActive(NMHDR *, LRESULT *pLResult)
{
  // Don't allow page switching if Notes being edited in the user's
  // external editor
  *pLResult = m_bUsingNotesExternalEditor ? 1 : 0;
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
              M_notes()        != M_originalnotesTRC()     ||
              M_URL()          != M_pci()->GetURL()        ||
              M_email()        != M_pci()->GetEmail()      ||
              (M_ipolicy()     != NAMED_POLICY &&
               M_symbols()     != M_pci()->GetSymbols())   ||
              M_realpassword() != M_oldRealPassword()      )
            return 1L;
          break;
        case IDS_ADDENTRY:
          if (!M_group().IsEmpty()        ||
              !M_title().IsEmpty()        ||
              !M_username().IsEmpty()     ||
              !M_realpassword().IsEmpty() ||
              !M_notes().IsEmpty()        ||
              !M_URL().IsEmpty()          ||
              !M_email().IsEmpty()        ||
              !M_symbols().IsEmpty()        )
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

BOOL CAddEdit_Basic::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  RelayToolTipEvent(pMsg);

  // Handle if user clicks on Hidden Notes
  if (m_ex_hidden_notes.m_hWnd == pMsg->hwnd) {
    switch (pMsg->message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
      if (!m_bUsingNotesExternalEditor)
        ShowNotes();
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      return TRUE;
    }
  }

  // Ctrl + 'key' in Notes
  if (pMsg->message == WM_KEYDOWN &&
      (GetKeyState(VK_CONTROL) & 0x8000) == 0x8000 &&
      m_ex_notes.m_hWnd == ::GetFocus()) {
    switch (pMsg->wParam) {
    case 'A':
      // Ctrl+A (Select All), then SelectAllNotes
      SelectAllNotes();
      return TRUE;
    case 'V':
      // Ctrl+V (Paste), then do PasteSpecial
      m_ex_notes.PasteSpecial(CF_UNICODETEXT);
      return TRUE;
    case VK_ADD:
    case VK_SUBTRACT:
      // Zoom in/out
      OnZoomNotes(0, pMsg->wParam == VK_ADD ? 1 : -1);
      return TRUE;
    }
  }

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_CONTROL &&
      GetDlgItem(IDC_LAUNCH)->IsWindowEnabled()) {
    CString cs_text(MAKEINTRESOURCE(m_bLaunchPlus ? IDS_LAUNCH : IDS_LAUNCHPLUS));
    GetDlgItem(IDC_LAUNCH)->SetWindowText(cs_text);
    m_bLaunchPlus = !m_bLaunchPlus;
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
  bool bPswdIsInAliasFormat, b_msg_issued;
  CSecString csBase(L"");

  UpdateData(TRUE);

  M_group().EmptyIfOnlyWhiteSpace();
  M_title().EmptyIfOnlyWhiteSpace();
  M_username().EmptyIfOnlyWhiteSpace();
  M_URL().EmptyIfOnlyWhiteSpace();
  M_email().EmptyIfOnlyWhiteSpace();
  M_symbols().EmptyIfOnlyWhiteSpace();

  M_notes().EmptyIfOnlyWhiteSpace();

  if (m_password.IsOnlyWhiteSpace()) {
    m_password.Empty();
    if (m_isPWHidden)
      m_password2.Empty();
  }

  if (!m_isPWHidden || m_password != HIDDEN_PASSWORD)
    M_realpassword() = m_password;

  UpdateData(FALSE);

  // Check that data is valid
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

  if (m_isPWHidden && m_password.Compare(m_password2) != 0) {
    gmb.AfxMessageBox(IDS_PASSWORDSNOTMATCH);
    UpdateData(FALSE);
    pFocus = &m_ex_password;
    goto error;
  }

  // If there is a matching entry in our list, tell the user to try again.
  listindex = GetMainDlg()->Find(M_group(), M_title(), M_username());
  if (listindex != GetMainDlg()->End()) {
    if (M_uicaller() == IDS_ADDENTRY) { // Add entry
      gmb.AfxMessageBox(IDS_ENTRYEXISTS, MB_OK | MB_ICONASTERISK);
      pFocus = &m_ex_title;
      goto error;
    } else { // Edit entry
      const CItemData &listItem = GetMainDlg()->GetEntryAt(listindex);
      if (listItem.GetUUID() != M_pci()->GetUUID()) {
        gmb.AfxMessageBox(IDS_ENTRYEXISTS, MB_OK | MB_ICONASTERISK);
        pFocus = &m_ex_title;
        goto error;
      }
    }
  }

  // Returns true if in alias format, false if not

  // ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and
  // is only valid if n = -1 or -2.
  bPswdIsInAliasFormat = CheckNewPassword(M_group(), M_title(), M_username(), M_realpassword(),
                                          M_uicaller() != IDS_ADDENTRY, CItemData::ET_ALIAS,
                                          M_base_uuid(), M_ibasedata(), b_msg_issued);

  if (!bPswdIsInAliasFormat && M_ibasedata() != 0) {
    if (!b_msg_issued)
      gmb.AfxMessageBox(IDS_MUSTHAVETARGET, MB_OK);

    UpdateData(FALSE);
    pFocus = &m_ex_password;
    goto error;
  }

  if (bPswdIsInAliasFormat && M_ibasedata() > 0) {
    if (M_original_entrytype() == CItemData::ET_ALIASBASE ||
        M_original_entrytype() == CItemData::ET_SHORTCUTBASE) {
      // User is trying to change a base to an alias!
      CString cs_errmsg, cs_title, cs_base, cs_alias;
      cs_base.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ? IDS_EXP_ABASE : IDS_EXP_SBASE);
      cs_alias.LoadString(IDS_EXP_ALIAS);
      cs_title.Format(IDS_CHANGINGBASEENTRY, static_cast<LPCWSTR>(cs_base),
                      static_cast<LPCWSTR>(cs_alias));
      cs_errmsg.Format(M_original_entrytype() == CItemData::ET_ALIASBASE ?
                       IDS_CHANGINGBASEENTRY1 : IDS_CHANGINGBASEENTRY2,
                       static_cast<LPCWSTR>(cs_alias));
      int rc = (int)gmb.MessageBox(cs_errmsg, cs_title, MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2);

      if (rc == IDNO) {
        UpdateData(FALSE);
        pFocus = &m_ex_password;
        goto error;
      }

      pws_os::CUUID entry_uuid = M_pci()->GetUUID();
      M_pci()->SetAlias();
      M_pci()->SetUUID(entry_uuid, CItemData::ALIASUUID);
      ShowHideBaseInfo(CItemData::ET_ALIAS, csBase);
    }
  }
  //End check

  if (!bPswdIsInAliasFormat && M_original_entrytype() == CItemData::ET_ALIAS) {
    // User has made this a normal entry
    M_pci()->SetNormal();
    ShowHideBaseInfo(CItemData::ET_NORMAL, csBase);
  }

  if (bPswdIsInAliasFormat && M_ibasedata() > 0) {
    if (M_original_base_uuid() != pws_os::CUUID::NullUUID() &&
        M_original_base_uuid() != M_base_uuid()) {
      // User has changed the alias to point to a different base entry
      CItemData *pbci(NULL);
      ItemListIter iter = M_pcore()->Find(M_base_uuid());
      if (iter != M_pcore()->GetEntryEndIter())
        pbci = &iter->second;

      ASSERT(pbci != NULL);

      if (pbci != NULL) {
        csBase = L"[" +
          pbci->GetGroup() + L":" +
          pbci->GetTitle() + L":" +
          pbci->GetUser() + L"]";
      } else
        csBase.Empty();

      M_pci()->SetAlias(); // Still an alias
      M_pci()->SetBaseUUID(M_base_uuid());
      ShowHideBaseInfo(CItemData::ET_ALIAS, csBase);
    }

    if (M_original_base_uuid() == pws_os::CUUID::NullUUID() &&
        M_original_base_uuid() != M_base_uuid()) {
      // User has changed the normal entry into an alias
      CItemData *pbci(NULL);
      ItemListIter iter = M_pcore()->Find(M_base_uuid());
      if (iter != M_pcore()->GetEntryEndIter())
        pbci = &iter->second;

      ASSERT(pbci != NULL);
      if (pbci != NULL) {
        csBase = L"[" +
          pbci->GetGroup() + L":" +
          pbci->GetTitle() + L":" +
          pbci->GetUser() + L"]";
      } else
        csBase.Empty();

      pws_os::CUUID entry_uuid = M_pci()->GetUUID();
      M_pci()->SetAlias();
      M_pci()->SetBaseUUID(M_base_uuid());
      M_pci()->SetUUID(entry_uuid, CItemData::ALIASUUID);
      ShowHideBaseInfo(CItemData::ET_ALIAS, csBase);
    }
  }

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

void CAddEdit_Basic::ShowHideBaseInfo(const CItemData::EntryType &entrytype, CSecString &csBase)
{
  switch (entrytype) {
  case CItemData::ET_ALIAS:
    m_cmbDependents.ShowWindow(SW_HIDE);
    m_stc_isdependent.ShowWindow(SW_SHOW);

    m_ex_base.SetWindowText(csBase);
    m_ex_base.ShowWindow(SW_SHOW);
    m_stc_dependent.ShowWindow(SW_HIDE);

    // Swap help buttons/information
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(TRUE);
    m_Help3.ShowWindow(SW_SHOW);
    break;
  case CItemData::ET_NORMAL:
    m_cmbDependents.ShowWindow(SW_HIDE);
    m_stc_isdependent.ShowWindow(SW_HIDE);

    m_ex_base.SetWindowText(L"");
    m_ex_base.ShowWindow(SW_HIDE);
    m_stc_dependent.ShowWindow(SW_HIDE);

    // Swap help buttons/information
    m_Help2.EnableWindow(TRUE);
    m_Help2.ShowWindow(SW_SHOW);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
    break;
  case CItemData::ET_ALIASBASE:
  case CItemData::ET_SHORTCUTBASE:
    break;
  // ET_SHORTCUT are edited via a different process
  // ET_LAST is just a placer
  case CItemData::ET_SHORTCUT:
  case CItemData::ET_LAST:
    break;
  case CItemData::ET_INVALID:
    ASSERT(0);
    break;
  }
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

void CAddEdit_Basic::OnShowPassword()
{
  // Prevent special OnCommand processing
  UpdateData(TRUE);

  if (m_isPWHidden) {
    ShowPassword();
  } else {
    M_realpassword() = m_password; // save visible password

    HidePassword();
  }
  UpdateData(FALSE);
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
  m_password2 = L"1234567890123456789012345678901234567890";
  m_ex_password2.SetSecure(false);
  m_ex_password2.EnableWindow(FALSE);
  m_ex_password2.SetPasswordChar(0);
  m_ex_password2.Invalidate();
}

void CAddEdit_Basic::HidePassword()
{
  m_isPWHidden = true;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_SHOW);

  m_ex_password.SetSecure(true);
  m_ex_password2.SetSecure(true);

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

void CAddEdit_Basic::SetZoomMenu()
{
  // If at the limit - don't allow to be called again in that direction
  m_ex_notes.EnableMenuItem(PWS_MSG_CALL_NOTESZOOMIN, m_iPointSize < 72);
  m_ex_notes.EnableMenuItem(PWS_MSG_CALL_NOTESZOOMOUT, m_iPointSize > 6);
}

LRESULT CAddEdit_Basic::OnZoomNotes(WPARAM, LPARAM lParam)
{
  // Zoom into and out of notes (by menu and also by keyboard)
  UpdateData(TRUE);

  ASSERT(lParam != 0);

  if ((lParam < 0 && m_iPointSize <= 6) || (lParam > 0 && m_iPointSize >= 72))
    return 0L;

  UINT wp_increment = (lParam > 0 ? 1 : -1) * 2;

  CHARFORMAT2 cf;
  memset(&cf, 0, sizeof(cf));
  cf.cbSize = sizeof(cf);
  cf.dwMask = CFM_SIZE;
  cf.yHeight = (m_iPointSize + wp_increment) * 20;
  m_ex_notes.SetDefaultCharFormat(cf);

  memset(&cf, 0, sizeof(cf));
  cf.cbSize = sizeof(cf);
  m_ex_notes.GetDefaultCharFormat(cf);
  ASSERT((cf.dwMask & CFM_SIZE) == CFM_SIZE);
  m_iPointSize = cf.yHeight / 20;

  SetZoomMenu();
  return 0L;
}

void CAddEdit_Basic::ShowNotes(const bool bForceShow)
{
  if (bForceShow || m_isNotesHidden) {
    m_isNotesHidden = false;

    // Swap windows
    m_ex_notes.ShowWindow(SW_SHOW);
    m_ex_notes.EnableWindow(TRUE);
    m_ex_hidden_notes.ShowWindow(SW_HIDE);
    m_ex_hidden_notes.EnableWindow(FALSE);

    m_ex_notes.SetFocus();

    SetZoomMenu();
  }
}

void CAddEdit_Basic::HideNotes(const bool bForceHide)
{
  if (bForceHide || !m_isNotesHidden) {
    m_isNotesHidden = true;

    // Swap windows
    m_ex_notes.ShowWindow(SW_HIDE);
    m_ex_notes.EnableWindow(FALSE);
    m_ex_hidden_notes.ShowWindow(SW_SHOW);
    m_ex_hidden_notes.EnableWindow(TRUE);
  }
}

void CAddEdit_Basic::OnGeneratePassword()
{
  UpdateData(TRUE);

  if (QuerySiblings(PP_UPDATE_PWPOLICY, 0L) != 0L) {
    return;
  }

  // Here we change the Alias's password
  StringX passwd;
  if (M_ipolicy() == NAMED_POLICY) {
    PWPolicy st_pp;
    GetMainDlg()->GetPolicyFromName(M_policyname(), st_pp);
    GetMainDlg()->MakeRandomPassword(passwd, st_pp);
  } else {
    PWPolicy policy(M_pwp());
    if (M_symbols().IsEmpty()) {
      // No specific entry symbols - use default
      policy.symbols = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultSymbols);
    } else {
      // This entry has its own list of symbols
      policy.symbols = LPCWSTR(M_symbols());
    }
    GetMainDlg()->MakeRandomPassword(passwd, policy);
  }

  M_realpassword() = m_password = passwd.c_str();
  if (m_isPWHidden) {
    m_password2 = m_password;
  }
  m_ae_psh->SetChanged(true);
  UpdateData(FALSE);

  QuerySiblings(PP_UPDATE_PWPOLICY, 0L);
}

void CAddEdit_Basic::OnGroupComboChanged()
{
  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic::OnChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic::OnENChangeNotes()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  // Called for any change - even just clicking on it - so check really changed
  if (m_ex_hidden_notes.IsWindowVisible()) {
    return;
  } else {
    CSecString current_notes;
    m_ex_notes.GetWindowText(current_notes);
    if (current_notes == M_notes())
      return;
  }

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

  CSecString cs_data;
  int iaction(0);

  // NOTE: These values must be contiguous in "resource.h"
  switch (nID) {
    case IDC_STATIC_GROUP:
      m_stc_group.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_group();
      iaction = CItemData::GROUP;
      break;
    case IDC_STATIC_TITLE:
      m_stc_title.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_title();
      iaction = CItemData::TITLE;
      break;
    case IDC_STATIC_USERNAME:
      m_stc_username.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_username();
      iaction = CItemData::USER;
      break;
    case IDC_STATIC_PASSWORD:
      m_stc_password.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_realpassword();
      iaction = CItemData::PASSWORD;
      break;
    case IDC_STATIC_NOTES:
      m_stc_notes.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = M_notes();
      iaction = CItemData::NOTES;
      break;
    case IDC_STATIC_URL:
      cs_data = M_URL();
      m_stc_URL.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      iaction = CItemData::URL;
      break;
    case IDC_STATIC_EMAIL:
      cs_data = M_email();
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
  GetMainDlg()->SetClipboardData(cs_data);
  GetMainDlg()->UpdateLastClipboardAction(iaction);
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
  m_ex_notes.SetTargetDevice(NULL, m_bWordWrap ? 0 : 1);

  m_ex_notes.UpdateState(PWS_MSG_EDIT_WORDWRAP, m_bWordWrap);

  // No idea why this is necessary but fixes the issue of no horizontaol
  // scroll bar active after turning off Word Wrap if the preferences
  // are Word Wrap and Hidden Notes. Without this "fix", user would have to
  // ensure that the Notes field looses focus by clicking on another field.
  m_ex_notes.EnableWindow(FALSE);
  m_ex_notes.EnableWindow(TRUE);

  if (m_isNotesHidden)
    ShowNotes();

  return 0L;
}

void CAddEdit_Basic::OnENKillFocusNotes()
{
  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault)) {
    HideNotes();
  }
}

void CAddEdit_Basic::OnLaunch()
{
  UpdateData(TRUE);
  std::vector<size_t> vactionverboffsets;

  CSecString sPassword(M_realpassword()), sLastPassword(M_lastpassword());
  if (m_AEMD.pci->IsAlias()) {
    CItemData *pciA = m_AEMD.pcore->GetBaseEntry(m_AEMD.pci);
    ASSERT(pciA != NULL);
    sPassword = pciA->GetPassword();
    sLastPassword = pciA->GetPreviousPassword();
  }

  StringX sx_autotype = PWSAuxParse::GetAutoTypeString(M_autotype(),
                                                       M_group(),
                                                       M_title(),
                                                       M_username(),
                                                       sPassword,
                                                       sLastPassword,
                                                       M_notes(),
                                                       M_URL(),
                                                       M_email(),
                                                       vactionverboffsets);

  const bool bDoAutoType = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  GetMainDlg()->LaunchBrowser(M_URL(), sx_autotype, vactionverboffsets, bDoAutoType);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::URL);

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

  GetMainDlg()->SendEmail(M_email());
  GetMainDlg()->UpdateLastClipboardAction(CItemData::EMAIL);
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

  m_ex_hidden_notes.SetWindowText(CS_EXTERNAL_EDITOR);
  HideNotes(true);
  m_bUsingNotesExternalEditor = true;

  // Clear events
  ghEvents[0] = ghEvents[1] = NULL;

  m_thread = CExtThread::BeginThread(ExternalEditorThread, this);
  return 0L;
}

UINT CAddEdit_Basic::ExternalEditorThread(LPVOID me) // static method!
{
  CAddEdit_Basic *self = (CAddEdit_Basic *)me;

  wchar_t szExecName[MAX_PATH + 1];
  wchar_t lpPathBuffer[4096];
  DWORD dwBufSize(4096);

  StringX sxEditorCmdLineParms = PWSprefs::GetInstance()->GetPref(PWSprefs::AltNotesEditorCmdLineParms);

  StringX sxEditor = PWSprefs::GetInstance()->GetPref(PWSprefs::AltNotesEditor);
  if (sxEditor.empty()) {
    // Find out the users default editor for "txt" files
    DWORD dwSize(MAX_PATH);
    HRESULT stat = ::AssocQueryString(0, ASSOCSTR_EXECUTABLE, L".txt", L"Open",
                                      szExecName, &dwSize);
    if (int(stat) != S_OK) {
#ifdef _DEBUG
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(L"oops");
#endif
      self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 8, 0);
      self->ResetHiddenNotes();
      return 8;
    }
    sxEditor = szExecName;
  }

  DWORD dwResult = ExpandEnvironmentStrings(sxEditor.c_str(), szExecName, MAX_PATH + 1);
  if (dwResult == 0 || dwResult > (MAX_PATH + 1)) {
    CGeneralMsgBox gmb;
    CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_EDITEXTERNALLY));
    cs_msg.Format(IDS_CANT_FIND_EXT_EDITOR, sxEditor.c_str());
    gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONEXCLAMATION);

    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 12, 0);
    self->ResetHiddenNotes();
    return 12;
  }

  sxEditor = szExecName;

  if (!pws_os::FileExists(sxEditor.c_str())) {
    CGeneralMsgBox gmb;
    CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_EDITEXTERNALLY));
    cs_msg.Format(IDS_CANT_FIND_EXT_EDITOR, sxEditor.c_str());
    gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONEXCLAMATION);

    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 16, 0);
    self->ResetHiddenNotes();
    return 16;
  }

  // Now we know the editor exists - go copy the data for it!
  // Get the temp path
  GetTempPath(dwBufSize,          // length of the buffer
              lpPathBuffer);      // buffer for path

  // Create a temporary file.
  GetTempFileName(lpPathBuffer,          // directory for temp files
                  L"NTE",                // temp file name prefix
                  0,                     // create unique name
                  self->m_szTempName);   // buffer for name

  FILE *fd;

  if ((fd = pws_os::FOpen(self->m_szTempName, L"w+b")) == NULL) {
    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 20, 0);
    self->ResetHiddenNotes();
    return 20;
  }

  // Write BOM
  const unsigned int iBOM = 0xFEFF;
  putwc(iBOM, fd);

  // Write out text
  fwrite(reinterpret_cast<const void *>((LPCWSTR)self->M_notes()), sizeof(BYTE),
             self->M_notes().GetLength() * sizeof(wchar_t), fd);

  // Close file before invoking editor
  fclose(fd);

  // Create an Edit process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

  DWORD dwCreationFlags;
  dwCreationFlags = CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED;

  CString cs_CommandLine;

  // Make the command line = "<program>" <parameters> "filename"
  // Note: the parameters reproduce the user's input as-is - it is up to them
  // to add quotes or not as required by their chosen external editor.
  // We add double quotes around the full path to the program and its name and
  // similarly for the temporary file name e.g.
  // "C:\somewhere\editor.exe" paramters "C:\somewhere_else\temp.txt"
  cs_CommandLine.Format(L"\"%s\" %s \"%s\"", sxEditor.c_str(),
                               sxEditorCmdLineParms.c_str(), self->m_szTempName);

  int ilen = cs_CommandLine.GetLength();
  LPWSTR pszCommandLine = cs_CommandLine.GetBuffer(ilen);

  if (!CreateProcess(NULL, pszCommandLine, NULL, NULL, FALSE, dwCreationFlags,
                     NULL, lpPathBuffer, &si, &pi)) {
    pws_os::IssueError(L"External Editor CreateProcess", false);
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANT_FIND_EXT_EDITOR, MB_OK | MB_ICONEXCLAMATION);

    // Delete temporary file
    _wremove(self->m_szTempName);
    SecureZeroMemory(self->m_szTempName, sizeof(self->m_szTempName));

    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 24, 0);
    self->ResetHiddenNotes();
    return 24;
  }

  // All set up now set process going
  ResumeThread(pi.hThread);

  WaitForInputIdle(pi.hProcess, INFINITE);

  // Disable me to stop other changes until editor ends or user cancels it
  self->EnableWindow(FALSE);

  // Wait until child process exits or we cancel it
  ghEvents[0] = pi.hProcess;                            // Thread ended
  ghEvents[1] = CreateEvent(NULL, FALSE, FALSE, NULL);  // We cancelled

  // Now wait for editor to end or user to cancel
  DWORD dwEvent = WaitForMultipleObjects(2, ghEvents, FALSE, INFINITE);

  // Close process and thread handles.
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  cs_CommandLine.ReleaseBuffer();

  // If edit ended normally (i.e. not cancelled by user) - go process
  // Otherwise indicate cancelled
  self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, dwEvent == (WAIT_OBJECT_0 + 0) ? 0 : 28, 0);

  self->ResetHiddenNotes();
  return 0;
}

void CAddEdit_Basic::ResetHiddenNotes()
{
  // Put back hidden notes message
  m_ex_hidden_notes.SetWindowText(CS_HIDDEN_NOTES);
  
  // Show notes again
  ShowNotes(true);

  // No longer in the external editor
  m_bUsingNotesExternalEditor = false;

  // Re-enable the property page
  EnableWindow(TRUE);
}

LRESULT CAddEdit_Basic::OnExternalEditorEnded(WPARAM wParam, LPARAM)
{
  std::wstring sNewNotes;
  CSecString sOldNotes = M_notes();

  if (wParam != 0) {
    // Tidy up and re-enable sheet OK/Cancel buttons
    goto error_exit;
  }

  // Now get what the user saved in this file and put it back into Notes field
  FILE *fd;

  if ((fd = pws_os::FOpen(m_szTempName, L"r+b")) == NULL) {
    goto error_exit;
  }

  M_notes().Empty();

  ulong64 flength = pws_os::fileLength(fd);

  ASSERT(flength % 2 == 0); // guess this is 'cause we assume editor saves wchar_t?

  size_t slength;
  if (flength > ((std::numeric_limits<size_t>::max)() - 2)) {
    // we're gonna truncate later in any case, due to the check
    // on MAXTEXTCHARS. This is just to keep memory in check
    slength = (std::numeric_limits<size_t>::max)() - 2;
  } else {
    slength = static_cast<size_t>(flength);
  }

  BYTE *pBuffer = new BYTE[slength + sizeof(wchar_t)];
  memset(pBuffer, 0, slength + sizeof(wchar_t));

  if (slength >= 2) {
    // Read in BOM and check it is
    const unsigned char BOM[] = {0xff, 0xfe};
    fread(pBuffer, 1, 2, fd);

    // if not a BOM - backspace and treat as data
    if (pBuffer[0] != BOM[0] || pBuffer[1] != BOM[1]) {
      fseek(fd, 0, SEEK_SET);
      slength += 2;
    }
  }

  // Clear BOM
  memset(pBuffer, 0, 2);
  // Read in text
  fread(pBuffer, sizeof(BYTE), slength - 2, fd);

  sNewNotes = reinterpret_cast<const LPCWSTR>(pBuffer);

  delete [] pBuffer;

  // Close file before invoking editor
  fclose(fd);

  if ((!M_pcore()->IsReadOnly() && M_protected() == 0) &&
      sNewNotes.length() > MAXTEXTCHARS) {
    sNewNotes = sNewNotes.substr(0, MAXTEXTCHARS);

    CGeneralMsgBox gmb;
    CString cs_text, cs_title(MAKEINTRESOURCE(IDS_WARNINGTEXTLENGTH));
    cs_text.Format(IDS_TRUNCATETEXT, MAXTEXTCHARS);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }

  // Set real notes field, and
  // we are still displaying the old text, so replace that too
  M_notes() = sNewNotes.c_str();

  UpdateData(FALSE);
  m_ex_notes.Invalidate();

  // Delete temporary file
  _wremove(m_szTempName);
  SecureZeroMemory(m_szTempName, sizeof(m_szTempName));

  // Restore Sheet buttons
  GetParent()->GetDlgItem(IDOK)->EnableWindow(m_bOKSave == 0 ? TRUE : FALSE);
  GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(m_bOKCancel == 0 ? TRUE : FALSE);

  if (sOldNotes != M_notes()) {
    m_ae_psh->SetChanged(true);
    m_ae_psh->SetNotesChanged(true); // Needed if Notes field is long and will be truncated
  }
  return 0L;

error_exit:
  // Get out
  UpdateData(FALSE);
  m_ex_notes.Invalidate();

  // Restore Sheet buttons
  GetParent()->GetDlgItem(IDOK)->EnableWindow(m_bOKSave == 0 ? TRUE : FALSE);
  GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(m_bOKCancel == 0 ? TRUE : FALSE);
  return 0L;
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

  bool bPswdIsInAliasFormat = M_pcore()->ParseBaseEntryPWD(password, pl);

  // Copy data back before possibly returning
  ibasedata = pl.ibasedata;
  base_uuid = pl.base_uuid;
  if (!bPswdIsInAliasFormat)
    return false;

  // if we ever return 'false', this routine will have issued a message to the user
  b_msg_issued = true;

  if (bIsEdit && 
    (pl.csPwdGroup == group && pl.csPwdTitle == title && pl.csPwdUser == user)) {
    // In Edit, check user isn't changing entry to point to itself (circular/self reference)
    // Can't happen during Add as already checked entry does not exist so if accepted the
    // password would be treated as an unusual "normal" password
    gmb.AfxMessageBox(IDS_ALIASCANTREFERTOITSELF, MB_OK);
    return false;
  }

  // ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and
  // is only valid if n = -1 or -2.

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

  if (pl.ibasedata > 0) { // base entry exists
    CGeneralMsgBox gmbx;
    if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
      // An alias can only point to a normal entry or an alias base entry
      CString cs_msg;
      cs_msg.Format(IDS_BASEISALIAS, 
                    pl.csPwdGroup.c_str(),
                    pl.csPwdTitle.c_str(),
                    pl.csPwdUser.c_str());
      gmbx.AfxMessageBox(cs_msg, NULL, MB_OK);
      return false;
    } else { // <= 0, no base entry. What's TargetType in this case??
      if (pl.TargetType != CItemData::ET_NORMAL && pl.TargetType != CItemData::ET_ALIASBASE) {
        // An alias can only point to a normal entry or an alias base entry
        CString cs_msg;
        cs_msg.Format(IDS_ABASEINVALID, 
                      pl.csPwdGroup.c_str(),
                      pl.csPwdTitle.c_str(), 
                      pl.csPwdUser.c_str());
        gmbx.AfxMessageBox(cs_msg, NULL, MB_OK);
        return false;
      } else {
        return true;
      }
    }
  }
  return true; // All OK
}

void CAddEdit_Basic::SetGroupComboBoxWidth()
{
  // Find the longest string in the combo box.
  CString str;
  CSize sz;
  int dx = 0;
  TEXTMETRIC tm;
  CDC *pDC = m_ex_group.GetDC();
  CFont *pFont = m_ex_group.GetFont();

  // Select the listbox font, save the old font
  CFont *pOldFont = pDC->SelectObject(pFont);

  // Get the text metrics for avg char width
  pDC->GetTextMetrics(&tm);

  for (int i = 0; i < m_ex_group.GetCount(); i++) {
    m_ex_group.GetLBText(i, str);
    sz = pDC->GetTextExtent(str);

    // Add the avg width to prevent clipping
    sz.cx += tm.tmAveCharWidth;

    if (sz.cx > dx)
      dx = sz.cx;
  }

  // Select the old font back into the DC
  pDC->SelectObject(pOldFont);
  m_ex_group.ReleaseDC(pDC);

  // Adjust the width for the vertical scroll bar and the left and right border.
  dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2 * ::GetSystemMetrics(SM_CXEDGE);

  // Set the width of the list box so that every item is completely visible.
  m_ex_group.SetDroppedWidth(dx);
}

void CAddEdit_Basic::OnCopyPassword()
{
  UpdateData(TRUE);

  GetMainDlg()->SetClipboardData(m_password);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);
}

void CAddEdit_Basic::SetUpDependentsCombo()
{
  // Get Add/Edit font
  CFont *pFont = Fonts::GetInstance()->GetAddEditFont();
  m_cmbDependents.SetFont(pFont);

  CString cs_type;
  cs_type.LoadString(M_original_entrytype() == CItemData::ET_ALIASBASE ?
      IDS_LIST_OF_ALIASES : IDS_LIST_OF_SHORTCUTS);

  m_stc_dependent.SetWindowText(cs_type);

  for (size_t i = 0; i < M_vsxdependents().size(); i++) {
    m_cmbDependents.AddString(M_vsxdependents()[i].c_str());
  }

  SetComboBoxWidth();
}

void CAddEdit_Basic::SetComboBoxWidth()
{
  // Find the longest string in the combo box.
  CString str;
  CSize sz;
  int dx = 0;
  TEXTMETRIC tm;
  CDC *pDC = m_cmbDependents.GetDC();
  CFont *pFont = m_cmbDependents.GetFont();

  // Select the listbox font, save the old font
  CFont *pOldFont = pDC->SelectObject(pFont);

  // Get the text metrics for avg char width
  pDC->GetTextMetrics(&tm);

  for (int i = 0; i < m_cmbDependents.GetCount(); i++) {
    m_cmbDependents.GetLBText(i, str);
    sz = pDC->GetTextExtent(str);

    // Add the avg width to prevent clipping
    sz.cx += tm.tmAveCharWidth;

    if (sz.cx > dx)
      dx = sz.cx;
  }

  // Select the old font back into the DC
  pDC->SelectObject(pOldFont);
  m_cmbDependents.ReleaseDC(pDC);

  // Adjust the width for the vertical scroll bar and the left and right border.
  dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2 * ::GetSystemMetrics(SM_CXEDGE);

  // If the width of the list box is too small, adjust it so that every
  // item is completely visible.
  m_cmbDependents.SetDroppedWidth(dx);
}
