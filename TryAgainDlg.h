/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// TryAgainDlg.h
//-----------------------------------------------------------------------------
#include "PWDialog.h"

//Globally useful values...
enum
{
  TAR_OK,
  TAR_INVALID,
  TAR_NEW,
  TAR_OPEN,
  TAR_CANCEL,
  TAR_EXIT
};

//-----------------------------------------------------------------------------
class CTryAgainDlg : public CPWDialog
{
  // Construction
public:
  CTryAgainDlg(CWnd* pParent = NULL);   // standard constructor
  int GetCancelReturnValue();

  // Dialog Data
  //{{AFX_DATA(CTryAgainDlg)
  enum { IDD = IDD_TRYAGAIN };
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTryAgainDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  int cancelreturnval;

  // Generated message map functions
  //{{AFX_MSG(CTryAgainDlg)
  afx_msg void OnQuit();
  afx_msg void OnTryagain();
  afx_msg void OnHelp();
  afx_msg void OnOpen();
  afx_msg void OnNew();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
