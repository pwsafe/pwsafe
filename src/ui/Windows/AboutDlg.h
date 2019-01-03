/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file AboutDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "RichEditCtrlExtn.h"
#include "core/CheckVersion.h"

class CAboutDlg : public CPWDialog
{
public:
  CAboutDlg(CWnd* pParent = NULL);

  // Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  // Generated message map functions
  //{{AFX_MSG(CAboutDlg)
  afx_msg void OnTakeTestdump();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  int m_nMajor;
  int m_nMinor;
  int m_nBuild;
  CString m_appversion;
  CString m_newVerStatus;
  CRichEditCtrlExtn m_RECExNewVerStatus;
  CRichEditCtrlExtn m_RECExWebSite;

  CheckVersion::CheckStatus CheckLatestVersion(std::wstring &latest);
  void CheckNewVer();

  static bool OnCheckVersion(const CString &URL, const CString &FName, LPARAM instance);
};
