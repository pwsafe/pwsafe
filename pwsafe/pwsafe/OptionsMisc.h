#if !defined(AFX_OPTIONSMISC_H__CB2E89DC_192A_49C6_83B5_AFC7ED368CB0__INCLUDED_)
#define AFX_OPTIONSMISC_H__CB2E89DC_192A_49C6_83B5_AFC7ED368CB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
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
	BOOL	m_escexits;
	int     m_doubleclickaction;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsMisc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsMisc)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSMISC_H__CB2E89DC_192A_49C6_83B5_AFC7ED368CB0__INCLUDED_)
