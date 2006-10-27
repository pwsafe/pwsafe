// ExportTextXDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ExportTextXDlg.h"
#include "corelib/ItemData.h"
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportTextXDlg dialog


CExportTextXDlg::CExportTextXDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportTextXDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportTextXDlg)
	m_export_group_title = 1;
	m_export_user = 1;
	m_export_notes = 1;
	m_export_password = 1;
	m_export_ctime = 1;
	m_export_pmtime = 1;
	m_export_atime = 1;
	m_export_ltime = 1;
	m_export_rmtime = 1;
	m_export_url = 1;
	m_export_autotype = 1;
	m_export_pwhist = 1;
	m_export_subgroup = 0;
	m_export_subgroup_name = _T("*");
	m_export_subgroup_case = 0;
	m_subgroup_object = -1;
	m_subgroup_function = -1;

	//}}AFX_DATA_INIT
}


BOOL CExportTextXDlg::OnInitDialog()
{
	 CDialog::OnInitDialog();
	 m_bsExport.set();  // note: impossible to set them all even via the advanced dialog

	int index, irc;
	CString cs_text;

	CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_FUNCTION);
	if(cboSubgroupFunction->GetCount() == 0) {
		cs_text.LoadString(IDS_EQUALS);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_EQUALS);
		cs_text.LoadString(IDS_DOESNOTEQUAL);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTEQUAL);
		cs_text.LoadString(IDS_BEGINSWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_BEGINS);
		cs_text.LoadString(IDS_DOESNOTBEGINSWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTBEGIN);
		cs_text.LoadString(IDS_ENDSWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_ENDS);
		cs_text.LoadString(IDS_DOESNOTENDWITH);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTEND);
		cs_text.LoadString(IDS_CONTAINS);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_CONTAINS);
		cs_text.LoadString(IDS_DOESNOTCONTAIN);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTCONTAIN);
	}
	cboSubgroupFunction->SetCurSel(0);

	CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_OBJECT);
	if(cboSubgroupObject->GetCount() == 0) {
		cs_text.LoadString(IDS_GROUP);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupObject->SetItemData(index, CItemData::SGO_GROUP);
		cs_text.LoadString(IDS_TITLE);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupObject->SetItemData(index, CItemData::SGO_TITLE);
		cs_text.LoadString(IDS_USERNAME);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupObject->SetItemData(index, CItemData::SGO_USER);
		cs_text.LoadString(IDS_GROUPTITLE);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupObject->SetItemData(index, CItemData::SGO_GROUPTITLE);
		cs_text.LoadString(IDS_URL);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupObject->SetItemData(index, CItemData::SGO_URL);
		cs_text.LoadString(IDS_NOTES);
		index = cboSubgroupFunction->AddString(cs_text);
		irc = cboSubgroupObject->SetItemData(index, CItemData::SGO_NOTES);
	}
	cboSubgroupObject->SetCurSel(0);

	return TRUE;
}


void CExportTextXDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportTextXDlg)
	DDX_Check(pDX, IDC_EXPORTX_GROUP_TITLE, m_export_group_title);
	DDX_Check(pDX, IDC_EXPORTX_USER, m_export_user);
	DDX_Check(pDX, IDC_EXPORTX_NOTES, m_export_notes);
	DDX_Check(pDX, IDC_EXPORTX_PASSWORD, m_export_password);
	DDX_Check(pDX, IDC_EXPORTX_CTIME, m_export_ctime);
	DDX_Check(pDX, IDC_EXPORTX_PMTIME, m_export_pmtime);
	DDX_Check(pDX, IDC_EXPORTX_ATIME, m_export_atime);
	DDX_Check(pDX, IDC_EXPORTX_LTIME, m_export_ltime);
	DDX_Check(pDX, IDC_EXPORTX_RMTIME, m_export_rmtime);
	DDX_Check(pDX, IDC_EXPORTX_URL, m_export_url);
	DDX_Check(pDX, IDC_EXPORTX_AUTOTYPE, m_export_autotype);
	DDX_Check(pDX, IDC_EXPORTX_PWHIST, m_export_pwhist);
	DDX_Check(pDX, IDC_EXPORTX_SUBGROUP, m_export_subgroup);
	DDX_Check(pDX, IDC_EXPORTX_SUBGROUP_CASE, m_export_subgroup_case);
	DDX_Text(pDX, IDC_EXPORTX_SUBGROUP_NAME, m_export_subgroup_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportTextXDlg, CDialog)
	//{{AFX_MSG_MAP(CExportTextXDlg)
	ON_BN_CLICKED(IDC_EXPORTX_GROUP_TITLE, OnSetExportGroupTitle)
	ON_BN_CLICKED(IDC_EXPORTX_USER, OnSetExportUser)
	ON_BN_CLICKED(IDC_EXPORTX_NOTES, OnSetExportNotes)
	ON_BN_CLICKED(IDC_EXPORTX_PASSWORD, OnSetExportPassword)
	ON_BN_CLICKED(IDC_EXPORTX_CTIME, OnSetExportCTime)
	ON_BN_CLICKED(IDC_EXPORTX_PMTIME, OnSetExportPMTime)
	ON_BN_CLICKED(IDC_EXPORTX_ATIME, OnSetExportATime)
	ON_BN_CLICKED(IDC_EXPORTX_LTIME, OnSetExportLTime)
	ON_BN_CLICKED(IDC_EXPORTX_RMTIME, OnSetExportRMTime)
	ON_BN_CLICKED(IDC_EXPORTX_URL, OnSetExportUrl)
	ON_BN_CLICKED(IDC_EXPORTX_AUTOTYPE, OnSetExportAutotype)
	ON_BN_CLICKED(IDC_EXPORTX_PWHIST, OnSetExportPWHist)
	ON_BN_CLICKED(IDC_EXPORTX_SETTIMES, OnSetTimes)
	ON_BN_CLICKED(IDC_EXPORTX_CLEARTIMES, OnClearTimes)
	ON_BN_CLICKED(IDC_EXPORTX_SUBGROUP, OnSetSubGroup)
	ON_BN_CLICKED(IDC_EXPORTX_SUBGROUP_CASE, OnSetSubgroupCase)
	ON_BN_CLICKED(IDC_EXPORTX_SETALL, OnSetAll)
	ON_BN_CLICKED(IDC_EXPORTX_CLEARALL, OnClearAll)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportTextXDlg message handlers

