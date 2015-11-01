/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"

// CFilterMediaTypeDlg dialog

class CFilterMediaTypeDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterMediaTypeDlg)

public:
  CFilterMediaTypeDlg(CWnd *pParent = NULL);   // standard constructor
  virtual ~CFilterMediaTypeDlg();

  // Dialog Data
  enum { IDD = IDD_FILTER_MEDIATYPES };
  int m_case;
  CString m_string;
  bool m_add_present;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangeStringRule();
  afx_msg void OnBnClickedOk();
  CComboBox m_cbxRule, m_cbxAvailableMTs;
  CEdit m_edtString;
  CButton m_btnCase;
  CStatic m_stcStatus;
  std::vector<StringX> *m_pvMediaTypes;

private:
  void EnableDialogItems();
};
