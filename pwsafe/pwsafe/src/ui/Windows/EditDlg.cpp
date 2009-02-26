/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file EditDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "EditDlg.h"
#include "PwFont.h"
#include "OptionsPasswordPolicy.h"
#include "corelib/PWCharPool.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"
#include "corelib/ItemData.h"
#include "os/typedefs.h"
#include "ExpDTDlg.h"
#include "PWHistDlg.h"
#include "ControlExtns.h"
#include "ExtThread.h"

#include <shlwapi.h>
#include <fstream>
#include <bitset>

using namespace std;

#if defined(POCKET_PC)
#include "pocketpc/PocketPC.h"
#include "editdlg.h"
#endif

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
CSecString CEditDlg::HIDDEN_NOTES;
CString CEditDlg::CS_ON;
CString CEditDlg::CS_OFF;
CString CEditDlg::CS_SHOW;
CString CEditDlg::CS_HIDE;

static const COLORREF crefGreen = (RGB(222, 255, 222));
static const COLORREF crefPink  = (RGB(255, 222, 222));
static const COLORREF crefWhite = (RGB(255, 255, 255));

CEditDlg::CEditDlg(CItemData *ci, CWnd* pParent)
  : CPWDialog(CEditDlg::IDD, pParent),
  m_ci(ci), m_Edit_IsReadOnly(false),
  m_tttXTime(time_t(0)), m_tttCPMTime(time_t(0)),
  m_locXTime(_T("")), m_oldlocXTime(_T("")), m_XTimeInt(0),
  m_original_entrytype(CItemData::ET_NORMAL), m_ToolTipCtrl(NULL),
  m_bWordWrap(FALSE), m_bShowNotes(FALSE)
{
  ASSERT(ci != NULL);
  m_pDbx = static_cast<DboxMain *>(pParent);

  if (HIDDEN_NOTES.IsEmpty()) { // one-time initializations
    HIDDEN_NOTES.LoadString(IDS_HIDDENNOTES);
    CS_ON.LoadString(IDS_ON);
    CS_OFF.LoadString(IDS_OFF);
#if defined(POCKET_PC)
    CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT1);
    CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT1);
#else
    CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT2);
    CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT2);
#endif
  }

  m_OverridePolicy = m_ci->GetPWPolicy().empty() ? FALSE : TRUE;
  m_ci->GetPWPolicy(m_pwp);
  size_t num_err;
  BOOL HasHistory = CreatePWHistoryList(ci->GetPWHistory(), m_MaxPWHistory,
                                        num_err, m_PWHistList,
                                        TMC_EXPORT_IMPORT) ? TRUE : FALSE;
  m_NumPWHistory = m_PWHistList.size();
  m_SavePWHistory = HasHistory;

  m_group = m_ci->GetGroup();
  m_title = m_ci->GetTitle();
  m_username = m_ci->GetUser();
  m_password = m_password2 = HIDDEN_PASSWORD;
  m_realpassword = m_oldRealPassword = m_ci->GetPassword();
  m_URL = m_ci->GetURL();
  m_autotype = m_ci->GetAutoType();
  m_notes = HIDDEN_NOTES;
  m_notesww = HIDDEN_NOTES;
  m_realnotes = m_ci->GetNotes();
  m_PWHistory = m_ci->GetPWHistory();
  m_locCTime = m_ci->GetCTimeL();
  m_locATime = m_ci->GetATimeL();
  m_locRMTime = m_ci->GetRMTimeL();
  m_locPMTime = m_ci->GetPMTimeL();
  m_executestring = m_ci->GetExecuteString();

  if (!m_locPMTime.IsEmpty())
    m_ci->GetPMTime(m_tttCPMTime);
  if ((long)m_tttCPMTime == 0L) // if never changed - try creation date
    m_ci->GetCTime(m_tttCPMTime);

  m_locXTime = m_ci->GetXTimeL();
  if (m_locXTime.IsEmpty()) {
    m_locXTime.LoadString(IDS_NEVER);
    m_tttXTime = 0;
  } else
    m_ci->GetXTime(m_tttXTime);

  m_oldlocXTime = m_locXTime;
  m_ci->GetXTimeInt(m_XTimeInt);
  m_oldXTimeInt = m_XTimeInt;

  PWSprefs *prefs = PWSprefs::GetInstance();
  m_bShowNotes = prefs->GetPref(PWSprefs::ShowNotesDefault) ? TRUE : FALSE;
  m_bWordWrap = prefs->GetPref(PWSprefs::NotesWordWrap) ? TRUE : FALSE;

  std::vector<st_context_menu> vmenu_items(3);

  st_context_menu st_cm;
  stringT cs_menu_string;

  LoadAString(cs_menu_string, IDS_WORD_WRAP);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = WM_EDIT_WORDWRAP;
  st_cm.flags = m_bWordWrap == TRUE ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items[0] = st_cm;

  LoadAString(cs_menu_string, IDS_SHOW_NOTES);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = WM_EDIT_SHOWNOTES;
  st_cm.flags = m_bShowNotes == TRUE ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items[1] = st_cm;

  LoadAString(cs_menu_string, IDS_EDITEXTERNALLY);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = WM_CALL_EXTERNAL_EDITOR;
  st_cm.flags = 0;
  vmenu_items[2] = st_cm;

  m_pex_notes = new CEditExtn(vmenu_items);
  m_pex_notesww = new CEditExtn(vmenu_items);

  m_num_dependents = 0;
  m_dependents = _T("");
  m_base = _T("");
}

