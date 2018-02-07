/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWTouch.h"
#include "SecString.h"        // for CSecEditExtn
#include <vector>             // for Listbox Tooltips & EditExtn menus

// ControlExtns.h : header file
// Extensions to standard Static, Edit, ListBox and Combobox Controls

// Pick a number at the end of the WM_USER range
#define EM_SELECTALL (WM_APP - 1)

class CStaticExtn : public CStatic
{
  // Construction
public:
  CStaticExtn();
  virtual ~CStaticExtn();

  void SetColour(COLORREF cfUser)
  {m_bUserColour = true; m_cfUser = cfUser;}
  void ResetColour()
  {m_bUserColour = false;}
  void SetBkColour(COLORREF cfBkUser);
  void ResetBkColour();

  void FlashBkgnd(COLORREF cfFlashColour);
  void SetHighlight(bool bHighlight, COLORREF cfHighlightColour)
  {m_bHighlight = bHighlight; m_cfHighlightColour = cfHighlightColour;}

  inline int IsFlashing() {return m_iFlashing;}
  inline bool GetColourState()  {return m_bUserColour;}
  inline bool IsHighlighted() {return m_bHighlight;}
  inline bool IsMouseInWindow() {return m_bMouseInWindow;}
  inline COLORREF GetFlashColour() {return m_cfFlashColour;}
  inline COLORREF GetHighlightColour() {return m_cfHighlightColour;}
  inline COLORREF GetUserColour() {return m_cfUser;}

protected:
  //{{AFX_MSG(CStaticExtn)
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  int m_iFlashing;
  COLORREF m_cfUser, m_cfOldColour, m_cfFlashColour, m_cfHighlightColour, m_cfBkUser;
  bool m_bUserColour, m_bMouseInWindow, m_bHighlight, m_bUserBkColour;
  CBrush m_brBkUser;
};

struct st_context_menu {
  UINT_PTR message_number;
  std::wstring menu_string;
  int flags;
  WPARAM wParam;
  LPARAM lParam;
  bool bEnable;

  st_context_menu() : message_number(0), menu_string(L""), 
    flags(0), wParam(0), lParam(0), bEnable(true) {}

  // copy c'tor and assignment operator, standard idioms
  st_context_menu(const st_context_menu &that)
    : message_number(that.message_number),
    menu_string(that.menu_string),
    flags(that.flags),
    wParam(that.wParam),
    lParam(that.lParam),
    bEnable(that.bEnable) {}

  st_context_menu &operator=(const st_context_menu &that)
  {
    if (this != &that) {
      message_number  = that.message_number;
      menu_string = that.menu_string;
      flags  = that.flags;
      wParam  = that.wParam;
      lParam = that.lParam;
      bEnable = that.bEnable;
    }
    return *this;
  }

  bool operator==(const st_context_menu &that) const;

  bool operator!=(const st_context_menu &that) const
  {return !(*this == that);}

  void Empty()
  { 
    message_number = 0;
    menu_string.clear();
    flags = 0;
    wParam = 0;
    lParam = 0;
    bEnable = true;
  }
};

class CEditExtnX : public CEdit
{
  // Construction
public:
  CEditExtnX(COLORREF focusColor = (RGB(222, 255, 222))); // light green
  CEditExtnX(std::vector<st_context_menu> vmenu_items, 
            COLORREF focusColor = (RGB(222, 255, 222))); //light green
  virtual ~CEditExtnX();

  void ChangeColour() {m_bIsFocused = TRUE;}
  void UpdateState(const int message_number, const bool new_state);

  DWORD GetSel();
  void GetSel(int &nStartChar, int &nEndChar);
  void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);
  void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE);
  void EnableMenuItem(const int message_number, const bool bEnable);

protected:
  //{{AFX_MSG(CEditExtnX)
  afx_msg void OnSetFocus(CWnd* pOldWnd);
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  // Attributes
private:
  BOOL m_bIsFocused;

  CBrush m_brInFocus;
  CBrush m_brNoFocus;
  const COLORREF m_crefInFocus;

  int m_lastposition, m_nStartChar, m_nEndChar;
  std::vector<st_context_menu> m_vmenu_items;
};

/**
* typedef to hide the fact that CEditExtn is really a mixin.
*/

typedef CPWTouch< CEditExtnX > CEditExtn;

class CRichEditExtnX : public CRichEditCtrl
{
  // Construction
public:
  CRichEditExtnX(COLORREF focusColor = (RGB(222, 255, 222))); // light green
  virtual ~CRichEditExtnX();

  void SetContextMenu(const std::vector<st_context_menu> &vmenu_items);
  void ChangeColour() {m_bIsFocused = TRUE;}
  void UpdateState(const int message_number, const bool new_state);

  void EnableMenuItem(const int message_number, const bool bEnable);

protected:
  //{{AFX_MSG(CRichEditExtnX)
  afx_msg void OnSetFocus(CWnd *pOldWnd);
  afx_msg void OnKillFocus(CWnd *pNewWnd);
  afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
  afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  // Attributes
private:
  BOOL m_bIsFocused;
  const COLORREF m_crefInFocus;

  std::vector<st_context_menu> m_vmenu_items;
  
  bool m_bContextMenu;
  HCURSOR m_hCursor;
};

/**
* typedef to hide the fact that CRichEditExtn is really a mixin.
*/

typedef CPWTouch< CRichEditExtnX > CRichEditExtn;

// Following is meant for sensitive information that you really don't
// want to be in memory more than necessary, such as master passwords
// We use a CSecEditExtn::Impl class member not for security, but to
// avoid #including stuff here that really shouldn't be of interest to
// users of these classes

