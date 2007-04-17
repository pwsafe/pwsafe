/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// CompareXDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "CompareXDlg.h"
#include "corelib/ItemData.h"
#include "ThisMfcApp.h"
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCompareXDlg dialog


CCompareXDlg::CCompareXDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCompareXDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompareXDlg)
	m_compare_group = BST_CHECKED;
  m_compare_title = BST_CHECKED;
	m_compare_user = BST_CHECKED;
	m_compare_notes = BST_CHECKED;
	m_compare_password = BST_CHECKED;
	m_compare_ctime = BST_CHECKED;
	m_compare_pmtime = BST_CHECKED;
	m_compare_atime = BST_CHECKED;
	m_compare_ltime = BST_CHECKED;
	m_compare_rmtime = BST_CHECKED;
	m_compare_url = BST_CHECKED;
	m_compare_autotype = BST_CHECKED;
	m_compare_pwhist = BST_CHECKED;
	m_compare_subgroup = BST_UNCHECKED;
	m_compare_subgroup_name = _T("*");
	m_compare_subgroup_case = BST_CHECKED;
	m_subgroup_object = -1;
	m_subgroup_function = -1;

	//}}AFX_DATA_INIT
}


BOOL CCompareXDlg::OnInitDialog()
{
	 CDialog::OnInitDialog();
	 m_bsCompare.set();  // note: impossible to set them all even via the advanced dialog

	int index;
	CString cs_text;

	CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_FUNCTION);
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

	CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_OBJECT);
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


void CCompareXDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompareXDlg)
	DDX_Check(pDX, IDC_COMPAREX_GROUP, m_compare_group);
  DDX_Check(pDX, IDC_COMPAREX_TITLE, m_compare_title);
	DDX_Check(pDX, IDC_COMPAREX_USER, m_compare_user);
	DDX_Check(pDX, IDC_COMPAREX_NOTES, m_compare_notes);
	DDX_Check(pDX, IDC_COMPAREX_PASSWORD, m_compare_password);
	DDX_Check(pDX, IDC_COMPAREX_CTIME, m_compare_ctime);
	DDX_Check(pDX, IDC_COMPAREX_PMTIME, m_compare_pmtime);
	DDX_Check(pDX, IDC_COMPAREX_ATIME, m_compare_atime);
	DDX_Check(pDX, IDC_COMPAREX_LTIME, m_compare_ltime);
	DDX_Check(pDX, IDC_COMPAREX_RMTIME, m_compare_rmtime);
	DDX_Check(pDX, IDC_COMPAREX_URL, m_compare_url);
	DDX_Check(pDX, IDC_COMPAREX_AUTOTYPE, m_compare_autotype);
	DDX_Check(pDX, IDC_COMPAREX_PWHIST, m_compare_pwhist);
	DDX_Check(pDX, IDC_COMPAREX_SUBGROUP, m_compare_subgroup);
	DDX_Check(pDX, IDC_COMPAREX_SUBGROUP_CASE, m_compare_subgroup_case);
	DDX_Text(pDX, IDC_COMPAREX_SUBGROUP_NAME, m_compare_subgroup_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompareXDlg, CDialog)
	//{{AFX_MSG_MAP(CCompareXDlg)
	ON_BN_CLICKED(IDC_COMPAREX_SETTIMES, OnSetTimes)
	ON_BN_CLICKED(IDC_COMPAREX_CLEARTIMES, OnClearTimes)
	ON_BN_CLICKED(IDC_COMPAREX_SUBGROUP, OnSetSubGroup)
	ON_BN_CLICKED(IDC_COMPAREX_SETALL, OnSetAll)
	ON_BN_CLICKED(IDC_COMPAREX_CLEARALL, OnClearAll)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompareXDlg message handlers

void CCompareXDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/exportx.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CCompareXDlg::OnOK()
{
	UpdateData();
	m_bsCompare.reset();

	m_bsCompare.set(CItemData::GROUP, m_compare_group == BST_CHECKED ? true : false);
  m_bsCompare.set(CItemData::TITLE, m_compare_title == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::PASSWORD, m_compare_password == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::USER, m_compare_user == BST_CHECKED? true : false);
	m_bsCompare.set(CItemData::NOTES, m_compare_notes == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::PASSWORD, m_compare_password == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::CTIME, m_compare_ctime == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::PMTIME, m_compare_pmtime == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::ATIME, m_compare_atime == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::LTIME, m_compare_ltime == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::RMTIME, m_compare_rmtime == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::URL, m_compare_url == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::AUTOTYPE, m_compare_autotype == BST_CHECKED ? true : false);
	m_bsCompare.set(CItemData::PWHIST, m_compare_pwhist == BST_CHECKED ? true : false);

	if (m_bsCompare.count() == 0) {
		AfxMessageBox(IDS_NOFIELDSFOREXPORT);
		m_bsCompare.set();  // note: impossible to set them all even via the advanced dialog
		return;
	}

	if (m_compare_subgroup == BST_CHECKED) {
		GetDlgItemText(IDC_COMPAREX_SUBGROUP_NAME, m_compare_subgroup_name);
		int nObject = ((CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_OBJECT))->GetCurSel();
		if (nObject == CB_ERR) {
			AfxMessageBox(IDS_NOOBJECT);
			m_bsCompare.set();  // note: impossible to set them all even via the advanced dialog
			((CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_OBJECT))->SetFocus();
			return;
		}
		int nFunction = ((CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_FUNCTION))->GetCurSel();
		if (nFunction == CB_ERR) {
			AfxMessageBox(IDS_NOFUNCTION);
			m_bsCompare.set();  // note: impossible to set them all even via the advanced dialog
			((CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_FUNCTION))->SetFocus();
			return;
		}
		m_subgroup_object = ((CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_OBJECT))->GetItemData(nObject);
		m_subgroup_function = ((CComboBox *)GetDlgItem(IDC_COMPAREX_SUBGROUP_FUNCTION))->GetItemData(nFunction);
		if (m_compare_subgroup_case == BST_CHECKED)
			m_subgroup_function *= (-1);
	}

	if (m_compare_subgroup_name == _T("*"))
		m_compare_subgroup_name.Empty();

	CDialog::OnOK();
}

void CCompareXDlg::OnSetSubGroup()
{
	m_compare_subgroup = ((CButton*)GetDlgItem(IDC_COMPAREX_SUBGROUP))->GetCheck();
	if (m_compare_subgroup == BST_CHECKED) {
		GetDlgItem(IDC_COMPAREX_SUBGROUP_NAME)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_COMPAREX_SUBGROUP_NAME)->EnableWindow(FALSE);
	}
}

void CCompareXDlg::OnSetAll()
{
	((CButton*)GetDlgItem(IDC_COMPAREX_GROUP))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_COMPAREX_TITLE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_USER))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_NOTES))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PASSWORD))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_CTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_ATIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_LTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_RMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_URL))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_AUTOTYPE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PWHIST))->SetCheck(BST_CHECKED);
	m_compare_group = m_compare_title = m_compare_user = m_compare_notes = 
    m_compare_password = m_compare_ctime = m_compare_pmtime = m_compare_atime = 
    m_compare_ltime = m_compare_rmtime = m_compare_url = m_compare_autotype = 
    m_compare_pwhist = m_compare_subgroup = BST_CHECKED;
}

void CCompareXDlg::OnClearAll()
{
	((CButton*)GetDlgItem(IDC_COMPAREX_GROUP))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_COMPAREX_TITLE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_USER))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_NOTES))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PASSWORD))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_CTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_ATIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_LTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_RMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_URL))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_AUTOTYPE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PWHIST))->SetCheck(BST_UNCHECKED);
	m_compare_group = m_compare_title = m_compare_user = m_compare_notes = 
    m_compare_password = m_compare_ctime = m_compare_pmtime = m_compare_atime = 
    m_compare_ltime = m_compare_rmtime = m_compare_url = m_compare_autotype = 
    m_compare_pwhist = m_compare_subgroup = BST_UNCHECKED;

}

void CCompareXDlg::OnSetTimes()
{
	((CButton*)GetDlgItem(IDC_COMPAREX_CTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_ATIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_LTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_RMTIME))->SetCheck(BST_CHECKED);
	m_compare_ctime = m_compare_pmtime = m_compare_atime = m_compare_ltime =
    m_compare_rmtime = BST_CHECKED;
}

void CCompareXDlg::OnClearTimes()
{
	((CButton*)GetDlgItem(IDC_COMPAREX_CTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_PMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_ATIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_LTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_COMPAREX_RMTIME))->SetCheck(BST_UNCHECKED);
	m_compare_ctime = m_compare_pmtime = m_compare_atime = m_compare_ltime = 
    m_compare_rmtime = BST_UNCHECKED;
}
