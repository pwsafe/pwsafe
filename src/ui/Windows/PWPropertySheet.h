/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class DboxMain;

class CPWPropertySheet : public CPropertySheet
{
public:
  CPWPropertySheet(UINT nID, CWnd* pParent, const bool bLongPPs);

  CPWPropertySheet(LPCTSTR pszCaption, CWnd* pParent, const bool bLongPPs);

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  // Following override to stop accelerators interfering
  virtual INT_PTR DoModal();
  virtual BOOL OnInitDialog();

protected:
  DECLARE_DYNAMIC(CPWPropertySheet)

  afx_msg void OnWindowPosChanging(WINDOWPOS *lpwndpos);
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

  DECLARE_MESSAGE_MAP()
  
private:
  // Used to determine if Tall will fit in OnInitDialog
  DboxMain *m_pDbx;
  bool m_bLongPPs;
  bool m_bKeepHidden;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
