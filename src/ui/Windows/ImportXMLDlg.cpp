/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ImportXMLDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ImportXMLDlg.h"
#include "ThisMfcApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportXMLDlg dialog

CImportXMLDlg::CImportXMLDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CImportXMLDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CImportXMLDlg)
  m_groupName.LoadString(IDS_IMPORTED);
  m_group = 0;
  m_bImportPSWDsOnly = FALSE;
  //}}AFX_DATA_INIT
}

void CImportXMLDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CImportXMLDlg)
  DDX_Radio(pDX, IDC_NO_GROUP, m_group);
  DDX_Text(pDX, IDC_GROUP_NAME, m_groupName);
  DDX_Check(pDX, IDC_IMPORT_PSWDS_ONLY, m_bImportPSWDsOnly);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportXMLDlg, CPWDialog)
  //{{AFX_MSG_MAP(CImportXMLDlg)
  ON_BN_CLICKED(IDC_NO_GROUP, OnNoGroup)
  ON_BN_CLICKED(IDC_YES_GROUP, OnYesGroup)
  ON_BN_CLICKED(IDC_IMPORT_PSWDS_ONLY, OnImportPSWDsOnly)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportXMLDlg message handlers

void CImportXMLDlg::OnNoGroup() 
{
  GetDlgItem(IDC_GROUP_NAME)->EnableWindow(FALSE);
  m_group=0;
}

void CImportXMLDlg::OnYesGroup() 
{
  GetDlgItem(IDC_GROUP_NAME)->EnableWindow(TRUE);
  m_group=1;
}

void CImportXMLDlg::OnImportPSWDsOnly()
{
  m_bImportPSWDsOnly = ((CButton*)GetDlgItem(IDC_IMPORT_PSWDS_ONLY))->GetCheck();

  BOOL bEnable = (m_bImportPSWDsOnly == BST_CHECKED) ? FALSE : TRUE;
  GetDlgItem(IDC_NO_GROUP)->EnableWindow(bEnable);
  GetDlgItem(IDC_YES_GROUP)->EnableWindow(bEnable);
  if (bEnable == FALSE)
    GetDlgItem(IDC_GROUP_NAME)->EnableWindow(bEnable);
  else {
    GetDlgItem(IDC_GROUP_NAME)->EnableWindow(m_group == 1 ? TRUE : FALSE);
  }
}

void CImportXMLDlg::OnHelp() 
{
  ShowHelp(L"::/html/import.html#XML");
}

void CImportXMLDlg::OnOK() 
{
  if (m_group == 0) {
    m_groupName = L"";
    UpdateData(FALSE);
  } else {
    GetDlgItemText(IDC_GROUP_NAME,m_groupName);
    UpdateData(FALSE);
  }

  CPWDialog::OnOK();
}
