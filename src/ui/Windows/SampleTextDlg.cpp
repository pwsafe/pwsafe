/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SampleTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeneralMsgBox.h"
#include "SampleTextDlg.h"
#include "ControlExtns.h"

#include "resource.h"
#include "resource3.h"

// SampleTextDlg dialog

IMPLEMENT_DYNAMIC(CSampleTextDlg, CPWDialog)

CSampleTextDlg::CSampleTextDlg(CWnd* pParent, CString sampletext)
  : CPWDialog(CSampleTextDlg::IDD, pParent), m_sampletext(sampletext)
{
}

CSampleTextDlg::~CSampleTextDlg()
{
}

BOOL CSampleTextDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

  GotoDlgCtrl(GetDlgItem(IDC_SAMPLETEXT));

  return FALSE;
}

void CSampleTextDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_SAMPLETEXT, m_sampletext);
  DDX_Control(pDX, IDC_SAMPLETEXT, m_ex_sampletext);
}

BEGIN_MESSAGE_MAP(CSampleTextDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

void CSampleTextDlg::OnOK()
{
  UpdateData(TRUE);
  if (m_sampletext.IsEmpty()) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_EMPTYSAMPLETEXT);
    ((CEdit*)GetDlgItem(IDC_SAMPLETEXT))->SetFocus();
    return;
  }

  CPWDialog::OnOK();
}
