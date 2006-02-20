#if !defined(AFX_OPTIONSDISPLAY_H__74060034_DADF_4333_B442_D1B0EBEF562E__INCLUDED_)
#define AFX_OPTIONSDISPLAY_H__74060034_DADF_4333_B442_D1B0EBEF562E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDisplay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay dialog

class COptionsDisplay : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsDisplay)

// Construction
public:
	COptionsDisplay();
	~COptionsDisplay();

// Dialog Data
	//{{AFX_DATA(COptionsDisplay)
	enum { IDD = IDD_PS_DISPLAY };
	BOOL	m_alwaysontop;
	BOOL	m_pwshowinlist;
	BOOL	m_pwshowinedit;
#if defined(POCKET_PC)
	BOOL	m_dcshowspassword;
#endif
	BOOL    m_usesystemtray;
	int		m_maxmruitems;
	BOOL	m_mruonfilemenu;
	BOOL    m_Display_Expanded;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsDisplay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsDisplay)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDISPLAY_H__74060034_DADF_4333_B442_D1B0EBEF562E__INCLUDED_)
