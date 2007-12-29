// SampleTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SampleTextDlg.h"
#include "resource.h"
#include "resource3.h"
#include "ControlExtns.h"

// SampleTextDlg dialog

IMPLEMENT_DYNAMIC(CSampleTextDlg, CDialog)

CSampleTextDlg::CSampleTextDlg(CWnd* pParent, CString sampletext)
	: CDialog(CSampleTextDlg::IDD, pParent), m_sampletext(sampletext)
{
}

CSampleTextDlg::~CSampleTextDlg()
{
}

BOOL CSampleTextDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  ((CEdit*)GetDlgItem(IDC_SAMPLETEXT))->SetFocus();
  return FALSE;
}

void CSampleTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_SAMPLETEXT, m_sampletext);
  DDX_Control(pDX, IDC_SAMPLETEXT, m_ex_sampletext);
}

BEGIN_MESSAGE_MAP(CSampleTextDlg, CDialog)
  ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

void CSampleTextDlg::OnOK()
{
  UpdateData(TRUE);
  if (m_sampletext.IsEmpty()) {
    AfxMessageBox(IDS_EMPTYSAMPLETEXT);
    ((CEdit*)GetDlgItem(IDC_SAMPLETEXT))->SetFocus();
    return;
  }

  CDialog::OnOK();
}