CEditDlg::~CEditDlg()
{
  delete m_pex_notes;
  delete m_pex_notesww;
  delete m_ToolTipCtrl;
}

void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
   CPWDialog::DoDataExchange(pDX);
   m_ex_password.DoDDX(pDX, m_password);
   m_ex_password2.DoDDX(pDX, m_password2);
   DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
   DDX_Text(pDX, IDC_NOTESWW, (CString&)m_notesww);
   DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
   DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
   DDX_Text(pDX, IDC_URL, (CString&)m_URL);
   DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);
   DDX_Text(pDX, IDC_EXECUTE, (CString&)m_executestring);
   DDX_Text(pDX, IDC_CTIME, (CString&)m_locCTime);
   DDX_Text(pDX, IDC_PMTIME, (CString&)m_locPMTime);
   DDX_Text(pDX, IDC_ATIME, (CString&)m_locATime);
   DDX_Text(pDX, IDC_XTIME, (CString&)m_locXTime);
   DDX_Text(pDX, IDC_RMTIME, (CString&)m_locRMTime);

   DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
   DDX_Control(pDX, IDC_MORE, m_MoreLessBtn);
   DDX_Control(pDX, IDC_VIEWDEPENDENTS, m_ViewDependentsBtn);

   DDX_Control(pDX, IDC_GROUP, m_ex_group);
   DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
   DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
   DDX_Control(pDX, IDC_NOTES, *m_pex_notes);
   DDX_Control(pDX, IDC_NOTESWW, *m_pex_notesww);
   DDX_Control(pDX, IDC_USERNAME, m_ex_username);
   DDX_Control(pDX, IDC_TITLE, m_ex_title);
   DDX_Control(pDX, IDC_URL, m_ex_URL);
   DDX_Control(pDX, IDC_AUTOTYPE, m_ex_autotype);
   DDX_Control(pDX, IDC_EXECUTE, m_ex_executestring);
   DDX_Check(pDX, IDC_OVERRIDE_POLICY, m_OverridePolicy);

   DDX_Control(pDX, IDC_STATIC_GROUP, m_stc_group);
   DDX_Control(pDX, IDC_STATIC_TITLE, m_stc_title);
   DDX_Control(pDX, IDC_STATIC_USERNAME, m_stc_username);
   DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stc_password);
   DDX_Control(pDX, IDC_STATIC_NOTES, m_stc_notes);
   DDX_Control(pDX, IDC_STATIC_URL, m_stc_URL);
   DDX_Control(pDX, IDC_STATIC_AUTO, m_stc_autotype);   
   DDX_Control(pDX, IDC_STATIC_EXECUTE, m_stc_executestring);  
}

BEGIN_MESSAGE_MAP(CEditDlg, CPWDialog)
  ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowPassword)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_RANDOM, OnRandom)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
  ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
  ON_BN_CLICKED(IDC_VIEWDEPENDENTS, OnBnClickViewDependents)
  ON_BN_CLICKED(IDC_XTIME_CLEAR, OnBnClickedClearXTime)
  ON_BN_CLICKED(IDC_XTIME_SET, OnBnClickedSetXTime)
  ON_BN_CLICKED(IDC_PWHIST, OnBnClickedPwhist)
  ON_BN_CLICKED(IDC_OVERRIDE_POLICY, OnBnClickedOverridePolicy)
  ON_BN_CLICKED(IDC_LAUNCH, OnBnClickedLaunch)
  ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetfocus)
  ON_EN_KILLFOCUS(IDC_PASSWORD, OnPasskeyKillfocus)
  ON_EN_SETFOCUS(IDC_NOTES, OnEnSetfocusNotes)
  ON_EN_SETFOCUS(IDC_NOTESWW, OnEnSetfocusNotes)
  ON_EN_KILLFOCUS(IDC_NOTES, OnEnKillfocusNotes)
  ON_EN_KILLFOCUS(IDC_NOTESWW, OnEnKillfocusNotes)
  ON_EN_CHANGE(IDC_URL, OnEnChangeUrl)
  ON_CONTROL_RANGE(STN_CLICKED, IDC_STATIC_GROUP, IDC_STATIC_EXECUTE, OnStcClicked)
  ON_MESSAGE(WM_CALL_EXTERNAL_EDITOR, OnCallExternalEditor)
  ON_MESSAGE(WM_EXTERNAL_EDITOR_ENDED, OnExternalEditorEnded)
  ON_MESSAGE(WM_EDIT_WORDWRAP, OnWordWrap)
  ON_MESSAGE(WM_EDIT_SHOWNOTES, OnShowNotes)
#if defined(POCKET_PC)
  ON_WM_SHOWWINDOW()
#endif
END_MESSAGE_MAP()

void CEditDlg::OnShowPassword() 
{
  UpdateData(TRUE);

  if (m_isPwHidden) {
    ShowPassword();
  } else {
    m_realpassword = m_password; // save visible password
    HidePassword();
  }
  UpdateData(FALSE);
}

