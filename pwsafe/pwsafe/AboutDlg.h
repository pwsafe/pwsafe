/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

/// \file AboutDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"

class CAboutDlg : public CPWDialog
{
public:
  CAboutDlg(CWnd* pParent = NULL);

    // Dialog Data
  //{{AFX_DATA(CAddDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA
  CString m_appversion;
  CString m_appcopyright;

protected:
  virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV support
  {
    CPWDialog::DoDataExchange(pDX);
  }

protected:
  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};
