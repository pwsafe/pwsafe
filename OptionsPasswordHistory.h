#pragma once

// OptionsPasswordHistory.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory dialog

class COptionsPasswordHistory : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPasswordHistory)

// Construction
public:
	COptionsPasswordHistory();
	~COptionsPasswordHistory();

// Dialog Data
	//{{AFX_DATA(COptionsPasswordHistory)
	enum { IDD = IDD_PS_PASSWORDHISTORY };
	BOOL	m_savepwhistory;
	UINT	m_pwhistorynumdefault;
	BOOL    m_applypwhistory;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
