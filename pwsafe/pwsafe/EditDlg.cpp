/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
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
typedef std::wstring stringT;
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::string stringT;
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');
CMyString CEditDlg::HIDDEN_NOTES;
CString CEditDlg::CS_ON;
CString CEditDlg::CS_OFF;
CString CEditDlg::CS_SHOW;
CString CEditDlg::CS_HIDE;

CEditDlg::CEditDlg(CItemData *ci, CWnd* pParent)
  : CPWDialog(CEditDlg::IDD, pParent),
    m_ci(ci), m_bIsModified(false), m_Edit_IsReadOnly(false),
    m_tttLTime (time_t(0)),
    m_locLTime(_T("")), m_oldlocLTime(_T("")),
    m_original_entrytype(CItemData::Normal)
{
  ASSERT(ci != NULL);

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

  m_pwp.Empty();

  BOOL HasHistory = FALSE;
  ci->CreatePWHistoryList(HasHistory, m_MaxPWHistory,
                          m_NumPWHistory, 
                          &m_PWHistList, TMC_EXPORT_IMPORT);
  m_SavePWHistory = HasHistory;

  m_group = ci->GetGroup();
  m_title = ci->GetTitle();
  m_username = ci->GetUser();
  m_password = m_password2 = HIDDEN_PASSWORD;
  m_realpassword = m_oldRealPassword = ci->GetPassword();
  m_URL = ci->GetURL();
  m_autotype = ci->GetAutoType();
  m_notes = HIDDEN_NOTES;
  m_realnotes = ci->GetNotes();
  m_PWHistory = ci->GetPWHistory();
  m_locCTime = ci->GetCTimeL();
  m_locPMTime = ci->GetPMTimeL();
  m_locATime = ci->GetATimeL();
  m_locRMTime = ci->GetRMTimeL();

  m_locLTime = ci->GetLTimeL();
  if (m_locLTime.IsEmpty()) {
    m_locLTime.LoadString(IDS_NEVER);
    m_tttLTime = 0;
  } else
    ci->GetLTime(m_tttLTime);

  m_oldlocLTime = m_locLTime;

  m_pex_notes = new CEditExtn(WM_CALL_EXTERNAL_EDITOR, 
                              _T("! &Edit externally"));
  m_num_dependents = 0;
  m_dependents = _T("");
  m_base = _T("");
}

CEditDlg::~CEditDlg()
{
  delete m_pex_notes;
}

void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
  DDX_Text(pDX, IDC_PASSWORD2, (CString&)m_password2);
  DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_URL, (CString&)m_URL);
  DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);
  DDX_Text(pDX, IDC_CTIME, (CString&)m_locCTime);
  DDX_Text(pDX, IDC_PMTIME, (CString&)m_locPMTime);
  DDX_Text(pDX, IDC_ATIME, (CString&)m_locATime);
  DDX_Text(pDX, IDC_LTIME, (CString&)m_locLTime);
  DDX_Text(pDX, IDC_RMTIME, (CString&)m_locRMTime);

  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Control(pDX, IDC_MORE, m_MoreLessBtn);
  DDX_Control(pDX, IDC_VIEWDEPENDENTS, m_ViewDependentsBtn);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
  DDX_Control(pDX, IDC_NOTES, *m_pex_notes);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_URL, m_ex_URL);
  DDX_Control(pDX, IDC_AUTOTYPE, m_ex_autotype);
}

BEGIN_MESSAGE_MAP(CEditDlg, CPWDialog)
  ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowPassword)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_RANDOM, OnRandom)
  ON_BN_CLICKED(IDC_SETPWPOLICY, OnSetPolicy)
  ON_BN_CLICKED(IDC_CLEARPWPOLICY, OnClearPolicy)
#if defined(POCKET_PC)
  ON_WM_SHOWWINDOW()
#endif
  ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetfocus)
  ON_EN_KILLFOCUS(IDC_PASSWORD, OnPasskeyKillfocus)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
  ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
  ON_BN_CLICKED(IDC_VIEWDEPENDENTS, OnBnClickViewDependents)
  ON_BN_CLICKED(IDC_LTIME_CLEAR, OnBnClickedClearLTime)
  ON_BN_CLICKED(IDC_LTIME_SET, OnBnClickedSetLTime)
  ON_BN_CLICKED(IDC_PWHIST, OnBnClickedPwhist)
  ON_EN_SETFOCUS(IDC_NOTES, OnEnSetfocusNotes)
  ON_EN_KILLFOCUS(IDC_NOTES, OnEnKillfocusNotes)
  ON_MESSAGE(WM_CALL_EXTERNAL_EDITOR, OnCallExternalEditor)
  ON_MESSAGE(WM_EXTERNAL_EDITOR_ENDED, OnExternalEditorEnded)
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