void CEditDlg::OnCancel() 
{
  if (!m_Edit_IsReadOnly &&
      (m_group        != m_ci->GetGroup() ||
       m_title        != m_ci->GetTitle() ||
       m_username     != m_ci->GetUser() ||
       m_realnotes    != m_ci->GetNotes() ||
       m_URL          != m_ci->GetURL() ||
       m_autotype     != m_ci->GetAutoType() ||
       m_executestring != m_ci->GetExecuteString() ||
       m_PWHistory    != m_ci->GetPWHistory() ||
       m_locXTime     != m_oldlocXTime ||
       m_XTimeInt     != m_oldXTimeInt ||
       m_realpassword != m_oldRealPassword)) {
    int rc = AfxMessageBox(IDS_AREYOUSURE, 
                           MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2);
    if (rc != IDYES)
      goto dont_cancel;
  }
  CPWDialog::OnCancel();
  return;

dont_cancel:
  return;
}

void CEditDlg::OnOK() 
{
  ItemListIter listindex;

  UpdateData(TRUE);
  m_group.EmptyIfOnlyWhiteSpace();
  m_title.EmptyIfOnlyWhiteSpace();
  m_username.EmptyIfOnlyWhiteSpace();
  if (m_password.IsOnlyWhiteSpace()) {
    m_password.Empty();
    if (m_isPwHidden)
      m_password2.Empty();
  }
  m_realnotes.EmptyIfOnlyWhiteSpace();
  m_URL.EmptyIfOnlyWhiteSpace();
  m_autotype.EmptyIfOnlyWhiteSpace();

  if (!m_isPwHidden || m_password != HIDDEN_PASSWORD)
    m_realpassword = m_password;

  if (!m_isNotesHidden)
    m_realnotes = m_bWordWrap == TRUE ? m_notesww : m_notes;

  bool bIsModified, bIsPswdModified;
  bIsModified = (m_group != m_ci->GetGroup() ||
                 m_title != m_ci->GetTitle() ||
                 m_username != m_ci->GetUser() ||
                 m_realnotes != m_ci->GetNotes() ||
                 m_URL != m_ci->GetURL() ||
                 m_autotype != m_ci->GetAutoType() ||
                 m_executestring != m_ci->GetExecuteString() ||
                 m_PWHistory != m_ci->GetPWHistory() ||
                 m_locXTime != m_oldlocXTime ||
                 m_XTimeInt != m_oldXTimeInt);

  bIsPswdModified = m_realpassword != m_oldRealPassword;

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    goto dont_close;
  }

  if (m_password.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVEPASSWORD);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    goto dont_close;
  }

  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    goto dont_close;
  }

  if (m_isPwHidden && (m_password.Compare(m_password2) != 0)) {
    AfxMessageBox(IDS_PASSWORDSNOTMATCH);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    goto dont_close;
  }

  ASSERT(m_pDbx != NULL);

  listindex = m_pDbx->Find(m_group, m_title, m_username);
  /*
  *  If there is a matching entry in our list, and that
  *  entry is not the same one we started editing, tell the
  *  user to try again.
  */
  if (listindex != m_pDbx->End()) {
    const CItemData &listItem = m_pDbx->GetEntryAt(listindex);
    uuid_array_t list_uuid, elem_uuid;
    listItem.GetUUID(list_uuid);
    m_ci->GetUUID(elem_uuid);
    bool notSame = (::memcmp(list_uuid, elem_uuid, sizeof(uuid_array_t)) != 0);
    if (notSame) {
      CSecString temp;
      temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
      AfxMessageBox(temp);
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
      goto dont_close;
    }
  }

  bool brc, b_msg_issued;
  brc = m_pDbx->CheckNewPassword(m_group, m_title, m_username, m_password,
                                 true, CItemData::ET_ALIAS,
                                 m_base_uuid, m_ibasedata, b_msg_issued);

  if (!brc && m_ibasedata != 0) {
    if (!b_msg_issued)
      AfxMessageBox(IDS_MUSTHAVETARGET, MB_OK);
    UpdateData(FALSE);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    goto dont_close;
  }

  if (!m_executestring.IsEmpty()) {
    //Check Execute string parses - don't substitute
    stringT errmsg;
    size_t st_column;
    m_pDbx->GetExpandedString(m_executestring, NULL, errmsg, st_column);
    if (errmsg.length() > 0) {
      CString cs_title(MAKEINTRESOURCE(IDS_EXECUTESTRING_ERROR));
      CString cs_temp(MAKEINTRESOURCE(IDS_EXS_IGNOREORFIX));
      CString cs_errmsg;
      cs_errmsg.Format(IDS_EXS_ERRORMSG, (int)st_column, errmsg.c_str());
      cs_errmsg += cs_temp;
      int rc = MessageBox(cs_errmsg, cs_title,
                          MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
      if (rc == IDNO)
        goto dont_close;
    }
  }
  //End check

  // Everything OK, update fields
  m_ci->SetGroup(m_group);
  m_ci->SetTitle(m_title);
  m_ci->SetUser(m_username.IsEmpty() ? m_defusername : m_username);
  m_ci->SetPassword(m_realpassword);
  m_ci->SetNotes(m_realnotes);
  m_ci->SetURL(m_URL);
  m_ci->SetAutoType(m_autotype);
  m_ci->SetPWHistory(m_PWHistory);
  m_ci->SetPWPolicy(m_pwp);
  m_ci->SetExecuteString(m_executestring);

  time_t t;
  time(&t);
  if (bIsPswdModified) {
    if (m_SavePWHistory)
      UpdateHistory();
    m_ci->SetPMTime(t);
  }
  if (bIsModified || bIsPswdModified)
    m_ci->SetRMTime(t);
  if (m_oldlocXTime != m_locXTime)
    m_ci->SetXTime(m_tttXTime);
  if (m_oldXTimeInt != m_XTimeInt)
    m_ci->SetXTimeInt(m_XTimeInt);

  CPWDialog::OnOK();
  return;

  // If we don't close, then update controls, as some of the fields
  // may have been modified (e.g., spaces removed).
dont_close:
  UpdateData(FALSE);
}

