/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CShowCompareDlg;

class CSCWListCtrl : public CListCtrl
{
public:
  CSCWListCtrl();
  ~CSCWListCtrl();

  void Initialize();

  enum {REDTEXT = 0x1000, PASSWORDFONT = 0x2000, NOTES = 0x4000};

protected:
  //{{AFX_MSG(CSCWListCtrl)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CShowCompareDlg *m_pParent;

  UINT_PTR m_nHoverNDTimerID, m_nShowNDTimerID;
  bool m_bMouseInWindow;
  CPoint m_HoverNDPoint;
};
