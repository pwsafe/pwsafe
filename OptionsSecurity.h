#if !defined(AFX_OPTIONSSECURITY_H__C7A81288_E3D7_487A_BD39_B66B5272FCF9__INCLUDED_)
#define AFX_OPTIONSSECURITY_H__C7A81288_E3D7_487A_BD39_B66B5272FCF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsSecurity.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity dialog

class COptionsSecurity : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsSecurity)

// Construction
public:
	COptionsSecurity();
	~COptionsSecurity();

// Dialog Data
	//{{AFX_DATA(COptionsSecurity)
	enum { IDD = IDD_PS_SECURITY };
	BOOL	m_clearclipboard;
	BOOL	m_lockdatabase;
	BOOL	m_confirmsaveonminimize;
	BOOL	m_confirmcopy;
	BOOL	m_LockOnWindowLock;
	BOOL	m_LockOnIdleTimeout;
	UINT    m_IdleTimeOut;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsSecurity)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsSecurity)
	afx_msg void OnLockbase();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSSECURITY_H__C7A81288_E3D7_487A_BD39_B66B5272FCF9__INCLUDED_)
