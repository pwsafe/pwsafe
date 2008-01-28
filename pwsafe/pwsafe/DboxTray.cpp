/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
  case ThisMfcApp::LOCKED:            // User clicked UnLock
    UnMinimize(true);
    break;
  case ThisMfcApp::UNLOCKED:          // User clicked Lock
    UpdateSystemTray(LOCKED);
    ClearClipboardData();
    ShowWindow(SW_HIDE);
    m_IdleLockCountDown = 1;          // lock the same way as a timer lock
    OnTimer(TIMER_USERLOCK);          // save db if needed, etc.
    break;
  case ThisMfcApp::CLOSED:
  default:
    ASSERT(0);
    break;
  }
}

void
DboxMain::OnTrayClearRecentEntries()
{
  m_RUEList.ClearEntries();
}

void
DboxMain::OnTrayCopyUsername(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYUSERNAME1) &&
    (nID <= ID_MENUITEM_TRAYCOPYUSERNAMEMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYUSERNAME1, ci))
    return;

  if (ci.IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

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

  const CItemData::EntryType entrytype = ci.GetEntryType();
  if (entrytype == CItemData::Alias || entrytype == CItemData::Shortcut) {
    // This is an alias/shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    if (entrytype == CItemData::Alias)
      m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
    else
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

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
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYNOTES1, ci))
    return;

  if (ci.IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

  CString cs_text;
  const CMyString notes = ci.GetNotes();
  const CMyString url = ci.GetURL();
  const CMyString autotype = ci.GetAutoType();
  CMyString clipboard_data;

  clipboard_data = notes;
  if (!url.IsEmpty()) {
    if (ci.IsURLEmail())
      cs_text.LoadString(IDS_COPYURL);
    else
      cs_text.LoadString(IDS_COPYEMAIL);
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

  if (ci.IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

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

  if (ci.IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

  // Has it an embedded URL
  if (ci.IsURLEmpty()) {
    pCmdUI->Enable(FALSE);
  } else {
    const bool bIsEmail = ci.IsURLEmail();
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

void
DboxMain::OnTrayCopyURL(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYURL1) && (nID <= ID_MENUITEM_TRAYCOPYURLMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYURL1, ci))
    return;

  if (ci.IsShortcut()) {
    // This is an shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

  const CMyString cs_URL = ci.GetURL();

  if (!cs_URL.IsEmpty())
    SetClipboardData(cs_URL);
  else
    ClearClipboardData();
  UpdateAccessTime(&ci);
}

void
DboxMain::OnUpdateTrayCopyURL(CCmdUI *)
{
}

#endif /*  POCKET_PC */
