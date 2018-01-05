/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ImportTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ImportTextDlg.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportTextDlg dialog
CImportTextDlg::CImportTextDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CImportTextDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CImportTextDlg)
  m_groupName.LoadString(IDS_IMPORTED);
  m_Separator = L"|";
  m_defimpdelim = L"\xbb";
  m_tab = 0;
  m_group = 0;
  m_bImportPSWDsOnly = FALSE;
  //}}AFX_DATA_INIT
}

void CImportTextDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CImportTextDlg)
  DDX_Text(pDX, IDC_GROUP_NAME, m_groupName);
  DDX_Text(pDX, IDC_OTHER_SEPARATOR, m_Separator);
  DDV_MaxChars(pDX, m_Separator, 1);
  DDX_Radio(pDX, IDC_TAB, m_tab);
  DDX_Radio(pDX, IDC_NO_GROUP, m_group);
  DDX_Text(pDX, IDC_DEFIMPDELIM, m_defimpdelim);
  DDV_MaxChars(pDX, m_defimpdelim, 1);
  DDX_Check(pDX, IDC_IMPORT_PSWDS_ONLY, m_bImportPSWDsOnly);
  //}}AFX_DATA_MAP
  DDV_CheckImpDelimiter(pDX, m_defimpdelim);
}

BEGIN_MESSAGE_MAP(CImportTextDlg, CPWDialog)
  //{{AFX_MSG_MAP(CImportTextDlg)
  ON_BN_CLICKED(IDC_OTHER, OnOther)
  ON_BN_CLICKED(IDC_COMMA, OnComma)
  ON_BN_CLICKED(IDC_TAB, OnTab)
  ON_BN_CLICKED(IDC_NO_GROUP, OnNoGroup)
  ON_BN_CLICKED(IDC_YES_GROUP, OnYesGroup)
  ON_BN_CLICKED(IDC_IMPORT_PSWDS_ONLY, OnImportPSWDsOnly)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Public functions

void AFXAPI DDV_CheckImpDelimiter(CDataExchange* pDX, const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    CGeneralMsgBox gmb;
    if (delimiter.IsEmpty()) {
      gmb.AfxMessageBox(IDS_NEEDDELIMITER);
      pDX->Fail();
      return;
    }

    if (delimiter[0] == '"') {
      gmb.AfxMessageBox(IDS_INVALIDDELIMITER);
      pDX->Fail();
      return;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CImportTextDlg message handlers

void CImportTextDlg::OnOther() 
{
  GetDlgItem(IDC_OTHER_SEPARATOR)->EnableWindow(TRUE);
  m_tab = 2;
}

void CImportTextDlg::OnComma() 
{
  GetDlgItem(IDC_OTHER_SEPARATOR)->EnableWindow(FALSE);
  m_tab = 1;
}

void CImportTextDlg::OnTab() 
{
  GetDlgItem(IDC_OTHER_SEPARATOR)->EnableWindow(FALSE);
  m_tab = 0;
}

void CImportTextDlg::OnNoGroup() 
{
  GetDlgItem(IDC_GROUP_NAME)->EnableWindow(FALSE);
  m_group = 0;
}

void CImportTextDlg::OnYesGroup() 
{
  GetDlgItem(IDC_GROUP_NAME)->EnableWindow(TRUE);
  m_group = 1;
}

void CImportTextDlg::OnImportPSWDsOnly()
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

void CImportTextDlg::OnHelp() 
{
  ShowHelp(L"::/html/import.html#text");
}

void CImportTextDlg::OnOK() 
{
  UpdateData(TRUE);

  switch (m_tab) {
    case 0: m_Separator = L"\t"; break;
    case 1: m_Separator = L","; break;
    case 2: break; // m_Separator will get value from edit control
    default: 
      ASSERT(0);
      break;
  }

  if (m_group == 0) {
    m_groupName = L"";
  }

  UpdateData(FALSE);
  CPWDialog::OnOK();
}
