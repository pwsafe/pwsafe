/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

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
	BOOL    m_notesshowinedit;
#if defined(POCKET_PC)
	BOOL	m_dcshowspassword;
#endif
	int     m_treedisplaystatusatopen;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
