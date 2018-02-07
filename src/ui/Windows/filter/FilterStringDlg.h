/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"
#include "../ControlExtns.h"

// CFilterStringDlg dialog

class CFilterStringDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterStringDlg)

public:
  CFilterStringDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterStringDlg();

  void SetSymbol(const bool bSymbol) {m_bSymbol = bSymbol;}
  
// Dialog Data
  enum { IDD = IDD_FILTER_STRING };
  int m_case;
  CString m_string;
  bool m_add_present;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnCbnSelchangeStringRule();
  afx_msg void OnBnClickedOk();

  DECLARE_MESSAGE_MAP()

  CComboBox m_cbxRule;
  CEdit m_edtString;
  CSymbolEdit m_edtSymbolString;
  CButton m_btnCase;
  CStatic m_stcStatus;

private:
  void EnableDialogItems();
  bool m_bSymbol;
};
