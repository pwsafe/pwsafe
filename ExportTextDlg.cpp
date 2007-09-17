/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ExportText.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportTextDlg.h"
#include "AdvancedDlg.h"
#include "PwFont.h"
#include "ThisMfcApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg dialog


CExportTextDlg::CExportTextDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CExportTextDlg::IDD, pParent),
    m_subgroup_set(BST_UNCHECKED),
    m_subgroup_name(_T("")), m_subgroup_object(0), m_subgroup_function(0)
{
	//{{AFX_DATA_INIT(CExportTextDlg)
	m_exportTextPassword = _T("");
	m_defexpdelim = _T("\xbb");
	//}}AFX_DATA_INIT
}


BOOL CExportTextDlg::OnInitDialog() 
{
   CPWDialog::OnInitDialog();
   SetPasswordFont(GetDlgItem(IDC_EXPORT_TEXT_PASSWORD));
   ((CEdit*)GetDlgItem(IDC_EXPORT_TEXT_PASSWORD))->SetPasswordChar(PSSWDCHAR);

   m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
   m_subgroup_name.Empty();

   LOGFONT lf1, lf2;
   CFont font1, font2;

   GetDlgItem(IDC_EXPWARNING1)->GetFont()->GetLogFont(&lf1);
   lf1.lfWeight = FW_BOLD;
   font1.CreateFontIndirect(&lf1);
   GetDlgItem(IDC_EXPWARNING1)->SetFont(&font1);

   GetDlgItem(IDC_EXPWARNING2)->GetFont()->GetLogFont(&lf2);
   lf2.lfWeight = FW_BOLD;
   font2.CreateFontIndirect(&lf2);
   GetDlgItem(IDC_EXPWARNING2)->SetFont(&font2);

   return TRUE;
}


void CExportTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CPWDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportTextDlg)
	DDX_Text(pDX, IDC_EXPORT_TEXT_PASSWORD, m_exportTextPassword);
	DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
	DDV_MaxChars(pDX, m_defexpdelim, 1);
	//}}AFX_DATA_MAP
	DDV_CheckExpDelimiter(pDX, m_defexpdelim);
}


BEGIN_MESSAGE_MAP(CExportTextDlg, CPWDialog)
	//{{AFX_MSG_MAP(CExportTextDlg)
	ON_BN_CLICKED(IDC_EXPORT_ADVANCED, OnAdvanced)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI CExportTextDlg::DDV_CheckExpDelimiter(CDataExchange* pDX, const CString &delimiter)
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
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CExportTextDlg message handlers

void CExportTextDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/export.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CExportTextDlg::OnOK() 
{
  if (UpdateData(TRUE) != TRUE)
	  return;

  GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);
  CPWDialog::OnOK();
}

void CExportTextDlg::OnAdvanced()
{
	CAdvancedDlg Adv(this, ADV_EXPORT_TEXT, m_bsExport, m_subgroup_name, 
                   m_subgroup_set, m_subgroup_object, m_subgroup_function);

  app.DisableAccelerator();
  int rc = Adv.DoModal();
  app.EnableAccelerator();

	if (rc == IDOK) {
		m_bsExport = Adv.m_bsFields;
		m_subgroup_set = Adv.m_subgroup_set;
		if (m_subgroup_set == BST_CHECKED) {
		  m_subgroup_name = Adv.m_subgroup_name;
			m_subgroup_object = Adv.m_subgroup_object;
			m_subgroup_function = Adv.m_subgroup_function;
		}	
	}
}