void CExportTextXDlg::OnHelp()
{
	::HtmlHelp(NULL,
		 "pwsafe.chm::/html/exportx.html",
		 HH_DISPLAY_TOPIC, 0);
}

void CExportTextXDlg::OnOK()
{
	UpdateData();
	m_bsExport.reset();

	m_bsExport.set(CItemData::GROUP, m_export_group_title == 1 ? true : false);
	m_bsExport.set(CItemData::PASSWORD, m_export_password == 1 ? true : false);
	m_bsExport.set(CItemData::USER, m_export_user == 1 ? true : false);
	m_bsExport.set(CItemData::NOTES, m_export_notes == 1 ? true : false);
	m_bsExport.set(CItemData::PASSWORD, m_export_password == 1 ? true : false);
	m_bsExport.set(CItemData::CTIME, m_export_ctime == 1 ? true : false);
	m_bsExport.set(CItemData::PMTIME, m_export_pmtime == 1 ? true : false);
	m_bsExport.set(CItemData::ATIME, m_export_atime == 1 ? true : false);
	m_bsExport.set(CItemData::LTIME, m_export_ltime == 1 ? true : false);
	m_bsExport.set(CItemData::RMTIME, m_export_rmtime == 1 ? true : false);
	m_bsExport.set(CItemData::URL, m_export_url == 1 ? true : false);
	m_bsExport.set(CItemData::AUTOTYPE, m_export_autotype == 1 ? true : false);
	m_bsExport.set(CItemData::PWHIST, m_export_pwhist == 1 ? true : false);

	if (m_bsExport.count() == 0) {
		AfxMessageBox(IDS_NOFIELDSFOREXPORT);
		m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
		return;
	}

	if (m_export_subgroup == 1) {
		GetDlgItemText(IDC_EXPORTX_SUBGROUP_NAME, m_export_subgroup_name);
		int nObject = ((CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_OBJECT))->GetCurSel();
		if (nObject == CB_ERR) {
			AfxMessageBox(IDS_NOOBJECT);
			m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
			((CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_OBJECT))->SetFocus();
			return;
		}
		int nFunction = ((CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_FUNCTION))->GetCurSel();
		if (nFunction == CB_ERR) {
			AfxMessageBox(IDS_NOFUNCTION);
			m_bsExport.set();  // note: impossible to set them all even via the advanced dialog
			((CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_FUNCTION))->SetFocus();
			return;
		}
		m_subgroup_object = ((CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_OBJECT))->GetItemData(nObject);
		m_subgroup_function = ((CComboBox *)GetDlgItem(IDC_EXPORTX_SUBGROUP_FUNCTION))->GetItemData(nFunction);
		if (m_export_subgroup_case == 1)
			m_subgroup_function *= (-1);
	}

	if (m_export_subgroup_name == _T("*"))
		m_export_subgroup_name.Empty();

	CDialog::OnOK();
}

