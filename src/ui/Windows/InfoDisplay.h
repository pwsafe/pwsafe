/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
*  This is nearly an exact copy of CInfoDisplay class in Asynch Explorer by
*  Joseph M. Newcomer [MVP]; http://www.flounder.com
*  Only minor formatting and naming changes have been made to fit in with this
*  project.
*/

#pragma once

// CInfoDisplay

class CInfoDisplay : public CWnd
{
  DECLARE_DYNAMIC(CInfoDisplay)

public:
  CInfoDisplay(bool use_current_monitor=true);
  virtual ~CInfoDisplay();
  
  BOOL Create(int x, int y, LPCWSTR caption, CWnd * parent);

  void SetWindowTextFont(CFont *pFont);

protected:
  virtual void PostNcDestroy();
  
  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
  afx_msg LRESULT OnGetFont(WPARAM, LPARAM);
  
  DECLARE_MESSAGE_MAP()
  
  CFont *m_pTextFont;
  HFONT m_font;
  bool m_use_current_monitor;
};
