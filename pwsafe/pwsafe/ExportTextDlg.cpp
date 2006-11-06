/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "ExportTextXDlg.h"
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
   m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
   m_subgroup.Empty();
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
  HtmlHelp(DWORD_PTR(_T("pwsafe.chm::/export.html")), HH_DISPLAY_TOPIC);
}

void CExportTextDlg::OnOK() 
{
  if (UpdateData(TRUE) != TRUE)
	  return;
  if (m_querysetexpdelim == 1)
    GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);
  CDialog::OnOK();
}

void CExportTextDlg::OnAdvanced()
{
	CExportTextXDlg etx;
	int rc = etx.DoModal();
	if (rc == IDOK) {
		m_bsExport = etx.m_bsExport;
		m_subgroup = etx.m_export_subgroup_name;
		if (etx.m_export_subgroup == 1) {
			m_subgroup_object = etx.m_subgroup_object;
			m_subgroup_function = etx.m_subgroup_function;
		}	
	}
}
