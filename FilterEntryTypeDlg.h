/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWDialog.h"
#include "corelib/ItemData.h"
#include "resource.h"

// CFilterEntryTypeDlg dialog

class CFilterEntryTypeDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CFilterEntryTypeDlg)

public:
  CFilterEntryTypeDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterEntryTypeDlg();

// Dialog Data
  enum { IDD = IDD_FILTER_ENTRYTYPE };
  PWSMatch::MatchRule m_rule;
  CItemData::EntryType m_etype;
  CString m_title;
  CString m_oldtitle;
  bool m_bFirst;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnCbnSelchangeEntryTypeRule();
  afx_msg void OnCbnSelchangeEntryType1();
  afx_msg void OnBnClickedOk();
  CComboBox m_cbxRule, m_cbxEType;

private:
  int m_rule2selection[PWSMatch::MR_LAST];
  int m_etype2selection[CItemData::ET_LAST];
};
