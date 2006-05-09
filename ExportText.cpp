// ExportText.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportText.h"
#include "PwFont.h"

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
	m_defexpdelim = _T("^");
	m_querysetexpdelim = 0;
	m_export_hdr = 0;
	//}}AFX_DATA_INIT
}


BOOL CExportTextDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
   SetPasswordFont(GetDlgItem(IDC_EXPORT_TEXT_PASSWORD));
   return TRUE;
}


void CExportTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportTextDlg)
	DDX_Text(pDX, IDC_EXPORT_TEXT_PASSWORD, m_exportTextPassword);
	DDX_Check(pDX, IDC_QUERYSETEXPDELIM, m_querysetexpdelim);
	DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
	DDX_Check(pDX, IDC_EXPORT_HDR, m_export_hdr);
	DDV_MaxChars(pDX, m_defexpdelim, 1);
	//}}AFX_DATA_MAP
	if (m_querysetexpdelim == 1)
	  DDV_CheckExpDelimiter(pDX, m_defexpdelim);
}


BEGIN_MESSAGE_MAP(CExportTextDlg, CDialog)
	//{{AFX_MSG_MAP(CExportTextDlg)
	ON_BN_CLICKED(IDC_QUERYSETEXPDELIM, OnSetMultilineExportNotesDelimiter)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI CExportTextDlg::DDV_CheckExpDelimiter(CDataExchange* pDX, const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    if (delimiter.IsEmpty()) {
      MessageBox(_T("If requested, then a delimiter character must be entered!"));
      pDX->Fail();
      return;
    }   
    if (delimiter[0] == '"') {
      MessageBox(_T("As the double quotation character is used to delimit the whole notes field,"
		    "it cannot be used within it to delimit multiple lines."
		    "Please enter another character."));
      pDX->Fail();
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg message handlers

void CExportTextDlg::OnSetMultilineExportNotesDelimiter() 
{
  if (((CButton*)GetDlgItem(IDC_QUERYSETEXPDELIM))->GetCheck() == 1) {
    GetDlgItem(IDC_DEFEXPDELIM)->EnableWindow(TRUE);
    m_querysetexpdelim = 1;
  } else {
    GetDlgItem(IDC_DEFEXPDELIM)->EnableWindow(FALSE);
    m_querysetexpdelim = 0;
  }
}

void CExportTextDlg::OnHelp()
{
  ::HtmlHelp(NULL,
	     "pwsafe.chm::/html/export.html",
	     HH_DISPLAY_TOPIC, 0);
}

void CExportTextDlg::OnOK() 
{
  UpdateData();
  if (m_querysetexpdelim == 1)
    GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);
  CDialog::OnOK();
}