void CEditDlg::UpdateHistory()
{
  size_t num = m_PWHistList.size();
  PWHistEntry pwh_ent;
  pwh_ent.password = LPCTSTR(m_oldRealPassword);
  time_t t;
  m_ci->GetPMTime(t);
  if ((long)t == 0L) // if never set - try creation date
    m_ci->GetCTime(t);
  pwh_ent.changetttdate = t;
  pwh_ent.changedate =
    PWSUtil::ConvertToDateTimeString(t, TMC_EXPORT_IMPORT);
  if (pwh_ent.changedate.empty()) {
    CString unk;
    unk.LoadString(IDS_UNKNOWN);
    pwh_ent.changedate = LPCTSTR(unk);
  }

  // Now add the latest
  m_PWHistList.push_back(pwh_ent);

  // Increment count
  num++;

  // Too many? remove the excess
  if (num > m_MaxPWHistory) {
    PWHistList hl(m_PWHistList.begin() + (num - m_MaxPWHistory),
                  m_PWHistList.end());
    ASSERT(hl.size() == m_MaxPWHistory);
    m_PWHistList = hl;
    num = m_MaxPWHistory;
  }

  // Now create string version!
  CSecString new_PWHistory;
  CString buffer;

  buffer.Format(_T("1%02x%02x"), m_MaxPWHistory, num);
  new_PWHistory = CSecString(buffer);

  PWHistList::iterator iter;
  for (iter = m_PWHistList.begin(); iter != m_PWHistList.end(); iter++) {
    const PWHistEntry pwshe = *iter;

    buffer.Format(_T("%08x%04x%s"),
                  (long) pwshe.changetttdate, pwshe.password.length(),
                  pwshe.password.c_str());
    new_PWHistory += CSecString(buffer);
    buffer.Empty();
  }
  m_ci->SetPWHistory(new_PWHistory);
}

BOOL CEditDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
  ApplyPasswordFont(GetDlgItem(IDC_PASSWORD2));

  CString cs_text;
  if (m_Edit_IsReadOnly) {
    // Disable buttons
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(IDC_RANDOM)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_SET)->EnableWindow(FALSE);

    // Disable Checkbox
    GetDlgItem(IDC_OVERRIDE_POLICY)->EnableWindow(FALSE);

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
    GetDlgItem(IDC_AUTOTYPE)->SendMessage(EM_SETREADONLY, TRUE, 0);

    cs_text.LoadString(IDS_VIEWENTRY);
    SetWindowText(cs_text);
    cs_text.LoadString(IDS_DATABASEREADONLY);
    GetDlgItem(IDC_EDITEXPLANATION)->SetWindowText(cs_text);
  }

  const int n = m_ci->NumberUnknownFields();
  if (n > 0) {
    cs_text.Format(IDS_RECORDUNKNOWNFIELDS, n);
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
  if (m_original_entrytype == CItemData::ET_ALIASBASE ||
      m_original_entrytype == CItemData::ET_SHORTCUTBASE) {
    // Show button to allow users to view dependents
    CString csButtonText;
    csButtonText.LoadString(m_original_entrytype == CItemData::ET_ALIASBASE ? IDS_VIEWALIASESBTN : IDS_VIEWSHORTCUTSBTN);
    GetDlgItem(IDC_VIEWDEPENDENTS)->SetWindowText(csButtonText);
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ALIASGRP)->ShowWindow(SW_HIDE);
  } else if (m_original_entrytype == CItemData::ET_ALIAS) {
    // Update password to alias form
    // Show text stating that it is an alias
    m_realpassword = m_oldRealPassword = m_base;
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_ALIASGRP)->ShowWindow(SW_SHOW);
  } else if (m_original_entrytype == CItemData::ET_NORMAL) {
    // Normal - do none of the above
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ALIASGRP)->ShowWindow(SW_HIDE);
  }

  ((CEdit*)GetDlgItem(IDC_PASSWORD2))->SetPasswordChar(PSSWDCHAR);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  if (m_bShowNotes == TRUE) {
    ShowNotes();
  } else {
    HideNotes();
  }

  GetDlgItem(IDC_NOTES)->EnableWindow(m_bWordWrap == TRUE ? FALSE : TRUE);
  GetDlgItem(IDC_NOTESWW)->EnableWindow(m_bWordWrap == TRUE ? TRUE : FALSE);
  GetDlgItem(IDC_NOTES)->ShowWindow(m_bWordWrap == TRUE ? SW_HIDE : SW_SHOW);
  GetDlgItem(IDC_NOTESWW)->ShowWindow(m_bWordWrap == TRUE ? SW_SHOW : SW_HIDE);

  if (!m_Edit_IsReadOnly) {
    // Populate the groups combo box
    if (m_ex_group.GetCount() == 0) {
      std::vector<stringT> aryGroups;
      app.m_core.GetUniqueGroups(aryGroups);
      for (size_t igrp = 0; igrp < aryGroups.size(); igrp++) {
        m_ex_group.AddString(aryGroups[igrp].c_str());
      }
    }
  }

  GetDlgItem(IDC_PWHSTATUS)->
    SetWindowText(m_SavePWHistory == TRUE ? CS_ON : CS_OFF);
  CString buffer;
  if (m_SavePWHistory == TRUE)
    buffer.Format(_T("%d"), m_MaxPWHistory);
  else
    buffer = _T("n/a");

  GetDlgItem(IDC_PWHMAX)->SetWindowText(buffer);

  if (m_XTimeInt != 0) // recurring expiration
    cs_text.Format(IDS_IN_N_DAYS, m_XTimeInt);
  else
    cs_text = _T("");
  GetDlgItem(IDC_XTIME_RECUR2)->SetWindowText(cs_text);

  UpdateData(FALSE);
  // create tooltip unconditionally, JIC
  m_ToolTipCtrl = new CToolTipCtrl;
  if (!m_ToolTipCtrl->Create(this, 0)) {
    TRACE("Unable To create Edit Dialog ToolTip\n");
  } else {
    CString cs_ToolTip;
    if (m_OverridePolicy == TRUE) {
     cs_ToolTip.LoadString(IDS_OVERRIDE_POLICY);
      m_ToolTipCtrl->AddTool(GetDlgItem(IDC_OVERRIDE_POLICY), cs_ToolTip);
    }
    cs_ToolTip.LoadString(IDS_CLICKTOCOPY);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_GROUP), cs_ToolTip);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_TITLE), cs_ToolTip);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_USERNAME), cs_ToolTip);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_PASSWORD), cs_ToolTip);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_NOTES), cs_ToolTip);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_URL), cs_ToolTip);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_AUTO), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_CLICKTOCOPYEXPAND);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_STATIC_EXECUTE), cs_ToolTip);

    EnableToolTips();
    m_ToolTipCtrl->Activate(TRUE);
  }

  m_stc_group.SetHighlight(true, crefWhite);
  m_stc_title.SetHighlight(true, crefWhite);
  m_stc_username.SetHighlight(true, crefWhite);
  m_stc_password.SetHighlight(true, crefWhite);
  m_stc_notes.SetHighlight(true, crefWhite);
  m_stc_URL.SetHighlight(true, crefWhite);
  m_stc_autotype.SetHighlight(true, crefWhite);
  m_stc_executestring.SetHighlight(true, crefWhite);

  GetDlgItem(IDC_LAUNCH)->EnableWindow(m_URL.IsEmpty() ? FALSE : TRUE);

  m_isExpanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DisplayExpandedAddEditDlg);
  ResizeDialog();

  m_ex_group.ChangeColour();

  return TRUE;
}

