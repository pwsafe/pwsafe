/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"

// CFilterPasswordDlg dialog

class CFilterPasswordDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterPasswordDlg)

public:
  CFilterPasswordDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterPasswordDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_PASSWORD };
  int m_case;
  CMyString m_string;
  int m_num1;
  int m_maxDays;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangePasswordRule();
  afx_msg void OnBnClickedOk();
  CComboBox m_cbxRule;
  CEdit m_edtString, m_edtInteger1;
  CButton m_btnCase;
  CStatic m_stcStatus, m_stcIn, m_stcDays;

private:
  void EnableDialogItems();
  void AFXAPI DDV_CheckMinMax(CDataExchange* pDX,
                              const int num, const int min, const int max);
};
