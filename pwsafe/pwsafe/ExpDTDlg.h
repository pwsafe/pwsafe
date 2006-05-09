#pragma once

#include "afxwin.h"
#include "resource.h"
#include "afxdtctl.h" // only needed for date/time controls
#include "corelib\MyString.h"

class CExpDTDlg
	: public CDialog
{

public:
	CExpDTDlg(CWnd* pParent = NULL)
		: CDialog(CExpDTDlg::IDD, pParent){};   // standard constructor

	CDateTimeCtrl* m_pTimeCtl;                // pointer to a time picker
	CDateTimeCtrl* m_pDateCtl;                // pointer to a date picker
	CMyString m_ascLTime;
	time_t m_tttLTime;

// Dialog Data
	enum { IDD = IDD_PICKEXPDATETIME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

};