void CEditDlg::ShowPassword()
{
  m_isPwHidden = false;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_HIDE);

  m_password = m_realpassword;
  m_ex_password.SetSecure(false);
  // Remove password character so that the password is displayed
  m_ex_password.SetPasswordChar(0);
  m_ex_password.Invalidate();

  // Don't need verification as the user can see the password entered
  // use m_password2 to show something useful...
  ostringstreamT os;
  for (int i = 1; i <= m_realpassword.GetLength(); i++)
    os << (i % 10);
  m_password2 = os.str().c_str();
  m_ex_password2.SetSecure(false);
  m_ex_password2.SetPasswordChar(0);
  m_ex_password2.EnableWindow(FALSE);
  m_ex_password2.Invalidate();
}

void CEditDlg::HidePassword()
{
  m_isPwHidden = true;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_SHOW);

  m_password = m_password2 = HIDDEN_PASSWORD;

  m_ex_password.SetSecure(true);
  // Set password character so that the password is not displayed
  m_ex_password.SetPasswordChar(PSSWDCHAR);
  m_ex_password2.SetSecure(true);
  m_ex_password2.SetPasswordChar(PSSWDCHAR);
  m_ex_password2.SetSecureText(m_password2);
  m_ex_password.Invalidate();

  // Need verification as the user can not see the password entered
  m_ex_password2.EnableWindow(TRUE);
}

void CEditDlg::ShowNotes()
{
  m_isNotesHidden = false;
  m_notes = m_realnotes;
  m_notesww = m_realnotes;

  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();
  ((CEdit*)GetDlgItem(IDC_NOTESWW))->Invalidate();
 }

void CEditDlg::HideNotes()
{
  m_isNotesHidden = true;
  if (m_bWordWrap == TRUE) {
    if (m_notesww != HIDDEN_NOTES) {
      m_realnotes = m_notesww;
      m_notes = HIDDEN_NOTES;
      m_notesww = HIDDEN_NOTES;
    }
  } else {
    if (m_notes != HIDDEN_NOTES) {
      m_realnotes = m_notes;
      m_notes = HIDDEN_NOTES;
      m_notesww = HIDDEN_NOTES;
    }
  }


  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();
  ((CEdit*)GetDlgItem(IDC_NOTESWW))->Invalidate();
}

