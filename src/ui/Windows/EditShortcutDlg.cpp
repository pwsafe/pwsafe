/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "EditShortcutDlg.h"
#include "ControlExtns.h"

#include "core/PWSprefs.h"
#include "core/ItemData.h"

#include <shlwapi.h>
#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEditShortcutDlg::CEditShortcutDlg(CItemData *pci, CWnd* pParent,
  const CSecString &cs_tg, const CSecString &cs_tt, const CSecString &cs_tu)
  : CPWDialog(CEditShortcutDlg::IDD, pParent),
  m_tg(cs_tg), m_tt(cs_tt), m_tu(cs_tu), m_group(cs_tg),
  m_pci(pci), m_bIsModified(false), m_Edit_IsReadOnly(false)
{
  ASSERT(pci != NULL);

  m_group = pci->GetGroup();
  m_title = pci->GetTitle();
  m_username = pci->GetUser();

  m_locCTime = pci->GetCTimeL();
  m_locPMTime = pci->GetPMTimeL();
  m_locATime = pci->GetATimeL();
  m_locRMTime = pci->GetRMTimeL();
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
  CSecString cs_target(L"\xab");
  // Leave \xab \xbb between group/title/user even if group or user is empty
  // so that the user knows the exact name. Looks similar to: <g><t><u>
  // Note: the title field is mandatory and never empty.
  if (m_tg.IsEmpty())
    cs_target += L" ";
  else
    cs_target += m_tg;

  cs_target += L"\xbb \xab" + m_tt + L"\xbb \xab";

  if (m_tu.IsEmpty())
    cs_target += L" ";
  else
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
    m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)
    m_ex_group.AddString(m_group);
    // Set fields to be read-only
    GetDlgItem(IDC_GROUP)->EnableWindow(FALSE);
    m_ex_title.EnableWindow(FALSE);
    m_ex_username.EnableWindow(FALSE);
  } else { // !read-only
    // Populate the groups combo box
    m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)
    std::vector<std::wstring> aryGroups;
    app.GetCore()->GetUniqueGroups(aryGroups);
    for (std::vector<std::wstring>::iterator iter = aryGroups.begin();
         iter != aryGroups.end(); ++iter) {
      m_ex_group.AddString(iter->c_str());
    }
  } // !read-only

  // Show base entry
  GetDlgItem(IDC_MYBASE)->SetWindowText(cs_target);

  UpdateData(FALSE);
  m_ex_group.ChangeColour();
  return TRUE;
}

void CEditShortcutDlg::OnHelp() 
{
  ShowHelp(L"::/html/entering_pwd.html");
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

  m_bIsModified |= (m_group != m_pci->GetGroup() ||
                    m_title != m_pci->GetTitle() ||
                    m_username != m_pci->GetUser());

  //Check that data is valid
  if (m_title.IsEmpty()) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    goto dont_close;
  }

  if (!m_group.IsEmpty() && m_group[0] == '.') {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    goto dont_close;
  }

  listindex = GetMainDlg()->Find(m_group, m_title, m_username);
  /*
  *  If there is a matching entry in our list, and that
  *  entry is not the same one we started editing, tell the
  *  user to try again.
  */
  if (listindex != GetMainDlg()->End()) {
    const CItemData &listItem = GetMainDlg()->GetEntryAt(listindex);
    bool notSame = listItem.GetUUID() != m_pci->GetUUID();
    if (notSame) {
      CGeneralMsgBox gmb;
      CSecString temp;
      temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
      gmb.AfxMessageBox(temp);
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
      ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
      goto dont_close;
    }
  }
  //End check

  // Everything OK, update fields
  m_pci->SetGroup(m_group);
  m_pci->SetTitle(m_title);
  m_pci->SetUser(m_username.IsEmpty() ? m_defusername : m_username);

  time_t t;
  time(&t);

  if (m_bIsModified)
    m_pci->SetRMTime(t);

  CPWDialog::OnOK();
  return;

  // If we don't close, then update controls, as some of the fields
  // may have been modified (e.g., spaces removed).
dont_close:
  UpdateData(FALSE);
}
