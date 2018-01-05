/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// SampleTextDlg dialog

#include "resource.h"
#include "ControlExtns.h"
#include "PWDialog.h"

class CSampleTextDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CSampleTextDlg)

public:
  CSampleTextDlg(CWnd* pParent = NULL, CString sampletext = L"");
  virtual ~CSampleTextDlg();

  // Dialog Data
  enum { IDD = IDD_SAMPLETEXT };
  CString m_sampletext;

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  //{{AFX_MSG(CSampleTextDlg)
  afx_msg void OnOK();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CEditExtn m_ex_sampletext;
};
