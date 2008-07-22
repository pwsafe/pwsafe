/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"
#include "afxdtctl.h" // only needed for date/time controls

// CFilterDateDlg dialog

class CFilterDateDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterDateDlg)

public:
  CFilterDateDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterDateDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_DATE };
  CTime m_ctime1, m_ctime2;
  time_t m_time_t1, m_time_t2;
  bool m_add_present;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangeDateRule();
  afx_msg void OnBnClickedOk();
  afx_msg void OnDtnDatetime1Change(NMHDR *pNMHDR, LRESULT *pResult);
  CComboBox m_cbxRule;
  CDateTimeCtrl m_dtp1, m_dtp2;
  CStatic m_stcAnd, m_stcStatus;
};