class CSecEditExtn : public CEditExtnX
{
public:
  CSecEditExtn();
  CSecEditExtn(std::vector<st_context_menu> vmenu_items);
  virtual ~CSecEditExtn();

  // Overriding virtuals doesn't work, due to defective
  // implementation of DDX_Text. Grr.
  void DoDDX(CDataExchange *pDX, CSecString &str);
  void SetSecure(bool on_off); // on by default
  bool GetSecure() const {return m_secure;}
  CSecString GetSecureText() const;
  void SetSecureText(const CSecString &str);

protected:
  //{{AFX_MSG(CSecEditExtn)
  afx_msg void OnUpdate();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP();

private:
  void OnSecureUpdate();
  struct Impl;
  Impl *m_pImpl;
  bool m_secure;
  bool m_in_recursion;
};

class CComboBoxExtn;
class CInfoDisplay;

class CListBoxExtn : public CListBox
{
  // ONLY used as the ListBox within CComboBoxExtn
  // Construction
public:
  CListBoxExtn();
  virtual ~CListBoxExtn();

  void ChangeColour() {m_bIsFocused = TRUE;}
  void ActivateToolTips();
  void SetCombo(CComboBoxExtn *pCombo) {m_pCombo = pCombo;}

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

  // Attributes
private:
  BOOL m_bIsFocused;

  CBrush m_brInFocus;
  CBrush m_brNoFocus;

  bool ShowToolTip(int nItem, const bool bVisible);

  CComboBoxExtn *m_pCombo;
  CInfoDisplay *m_pLBToolTips;
  UINT_PTR m_nHoverLBTimerID, m_nShowLBTimerID;
  CPoint m_HoverLBPoint;
  int m_HoverLBnItem;
  bool m_bUseToolTips, m_bMouseInWindow;
};

class CComboBoxExtn : public CComboBox
{
  // Construction
public:
  CComboBoxExtn();
  virtual ~CComboBoxExtn();

  void SetToolTipStrings(std::vector<CSecString> vtooltips);
  CSecString GetToolTip(int nItem)
  {return m_vtooltips[nItem];}

  /*
    Based on original "CComboBox with separators" code
    CSeparatorComboBox class, created by: Zuoliu Ding in 01/30/2004
    (C) Copyright 2004 Zuoliu Ding.
    See: http://www.codeproject.com/Articles/7356/A-separator-combo-box
    Covered by The Code Project Open License (CPOL) 1.02
    
    The original code has been merged into this code and has been updated for
    PasswordSafe use as follows:
      1. Changed to use std::vector instead of CArray.
      2. Extra member functions: SetSeparator() and ClearSeparators()
      3. Changed some default values
  */

  void SetSeparator(int iSep);
  void SetSeparator();
  void ClearSeparators() { m_vSeparators.clear(); }
  void AdjustItemHeight(int nInc = 3);

  void SetSepLineStyle(int iSep) { m_nPenStyle = iSep; }
  void SetSepLineColor(COLORREF crColor) { m_crColor = crColor; }
  void SetSepLineWidth(int iWidth) { m_nSepWidth = iWidth; }
  void SetBottomMargin(int iMargin) { m_nBottomMargin = iMargin; }
  void SetHorizontalMargin(int iMargin) { m_nHorizontalMargin = iMargin; }

  CEditExtnX m_edit;
  CListBoxExtn m_listbox;
  void ChangeColour();

protected:
  //{{AFX_MSG(CComboBoxExtn)
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  afx_msg void OnDestroy();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool m_bUseToolTips;
  std::vector<CSecString> m_vtooltips;
  std::vector<int> m_vSeparators;

  int m_nHorizontalMargin, m_nBottomMargin, m_nSepWidth, m_nPenStyle, m_crColor;
};

/////////////////////////////////////////////////////////////////////////////
// CSymbolEdit

class CSymbolEdit : public CEdit
{
  // Construction
public:
  CSymbolEdit();
  void SetValidSym(const std::wstring &s);

protected:
  //{{AFX_MSG(CSymbolEdit)
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
private:
  std::wstring m_validSym; // defaults to CPasswordCharPool::GetDefaultSymbols()
};

/////////////////////////////////////////////////////////////////////////////
// CButtonExtn

class CButtonExtn : public CButton
{
  // Construction
public:
  CButtonExtn();
  virtual ~CButtonExtn();

  void SetTextColour(COLORREF crf)
  {m_bUseTextColour = true; m_crfText = crf;}
  void ResetTextColour()
  {m_bUseTextColour = false;}
  void SetBkgColour(int icolour)
  {m_bUseBkgColour = true; m_icolour = icolour;}
  void ResetBkgColour()
  {m_bUseBkgColour = false;}
  void SetType(int type);

protected:
  //{{AFX_MSG(CButtonExtn)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void DrawButton(HWND hWnd, HDC hDC, RECT *pRect, BOOL fChecked, BOOL fHot, BOOL fFocus);

  CString m_caption;
  bool m_bUseTextColour, m_bUseBkgColour;
  COLORREF m_crfText;
  int m_icolour;
  int m_type;
};

/////////////////////////////////////////////////////////////////////////////
// CButtonBitmapExtn

class CButtonBitmapExtn : public CButton
{
  // Construction
public:
  CButtonBitmapExtn();
  virtual ~CButtonBitmapExtn();

  void SetBitmapMaskAndID(const COLORREF cfMAsk, const int IDB)
  {
    m_cfMAsk = cfMAsk;  m_IDB = IDB;
  }

protected:
  //{{AFX_MSG(CButtonBitmapExtn)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

  int m_IDB;
  COLORREF m_cfMAsk;
};

// Common to dialog and property page:
void FixBitmapBackground(CBitmap &bm);
