/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Additional.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"
#include "PWHistListCtrl.h"
#include "EntryKBHotKey.h"
#include "TBMStatic.h"

#include "resource.h"

class CAddEdit_Additional : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_Additional)

  CAddEdit_Additional(CWnd * pParent, st_AE_master_data *pAEMD);

  // Dialog Data
  //{{AFX_DATA(CAddEdit_Additional)
  enum { IDD = IDD_ADDEDIT_ADDITIONAL,
         IDD_SHORT = IDD_ADDEDIT_ADDITIONAL_SHORT };

  // 0 is Best, > 0 is OK, < 0 is Bad
  enum {KBSHORTCUT_IN_USE_BY_PWS     = -5,
        KBSHORTCUT_INVALID_CHARACTER = -2,
        KBSHORTCUT_IN_USE_BY_ENTRY   = -1,
        KBSHORTCUT_UNIQUE            =  0,
        KBSHORTCUT_MADE_UNIQUE       =  1,
        KBSHORTCUT_IN_USE_BY_MENU    =  2,        
        KBSHORTCUT_CANT_MAKE_UNIQUE  =  3};

  void OnEntryHotKeyKillFocus();
  void OnEntryHotKeySetFocus();
  void UpdatePasswordHistoryLC();
  bool HasBeenShown() const { return m_bInitdone; }

protected:
  CEditExtn m_ex_autotype;
  CEditExtn m_ex_runcommand;

  CStaticExtn m_stc_autotype;
  CStaticExtn m_stc_runcommand;
  CStaticExtn m_stc_warning;

  CComboBox m_dblclk_cbox;
  CComboBox m_shiftdblclk_cbox;
  int m_DCA_to_Index[PWSprefs::maxDCA + 1];

  CPWHistListCtrl m_PWHistListCtrl;
  CEntryKBHotKey m_KBShortcutCtrl;

  int m_iSortedColumn;
  bool m_bSortAscending;
  bool m_bClearPWHistory;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Additional)
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Additional)
  afx_msg void OnHelp();
  afx_msg void OnAutoTypeHelp();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

  afx_msg void OnDCAComboChanged();
  afx_msg void OnShiftDCAComboChanged();
  afx_msg void OnChanged();
  afx_msg void OnHotKeyChanged();

  afx_msg void OnSTCExClicked(UINT nId);
  afx_msg void OnCheckedSavePasswordHistory();
  afx_msg void OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHistListClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnPWHCopyAll();
  afx_msg void OnClearPWHist();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK PWHistCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                        LPARAM lParamSort);
  void SetupDCAComboBoxes(CComboBox *pcbox, bool isShift);
  int CheckKeyboardShortcut();
 
  CTBMStatic m_Help1, m_Help2;

  COLORREF m_autotype_cfOldColour, m_runcmd_cfOldColour;
  bool m_bInitdone;
  bool m_bWarnUserKBShortcut;

  int32 m_iAppHotKey, m_iOldHotKey;
  WORD m_wAppVirtualKeyCode;
  WORD m_wAppPWSModifiers;
  bool m_bAppHotKeyEnabled;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
