/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"

// CFilterBoolDlg dialog

class CFilterBoolDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterBoolDlg)

public:
  enum BoolType {BT_PRESENT, BT_ACTIVE, BT_SET, BT_IS};

  CFilterBoolDlg(CWnd* pParent = NULL);
  virtual ~CFilterBoolDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_BOOL };
  BoolType m_bt;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangeBoolRule();
  afx_msg void OnBnClickedOk();
  CComboBox m_cbxRule;
};
