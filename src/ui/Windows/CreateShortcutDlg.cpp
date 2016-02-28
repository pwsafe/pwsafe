/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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
  const CSecString &cs_tg, const CSecString &cs_tt, const CSecString &cs_tu)
  : CPWDialog(CCreateShortcutDlg::IDD, pParent),
  m_tg(cs_tg), m_tt(cs_tt), m_tu(cs_tu), m_group(cs_tg), m_username(cs_tu)
{
}

BOOL CCreateShortcutDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  // Populate the combo box
  m_ex_group.ResetContent(); // groups might be from a previous DB (BR 3062758)
  std::vector<std::wstring> aryGroups;
  app.GetCore()->GetUniqueGroups(aryGroups);
  for (std::vector<std::wstring>::iterator iter = aryGroups.begin();
       iter != aryGroups.end(); ++iter) {
    m_ex_group.AddString(iter->c_str());
  }

  // Make sure Group combobox is wide enough
  SetGroupComboBoxWidth();

  m_title.Format(IDS_SCTARGET, m_tt);

  CSecString cs_target(L"");
  if (!m_tg.IsEmpty())
    cs_target = m_tg + L".";
  cs_target += m_tt;
  if (!m_tu.IsEmpty())
    cs_target += L"." + m_tu;
  GetDlgItem(IDC_MYBASE)->SetWindowText(cs_target);

  m_ex_group.ChangeColour();

  UpdateData(FALSE);

  return TRUE;
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
    CSecString temp;
    if (m_group.IsEmpty())
      temp.Format(IDS_ENTRYEXISTS2, m_title, m_username);
    else
      temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
    gmb.AfxMessageBox(temp);
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
