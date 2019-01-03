/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
////////////////////////////////////////////////////////////////
// Based on MSDN Magazine -- November 2003
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio .NET on Windows XP. Tab size=3.
//
#include "stdafx.h"
#include "ThisMfcApp.h"
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#include "MenuTipper.h"
#include <afxpriv.h> // for AfxLoadString

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////
// Override CSubclassWnd::WindowProc to hook messages on behalf of
// main window.
//
LRESULT CMenuTipManager::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
  if (msg == WM_MENUSELECT) {
    OnMenuSelect(LOWORD(wp), HIWORD(wp), (HMENU)lp);
  } else if (msg==WM_ENTERIDLE) {
    OnEnterIdle(wp, (HWND)lp);
  }
  return CSubclassWnd::WindowProc(msg, wp, lp);
}

//////////////////
// Got WM_MENUSELECT: show tip.
//
void CMenuTipManager::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hMenu)
{
  //  Only for MRU entries to display full path
  if (nItemID < ID_FILE_MRU_ENTRY1 || nItemID > ID_FILE_MRU_ENTRYMAX) {
    m_wndTip.Cancel();  // cancel/hide tip
    m_bMouseSelect = FALSE;
    m_bSticky = FALSE;
    return;
  }

  if (!m_wndTip.m_hWnd) {
    m_wndTip.Create(CPoint(0, 0), CWnd::FromHandle(m_hWnd));
    m_wndTip.m_szMargins = CSize(4, 0);
  }

  if ((nFlags & 0xFFFF) == 0xFFFF) {
    m_wndTip.Cancel();  // cancel/hide tip
    m_bMouseSelect = FALSE;
    m_bSticky = FALSE;

  } else if (nFlags & MF_POPUP) {
    m_wndTip.Cancel();  // new popup: cancel
    m_bSticky = FALSE;

  } else if (nFlags & MF_SEPARATOR) {
    // separator: hide tip but remember sticky state
    m_bSticky = m_wndTip.IsWindowVisible();
    m_wndTip.Cancel();

  } else if (nItemID && hMenu) {
    // if tips already displayed, keep displayed
    m_bSticky = m_wndTip.IsWindowVisible() || m_bSticky;

    // remember if mouse used to invoke menu
    m_bMouseSelect = (nFlags & MF_MOUSESELECT)!= 0;

    // get prompt and display tip (with or without timeout)
    CString prompt = (*app.GetMRU())[nItemID - ID_FILE_MRU_ENTRY1];
    if (prompt.IsEmpty())
      m_wndTip.Cancel(); // no prompt: cancel tip
    else {
      prompt.Replace(L"&", L"&&");
      prompt = L" " + prompt + L" ";
      CRect rc = GetMenuTipRect(hMenu, nItemID);
      m_wndTip.SetWindowPos(&CWnd::wndTopMost, rc.left, rc.top,
                            rc.Width(), rc.Height(), SWP_NOACTIVATE);
      m_wndTip.SetWindowText(prompt);
      m_wndTip.ShowDelayed(m_bSticky ? 0 : m_iDelay);
    }
  }
}

//////////////////
// Calculate position of tip: next to menu item.
//
CRect CMenuTipManager::GetMenuTipRect(HMENU hmenu, UINT nID)
{
  CWnd* pWndMenu = GetRunningMenuWnd(); //CWnd::WindowFromPoint(pt);
  ASSERT(pWndMenu);

  CRect rcMenu;
  pWndMenu->GetWindowRect(rcMenu); // whole menu rect

  // add heights of menu items until i reach nID
  int count = ::GetMenuItemCount(hmenu);
  int cy = rcMenu.top + GetSystemMetrics(SM_CYEDGE) + 1;
  for (int i = 0; i < count; i++) {
    CRect rc;
    ::GetMenuItemRect(m_hWnd, hmenu, i, &rc);
    if (::GetMenuItemID(hmenu,i)==nID) {
      // found menu item: adjust rectangle to right and down
      rc += CPoint(rcMenu.right - rc.left, cy - rc.top);
      return rc;
    }
    cy += rc.Height(); // add height
  }
  return CRect(0, 0, 0, 0);
}

//////////////////
// Note that windows are enumerated in top-down Z-order, so the menu
// window should always be the first one found.
//
static BOOL CALLBACK MyEnumProc(HWND hwnd, LPARAM lParam)
{
  wchar_t buf[16];
  GetClassName(hwnd, buf, _countof(buf));
  if (wcscmp(buf, L"#32768") == 0) { // special classname for menus
    *((HWND*)lParam) = hwnd;  // found it
    return FALSE;
  }
  return TRUE;
}

//////////////////
// Get running menu window.
//
CWnd* CMenuTipManager::GetRunningMenuWnd()
{
  HWND hwnd = NULL;
  EnumWindows(MyEnumProc,(LPARAM)&hwnd);
  return CWnd::FromHandle(hwnd);
}

//////////////////
// Need to handle WM_ENTERIDLE to cancel the tip if the user moved
// the mouse off the popup menu. For main menus, Windows will send a
// WM_MENUSELECT message for the parent menu when this happens, but for
// context menus there's no other way to know the user moved the mouse
// off the menu.
//
void CMenuTipManager::OnEnterIdle(WPARAM nWhy, HWND hwndWho)
{
  if (m_bMouseSelect && nWhy == MSGF_MENU) {
    CPoint pt;
    GetCursorPos(&pt);
    if (hwndWho != ::WindowFromPoint(pt)) {
      m_wndTip.Cancel();
    }
  }
}
