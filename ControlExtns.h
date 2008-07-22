/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once
#include "corelib/MyString.h" // for CSecEditExtn
#include "InfoDisplay.h"      // for Listbox Tooltips
#include <vector>             // for Listbox Tooltips

// ControlExtns.h : header file
// Extensions to standard Static, Edit, ListBox and Combobox Controls


// timer event numbers used to by ControlExtns for ListBox tooltips.
#define TIMER_LB_HOVER     0x0A
#define TIMER_LB_SHOWING   0x0B 

/*
HOVER_TIME_LB       The length of time the pointer must remain stationary
                    within a tool's bounding rectangle before the tool tip
                    window appears.

TIMEINT_LB_SHOWING The length of time the tool tip window remains visible
                   if the pointer is stationary within a tool's bounding
                   rectangle.
*/
#define HOVER_TIME_LB      1000
#define TIMEINT_LB_SHOWING 5000

class CStaticExtn : public CStatic
{
  // Construction
public:
  CStaticExtn();
  void SetColour(COLORREF cfUser)
  {m_bUserColour = TRUE; m_cfUser = cfUser;}
  void ResetColour()
  {m_bUserColour = FALSE;}
  BOOL GetColourState()
  {return m_bUserColour;}

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
  CEditExtn(COLORREF focusColor = (RGB(222, 255, 222))); // light green
  CEditExtn(int message_number, LPCTSTR szmenustring,
            COLORREF focusColor = (RGB(222, 255, 222))); //light green
  void ChangeColour() {m_bIsFocused = TRUE;}

  // Attributes
private:
  BOOL m_bIsFocused;

  CBrush m_brInFocus;
  CBrush m_brNoFocus;
  const COLORREF m_crefInFocus;

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

// Following is meant for sensitive information that you really don't
// want to be in memory more than necessary, such as master passwords
// We use a CSecEditExtn::Impl class member not for security, but to
// avoid #including stuff here that really shouldn't be of interest to
// users of these classes

class CSecEditExtn : public CEditExtn
{
 public:
  CSecEditExtn();
  CSecEditExtn(int message_number, LPCTSTR szmenustring);
  virtual ~CSecEditExtn();
  // Overriding virtuals doesn't work, due to defective
  // implementation of DDX_Text. Grr.
  void DoDDX(CDataExchange *pDX, CMyString &str);
  void SetSecure(bool on_off); // on by default
  bool GetSecure() const {return m_secure;}
  CMyString GetSecureText() const;
  void SetSecureText(const CMyString &str);

 protected:
  DECLARE_MESSAGE_MAP();
  afx_msg void OnUpdate();
 private:
  void OnSecureUpdate();
  struct Impl;
  Impl *m_impl;
  bool m_secure;
  bool m_in_recursion;
};

class CComboBoxExtn;

class CListBoxExtn : public CListBox
{
  // Construction
public:
  CListBoxExtn();
  void ChangeColour() {m_bIsFocused = TRUE;}
  void ActivateToolTips();
  void SetCombo(CComboBoxExtn *pCombo) {m_pCombo = pCombo;}

  // Attributes
private:
  BOOL m_bIsFocused;

  CBrush m_brInFocus;
  CBrush m_brNoFocus;

  bool ShowToolTip(int nItem, const bool bVisible);

  CComboBoxExtn *m_pCombo;
  CInfoDisplay *m_pLBToolTips;
  UINT m_nHoverLBTimerID, m_nShowLBTimerID;
  CPoint m_HoverLBPoint;
  int m_HoverLBnItem;
  bool m_bUseToolTips, m_bMouseInWindow;

  // Operations
public:

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CListBoxExtn)
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
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

class CComboBoxExtn : public CComboBox
{
  // Construction
public:
  CComboBoxExtn();
  void SetToolTipStrings(std::vector<CMyString> vtooltips);
  CMyString GetToolTip(int nItem)
  {return m_vtooltips[nItem];}

private:
  bool m_bUseToolTips;
  std::vector<CMyString> m_vtooltips;

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
public:
  virtual ~CComboBoxExtn();

protected:
  //{{AFX_MSG(CComboBoxExtn)
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
