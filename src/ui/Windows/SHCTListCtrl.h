/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWTouch.h"
#include "GridListCtrl.h"
#include "MenuShortcuts.h"

class COptionsShortcuts;
class CSHCTHotKey;

// Subitem indices
#define SHCT_SHORTCUTKEYS  0
#define SHCT_MENUITEMTEXT  1
#define SHCT_NUM_COLUMNS   2

class CSHCTListCtrlX : public CGridListCtrl
{
public:
  CSHCTListCtrlX();
  ~CSHCTListCtrlX();

  void Init(COptionsShortcuts *pParent);

  void SaveHotKey();
  bool IsHotKeyActive() {return m_bHotKeyActive;}

  void OnMenuShortcutKillFocus(const WORD wVirtualKeyCode, const WORD wPWSModifiers);

protected:
  //{{AFX_MSG(CSHCTListCtrlX)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  COptionsShortcuts *m_pParent;

  CSHCTHotKey *m_pHotKey;
  int m_item;
  UINT m_id;
  bool m_bHotKeyActive;
  COLORREF m_crWindowText, m_crRedText;
};

/**
* typedef to hide the fact that CSHCTListCtrl is really a mixin.
*/

typedef CPWTouch< CSHCTListCtrlX > CSHCTListCtrl;
