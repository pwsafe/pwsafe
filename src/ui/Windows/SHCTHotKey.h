/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "SHCTListCtrl.h"

// SHCTHotKey

class CSHCTHotKey : public CHotKeyCtrl
{
  DECLARE_DYNAMIC(CSHCTHotKey)

public:
  CSHCTHotKey();
  virtual ~CSHCTHotKey();

  void SetLCParent(CSHCTListCtrl *pParent)
  {m_pLCParent = pParent;}
  void SetOSParent(COptionsShortcuts *pParent)
  {m_pOSParent = pParent;}

  void KillFocus() { OnKillFocus(NULL); }

protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnKillFocus(CWnd *pWnd);

  DECLARE_MESSAGE_MAP()

private:
  CSHCTListCtrl *m_pLCParent;
  COptionsShortcuts *m_pOSParent;
  bool m_bHandled;
};
