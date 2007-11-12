/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/// file DboxTray.cpp
//
// Tray-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include <errno.h>
  #include "resource.h"
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif

#include "DboxMain.h"
#include "RUEList.h"
#include "corelib/pwsprefs.h"
#include "corelib/pwscore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////// New System Tray Commands /////////////////////
#ifndef POCKET_PC
void
DboxMain::OnTrayLockUnLock()
{
  switch(app.GetSystemTrayState()) {
  case ThisMfcApp::LOCKED:					// User clicked UnLock
    UnMinimize(true);
    break;
  case ThisMfcApp::UNLOCKED:					// User clicked Lock
    UpdateSystemTray(LOCKED);
    ClearClipboardData();
    ShowWindow(SW_HIDE);
    m_IdleLockCountDown = 1; // lock the same way as a timer lock
    OnTimer(TIMER_USERLOCK); // save db if needed, etc.
    break;
  case ThisMfcApp::CLOSED:
  default:
    ASSERT(0);
    break;
	}
}

void
DboxMain::OnUpdateTrayMinimizeCommand(CCmdUI* pCmdUI)
{
  // If already minimized, don't enable Minimize menu item
  WINDOWPLACEMENT wndpl;
  GetWindowPlacement(&wndpl);
  pCmdUI->Enable(wndpl.showCmd == SW_SHOWMINIMIZED ? FALSE : TRUE);
}

void
DboxMain::OnUpdateTrayUnMinimizeCommand(CCmdUI* pCmdUI)
{
  // If not minimized, don't enable Restore menu item
  WINDOWPLACEMENT wndpl;
  GetWindowPlacement(&wndpl);
  pCmdUI->Enable(wndpl.showCmd != SW_SHOWMINIMIZED ? FALSE : TRUE);
}

void
DboxMain::OnUpdateTrayLockUnLockCommand(CCmdUI *pCmdUI)
{
	const CString csUnLock(MAKEINTRESOURCE(IDS_UNLOCKSAFE));
	const CString csLock(MAKEINTRESOURCE(IDS_LOCKSAFE));
	const CString csClosed(MAKEINTRESOURCE(IDS_NOSAFE));

	const int i_state = app.GetSystemTrayState();
	// Set text to "UnLock" or "Lock"
	switch (i_state) {
		case ThisMfcApp::UNLOCKED:
			pCmdUI->SetText(csLock);
			break;
		case ThisMfcApp::LOCKED:
			pCmdUI->SetText(csUnLock);
			break;
		case ThisMfcApp::CLOSED:
			pCmdUI->Enable(FALSE);
			pCmdUI->SetText(csClosed);
			break;
		default:
            ASSERT(0);
			break;
	}

	if (i_state != ThisMfcApp::CLOSED) {
		// If dialog visible - obviously unlocked and no need to have option to lock
		if (this->IsWindowVisible() == FALSE)
			pCmdUI->Enable(TRUE);
		else
			pCmdUI->Enable(FALSE);
	}
}

void
DboxMain::OnTrayClearRecentEntries()
{
	m_RUEList.ClearEntries();
}

void
DboxMain::OnUpdateTrayClearRecentEntries(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pSubMenu != NULL) {
        // disable or enable entire popup for "Recent Entries"
        // CCmdUI::Enable is a no-op for this case, so we
        //   must do what it would have done.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION |
			(app.GetSystemTrayState() == ThisMfcApp::UNLOCKED ? MF_ENABLED : MF_GRAYED));
        return;
    }
    // otherwise enable
    pCmdUI->Enable((m_RUEList.GetCount() != 0) ? TRUE : FALSE);
}

void
DboxMain::OnTrayCopyUsername(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYUSERNAME1) &&
         (nID <= ID_MENUITEM_TRAYCOPYUSERNAMEMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYUSERNAME1, ci))
	  return;

  const CMyString username = ci.GetUser();
  if (!username.IsEmpty()) {
    SetClipboardData(username);
    UpdateAccessTime(&ci);
  }
}

void
DboxMain::OnUpdateTrayCopyUsername(CCmdUI *)
{
}

void
DboxMain::OnTrayCopyPassword(UINT nID)
{
	ASSERT((nID >= ID_MENUITEM_TRAYCOPYPASSWORD1) && (nID <= ID_MENUITEM_TRAYCOPYPASSWORDMAX));

	CItemData ci;
	if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYPASSWORD1, ci))
		return;

	const CMyString curPassString = ci.GetPassword();
	SetClipboardData(curPassString);
  UpdateAccessTime(&ci);
}

void
DboxMain::OnUpdateTrayCopyPassword(CCmdUI *)
{
}

void
DboxMain::OnTrayCopyNotes(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYNOTES1) && (nID <= ID_MENUITEM_TRAYCOPYNOTESMAX));

  CItemData ci;
  CString cs_text;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYNOTES1, ci))
	  return;

  const CMyString notes = ci.GetNotes();
  const CMyString url = ci.GetURL();
  const CMyString autotype = ci.GetAutoType();
  CMyString clipboard_data;

  clipboard_data = notes;
  if (!url.IsEmpty()) {
  	cs_text.LoadString(IDS_COPYURL);
    clipboard_data += CMyString(cs_text);
    clipboard_data += url;
  }
  if (!autotype.IsEmpty()) {
  	cs_text.LoadString(IDS_COPYAUTOTYPE);
    clipboard_data += CMyString(cs_text);
    clipboard_data += autotype;
  }

  if (!clipboard_data.IsEmpty()) {
    SetClipboardData(clipboard_data);
    UpdateAccessTime(&ci);
  }
}

void
DboxMain::OnUpdateTrayCopyNotes(CCmdUI *)
{
}

void
DboxMain::OnTrayBrowse(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSE1, ci))
	  return;

  if (!ci.IsURLEmpty()) {
    LaunchBrowser(ci.GetURL());
  }
  UpdateAccessTime(&ci);
}


void
DboxMain::OnUpdateTrayBrowse(CCmdUI *pCmdUI)
{
  int nID = pCmdUI->m_nID;

  ASSERT((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSE1, ci))
	  return;

  // Has it an embedded URL
  if (ci.IsURLEmpty()) {
    pCmdUI->Enable(FALSE);
  } else {
    const bool bIsEmail = ci.GetURL().Left(7) == _T("[email]");
    CString cs_text = bIsEmail ? CS_SENDEMAIL : CS_BROWSEURL;
    int nPos = cs_text.Find(_T("\t"));
    if (nPos > 0)
      cs_text = cs_text.Left(nPos);
    pCmdUI->SetText(cs_text);
  }
}

void
DboxMain::OnTrayDeleteEntry(UINT nID)
{
	ASSERT((nID >= ID_MENUITEM_TRAYDELETE1) && (nID <= ID_MENUITEM_TRAYDELETEMAX));

	m_RUEList.DeleteRUEntry(nID - ID_MENUITEM_TRAYDELETE1);
}

void
DboxMain::OnUpdateTrayDeleteEntry(CCmdUI *)
{
}

void
DboxMain::OnTrayAutoType(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYAUTOTYPE1) && (nID <= ID_MENUITEM_TRAYAUTOTYPEMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYAUTOTYPE1, ci))
	  return;

  AutoType(ci);
  UpdateAccessTime(&ci);
}

void
DboxMain::OnUpdateTrayAutoType(CCmdUI *)
{
}

#endif /*  POCKET_PC */
