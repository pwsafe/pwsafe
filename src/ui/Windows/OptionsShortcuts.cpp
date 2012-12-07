/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsShortcuts.cpp : implementation file
//

#include "stdafx.h"
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu   resources
#include "resource3.h"  // String resources
#endif

#include "OptionsShortcuts.h" // Must be after resource.h

#include <algorithm>

// COptionsShortcuts dialog

IMPLEMENT_DYNAMIC(COptionsShortcuts, COptions_PropertyPage)

COptionsShortcuts::COptionsShortcuts(CWnd *pParent, st_Opt_master_data *pOPTMD)
: COptions_PropertyPage(pParent,
                        COptionsShortcuts::IDD, COptionsShortcuts::IDD_SHORT,
                        pOPTMD),
  m_bShortcutsChanged(false)
{
  m_iColWidth = M_ColWidth();
  m_iDefColWidth = M_DefColWidth();
}

COptionsShortcuts::~COptionsShortcuts(){ }

void COptionsShortcuts::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_SHORTCUTLIST, m_ShortcutLC);
  DDX_Control(pDX, IDC_STATIC_SHCTWARNING, m_stc_warning);
}

BEGIN_MESSAGE_MAP(COptionsShortcuts, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsShortcuts)
  ON_WM_MEASUREITEM()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_RESETALLSHORTCUTS, OnResetAll)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  ON_NOTIFY(HDN_ENDTRACK, IDC_LIST_HEADER, OnHeaderNotify)
  ON_NOTIFY(NM_RCLICK, IDC_LIST_HEADER, OnHeaderRClick)
  ON_COMMAND(ID_MENUITEM_RESETCOLUMNWIDTH, OnResetColumnWidth)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsShortcuts message handlers

BOOL COptionsShortcuts::OnInitDialog()
{
  BOOL brc;
  COptions_PropertyPage::OnInitDialog();

  m_ShortcutLC.Init(this);

  // Override default HeaderCtrl ID of 0
  CHeaderCtrl *pLCHdrCtrl = m_ShortcutLC.GetHeaderCtrl();
  pLCHdrCtrl->SetDlgCtrlID(IDC_LIST_HEADER);

  DWORD dwExtendedStyle = m_ShortcutLC.GetExtendedStyle() | LVS_EX_GRIDLINES;
  m_ShortcutLC.SetExtendedStyle(dwExtendedStyle);

  CString cs_colname;
  cs_colname.LoadString(IDS_COL_SHORTCUT);
  m_ShortcutLC.InsertColumn(0, cs_colname);  // SHCT_SHORTCUTKEYS
  cs_colname.LoadString(IDS_COL_MENUITEM);
  m_ShortcutLC.InsertColumn(1, cs_colname);  // SHCT_MENUITEMTEXT

  MapMenuShortcutsIter iter, iter_parent;
  CString str;
  int iItem(0);

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

    str = CMenuShortcut::FormatShortcut(iter);

    iter_parent = m_MapMenuShortcuts.find(iter->second.uiParentID);
    ASSERT(iter_parent != m_MapMenuShortcuts.end());
    CString sMenuItemtext = (CString(iter_parent->second.name.c_str()) + 
                             CString(L" \xbb ") +
                             CString(iter->second.name.c_str()));

    // Remove the ampersand from the menu item the user sees here
    sMenuItemtext.Remove(L'&');

    iItem = m_ShortcutLC.InsertItem(iItem, str);  // SHCT_SHORTCUTKEYS
    ASSERT(iItem != -1);
    brc = m_ShortcutLC.SetItemText(iItem, 1, sMenuItemtext); // SHCT_MENUITEMTEXT
    ASSERT(brc != 0);
    DWORD dwData = MAKELONG(iter->first, iter->second.iMenuPosition);
    brc = m_ShortcutLC.SetItemData(iItem, dwData);
    ASSERT(brc != 0);
  } // foreach m_MapMenuShortcuts

  // Now sort via Menu item position
  brc = m_ShortcutLC.SortItems(CompareFunc, NULL);
  ASSERT(brc != 0);

  brc = m_ShortcutLC.SetColumnWidth(0, m_iColWidth); // SHCT_SHORTCUTKEYS
  ASSERT(brc != 0);
  brc = m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // SHCT_MENUITEMTEXT
  ASSERT(brc != 0);

  brc = m_ShortcutLC.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
  ASSERT(brc != 0);

  m_stc_warning.SetColour(RGB(255, 0, 0));
  m_stc_warning.ShowWindow(SW_HIDE);

  return TRUE;
}


