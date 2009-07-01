/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterActionsDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "FilterActionsDlg.h"
#include "../PWDialog.h"
#include "ComboAdder.h"

// FilterActionsDlg dialog

IMPLEMENT_DYNAMIC(CFilterActionsDlg, CPWDialog)

CFilterActionsDlg::CFilterActionsDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CFilterActionsDlg::IDD, pParent),
  m_function(FA_SELECT)
{
}

CFilterActionsDlg::~CFilterActionsDlg()
{
}

void CFilterActionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FILTERNAMECOMBO, m_combo);
}

BEGIN_MESSAGE_MAP(CFilterActionsDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, OnExecute)
END_MESSAGE_MAP()

// FilterActions message handlers

BOOL CFilterActionsDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString cs_title(L""), cs_button(L"");
  UINT idTitle(0), idButton(0);
  switch (m_function) {
    case FA_DELETE:
      idTitle = IDS_DELETEFROMSTORE;
      idButton = IDS_DELETE;
      break;
    case FA_SELECT:
      idTitle = IDS_SELECTFROMSTORE;
      idButton = IDS_SELECT;
      break;
    default:
      break;      
  }
  cs_title.LoadString(idTitle);
  cs_button.LoadString(idButton);
  SetWindowText(cs_title);
  GetDlgItem(IDOK)->SetWindowText(cs_button);

  if (m_vcs_db.empty()) {
    GetDlgItem(IDC_DATABASEFILTERSBTN)->EnableWindow(FALSE);
  }

  if (m_combo.GetCount() == 0) {
    ComboAdder ca(m_combo);
      ca.doit(m_vcs_db);
  }

  m_combo.SetCurSel(0);
  UpdateData(FALSE);
  return TRUE;
}

void CFilterActionsDlg::OnExecute()
{
  int isel = m_combo.GetCurSel();
  if (isel != -1) {
    m_combo.GetLBText(isel, m_selected);
  } else {
    m_selected = L"";
  }

  CPWDialog::OnOK();
}
