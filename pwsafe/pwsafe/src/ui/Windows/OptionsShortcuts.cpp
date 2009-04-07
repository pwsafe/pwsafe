/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsShortcuts.cpp : implementation file
//

#include "stdafx.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu   resources
#include "resource3.h"  // String resources
#endif

#include "OptionsShortcuts.h"

// COptionsShortcuts dialog

IMPLEMENT_DYNAMIC(COptionsShortcuts, CPropertyPage)

COptionsShortcuts::COptionsShortcuts(): CPWPropertyPage(COptionsShortcuts::IDD),
  m_bChanged(false)
{
  //{{AFX_DATA_INIT(COptionsShortcuts)
  //}}AFX_DATA_INIT
}

COptionsShortcuts::~COptionsShortcuts()
{
}

void COptionsShortcuts::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_SHORTCUTLIST, m_ShortcutLC);
  DDX_Control(pDX, IDC_STATIC_SHCTWARNING, m_stc_warning);
}

BEGIN_MESSAGE_MAP(COptionsShortcuts, CPropertyPage)
  ON_BN_CLICKED(IDC_RESETALLSHORTCUTS, OnBnClickedResetAll)
  ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

void COptionsShortcuts::InitialSetup(const MapMenuShortcuts MapMenuShortcuts,
                    const MapKeyNameID MapKeyNameID,
                    const std::vector<UINT> ExcludedMenuItems,
                    const std::vector<st_MenuShortcut> ReservedShortcuts)
{
  m_MapMenuShortcuts = MapMenuShortcuts;
  m_MapKeyNameID = MapKeyNameID;
  m_ExcludedMenuItems = ExcludedMenuItems;
  m_ReservedShortcuts = ReservedShortcuts;
}

BOOL COptionsShortcuts::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  m_ShortcutLC.Init(this);

  DWORD dwExtendedStyle = m_ShortcutLC.GetExtendedStyle() | LVS_EX_GRIDLINES;
  m_ShortcutLC.SetExtendedStyle(dwExtendedStyle);

  CString cs_colname;
  cs_colname.LoadString(IDS_COL_MENUITEM);
  m_ShortcutLC.InsertColumn(0, cs_colname);
  cs_colname.LoadString(IDS_COL_SHORTCUT);
  m_ShortcutLC.InsertColumn(1, cs_colname);

  MapMenuShortcutsIter iter, iter_parent;
  MapKeyNameIDConstIter citer;
  CString str;
  int iItem(-1);

  for (iter = m_MapMenuShortcuts.begin(); iter != m_MapMenuShortcuts.end();
    iter++) {
    // We don't allow change of certain menu items
    // Just don't put in the list that the user sees.
    if (iter->second.uiParentID == 0)
      continue;

    if (std::find(m_ExcludedMenuItems.begin(),
                  m_ExcludedMenuItems.end(),
                  iter->first) != m_ExcludedMenuItems.end())
        continue;

    if (iter->second.cVirtKey != 0) {
       st_KeyIDExt st_KIDEx;
       st_KIDEx.id = iter->second.cVirtKey;
       st_KIDEx.bExtended = (iter->second.cModifier & HOTKEYF_EXT) == HOTKEYF_EXT;
       citer = m_MapKeyNameID.find(st_KIDEx);
       str = CMenuShortcut::FormatShortcut(iter, citer);
    } else {
      str = _T("");
    }

    // Remove the ampersand from the menu item the user sees here
    iter_parent = m_MapMenuShortcuts.find(iter->second.uiParentID);
    CString sMenuItemtext = CString(iter_parent->second.name) + 
                                  CString(_T(" \xbb ")) +
                                  CString(iter->second.name);
    sMenuItemtext.Remove(TCHAR('&'));
    iItem = m_ShortcutLC.InsertItem(++iItem, sMenuItemtext);
    m_ShortcutLC.SetItemText(iItem, 1, str);
    DWORD dwData = MAKELONG(iter->first, iter->second.iMenuPosition);
    m_ShortcutLC.SetItemData(iItem, dwData);
  }

  // Now sort via Menu item position
  m_ShortcutLC.SortItems(CompareFunc, NULL);

  m_ShortcutLC.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

  m_ShortcutLC.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);

  CHeaderCtrl* pHCtrl;
  pHCtrl = m_ShortcutLC.GetHeaderCtrl();
  ASSERT(pHCtrl != NULL);
  pHCtrl->SetDlgCtrlID(IDC_SHORTCUTLC_HEADER);
  m_SHCTHeader.SubclassWindow(pHCtrl->GetSafeHwnd());
  m_SHCTHeader.SetStopChangeFlag(true);

  m_stc_warning.SetColour(RGB(255, 0, 0));
  m_stc_warning.ShowWindow(SW_HIDE);

  return TRUE;
}

void COptionsShortcuts::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
   // If this is our list control then increase height
   if (nIDCtl == IDC_SHORTCUTLIST) {
     lpMIS->itemHeight += 10;
   }
}

