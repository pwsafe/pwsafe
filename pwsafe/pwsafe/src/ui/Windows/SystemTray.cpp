/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/////////////////////////////////////////////////////////////////////////////
// SystemTray.cpp : implementation file
//
// This is a conglomeration of ideas from the MSJ "Webster" application,
// sniffing round the online docs, and from other implementations such
// as PJ Naughter's "CTrayNotifyIcon" (http://indigo.ie/~pjn/ntray.html)
// especially the "CSystemTray::OnTrayNotification" member function.
// Joerg Koenig suggested the icon animation stuff
//
// This class is a light wrapper around the windows system tray stuff. It
// adds an icon to the system tray with the specified ToolTip text and
// callback notification value, which is sent back to the Parent window.
//
// The tray icon can be instantiated using either the constructor or by
// declaring the object and creating (and displaying) it later on in the
// program. eg.
//
//        CSystemTray m_SystemTray;    // Member variable of some class
//        
//        ... 
//        // in some member function maybe...
//        m_SystemTray.Create(pParentWnd, WM_MY_NOTIFY, "Click here", 
//                          hIcon, nSystemTrayID);
//
// Written by Chris Maunder (chrismaunder@codeguru.com)
// Copyright (c) 1998.
//
// Updated: 25 Jul 1998 - Added icon animation, and derived class
//                        from CWnd in order to handle messages. (CJM)
//                        (icon animation suggested by Joerg Koenig.
//                        Added API to set default menu item. Code provided
//                        by Enrico Lelina.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to your
// computer, causes your pet cat to fall ill, increases baldness or
// makes you car start emitting strange noises when you start it up.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SystemTray.h"
#include "ThisMfcApp.h"
#include "PWDialog.h" // for access to CPWDialogTracker
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#include "core/StringX.h"
#include "core/ItemData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSystemTray, CWnd)

UINT CSystemTray::m_nIDEvent = 4567;
const UINT CSystemTray::m_nTaskbarCreatedMsg = ::RegisterWindowMessage(L"TaskbarCreated");

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

CSystemTray::CSystemTray(CWnd* pParent, UINT uCallbackMessage, LPCWSTR szToolTip,
                         HICON icon, CRUEList &RUEList,
                         UINT uID, UINT menuID)
  : m_RUEList(RUEList), m_pParent(pParent), m_bEnabled(FALSE),
  m_bHidden(FALSE), m_uIDTimer(0), m_hSavedIcon(NULL), m_DefaultMenuItemID(0),
  m_DefaultMenuItemByPos(TRUE), m_pTarget(NULL), m_menuID(0)
{
  ASSERT(m_pParent != NULL);
  SecureZeroMemory(&m_tnd, sizeof(m_tnd));
  Create(pParent, uCallbackMessage, szToolTip, icon, uID, menuID);
}

BOOL CSystemTray::Create(CWnd *pParent, UINT uCallbackMessage, LPCWSTR szToolTip,
                         HICON icon, UINT uID, UINT menuID)
{
  // this is only for Windows 95 (or higher)
  m_bEnabled = (GetVersion() & 0xff) >= 4;
  if (!m_bEnabled)
    return FALSE;

  // Make sure Notification window is valid (not needed - CJM)
  // VERIFY(m_bEnabled = (pParent && ::IsWindow(pParent->GetSafeHwnd())));
  // if (!m_bEnabled) return FALSE;

  // Make sure we avoid conflict with other messages
  ASSERT(uCallbackMessage >= WM_USER);

  StringX ttt = PWSUtil::NormalizeTTT(szToolTip);

  // Create an invisible window
  CWnd::CreateEx(0, AfxRegisterWndClass(0), L"", WS_POPUP, 0,0,10,10, NULL, 0);

  // load up the NOTIFYICONDATA structure
  m_tnd.cbSize = sizeof(m_tnd);
  m_tnd.hWnd   = pParent->GetSafeHwnd() ? pParent->GetSafeHwnd() : m_hWnd;
  m_tnd.uID    = uID;
  m_tnd.hIcon  = icon;
  m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  m_tnd.uCallbackMessage = uCallbackMessage;
#if (_MSC_VER >= 1400)
  wcsncpy_s(m_tnd.szTip, sizeof(m_tnd.szTip), ttt.c_str(), ttt.length());
#else
  wcsncpy(m_tnd.szTip, ttt.c_str(), ttt.length());
#endif

  // Set the tray icon
  m_bEnabled = Shell_NotifyIcon(NIM_ADD, &m_tnd);
  ASSERT(m_bEnabled);

  m_menuID = menuID;
  return m_bEnabled;
}

