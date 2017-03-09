/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
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
#include "TBMStatic.h"

#include "core/coredefs.h"

#include "resource.h"

#include <vector>

// Entry keyboard shortcut subitem indices
#define ENTRYSHCT_SHORTCUTKEYS  0
#define ENTRYSHCT_GROUP         1
#define ENTRYSHCT_TITLE         2
#define ENTRYSHCT_USER          3
#define ENTRYSHCT_NUM_COLUMNS   4

// COptionsShortcuts dialog

class COptionsShortcuts : public COptions_PropertyPage
{
public:
  DECLARE_DYNAMIC(COptionsShortcuts)

  // Construction
  COptionsShortcuts(CWnd *pParent, st_Opt_master_data *pOPTMD);   // standard constructor
  ~COptionsShortcuts();

  bool HaveShortcutsChanged() {return m_bShortcutsChanged;}

  void InitialSetup(const MapMenuShortcuts MapMenuShortcuts,
                    const std::vector<UINT> &ExcludedMenuItems,
                    const std::vector<st_MenuShortcut> &ReservedShortcuts);

  MapMenuShortcuts GetMaps() {return m_OSMapMenuShortcuts;}

  bool GetMapMenuShortcutsIter(const UINT &id, MapMenuShortcutsIter &iter);
  
  bool OnMenuShortcutKillFocus(const int item, const UINT id,
                               const WORD wVirtualKeyCode, const WORD wModifiers);

  pws_os::CUUID &GetKBShortcutUUID(int lParam)
  {return m_OSKBShortcutMap[lParam];}

protected:
  // Dialog Data
  //{{AFX_DATA(COptionsShortcuts)
  enum { IDD = IDD_PS_SHORTCUTS, IDD_SHORT = IDD_PS_SHORTCUTS_SHORT };

  // 0 is Best, > 0 is OK, < 0 is Bad (negative values much larger than positive)
  enum {
        HOTKEY_IN_USE_BY_MENU       = -20,
        HOTKEY_IN_USE_BY_ENTRY      = -10,
        HOTKEY_UNIQUE               =  0,
        HOTKEY_MADE_UNIQUE          =  1,
        HOTKEY_CANT_MAKE_UNIQUE     =  2
  };

  CHotKeyCtrl m_AppHotKeyCtrl;
  CSHCTListCtrl m_ShortcutLC;
  CListCtrl m_EntryShortcutLC;
  //}}AFX_DATA

  int32 m_AppHotKeyValue;
  int m_iColWidth, m_iDefColWidth;
  BOOL m_bOS_AppHotKeyEnabled, m_bOS_AutotypeHotKeyEnabled;

  CTBMStatic m_Help1, m_Help2, m_Help3, m_Help4;

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsShortcuts)
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(COptionsShortcuts)
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
  afx_msg void OnContextMenu(CWnd *, CPoint point);

  afx_msg void OnHelp();
  afx_msg void OnEnableAppHotKey();
  afx_msg void OnEnableAutotypeHotKey();
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS);
  afx_msg void OnResetAll();
  afx_msg void OnHeaderNotify(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHeaderRClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnResetColumnWidth();
  afx_msg void OnKBShortcutDoulbleClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnAppHotKeyChanged();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  friend class CAppHotKey;      // To access "CheckHotKey"
  friend class CSHCTListCtrlX;  // To access "CheckHotKey"

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  static int CALLBACK CKBSHCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  void RefreshKBShortcuts();
  int CheckHotKey(WORD &wVirtualKeyCode, WORD &wHKModifiers, int iWhichHotKey);

  MapMenuShortcuts m_OSMapMenuShortcuts, m_OSMapSaveMenuShortcuts;
  std::vector<UINT> m_OSExcludedMenuItems;
  std::vector<st_MenuShortcut> m_OSReservedShortcuts;

  KBShortcutMap m_OSKBShortcutMap;

  int32 m_iOldAppHotKey;
  bool m_bShortcutsChanged;
  UINT m_id;
  int m_iSortedColumn, m_iKBSortedColumn;
  int m_PrevItem;
  bool m_bSortAscending, m_bKBSortAscending;
  bool m_bFirstShown, m_bConflictDetected;
  WORD m_wAppVirtualKeyCode, m_wAppModifiers;

  CFont *m_pAutotype_HK_Font;
};
