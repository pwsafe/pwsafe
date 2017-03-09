/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
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
#include "HKModifiers.h"
#include "HotKeyConflictDlg.h"
#include "HKModifiers.h"

#include "resource.h"
#include "resource2.h"  // Menu   resources
#include "resource3.h"  // String resources

#include "OptionsShortcuts.h" // Must be after resource.h
#include "GeneralMsgBox.h"

#include <algorithm>

using pws_os::CUUID;

// COptionsShortcuts dialog

IMPLEMENT_DYNAMIC(COptionsShortcuts, COptions_PropertyPage)

COptionsShortcuts::COptionsShortcuts(CWnd *pParent, st_Opt_master_data *pOPTMD)
: COptions_PropertyPage(pParent,
                        COptionsShortcuts::IDD, COptionsShortcuts::IDD_SHORT,
                        pOPTMD),
  m_bShortcutsChanged(false),
  m_bSortAscending(true), m_iSortedColumn(0),
  m_bKBSortAscending(true), m_iKBSortedColumn(0),
  m_iOldAppHotKey(0), m_pAutotype_HK_Font(NULL), m_bConflictDetected(false), m_PrevItem(-1)
{
  m_AppHotKeyValue = M_AppHotKey_Value();
  m_bOS_AppHotKeyEnabled = M_AppHotKeyEnabled();
  m_bOS_AutotypeHotKeyEnabled = M_AutotypeHotKeyEnabled();
  m_iColWidth = M_ColWidth();
  m_iDefColWidth = M_DefColWidth();

  m_wAppVirtualKeyCode = LOWORD(m_AppHotKeyValue);
  m_wAppModifiers = HIWORD(m_AppHotKeyValue);
}

COptionsShortcuts::~COptionsShortcuts()
{
  if (m_pAutotype_HK_Font) {
    m_pAutotype_HK_Font->DeleteObject();
    delete m_pAutotype_HK_Font;
  }
}

void COptionsShortcuts::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  DDX_Check(pDX, IDC_APPHOTKEY_ENABLE, m_bOS_AppHotKeyEnabled);
  DDX_Check(pDX, IDC_AUTOTYPEHOTKEY_ENABLE, m_bOS_AutotypeHotKeyEnabled);
  DDX_Control(pDX, IDC_APPHOTKEY_CTRL, m_AppHotKeyCtrl);
  DDX_Control(pDX, IDC_SHORTCUTLIST, m_ShortcutLC);
  DDX_Control(pDX, IDC_ENTSHORTCUTLIST, m_EntryShortcutLC);

  DDX_Control(pDX, IDC_ENTSHORTCUTLISTHELP, m_Help1);
  DDX_Control(pDX, IDC_SHORTCUTLISTHELP, m_Help2);
  DDX_Control(pDX, IDC_APPHOTKEYHELP, m_Help3);
  DDX_Control(pDX, IDC_AUTOTYPEHOTKEYHELP, m_Help4);
}

BEGIN_MESSAGE_MAP(COptionsShortcuts, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsShortcuts)
  ON_WM_MEASUREITEM()
  ON_WM_WINDOWPOSCHANGED()
  ON_WM_CONTEXTMENU()

  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_APPHOTKEY_ENABLE, OnEnableAppHotKey)
  ON_BN_CLICKED(IDC_AUTOTYPEHOTKEY_ENABLE, OnEnableAutotypeHotKey)
  ON_BN_CLICKED(IDC_RESETALLSHORTCUTS, OnResetAll)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  ON_NOTIFY(HDN_ENDTRACK, IDC_LIST_HEADER, OnHeaderNotify)
  ON_NOTIFY(NM_RCLICK, IDC_LIST_HEADER, OnHeaderRClick)
  ON_COMMAND(ID_MENUITEM_RESETCOLUMNWIDTH, OnResetColumnWidth)
  ON_EN_CHANGE(IDC_APPHOTKEY_CTRL, OnAppHotKeyChanged)

  // Entry keyboard shortcuts
  ON_NOTIFY(HDN_ITEMCLICK, IDC_ENTLIST_HEADER, OnColumnClick)
  ON_NOTIFY(NM_DBLCLK, IDC_ENTSHORTCUTLIST, OnKBShortcutDoulbleClick) 
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsShortcuts message handlers

