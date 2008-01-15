/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasskeyChangeDlg.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"
#include "PWDialog.h"

class CPasskeyChangeDlg : public CPWDialog
{
  // Construction
public:
  CPasskeyChangeDlg(CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CPasskeyChangeDlg)
  enum { IDD = IDD_KEYCHANGE_DIALOG };
  CMyString	m_confirmnew;
  CMyString	m_newpasskey;
  CMyString	m_oldpasskey;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CPasskeyChangeDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CPasskeyChangeDlg)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnHelp();
#if defined(POCKET_PC)
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();
#endif
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