void COptionsShortcuts::OnBnClickedResetAll()
{
  MapMenuShortcutsIter iter;
  MapKeyNameIDConstIter citer;
  CString str;
  UINT id;
  st_KeyIDExt st_KIDEx;

  for (int i = 0; i < m_ShortcutLC.GetItemCount(); i++) {
    id = (UINT)LOWORD(m_ShortcutLC.GetItemData(i));

    iter = m_MapMenuShortcuts.find(id);
    st_KIDEx.id = iter->second.cdefVirtKey;
    st_KIDEx.bExtended = (iter->second.cModifier & HOTKEYF_EXT) == HOTKEYF_EXT;
    citer = m_MapKeyNameID.find(st_KIDEx);
    iter->second.cVirtKey = iter->second.cdefVirtKey;
    iter->second.cModifier = iter->second.cdefModifier;
  
    if (citer != m_MapKeyNameID.end() || iter->second.cdefVirtKey != 0) {
      str = CMenuShortcut::FormatShortcut(iter, citer);
    } else {
      str = _T("");
    }
    m_ShortcutLC.SetItemText(i, 1, str);
  }

  ClearWarning();

  m_ShortcutLC.RedrawItems(0, m_ShortcutLC.GetItemCount());
  m_ShortcutLC.UpdateWindow();
  m_bChanged = true;
}

// Functor for find_if to see if shortcut is reserved
struct reserved {
  reserved(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(st_MenuShortcut const& rdata) const
  {
    return (m_st_mst.cVirtKey  == rdata.cVirtKey &&
            m_st_mst.cModifier == rdata.cModifier);
  }

  st_MenuShortcut m_st_mst;
};

// Functor for find_if to see if shortcut is already in use
struct already_inuse {
  already_inuse(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(MapMenuShortcutsPair const & p) const
  {
    return (p.second.cVirtKey  == m_st_mst.cVirtKey &&
            p.second.cModifier == m_st_mst.cModifier);
  }

  st_MenuShortcut m_st_mst;
};

// Tortuous route to get here!
// m_HotKey looses focus and calls parent (CListCtrl) that calls here
void COptionsShortcuts::OnHotKeyKillFocus(const int item, const UINT id,
                                          const WORD wVirtualKeyCode, 
                                          const WORD wModifiers)
{
  CString str(_T("")), mst_str(_T(""));
  CString cs_warning;
  MapMenuShortcutsIter iter, inuse_iter;
  MapKeyNameIDConstIter citer;
  st_MenuShortcut st_mst;
  st_KeyIDExt st_KIDEx;

  st_KIDEx.id = (unsigned char)wVirtualKeyCode;
  st_KIDEx.bExtended = (wModifiers & HOTKEYF_EXT) == HOTKEYF_EXT;
  citer = m_MapKeyNameID.find(st_KIDEx);

  // Stop compiler complaining - put this here even if not needed
  st_mst.cVirtKey  = (unsigned char)wVirtualKeyCode;
  st_mst.cModifier = (unsigned char)wModifiers;
  already_inuse inuse(st_mst);

  if (citer == m_MapKeyNameID.end()) {
    // Invalid shortcut
    cs_warning.LoadString(IDS_SHCT_WARNING1);
    goto set_warning;
  }

  if (st_mst.cVirtKey != 0) {
    mst_str = CMenuShortcut::FormatShortcut(st_mst, citer);
  }

  if (std::find_if(m_ReservedShortcuts.begin(),
                   m_ReservedShortcuts.end(),
                   reserved(st_mst)) != m_ReservedShortcuts.end()) {
    // Reserved shortcut ignored
    cs_warning.Format(IDS_SHCT_WARNING2, mst_str);
    goto set_warning;
  }

  // Check not already in use (ignore if deleting current shortcut)
  iter = m_MapMenuShortcuts.find(id);
  if (st_mst.cVirtKey != (unsigned char)0) {
    inuse_iter = std::find_if(m_MapMenuShortcuts.begin(),
                              m_MapMenuShortcuts.end(),
                              inuse);
    if (inuse_iter != m_MapMenuShortcuts.end() && 
        inuse_iter->first != iter->first) {
      // Shortcut in use
      cs_warning.Format(IDS_SHCT_WARNING3, mst_str, inuse_iter->second.name);
      goto set_warning;
    }
  }

  if (iter->second.cVirtKey != 0) {
    str = CMenuShortcut::FormatShortcut(iter, citer);
  }

  // Not reserved and not already in use - implement
  iter->second.cVirtKey = st_mst.cVirtKey;
  iter->second.cModifier = st_mst.cModifier;

  m_ShortcutLC.SetItemText(item, 1, str);
  m_ShortcutLC.RedrawItems(item, item);
  m_ShortcutLC.UpdateWindow();
  m_bChanged = true;
  return;

set_warning:
  m_stc_warning.SetWindowText(cs_warning);
  m_stc_warning.ShowWindow(SW_SHOW);
}

BOOL COptionsShortcuts::PreTranslateMessage(MSG* pMsg)
{
  // If HotKey active, allow Enter key to set instead of close
  // Property Page (OK button)
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
      m_ShortcutLC.IsHotKeyActive()) {
    m_ShortcutLC.SaveHotKey();
    return TRUE;
  }

  return CPWPropertyPage::PreTranslateMessage(pMsg);
}

int CALLBACK COptionsShortcuts::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                            LPARAM lParamSort)
{
  UNREFERENCED_PARAMETER(lParamSort);
  return (int)(HIWORD(lParam1) - HIWORD(lParam2));
}
