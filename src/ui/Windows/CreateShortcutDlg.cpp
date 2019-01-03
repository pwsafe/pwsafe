/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file CCreateShortcutDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "CreateShortcutDlg.h"
#include "ControlExtns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CCreateShortcutDlg::CCreateShortcutDlg(CWnd* pParent, 
  const CSecString &cs_basegroup, const CSecString &cs_basetitle, const CSecString &cs_baseuser)
  : CPWDialog(CCreateShortcutDlg::IDD, pParent),
  m_basegroup(cs_basegroup), m_basetitle(cs_basetitle), m_baseuser(cs_baseuser),
  m_group(cs_basegroup), m_username(cs_baseuser)
{
}

BOOL CCreateShortcutDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  // Get Add/Edit font
  CFont *pFont = Fonts::GetInstance()->GetAddEditFont();

  // Change font size of the group, title & username fields and the base entry name
  m_ex_group.SetFont(pFont);
  m_ex_title.SetFont(pFont);
  m_ex_username.SetFont(pFont);
  GetDlgItem(IDC_MYBASE)->SetFont(pFont);

  // Populate the combo box
  m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)
  std::vector<std::wstring> vGroups;
  app.GetCore()->GetAllGroups(vGroups);
  for (std::vector<std::wstring>::iterator iter = vGroups.begin();
       iter != vGroups.end(); ++iter) {
    m_ex_group.AddString(iter->c_str());
  }

  // Make sure Group combobox is wide enough
  SetGroupComboBoxWidth();

  m_title.Format(IDS_SCTARGET, static_cast<LPCWSTR>(m_basetitle));

  CSecString cs_base(L"");
  cs_base = L"\xab" + m_basegroup + L"\xbb " +
            L"\xab" + m_basetitle + L"\xbb " +
            L"\xab" + m_baseuser  + L"\xbb";
  GetDlgItem(IDC_MYBASE)->SetWindowText(cs_base);

  m_ex_group.ChangeColour();

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CCreateShortcutDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
}

BEGIN_MESSAGE_MAP(CCreateShortcutDlg, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()

void CCreateShortcutDlg::OnCancel() 
{
  CPWDialog::OnCancel();
}

void CCreateShortcutDlg::OnOK() 
{
  if (UpdateData(TRUE) == FALSE)
    return;

  CGeneralMsgBox gmb;
  m_group.EmptyIfOnlyWhiteSpace();
  m_title.EmptyIfOnlyWhiteSpace();
  m_username.EmptyIfOnlyWhiteSpace();

  //Check that data is valid
  if (m_title.IsEmpty()) {
    gmb.AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }

  if (!m_group.IsEmpty() && m_group[0] == '.') {
    gmb.AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    return;
  }

  // If there is a matching entry in our list, tell the user to try again.
  if (GetMainDlg()->Find(m_group, m_title, m_username) != app.GetMainDlg()->End()) {
    gmb.AfxMessageBox(IDS_ENTRYEXISTS, MB_OK | MB_ICONASTERISK);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }
  //End check

  CPWDialog::OnOK();
}

void CCreateShortcutDlg::OnHelp() 
{
  ShowHelp(L"::/html/entering_pwd.html");
}

void CCreateShortcutDlg::SetGroupComboBoxWidth()
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
