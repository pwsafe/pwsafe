/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once

/// \file AboutDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "DboxMain.h"
#include "RichEditCtrlExtn.h"

class CAboutDlg : public CPWDialog
{
public:
  CAboutDlg(CWnd* pParent = NULL);

    // Dialog Data
  //{{AFX_DATA(CAddDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

public:

private:
  int m_nMajor;
  int m_nMinor;
  int m_nBuild;
  CString m_appversion;
  CString m_appcopyright;
  CString m_newVerStatus;
  CRichEditCtrlExtn m_RECExNewVerStatus;
  CRichEditCtrlExtn m_RECExWebSite;

  enum CheckStatus {UP2DATE, NEWER_AVAILABLE, CANT_CONNECT, CANT_READ};
  CheckStatus CheckLatestVersion(CString &latest);
  void CheckNewVer();
  
  static bool OnCheckVersion(const CString &URL, const CString &FName, LPARAM instance);
};
