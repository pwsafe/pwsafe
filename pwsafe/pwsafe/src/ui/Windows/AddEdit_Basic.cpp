/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "DboxMain.h"
#include "PwFont.h"

#include "AddEdit_Basic.h"
#include "AddEdit_PropertySheet.h"

#include "corelib/PWCharPool.h"
#include "corelib/PWSprefs.h"
#include "corelib/PWSAuxParse.h"

#include <shlwapi.h>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

CSecString CAddEdit_Basic::HIDDEN_NOTES;

CString CAddEdit_Basic::CS_SHOW;
CString CAddEdit_Basic::CS_HIDE;

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic property page

IMPLEMENT_DYNAMIC(CAddEdit_Basic, CAddEdit_PropertyPage)

CAddEdit_Basic::CAddEdit_Basic(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, CAddEdit_Basic::IDD, pAEMD),
  m_pToolTipCtrl(NULL)
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
  m_notes = m_notesww = M_realnotes();

  // Set up right-click Notes context menu additions
  std::vector<st_context_menu> vmenu_items(3);

  st_context_menu st_cm;
  stringT cs_menu_string;

  LoadAString(cs_menu_string, IDS_WORD_WRAP);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = WM_EDIT_WORDWRAP;
  st_cm.flags = m_bWordWrap ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items[0] = st_cm;

  LoadAString(cs_menu_string, IDS_SHOW_NOTES);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = WM_EDIT_SHOWNOTES;
  st_cm.flags = m_isNotesHidden ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items[1] = st_cm;

  LoadAString(cs_menu_string, IDS_EDITEXTERNALLY);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = WM_CALL_EXTERNAL_EDITOR;
  st_cm.flags = 0;
  vmenu_items[2] = st_cm;

  m_pex_notes = new CEditExtn(vmenu_items);
  m_pex_notesww = new CEditExtn(vmenu_items);
}

CAddEdit_Basic::~CAddEdit_Basic()
{
  delete m_pex_notes;
  delete m_pex_notesww;
  delete m_pToolTipCtrl;
}

void CAddEdit_Basic::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_Basic)
  m_ex_password.DoDDX(pDX, m_password);
  m_ex_password2.DoDDX(pDX, m_password2);

  DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
  DDX_Text(pDX, IDC_NOTESWW, (CString&)m_notesww);

  DDX_CBString(pDX, IDC_GROUP, (CString&)M_group());
  DDX_Text(pDX, IDC_TITLE, (CString&)M_title());
  DDX_Text(pDX, IDC_USERNAME, (CString&)M_username());
  DDX_Text(pDX, IDC_URL, (CString&)M_URL());

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
  DDX_Control(pDX, IDC_NOTES, *m_pex_notes);
  DDX_Control(pDX, IDC_NOTESWW, *m_pex_notesww);
  DDX_Control(pDX, IDC_URL, m_ex_URL);
  DDX_Control(pDX, IDC_VIEWDEPENDENTS, m_ViewDependentsBtn);

  if (M_uicaller() != IDS_ADDENTRY) {
    DDX_Control(pDX, IDC_STATIC_GROUP, m_stc_group);
    DDX_Control(pDX, IDC_STATIC_TITLE, m_stc_title);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_stc_username);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stc_password);
    DDX_Control(pDX, IDC_STATIC_NOTES, m_stc_notes);
    DDX_Control(pDX, IDC_STATIC_URL, m_stc_URL);
  }
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Basic, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Basic)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowPassword)
  ON_BN_CLICKED(IDC_RANDOM, OnRandom)
  ON_BN_CLICKED(IDC_LAUNCH, OnLaunch)
  ON_BN_CLICKED(IDC_VIEWDEPENDENTS, OnViewDependents)
  ON_EN_CHANGE(IDC_URL, OnChangeURL)
  ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetFocus)
  ON_EN_SETFOCUS(IDC_NOTES, OnENSetFocusNotes)
  ON_EN_SETFOCUS(IDC_NOTESWW, OnENSetFocusNotes)
  ON_EN_KILLFOCUS(IDC_NOTES, OnENKillFocusNotes)
  ON_EN_KILLFOCUS(IDC_NOTESWW, OnENKillFocusNotes)

  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_GROUP, IDC_STATIC_URL, OnSTCExClicked)
  ON_MESSAGE(WM_CALL_EXTERNAL_EDITOR, OnCallExternalEditor)
  ON_MESSAGE(WM_EXTERNAL_EDITOR_ENDED, OnExternalEditorEnded)
  ON_MESSAGE(WM_EDIT_WORDWRAP, OnWordWrap)
  ON_MESSAGE(WM_EDIT_SHOWNOTES, OnShowNotes)
  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Basic message handlers

