/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "corelib/ItemData.h"

class CItemData;

class CEditShortcutDlg : public CPWDialog
{

public:
  // default constructor
  CEditShortcutDlg(CItemData *ci, CWnd* pParent = NULL);
  virtual ~CEditShortcutDlg();

  enum { IDD = IDD_EDIT_SHORTCUT };
  CMyString m_defusername, m_username, m_base;
  bool m_Edit_IsReadOnly;
  int m_ibasedata;
  uuid_array_t m_base_uuid;


private:
  CItemData *m_ci; // The shortcut being edited
  CMyString m_group;
  CMyString m_title;
  CMyString m_target, m_oldtarget;
  CMyString m_locCTime, m_locPMTime, m_locATime, m_locRMTime;
  bool m_bIsModified;

  CComboBoxExtn m_ex_group;
  CEditExtn m_ex_title;
  CEditExtn m_ex_username;
  CEditExtn m_ex_target;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  virtual void OnOK();
  virtual BOOL OnInitDialog();
  afx_msg void OnHelp();

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnBnClickedOk();
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
