/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file EditShortcutDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "EditShortcutDlg.h"
#include "corelib/PWSprefs.h"
#include "corelib/ItemData.h"
#include "ControlExtns.h"

#include <shlwapi.h>
#include <fstream>
using namespace std;

#if defined(POCKET_PC)
#include "pocketpc/PocketPC.h"
#include "editshortcutdlg.h"
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

CEditShortcutDlg::CEditShortcutDlg(CItemData *ci, CWnd* pParent)
  : CPWDialog(CEditShortcutDlg::IDD, pParent),
  m_ci(ci), m_bIsModified(false), m_Edit_IsReadOnly(false)
{
  ASSERT(ci != NULL);

  m_group = ci->GetGroup();
  m_title = ci->GetTitle();
  m_username = ci->GetUser();

  m_locCTime = ci->GetCTimeL();
  m_locPMTime = ci->GetPMTimeL();
  m_locATime = ci->GetATimeL();
  m_locRMTime = ci->GetRMTimeL();
}

CEditShortcutDlg::~CEditShortcutDlg()
{
}

void CEditShortcutDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
  DDX_Text(pDX, IDC_TARGET, (CString&)m_target);

  DDX_Text(pDX, IDC_CTIME, (CString&)m_locCTime);
  DDX_Text(pDX, IDC_PMTIME, (CString&)m_locPMTime);
  DDX_Text(pDX, IDC_ATIME, (CString&)m_locATime);
  DDX_Text(pDX, IDC_RMTIME, (CString&)m_locRMTime);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_TARGET, m_ex_target);
}

BEGIN_MESSAGE_MAP(CEditShortcutDlg, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

void
CEditShortcutDlg::OnOK() 
{
  ItemListIter listindex;

  UpdateData(TRUE);
  m_group.EmptyIfOnlyWhiteSpace();
  m_title.EmptyIfOnlyWhiteSpace();
  m_username.EmptyIfOnlyWhiteSpace();
  if (m_target.IsOnlyWhiteSpace()) {
    m_target.Empty();
  }

  m_bIsModified |= (m_group != m_ci->GetGroup() ||
    m_title != m_ci->GetTitle() ||
    m_username != m_ci->GetUser());

  bool IsPswdModified = m_target != m_oldtarget;

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    goto dont_close;
  }

  if (m_target.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETARGET);
    ((CEdit*)GetDlgItem(IDC_TARGET))->SetFocus();
    goto dont_close;
  }

  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
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

  bool b_msg_issued;
  if (!dbx->CheckNewPassword(m_group, m_title, m_username, m_target,
    true, CItemData::Shortcut,
    m_base_uuid, m_ibasedata, b_msg_issued)) {
      if (!b_msg_issued)
        AfxMessageBox(IDS_MUSTHAVETARGET, MB_OK);
      UpdateData(FALSE);
      ((CEdit*)GetDlgItem(IDC_TARGET))->SetFocus();
      goto dont_close;
  }
  //End check

  // Everything OK, update fields
  m_ci->SetGroup(m_group);
  m_ci->SetTitle(m_title);
  m_ci->SetUser(m_username.IsEmpty() ? m_defusername : m_username);
  m_ci->SetPassword(m_target);

  time_t t;
  time(&t);

  if (m_bIsModified || IsPswdModified)
    m_ci->SetRMTime(t);

  CPWDialog::OnOK();
  return;
  // If we don't close, then update controls, as some of the fields
  // may have been modified (e.g., spaces removed).
dont_close:
  UpdateData(FALSE);
}

BOOL CEditShortcutDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  CString cs_text;
  if (m_Edit_IsReadOnly) {
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    cs_text.LoadString(IDS_VIEWSHORTCUTS);
    SetWindowText(cs_text);
    cs_text.LoadString(IDS_DATABASEREADONLY);
    GetDlgItem(IDC_EDITEXPLANATION)->SetWindowText(cs_text);
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

  m_target = m_oldtarget = m_base;

  UpdateData(FALSE);
  m_ex_group.ChangeColour();
  return TRUE;
}

void CEditShortcutDlg::OnHelp() 
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

void CEditShortcutDlg::OnBnClickedOk()
{
  OnOK();
}
