// ExportXML.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportXML.h"
#include "PwFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportXMLDlg dialog


CExportXMLDlg::CExportXMLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportXMLDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportXMLDlg)
	m_ExportXMLPassword = _T("");
	m_defexpdelim = _T("^");
	//}}AFX_DATA_INIT
}


BOOL CExportXMLDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();
   SetPasswordFont(GetDlgItem(IDC_EXPORT_XML_PASSWORD));
   return TRUE;
}


void CExportXMLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportXMLDlg)
	DDX_Text(pDX, IDC_EXPORT_XML_PASSWORD, m_ExportXMLPassword);
	DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
	DDV_MaxChars(pDX, m_defexpdelim, 1);
	//}}AFX_DATA_MAP
	DDV_CheckExpDelimiter(pDX, m_defexpdelim);
}

BEGIN_MESSAGE_MAP(CExportXMLDlg, CDialog)
	//{{AFX_MSG_MAP(CExportXMLDlg)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI CExportXMLDlg::DDV_CheckExpDelimiter(CDataExchange* pDX, const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    if (delimiter.IsEmpty()) {
      MessageBox(_T("A delimiter character must be entered!"));
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
// CExportXMLDlg message handlers

void CExportXMLDlg::OnHelp()
{
  ::HtmlHelp(NULL,
	     "pwsafe.chm::/html/export.html",
	     HH_DISPLAY_TOPIC, 0);
}

void CExportXMLDlg::OnOK() 
{
  if(UpdateData(TRUE) != TRUE)
	  return;
  GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);

  CDialog::OnOK();
}
