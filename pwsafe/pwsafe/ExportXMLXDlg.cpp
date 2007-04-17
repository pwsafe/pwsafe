/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ExportXMLXDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportXMLXDlg.h"
#include "corelib/ItemData.h"
#include "ThisMfcApp.h"
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportXMLXDlg dialog


CExportXMLXDlg::CExportXMLXDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportXMLXDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportXMLXDlg)
  // Mandatory
  m_exportxml_title = BST_INDETERMINATE;
	m_exportxml_password = BST_INDETERMINATE;

  // Optional
	m_exportxml_group = BST_CHECKED;
  m_exportxml_user = BST_CHECKED;
	m_exportxml_notes = BST_CHECKED;
	m_exportxml_ctime = BST_CHECKED;
	m_exportxml_pmtime = BST_CHECKED;
	m_exportxml_atime = BST_CHECKED;
	m_exportxml_ltime = BST_CHECKED;
	m_exportxml_rmtime = BST_CHECKED;
	m_exportxml_url = BST_CHECKED;
	m_exportxml_autotype = BST_CHECKED;
	m_exportxml_pwhist = BST_CHECKED;
	m_exportxml_subgroup = 0;
	m_exportxml_subgroup_name = _T("*");
	m_exportxml_subgroup_case = 0;
	m_subgroup_object = -1;
	m_subgroup_function = -1;

	//}}AFX_DATA_INIT
}


BOOL CExportXMLXDlg::OnInitDialog()
{
	 CDialog::OnInitDialog();
	 m_bsExport.set();  // note: impossible to set them all even via the advanced dialog

	int index;
	CString cs_text;

	CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_FUNCTION);
	if(cboSubgroupFunction->GetCount() == 0) {
		cs_text.LoadString(IDS_EQUALS);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_EQUALS);
		cs_text.LoadString(IDS_DOESNOTEQUAL);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTEQUAL);
		cs_text.LoadString(IDS_BEGINSWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_BEGINS);
		cs_text.LoadString(IDS_DOESNOTBEGINSWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTBEGIN);
		cs_text.LoadString(IDS_ENDSWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_ENDS);
		cs_text.LoadString(IDS_DOESNOTENDWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTEND);
		cs_text.LoadString(IDS_CONTAINS);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_CONTAINS);
		cs_text.LoadString(IDS_DOESNOTCONTAIN);
		index = cboSubgroupFunction->AddString(cs_text);
		cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTCONTAIN);
	}
	cboSubgroupFunction->SetCurSel(0);

	CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_OBJECT);
	if(cboSubgroupObject->GetCount() == 0) {
		cs_text.LoadString(IDS_GROUP);
		index = cboSubgroupObject->AddString(cs_text);
		cboSubgroupObject->SetItemData(index, CItemData::SGO_GROUP);
		cs_text.LoadString(IDS_TITLE);
		index = cboSubgroupObject->AddString(cs_text);
		cboSubgroupObject->SetItemData(index, CItemData::SGO_TITLE);
		cs_text.LoadString(IDS_USERNAME);
		index = cboSubgroupObject->AddString(cs_text);
		cboSubgroupObject->SetItemData(index, CItemData::SGO_USER);
		cs_text.LoadString(IDS_GROUPTITLE);
		index = cboSubgroupObject->AddString(cs_text);
		cboSubgroupObject->SetItemData(index, CItemData::SGO_GROUPTITLE);
		cs_text.LoadString(IDS_URL);
		index = cboSubgroupObject->AddString(cs_text);
		cboSubgroupObject->SetItemData(index, CItemData::SGO_URL);
		cs_text.LoadString(IDS_NOTES);
		index = cboSubgroupObject->AddString(cs_text);
		cboSubgroupObject->SetItemData(index, CItemData::SGO_NOTES);
	}
	cboSubgroupObject->SetCurSel(0);

	return TRUE;
}


void CExportXMLXDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportXMLXDlg)
	DDX_Check(pDX, IDC_XMLX_GROUP, m_exportxml_group);
  DDX_Check(pDX, IDC_XMLX_TITLE, m_exportxml_title);
	DDX_Check(pDX, IDC_XMLX_USER, m_exportxml_user);
	DDX_Check(pDX, IDC_XMLX_NOTES, m_exportxml_notes);
	DDX_Check(pDX, IDC_XMLX_PASSWORD, m_exportxml_password);
	DDX_Check(pDX, IDC_XMLX_CTIME, m_exportxml_ctime);
	DDX_Check(pDX, IDC_XMLX_PMTIME, m_exportxml_pmtime);
	DDX_Check(pDX, IDC_XMLX_ATIME, m_exportxml_atime);
	DDX_Check(pDX, IDC_XMLX_LTIME, m_exportxml_ltime);
	DDX_Check(pDX, IDC_XMLX_RMTIME, m_exportxml_rmtime);
	DDX_Check(pDX, IDC_XMLX_URL, m_exportxml_url);
	DDX_Check(pDX, IDC_XMLX_AUTOTYPE, m_exportxml_autotype);
	DDX_Check(pDX, IDC_XMLX_PWHIST, m_exportxml_pwhist);
	DDX_Check(pDX, IDC_XMLX_SUBGROUP, m_exportxml_subgroup);
	DDX_Check(pDX, IDC_XMLX_SUBGROUP_CASE, m_exportxml_subgroup_case);
	DDX_Text(pDX, IDC_XMLX_SUBGROUP_NAME, m_exportxml_subgroup_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportXMLXDlg, CDialog)
	//{{AFX_MSG_MAP(CExportXMLXDlg)
	ON_BN_CLICKED(IDC_XMLX_SETTIMES, OnSetTimes)
	ON_BN_CLICKED(IDC_XMLX_CLEARTIMES, OnClearTimes)
	ON_BN_CLICKED(IDC_XMLX_SUBGROUP, OnSetSubGroup)
	ON_BN_CLICKED(IDC_XMLX_SETALL, OnSetAll)
	ON_BN_CLICKED(IDC_XMLX_CLEARALL, OnClearAll)
  ON_BN_CLICKED(IDC_XMLX_TITLE, OnSetTitle)
  ON_BN_CLICKED(IDC_XMLX_PASSWORD, OnSetPassword)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportXMLXDlg message handlers

void CExportXMLXDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/exportx.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CExportXMLXDlg::OnOK()
{
	UpdateData();
	m_bsExport.reset();

  // Mandatory
  m_bsExport.set(CItemData::TITLE, true);
  m_bsExport.set(CItemData::PASSWORD, true);

  // Optional
	m_bsExport.set(CItemData::GROUP, m_exportxml_group == BST_CHECKED ? true : false);
  m_bsExport.set(CItemData::USER, m_exportxml_user == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::NOTES, m_exportxml_notes == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::PASSWORD, m_exportxml_password == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::CTIME, m_exportxml_ctime == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::PMTIME, m_exportxml_pmtime == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::ATIME, m_exportxml_atime == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::LTIME, m_exportxml_ltime == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::RMTIME, m_exportxml_rmtime == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::URL, m_exportxml_url == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::AUTOTYPE, m_exportxml_autotype == BST_CHECKED ? true : false);
	m_bsExport.set(CItemData::PWHIST, m_exportxml_pwhist == BST_CHECKED ? true : false);

	if (m_bsExport.count() == 0) {
		AfxMessageBox(IDS_NOFIELDSFOREXPORT);
		m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
		return;
	}

	if (m_exportxml_subgroup == BST_CHECKED) {
		GetDlgItemText(IDC_XMLX_SUBGROUP_NAME, m_exportxml_subgroup_name);
		int nObject = ((CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_OBJECT))->GetCurSel();
		if (nObject == CB_ERR) {
			AfxMessageBox(IDS_NOOBJECT);
			m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
			((CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_OBJECT))->SetFocus();
			return;
		}
		int nFunction = ((CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_FUNCTION))->GetCurSel();
		if (nFunction == CB_ERR) {
			AfxMessageBox(IDS_NOFUNCTION);
			m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
			((CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_FUNCTION))->SetFocus();
			return;
		}
		m_subgroup_object = ((CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_OBJECT))->GetItemData(nObject);
		m_subgroup_function = ((CComboBox *)GetDlgItem(IDC_XMLX_SUBGROUP_FUNCTION))->GetItemData(nFunction);
		if (m_exportxml_subgroup_case == BST_CHECKED)
			m_subgroup_function *= (-1);
	}

	if (m_exportxml_subgroup_name == _T("*"))
		m_exportxml_subgroup_name.Empty();

	CDialog::OnOK();
}

