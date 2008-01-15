/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once

// PWHisDlg.h CPWHistDlg
//-----------------------------------------------------------------

#include "PWDialog.h"

class CItemData;

class CPWHistDlg : public CPWDialog
{
 DECLARE_DYNAMIC(CPWHistDlg)

   public:
  CPWHistDlg(CWnd* pParent, bool IsReadOnly,
             CMyString &HistStr, PWHistList &PWHistList,
             size_t NumPWHistory, size_t &MaxPWHistory,
             BOOL &SavePWHistory);

  virtual ~CPWHistDlg();

  // Dialog Data
  enum { IDD = IDD_DLG_PWHIST };

 protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual void OnOK();
  virtual BOOL OnInitDialog();
 private:
  const bool m_PWH_IsReadOnly;
  // Following reference members from EditDlg
  CMyString &m_HistStr;
  PWHistList &m_PWHistList;
  const size_t m_NumPWHistory;
  size_t &m_MaxPWHistory;
  BOOL &m_SavePWHistory;

  CListCtrl m_PWHistListCtrl;
  int m_iSortedColumn;
  BOOL m_bSortAscending;
  size_t m_oldMaxPWHistory;
  bool m_ClearPWHistory;

  afx_msg void OnCheckedSavePasswordHistory();
  afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHistListClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnBnClickedPwhCopyAll();

 DECLARE_MESSAGE_MAP()
   public:
  afx_msg void OnBnClickedClearPWHist();
private:
  static int CALLBACK PWHistCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                  LPARAM lParamSort);
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
