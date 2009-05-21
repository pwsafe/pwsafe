/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "PWPropertyPage.h"
#include "MenuShortcuts.h"
#include "SHCTListCtrl.h"
#include "PWHdrCtrlNoChng.h"
#include "ControlExtns.h"

#include "resource.h"

#include <vector>

// COptionsShortcuts dialog

class COptionsShortcuts : public CPWPropertyPage
{
  DECLARE_DYNAMIC(COptionsShortcuts)

public:
  COptionsShortcuts();   // standard constructor
  ~COptionsShortcuts();

  bool IsChanged() {return m_bChanged;}
  void SetChanged() {m_bChanged = true;}

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

  const TCHAR *GetHelpName() const {return _T("menu_shortcuts");}

  // Dialog Data
  //{{AFX_DATA(COptionsBackup)
  enum { IDD = IDD_PS_SHORTCUTS };
  CSHCTListCtrl m_ShortcutLC;
  CStaticExtn m_stc_warning;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  BOOL OnInitDialog();
  BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS);
  afx_msg void OnBnClickedResetAll();

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  CPWHdrCtrlNoChng m_SHCTHeader;

  MapMenuShortcuts m_MapMenuShortcuts;
  MapKeyNameID m_MapKeyNameID;
  std::vector<UINT> m_ExcludedMenuItems;
  std::vector<st_MenuShortcut> m_ReservedShortcuts;

  bool m_bChanged;
  UINT m_id;
};
