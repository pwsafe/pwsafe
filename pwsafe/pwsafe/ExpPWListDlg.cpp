/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ExpPWListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThisMfcApp.h"
#include "ExpPWListDlg.h"
#include "corelib/MyString.h"
#include "corelib/Util.h"

// CExpPWListDlg dialog


CExpPWListDlg::CExpPWListDlg(CWnd* pParent, const CString& a_filespec)
	: CDialog(CExpPWListDlg::IDD, pParent)
{
	const int FILE_DISP_LEN = 75;

	if (a_filespec.GetLength() > FILE_DISP_LEN) {
    // m_message = a_filespec.Right(FILE_DISP_LEN - 3); // truncate for display
    // m_message.Insert(0, _T("..."));
		m_message =  a_filespec.Left(FILE_DISP_LEN/2-5) + 
      _T(" ... ") + a_filespec.Right(FILE_DISP_LEN/2);
	} else {
		m_message = a_filespec;
	}

	m_iSortedColumn = -1; 
	m_bSortAscending = TRUE; 
}

CExpPWListDlg::~CExpPWListDlg()
{
}

void CExpPWListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPIRED_PASSWORD_LIST, m_expPWListCtrl);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
}


BEGIN_MESSAGE_MAP(CExpPWListDlg, CDialog)
	ON_BN_CLICKED(IDC_COPY_EXP_TO_CLIPBOARD, OnBnClickedCopyExpToClipboard)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
END_MESSAGE_MAP()


// CExpPWListDlg message handlers

BOOL
CExpPWListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString cs_text;
	cs_text.LoadString(IDS_GROUP);
	m_expPWListCtrl.InsertColumn(0, cs_text);
	cs_text.LoadString(IDS_TITLE);
	m_expPWListCtrl.InsertColumn(1, cs_text);
	cs_text.LoadString(IDS_USERNAME);
	m_expPWListCtrl.InsertColumn(2, cs_text);
	cs_text.LoadString(IDS_EXPIRYDATETIME);
	m_expPWListCtrl.InsertColumn(3, cs_text);

	int nPos = 0;
	POSITION itempos;

	POSITION listpos = m_pexpPWList->GetHeadPosition();
	while (listpos != NULL) {
		itempos = listpos;
		const ExpPWEntry exppwentry = m_pexpPWList->GetAt(listpos);
		nPos = m_expPWListCtrl.InsertItem(nPos, exppwentry.group);
		m_expPWListCtrl.SetItemText(nPos, 1, exppwentry.title);
		m_expPWListCtrl.SetItemText(nPos, 2, exppwentry.user);
		m_expPWListCtrl.SetItemText(nPos, 3, exppwentry.expirylocdate);
		m_expPWListCtrl.SetItemData(nPos, (DWORD)itempos);
		m_pexpPWList->GetNext(listpos);
	}

	m_expPWListCtrl.SetRedraw(FALSE);
	for (int i = 0; i < 4; i++) {
		m_expPWListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
		int nColumnWidth = m_expPWListCtrl.GetColumnWidth(i);
		m_expPWListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		int nHeaderWidth = m_expPWListCtrl.GetColumnWidth(i);
		m_expPWListCtrl.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
	}
	m_expPWListCtrl.SetRedraw(TRUE);

	return TRUE;
}

void
CExpPWListDlg::OnOK() 
{
	CDialog::OnOK();
}

void
CExpPWListDlg::OnBnClickedCopyExpToClipboard()
{
	CString data(MAKEINTRESOURCE(IDS_COPYTITLE));
	const CString CRLF = _T("\r\n");
	const CString TAB = _T('\t');

	POSITION listpos = m_pexpPWList->GetHeadPosition();
	while (listpos != NULL) {
		const ExpPWEntry exppwentry = m_pexpPWList->GetAt(listpos);
		data = data +
			(CString)exppwentry.group + TAB + 
			(CString)exppwentry.title + TAB + 
			(CString)exppwentry.user + TAB + 
			(CString)exppwentry.expiryexpdate + CRLF;
		m_pexpPWList->GetNext(listpos);
	}
					
	app.SetClipboardData(data);
}

void
CExpPWListDlg::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

	if(phdn->iButton == 0) {
		// User clicked on header using left mouse button
		if(phdn->iItem == m_iSortedColumn)
			m_bSortAscending = !m_bSortAscending;
		else
			m_bSortAscending = TRUE;

		m_iSortedColumn = phdn->iItem;
		m_expPWListCtrl.SortItems(CompareFunc, (LPARAM)this);

		// Note: WINVER defines the minimum system level for which this is program compiled and 
		// NOT the level of system it is running on!
		// In this case, these values are defined in Windows XP and later and supported
		// by V6 of comctl32.dll (supplied with Windows XP) and later.
		// They should be ignored by earlier levels of this dll or .....
		//     we can check the dll version (code available on request)!

#if (WINVER < 0x0501)	// These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
		HDITEM HeaderItem;
		HeaderItem.mask = HDI_FORMAT;
		m_expPWListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
		// Turn off all arrows
		HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
		// Turn on the correct arrow
		HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
		m_expPWListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
	}

	*pResult = 0;
}

int CALLBACK CExpPWListDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2,
										LPARAM closure)
{
	CExpPWListDlg *self = (CExpPWListDlg*)closure;
	int nSortColumn = self->m_iSortedColumn;
	POSITION Lpos = (POSITION)lParam1;
	POSITION Rpos = (POSITION)lParam2;
	const ExpPWEntry pLHS = self->m_pexpPWList->GetAt(Lpos);
	const ExpPWEntry pRHS = self->m_pexpPWList->GetAt(Rpos);
	CMyString group1, title1, username1;
	CMyString group2, title2, username2;
	time_t t1, t2;

	int iResult;
	switch(nSortColumn) {
		case 0:
			group1 = pLHS.group;
			group2 = pRHS.group;
			iResult = ((CString)group1).CompareNoCase(group2);
			break;
		case 1:
			title1 = pLHS.title;
			title2 = pRHS.title;
			iResult = ((CString)title1).CompareNoCase(title2);
			break;
		case 2:
			username1 = pLHS.user;
			username2 = pRHS.user;
			iResult = ((CString)username1).CompareNoCase(username2);
			break;
		case 3:
			t1 = pLHS.expirytttdate;
			t2 = pRHS.expirytttdate;
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		default:
		    iResult = 0; // should never happen - just keep compiler happy
			ASSERT(FALSE);
	}

	if (!self->m_bSortAscending)
		iResult *= -1;

	return iResult;
}
