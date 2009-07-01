/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Additional.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"

#include "resource.h"

class CAddEdit_Additional : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_Additional)

  CAddEdit_Additional(CWnd * pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_Additional();

  const wchar_t *GetHelpName() const {return L"TO_DO!";}

    // Dialog Data
  //{{AFX_DATA(CAddEdit_Additional)
  enum { IDD = IDD_ADDEDIT_ADDITIONAL };

  CEditExtn m_ex_autotype;
  CEditExtn m_ex_runcommand;

  CStaticExtn m_stc_autotype;
  CStaticExtn m_stc_runcommand;

  CComboBox m_dblclk_cbox;
  BOOL m_UseDefaultDCA;
  int m_DCA_to_Index[PWSprefs::maxDCA + 1];

  CListCtrl m_PWHistListCtrl;
  int m_iSortedColumn;
  bool m_bSortAscending;
  bool m_ClearPWHistory;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Additional)
protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Additional)
  afx_msg void OnHelp();
  afx_msg BOOL OnKillActive();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnDCAComboChanged();
  afx_msg void OnSetDCACheck();
  afx_msg void OnSTCExClicked(UINT nId);
  afx_msg void OnCheckedSavePasswordHistory();
  afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHistListClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnPWHCopyAll();
  afx_msg void OnClearPWHist();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK PWHistCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                        LPARAM lParamSort);
  CToolTipCtrl *m_pToolTipCtrl;

  COLORREF m_autotype_cfOldColour, m_runcmd_cfOldColour;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
