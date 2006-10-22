#pragma once

// OptionsPasswordHistory.h : header file
//

#include "DboxMain.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory dialog

class COptionsPasswordHistory : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPasswordHistory)

// Construction
public:
	COptionsPasswordHistory();
	~COptionsPasswordHistory();
	DboxMain *m_pDboxMain;

// Dialog Data
	//{{AFX_DATA(COptionsPasswordHistory)
	enum { IDD = IDD_PS_PASSWORDHISTORY };
	BOOL	m_savepwhistory;
	UINT	m_pwhistorynumdefault;
	int     m_pwhaction;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPasswordHistory)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPasswordHistory)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnKillActive();
	afx_msg void OnSavePWHistory();
	afx_msg void OnApplyPWHChanges();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// Implementation
protected:
	BOOL PreTranslateMessage(MSG* pMsg);

private:
	CToolTipCtrl* m_ToolTipCtrl;
	afx_msg void OnPWHistoryNoAction();
	afx_msg void OnPWHistoryDoAction();
};