bool shortcutmaps_equal (MapMenuShortcutsPair p1, MapMenuShortcutsPair p2 )
{
  return (p1.first == p2.first && 
          p1.second.cModifier == p2.second.cModifier &&
          p1.second.siVirtKey == p2.second.siVirtKey);
}

LRESULT COptionsShortcuts::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (m_MapMenuShortcuts.size() != m_MapSaveMenuShortcuts.size() ||
          !std::equal(m_MapMenuShortcuts.begin(), m_MapMenuShortcuts.end(), m_MapSaveMenuShortcuts.begin(),
                   shortcutmaps_equal)) {
        m_bShortcutsChanged = true;
        return 1L;
      } else
        m_bShortcutsChanged = false;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL COptionsShortcuts::OnApply()
{
  UpdateData(TRUE);

  if (m_MapMenuShortcuts.size() != m_MapSaveMenuShortcuts.size() ||
      !std::equal(m_MapMenuShortcuts.begin(), m_MapMenuShortcuts.end(), m_MapSaveMenuShortcuts.begin(),
                  shortcutmaps_equal))
     m_bShortcutsChanged = true;
   else
     m_bShortcutsChanged = false;

  M_ColWidth() = m_iColWidth;
  M_DefColWidth() = m_iDefColWidth;

  return COptions_PropertyPage::OnApply();
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

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsShortcuts::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/shortcuts_tab.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void COptionsShortcuts::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
   // If this is our list control then increase height
   if (nIDCtl == IDC_SHORTCUTLIST) {
     lpMIS->itemHeight += 10;
   }
}

void COptionsShortcuts::OnResetAll()
{
  MapMenuShortcutsIter iter;
  CString str;
  UINT id;

  for (int i = 0; i < m_ShortcutLC.GetItemCount(); i++) {
    id = (UINT)LOWORD(m_ShortcutLC.GetItemData(i));

    iter = m_MapMenuShortcuts.find(id);
    iter->second.siVirtKey = iter->second.siDefVirtKey;
    iter->second.cModifier = iter->second.cDefModifier;
  
    str = CMenuShortcut::FormatShortcut(iter);
    m_ShortcutLC.SetItemText(i, 0, str);  // SHCT_SHORTCUTKEYS
  }

  ClearWarning();

  m_ShortcutLC.RedrawItems(0, m_ShortcutLC.GetItemCount());
  m_ShortcutLC.UpdateWindow();
}

void COptionsShortcuts::OnHeaderNotify(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMHEADER *phdn = (NMHEADER *) pNotifyStruct;
  *pLResult = FALSE;

  if (phdn->pitem == NULL)
    return;

  UINT mask = phdn->pitem->mask;
  if ((mask & HDI_WIDTH) != HDI_WIDTH)
    return;

  // column width changed
  switch (phdn->hdr.code) {
    case HDN_ENDTRACK:
      // Deal with last column
      m_iColWidth = m_ShortcutLC.GetColumnWidth(0);             // SHCT_SHORTCUTKEYS
      m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // SHCT_MENUITEMTEXT
      break;
    default:
      break;
  }
}

void COptionsShortcuts::OnHeaderRClick(NMHDR *, LRESULT *pLResult)
{
  if (m_iColWidth == m_iDefColWidth)
    return;

  const DWORD dwTrackPopupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;

  CMenu menu;
  CPoint ptMousePos;
  GetCursorPos(&ptMousePos);

  if (menu.LoadMenu(IDR_POPRESETCOLUMNWIDTH)) {
    MENUINFO minfo;
    SecureZeroMemory(&minfo, sizeof(minfo));
    minfo.cbSize = sizeof(minfo);
    minfo.fMask = MIM_MENUDATA;
    minfo.dwMenuData = IDR_POPRESETCOLUMNWIDTH;
    menu.SetMenuInfo(&minfo);
    CMenu* pPopup = menu.GetSubMenu(0);

    pPopup->TrackPopupMenu(dwTrackPopupFlags, ptMousePos.x, ptMousePos.y, this);
  }
  *pLResult = TRUE;
}

