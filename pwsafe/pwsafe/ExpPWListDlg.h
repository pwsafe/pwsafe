/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

#include "afxcmn.h"
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#include "corelib/MyString.h"
#include "corelib/sha256.h"

// Expired password Entry structure for CList
struct ExpPWEntry {
  CMyString group;
  CMyString title;
  CMyString user;
  CMyString expirylocdate;	// user's long dat/time   - format displayed in ListCtrl
  CMyString expiryexpdate;	// "YYYY/MM/DD HH:MM:SS"  - format copied to clipboard - best for sorting
  time_t expirytttdate;
};

// CExpPWListDlg dialog

class CExpPWListDlg : public CDialog
{

public:
	CExpPWListDlg(CWnd* pParent = NULL,
		const CString& a_filespec = _T(""));   // standard constructor
	virtual ~CExpPWListDlg();

// Dialog Data
	enum { IDD = IDD_DISPLAY_EXPIRED_ENTRIES };
	CList<ExpPWEntry, ExpPWEntry&>* m_pexpPWList;
	CListCtrl m_expPWListCtrl;
	CString m_message;
	int m_iSortedColumn; 
	BOOL m_bSortAscending; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCopyExpToClipboard();
	afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);

private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