void CEditDlg::OnRandom() 
{
  UpdateData(TRUE);
  StringX passwd;
  m_pDbx->MakeRandomPassword(passwd, m_pwp);
  m_realpassword = passwd.c_str();
  if (!m_isPwHidden) {
      m_password = m_realpassword;
      UpdateData(FALSE);
  }
  // This will be reset to the time the user eventually presses OK
  time(&m_tttCPMTime);
  if (m_XTimeInt != 0) {
    CTime ct = CTime(m_tttCPMTime) + CTimeSpan(m_XTimeInt, 0, 0, 0);
    m_tttXTime = (time_t)ct.GetTime();
    m_locXTime = PWSUtil::ConvertToDateTimeString(m_tttXTime, TMC_LOCALE);
  } else {
    if (m_tttXTime <= m_tttCPMTime) {
      m_locXTime.LoadString(IDS_NEVER);
      m_tttXTime = 0;
    }
  }
  GetDlgItem(IDC_XTIME)->SetWindowText(m_locXTime);
  CSecString cs_locPMTime = PWSUtil::ConvertToDateTimeString(m_tttCPMTime, TMC_LOCALE);
  GetDlgItem(IDC_PMTIME)->SetWindowText(cs_locPMTime);
  UpdateData(TRUE);
}

void CEditDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#editview"), 
                NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/entering_pwd.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

void CEditDlg::OnPasskeyKillfocus()
{
#if defined(POCKET_PC)
  /************************************************************************/
  /* Restore the state of word completion when the password field loses   */
  /* focus.                                                               */
  /************************************************************************/
  EnableWordCompletion( m_hWnd );
#endif
}

void CEditDlg::OnPasskeySetfocus()
{
  m_ex_password.SetSel(0, -1);
#if defined(POCKET_PC)
  /************************************************************************/
  /* When the password field is activated, pull up the SIP and disable    */
  /* word completion.                                                     */
  /************************************************************************/
  DisableWordCompletion( m_hWnd );
#endif
}

void CEditDlg::OnBnClickedOk()
{
  OnOK();
}

void CEditDlg::OnBnClickedMore()
{
  m_isExpanded = !m_isExpanded;
  PWSprefs::GetInstance()->
    SetPref(PWSprefs::DisplayExpandedAddEditDlg, m_isExpanded);
  ResizeDialog();
}

void CEditDlg::ResizeDialog()
{
  const int TopHideableControl = IDC_TOP_HIDEABLE;
  const int BottomHideableControl = IDC_BOTTOM_HIDEABLE;
  const int controls[]={
    IDC_URL,
    IDC_STATIC_URL,
    IDC_AUTOTYPE,
    IDC_STATIC_AUTO,
    IDC_EXECUTE,
    IDC_STATIC_EXECUTE,
    IDC_CTIME,
    IDC_STATIC_CTIME,
    IDC_PMTIME,
    IDC_STATIC_PMTIME,
    IDC_ATIME,
    IDC_STATIC_ATIME,
    IDC_XTIME,
    IDC_XTIME_RECUR2,
    IDC_STATIC_XTIME,
    IDC_RMTIME,
    IDC_STATIC_RMTIME,
    IDC_XTIME_CLEAR,
    IDC_XTIME_SET,
    IDC_STATIC_DTGROUP,
    IDC_STATIC_DTEXPGROUP,    
  };

  int windows_state = m_isExpanded ? SW_SHOW : SW_HIDE;
  for(unsigned n = 0; n < sizeof(controls)/sizeof(controls[0]); n++) {
    CWnd* pWind;
    pWind = (CWnd *)GetDlgItem(controls[n]);
    pWind->ShowWindow(windows_state);
  }

  RECT curDialogRect;

  this->GetWindowRect(&curDialogRect);

  RECT newDialogRect = curDialogRect;

  RECT curLowestCtlRect;
  CWnd* pLowestCtl;
  int newHeight;
  CString cs_text;

  if (m_isExpanded) {
    // from less to more
    pLowestCtl = (CWnd *)GetDlgItem(BottomHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);
    newHeight = curLowestCtlRect.bottom + 15 - newDialogRect.top;

    cs_text.LoadString(IDS_LESS);
    m_MoreLessBtn.SetWindowText(cs_text);
  } else {
    // from more to less
    pLowestCtl = (CWnd *)GetDlgItem(TopHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);
    newHeight = curLowestCtlRect.top + 5 - newDialogRect.top;

    cs_text.LoadString(IDS_MORE);
    m_MoreLessBtn.SetWindowText(cs_text);
  }

  this->SetWindowPos(NULL, 0, 0, newDialogRect.right - newDialogRect.left,
                     newHeight, SWP_NOMOVE);
}

void CEditDlg::OnBnClickedClearXTime()
{
  m_locXTime.LoadString(IDS_NEVER);
  GetDlgItem(IDC_XTIME)->SetWindowText((CString)m_locXTime);
  GetDlgItem(IDC_XTIME_RECUR2)->SetWindowText(_T(""));
  m_tttXTime = (time_t)0;
  m_XTimeInt = 0;
}

void CEditDlg::OnBnClickedSetXTime()
{
  CExpDTDlg dlg_expDT(m_tttCPMTime,
                      m_tttXTime,
                      m_XTimeInt,
                      this);

  app.DisableAccelerator();
  INT_PTR rc = dlg_expDT.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    CString cs_text;
    m_tttXTime = dlg_expDT.m_tttXTime;
    m_locXTime = dlg_expDT.m_locXTime;
    m_XTimeInt = dlg_expDT.m_XTimeInt;
    if (m_XTimeInt != 0) // recurring expiration
      cs_text.Format(IDS_IN_N_DAYS, m_XTimeInt);
    GetDlgItem(IDC_XTIME)->SetWindowText(m_locXTime);
    GetDlgItem(IDC_XTIME_RECUR2)->SetWindowText(cs_text);
  }
}

