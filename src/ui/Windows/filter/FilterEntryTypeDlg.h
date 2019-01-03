/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"
#include "core/Itemdata.h" // for CItemData::EntryType

// CFilterEntryTypeDlg dialog

class CFilterEntryTypeDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterEntryTypeDlg)

public:
  CFilterEntryTypeDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterEntryTypeDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_ENTRYTYPE };
  CItemData::EntryType m_etype;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnCbnSelchangeEntryTypeRule();
  afx_msg void OnCbnSelchangeEntryType1();
  afx_msg void OnBnClickedOk();

  DECLARE_MESSAGE_MAP()

  CComboBox m_cbxRule, m_cbxEType;

private:
  int m_etype2selection[CItemData::ET_LAST];
};
