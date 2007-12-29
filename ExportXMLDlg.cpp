/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// ExportXML.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportXMLDlg.h"
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
// CExportXMLDlg dialog


CExportXMLDlg::CExportXMLDlg(CWnd* pParent /*=NULL*/)
	: CPWDialog(CExportXMLDlg::IDD, pParent),
    m_subgroup_set(BST_UNCHECKED),
    m_subgroup_name(_T("")), m_subgroup_object(0), m_subgroup_function(0)
{
	//{{AFX_DATA_INIT(CExportXMLDlg)
	m_ExportXMLPassword = _T("");
	m_defexpdelim = _T("\xbb");
	//}}AFX_DATA_INIT
}


BOOL CExportXMLDlg::OnInitDialog() 
{
   CPWDialog::OnInitDialog();

   m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
   m_subgroup_name.Empty();

   ApplyPasswordFont(GetDlgItem(IDC_EXPORT_XML_PASSWORD));
   ((CEdit*)GetDlgItem(IDC_EXPORT_XML_PASSWORD))->SetPasswordChar(PSSWDCHAR);
   return TRUE;
}


void CExportXMLDlg::DoDataExchange(CDataExchange* pDX)
{
	CPWDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportXMLDlg)
	DDX_Text(pDX, IDC_EXPORT_XML_PASSWORD, m_ExportXMLPassword);
	DDX_Text(pDX, IDC_DEFEXPDELIM, m_defexpdelim);
	DDV_MaxChars(pDX, m_defexpdelim, 1);
	//}}AFX_DATA_MAP
	DDV_CheckExpDelimiter(pDX, m_defexpdelim);
}

BEGIN_MESSAGE_MAP(CExportXMLDlg, CPWDialog)
	//{{AFX_MSG_MAP(CExportXMLDlg)
  ON_BN_CLICKED(IDC_XML_ADVANCED, OnAdvanced)
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
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/export.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CExportXMLDlg::OnOK() 
{
  if(UpdateData(TRUE) != TRUE)
	  return;
  GetDlgItemText(IDC_DEFEXPDELIM, m_defexpdelim);

  CPWDialog::OnOK();
}

void CExportXMLDlg::OnAdvanced()
{
	CAdvancedDlg Adv(this, ADV_EXPORT_XML, m_bsExport, m_subgroup_name, 
                   m_subgroup_set, m_subgroup_object, m_subgroup_function);

  app.DisableAccelerator();
  INT_PTR rc = Adv.DoModal();
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
