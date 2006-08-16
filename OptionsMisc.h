#pragma once

// OptionsMisc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc dialog

class COptionsMisc : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsMisc)

// Construction
public:
	COptionsMisc();
	~COptionsMisc();

// Dialog Data
	//{{AFX_DATA(COptionsMisc)
	enum { IDD = IDD_PS_MISC };
	BOOL	m_confirmdelete;
	BOOL	m_saveimmediately;
	BOOL	m_maintaindatetimestamps;
	BOOL	m_escexits;
	BOOL    m_hotkey_enabled;
	// JHF : class CHotKeyCtrl not supported by WinCE
#if !defined(POCKET_PC)
	CHotKeyCtrl	m_hotkey;
#endif
	CComboBox m_dblclk_cbox;
	BOOL	m_usedefuser;
	BOOL	m_querysetdef;
	CString	m_defusername;
	BOOL    m_continuefindateodb;
	//}}AFX_DATA
	DWORD m_hotkey_value;
	int     m_doubleclickaction;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsMisc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	// Generated message map functions
	//{{AFX_MSG(COptionsMisc)
	afx_msg void OnEnableHotKey();
	afx_msg void OnUsedefuser();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedMaintaindatetimestamps();
	afx_msg void OnComboChanged();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
