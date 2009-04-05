/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "SHCTHotKey.h"

class CSHCTListCtrl;

// SHCTHotKey

class CSHCTHotKey : public CHotKeyCtrl
{
  DECLARE_DYNAMIC(CSHCTHotKey)

public:
  CSHCTHotKey();
  virtual ~CSHCTHotKey();

  void SetMyParent(CSHCTListCtrl *pParent)
  {m_pParent = pParent;}

protected:
  afx_msg void OnKillFocus(CWnd *pWnd);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags); 
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags); 

  DECLARE_MESSAGE_MAP()

private:
  CSHCTListCtrl *m_pParent;
  bool m_bHandled;
};