void
CEditDlg::OnOK() 
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
    m_realnotes = m_notes;

  m_bIsModified |= (m_group != m_ci->GetGroup() ||
                    m_title != m_ci->GetTitle() ||
                    m_username != m_ci->GetUser() ||
                    m_realnotes != m_ci->GetNotes() ||
                    m_URL != m_ci->GetURL() ||
                    m_autotype != m_ci->GetAutoType() ||
                    m_PWHistory != m_ci->GetPWHistory() ||
                    m_locLTime != m_oldlocLTime);

  bool IsPswdModified = m_realpassword != m_oldRealPassword;

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

  DboxMain* dbx = static_cast<DboxMain *>(GetParent());
  ASSERT(dbx != NULL);

  listindex = dbx->Find(m_group, m_title, m_username);
  /*
   *  If there is a matching entry in our list, and that
   *  entry is not the same one we started editing, tell the
   *  user to try again.
   */
  if (listindex != dbx->End()) {
    const CItemData &listItem = dbx->GetEntryAt(listindex);
    uuid_array_t list_uuid, elem_uuid;
    listItem.GetUUID(list_uuid);
    m_ci->GetUUID(elem_uuid);
    bool notSame = (::memcmp(list_uuid, elem_uuid, sizeof(uuid_array_t)) != 0);
    if (notSame) {
      CMyString temp;
      temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
      AfxMessageBox(temp);
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
      goto dont_close;
    }
  }

  bool brc = dbx->CheckNewPassword(m_group, m_title, m_username, m_password,
                             true, CItemData::Alias, m_base_uuid, m_ibasedata);
  if (!brc && m_ibasedata != 0) {
    // Already issued error message to user
    UpdateData(FALSE);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    goto dont_close;
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

  time_t t;
  time(&t);
  if (IsPswdModified) {
    if (m_SavePWHistory)
      UpdateHistory();
    m_ci->SetPMTime(t);
  }
  if (m_bIsModified || IsPswdModified)
    m_ci->SetRMTime(t);
  if (m_oldlocLTime != m_locLTime)
    m_ci->SetLTime(m_tttLTime);

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
    pwh_ent.password = m_oldRealPassword;
    time_t t;
    m_ci->GetPMTime(t);
    if ((long)t == 0L) // if never set - try creation date
        m_ci->GetCTime(t);
    pwh_ent.changetttdate = t;
    pwh_ent.changedate =
        PWSUtil::ConvertToDateTimeString(t, TMC_EXPORT_IMPORT);
    if (pwh_ent.changedate.IsEmpty()) {
        pwh_ent.changedate.LoadString(IDS_UNKNOWN);
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
    CMyString new_PWHistory;
    CString buffer;

    buffer.Format(_T("1%02x%02x"), m_MaxPWHistory, num);
    new_PWHistory = CMyString(buffer);

    PWHistList::iterator iter;
    for (iter = m_PWHistList.begin(); iter != m_PWHistList.end(); iter++) {
        const PWHistEntry pwshe = *iter;

        buffer.Format(_T("%08x%04x%s"),
                      (long) pwshe.changetttdate, pwshe.password.GetLength(),
                      pwshe.password);
        new_PWHistory += CMyString(buffer);
        buffer.Empty();
    }
    m_ci->SetPWHistory(new_PWHistory);
}

BOOL CEditDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  SetPasswordFont(GetDlgItem(IDC_PASSWORD));
  SetPasswordFont(GetDlgItem(IDC_PASSWORD2));

  CString cs_text;
  if (m_Edit_IsReadOnly) {
    GetDlgItem(IDOK)->EnableWindow(FALSE);
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
  if (m_original_entrytype == CItemData::AliasBase ||
      m_original_entrytype == CItemData::ShortcutBase) {
    // Show button to allow users to view dependents
    CString csButtonText;
    csButtonText.LoadString(m_original_entrytype == CItemData::AliasBase ? IDS_VIEWALIASESBTN : IDS_VIEWSHORTCUTSBTN);
    GetDlgItem(IDC_VIEWDEPENDENTS)->SetWindowText(csButtonText);
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ALIASGRP)->ShowWindow(SW_HIDE);
  } else
  if (m_original_entrytype == CItemData::Alias) {
    // Update password to alias form
    // Show text stating that it is an alias
    m_realpassword = m_oldRealPassword = m_base;
    GetDlgItem(IDC_VIEWDEPENDENTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ISANALIAS)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_ALIASGRP)->ShowWindow(SW_SHOW);
  } else
  if (m_original_entrytype == CItemData::Normal) {
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

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault)) {
    ShowNotes();
  } else {
    HideNotes();
  }

  if (!m_Edit_IsReadOnly) {
    // Populate the groups combo box
    if (m_ex_group.GetCount() == 0) {
      CStringArray aryGroups;
      app.m_core.GetUniqueGroups(aryGroups);
      for (int igrp = 0; igrp < aryGroups.GetSize(); igrp++) {
        m_ex_group.AddString((LPCTSTR)aryGroups[igrp]);
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

  UpdateData(FALSE);

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
  // Remove password character so that the password is displayed
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(0);
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
  // Don't need verification as the user can see the password entered
  m_password2.Empty();
  GetDlgItem(IDC_PASSWORD2)->EnableWindow(FALSE);
}

void CEditDlg::HidePassword()
{
  m_isPwHidden = true;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_SHOW);

  m_password = m_password2 = HIDDEN_PASSWORD;

  // Set password character so that the password is not displayed
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(PSSWDCHAR);
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
  // Need verification as the user can not see the password entered
  GetDlgItem(IDC_PASSWORD2)->EnableWindow(TRUE);
}

void CEditDlg::ShowNotes()
{
  m_isNotesHidden = false;
  m_notes = m_realnotes;

  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();
  GetDlgItem(IDC_NOTES)->EnableWindow(TRUE);
}

void CEditDlg::HideNotes()
{
  m_isNotesHidden = true;
  if (m_notes != HIDDEN_NOTES) {
    m_realnotes = m_notes;
    m_notes = HIDDEN_NOTES;
  }
  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();
}

void CEditDlg::OnRandom() 
{
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);

  UpdateData(TRUE);

  if (pParent->MakeRandomPassword(this, m_realpassword, m_pwp) &&
      !m_isPwHidden) {
    m_password = m_realpassword;
    UpdateData(FALSE);
  }
}

