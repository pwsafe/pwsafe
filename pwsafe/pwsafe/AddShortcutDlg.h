/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file AddShortcutDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "ControlExtns.h"

class CAddShortcutDlg : public CPWDialog
{
  // Construction
public:
  CAddShortcutDlg(CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CAddShortcutDlg)
  enum { IDD = IDD_ADD_SHORTCUT };
  CMyString m_target;
  CMyString m_username;
  CMyString m_title;
  CMyString m_group;
  int m_ibasedata;
  uuid_array_t m_base_uuid;


  CComboBoxExtn m_ex_group;
  CEditExtn m_ex_target;
  CEditExtn m_ex_username;
  CEditExtn m_ex_title;

  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAddShortcutDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:

  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAddDlg)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOk();

};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