BOOL COptionsShortcuts::OnInitDialog()
{
  COptions_PropertyPage::OnInitDialog();

  m_AppHotKeyCtrl.SetHotKey(LOWORD(m_AppHotKeyValue), HIWORD(m_AppHotKeyValue));

  // Program shortcuts
  m_ShortcutLC.Init(this);

  // Increase font of Autotype HotKey fixed value (Shift + Ctrl + T)
  CWnd *pWnd = GetDlgItem(IDC_STATIC_AUTOTYPEHOTKEY);
  pWnd->SetWindowText(CString(MAKEINTRESOURCE(IDS_AUTOTYPE_HOTKEY_VALUE)));
  CFont *pFont = pWnd->GetFont();
  LOGFONT lf;
  pFont->GetLogFont(&lf);
  lf.lfHeight -= 2;

  m_pAutotype_HK_Font = new CFont;
  m_pAutotype_HK_Font->CreateFontIndirect(&lf);
  pWnd->SetFont(m_pAutotype_HK_Font);

  // Override default HeaderCtrl ID of 0
  CHeaderCtrl *pLCHdrCtrl = m_ShortcutLC.GetHeaderCtrl();
  pLCHdrCtrl->SetDlgCtrlID(IDC_LIST_HEADER);
  m_ShortcutLC.SetHeaderCtrlID(IDC_LIST_HEADER);

  CString cs_colname;
  cs_colname.LoadString(IDS_COL_SHORTCUT);
  m_ShortcutLC.InsertColumn(0, cs_colname);  // SHCT_SHORTCUTKEYS
  cs_colname.LoadString(IDS_COL_MENUITEM);
  m_ShortcutLC.InsertColumn(1, cs_colname);  // SHCT_MENUITEMTEXT

  MapMenuShortcutsIter MenuMapiter, MenuMapiter_parent;
  CString str;
  int iItem(0);

  for (MenuMapiter = m_OSMapMenuShortcuts.begin(); MenuMapiter != m_OSMapMenuShortcuts.end();
       MenuMapiter++) {
    // We don't allow change of certain menu items
    // Just don't put in the list that the user sees.
    if (MenuMapiter->second.uiParentID == 0)
      continue;

    if (std::find(m_OSExcludedMenuItems.begin(),
                  m_OSExcludedMenuItems.end(),
                  MenuMapiter->first) != m_OSExcludedMenuItems.end())
        continue;

    str = CMenuShortcut::FormatShortcut(MenuMapiter);

    MenuMapiter_parent = m_OSMapMenuShortcuts.find(MenuMapiter->second.uiParentID);
    ASSERT(MenuMapiter_parent != m_OSMapMenuShortcuts.end());
    CString sMenuItemtext = (CString(MenuMapiter_parent->second.name.c_str()) +
                             CString(L" \xbb ") +
                             CString(MenuMapiter->second.name.c_str()));

    // Remove the ampersand from the menu item the user sees here
    sMenuItemtext.Remove(L'&');

    iItem = m_ShortcutLC.InsertItem(iItem, str);  // SHCT_SHORTCUTKEYS
    m_ShortcutLC.SetItemText(iItem, 1, sMenuItemtext); // SHCT_MENUITEMTEXT
    DWORD dwData = MAKELONG(MenuMapiter->first, MenuMapiter->second.iMenuPosition);
    m_ShortcutLC.SetItemData(iItem, dwData);
  } // foreach m_MapMenuShortcuts

  // Now sort via Menu item position
  m_ShortcutLC.SortItems(CompareFunc, NULL);

  m_ShortcutLC.SetColumnWidth(0, m_iColWidth); // SHCT_SHORTCUTKEYS
  m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // SHCT_MENUITEMTEXT

  m_ShortcutLC.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);

  // Entry shortcuts

  // Override default HeaderCtrl ID of 0
  CHeaderCtrl *pEntryLCHdrCtrl = m_EntryShortcutLC.GetHeaderCtrl();
  pEntryLCHdrCtrl->SetDlgCtrlID(IDC_ENTLIST_HEADER);

  DWORD dwExtendedStyle = m_EntryShortcutLC.GetExtendedStyle() | LVS_EX_FULLROWSELECT;
  m_EntryShortcutLC.SetExtendedStyle(dwExtendedStyle);

  cs_colname.LoadString(IDS_COL_SHORTCUT);
  m_EntryShortcutLC.InsertColumn(0, cs_colname);  // SHORTCUT
  cs_colname.LoadString(IDS_GROUP);
  m_EntryShortcutLC.InsertColumn(1, cs_colname);  // GROUP
  cs_colname.LoadString(IDS_TITLE);
  m_EntryShortcutLC.InsertColumn(2, cs_colname);  // TITLE
  cs_colname.LoadString(IDS_USERNAME);
  m_EntryShortcutLC.InsertColumn(3, cs_colname);  // USER

  iItem = 0;

  KBShortcutMapConstIter kbiter;

  m_OSKBShortcutMap = GetMainDlg()->GetAllKBShortcuts();

  for (kbiter = m_OSKBShortcutMap.begin(); kbiter != m_OSKBShortcutMap.end();
       kbiter++) {
    int32 iKBShortcut = kbiter->first;
    WORD wVirtualKeyCode = iKBShortcut & 0xff;
    WORD wPWSModifiers = iKBShortcut >> 16;

    // Translate from PWS modifiers to HotKey
    WORD wHKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);

    str = CMenuShortcut::FormatShortcut(wHKModifiers, wVirtualKeyCode);

    ItemListIter iter = app.GetCore()->Find(kbiter->second);
    const StringX sxGroup = iter->second.GetGroup();
    const StringX sxTitle = iter->second.GetTitle();
    const StringX sxUser  = iter->second.GetUser();
    iItem = m_EntryShortcutLC.InsertItem(iItem, str);  // SHCT_SHORTCUTKEYS
    ASSERT(iItem != -1);
    m_EntryShortcutLC.SetItemText(iItem, 1, sxGroup.c_str()); // Group
    m_EntryShortcutLC.SetItemText(iItem, 2, sxTitle.c_str()); // Title
    m_EntryShortcutLC.SetItemText(iItem, 3, sxUser.c_str());  // User
    m_EntryShortcutLC.SetItemData(iItem, iKBShortcut);
  } // foreach mapKBShortcutMap

  // Now sort via keyboard shortcut
  m_EntryShortcutLC.SortItems(CKBSHCompareFunc, (LPARAM)this);

  m_EntryShortcutLC.SetColumnWidth(0, m_iColWidth); // SHCT_SHORTCUTKEYS
  m_EntryShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // Group
  m_EntryShortcutLC.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER); // Title
  m_EntryShortcutLC.SetColumnWidth(3, LVSCW_AUTOSIZE_USEHEADER); // User

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_QUESTIONMARK);
    m_Help4.Init(IDB_QUESTIONMARK);

    AddTool(IDC_ENTSHORTCUTLISTHELP, IDS_ENTKBSHCTHOTKEYHELP2);
    AddTool(IDC_SHORTCUTLISTHELP, IDS_SHORTCUTLISTHELP);
    AddTool(IDC_APPHOTKEYHELP, IDS_APPHOTKEYHELP);
    AddTool(IDC_AUTOTYPEHOTKEYHELP, IDS_AUTOTYPEHOTKEYHELP);

    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
    m_Help4.EnableWindow(FALSE);
    m_Help4.ShowWindow(SW_HIDE);
  }

  // Set variable so that OnSetActive checks the Application & Autotype hotkeys
  m_bFirstShown = true;
  return TRUE;
}