CSystemTray::~CSystemTray()
{
  RemoveIcon();
  m_IconList.RemoveAll();
  ::DestroyIcon(m_hSavedIcon);
  DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray icon manipulation

void CSystemTray::MoveToRight()
{
  HideIcon();
  ShowIcon();
}

void CSystemTray::RemoveIcon()
{
  if (!m_bEnabled) return;

  m_tnd.uFlags = 0;
  Shell_NotifyIcon(NIM_DELETE, &m_tnd);
  m_bEnabled = FALSE;
}

void CSystemTray::HideIcon()
{
  if (m_bEnabled && !m_bHidden) {
    m_tnd.uFlags = NIF_ICON;
    Shell_NotifyIcon (NIM_DELETE, &m_tnd);
    m_bHidden = TRUE;
  }
}

void CSystemTray::ShowIcon()
{
  if (m_bEnabled && m_bHidden) {
    m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    Shell_NotifyIcon(NIM_ADD, &m_tnd);
    m_bHidden = FALSE;
  }
}

BOOL CSystemTray::SetIcon(HICON hIcon)
{
  if (!m_bEnabled) return FALSE;

  m_tnd.uFlags = NIF_ICON;
  m_tnd.hIcon = hIcon;

  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetIcon(LPCWSTR lpszIconName)
{
  return SetIcon(AfxGetApp()->LoadIcon(lpszIconName));
}

BOOL CSystemTray::SetIcon(UINT nIDResource)
{
  return SetIcon(AfxGetApp()->LoadIcon(nIDResource));
}

BOOL CSystemTray::SetStandardIcon(LPCWSTR lpIconName)
{
  return SetIcon(LoadIcon(NULL, lpIconName));
}

BOOL CSystemTray::SetStandardIcon(UINT nIDResource)
{
  return SetIcon(LoadIcon(NULL, MAKEINTRESOURCE(nIDResource)));
}

HICON CSystemTray::GetIcon() const
{
  return (m_bEnabled)? m_tnd.hIcon : NULL;
}

BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID) 
{
  ASSERT(uFirstIconID <= uLastIconID); // logical error
  if (uFirstIconID > uLastIconID) // fail gracefully in Release build
    return FALSE;

  const CWinApp *pApp = AfxGetApp();
  ASSERT(pApp != NULL);

  UINT nIcons = uLastIconID - uFirstIconID + 1;
  HICON *icons = new HICON[nIcons];

  for (UINT i = uFirstIconID; i <= uLastIconID; i++)
    icons[i - uFirstIconID] = pApp->LoadIcon(i);

  BOOL retval = SetIconList(icons, nIcons);
  delete[] icons;
  return retval;
}

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons)
{
  for (int i = 0; i < m_IconList.GetCount(); i++) {
   HICON& hicon = m_IconList.ElementAt(i);
   ::DestroyIcon(hicon);
  }

  m_IconList.RemoveAll();

  try {
    for (UINT i = 0; i <= nNumIcons; i++)
      m_IconList.Add(pHIconList[i]);
  }
  catch (CMemoryException *pe)
  {
    pe->ReportError();
    pe->Delete();
    m_IconList.RemoveAll();
    return FALSE;
  }

  return TRUE;
}

BOOL CSystemTray::Animate(UINT nDelayMilliSeconds, int nNumSeconds /*=-1*/)
{
  StopAnimation();

  m_nCurrentIcon = 0;
  m_StartTime = COleDateTime::GetCurrentTime();
  m_nAnimationPeriod = nNumSeconds;
  m_hSavedIcon = GetIcon();

  // Setup a timer for the animation
  m_uIDTimer = SetTimer(m_nIDEvent, nDelayMilliSeconds, NULL);

  return (m_uIDTimer != 0);
}

