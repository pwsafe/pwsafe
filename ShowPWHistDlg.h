#pragma once

#include "afxcmn.h"
#include "resource.h"
#include "corelib/MyString.h"
#include "corelib/sha256.h"

// CShowPWHistDlg dialog

class CShowPWHistDlg : public CDialog
{

public:
	CShowPWHistDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowPWHistDlg();

// Dialog Data
	enum { IDD = IDD_SHOW_PWHISTORY };
	CListCtrl m_PWHistListCtrl;
	CString m_message;

	CList<PWHistEntry, PWHistEntry&>* m_pPWHistList;
	int m_iSortedColumn;
	BOOL m_bSortAscending;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCopyToClipboard();
	afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);

private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