void CExportXMLXDlg::OnSetSubGroup()
{
	m_exportxml_subgroup = ((CButton*)GetDlgItem(IDC_XMLX_SUBGROUP))->GetCheck();
	if (m_exportxml_subgroup == 1) {
		GetDlgItem(IDC_XMLX_SUBGROUP_NAME)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_XMLX_SUBGROUP_NAME)->EnableWindow(FALSE);
	}
}

void CExportXMLXDlg::OnSetAll()
{
	((CButton*)GetDlgItem(IDC_XMLX_GROUP))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_XMLX_TITLE))->SetCheck(BST_INDETERMINATE);
	((CButton*)GetDlgItem(IDC_XMLX_USER))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_NOTES))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PASSWORD))->SetCheck(BST_INDETERMINATE);
	((CButton*)GetDlgItem(IDC_XMLX_CTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_ATIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_LTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_RMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_URL))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_AUTOTYPE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PWHIST))->SetCheck(BST_CHECKED);

  // Mandatory
  m_exportxml_title = m_exportxml_password = BST_INDETERMINATE;

  // Optional
	m_exportxml_group = m_exportxml_user = m_exportxml_notes = m_exportxml_ctime = 
    m_exportxml_pmtime = m_exportxml_atime = m_exportxml_ltime = m_exportxml_rmtime =
    m_exportxml_url = m_exportxml_autotype = m_exportxml_pwhist = 
    m_exportxml_subgroup = BST_CHECKED;
}

void CExportXMLXDlg::OnClearAll()
{
	((CButton*)GetDlgItem(IDC_XMLX_GROUP))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_XMLX_TITLE))->SetCheck(BST_INDETERMINATE);
	((CButton*)GetDlgItem(IDC_XMLX_USER))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_NOTES))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PASSWORD))->SetCheck(BST_INDETERMINATE);
	((CButton*)GetDlgItem(IDC_XMLX_CTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_ATIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_LTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_RMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_URL))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_AUTOTYPE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PWHIST))->SetCheck(BST_UNCHECKED);

  // Mandatory
  m_exportxml_title = m_exportxml_password = BST_INDETERMINATE;

  // Optional
	m_exportxml_group = m_exportxml_user = m_exportxml_notes = m_exportxml_ctime =
    m_exportxml_pmtime = m_exportxml_atime = m_exportxml_ltime = m_exportxml_rmtime = 
    m_exportxml_url = m_exportxml_autotype = m_exportxml_pwhist = 
    m_exportxml_subgroup = BST_UNCHECKED;
}

void CExportXMLXDlg::OnSetTimes()
{
	((CButton*)GetDlgItem(IDC_XMLX_CTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_ATIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_LTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_RMTIME))->SetCheck(BST_CHECKED);
	m_exportxml_ctime = m_exportxml_pmtime = m_exportxml_atime = m_exportxml_ltime = 
    m_exportxml_rmtime = BST_CHECKED;
}

void CExportXMLXDlg::OnClearTimes()
{
	((CButton*)GetDlgItem(IDC_XMLX_CTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_PMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_ATIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_LTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_XMLX_RMTIME))->SetCheck(BST_UNCHECKED);
	m_exportxml_ctime = m_exportxml_pmtime = m_exportxml_atime = m_exportxml_ltime =
    m_exportxml_rmtime = BST_UNCHECKED;
}

void CExportXMLXDlg::OnSetTitle()
{
   // Disabled control looks bad, just grey out tick using INDETERMINATE trick
  ((CButton*)GetDlgItem(IDC_XMLX_TITLE))->SetCheck(BST_INDETERMINATE);
  UpdateData(FALSE);
}

void CExportXMLXDlg::OnSetPassword()
{
  // Disabled control looks bad, just grey out tick using INDETERMINATE trick
  ((CButton*)GetDlgItem(IDC_XMLX_PASSWORD))->SetCheck(BST_INDETERMINATE);
  UpdateData(FALSE);
}
