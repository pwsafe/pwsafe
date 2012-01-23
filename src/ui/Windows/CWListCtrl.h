/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CCWListCtrl : public CListCtrl
{
public:
  CCWListCtrl();
  ~CCWListCtrl();

protected:
  //{{AFX_MSG(CCWListCtrl)
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  //}}AFX_MSG

  LRESULT OnCharItemlist(WPARAM wParam, LPARAM lParam);

  DECLARE_MESSAGE_MAP()

private:
  bool FindNext(const CString &cs_find, const int iSubItem);

  CString m_csFind;
  UINT_PTR m_FindTimerID;
};
