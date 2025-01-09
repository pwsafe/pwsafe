/*
* Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class DboxMain; // for GetMainDlg()

class CPWPropertySheet : public CMFCPropertySheet
{
public:
  CPWPropertySheet(UINT nID, CWnd* pParent, const bool bLongPPs);

  CPWPropertySheet(LPCTSTR pszCaption, CWnd* pParent, const bool bLongPPs);

  // Following override to stop accelerators interfering (can't be protected as normal)
  INT_PTR DoModal();

protected:
  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
  // Following override to stop accelerators interfering
  virtual BOOL OnInitDialog() override;

  DECLARE_DYNAMIC(CPWPropertySheet)

  DboxMain *GetMainDlg() const;

  afx_msg void OnWindowPosChanging(WINDOWPOS *lpwndpos);
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu *pMenu);

  DECLARE_MESSAGE_MAP()
  
private:
  // Used to determine if Tall will fit in OnInitDialog
  bool m_bLongPPs;
  bool m_bKeepHidden;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