void CEditDlg::OnBnClickedPwhist()
{
  CPWHistDlg dlg(this, m_Edit_IsReadOnly,
    m_PWHistory, m_PWHistList,
    m_NumPWHistory, m_MaxPWHistory,
    m_SavePWHistory);

  dlg.DoModal();

  GetDlgItem(IDC_PWHSTATUS)->
             SetWindowText(m_SavePWHistory == TRUE ? CS_ON : CS_OFF);
  CString buffer;
  if (m_SavePWHistory == TRUE)
    buffer.Format(_T("%d"), m_MaxPWHistory);
  else
    buffer = _T("n/a");

  GetDlgItem(IDC_PWHMAX)->SetWindowText(buffer);
}

void CEditDlg::OnEnSetfocusNotes()
{
  UpdateData(TRUE);
  if (m_isNotesHidden) {
    ShowNotes();
  }
  UpdateData(FALSE);
}

void CEditDlg::OnEnKillfocusNotes()
{
  UpdateData(TRUE);
  if (!m_isNotesHidden && m_bShowNotes == FALSE) {
    HideNotes();
  }
  UpdateData(FALSE);
}

LRESULT CEditDlg::OnCallExternalEditor(WPARAM, LPARAM)
{
  // Warn the user about sensitive data lying around
  int rc = AfxMessageBox(IDS_EXTERNAL_EDITOR_WARNING, 
                         MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2);
  if (rc != IDYES)
    return 0L;

  GetDlgItem(IDOK)->EnableWindow(FALSE);
  GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

  const CString cs_text(MAKEINTRESOURCE(IDS_NOTES_IN_EXTERNAL_EDITOR));
  CWnd *pwnotes = GetDlgItem(IDC_NOTES);
  pwnotes->EnableWindow(FALSE);
  pwnotes->SetWindowText(cs_text);

  m_thread = CExtThread::BeginThread(ExternalEditorThread, this);
  return 0L;
}

UINT CEditDlg::ExternalEditorThread(LPVOID me) // static method!
{
  CEditDlg *self = (CEditDlg *)me;

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

  ofs << LPCTSTR(self->m_realnotes) << std::endl;
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

LRESULT CEditDlg::OnExternalEditorEnded(WPARAM, LPARAM)
{
  if (!m_Edit_IsReadOnly)
    GetDlgItem(IDOK)->EnableWindow(TRUE);

  GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
  GetDlgItem(IDC_NOTES)->EnableWindow(TRUE);

  // Now get what the user saved in this file and put it back into Notes field
  ifstreamT ifs(m_szTempName);
  if (ifs.bad())
    return 16;

  m_realnotes.Empty();
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

  // Set real notes field
  m_realnotes = note.c_str();
  // We are still displaying the old text, so replace that too
  m_notes = m_realnotes;
  m_notesww = m_realnotes;

  UpdateData(FALSE);
  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();
  ((CEdit*)GetDlgItem(IDC_NOTESWW))->Invalidate();

  // Delete temporary file
  _tremove(m_szTempName);
  memset(m_szTempName, 0, sizeof(m_szTempName));
  return 0;
}

void CEditDlg::OnBnClickViewDependents()
{
  CString cs_msg, cs_type;

  if (m_original_entrytype == CItemData::ET_ALIASBASE)
    cs_type.LoadString(m_num_dependents == 1 ? IDS_ALIAS : IDS_ALIASES);
  else
    cs_type.LoadString(m_num_dependents == 1 ? IDS_SHORTCUT : IDS_SHORTCUTS);

  cs_msg.Format(IDS_VIEWDEPENDENTS, m_num_dependents, cs_type, m_dependents);
  MessageBox(cs_msg, AfxGetAppName(), MB_OK);
}

void CEditDlg::OnBnClickedOverridePolicy()
{
  // If state was true AND shift key pressed,
  // show existing policy rather than transit to clear
  if (m_OverridePolicy && GetAsyncKeyState(VK_SHIFT) < 0) {
    m_pDbx->SetPasswordPolicy(m_pwp);
    UpdateData(FALSE); // don't change state
    return;
  }
  UpdateData(TRUE);
  if (m_OverridePolicy == TRUE) {
    m_pDbx->SetPasswordPolicy(m_pwp);
  } else
    m_pwp.Empty();

  if (m_OverridePolicy == TRUE) {
    CString cs_ToolTip;
    cs_ToolTip.LoadString(IDS_OVERRIDE_POLICY);
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_OVERRIDE_POLICY), cs_ToolTip);
  } else
    m_ToolTipCtrl->DelTool(GetDlgItem(IDC_OVERRIDE_POLICY));
}