BOOL CAddEdit_Basic::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx (0, WS_EX_CONTROLPARENT);

  ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
  ApplyPasswordFont(GetDlgItem(IDC_PASSWORD2));

  if (M_uicaller() != IDS_ADDENTRY) {
    m_pToolTipCtrl = new CToolTipCtrl;
    if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
      TRACE("Unable To create CAddEdit_Basic Dialog ToolTip\n");
    } else {
      EnableToolTips();
      // Delay initial show & reshow
      int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
      m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, iTime);
      m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW, iTime);
      m_pToolTipCtrl->SetMaxTipWidth(300);

      CString cs_ToolTip;
      cs_ToolTip.LoadString(IDS_CLICKTOCOPY);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_GROUP), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_TITLE), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_USERNAME), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_PASSWORD), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_NOTES), cs_ToolTip);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_URL), cs_ToolTip);
      cs_ToolTip.LoadString(IDS_CLICKTOGOPLUS);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDC_LAUNCH), cs_ToolTip);

      m_pToolTipCtrl->Activate(TRUE);
    }

    m_stc_group.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_title.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_username.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_password.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_notes.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
    m_stc_URL.SetHighlight(true, CAddEdit_PropertyPage::crefWhite);
  }

  m_ex_group.ChangeColour();
  GetDlgItem(IDC_LAUNCH)->EnableWindow(M_URL().IsEmpty() ? FALSE : TRUE);

  if (M_uicaller() == IDS_VIEWENTRY) {
    // Change 'OK' to 'Close' and disable 'Cancel'
    CancelToClose();

    // Disable Edit control in the Group Combo [always ID=1001]
    GetDlgItem(IDC_GROUP)->GetDlgItem(1001)->SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable normal Edit controls
    GetDlgItem(IDC_TITLE)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_USERNAME)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_PASSWORD)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_PASSWORD2)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_NOTES)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_NOTESWW)->SendMessage(EM_SETREADONLY, TRUE, 0);
    GetDlgItem(IDC_URL)->SendMessage(EM_SETREADONLY, TRUE, 0);

    // Disable Button
    GetDlgItem(IDC_RANDOM)->EnableWindow(FALSE);
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

  m_pex_notes->EnableWindow(m_bWordWrap ? FALSE : TRUE);
  m_pex_notes->ShowWindow(m_bWordWrap ? SW_HIDE : SW_SHOW);

  m_pex_notesww->EnableWindow(m_bWordWrap ? TRUE : FALSE);
  m_pex_notesww->ShowWindow(m_bWordWrap ? SW_SHOW : SW_HIDE);

  // Populate the combo box
  if (m_ex_group.GetCount() == 0) {
      std::vector<stringT> aryGroups;
      M_pcore()->GetUniqueGroups(aryGroups);
      for (size_t igrp = 0; igrp < aryGroups.size(); igrp++) {
        m_ex_group.AddString(aryGroups[igrp].c_str());
      }
  }

  int num_unkwn(0);
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
    M_realpassword() = M_oldRealPassword() = m_password = M_base();
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);

    cs_text.Format(IDS_ISANALIAS, M_base());
    GetDlgItem(IDC_STATIC_ISANALIAS)->SetWindowText(cs_text);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_SHOW);
  } else if (M_original_entrytype() == CItemData::ET_NORMAL) {
    // Normal - do none of the above
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_HIDE);
  }

  UpdateData(FALSE);

  return TRUE;
}

void CAddEdit_Basic::OnHelp()
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL,
                FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/entering_pwd.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

HBRUSH CAddEdit_Basic::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CAddEdit_Basic::OnKillActive()
{
  CAddEdit_PropertyPage::OnKillActive();

  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return TRUE;
}

