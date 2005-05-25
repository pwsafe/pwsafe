// ExportTextSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportTextSettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportTextSettingsDlg dialog


CExportTextSettingsDlg::CExportTextSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportTextSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportTextSettingsDlg)
	m_defexpdelim = _T("^");
	m_querysetexpdelim = 0;
	//}}AFX_DATA_INIT
}


void CExportTextSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportTextSettingsDlg)
	DDX_Check(pDX, IDC_QUERYSETEXPDELIM, m_querysetexpdelim);
	DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
	DDV_MaxChars(pDX, m_defexpdelim, 1);
	//}}AFX_DATA_MAP
	if (m_querysetexpdelim == 1)
		DDV_CheckExpDelimiter(pDX, m_defexpdelim);
}


BEGIN_MESSAGE_MAP(CExportTextSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CExportTextSettingsDlg)
	ON_BN_CLICKED(IDC_QUERYSETEXPDELIM, OnSetMultilineExportNotesDelimiter)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Public functions

void AFXAPI DDV_CheckExpDelimiter(CDataExchange* pDX, CString delimiter)
{
	if (pDX->m_bSaveAndValidate)
	{
		if (delimiter.IsEmpty())
		{
			AfxMessageBox("If requested, then a delimiter character must be entered!");
			pDX->Fail();
			return;
		}

		if (delimiter[0] == '"')
		{
			AfxMessageBox("As the double quotation character is used to delimit the whole notes field, it cannot be used within it to delimit multiple lines. Please enter another character.");
			pDX->Fail();
			return;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CExportTextSettingsDlg message handlers

void CExportTextSettingsDlg::OnSetMultilineExportNotesDelimiter() 
{
   if (((CButton*)GetDlgItem(IDC_QUERYSETEXPDELIM))->GetCheck() == 1)
   {
      GetDlgItem(IDC_DEFEXPDELIM)->EnableWindow(TRUE);
	  m_querysetexpdelim = 1;
   }
   else
   {
      GetDlgItem(IDC_DEFEXPDELIM)->EnableWindow(FALSE);
	  m_querysetexpdelim = 0;
   }
}

void CExportTextSettingsDlg::OnHelp() 
{
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/export.html",
              HH_DISPLAY_TOPIC, 0);
}

void CExportTextSettingsDlg::OnOK() 
{
	if (m_querysetexpdelim == 1) {
	  GetDlgItemText(IDC_DEFEXPDELIM,m_defexpdelim);
	}

  CDialog::OnOK();
}