bool shortcutmaps_equal (MapMenuShortcutsPair p1, MapMenuShortcutsPair p2)
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
      if (M_AppHotKeyEnabled()        != m_bOS_AppHotKeyEnabled          ||
          M_AppHotKey_Value()         != m_AppHotKeyValue                ||
          M_AutotypeHotKeyEnabled()   != m_bOS_AutotypeHotKeyEnabled     ||
          m_OSMapMenuShortcuts.size() != m_OSMapSaveMenuShortcuts.size() ||
          !std::equal(m_OSMapMenuShortcuts.begin(), m_OSMapMenuShortcuts.end(),
                      m_OSMapSaveMenuShortcuts.begin(), shortcutmaps_equal)) {
        m_bShortcutsChanged = true;
        return 1L;
      } else
        m_bShortcutsChanged = false;
      break;
    case PPOPT_HOTKEY_SET:
      return (m_bOS_AppHotKeyEnabled == TRUE) ? 1L : 0L;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL COptionsShortcuts::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  WORD wVirtualKeyCode, wHKModifiers;
  m_AppHotKeyCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);

  if (CheckHotKey(wVirtualKeyCode, wHKModifiers, PWS_HOTKEY_ID) < 0)
    return FALSE;

  if (CheckHotKey(wVirtualKeyCode, wHKModifiers, PWS_AT_HOTKEY_ID) < 0)
    return FALSE;

  return CPWPropertyPage::OnKillActive();
}

void COptionsShortcuts::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
  CPWPropertyPage::OnWindowPosChanged(lpwndpos);

  if (m_bFirstShown && (lpwndpos->flags & SWP_SHOWWINDOW)) {
    m_bFirstShown = false;
    int iRC = app.GetMainDlg()->SetAndCheckHotKeys(false);
    if (iRC != CHotKeyConflictDlg::HKE_NONE) {
      if (iRC & CHotKeyConflictDlg::HKE_APP) {
        ((CButton *)GetDlgItem(IDC_APPHOTKEY_ENABLE))->SetCheck(BST_UNCHECKED);
        m_bOS_AppHotKeyEnabled = FALSE;
      }
      if (iRC & CHotKeyConflictDlg::HKE_AT) {
        ((CButton *)GetDlgItem(IDC_AUTOTYPEHOTKEY_ENABLE))->SetCheck(BST_UNCHECKED);
        m_bOS_AutotypeHotKeyEnabled = FALSE;
      }
    }
  }
}

BOOL COptionsShortcuts::OnApply()
{
  UpdateData(TRUE);

  WORD wVirtualKeyCode, wHKModifiers;
  m_AppHotKeyCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);

  if (CheckHotKey(wVirtualKeyCode, wHKModifiers, PWS_HOTKEY_ID) < 0)
    return FALSE;

  if (CheckHotKey(wVirtualKeyCode, wHKModifiers, PWS_AT_HOTKEY_ID) < 0)
    return FALSE;

  m_AppHotKeyValue = (wHKModifiers << 16) + wVirtualKeyCode;

  if (m_OSMapMenuShortcuts.size() != m_OSMapSaveMenuShortcuts.size() ||
      !std::equal(m_OSMapMenuShortcuts.begin(), m_OSMapMenuShortcuts.end(),
                  m_OSMapSaveMenuShortcuts.begin(), shortcutmaps_equal))
     m_bShortcutsChanged = true;
   else
     m_bShortcutsChanged = false;

  M_AppHotKey_Value() = m_AppHotKeyValue;
  M_AppHotKeyEnabled() = m_bOS_AppHotKeyEnabled;
  M_AutotypeHotKeyEnabled() = m_bOS_AutotypeHotKeyEnabled;
  M_ColWidth() = m_iColWidth;
  M_DefColWidth() = m_iDefColWidth;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsShortcuts::PreTranslateMessage(MSG* pMsg)
{
  RelayToolTipEvent(pMsg);

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

  // Prevent user pressing the ENTER key in the Application HotKeyCtrl and
  // causing the property page to close (e.g. OK button).
  // This will just reset the HotKey to None
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
      GetFocus() == (CWnd *)&m_AppHotKeyCtrl)
    return TRUE;

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsShortcuts::OnHelp()
{
  ShowHelp(L"::/html/shortcuts_tab.html");
}