LRESULT CAddEdit_Basic::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      switch (M_uicaller()) {
        case IDS_EDITENTRY:
          if (M_group()        != M_pci()->GetGroup() ||
              M_title()        != M_pci()->GetTitle() ||
              M_username()     != M_pci()->GetUser() ||
              M_realnotes()    != M_pci()->GetNotes() ||
              M_URL()          != M_pci()->GetURL() ||
              M_realpassword() != M_oldRealPassword())
            return 1L;
          break;
        case IDS_ADDENTRY:
          if (!M_group().IsEmpty() ||
              !M_title().IsEmpty() ||
              !M_username().IsEmpty() ||
              !M_realpassword().IsEmpty() ||
              !M_realnotes().IsEmpty() ||
              !M_URL().IsEmpty())
            return 1L;
          break;
      }
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL CAddEdit_Basic::PreTranslateMessage(MSG* pMsg)
{
  // Do tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  // if user hit Ctrl+A in Notes control, then SelectAllNotes
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'A' &&
      (GetKeyState(VK_CONTROL) & 0x8000) &&
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

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_Basic::OnApply()
{
  CWnd *pFocus(NULL);
  ItemListIter listindex;
  bool brc, b_msg_issued;

  UpdateData(TRUE);

  M_group().EmptyIfOnlyWhiteSpace();
  M_title().EmptyIfOnlyWhiteSpace();
  M_username().EmptyIfOnlyWhiteSpace();
  M_URL().EmptyIfOnlyWhiteSpace();

  m_notes.EmptyIfOnlyWhiteSpace();
  m_notesww.EmptyIfOnlyWhiteSpace();

  if (m_password.IsOnlyWhiteSpace()) {
    m_password.Empty();
    if (m_isPWHidden)
      m_password2.Empty();
  }

  if (!m_isPWHidden || m_password != HIDDEN_PASSWORD)
    M_realpassword() = m_password;

  if (!m_isNotesHidden)
    M_realnotes() = m_bWordWrap ? m_notesww : m_notes;

  UpdateData(FALSE);

  //Check that data is valid
  if (M_title().IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    pFocus = &m_ex_title;
    goto error;
  }

  if (M_realpassword().IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVEPASSWORD);
    pFocus = &m_ex_password;
    goto error;
  }

  if (!M_group().IsEmpty() && M_group()[0] == '.') {
    AfxMessageBox(IDS_DOTINVALID);
    pFocus = &m_ex_group;
    goto error;
  }

  if (m_isPWHidden && (m_password.Compare(m_password2) != 0)) {
    AfxMessageBox(IDS_PASSWORDSNOTMATCH);
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

      AfxMessageBox(temp);
      pFocus = &m_ex_title;
      goto error;
    }
  } else {
    // Edit entry
    if (listindex != M_pDbx()->End()) {
      const CItemData &listItem = M_pDbx()->GetEntryAt(listindex);
      uuid_array_t list_uuid, elem_uuid;
      listItem.GetUUID(list_uuid);
      M_pci()->GetUUID(elem_uuid);
      bool notSame = (::memcmp(list_uuid, elem_uuid, sizeof(uuid_array_t)) != 0);
      if (notSame) {
        CSecString temp;
        temp.Format(IDS_ENTRYEXISTS, M_group(), M_title(), M_username());
        AfxMessageBox(temp);
        pFocus = &m_ex_title;
        goto error;
      }
    }
  }

  brc = M_pDbx()->CheckNewPassword(M_group(), M_title(),
                                   M_username(), M_realpassword(),
                                   M_uicaller() != IDS_ADDENTRY,
                                   CItemData::ET_ALIAS,
                                   M_base_uuid(), M_ibasedata(),
                                   b_msg_issued);

  if (!brc && M_ibasedata() != 0) {
    if (!b_msg_issued)
      AfxMessageBox(IDS_MUSTHAVETARGET, MB_OK);

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

void CAddEdit_Basic::OnPasskeySetFocus()
{
  m_ex_password.SetSel(0, -1);
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

  m_pex_notes->UpdateState(WM_EDIT_SHOWNOTES, m_isNotesHidden ? FALSE : TRUE);
  m_pex_notesww->UpdateState(WM_EDIT_SHOWNOTES, m_isNotesHidden ? FALSE : TRUE);
  return 0L;
}

void CAddEdit_Basic::ShowNotes()
{
  m_isNotesHidden = false;
  m_notes = M_realnotes();
  m_notesww = M_realnotes();

  ((CEdit *)GetDlgItem(IDC_NOTES))->Invalidate();
  ((CEdit *)GetDlgItem(IDC_NOTESWW))->Invalidate();
 }

void CAddEdit_Basic::HideNotes()
{
  m_isNotesHidden = true;
  if (m_bWordWrap) {
    if (m_notesww != HIDDEN_NOTES) {
      M_realnotes() = m_notesww;
    }
  } else {
    if (m_notes != HIDDEN_NOTES) {
      M_realnotes() = m_notes;
    }
  }

  m_notes = HIDDEN_NOTES;
  m_notesww = HIDDEN_NOTES;

  ((CEdit *)GetDlgItem(IDC_NOTES))->Invalidate();
  ((CEdit *)GetDlgItem(IDC_NOTESWW))->Invalidate();
}

void CAddEdit_Basic::OnRandom()
{
  UpdateData(TRUE);

  StringX passwd;
  M_pDbx()->MakeRandomPassword(passwd, M_pwp());
  M_realpassword() = m_password = passwd.c_str();
  if (m_isPWHidden) {
    m_password2 = m_password;
  }
  UpdateData(FALSE);
}

void CAddEdit_Basic::OnChangeURL()
{
  UpdateData(TRUE);
  GetDlgItem(IDC_LAUNCH)->EnableWindow(M_URL().IsEmpty() ? FALSE : TRUE);
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
      m_stc_URL.FlashBkgnd(CAddEdit_PropertyPage::crefGreen);
      cs_data = StringX(M_URL());
      iaction = CItemData::URL;
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
  ((CEdit *)GetDlgItem(m_bWordWrap ? IDC_NOTESWW : IDC_NOTES))->
                       SetSel(0, -1, TRUE);
}

LRESULT CAddEdit_Basic::OnWordWrap(WPARAM, LPARAM)
{
  // Get value of notes from dialog.
  UpdateData(TRUE);
  if (m_bWordWrap)
    m_notes = m_notesww;
  else
    m_notesww = m_notes;

  m_bWordWrap = !m_bWordWrap;
  // Update dalog
  UpdateData(FALSE);

  m_pex_notes->EnableWindow(m_bWordWrap ? FALSE : TRUE);
  m_pex_notes->ShowWindow(m_bWordWrap ? SW_HIDE : SW_SHOW);
  m_pex_notes->Invalidate();
  m_pex_notes->UpdateState(WM_EDIT_WORDWRAP, m_bWordWrap);

  m_pex_notesww->EnableWindow(m_bWordWrap ? TRUE : FALSE);
  m_pex_notesww->ShowWindow(m_bWordWrap ? SW_SHOW : SW_HIDE);
  m_pex_notesww->Invalidate();
  m_pex_notesww->UpdateState(WM_EDIT_WORDWRAP, m_bWordWrap);

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
    M_realnotes() = m_bWordWrap ? m_notesww : m_notes;

  UpdateData(FALSE);
}

void CAddEdit_Basic::OnLaunch()
{
  UpdateData(TRUE);
  StringX sx_url = StringX(M_URL());
  StringX sx_autotype = PWSAuxParse::GetAutoTypeString(M_autotype(),
                                                       M_group(),
                                                       M_title(),
                                                       M_username(),
                                                       M_realpassword(),
                                                       M_realnotes());
  int iaction = CItemData::URL;
  const bool bDoAutoType = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  M_pDbx()->LaunchBrowser(sx_url.c_str(), sx_autotype, bDoAutoType);
  M_pDbx()->UpdateLastClipboardAction(iaction);

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

LRESULT CAddEdit_Basic::OnCallExternalEditor(WPARAM, LPARAM)
{
  // Warn the user about sensitive data lying around
  int rc = AfxMessageBox(IDS_EXTERNAL_EDITOR_WARNING,
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

  TCHAR szExecName[MAX_PATH + 1];
  TCHAR lpPathBuffer[4096];
  DWORD dwBufSize(4096);

  // Get the temp path
  GetTempPath(dwBufSize,          // length of the buffer
              lpPathBuffer);      // buffer for path

  // Create a temporary file.
  GetTempFileName(lpPathBuffer,          // directory for temp files
                  _T("NTE"),             // temp file name prefix
                  0,                     // create unique name
                  self->m_szTempName);   // buffer for name

  // Open it and put the Notes field in it
  ofstreamT ofs(self->m_szTempName);
  if (ofs.bad())
    return 16;

  ofs << LPCTSTR(self->M_realnotes()) << std::endl;
  ofs.flush();
  ofs.close();

  // Find out the users default editor for "txt" files
  DWORD dwSize(MAX_PATH);
  HRESULT stat = ::AssocQueryString(0, ASSOCSTR_EXECUTABLE, _T(".txt"), _T("Open"),
                                    szExecName, &dwSize);
  if (int(stat) != S_OK) {
#ifdef _DEBUG
    AfxMessageBox(_T("oops"));
#endif
    return 16;
  }

  // Create an Edit process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  DWORD dwCreationFlags(0);
#ifdef _UNICODE
  dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;
#endif

  CString cs_CommandLine;

  // Make the command line = "<program>" "file"
  cs_CommandLine.Format(_T("\"%s\" \"%s\""), szExecName, self->m_szTempName);
  int ilen = cs_CommandLine.GetLength();
  LPTSTR pszCommandLine = cs_CommandLine.GetBuffer(ilen);

  if (!CreateProcess(NULL, pszCommandLine, NULL, NULL, FALSE, dwCreationFlags,
                     NULL, lpPathBuffer, &si, &pi)) {
    TRACE("CreateProcess failed (%d).\n", GetLastError());
    // Delete temporary file
    _tremove(self->m_szTempName);
    memset(self->m_szTempName, 0, sizeof(self->m_szTempName));
    return 0;
  }

  TRACE(_T("%d\n"), sizeof(self->m_szTempName));
  WaitForInputIdle(pi.hProcess, INFINITE);

  // Wait until child process exits.
  WaitForSingleObject(pi.hProcess, INFINITE);

  // Close process and thread handles.
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  cs_CommandLine.ReleaseBuffer();

  self->PostMessage(WM_EXTERNAL_EDITOR_ENDED, 0, 0);
  return 0;
}

LRESULT CAddEdit_Basic::OnExternalEditorEnded(WPARAM, LPARAM)
{
  GetDlgItem(IDC_NOTES)->EnableWindow(TRUE);

  // Now get what the user saved in this file and put it back into Notes field
  ifstreamT ifs(m_szTempName);
  if (ifs.bad())
    return 16;

  M_realnotes().Empty();
  stringT linebuf, note;

  // Get first line
  getline(ifs, note, TCHAR('\n'));

  // Now get the rest (if any)
  while (!ifs.eof()) {
    getline(ifs, linebuf, TCHAR('\n'));
    note += _T("\r\n");
    note += linebuf;
  }

  ifs.close();

  // Set real notes field, and
  // we are still displaying the old text, so replace that too
  M_realnotes() = m_notes = m_notesww = note.c_str();

  UpdateData(FALSE);
  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();
  ((CEdit*)GetDlgItem(IDC_NOTESWW))->Invalidate();

  // Delete temporary file
  _tremove(m_szTempName);
  memset(m_szTempName, 0, sizeof(m_szTempName));

  // Restore Sheet buttons
  GetParent()->GetDlgItem(IDOK)->EnableWindow(m_bOKSave == 0 ? TRUE : FALSE);
  GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(m_bOKCancel == 0 ? TRUE : FALSE);

  return 0;
}

void CAddEdit_Basic::OnViewDependents()
{
  CString cs_msg, cs_type;

  if (M_original_entrytype() == CItemData::ET_ALIASBASE)
    cs_type.LoadString(M_num_dependents() == 1 ? IDS_ALIAS : IDS_ALIASES);
  else
    cs_type.LoadString(M_num_dependents() == 1 ? IDS_SHORTCUT : IDS_SHORTCUTS);

  cs_msg.Format(IDS_VIEWDEPENDENTS, M_num_dependents(), cs_type, M_dependents());
  MessageBox(cs_msg, AfxGetAppName(), MB_OK);
}
