/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "InfoDisplay.h"      // for Tooltips

// CPWStatusBar

class CPWStatusBar : public CStatusBar
{
  DECLARE_DYNAMIC(CPWStatusBar)

  enum {SB_DBLCLICK = 0, SB_CLIPBOARDACTION,
#if defined(_DEBUG) || defined(DEBUG)
        SB_CONFIG,
#endif
        SB_MODIFIED, SB_READONLY, SB_NUM_ENT, SB_FILTER,
        SB_TOTAL /* this must be the last entry */};

public:
  CPWStatusBar();
  virtual ~CPWStatusBar();
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
  virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

  void SetFilterStatus(const bool bStatus)
  {m_bSTBFilterStatus = bStatus;}

  int GetBitmapWidth()
  {return m_bmWidth;}

  void SetFileStatus(const bool bFileOpen, const bool bFileReadOnly)
  {m_bFileOpen = bFileOpen; m_bFileReadOnly = bFileReadOnly;}

protected:
  //{{AFX_MSG(CPWStatusBar)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool ShowToolTip(int nPane, const bool bVisible);

  CInfoDisplay *m_pSBToolTips;
  bool m_bSTBFilterStatus;
  int m_bmHeight, m_bmWidth;
  CBitmap m_FilterBitmap;

  UINT_PTR m_nHoverSBTimerID, m_nShowSBTimerID;
  CPoint m_HoverSBPoint;
  int m_HoverSBnPane;
  bool m_bUseToolTips, m_bMouseInWindow, m_bFileReadOnly, m_bFileOpen;
};
