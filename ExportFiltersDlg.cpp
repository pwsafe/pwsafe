/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ExportFilters.cpp : implementation file
//

#include "stdafx.h"
#include "ExportFiltersDlg.h"
#include "PWDialog.h"

// ExportFilters dialog

IMPLEMENT_DYNAMIC(CExportFiltersDlg, CPWDialog)

CExportFiltersDlg::CExportFiltersDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CExportFiltersDlg::IDD, pParent),
  m_bDB(false), m_bGlobal(false), m_selectedstore(1)
{
}

CExportFiltersDlg::~CExportFiltersDlg()
{
}

void CExportFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_DATABASEFILTERSBTN, m_selectedstore); // only first!
}

BEGIN_MESSAGE_MAP(CExportFiltersDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, OnExport)
END_MESSAGE_MAP()

// ExportFilters message handlers

BOOL CExportFiltersDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  if (!m_bDB) {
    m_selectedstore = 1;
    GetDlgItem(IDC_DATABASEFILTERSBTN)->EnableWindow(FALSE);
  }

  if (!m_bGlobal) {
    m_selectedstore = 0;
    GetDlgItem(IDC_GLOBALFILTERBTN)->EnableWindow(FALSE);
  }

  UpdateData(FALSE);

  return TRUE;
}

void CExportFiltersDlg::OnExport()
{
  CPWDialog::OnOK();
}