BOOL CSystemTray::StepAnimation()
{
  if (!m_IconList.GetSize())
    return FALSE;

  m_nCurrentIcon++;
  if (m_nCurrentIcon >= m_IconList.GetSize())
    m_nCurrentIcon = 0;

  return SetIcon(m_IconList[m_nCurrentIcon]);
}

BOOL CSystemTray::StopAnimation()
{
  BOOL bResult = FALSE;

  if (m_uIDTimer)
    bResult = KillTimer(m_uIDTimer);

  m_uIDTimer = 0;

  if (m_hSavedIcon)
    SetIcon(m_hSavedIcon);

  m_hSavedIcon = NULL;

  return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray tooltip text manipulation

BOOL CSystemTray::SetTooltipText(LPCWSTR pszTip)
{
  if (!m_bEnabled)
    return FALSE;

  StringX ttt = PWSUtil::NormalizeTTT(pszTip);
  m_tnd.uFlags = NIF_TIP;
#if (_MSC_VER >= 1400)
  wcsncpy_s(m_tnd.szTip, sizeof(m_tnd.szTip), ttt.c_str(), ttt.length());
#else
  wcsncpy(m_tnd.szTip, ttt.c_str(), ttt.length());
#endif

  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetTooltipText(UINT nID)
{
  CString strText;
  VERIFY(strText.LoadString(nID));

  return SetTooltipText(strText);
}

CString CSystemTray::GetTooltipText() const
{
  CString strText;
  if (m_bEnabled)
    strText = m_tnd.szTip;

  return strText;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification window stuff

BOOL CSystemTray::SetNotificationWnd(CWnd* pWnd)
{
  if (!m_bEnabled) return FALSE;

  // Make sure Notification window is valid
  ASSERT(pWnd && ::IsWindow(pWnd->GetSafeHwnd()));

  m_tnd.hWnd = pWnd->GetSafeHwnd();
  m_tnd.uFlags = 0;

  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

CWnd* CSystemTray::GetNotificationWnd() const
{
  return CWnd::FromHandle(m_tnd.hWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray menu manipulation

BOOL CSystemTray::SetMenuDefaultItem(UINT uItem, BOOL bByPos)
{
  if ((m_DefaultMenuItemID == uItem) && (m_DefaultMenuItemByPos == bByPos)) 
    return TRUE;

  m_DefaultMenuItemID = uItem;
  m_DefaultMenuItemByPos = bByPos;   

  CMenu menu, *pSubMenu;

  if (!menu.LoadMenu(m_menuID))
    return FALSE;

  pSubMenu = menu.GetSubMenu(0);
  if (!pSubMenu)
    return FALSE;

  ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_DefaultMenuItemID,
                       m_DefaultMenuItemByPos);

  return TRUE;
}

void CSystemTray::GetMenuDefaultItem(UINT& uItem, BOOL& bByPos) const
{
  uItem = m_DefaultMenuItemID;
  bByPos = m_DefaultMenuItemByPos;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray message handlers

BEGIN_MESSAGE_MAP(CSystemTray, CWnd)
  //{{AFX_MSG_MAP(CSystemTray)
  ON_WM_TIMER()
  ON_REGISTERED_MESSAGE(CSystemTray::m_nTaskbarCreatedMsg, OnTaskbarCreated)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#ifdef _DEBUG
void CSystemTray::OnTimer(UINT_PTR nIDEvent) 
#else
void CSystemTray::OnTimer(UINT_PTR ) 
#endif
{
  ASSERT(nIDEvent == m_nIDEvent);

  COleDateTime CurrentTime = COleDateTime::GetCurrentTime();
  COleDateTimeSpan period = CurrentTime - m_StartTime;
  if (m_nAnimationPeriod > 0 && m_nAnimationPeriod < period.GetTotalSeconds()) {
    StopAnimation();
    return;
  }

  StepAnimation();
}

// Helper function to set up recent entry submenu based on entry's attributes
static BOOL SetupRecentEntryMenu(CMenu *&pMenu, const int i, const CItemData *pci)
{
  BOOL brc;
  CString cs_text, cs_select;
  pMenu = new CMenu;
  brc = pMenu->CreatePopupMenu();
  if (brc == 0) goto exit;

  cs_text.LoadStringW(ID_MENUITEM_TRAYSELECT);
  cs_select = cs_text.Mid(1);
  brc = pMenu->InsertMenu(0, MF_BYPOSITION | MF_STRING,
                          ID_MENUITEM_TRAYSELECT1 + i,
                          cs_text);
  if (brc == 0) goto exit;

  brc = pMenu->InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
  if (brc == 0) goto exit;
  cs_text.Empty();

  int ipos = 2;

  cs_text.LoadString(IDS_TRAYCOPYPASSWORD);
  brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                          ID_MENUITEM_TRAYCOPYPASSWORD1 + i,
                          cs_text);
  if (brc == 0) goto exit;
  ipos++;

  if (!pci->IsUserEmpty()) {
    cs_text.LoadString(IDS_TRAYCOPYUSERNAME);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYCOPYUSERNAME1 + i,
                            cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  if (!pci->IsNotesEmpty()) {
    cs_text.LoadString(IDS_TRAYCOPYNOTES);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYCOPYNOTES1 + i,
                            cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  cs_text.LoadString(IDS_TRAYAUTOTYPE);
  brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                          ID_MENUITEM_TRAYAUTOTYPE1 + i,
                          cs_text);
  if (brc == 0) goto exit;
  ipos++;

  if (!pci->IsURLEmpty() && !pci->IsURLEmail()) {
    cs_text.LoadString(IDS_TRAYCOPYURL);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYCOPYURL1 + i,
                            cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  if (!pci->IsEmailEmpty() || 
      (pci->IsEmailEmpty() && !pci->IsURLEmpty() && pci->IsURLEmail())) {
    cs_text.LoadString(IDS_TRAYCOPYEMAIL);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYCOPYEMAIL1 + i,
                            cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  if (!pci->IsURLEmpty() && !pci->IsURLEmail()) {
    cs_text.LoadString(IDS_TRAYBROWSE);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYBROWSE1 + i,
                            cs_text);
    ipos++;
    cs_text.LoadString(IDS_TRAYBROWSEPLUS);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                      ID_MENUITEM_TRAYBROWSEPLUS1 + i,
                      cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  if (!pci->IsEmailEmpty() || (!pci->IsURLEmpty() && pci->IsURLEmail())) {
    cs_text.LoadString(IDS_TRAYSENDEMAIL);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYSENDEMAIL1 + i,
                            cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  if (!pci->IsRunCommandEmpty()) {
    cs_text.LoadString(IDS_TRAYRUNCOMMAND);
    brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                            ID_MENUITEM_TRAYRUNCMD1 + i,
                            cs_text);
    if (brc == 0) goto exit;
    ipos++;
  }

  cs_text.LoadString(IDS_TRAYDELETETRAYENTRY);
  brc = pMenu->InsertMenu(ipos, MF_BYPOSITION | MF_STRING,
                          ID_MENUITEM_TRAYDELETE1 + i,
                          cs_text);
  if (brc == TRUE)
    return TRUE;

exit:
  delete pMenu;
  pMenu = NULL;
  return FALSE;
}

LRESULT CSystemTray::OnTrayNotification(WPARAM wParam, LPARAM lParam) 
{
  //Return quickly if its not for this tray icon
  if (wParam != m_tnd.uID)
    return 0L;

  CMenu menu;

  // Clicking with right button brings up a context menu
  if (LOWORD(lParam) == WM_RBUTTONUP) {    
    ASSERT(m_pTarget != NULL);
    if (!menu.LoadMenu(m_menuID))
      return 0L;

    // Get pointer to the real menu (must be POPUP for TrackPopupMenu)
    CMenu *pContextMenu = menu.GetSubMenu(0);
    if (!pContextMenu)
      return 0L;
 
    int iPopupPos(2);
    const ThisMfcApp::STATE app_state = app.GetSystemTrayState();
    switch (app_state) {
      case ThisMfcApp::UNLOCKED:
      {
        if (m_pParent->IsWindowVisible()) {
          // unlocked & visible, remove "Unlock" menu item
          pContextMenu->RemoveMenu(0, MF_BYPOSITION); // Unlock
          pContextMenu->RemoveMenu(0, MF_BYPOSITION); // Separator
          iPopupPos = 0;
        } else {  // Unlocked & invisible, change 1st item to "Lock"
          const CString csLock(MAKEINTRESOURCE(IDS_LOCKSAFE));
          pContextMenu->ModifyMenu(0, MF_BYPOSITION | MF_STRING,
                                   ID_MENUITEM_TRAYLOCK, csLock);
        }
        break;
      }
      case ThisMfcApp::LOCKED:
      { // ensure 1st item is "Unlock"
        const CString csUnLock(MAKEINTRESOURCE(IDS_UNLOCKSAFE));
        pContextMenu->ModifyMenu(0, MF_BYPOSITION | MF_STRING,
                                 ID_MENUITEM_TRAYUNLOCK, csUnLock);
        // Can't do Minimize if locked
        pContextMenu->RemoveMenu(ID_MENUITEM_MINIMIZE, MF_BYCOMMAND);
        break;
      }
      case ThisMfcApp::CLOSED:
        // Remove separator and then Recent Entries popup menu
        pContextMenu->RemoveMenu(1, MF_BYPOSITION);
        pContextMenu->RemoveMenu(1, MF_BYPOSITION);
        // And then Close
        pContextMenu->RemoveMenu(ID_MENUITEM_CLOSE, MF_BYCOMMAND);
        break;
      default:
        break;
    }

    if (CPWDialog::GetDialogTracker()->AnyOpenDialogs()) {
      // Delete Close
      pContextMenu->RemoveMenu(ID_MENUITEM_CLOSE, MF_BYCOMMAND);
      // Delete Exit
      pContextMenu->RemoveMenu(ID_MENUITEM_EXIT, MF_BYCOMMAND);
      // Now that Exit is gone, delete last separator
      pContextMenu->RemoveMenu(pContextMenu->GetMenuItemCount() - 1, MF_BYPOSITION);
    }

    // Process Recent entries.
    // Allow disabled menu item if open but   locked
    // Allow  enabled menu item if open and unlocked
    size_t num_recent_entries = m_RUEList.GetCount();
    int irc;
    CMenu *pMainRecentEntriesMenu(NULL);
    CMenu **ppNewRecentEntryMenu = (CMenu **)(NULL);
    CRUEItemData* pmd;

    MENUITEMINFO miteminfo;
    SecureZeroMemory(&miteminfo, sizeof(miteminfo));
    miteminfo.cbSize = sizeof(miteminfo);
    miteminfo.fMask = MIIM_DATA;

    if (app_state != ThisMfcApp::CLOSED) {
      pMainRecentEntriesMenu = pContextMenu->GetSubMenu(iPopupPos);

      MENUINFO minfo;
      SecureZeroMemory(&minfo, sizeof(minfo));
      minfo.cbSize = sizeof(minfo);
      minfo.fMask = MIM_MENUDATA;

      minfo.dwMenuData = IDR_POPTRAY;
      pMainRecentEntriesMenu->SetMenuInfo(&minfo);

      if (num_recent_entries == 0) {
        // Only leave the "Clear Entries" menu item (greyed out in ON_UPDATE_COMMAND_UI function)
        // Delete help entry1
        pMainRecentEntriesMenu->RemoveMenu(ID_TRAYRECENT_ENTRY_HELP1, MF_BYCOMMAND);
        // Delete help entry2
        pMainRecentEntriesMenu->RemoveMenu(ID_TRAYRECENT_ENTRY_HELP2, MF_BYCOMMAND);
        // Separator - now the one after the Clear
        pMainRecentEntriesMenu->RemoveMenu(1, MF_BYPOSITION);
      } 

      // No point in doing Recent Entries if database is locked
      if (num_recent_entries != 0 && app_state == ThisMfcApp::UNLOCKED) {
        // Build extra popup menus (1 per entry in list)
        typedef CMenu* CMenuPtr;
        ppNewRecentEntryMenu = new CMenuPtr[num_recent_entries];
        m_RUEList.GetAllMenuItemStrings(m_menulist);

        for (size_t i = 0; i < num_recent_entries; i++) {
          ppNewRecentEntryMenu[i] = NULL;  // Ensure empty
          const StringX cEntry = m_menulist[i].string;
          CItemData *pci = m_menulist[i].pci;

          if (pci == NULL) {
            pws_os::Trace(L"CSystemTray::OnTrayNotification: NULL m_menulist[%d].pci\n", i);
            continue;
          }

          BOOL brc = SetupRecentEntryMenu(ppNewRecentEntryMenu[i], (int)i, pci);
          if (brc == 0) {
            pws_os::Trace(L"CSystemTray::OnTrayNotification: SetupRecentEntryMenu - ppNewRecentEntryMenu[%d] failed\n", i);
            continue;
          }

          // Insert new popup menu at the bottom of the list
          // pos 0  = Clear Entries
          // pos 1  = Note on entry format
          // pos 2  = Note on missing fields in entry
          // pos 3  = Separator
          // pos 4+ = entries.....
          irc = pMainRecentEntriesMenu->InsertMenu((int)i + 4, MF_BYPOSITION | MF_POPUP,
                                                   UINT_PTR(ppNewRecentEntryMenu[i]->m_hMenu),
                                                   cEntry.c_str());
          if (irc == 0) {
            pws_os::Trace(L"CSystemTray::OnTrayNotification: InsertMenu(%d + 4) MF_POPUP failed\n", i);
            continue;
          }

          pmd = new CRUEItemData;
          pmd->nImage = m_menulist[i].image; // Needed by OnInitMenuPopup
          miteminfo.dwItemData = (ULONG_PTR)pmd;
          irc = pMainRecentEntriesMenu->SetMenuItemInfo((int)i + 4, &miteminfo, TRUE);
          if (irc == 0) {
            pws_os::Trace(L"CSystemTray::OnTrayNotification: SetMenuItemInfo(%d + 4) failed\n", i);
          }
        } // for num_recent_entries
      } // num_recent_entries != 0 && unlocked
    } // !closed

    // Make chosen menu item the default (bold font)
    ::SetMenuDefaultItem(pContextMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

    // Display and track the popup menu
    CPoint pos;
    GetCursorPos(&pos);

    m_pTarget->SetForegroundWindow();
    ::TrackPopupMenu(pContextMenu->m_hMenu, TPM_LEFTBUTTON, pos.x, pos.y, 0,
                     m_pTarget->GetSafeHwnd(), NULL);

    // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
    m_pTarget->PostMessage(WM_NULL, 0, 0);

    if (num_recent_entries != 0 && app_state == ThisMfcApp::UNLOCKED && ppNewRecentEntryMenu != NULL) {
      for (size_t i = 0; i < num_recent_entries; i++) {
        if (ppNewRecentEntryMenu[i] == NULL)
          continue;

        irc = pMainRecentEntriesMenu->GetMenuItemInfo((int)i + 4, &miteminfo, TRUE);
        if (irc == 0) {
          pws_os::Trace(L"CSystemTray::OnTrayNotification: GetMenuItemInfo(%d + 4) failed\n", i);
          continue;
        }
        pmd = (CRUEItemData *)miteminfo.dwItemData;
        delete pmd;
        delete ppNewRecentEntryMenu[i];
      }
      delete [] ppNewRecentEntryMenu;
    }
    m_menulist.clear();
    menu.DestroyMenu();
  } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
    ASSERT(m_pTarget != NULL);
    // double click received, the default action is to execute default menu item
    m_pTarget->SetForegroundWindow();  

    UINT uItem;
    if (m_DefaultMenuItemByPos) {
      if (!menu.LoadMenu(m_menuID))
        return 0L;

      CMenu *pContextMenu = menu.GetSubMenu(0);
      if (!pContextMenu)
        return 0L;

      uItem = pContextMenu->GetMenuItemID(m_DefaultMenuItemID);
    } else
      uItem = m_DefaultMenuItemID;

    m_pTarget->SendMessage(WM_COMMAND, uItem, 0);
    menu.DestroyMenu();
  } // WM_LBUTTONDBLCLK
  return 1L;
}

LRESULT CSystemTray::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
  if (message == m_tnd.uCallbackMessage)
    return OnTrayNotification(wParam, lParam);

  return CWnd::WindowProc(message, wParam, lParam);
}

// This is called whenever the taskbar is created (eg after explorer crashes
// and restarts. Please note that the WM_TASKBARCREATED message is only passed
// to TOP LEVEL windows (like WM_QUERYNEWPALETTE)
LRESULT CSystemTray::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
  if (m_bHidden ==  FALSE) {
    m_bHidden = TRUE;
    ShowIcon();
  }
  return 0L;
}
