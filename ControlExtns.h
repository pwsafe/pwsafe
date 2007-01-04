/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// ControlExtns.h : header file
// Extensions to standard Edit, ListBox and Combobox Controls

class CEditExtn : public CEdit
{
// Construction
public:
	CEditExtn();
	void ChangeColour() {m_bIsFocused = TRUE;}

// Attributes
private:
	BOOL m_bIsFocused;

	CBrush brInFocus;
	CBrush brNoFocus;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditExtn();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditExtn)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CListBoxExtn : public CListBox
{
// Construction
public:
	CListBoxExtn();
	void ChangeColour() {m_bIsFocused = TRUE;}

// Attributes
private:
	BOOL m_bIsFocused;

	CBrush brInFocus;
	CBrush brNoFocus;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListBoxExtn();

	// Generated message map functions
protected:
	//{{AFX_MSG(CListBoxExtn)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CComboBoxExtn : public CComboBox
{
public:
	CEditExtn m_edit;
	CListBoxExtn m_listbox;

// Operations
public:
	void ChangeColour();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboBoxExtn)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CComboBoxExtn)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
