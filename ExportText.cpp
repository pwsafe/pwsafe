// ExportText.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg dialog


CExportTextDlg::CExportTextDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportTextDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportTextDlg)
	m_exportTextPassword = _T("");
	//}}AFX_DATA_INIT
}


void CExportTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportTextDlg)
	DDX_Text(pDX, IDC_EXPORT_TEXT_PASSWORD, m_exportTextPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportTextDlg, CDialog)
	//{{AFX_MSG_MAP(CExportTextDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg message handlers

void CExportTextDlg::OnOK() 
{
  // just update the password for checking
  UpdateData();
  CDialog::OnOK();
}
