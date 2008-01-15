/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

class DboxMain;

class CPWListCtrl : public CListCtrl
{
public:
  CPWListCtrl();
  ~CPWListCtrl();

  void Initialize(CWnd *pWnd)
  {m_pDbx = (DboxMain *)pWnd;}

  LRESULT OnCharItemlist(WPARAM wParam, LPARAM lParam);
  bool FindNext(const CString &cs_find, const int iSubItem);

protected:
  //{{AFX_MSG(CPWListCtrl)
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;
  CString m_csFind;
  UINT_PTR m_FindTimerID;
};