void COptionsShortcuts::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
   // If this is our list control then increase height
   if (nIDCtl == IDC_SHORTCUTLIST) {
     lpMIS->itemHeight += 10;
   }
}

void COptionsShortcuts::OnAppHotKeyChanged()
{
  WORD wVirtualKeyCode, wModifiers;
  m_AppHotKeyCtrl.GetHotKey(wVirtualKeyCode, wModifiers);

  if (wVirtualKeyCode == AUTOTYPE_HOTKEY_KEYCODE && wModifiers == AUTOTYPE_HOTKEY_MODIFIERS) {
    // Same as PWS Autotype HotKey
    CGeneralMsgBox gmb;
    CString cs_errmsg, csHotKey(MAKEINTRESOURCE(IDS_AUTOTYPE_HOTKEY_VALUE));
    CString cs_Reset(MAKEINTRESOURCE(IDS_KBS_RESET));
    cs_errmsg.Format(IDS_AUTOTYPE_RESERVED, static_cast<LPCWSTR>(cs_Reset));
    CString cs_title;
    cs_title.Format(IDS_SHORTCUT_CONFLICT, csHotKey);

    gmb.MessageBox(cs_errmsg, cs_title, MB_OK | MB_ICONSTOP);

    // Restore last saved values
    m_AppHotKeyCtrl.SetHotKey(m_wAppVirtualKeyCode, m_wAppModifiers);
    return;
  }
}

void COptionsShortcuts::OnEnableAppHotKey()
{
  UpdateData(TRUE);
  
  m_PrevItem = -1;
  m_bConflictDetected = false;

  if (((CButton *)GetDlgItem(IDC_APPHOTKEY_ENABLE))->GetCheck() == BST_CHECKED) {
    WORD wVirtualKeyCode, wHKModifiers;
    m_AppHotKeyCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);

    if (CheckHotKey(wVirtualKeyCode, wHKModifiers, PWS_HOTKEY_ID) == HOTKEY_UNIQUE) {
      GetDlgItem(IDC_APPHOTKEY_CTRL)->EnableWindow(TRUE);
      GetDlgItem(IDC_APPHOTKEY_CTRL)->SetFocus();
    } else {
      ((CButton *)GetDlgItem(IDC_APPHOTKEY_ENABLE))->SetCheck(BST_UNCHECKED);
      m_bOS_AppHotKeyEnabled = FALSE;
    }
  }
}

void COptionsShortcuts::OnEnableAutotypeHotKey()
{
  UpdateData(TRUE);

  m_PrevItem = -1;
  m_bConflictDetected = false;

  if (((CButton *)GetDlgItem(IDC_AUTOTYPEHOTKEY_ENABLE))->GetCheck() == BST_CHECKED) {
    WORD wVirtualKeyCode(AUTOTYPE_HOTKEY_KEYCODE), wHKModifiers(AUTOTYPE_HOTKEY_MODIFIERS);
    if (CheckHotKey(wVirtualKeyCode, wHKModifiers, PWS_AT_HOTKEY_ID) != HOTKEY_UNIQUE) {
      ((CButton *)GetDlgItem(IDC_AUTOTYPEHOTKEY_ENABLE))->SetCheck(BST_UNCHECKED);
      m_bOS_AutotypeHotKeyEnabled = FALSE;
    }
  }
}

void COptionsShortcuts::OnResetAll()
{
  MapMenuShortcutsIter iter;
  CString str;

  for (int i = 0; i < m_ShortcutLC.GetItemCount(); i++) {
    UINT id = (UINT)LOWORD(m_ShortcutLC.GetItemData(i));

    iter = m_OSMapMenuShortcuts.find(id);
    iter->second.siVirtKey = iter->second.siDefVirtKey;
    iter->second.cModifier = iter->second.cDefModifier;
  
    str = CMenuShortcut::FormatShortcut(iter);
    m_ShortcutLC.SetItemText(i, 0, str);  // SHCT_SHORTCUTKEYS
  }

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
    CMenu *pPopup = menu.GetSubMenu(0);

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
  m_OSMapMenuShortcuts = m_OSMapSaveMenuShortcuts = MapMenuShortcuts;
  m_OSExcludedMenuItems = ExcludedMenuItems;
  m_OSReservedShortcuts = ReservedShortcuts;
}