void CExportTextXDlg::OnSetSubGroup()
{
	m_export_subgroup = ((CButton*)GetDlgItem(IDC_EXPORTX_SUBGROUP))->GetCheck();
	if (m_export_subgroup == 1) {
		GetDlgItem(IDC_EXPORTX_SUBGROUP_NAME)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_EXPORTX_SUBGROUP_NAME)->EnableWindow(FALSE);
	}
}

void CExportTextXDlg::OnSetAll()
{
	((CButton*)GetDlgItem(IDC_EXPORTX_GROUP_TITLE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_USER))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_NOTES))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PASSWORD))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_CTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_ATIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_LTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_RMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_URL))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_AUTOTYPE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PWHIST))->SetCheck(BST_CHECKED);
	m_export_group_title = m_export_user = m_export_notes = m_export_password =
		m_export_ctime = m_export_pmtime = m_export_atime = m_export_ltime = m_export_rmtime =
		m_export_url = m_export_autotype = m_export_pwhist = m_export_subgroup = 1;
}

void CExportTextXDlg::OnClearAll()
{
	((CButton*)GetDlgItem(IDC_EXPORTX_GROUP_TITLE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_USER))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_NOTES))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PASSWORD))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_CTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_ATIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_LTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_RMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_URL))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_AUTOTYPE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PWHIST))->SetCheck(BST_UNCHECKED);
	m_export_group_title = m_export_user = m_export_notes = m_export_password =
		m_export_ctime = m_export_pmtime = m_export_atime = m_export_ltime = m_export_rmtime =
		m_export_url = m_export_autotype = m_export_pwhist = m_export_subgroup = 0;

}

void CExportTextXDlg::OnSetTimes()
{
	((CButton*)GetDlgItem(IDC_EXPORTX_CTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PMTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_ATIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_LTIME))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_RMTIME))->SetCheck(BST_CHECKED);
	m_export_ctime = m_export_pmtime = m_export_atime = m_export_ltime = m_export_rmtime = 1;
}

void CExportTextXDlg::OnClearTimes()
{
	((CButton*)GetDlgItem(IDC_EXPORTX_CTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_PMTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_ATIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_LTIME))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_EXPORTX_RMTIME))->SetCheck(BST_UNCHECKED);
	m_export_ctime = m_export_pmtime = m_export_atime = m_export_ltime = m_export_rmtime = 0;
}

void CExportTextXDlg::OnSetExportGroupTitle()
{
	m_export_group_title = ((CButton*)GetDlgItem(IDC_EXPORTX_GROUP_TITLE))->GetCheck();
}

void CExportTextXDlg::OnSetExportUser()
{
	m_export_user = ((CButton*)GetDlgItem(IDC_EXPORTX_USER))->GetCheck();
}

void CExportTextXDlg::OnSetExportPassword()
{
	m_export_password = ((CButton*)GetDlgItem(IDC_EXPORTX_PASSWORD))->GetCheck();
}

void CExportTextXDlg::OnSetExportNotes()
{
	m_export_notes = ((CButton*)GetDlgItem(IDC_EXPORTX_NOTES))->GetCheck();
}

void CExportTextXDlg::OnSetExportCTime()
{
	m_export_ctime = ((CButton*)GetDlgItem(IDC_EXPORTX_CTIME))->GetCheck();
}

void CExportTextXDlg::OnSetExportPMTime()
{
	m_export_pmtime = ((CButton*)GetDlgItem(IDC_EXPORTX_PMTIME))->GetCheck();
}

void CExportTextXDlg::OnSetExportATime()
{
	m_export_atime = ((CButton*)GetDlgItem(IDC_EXPORTX_ATIME))->GetCheck();
}

void CExportTextXDlg::OnSetExportLTime()
{
	m_export_ltime = ((CButton*)GetDlgItem(IDC_EXPORTX_LTIME))->GetCheck();
}

void CExportTextXDlg::OnSetExportRMTime()
{
	m_export_rmtime = ((CButton*)GetDlgItem(IDC_EXPORTX_RMTIME))->GetCheck();
}

void CExportTextXDlg::OnSetExportUrl()
{
	m_export_url = ((CButton*)GetDlgItem(IDC_EXPORTX_URL))->GetCheck();
}

void CExportTextXDlg::OnSetExportAutotype()
{
	m_export_autotype = ((CButton*)GetDlgItem(IDC_EXPORTX_AUTOTYPE))->GetCheck();
}

void CExportTextXDlg::OnSetExportPWHist()
{
	m_export_pwhist = ((CButton*)GetDlgItem(IDC_EXPORTX_PWHIST))->GetCheck();
}

void CExportTextXDlg::OnSetSubgroupCase()
{
	m_export_subgroup_case = ((CButton*)GetDlgItem(IDC_EXPORTX_SUBGROUP_CASE))->GetCheck();
}
