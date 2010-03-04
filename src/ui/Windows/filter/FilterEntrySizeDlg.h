/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"

// CFilterEntrySizeDlg dialog

class CFilterEntrySizeDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterEntrySizeDlg)

public:
  CFilterEntrySizeDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterEntrySizeDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_SIZE };
  int m_size1;
  int m_size2;
  int m_min, m_max;
  int m_unit;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangeSizeRule();
  afx_msg void OnSizeUnit(UINT nID);
  afx_msg void OnBnClickedOk();
  CComboBox m_cbxRule;
  CEdit m_edtSize1, m_edtSize2;
  CStatic m_stcAnd, m_stcStatus;

private:
  void AFXAPI DDV_CheckNumbers(CDataExchange* pDX,
                               const int &size1, const int &size2);
  void AFXAPI DDV_CheckMinMax(CDataExchange* pDX,
                              const int &num, const int &min, const int &max);
};
