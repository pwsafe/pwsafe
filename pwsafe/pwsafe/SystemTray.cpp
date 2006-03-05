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
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSystemTray, CWnd)

UINT CSystemTray::m_nIDEvent = 4567;
const UINT CSystemTray::m_nTaskbarCreatedMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

#if 0 // XXX cleanup 
CSystemTray::CSystemTray()
{
    Initialise();
}
#endif

CSystemTray::CSystemTray(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip,
                         HICON icon, CList<CString,CString&> &recentEntriesList,
                         UINT uID, UINT menuID)
  : m_RecentEntriesList(recentEntriesList)
{
  Initialise();
  Create(pParent, uCallbackMessage, szToolTip, icon, uID, menuID);
}

void CSystemTray::Initialise()
{
  memset(&m_tnd, 0, sizeof(m_tnd));
  m_bEnabled   = FALSE;
  m_bHidden    = FALSE;
  m_uIDTimer   = 0;
  m_hSavedIcon = NULL;
  m_DefaultMenuItemID = 0;
  m_DefaultMenuItemByPos = TRUE;
  m_pTarget = NULL; // ronys
  m_menuID = 0;
}

static const int MAX_TTT_LEN = 64; // Max tooltip text length
static void NormalizeTTT(LPCTSTR in, LPTSTR out)
{
  CString t(in), ttt;
  if (t.GetLength() > MAX_TTT_LEN) {
    ttt = t.Left(MAX_TTT_LEN/2-5) + 
      _T(" ... ") + t.Right(MAX_TTT_LEN/2);
  } else {
    ttt = t;
  }
  _tcsncpy(out, ttt, MAX_TTT_LEN);
}

BOOL CSystemTray::Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip,
                         HICON icon, UINT uID, UINT menuID)
{
  // this is only for Windows 95 (or higher)
  m_bEnabled = ( GetVersion() & 0xff ) >= 4;
  if (!m_bEnabled) return FALSE;

  // Make sure Notification window is valid (not needed - CJM)
  // VERIFY(m_bEnabled = (pParent && ::IsWindow(pParent->GetSafeHwnd())));
  // if (!m_bEnabled) return FALSE;
    
  // Make sure we avoid conflict with other messages
  ASSERT(uCallbackMessage >= WM_USER);

  // Tray only supports tooltip text up to MAX_TTT_LEN characters
  // Truncate gracefully to MAX_TTT_LEN (... in middle)
  TCHAR ttt[MAX_TTT_LEN];
  NormalizeTTT(szToolTip, ttt);

  // Create an invisible window
  CWnd::CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP, 0,0,10,10, NULL, 0);

  // load up the NOTIFYICONDATA structure
  m_tnd.cbSize = sizeof(NOTIFYICONDATA);
  m_tnd.hWnd   = pParent->GetSafeHwnd()? pParent->GetSafeHwnd() : m_hWnd;
  m_tnd.uID    = uID;
  m_tnd.hIcon  = icon;
  m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  m_tnd.uCallbackMessage = uCallbackMessage;
  _tcscpy(m_tnd.szTip, ttt);

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

BOOL CSystemTray::SetIcon(LPCTSTR lpszIconName)
{
  HICON hIcon = AfxGetApp()->LoadIcon(lpszIconName);

  return SetIcon(hIcon);
}

BOOL CSystemTray::SetIcon(UINT nIDResource)
{
  HICON hIcon = AfxGetApp()->LoadIcon(nIDResource);

  return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(LPCTSTR lpIconName)
{
  HICON hIcon = LoadIcon(NULL, lpIconName);

  return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(UINT nIDResource)
{
  HICON hIcon = LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));

  return SetIcon(hIcon);
}
 
HICON CSystemTray::GetIcon() const
{
  return (m_bEnabled)? m_tnd.hIcon : NULL;
}

BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID) 
{
  if (uFirstIconID > uLastIconID)
    return FALSE;

  const CWinApp * pApp = AfxGetApp();
  ASSERT(pApp != 0);

  m_IconList.RemoveAll();
  try {
    for (UINT i = uFirstIconID; i <= uLastIconID; i++)
      m_IconList.Add(pApp->LoadIcon(i));
  }
  catch (CMemoryException *e)
    {
      e->ReportError();
      e->Delete();
      m_IconList.RemoveAll();
      return FALSE;
    }

  return TRUE;
}

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons)
{
  m_IconList.RemoveAll();

  try {
    for (UINT i = 0; i <= nNumIcons; i++)
      m_IconList.Add(pHIconList[i]);
  }
  catch (CMemoryException *e)
    {
      e->ReportError();
      e->Delete();
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

BOOL CSystemTray::SetTooltipText(LPCTSTR pszTip)
{
  if (!m_bEnabled) return FALSE;
  // Tray only supports tooltip text up to MAX_TTT_LEN characters
  // Truncate gracefully to MAX_TTT_LEN (... in middle)
  TCHAR ttt[MAX_TTT_LEN];
  NormalizeTTT(pszTip, ttt);
  m_tnd.uFlags = NIF_TIP;
  _tcscpy(m_tnd.szTip, ttt);

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

  if (!menu.LoadMenu(m_menuID)) return FALSE;
  pSubMenu = menu.GetSubMenu(0);
  if (!pSubMenu) return FALSE;

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

void CSystemTray::OnTimer(UINT nIDEvent) 
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

LRESULT CSystemTray::OnTrayNotification(UINT wParam, LONG lParam) 
{
  //Return quickly if its not for this tray icon
  if (wParam != m_tnd.uID)
    return 0L;

  CMenu menu, *pContextMenu;
  CWnd* pTarget = m_pTarget; // AfxGetMainWnd();

  // Clicking with right button brings up a context menu
  if (LOWORD(lParam) == WM_RBUTTONUP)
    {    
      ASSERT(pTarget != NULL);
      if (!menu.LoadMenu(m_menuID)) return 0;

      // Get pointer to the real menu (must be POPUP for TrackPopupMenu)
      pContextMenu = menu.GetSubMenu(0);
      if (!pContextMenu) return 0;

      CString cEntry, group, title, user;
      CMenu *pMainRecentEntriesMenu;
      POSITION re_listpos;
      int irc;

      pMainRecentEntriesMenu = pContextMenu->GetSubMenu(2);

      CMenu *pNewRecentEntryMenu[ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1];

      int num_recent_entries = m_RecentEntriesList.GetCount();

      if (num_recent_entries == 0) {
        // Only leave the "Clear Entries" menu item (greyed out in ON_UPDATE_COMMAND_UI function)
        pMainRecentEntriesMenu->RemoveMenu(3, MF_BYPOSITION);  // Separator
        pMainRecentEntriesMenu->RemoveMenu(2, MF_BYPOSITION);  // Help entry
        pMainRecentEntriesMenu->RemoveMenu(1, MF_BYPOSITION);  // Help entry
      } else {
        // Build extra popup menus (1 per entry in list)
        re_listpos = m_RecentEntriesList.GetHeadPosition();

        for (int i = 0; i < num_recent_entries; i++) {
          cEntry = m_RecentEntriesList.GetAt(re_listpos);
          AfxExtractSubString(group, cEntry, 1, '\xbb');
          AfxExtractSubString(title, cEntry, 2, '\xbb');
          AfxExtractSubString(user, cEntry, 3, '\xbb');

          if (group.IsEmpty())
            group = _T("*");

          if (title.IsEmpty())
            title = _T("*");

          if (user.IsEmpty())
            user = _T("*");

          cEntry = "\xbb" + group + "\xbb" + title + "\xbb" + user + "\xbb";

          pNewRecentEntryMenu[i] = new CMenu;
          pNewRecentEntryMenu[i]->CreatePopupMenu();

          pNewRecentEntryMenu[i]->InsertMenu(0, MF_BYPOSITION | MF_STRING,
                                             ID_MENUITEM_TRAYCOPYUSERNAME1 + i,
                                             _T("Copy &Username to Clipboard"));
          pNewRecentEntryMenu[i]->InsertMenu(1, MF_BYPOSITION | MF_STRING,
                                             ID_MENUITEM_TRAYCOPYPASSWORD1 + i,
                                             _T("&Copy Password to Clipboard"));
          pNewRecentEntryMenu[i]->InsertMenu(2, MF_BYPOSITION | MF_STRING,
                                             ID_MENUITEM_TRAYBROWSE1 + i,
                                             _T("&Browse to URL"));
          pNewRecentEntryMenu[i]->InsertMenu(3, MF_BYPOSITION | MF_STRING,
                                             ID_MENUITEM_TRAYDELETE1 + i,
                                             _T("&Delete Entry from History"));
          pNewRecentEntryMenu[i]->InsertMenu(4, MF_BYPOSITION | MF_STRING,
                                             ID_MENUITEM_TRAYAUTOTYPE1 + i,
                                             _T("Perform Auto&Type"));

          // Insert new popup menu at the bottom of the list
          // pos 0  = Clear Entries
          // pos 1  = Note on entry format
          // pos 2  = Note on missing fields in entry
          // pos 3  = Separator
          // pos 4+ = entries.....
          irc = pMainRecentEntriesMenu->InsertMenu(i + 4, MF_BYPOSITION | MF_POPUP,
                                                   (UINT)pNewRecentEntryMenu[i]->m_hMenu, cEntry);
          ASSERT(irc != 0);

          m_RecentEntriesList.GetNext(re_listpos);
        }
      }

      // Make chosen menu item the default (bold font)
      ::SetMenuDefaultItem(pContextMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

      // Display and track the popup menu
      CPoint pos;
      GetCursorPos(&pos);

      pTarget->SetForegroundWindow();
      ::TrackPopupMenu(pContextMenu->m_hMenu, TPM_LEFTBUTTON, pos.x, pos.y, 0,
                       pTarget->GetSafeHwnd(), NULL);

      // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
      pTarget->PostMessage(WM_NULL, 0, 0);
        
      for (int i = 0; i < num_recent_entries; i++)
        delete pNewRecentEntryMenu[i];

      menu.DestroyMenu();
    } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
      ASSERT(pTarget != NULL);
      // double click received, the default action is to execute default menu item
      pTarget->SetForegroundWindow();  

      UINT uItem;
      if (m_DefaultMenuItemByPos) {
        if (!menu.LoadMenu(m_menuID)) return 0;
        pContextMenu = menu.GetSubMenu(0);
        if (!pContextMenu) return 0;
        uItem = pContextMenu->GetMenuItemID(m_DefaultMenuItemID);
      } else
        uItem = m_DefaultMenuItemID;
        
      pTarget->SendMessage(WM_COMMAND, uItem, 0);

      menu.DestroyMenu();
    }

  return 1;
}

LRESULT CSystemTray::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
  if(message==m_tnd.uCallbackMessage)
    return OnTrayNotification(wParam, lParam);
	
        
  return CWnd::WindowProc(message, wParam, lParam);
}

// This is called whenever the taskbar is created (eg after explorer crashes
// and restarts. Please note that the WM_TASKBARCREATED message is only passed
// to TOP LEVEL windows (like WM_QUERYNEWPALETTE)
LRESULT CSystemTray::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
  if(m_bHidden ==  FALSE) {
    m_bHidden = TRUE;
    ShowIcon();
  }
  return 0;
}