void CEditDlg::OnStcClicked(UINT nID)
{
  UpdateData(TRUE);
  StringX cs_data;
  int iaction(0);
  // NOTE: These values must be contiguous in "resource.h"
  switch (nID) {
    case IDC_STATIC_GROUP:
      m_stc_group.FlashBkgnd(crefGreen);
      cs_data = StringX(m_group);
      iaction = CItemData::GROUP;
      break;
    case IDC_STATIC_TITLE:
      m_stc_title.FlashBkgnd(crefGreen);
      cs_data = StringX(m_title);
      iaction = CItemData::TITLE;
      break;
    case IDC_STATIC_USERNAME:
      m_stc_username.FlashBkgnd(crefGreen);
      cs_data = StringX(m_username);
      iaction = CItemData::USER;
      break;
    case IDC_STATIC_PASSWORD:
      m_stc_password.FlashBkgnd(crefGreen);
      cs_data = StringX(m_realpassword);
      iaction = CItemData::PASSWORD;
      break;
    case IDC_STATIC_NOTES:
      m_stc_notes.FlashBkgnd(crefGreen);
      cs_data = StringX(m_bWordWrap == TRUE ? m_notesww : m_notes);
      iaction = CItemData::NOTES;
      break;
    case IDC_STATIC_URL:
      m_stc_URL.FlashBkgnd(crefGreen);
      cs_data = StringX(m_URL);
      iaction = CItemData::URL;
      break;
    case IDC_STATIC_AUTO:
      m_stc_autotype.FlashBkgnd(crefGreen);
      cs_data = m_pDbx->GetAutoTypeString(StringX(m_autotype),
                                          StringX(m_group), StringX(m_title), StringX(m_username), 
                                          StringX(m_realpassword), 
                                          StringX(m_bWordWrap == TRUE ? m_notesww : m_notes));
      iaction = CItemData::AUTOTYPE;
      break;
    case IDC_STATIC_EXECUTE:
      m_stc_executestring.FlashBkgnd(crefGreen);
      // If Shift pressed - just copy un-substituted Execute String
      if (GetKeyState(VK_CONTROL) != 0 || m_executestring.IsEmpty()) {
        cs_data = StringX(m_executestring);
      } else {
        stringT errmsg;
        size_t st_column;
        cs_data = m_pDbx->GetExpandedString(m_executestring, m_ci, errmsg, st_column);
        if (errmsg.length() > 0) {
          CString cs_title(MAKEINTRESOURCE(IDS_EXECUTESTRING_ERROR));
          CString cs_errmsg;
          cs_errmsg.Format(IDS_EXS_ERRORMSG, (int)st_column, errmsg.c_str());
          MessageBox(cs_errmsg, cs_title, MB_ICONERROR);
        }
      }
      iaction = CItemData::EXECUTE;
      break;
    default:
      ASSERT(0);
  }
  m_pDbx->SetClipboardData(cs_data);
  m_pDbx->UpdateLastClipboardAction(iaction);
}

void CEditDlg::SelectAllNotes()
{
  // User pressed Ctrl+A
  ShowNotes();
  ((CEdit *)GetDlgItem(m_bWordWrap == TRUE ? IDC_NOTESWW : IDC_NOTES))->
            SetSel(0, -1, TRUE);
}

BOOL CEditDlg::PreTranslateMessage(MSG* pMsg)
{
/*
 * Part of the chicken waving needed to get
 * tooltips to do their thing.
 */
  if (m_ToolTipCtrl != NULL)
    m_ToolTipCtrl->RelayEvent(pMsg);

  // if user hit Ctrl+A in Notes control, then SelectAllNotes
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'A' &&
      (GetKeyState(VK_CONTROL) & 0x8000) &&
      GetDlgItem(IDC_NOTES)->m_hWnd == ::GetFocus()) {
    SelectAllNotes();
    return TRUE;
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CEditDlg::OnBnClickedLaunch()
{
  UpdateData(TRUE);
  StringX cs_data = StringX(m_URL);
  int iaction = CItemData::URL;
  m_pDbx->LaunchBrowser(CString(cs_data.c_str()));
  m_pDbx->UpdateLastClipboardAction(iaction);
}

void CEditDlg::OnEnChangeUrl()
{
  UpdateData(TRUE);
  GetDlgItem(IDC_LAUNCH)->EnableWindow(m_URL.IsEmpty() ? FALSE : TRUE);
}

LRESULT CEditDlg::OnWordWrap(WPARAM, LPARAM)
{
  m_bWordWrap = m_bWordWrap == TRUE ? FALSE : TRUE;
  // Get value of notes from dialog.
  UpdateData(TRUE);
  if (m_bWordWrap == FALSE)
    m_notes = m_notesww;
  else
    m_notesww = m_notes;
  // Update dalog
  UpdateData(FALSE);

  GetDlgItem(IDC_NOTES)->EnableWindow(m_bWordWrap == TRUE ? FALSE : TRUE);
  GetDlgItem(IDC_NOTESWW)->EnableWindow(m_bWordWrap == TRUE ? TRUE : FALSE);
  GetDlgItem(IDC_NOTES)->ShowWindow(m_bWordWrap == TRUE ? SW_HIDE : SW_SHOW);
  GetDlgItem(IDC_NOTESWW)->ShowWindow(m_bWordWrap == TRUE ? SW_SHOW : SW_HIDE);

  m_pex_notes->UpdateState(WM_EDIT_WORDWRAP, m_bWordWrap);
  m_pex_notesww->UpdateState(WM_EDIT_WORDWRAP, m_bWordWrap);
  return 0L;
}

LRESULT CEditDlg::OnShowNotes(WPARAM, LPARAM)
{
  m_bShowNotes = m_bShowNotes == TRUE ? FALSE : TRUE;
  if (m_bShowNotes == TRUE) {
    ShowNotes();
  } else {
    HideNotes();
  }
  UpdateData(FALSE);

  m_pex_notes->UpdateState(WM_EDIT_SHOWNOTES, m_bShowNotes);
  m_pex_notesww->UpdateState(WM_EDIT_SHOWNOTES, m_bShowNotes);
  return 0L;
}
