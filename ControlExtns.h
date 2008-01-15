/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// ControlExtns.h : header file
// Extensions to standard Static, Edit, ListBox and Combobox Controls

// Pick a number at the end of the WM_USER range
#define EM_SELECTALL (WM_APP - 1)

class CStaticExtn : public CStatic
{
  // Construction
public:
  CStaticExtn();
  void SetColour(COLORREF cfUser)
  {m_bUserColour = TRUE; m_cfUser = cfUser;}
  void ResetColour()
  {m_bUserColour = FALSE;}

  // Attributes
private:
  BOOL m_bUserColour;
  COLORREF m_cfUser;

  // Operations
public:

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CEditEx)
  //}}AFX_VIRTUAL

  // Implementation
public:
  virtual ~CStaticExtn();

  // Generated message map functions
protected:
  //{{AFX_MSG(CEditExtn)
  afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

class CEditExtn : public CEdit
{
  // Construction
public:
  CEditExtn();
  CEditExtn(int message_number, LPCTSTR szmenustring);
  void ChangeColour() {m_bIsFocused = TRUE;}

  // Attributes
private:
  BOOL m_bIsFocused;

  CBrush m_brInFocus;
  CBrush m_brNoFocus;

  int m_lastposition, m_nStartChar, m_nEndChar;
  int m_message_number;
  CString m_menustring;

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
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
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

  CBrush m_brInFocus;
  CBrush m_brNoFocus;

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
