// ShowPWHistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "ShowPWHistDlg.h"
#include "corelib/MyString.h"
#include "corelib/Util.h"

// CShowPWHistDlg dialog


CShowPWHistDlg::CShowPWHistDlg(CWnd* pParent)
	: CDialog(CShowPWHistDlg::IDD, pParent)
{
	m_iSortedColumn = -1;
	m_bSortAscending = TRUE;
}

CShowPWHistDlg::~CShowPWHistDlg()
{
}

void CShowPWHistDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PWHISTORY_LIST, m_PWHistListCtrl);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
}


BEGIN_MESSAGE_MAP(CShowPWHistDlg, CDialog)
	ON_BN_CLICKED(IDC_COPY_OLDPW_TO_CLIPBOARD, OnBnClickedCopyToClipboard)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
END_MESSAGE_MAP()


// CShowPWHistDlg message handlers

BOOL
CShowPWHistDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_PWHistListCtrl.InsertColumn(0, _T("Changed Date/Time"));
	m_PWHistListCtrl.InsertColumn(1, _T("Password"));

	int nPos = 0;
	POSITION itempos;

	POSITION listpos = m_pPWHistList->GetHeadPosition();
	while (listpos != NULL) {
		itempos = listpos;
		const PWHistEntry pwhentry = m_pPWHistList->GetAt(listpos);
		nPos = m_PWHistListCtrl.InsertItem(nPos, pwhentry.changedate);
		m_PWHistListCtrl.SetItemText(nPos, 1, pwhentry.password);
		m_PWHistListCtrl.SetItemData(nPos, (DWORD)itempos);
		m_pPWHistList->GetNext(listpos);
	}

	m_PWHistListCtrl.SetRedraw(FALSE);
	for (int i = 0; i < 2; i++) {
		m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
		int nColumnWidth = m_PWHistListCtrl.GetColumnWidth(i);
		m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		int nHeaderWidth = m_PWHistListCtrl.GetColumnWidth(i);
		m_PWHistListCtrl.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
	}
	m_PWHistListCtrl.SetRedraw(TRUE);

	return TRUE;
}

void
CShowPWHistDlg::OnOK()
{
	CDialog::OnOK();
}

void
CShowPWHistDlg::OnBnClickedCopyToClipboard()
{
	CString data = _T("");
	const CString CRLF = _T("\r\n");
	const CString TAB = _T('\t');

	POSITION listpos = m_pPWHistList->GetHeadPosition();
	while (listpos != NULL) {
		const PWHistEntry pwhentry = m_pPWHistList->GetAt(listpos);
		data = data +
			(CString)pwhentry.changedate + TAB +
			(CString)pwhentry.password + CRLF;
		m_pPWHistList->GetNext(listpos);
	}

	app.SetClipboardData(data);
}

void
CShowPWHistDlg::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

	if(phdn->iButton == 0) {
		// User clicked on header using left mouse button
		if(phdn->iItem == m_iSortedColumn)
			m_bSortAscending = !m_bSortAscending;
		else
			m_bSortAscending = TRUE;

		m_iSortedColumn = phdn->iItem;
		m_PWHistListCtrl.SortItems(CompareFunc, (LPARAM)this);

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
		m_PWHistListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
		// Turn off all arrows
		HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
		// Turn on the correct arrow
		HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
		m_PWHistListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
	}

	*pResult = 0;
}

int CALLBACK CShowPWHistDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2,
										LPARAM closure)
{
	CShowPWHistDlg *self = (CShowPWHistDlg*)closure;
	int nSortColumn = self->m_iSortedColumn;
	POSITION Lpos = (POSITION)lParam1;
	POSITION Rpos = (POSITION)lParam2;
	const PWHistEntry pLHS = self->m_pPWHistList->GetAt(Lpos);
	const PWHistEntry pRHS = self->m_pPWHistList->GetAt(Rpos);
	CMyString password1, changedate1;
	CMyString password2, changedate2;
	time_t t1, t2;

	int iResult;
	switch(nSortColumn) {
		case 0:
			t1 = pLHS.changetttdate;
			t2 = pRHS.changetttdate;
			iResult = ((long) t1 < (long) t2) ? -1 : 1;
			break;
		case 1:
			password1 = pLHS.password;
			password2 = pRHS.password;
			iResult = ((CString)password1).Compare(password2);
			break;
		default:
		    iResult = 0; // should never happen - just keep compiler happy
			ASSERT(FALSE);
	}

	if (!self->m_bSortAscending)
		iResult *= -1;

	return iResult;
}
