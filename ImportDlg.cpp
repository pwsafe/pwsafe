// ImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ImportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportDlg dialog


CImportDlg::CImportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportDlg)
	m_groupName = _T("Imported");
	m_Separator = _T("|");
	m_tab = 0;
	m_group = 0;
	//}}AFX_DATA_INIT
}


void CImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportDlg)
	DDX_Text(pDX, IDC_GROUP_NAME, m_groupName);
	DDX_Text(pDX, IDC_OTHER_SEPARATOR, m_Separator);
	DDV_MaxChars(pDX, m_Separator, 1);
	DDX_Radio(pDX, IDC_TAB, m_tab);
	DDX_Radio(pDX, IDC_NO_GROUP, m_group);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportDlg, CDialog)
	//{{AFX_MSG_MAP(CImportDlg)
	ON_BN_CLICKED(IDC_OTHER, OnOther)
	ON_BN_CLICKED(IDC_COMMA, OnComma)
	ON_BN_CLICKED(IDC_TAB, OnTab)
	ON_BN_CLICKED(IDC_NO_GROUP, OnNoGroup)
	ON_BN_CLICKED(IDC_YES_GROUP, OnYesGroup)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportDlg message handlers

void CImportDlg::OnOther() 
{
  GetDlgItem(IDC_OTHER_SEPARATOR)->EnableWindow(TRUE);
  m_tab=2;
}

void CImportDlg::OnComma() 
{
  GetDlgItem(IDC_OTHER_SEPARATOR)->EnableWindow(FALSE);
  m_tab=1;
}

void CImportDlg::OnTab() 
{
  GetDlgItem(IDC_OTHER_SEPARATOR)->EnableWindow(FALSE);
  m_tab=0;
}

void CImportDlg::OnNoGroup() 
{
  GetDlgItem(IDC_GROUP_NAME)->EnableWindow(FALSE);
  m_group=0;
}

void CImportDlg::OnYesGroup() 
{
  GetDlgItem(IDC_GROUP_NAME)->EnableWindow(TRUE);
  m_group=1;
}

void CImportDlg::OnHelp() 
{
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/import.html",
              HH_DISPLAY_TOPIC, 0);
}

void CImportDlg::OnOK() 
{
  switch (m_tab) {
    case 0: m_Separator = _T("\t"); break;
    case 1: m_Separator = _T(","); break;
    case 2: GetDlgItemText(IDC_OTHER_SEPARATOR,m_Separator); break;
    default: break; // m_Separator will get value from edit control
  }

  if (m_group == 0) {
    m_groupName = _T("");
    UpdateData(FALSE);
  } else {
    GetDlgItemText(IDC_GROUP_NAME,m_groupName);
    UpdateData(FALSE);
  }
  
  CDialog::OnOK();
}
