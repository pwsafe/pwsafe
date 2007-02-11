/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ImportDlg.h"
#include "ThisMfcApp.h"

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
	m_groupName.LoadString(IDS_IMPORTED);
	m_Separator = _T("|");
	m_defimpdelim = _T("\xbb");
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
	DDX_Text(pDX, IDC_DEFIMPDELIM, m_defimpdelim);
	DDV_MaxChars(pDX, m_defimpdelim, 1);
	//}}AFX_DATA_MAP
	DDV_CheckImpDelimiter(pDX, m_defimpdelim);
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
// Public functions

void AFXAPI DDV_CheckImpDelimiter(CDataExchange* pDX, const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    if (delimiter.IsEmpty()) {
      AfxMessageBox(IDS_NEEDDELIMITER);
      pDX->Fail();
      return;
    }

    if (delimiter[0] == '"') {
      AfxMessageBox(IDS_INVALIDDELIMITER);
      pDX->Fail();
      return;
    }
  }
}

BOOL CImportDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    LOGFONT lf; 
    CFont font; 
    GetDlgItem(IDC_TITLEROW)->GetFont()->GetLogFont(&lf); 
    lf.lfWeight = FW_BOLD;
    font.CreateFontIndirect(&lf); 
    GetDlgItem(IDC_TITLEROW)->SetFont(&font); 
    return TRUE;
}

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
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/import.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
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
  
  GetDlgItemText(IDC_DEFIMPDELIM,m_defimpdelim);

  CDialog::OnOK();
}
