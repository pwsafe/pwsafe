/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// OptionsShortcuts.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsShortcuts dialog
#include "Options_PropertyPage.h"
#include "MenuShortcuts.h"
#include "SHCTListCtrl.h"
#include "PWHdrCtrlNoChng.h"
#include "ControlExtns.h"

#include "resource.h"

#include <vector>

// COptionsShortcuts dialog

class COptionsShortcuts : public COptions_PropertyPage
{
  DECLARE_DYNAMIC(COptionsShortcuts)

public:
  COptionsShortcuts();   // standard constructor
  ~COptionsShortcuts();

  bool HaveShortcutsChanged() {return m_bShortcutsChanged;}
  void SetShortcutsChanged() {m_bShortcutsChanged = true;}

  void InitialSetup(const MapMenuShortcuts MapMenuShortcuts,
                    const MapKeyNameID MapKeyNameID,
                    const std::vector<UINT> ExcludedMenuItems,
                    const std::vector<st_MenuShortcut> ReservedShortcuts);

  MapMenuShortcuts GetMaps() {return m_MapMenuShortcuts;}

  MapMenuShortcutsIter GetMapMenuShortcutsIter(const UINT &id)
  {return m_MapMenuShortcuts.find(id);}

  MapKeyNameIDConstIter GetMapKeyNameIDConstIter(const st_KeyIDExt &st_KIDEx)
  {return m_MapKeyNameID.find(st_KIDEx);}

  void OnHotKeyKillFocus(const int item, const UINT id,
                         const WORD wVirtualKeyCode, const WORD wModifiers);

  void ClearWarning() {m_stc_warning.ShowWindow(SW_HIDE);}

  // Dialog Data
  //{{AFX_DATA(COptionsShortcuts)
  enum { IDD = IDD_PS_SHORTCUTS };
  CSHCTListCtrl m_ShortcutLC;
  CStaticExtn m_stc_warning;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsShortcuts)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsShortcuts)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnHelp();
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS);
  afx_msg void OnBnClickedResetAll();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  CPWHdrCtrlNoChng m_SHCTHeader;

  MapMenuShortcuts m_MapMenuShortcuts;
  MapKeyNameID m_MapKeyNameID;
  std::vector<UINT> m_ExcludedMenuItems;
  std::vector<st_MenuShortcut> m_ReservedShortcuts;

  bool m_bShortcutsChanged;
  UINT m_id;
};