// Tortuous route to get here!
// Menu m_HotKey looses focus and calls parent (CListCtrl) that calls here
bool COptionsShortcuts::OnMenuShortcutKillFocus(const int item, const UINT id,
                                                const WORD wVirtualKeyCode,
                                                const WORD wHKModifiers)
{
  if (item != m_PrevItem)
    m_bConflictDetected = false;

  m_PrevItem = item;

  if (m_bConflictDetected)
    return false;

  CString str(L"");
  CString cs_warning;
  CString cs_title;
  CGeneralMsgBox gmb;
  MapMenuShortcutsIter iter, inuse_iter;
  st_MenuShortcut st_mst;

  st_mst.siVirtKey = wVirtualKeyCode;
  st_mst.cModifier = wVirtualKeyCode == 0 ? 0 : (unsigned char)wHKModifiers;

  // Check if user has assigned the application hotkey to a menu item
  WORD wAppVirtualKeyCode, wAppHKModifiers;
  m_AppHotKeyCtrl.GetHotKey(wAppVirtualKeyCode, wAppHKModifiers);

  if (m_bOS_AppHotKeyEnabled &&
      wVirtualKeyCode == wAppVirtualKeyCode &&
      wHKModifiers == wAppHKModifiers) {

    // Same as PWS application HotKey
    gmb.AfxMessageBox(IDS_KBS_SAMEASAPP, MB_OK);
    return false;
  }

  if (m_bOS_AutotypeHotKeyEnabled &&
      wVirtualKeyCode == AUTOTYPE_HOTKEY_KEYCODE &&
      wHKModifiers == AUTOTYPE_HOTKEY_MODIFIERS) {

    // Same as PWS Autotype HotKey
    CString cs_errmsg, csHotKey(MAKEINTRESOURCE(IDS_AUTOTYPE_HOTKEY_VALUE));
    CString cs_Reset(MAKEINTRESOURCE(IDS_KBS_RESET));
    cs_errmsg.Format(IDS_AUTOTYPE_RESERVED, static_cast<LPCWSTR>(cs_Reset));
    cs_title.Format(IDS_SHORTCUT_CONFLICT, csHotKey);

    gmb.MessageBox(cs_errmsg, cs_title, MB_OK | MB_ICONSTOP);
    return false;
  }

  // Stop compiler complaining - put this here even if not needed
  already_inuse inuse(st_mst);

  if (!CMenuShortcut::IsNormalShortcut(st_mst)) {
    // Invalid shortcut
    cs_title.LoadString(IDS_SHORTCUT_WARNING);
    cs_warning.LoadString(IDS_SHORTCUT_WARNING1);
    goto set_warning;
  }

  str = CMenuShortcut::FormatShortcut(st_mst);

  if (std::find_if(m_OSReservedShortcuts.begin(),
                   m_OSReservedShortcuts.end(),
                   reserved(st_mst)) != m_OSReservedShortcuts.end()) {
    // Reserved shortcut ignored
    cs_title.LoadString(IDS_SHORTCUT_WARNING);
    cs_warning.Format(IDS_SHORTCUT_WARNING2, static_cast<LPCWSTR>(str));
    goto set_warning;
  }

  // Check not already in use (ignore if deleting current shortcut)
  iter = m_OSMapMenuShortcuts.find(id);
  if (st_mst.siVirtKey != 0) {
    inuse_iter = std::find_if(m_OSMapMenuShortcuts.begin(),
                              m_OSMapMenuShortcuts.end(),
                              inuse);
    if (inuse_iter != m_OSMapMenuShortcuts.end() &&
        inuse_iter->first != iter->first) {
      // Shortcut in use
      CString cs_MenuItemName = inuse_iter->second.name.c_str();
      cs_MenuItemName.Remove(L'&');
      cs_title.Format(IDS_SHORTCUT_CONFLICT, static_cast<LPCWSTR>(str));
      cs_warning.Format(IDS_SHORTCUT_CONFLICT1, static_cast<LPCWSTR>(cs_MenuItemName));
      goto set_warning;
    }
  }

  // Not reserved and not already in use - implement
  iter->second.siVirtKey = st_mst.siVirtKey;
  iter->second.cModifier = st_mst.cModifier;

  m_ShortcutLC.SetItemText(item, 0, str);                   // SHCT_SHORTCUTKEYS
  m_ShortcutLC.RedrawItems(item, item);                     // SHCT_MENUITEMTEXT
  m_ShortcutLC.SetColumnWidth(0, m_iColWidth);              // SHCT_SHORTCUTKEYS
  m_ShortcutLC.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // SHCT_MENUITEMTEXT
  m_ShortcutLC.UpdateWindow();
  return true;

set_warning:
  gmb.MessageBox(cs_warning, cs_title, MB_OK | MB_ICONSTOP);
  m_bConflictDetected = true;
  return false;
}

void COptionsShortcuts::OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMHEADER *pNMHeaderCtrl  = (NMHEADER *)pNotifyStruct;

  // Get column number to CItemData value
  int iKBSortColumn = pNMHeaderCtrl->iItem;

  if (m_iKBSortedColumn == iKBSortColumn) {
    m_bKBSortAscending = !m_bKBSortAscending;
  } else {
    m_iKBSortedColumn = iKBSortColumn;
    m_bKBSortAscending = true;
  }

  m_EntryShortcutLC.SortItems(CKBSHCompareFunc, (LPARAM)this);

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  CHeaderCtrl *pHDRCtrl;

  pHDRCtrl = m_EntryShortcutLC.GetHeaderCtrl();
  pHDRCtrl->GetItem(iKBSortColumn, &hdi);
  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  pHDRCtrl->SetItem(iKBSortColumn, &hdi);

  *pLResult = TRUE; // Say we have done all processing on return
}

