/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"

// CFilterDCADlg dialog

class CFilterDCADlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterDCADlg)

public:
  CFilterDCADlg(CWnd* pParent = NULL);
  virtual ~CFilterDCADlg();

// Dialog Data
  enum { IDD = IDD_FILTER_DCA };
  short m_DCA;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnCbnSelchangeDCARule();
  afx_msg void OnCbnSelchangeDCA1();
  afx_msg void OnBnClickedOk();

  DECLARE_MESSAGE_MAP()

  CComboBox m_cbxRule, m_cbxDCA;

private:
  // Extra element to cope with index = -1
  // Unlike other similar arrays
  int m_DCA2selection[PWSprefs::maxDCA + 2];
};
