/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file AddDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "ControlExtns.h"
#include "corelib/ItemData.h"

class DboxMain;

class CAddDlg : public CPWDialog
{
  // Construction
public:
  CAddDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CAddDlg();

  // Dialog Data
  //{{AFX_DATA(CAddDlg)
  enum { IDD = IDD_ADD };
  CSecString m_password, m_password2;
  CSecString m_notes;
  CSecString m_notesww;
  CSecString m_username;
  CSecString m_title;
  CSecString m_group;
  CSecString m_URL;
  CSecString m_autotype;
  CSecString m_executestring;
  CSecString m_locXTime;
  time_t m_tttXTime;
  time_t m_tttCPMTime;  // Password creation or last changed datetime
  int m_XTimeInt;
  BOOL m_SavePWHistory;
  size_t m_MaxPWHistory;
  int m_ibasedata;
  uuid_array_t m_base_uuid;
  PWPolicy m_pwp;

  void  ShowPassword();
  void  HidePassword();

  CComboBoxExtn m_ex_group;
  CSecEditExtn m_ex_password, m_ex_password2;
  CEditExtn *m_pex_notes;
  CEditExtn *m_pex_notesww;
  CEditExtn m_ex_username;
  CEditExtn m_ex_title;
  CEditExtn m_ex_URL;
  CEditExtn m_ex_autotype;
  CEditExtn m_ex_executestring;
  //}}AFX_DATA
  BOOL m_OverridePolicy;
  BOOL m_bWordWrap;

  // Overrides
  // Implementation
protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual void OnCancel();
  virtual void OnOK();
  // Generated message map functions
  //{{AFX_MSG(CAddDlg)
  afx_msg void OnHelp();
  afx_msg void OnRandom();
  afx_msg void OnShowpassword();
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
  afx_msg void OnBnClickedClearXTime();
  afx_msg void OnBnClickedSetXTime();
  afx_msg void OnCheckedSavePasswordHistory();
  afx_msg void OnBnClickedOverridePolicy();
  afx_msg LRESULT OnWordWrap(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;
  void SelectAllNotes();

  CButton m_moreLessBtn;
  bool m_isPwHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  void ResizeDialog();
  static CString CS_SHOW, CS_HIDE;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
