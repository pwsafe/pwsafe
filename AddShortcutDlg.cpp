/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddShortcutDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "AddShortcutDlg.h"
#include "ControlExtns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CAddShortcutDlg::CAddShortcutDlg(CWnd* pParent)
  : CPWDialog(CAddShortcutDlg::IDD, pParent), m_target(_T("")), 
  m_username(_T("")), m_title(_T("")), m_group(_T(""))
{
}

BOOL CAddShortcutDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  UpdateData(FALSE);

  // Populate the combo box
  if(m_ex_group.GetCount() == 0) {
    CStringArray aryGroups;
    app.m_core.GetUniqueGroups(aryGroups);
    for(int igrp = 0; igrp < aryGroups.GetSize(); igrp++) {
      m_ex_group.AddString((LPCTSTR)aryGroups[igrp]);
    }
  }

  m_ex_group.ChangeColour();
  return TRUE;
}

void CAddShortcutDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
  DDX_Text(pDX, IDC_TARGET, (CString&)m_target);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_TARGET, m_ex_target);
}

BEGIN_MESSAGE_MAP(CAddShortcutDlg, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

void CAddShortcutDlg::OnCancel() 
{
  CPWDialog::OnCancel();
}

void CAddShortcutDlg::OnOK() 
{
  if (UpdateData(TRUE) != TRUE)
    return;

  m_group.EmptyIfOnlyWhiteSpace();
  m_title.EmptyIfOnlyWhiteSpace();
  m_username.EmptyIfOnlyWhiteSpace();
  if (m_target.IsOnlyWhiteSpace()) {
    m_target.Empty();
  }

  UpdateData(FALSE);

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }

  if (m_target.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETARGET);
    ((CEdit*)GetDlgItem(IDC_TARGET))->SetFocus();
    return;
  }

  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    return;
  }

  DboxMain* pDbx = static_cast<DboxMain *>(GetParent());
  ASSERT(pDbx != NULL);

  // If there is a matching entry in our list, tell the user to try again.
  if (pDbx->Find(m_group, m_title, m_username) != pDbx->End()) {
    CMyString temp;
    if (m_group.IsEmpty())
      temp.Format(IDS_ENTRYEXISTS2, m_title, m_username);
    else
      temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
    AfxMessageBox(temp);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }

  bool b_msg_issued;
  if (!pDbx->CheckNewPassword(m_group, m_title, m_username, m_target,
                             false, CItemData::Shortcut,
                             m_base_uuid, m_ibasedata, b_msg_issued)) {
    if (!b_msg_issued)
      AfxMessageBox(IDS_MUSTHAVETARGET, MB_OK);
    UpdateData(FALSE);
    ((CEdit*)GetDlgItem(IDC_TARGET))->SetFocus();
    return;
  }
  //End check

  CPWDialog::OnOK();
}

void CAddShortcutDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/entering_pwd.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

void CAddShortcutDlg::OnBnClickedOk()
{
  OnOK();
}