/*
* Compare function used by m_EntryShortcutLC.SortItems()
* "The comparison function must return a negative value if the first item should precede
* the second, a positive value if the first item should follow the second, or zero if
* the two items are equivalent."
*/
int CALLBACK COptionsShortcuts::CKBSHCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                                 LPARAM lParamSort)
{
  // m_bSortAscending to determine the direction of the sort (duh)

  COptionsShortcuts *self = (COptionsShortcuts *)lParamSort;
  int iResult(0);
  const int nSortColumn = self->m_iKBSortedColumn;
  StringX sxLHS, sxRHS;
  ItemListIter iLHS, iRHS;

  if (nSortColumn == 0) {
    WORD wLHS_VirtualKeyCode, wRHS_VirtualKeyCode, wPWSModifiers;
    wLHS_VirtualKeyCode = lParam1& 0xff;
    wPWSModifiers = WORD(lParam1 >> 16);

    WORD wLHS_HKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);

    wRHS_VirtualKeyCode = lParam2 & 0xff;
    wPWSModifiers = WORD(lParam2 >> 16);

    WORD wRHS_HKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);

    if (wLHS_HKModifiers != wRHS_HKModifiers)
      iResult = wRHS_HKModifiers < wLHS_HKModifiers ? -1 : 1;
    else
      iResult = wRHS_VirtualKeyCode < wLHS_VirtualKeyCode ? 1 : -1;

  } else {
    pws_os::CUUID &LHS_UUID = self->m_OSKBShortcutMap[int(lParam1)];
    pws_os::CUUID &RHS_UUID = self->m_OSKBShortcutMap[int(lParam2)];
    iLHS = app.GetCore()->Find(LHS_UUID);
    iRHS = app.GetCore()->Find(RHS_UUID);
    switch (nSortColumn) {
      case 1:
        sxLHS = iLHS->second.GetGroup();
        sxRHS = iRHS->second.GetGroup();
        break;
      case 2:
        sxLHS = iLHS->second.GetTitle();
        sxRHS = iRHS->second.GetTitle();
        break;
      case 3:
        sxLHS = iLHS->second.GetUser();
        sxRHS = iRHS->second.GetUser();
        break;
      default:
        break;
    }

    iResult = CompareNoCase(sxLHS, sxRHS);
  }

  if (!self->m_bKBSortAscending && iResult != 0) {
    iResult *= -1;
  }
  return iResult;
}

int CALLBACK COptionsShortcuts::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                            LPARAM /* lParamSort */)
{
  // HIWORD is the menu position
  return (int)(HIWORD(lParam1) - HIWORD(lParam2));
}

bool COptionsShortcuts::GetMapMenuShortcutsIter(const UINT &id, MapMenuShortcutsIter &iter)
{
  iter = m_OSMapMenuShortcuts.find(id);
  return iter != m_OSMapMenuShortcuts.end();
}

void COptionsShortcuts::RefreshKBShortcuts()
{
  m_OSKBShortcutMap = GetMainDlg()->GetAllKBShortcuts();
}

void COptionsShortcuts::OnKBShortcutDoulbleClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0;

  NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNotifyStruct; 
  int iItem = pNMListView->iItem; 

  if (iItem == -1)
    return;

  ItemListIter iter;

  // Disable parent whilst editing entry
  m_options_psh->EnableWindow(FALSE);

  DWORD_PTR lParam = m_EntryShortcutLC.GetItemData(iItem);
  pws_os::CUUID &EntryUUID = GetKBShortcutUUID((int)(INT_PTR)lParam);
  iter = app.GetCore()->Find(EntryUUID);
  bool bEdited = GetMainDlg()->EditItem(&iter->second, NULL);
  if (!GetMainDlg()->IsDBReadOnly() && bEdited) {
    // User may have changed shortcut, group, title and/or user fields.
    int32 iKBShortcut;
    iter->second.GetKBShortcut(iKBShortcut);

    // Refresh map
    RefreshKBShortcuts();

    WORD wVirtualKeyCode, wPWSModifiers;
    wVirtualKeyCode = iKBShortcut & 0xff;
    wPWSModifiers = iKBShortcut >> 16;

    // Translate from PWS modifiers to HotKey
    WORD wHKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);
    CString str = CMenuShortcut::FormatShortcut(wHKModifiers, wVirtualKeyCode);

    const StringX sxGroup = iter->second.GetGroup();
    const StringX sxTitle = iter->second.GetTitle();
    const StringX sxUser  = iter->second.GetUser();
    m_EntryShortcutLC.SetItemText(iItem, ENTRYSHCT_SHORTCUTKEYS, str);      // SHORTCUTKEYS
    m_EntryShortcutLC.SetItemText(iItem, ENTRYSHCT_GROUP, sxGroup.c_str()); // Group
    m_EntryShortcutLC.SetItemText(iItem, ENTRYSHCT_TITLE, sxTitle.c_str()); // Title
    m_EntryShortcutLC.SetItemText(iItem, ENTRYSHCT_USER, sxUser.c_str());   // User
    m_EntryShortcutLC.SetItemData(iItem, iKBShortcut);
   
    m_EntryShortcutLC.RedrawItems(iItem, iItem);
  }

  // Re-enable parent whilst editing entry
  m_options_psh->EnableWindow(TRUE);

  UpdateWindow();
}

