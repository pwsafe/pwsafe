/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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

class COptionsMisc : public COptions_PropertyPage
{
  DECLARE_DYNCREATE(COptionsMisc)

  // Construction
public:
  COptionsMisc();
  ~COptionsMisc();

  // Dialog Data
  //{{AFX_DATA(COptionsMisc)
  enum { IDD = IDD_PS_MISC };
  BOOL m_confirmdelete;
  BOOL m_maintaindatetimestamps;
  BOOL m_escexits;
  BOOL m_hotkey_enabled;
  // JHF : class CHotKeyCtrl not supported by WinCE
#if !defined(POCKET_PC)
  CHotKeyCtrl m_hotkey;
#endif
  CComboBox m_dblclk_cbox;
  BOOL m_usedefuser;
  BOOL m_querysetdef;
  CString m_defusername;
  CString m_otherbrowserlocation;
  CString m_othereditorlocation;
  //}}AFX_DATA

  BOOL m_saveconfirmdelete;
  BOOL m_savemaintaindatetimestamps;
  BOOL m_saveescexits;
  BOOL m_savehotkey_enabled;
  BOOL m_saveusedefuser;
  BOOL m_savequerysetdef;
  CString m_savedefusername;
  CString m_saveotherbrowserlocation;
  CString m_saveothereditorlocation;
  DWORD m_savehotkey_value;
  int m_savedoubleclickaction;
  CString m_saveBrowserCmdLineParms;
  CString m_saveAutotype;
  BOOL m_saveminauto;

  DWORD m_hotkey_value;
  int m_doubleclickaction;
  int m_DCA_to_Index[PWSprefs::maxDCA + 1];
  CString m_csBrowserCmdLineParms;
  CString m_csAutotype;
  BOOL m_minauto;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsMisc)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsMisc)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
  afx_msg void OnHelp();
  afx_msg void OnEnableHotKey();
  afx_msg void OnUsedefuser();
  afx_msg void OnBrowseForLocation(UINT nID);
  afx_msg void OnComboChanged();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CToolTipCtrl* m_pToolTipCtrl;
};
