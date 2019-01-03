/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// EditShortcutDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "ControlExtns.h"
#include "core/ItemData.h"

class CItemData;

class CEditShortcutDlg : public CPWDialog
{

public:
  // default constructor
  CEditShortcutDlg(CItemData *pci, CWnd* pParent = NULL,
    const CSecString &cs_tg = L"", const CSecString &cs_tt = L"", 
    const CSecString &cs_tu = L"");
  virtual ~CEditShortcutDlg();

  bool IsEntryModified() {return m_bIsModified;}

  enum { IDD = IDD_EDIT_SHORTCUT };
  CSecString m_defusername, m_username;
  bool m_Edit_IsReadOnly;

private:
  void SetGroupComboBoxWidth();

  CItemData *m_pci; // The shortcut being edited
  CSecString m_group;
  CSecString m_title;
  // target's group, title, user
  CSecString m_tg, m_tt, m_tu;
  CSecString m_locCTime, m_locPMTime, m_locATime, m_locRMTime;
  bool m_bIsModified;

  CComboBoxExtn m_ex_group;
  CEditExtn m_ex_title;
  CEditExtn m_ex_username;

  static bool m_bShowUUID;

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnHelp();
  afx_msg void OnOK();

  DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