int COptionsShortcuts::CheckHotKey(WORD &wVirtualKeyCode, WORD &wHKModifiers, int iWhichHotKey)
{
  if (m_bConflictDetected)
    return -1;

  const CString cs_AutotypeHotKeyValue(MAKEINTRESOURCE(IDS_AUTOTYPE_HOTKEY_VALUE));

  int32 iAppHotKey;
  CGeneralMsgBox gmb;
  CString cs_errmsg1, cs_errmsg2, cs_errmsg3, cs_errmsg4, cs_msg;
  int iRC(0);

  WORD wPWSModifiers;
  m_AppHotKeyCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);
  
  // Translate from CHotKeyCtrl to PWS modifiers
  wPWSModifiers = ConvertModifersMFC2PWS(wHKModifiers);
  iAppHotKey = (wPWSModifiers << 16) + wVirtualKeyCode;

  if (iAppHotKey != 0 && iWhichHotKey == PWS_HOTKEY_ID) {
    if ((wPWSModifiers & (PWS_HOTKEYF_ALT | PWS_HOTKEYF_CONTROL)) == 0) {
      // Add Alt and/or Ctrl key and tell user but first check not already in use
      pws_os::CUUID chk_uuid;

      WORD wValidModifierCombos[] = {
                   PWS_HOTKEYF_ALT,
                   PWS_HOTKEYF_ALT | PWS_HOTKEYF_SHIFT,
                   PWS_HOTKEYF_CONTROL,
                   PWS_HOTKEYF_CONTROL | PWS_HOTKEYF_SHIFT,
                   PWS_HOTKEYF_ALT | PWS_HOTKEYF_CONTROL,
                   PWS_HOTKEYF_ALT | PWS_HOTKEYF_CONTROL | PWS_HOTKEYF_SHIFT };
      int iActions[] = { IDS_KBS_ADD_ALT,     IDS_KBS_ADD_ALTSHIFT,
                         IDS_KBS_ADD_CTRL,    IDS_KBS_ADD_CTRLSHIFT,
                         IDS_KBS_ADD_ALTCTRL, IDS_KBS_ADD_ALTCTRLSHIFT };

      // Try them in order
      int iChange, ierror(IDS_KBS_CANTADD);
      for (iChange = 0; iChange < sizeof(wValidModifierCombos) / sizeof(WORD); iChange++) {
        StringX sxMenuItemName;

        int iNewAppHotKey = wValidModifierCombos[iChange] + wVirtualKeyCode;
        wPWSModifiers = wValidModifierCombos[iChange];
        wHKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);
        unsigned char ucModifiers = wHKModifiers & 0xff;

        unsigned int nCID = GetMainDlg()->GetMenuShortcut(wVirtualKeyCode,
                                                  ucModifiers, sxMenuItemName, &m_OSMapMenuShortcuts);

        // If not in use by a menu, check entry shortcuts
        if (nCID == 0) {
          chk_uuid = app.GetCore()->GetKBShortcut(iNewAppHotKey);
          if (chk_uuid == CUUID::NullUUID()) {
            ierror = iActions[iChange];
            break;
          }
        }
      }

      if (ierror == IDS_KBS_CANTADD) {
        wVirtualKeyCode = wHKModifiers = wPWSModifiers = 0;
        iRC += HOTKEY_CANT_MAKE_UNIQUE;
      } else {
        iRC += HOTKEY_MADE_UNIQUE;
      }
      
      cs_msg.LoadString(ierror);
      cs_errmsg1.Format(IDS_KBS_INVALID, static_cast<LPCWSTR>(cs_msg));
      CString cs_title(MAKEINTRESOURCE(IDS_SHORTCUT_WARNING));
      gmb.MessageBox(cs_errmsg1, cs_title, MB_OK | MB_ICONSTOP);

      // Get new keyboard shortcut
      m_iOldAppHotKey = iAppHotKey = (wPWSModifiers << 16) + wVirtualKeyCode;
      ((CHotKeyCtrl *)GetDlgItem(IDC_APPHOTKEY_CTRL))->SetFocus();

      m_AppHotKeyCtrl.SetHotKey(wVirtualKeyCode, wHKModifiers);
      m_bConflictDetected = true;
      return iRC;
    }
  }

  StringX sxMenuItemName;
  unsigned char ucModifiers = wHKModifiers & 0xff;

  if (m_bOS_AppHotKeyEnabled && iWhichHotKey == PWS_HOTKEY_ID) {
    iRC = 0;
    CString cs_AppHotKeyValue = m_AppHotKeyCtrl.GetHotKeyName();

    // First check if in use by an entry
    pws_os::CUUID uuid = app.GetCore()->GetKBShortcut(iAppHotKey);

    if (uuid != CUUID::NullUUID()) {
      // Tell user that it already exists as an entry keyboard shortcut
      ItemListIter iter = app.GetCore()->Find(uuid);
      const StringX sxGroup = iter->second.GetGroup();
      const StringX sxTitle = iter->second.GetTitle();
      const StringX sxUser = iter->second.GetUser();

      cs_errmsg1.Format(IDS_KBS_INUSEBYENTRY, static_cast<LPCWSTR>(cs_AppHotKeyValue),
        static_cast<LPCWSTR>(sxGroup.c_str()),
        static_cast<LPCWSTR>(sxTitle.c_str()),
        static_cast<LPCWSTR>(sxUser.c_str()),
        L"");

      iRC = HOTKEY_IN_USE_BY_ENTRY;
    }

    unsigned int nCID = GetMainDlg()->GetMenuShortcut(wVirtualKeyCode,
                                              ucModifiers, sxMenuItemName, &m_OSMapMenuShortcuts);

    // Next check if in use by a menu item (on this instance for this user!)
    if (nCID != 0) {
      Remove(sxMenuItemName, L'&');

      cs_errmsg2.Format(IDS_KBS_INUSEBYMENU, static_cast<LPCWSTR>(cs_AppHotKeyValue),
        static_cast<LPCWSTR>(sxMenuItemName.c_str()));

      iRC += HOTKEY_IN_USE_BY_MENU;
    }
  }

  if (m_bOS_AutotypeHotKeyEnabled && iWhichHotKey == PWS_AT_HOTKEY_ID) {
    int32 iAutotypeHotKey = (AUTOTYPE_HOTKEY_MODIFIERS << 16) + AUTOTYPE_HOTKEY_KEYCODE;
    pws_os::CUUID uuid = app.GetCore()->GetKBShortcut(iAutotypeHotKey);

    if (uuid != CUUID::NullUUID()) {
      // Tell user that it already exists as an entry keyboard shortcut
      ItemListIter iter = app.GetCore()->Find(uuid);
      const StringX sxGroup = iter->second.GetGroup();
      const StringX sxTitle = iter->second.GetTitle();
      const StringX sxUser = iter->second.GetUser();

      cs_errmsg3.Format(IDS_KBS_INUSEBYENTRY, static_cast<LPCWSTR>(cs_AutotypeHotKeyValue),
        static_cast<LPCWSTR>(sxGroup.c_str()),
        static_cast<LPCWSTR>(sxTitle.c_str()),
        static_cast<LPCWSTR>(sxUser.c_str()),
        L"");

      iRC += HOTKEY_IN_USE_BY_ENTRY;
    }

    ucModifiers = (unsigned char)ConvertModifersPWS2MFC(AUTOTYPE_HOTKEY_MODIFIERS);
    unsigned int nCID = GetMainDlg()->GetMenuShortcut(AUTOTYPE_HOTKEY_KEYCODE,
                                              ucModifiers, sxMenuItemName, &m_OSMapMenuShortcuts);

    // Next check if in use by a menu item (on this instance for this user!)
    if (nCID != 0) {
      Remove(sxMenuItemName, L'&');

      cs_errmsg4.Format(IDS_KBS_INUSEBYMENU, static_cast<LPCWSTR>(cs_AutotypeHotKeyValue),
        static_cast<LPCWSTR>(sxMenuItemName.c_str()));

      iRC += HOTKEY_IN_USE_BY_MENU;
    }
  }

  if (iRC != 0) {
    CString cs_errmsg(cs_errmsg1);
    if (!cs_errmsg2.IsEmpty())
      cs_errmsg += (cs_errmsg.IsEmpty() ? L"" : L"\n\n") + cs_errmsg2;
    if (!cs_errmsg3.IsEmpty())
      cs_errmsg += (cs_errmsg.IsEmpty() ? L"" : L"\n\n") + cs_errmsg3;
    if (!cs_errmsg4.IsEmpty())
      cs_errmsg += (cs_errmsg.IsEmpty() ? L"" : L"\n\n") + cs_errmsg4;

    pws_os::Trace(L"COptionsShortcuts::CheckHotKey#2\n");
    CString cs_title, cs_Key(MAKEINTRESOURCE(iWhichHotKey == PWS_HOTKEY_ID ? IDS_APPHOTKEY : IDS_AUTOTYPE_HOTKEY));
    cs_title.Format(IDS_SHORTCUT_CONFLICT, static_cast<LPCWSTR>(cs_Key));
    gmb.MessageBox(cs_errmsg, cs_title, MB_OK | MB_ICONSTOP);

    m_bConflictDetected = true;
    return iRC;
  } else {
    m_bConflictDetected = false;
    return HOTKEY_UNIQUE;
  }
}

