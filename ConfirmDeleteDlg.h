/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// ConfirmDeleteDlg.h
//-----------------------------------------------------------------------------
#include "corelib/PwsPlatform.h"

#include "PWDialog.h"
class CConfirmDeleteDlg : public CPWDialog
{
public:
  CConfirmDeleteDlg(CWnd* pParent = NULL, int numchildren = 0);

private:
  // Dialog Data
  //{{AFX_DATA(CConfirmDeleteDlg)
  enum { IDD = IDD_CONFIRMDELETE_DIALOG };
  bool m_dontaskquestion;
  int m_numchildren;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CConfirmDeleteDlg)

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CConfirmDeleteDlg)
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