void CEditDlg::OnSetPolicy() 
{
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);

  pParent->SetPasswordPolicy(m_pwp);
}

void CEditDlg::OnClearPolicy() 
{
  m_pwp.Empty();
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
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetSel(0, -1);
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
    IDC_CTIME,
    IDC_STATIC_CTIME,
    IDC_PMTIME,
    IDC_STATIC_PMTIME,
    IDC_ATIME,
    IDC_STATIC_ATIME,
    IDC_LTIME,
    IDC_STATIC_LTIME,
    IDC_RMTIME,
    IDC_STATIC_RMTIME,
    IDC_LTIME_CLEAR,
    IDC_LTIME_SET,
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

void CEditDlg::OnBnClickedClearLTime()
{
  m_locLTime.LoadString(IDS_NEVER);
  GetDlgItem(IDC_LTIME)->SetWindowText((CString)m_locLTime);
  m_tttLTime = (time_t)0;
}

void CEditDlg::OnBnClickedSetLTime()
{
  CExpDTDlg dlg_expDT(this);

  dlg_expDT.m_tttLTime = m_tttLTime;

  app.DisableAccelerator();
  INT_PTR rc = dlg_expDT.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    m_tttLTime = dlg_expDT.m_tttLTime;
    m_locLTime = dlg_expDT.m_locLTime;
    GetDlgItem(IDC_LTIME)->SetWindowText(m_locLTime);
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
  if (!m_isNotesHidden &&
      !PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault)) {
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
  GetTempPath(dwBufSize,   // length of the buffer
       lpPathBuffer);      // buffer for path

  // Create a temporary file.
  GetTempFileName(lpPathBuffer, // directory for temp files
      _T("NTE"),                // temp file name prefix
      0,                        // create unique name
      self->m_szTempName);            // buffer for name

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

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

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
    TRACE( "CreateProcess failed (%d).\n", GetLastError() );
  }

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
  UpdateData(FALSE);
  ((CEdit*)GetDlgItem(IDC_NOTES))->Invalidate();

  // Delete temporary file
  _tremove(m_szTempName);
  return 0;
}

void CEditDlg::OnBnClickViewDependents()
{
  CString cs_msg, cs_type;

  if (m_original_entrytype == CItemData::AliasBase)
    cs_type.LoadString(m_num_dependents == 1 ? IDS_ALIAS : IDS_ALIASES);
  else
    cs_type.LoadString(m_num_dependents == 1 ? IDS_SHORTCUT : IDS_SHORTCUTS);

  const UINT iMsg_Num = (m_original_entrytype == CItemData::AliasBase) ? IDS_VIEWALIASES : IDS_VIEWSHORTCUTS;
  cs_msg.Format(iMsg_Num, m_num_dependents, cs_type, m_dependents);
  MessageBox(cs_msg, AfxGetAppName(), MB_OK);
}
