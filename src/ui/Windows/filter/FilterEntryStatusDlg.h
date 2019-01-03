/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"
#include "core/Itemdata.h" // for CItemData::EntryStatus

// CFilterEntryStatusDlg dialog

class CFilterEntryStatusDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterEntryStatusDlg)

public:
  CFilterEntryStatusDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterEntryStatusDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_ENTRYSTATUS };
  CItemData::EntryStatus m_estatus;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnCbnSelchangeEntryStatusRule();
  afx_msg void OnCbnSelchangeEntryStatus1();
  afx_msg void OnBnClickedOk();

  DECLARE_MESSAGE_MAP()

  CComboBox m_cbxRule, m_cbxEStatus;

private:
  int m_estatus2selection[CItemData::ES_LAST];
};
