/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CAddEdit_Additional;

// EntryKBHotKey

class CEntryKBHotKey : public CHotKeyCtrl
{
  DECLARE_DYNAMIC(CEntryKBHotKey)

public:
  CEntryKBHotKey();
  virtual ~CEntryKBHotKey();

  void SetMyParent(CAddEdit_Additional *pParent)
  {m_pParent = pParent;}

protected:
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  // Needed to Add/Remove application HotKey
  afx_msg void OnKillFocus(CWnd *pWnd);
  afx_msg void OnSetFocus(CWnd *pWnd);

  DECLARE_MESSAGE_MAP()

private:
  CAddEdit_Additional *m_pParent;
};