void COptionsShortcuts::OnResetColumnWidth()
{
  m_ShortcutLC.SetColumnWidth(0, m_iDefColWidth);           // SHCT_SHORTCUTKEYS
  m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // SHCT_MENUITEMTEXT
  m_iColWidth = m_iDefColWidth;
}

void COptionsShortcuts::InitialSetup(const MapMenuShortcuts MapMenuShortcuts,
                    const std::vector<UINT> &ExcludedMenuItems,
                    const std::vector<st_MenuShortcut> &ReservedShortcuts)
{
  m_MapMenuShortcuts = m_MapSaveMenuShortcuts = MapMenuShortcuts;
  m_ExcludedMenuItems = ExcludedMenuItems;
  m_ReservedShortcuts = ReservedShortcuts;
}

// Functor for find_if to see if shortcut is reserved
struct reserved {
  reserved(st_MenuShortcut& st_mst) : m_st_mst(st_mst) {}
  bool operator()(st_MenuShortcut const& rdata) const
  {
    return (m_st_mst.siVirtKey  == rdata.siVirtKey &&
            m_st_mst.cModifier == rdata.cModifier);
  }

  st_MenuShortcut m_st_mst;
};

// Tortuous route to get here!
// m_HotKey looses focus and calls parent (CListCtrl) that calls here
void COptionsShortcuts::OnHotKeyKillFocus(const int item, const UINT id,
                                          const WORD wVirtualKeyCode, 
                                          const WORD wModifiers)
{
  CString str(L"");
  CString cs_warning;
  MapMenuShortcutsIter iter, inuse_iter;
  st_MenuShortcut st_mst;

  st_mst.siVirtKey  = wVirtualKeyCode;
  st_mst.cModifier = wVirtualKeyCode == 0 ? 0 : (unsigned char)wModifiers;

  // Stop compiler complaining - put this here even if not needed
  already_inuse inuse(st_mst);

  if (!CMenuShortcut::IsNormalShortcut(st_mst)) {
    // Invalid shortcut
    cs_warning.LoadString(IDS_SHCT_WARNING1);
    goto set_warning;
  }

  str = CMenuShortcut::FormatShortcut(st_mst);

  if (std::find_if(m_ReservedShortcuts.begin(),
                   m_ReservedShortcuts.end(),
                   reserved(st_mst)) != m_ReservedShortcuts.end()) {
    // Reserved shortcut ignored
    cs_warning.Format(IDS_SHCT_WARNING2, str);
    goto set_warning;
  }

  // Check not already in use (ignore if deleting current shortcut)
  iter = m_MapMenuShortcuts.find(id);
  if (st_mst.siVirtKey != 0) {
    inuse_iter = std::find_if(m_MapMenuShortcuts.begin(),
                              m_MapMenuShortcuts.end(),
                              inuse);
    if (inuse_iter != m_MapMenuShortcuts.end() && 
        inuse_iter->first != iter->first) {
      // Shortcut in use
      cs_warning.Format(IDS_SHCT_WARNING3, str, inuse_iter->second.name.c_str());
      goto set_warning;
    }
  }

  // Not reserved and not already in use - implement
  iter->second.siVirtKey = st_mst.siVirtKey;
  iter->second.cModifier = st_mst.cModifier;

  m_ShortcutLC.SetItemText(item, 0, str);  // SHCT_SHORTCUTKEYS
  m_ShortcutLC.RedrawItems(item, item);    // SHCT_MENUITEMTEXT
  m_ShortcutLC.SetColumnWidth(0, m_iColWidth);              // SHCT_SHORTCUTKEYS
  m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // SHCT_MENUITEMTEXT
  m_ShortcutLC.UpdateWindow();
  return;

set_warning:
  m_stc_warning.SetWindowText(cs_warning);
  m_stc_warning.ShowWindow(SW_SHOW);
}

int CALLBACK COptionsShortcuts::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                            LPARAM /* lParamSort */)
{
  // HIWORD is the menu position
  return (int)(HIWORD(lParam1) - HIWORD(lParam2));
}

bool COptionsShortcuts::GetMapMenuShortcutsIter(const UINT &id, MapMenuShortcutsIter &iter)
{
  iter = m_MapMenuShortcuts.find(id);
  return iter != m_MapMenuShortcuts.end();
}