void COptionsShortcuts::OnContextMenu(CWnd *, CPoint point)
{
  CRect rect;

  m_AppHotKeyCtrl.GetWindowRect(&rect);
  if (rect.PtInRect(point)) {
    WORD wVirtualKeyCode, wHKModifiers;
    m_AppHotKeyCtrl.GetHotKey(wVirtualKeyCode, wHKModifiers);

    if (wVirtualKeyCode == 0 && wHKModifiers == 0)
      return;

    CMenu PopupMenu;
    PopupMenu.LoadMenu(IDR_POPRESETSHORTCUT);
    CMenu *pContextMenu = PopupMenu.GetSubMenu(0);
    pContextMenu->RemoveMenu(ID_MENUITEM_REMOVESHORTCUT, MF_BYCOMMAND);

    int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
      point.x, point.y, this);

    if (nID == ID_MENUITEM_RESETSHORTCUT) {
      m_AppHotKeyCtrl.SetHotKey(0, 0);

      // Need to update my copy of the map
      // Get row for Menu information
      UINT uFlags;
      int index = m_ShortcutLC.HitTest(point, &uFlags);
      ASSERT(index != -1);  // Can't be -1 as user got here via right click on a row!
      UINT id = (UINT)LOWORD(m_ShortcutLC.GetItemData(index));

      // Now remove from my copy.
      MapMenuShortcutsIter iter = m_OSMapMenuShortcuts.find(id);
      ASSERT(iter != m_OSMapMenuShortcuts.end());

      if (iter != m_OSMapMenuShortcuts.end()) {
        iter->second.cModifier = iter->second.cDefModifier;
        iter->second.siVirtKey = iter->second.siDefVirtKey;
      }
    }
  }
}
