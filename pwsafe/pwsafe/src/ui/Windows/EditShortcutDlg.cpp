/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

CEditShortcutDlg::CEditShortcutDlg(CItemData *ci, CWnd* pParent,
  const CSecString &cs_tg, const CSecString &cs_tt, const CSecString &cs_tu)
  : CPWDialog(CEditShortcutDlg::IDD, pParent),
  m_tg(cs_tg), m_tt(cs_tt), m_tu(cs_tu), m_group(cs_tg),
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

  DDX_Text(pDX, IDC_CTIME, (CString&)m_locCTime);
  DDX_Text(pDX, IDC_PMTIME, (CString&)m_locPMTime);
  DDX_Text(pDX, IDC_ATIME, (CString&)m_locATime);
  DDX_Text(pDX, IDC_RMTIME, (CString&)m_locRMTime);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
}

BEGIN_MESSAGE_MAP(CEditShortcutDlg, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

BOOL CEditShortcutDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  CString cs_text;
  CSecString cs_explanation, cs_target(L"\xbb");
  // Leave \xbb between group/title/user even if group or user is empty
  // so that the user knows the exact name
  // Note: the title field is mandatory and never empty.
  if (!m_tg.IsEmpty())
    cs_target += m_tg;

  cs_target += L"\xbb" + m_tt + L"\xbb";

  if (!m_tu.IsEmpty())
    cs_target += m_tu;

  cs_target += L"\xbb";

  if (m_Edit_IsReadOnly) {
    // Hide OK btton
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
    // Set Cancel button to read 'Close'
    cs_text.LoadString(IDS_CLOSE);
    GetDlgItem(IDCANCEL)->SetWindowText(cs_text);
    // Set Window caption to indicate View rather than Edit
    cs_text.LoadString(IDS_VIEWSHORTCUTS);
    SetWindowText(cs_text);
    // Only add this shortcut's group to combo box
    if (m_ex_group.GetCount() == 0) {
      m_ex_group.AddString(m_group);
    }
    // Set fields to be read-only
    GetDlgItem(IDC_GROUP)->EnableWindow(FALSE);
    m_ex_title.EnableWindow(FALSE);
    m_ex_username.EnableWindow(FALSE);
    // setup explanatory text
    cs_explanation.Format(IDS_SHORTCUTROEXPLANATION, cs_target);
  } else { // !read-only
    // Populate the groups combo box
    if (m_ex_group.GetCount() == 0) {
      std::vector<std::wstring> aryGroups;
      app.m_core.GetUniqueGroups(aryGroups);
      for (size_t igrp = 0; igrp < aryGroups.size(); igrp++) {
        m_ex_group.AddString(aryGroups[igrp].c_str());
      }
    } // group combo-box handling
    // setup explanatory text
    cs_explanation.Format(IDS_SHORTCUTEXPLANATION, cs_target);
  } // !read-only

  // Show explanatory text
  GetDlgItem(IDC_EDITSCEXPLANATION)->SetWindowText(cs_explanation);

  UpdateData(FALSE);
  m_ex_group.ChangeColour();
  return TRUE;
}

void CEditShortcutDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess(L"PegHelp.exe", L"pws_ce_help.html#editview", 
                NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/entering_pwd.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

void CEditShortcutDlg::OnOK() 
{
  ItemListIter listindex;

  if (m_Edit_IsReadOnly) {
    CPWDialog::OnOK();
    return;
  }

  UpdateData(TRUE);
  m_group.EmptyIfOnlyWhiteSpace();
  m_title.EmptyIfOnlyWhiteSpace();
  m_username.EmptyIfOnlyWhiteSpace();

  m_bIsModified |= (m_group != m_ci->GetGroup() ||
                    m_title != m_ci->GetTitle() ||
                    m_username != m_ci->GetUser());

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    goto dont_close;
  }

  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    goto dont_close;
  }

  DboxMain* pDbx = static_cast<DboxMain *>(GetParent());
  ASSERT(pDbx != NULL);

  listindex = pDbx->Find(m_group, m_title, m_username);
  /*
  *  If there is a matching entry in our list, and that
  *  entry is not the same one we started editing, tell the
  *  user to try again.
  */
  if (listindex != pDbx->End()) {
    const CItemData &listItem = pDbx->GetEntryAt(listindex);
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
  //End check

  // Everything OK, update fields
  m_ci->SetGroup(m_group);
  m_ci->SetTitle(m_title);
  m_ci->SetUser(m_username.IsEmpty() ? m_defusername : m_username);

  time_t t;
  time(&t);

  if (m_bIsModified)
    m_ci->SetRMTime(t);

  CPWDialog::OnOK();
  return;

  // If we don't close, then update controls, as some of the fields
  // may have been modified (e.g., spaces removed).
dont_close:
  UpdateData(FALSE);
}
