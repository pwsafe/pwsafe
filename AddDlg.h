/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

class CAddDlg : public CPWDialog
{
  // Construction
public:
  CAddDlg(CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CAddDlg)
  enum { IDD = IDD_ADD };
  CMyString m_password, m_password2;
  CMyString m_notes;
  CMyString m_username;
  CMyString m_title;
  CMyString m_group;
  CMyString m_URL;
  CMyString m_autotype;
  CMyString m_locLTime;
  time_t m_tttLTime;
  BOOL m_SavePWHistory;
  int m_MaxPWHistory;
  int m_ibasedata;
  uuid_array_t m_base_uuid;
  PWPolicy m_pwp;

  void  ShowPassword();
  void  HidePassword();

  CComboBoxExtn m_ex_group;
  CEditExtn m_ex_password, m_ex_password2;
  CEditExtn m_ex_notes;
  CEditExtn m_ex_username;
  CEditExtn m_ex_title;
  CEditExtn m_ex_URL;
  CEditExtn m_ex_autotype;

  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAddDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

private:
  bool m_isPwHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  void ResizeDialog();
  static CString CS_SHOW, CS_HIDE;

  // Implementation
protected:

  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAddDlg)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnRandom();
  afx_msg void OnShowpassword();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
  afx_msg void OnBnClickedClearLTime();
  afx_msg void OnBnClickedSetLTime();
  afx_msg void OnCheckedSavePasswordHistory();
  CButton m_moreLessBtn;
  BOOL m_OverridePolicy;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
