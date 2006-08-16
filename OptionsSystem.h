// OptionsSystem.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem dialog

class COptionsSystem : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsSystem)

// Construction
public:
	COptionsSystem();
	~COptionsSystem();

// Dialog Data
	//{{AFX_DATA(COptionsSystem)
	enum { IDD = IDD_PS_SYSTEM };
	int		m_maxreitems;
	BOOL    m_usesystemtray;
	int		m_maxmruitems;
	BOOL	m_mruonfilemenu;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsSystem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsSystem)
	afx_msg void OnUseSystemTray();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnKillActive();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
