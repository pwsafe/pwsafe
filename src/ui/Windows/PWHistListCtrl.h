/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CPWHistListCtrl : public CListCtrl
{
public:
  CPWHistListCtrl() {}
  virtual ~CPWHistListCtrl() {}
  void UpdateRowHeight(bool bInvalidate);
protected:
  //{{AFX_MSG(CPWHistListCtrl)
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
  afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  //}}AFX_MSG
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
  DECLARE_MESSAGE_MAP()
};
