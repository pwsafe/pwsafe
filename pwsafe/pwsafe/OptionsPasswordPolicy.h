/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once
#include "afxwin.h"

// OptionsPasswordPolicy.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy dialog
#include "PWPropertyPage.h"

class COptionsPasswordPolicy : public CPWPropertyPage
{
	DECLARE_DYNCREATE(COptionsPasswordPolicy)

// Construction
public:
	COptionsPasswordPolicy();
	~COptionsPasswordPolicy();

  const TCHAR *GetHelpName() const {return _T("password_policies");}
    
// Dialog Data
	//{{AFX_DATA(COptionsPasswordPolicy)
	enum { IDD = IDD_PS_PASSWORDPOLICY };
	UINT	m_pwdefaultlength;
	BOOL	m_pwuselowercase;
	BOOL	m_pwuseuppercase;
	BOOL	m_pwusedigits;
	BOOL	m_pwusesymbols;
	BOOL	m_pweasyvision;
	BOOL	m_pwusehexdigits;
  BOOL  m_pwmakepronounceable;
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
	afx_msg void OnUsehexdigits();
	afx_msg BOOL OnKillActive();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  void do_nohex(bool mode); // mode == true enable non-hex
  enum {N_NOHEX = 6}; // number of checkboxes disabled when hex chosen
  static const int nonHex[N_NOHEX]; // IDs of said checkboxes
  BOOL m_save[N_NOHEX]; // save cb's state when disabling hex
};
