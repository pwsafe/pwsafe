#if !defined(AFX_OPTIONSPASSWORDPOLICY_H__62F727BC_2E3E_4299_BB17_6FFD9AC73C23__INCLUDED_)
#define AFX_OPTIONSPASSWORDPOLICY_H__62F727BC_2E3E_4299_BB17_6FFD9AC73C23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsPasswordPolicy.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy dialog

class COptionsPasswordPolicy : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPasswordPolicy)

// Construction
public:
	COptionsPasswordPolicy();
	~COptionsPasswordPolicy();

// Dialog Data
	//{{AFX_DATA(COptionsPasswordPolicy)
	enum { IDD = IDD_PS_PASSWORDPOLICY };
	UINT	m_pwlendefault;
	BOOL	m_pwusedigits;
	BOOL	m_pwuselowercase;
	BOOL	m_pwusesymbols;
	BOOL	m_pwuseuppercase;
	BOOL	m_pweasyvision;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPasswordPolicy)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPasswordPolicy)
	virtual BOOL OnInitDialog();
	afx_msg void OnUselowercase();
	afx_msg void OnUseuppercase();
	afx_msg void OnUsedigits();
	afx_msg void OnUsesymbols();
   afx_msg BOOL OnKillActive();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSPASSWORDPOLICY_H__62F727BC_2E3E_4299_BB17_6FFD9AC73C23__INCLUDED_)
