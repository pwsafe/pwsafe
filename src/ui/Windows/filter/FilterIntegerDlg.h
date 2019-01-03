/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"

// CFilterIntegerDlg dialog

class CFilterIntegerDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterIntegerDlg)

public:
  CFilterIntegerDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterIntegerDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_INTEGER };
  int m_num1;
  int m_num2;
  int m_min, m_max;
  bool m_add_present;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnCbnSelchangeIntegerRule();
  afx_msg void OnBnClickedOk();

  DECLARE_MESSAGE_MAP()

  CComboBox m_cbxRule;
  CEdit m_edtInteger1, m_edtInteger2;
  CStatic m_stcAnd, m_stcStatus;

private:
  void AFXAPI DDV_CheckNumbers(CDataExchange* pDX,
                               const int &num1, const int &num2);
  void AFXAPI DDV_CheckMinMax(CDataExchange* pDX,
                              const int &num, const int &min, const int &max);
};
