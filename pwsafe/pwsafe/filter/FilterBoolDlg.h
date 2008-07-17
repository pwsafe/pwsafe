      /*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "../PWDialog.h"
#include "../corelib/ItemData.h"
#include "../resource.h"

// CFilterBoolDlg dialog

class CFilterBoolDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CFilterBoolDlg)

public:
  enum BoolType {BT_PRESENT, BT_ACTIVE, BT_SET};

  CFilterBoolDlg(CWnd* pParent = NULL);
  virtual ~CFilterBoolDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_BOOL };
  PWSMatch::MatchRule m_rule;
  CString m_title;
  CString m_oldtitle;
  bool m_bFirst;
  BoolType m_bt;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangeBoolRule();
  afx_msg void OnBnClickedOk();
  CComboBox m_cbxRule;

private:
  int m_rule2selection[PWSMatch::MR_LAST];
};
