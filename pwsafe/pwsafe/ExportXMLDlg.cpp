/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ExportXML.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportXMLDlg.h"
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
// CExportXMLDlg message handlers

void CExportXMLDlg::OnHelp()
{
  HtmlHelp(DWORD_PTR(_T("pwsafe.chm::/export.html")), HH_DISPLAY_TOPIC);
}

void CExportXMLDlg::OnOK() 
{
  if(UpdateData(TRUE) != TRUE)
	  return;
  GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);

  CDialog::OnOK();
}
