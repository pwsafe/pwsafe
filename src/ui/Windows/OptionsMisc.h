/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// OptionsMisc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc dialog
#include "Options_PropertyPage.h"
#include "resource.h"

class COptionsMisc : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsMisc)

  // Construction
  COptionsMisc(CWnd *pParent, st_Opt_master_data *pOPTMD);
  ~COptionsMisc();

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsMisc)
  enum { IDD = IDD_PS_MISC, IDD_SHORT = IDD_PS_MISC_SHORT };

  CComboBox m_dblclk_cbox, m_shiftdblclk_cbox;
  CHotKeyCtrl m_HotkeyCtrl;

  CString m_DefUsername;
  CString m_OtherBrowserLocation;
  CString m_OtherEditorLocation;
  CString m_BrowserCmdLineParms;
  CString m_Autotype;
  DWORD m_HotkeyValue;
  BOOL m_ConfirmDelete;
  BOOL m_MaintainDatetimeStamps;
  BOOL m_EscExits;
  BOOL m_HotkeyEnabled;
  BOOL m_UseDefUsername;
  BOOL m_QuerySetDefUsername;
  BOOL m_AutotypeMinimize;
  int m_DoubleClickAction, m_ShiftDoubleClickAction;
  //}}AFX_DATA

  int m_DCA_to_Index[PWSprefs::maxDCA + 1];

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsMisc)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsMisc)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
  afx_msg void OnHelp();
  afx_msg void OnEnableHotKey();
  afx_msg void OnUseDefUser();
  afx_msg void OnBrowseForLocation(UINT nID);
  afx_msg void OnDCAComboChanged();
  afx_msg void OnShiftDCAComboChanged();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetupCombo(CComboBox *pcbox);

  CToolTipCtrl* m_pToolTipCtrl;
};
