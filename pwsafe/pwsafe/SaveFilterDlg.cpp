/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SaveFilter.cpp : implementation file
//

#include "stdafx.h"
#include "SaveFilterDlg.h"
#include "PWDialog.h"

// SaveFilter dialog

IMPLEMENT_DYNAMIC(CSaveFilterDlg, CPWDialog)

CSaveFilterDlg::CSaveFilterDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CSaveFilterDlg::IDD, pParent),
  m_selectedstore(0)
{
}

CSaveFilterDlg::~CSaveFilterDlg()
{
}

void CSaveFilterDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_DATABASEFILTERSBTN, m_selectedstore); // only first!
}

BEGIN_MESSAGE_MAP(CSaveFilterDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, OnSave)
END_MESSAGE_MAP()

// SaveFilter message handlers

BOOL CSaveFilterDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  return TRUE;
}

void CSaveFilterDlg::OnSave()
{
  //m_selectedstore = ((CButton *)GetDlgItem(IDC_DATABASEFILTERSBTN))->GetCheck() 
  //                     == BST_CHECKED ? 0 : 1; 

  CPWDialog::OnOK();
}
