#pragma once

#include "afxcmn.h"
#include "resource.h"
#include "corelib/MyString.h"
#include "corelib/sha256.h"

// Expired password Entry structure for CList
struct ExpPWEntry {
  CMyString group;
  CMyString title;
  CMyString user;
  CMyString expiryascdate;	// "Day Mon DD HH:MM:SS YYYY"	- format displayed in ListCtrl
  CMyString expiryexpdate;	// "YYYY/MM/DD HH:MM:SS"		- format copied to clipboard - best for sorting
  time_t expirytttdate;
};

// CExpPWListDlg dialog

class CExpPWListDlg : public CDialog
{

public:
	CExpPWListDlg(CWnd* pParent = NULL,
		const CString& a_filespec = "");   // standard constructor
	virtual ~CExpPWListDlg();

// Dialog Data
	enum { IDD = IDD_DISPLAY_EXPIRED_ENTRIES };
	CList<ExpPWEntry, ExpPWEntry&>* m_pexpPWList;
	CListCtrl m_expPWList;
	CString m_message;
	unsigned char m_expPWL_clipboard_digest[SHA256::HASHLEN];
	bool m_copied_to_clipboard;
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
