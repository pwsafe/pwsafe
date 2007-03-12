/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ColumnPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ColumnPickerDlg.h"
#include "ThisMfcApp.h"
#include "corelib/ItemData.h"
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColumnPickerDlg dialog


CColumnPickerDlg::CColumnPickerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColumnPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColumnPickerDlg)
	m_column_title = BST_INDETERMINATE;  // mandatory
	m_column_user = BST_INDETERMINATE;   // mandatory
	//}}AFX_DATA_INIT
}

BOOL CColumnPickerDlg::OnInitDialog()
{
    m_column_group = m_bsColumn.test(CItemData::GROUP) ? 1 : 0;
	m_column_password = m_bsColumn.test(CItemData::PASSWORD) ? 1 : 0;
	m_column_notes = m_bsColumn.test(CItemData::NOTES) ? 1 : 0;
	m_column_password = m_bsColumn.test(CItemData::PASSWORD) ? 1 : 0;
	m_column_ctime = m_bsColumn.test(CItemData::CTIME) ? 1 : 0;
	m_column_pmtime = m_bsColumn.test(CItemData::PMTIME) ? 1 : 0;
	m_column_atime = m_bsColumn.test(CItemData::ATIME) ? 1 : 0;
	m_column_ltime = m_bsColumn.test(CItemData::LTIME) ? 1 : 0;
	m_column_rmtime = m_bsColumn.test(CItemData::RMTIME) ? 1 : 0;

    CDialog::OnInitDialog();
	return TRUE;
}

void CColumnPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColumnPickerDlg)
	DDX_Check(pDX, IDC_COLPICK_GROUP, m_column_group);
	DDX_Check(pDX, IDC_COLPICK_TITLE, m_column_title);
	DDX_Check(pDX, IDC_COLPICK_USER, m_column_user);
	DDX_Check(pDX, IDC_COLPICK_NOTES, m_column_notes);
	DDX_Check(pDX, IDC_COLPICK_PASSWORD, m_column_password);
	DDX_Check(pDX, IDC_COLPICK_CTIME, m_column_ctime);
	DDX_Check(pDX, IDC_COLPICK_PMTIME, m_column_pmtime);
	DDX_Check(pDX, IDC_COLPICK_ATIME, m_column_atime);
	DDX_Check(pDX, IDC_COLPICK_LTIME, m_column_ltime);
	DDX_Check(pDX, IDC_COLPICK_RMTIME, m_column_rmtime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CColumnPickerDlg, CDialog)
	//{{AFX_MSG_MAP(CColumnPickerDlg)
    ON_BN_CLICKED(IDC_COLPICK_TITLE, OnSetColumnTitle)
    ON_BN_CLICKED(IDC_COLPICK_USER, OnSetColumnUser)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColumnPickerDlg message handlers

void CColumnPickerDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/colpicker.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CColumnPickerDlg::OnOK()
{
	UpdateData();
	m_bsColumn.reset();

    m_bsColumn.set(CItemData::TITLE, true);  // mandatory
    m_bsColumn.set(CItemData::USER, true);   // mandatory

	m_bsColumn.set(CItemData::GROUP, m_column_group == 1 ? true : false);
	m_bsColumn.set(CItemData::PASSWORD, m_column_password == 1 ? true : false);
	m_bsColumn.set(CItemData::NOTES, m_column_notes == 1 ? true : false);
	m_bsColumn.set(CItemData::PASSWORD, m_column_password == 1 ? true : false);
	m_bsColumn.set(CItemData::CTIME, m_column_ctime == 1 ? true : false);
	m_bsColumn.set(CItemData::PMTIME, m_column_pmtime == 1 ? true : false);
	m_bsColumn.set(CItemData::ATIME, m_column_atime == 1 ? true : false);
	m_bsColumn.set(CItemData::LTIME, m_column_ltime == 1 ? true : false);
	m_bsColumn.set(CItemData::RMTIME, m_column_rmtime == 1 ? true : false);


	CDialog::OnOK();
}

void CColumnPickerDlg::OnSetColumnTitle()
{
    // Disabled control looks bad, just grey out tick using INDETERMINATE trick
	((CButton*)GetDlgItem(IDC_COLPICK_TITLE))->SetCheck(BST_INDETERMINATE);
    UpdateData(FALSE);
}

void CColumnPickerDlg::OnSetColumnUser()
{
    // Disabled control looks bad, just grey out tick using INDETERMINATE trick
	((CButton*)GetDlgItem(IDC_COLPICK_USER))->SetCheck(BST_INDETERMINATE);
    UpdateData(FALSE);
}
