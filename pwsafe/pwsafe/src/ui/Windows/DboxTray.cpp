/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "DboxMain.h"
#include "RUEList.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include <errno.h>
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#include "corelib/pwsprefs.h"
#include "corelib/pwscore.h"
#include "corelib/PWSAuxParse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////// New System Tray Commands /////////////////////
#ifndef POCKET_PC
void DboxMain::OnTrayLockUnLock()
{
  switch(app.GetSystemTrayState()) {
    case ThisMfcApp::LOCKED:            // User clicked UnLock!
      // This only unlocks the database - it does not restore the window
      TRACE(L"OnTrayLockUnLock: User clicked Unlock\n");
      RestoreWindowsData(false, false);
      break;
    case ThisMfcApp::UNLOCKED:          // User clicked Lock!
      UpdateSystemTray(LOCKED);
      ClearClipboardData();
      if (LockDataBase())  // save db if needed, clear data
        ShowWindow(SW_HIDE); // XXX apply to children as well!!!
      break;
    case ThisMfcApp::CLOSED:
      break;
    default:
      ASSERT(0);
      break;
  }
}

void DboxMain::OnTrayClearRecentEntries()
{
  m_RUEList.ClearEntries();
}

void DboxMain::OnTrayCopyUsername(UINT nID)
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

  const StringX cs_username = ci.GetUser();
  SetClipboardData(cs_username);
  UpdateLastClipboardAction(CItemData::USER);
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayCopyUsername(CCmdUI *)
{
}

void DboxMain::OnTrayCopyPassword(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYPASSWORD1) && (nID <= ID_MENUITEM_TRAYCOPYPASSWORDMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYPASSWORD1, ci))
    return;

  const CItemData::EntryType entrytype = ci.GetEntryType();
  if (entrytype == CItemData::ET_ALIAS || entrytype == CItemData::ET_SHORTCUT) {
    // This is an alias/shortcut
    uuid_array_t entry_uuid, base_uuid;
    ci.GetUUID(entry_uuid);
    if (entrytype == CItemData::ET_ALIAS)
      m_core.GetAliasBaseUUID(entry_uuid, base_uuid);
    else
      m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_core.Find(base_uuid);
    if (iter != End()) {
      ci = iter->second;
    }
  }

  const StringX cs_password = ci.GetPassword();
  SetClipboardData(cs_password);
  UpdateLastClipboardAction(CItemData::PASSWORD);
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayCopyPassword(CCmdUI *)
{
}

void DboxMain::OnTrayCopyNotes(UINT nID)
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

  SetClipboardData(ci.GetNotes());
  UpdateLastClipboardAction(CItemData::NOTES);
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayCopyNotes(CCmdUI *)
{
}

void DboxMain::OnTrayBrowse(UINT nID)
{
  ASSERT(((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX)) ||
         ((nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX)));

  CItemData ci;
  const bool bDoAutotype = (nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && 
                           (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX);
  if (!bDoAutotype) {
    if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSE1, ci))
      return;
  } else {
    if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSEPLUS1, ci))
      return;
  }

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
    std::vector<size_t> vactionverboffsets;
    StringX sxAutotype = PWSAuxParse::GetAutoTypeString(ci.GetAutoType(),
                                  ci.GetGroup(), ci.GetTitle(), 
                                  ci.GetUser(), ci.GetPassword(), 
                                  ci.GetNotes(),
                                  vactionverboffsets);
    LaunchBrowser(ci.GetURL().c_str(), sxAutotype, vactionverboffsets, bDoAutotype);
    SetClipboardData(ci.GetPassword());
    UpdateLastClipboardAction(CItemData::PASSWORD);
  }
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayBrowse(CCmdUI *pCmdUI)
{
  int nID = pCmdUI->m_nID;

  ASSERT(((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX)) ||
         ((nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX)));

  CItemData ci;
  const bool bDoAutotype = (nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && 
                           (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX);
  if (!bDoAutotype) {
    if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSE1, ci))
      return;
  } else {
    if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSEPLUS1, ci))
      return;
  }

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
    MapMenuShortcutsIter iter;
  if (!bIsEmail && (nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX))
    iter = m_MapMenuShortcuts.find(ID_MENUITEM_BROWSEURL);
  else if (!bIsEmail && (nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX))
    iter = m_MapMenuShortcuts.find(ID_MENUITEM_BROWSEURLPLUS);
  else if (bIsEmail && (nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX))
    iter = m_MapMenuShortcuts.find(ID_MENUITEM_SENDEMAIL);
    ASSERT(iter != m_MapMenuShortcuts.end());
    CString cs_text = iter->second.name.c_str();
    int nPos = cs_text.Find(L"\t");
    if (nPos > 0)
      cs_text = cs_text.Left(nPos);
    pCmdUI->SetText(cs_text);
  }
}


void DboxMain::OnTrayCopyEmail(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYEMAIL1) &&
    (nID <= ID_MENUITEM_TRAYCOPYEMAILMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYEMAIL1, ci))
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

  const StringX cs_email = ci.GetEmail();
  SetClipboardData(cs_email);
  UpdateLastClipboardAction(CItemData::EMAIL);
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayCopyEmail(CCmdUI *)
{
}

void DboxMain::OnTraySendEmail(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYSENDEMAIL1) && (nID <= ID_MENUITEM_TRAYSENDEMAILMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYSENDEMAIL1, ci))
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

  CString cs_command;
  if (!ci.IsEmailEmpty()) {
    cs_command = L"mailto:";
    cs_command += ci.GetEmail().c_str();
  } else {
    cs_command = ci.GetURL().c_str();
  }
  if (!cs_command.IsEmpty()) {
    std::vector<size_t> vactionverboffsets;
    LaunchBrowser(cs_command, L"", vactionverboffsets, false);
    UpdateAccessTime(&ci);
  }
}

void DboxMain::OnUpdateTraySendEmail(CCmdUI *)
{
}

void DboxMain::OnTrayDeleteEntry(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYDELETE1) && (nID <= ID_MENUITEM_TRAYDELETEMAX));

  m_RUEList.DeleteRUEntry(nID - ID_MENUITEM_TRAYDELETE1);
}

void DboxMain::OnUpdateTrayDeleteEntry(CCmdUI *)
{
}

void DboxMain::OnTrayAutoType(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYAUTOTYPE1) && (nID <= ID_MENUITEM_TRAYAUTOTYPEMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYAUTOTYPE1, ci))
    return;

  m_bInAT = true;
  AutoType(ci);
  UpdateAccessTime(&ci);
  m_bInAT = false;
}

void DboxMain::OnUpdateTrayAutoType(CCmdUI *)
{
}

void DboxMain::OnTrayCopyURL(UINT nID)
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

  StringX cs_URL = ci.GetURL();
  StringX::size_type ipos;
  ipos = cs_URL.find(L"[alt]");
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, L"");
  ipos = cs_URL.find(L"[ssh]");
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, L"");
  ipos = cs_URL.find(L"{alt}");
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, L"");

  SetClipboardData(cs_URL);
  UpdateLastClipboardAction(CItemData::URL);
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayCopyURL(CCmdUI *)
{
}

void DboxMain::OnTrayRunCommand(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYRUNCMD1) && (nID <= ID_MENUITEM_TRAYRUNCMDMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYRUNCMD1, ci))
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

  StringX cs_URL = ci.GetURL();
  StringX::size_type ipos;
  ipos = cs_URL.find(L"[alt]");
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, L"");
  ipos = cs_URL.find(L"[ssh]");
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, L"");
  ipos = cs_URL.find(L"{alt}");
  if (ipos != StringX::npos)
    cs_URL.replace(ipos, 5, L"");

  SetClipboardData(cs_URL);
  UpdateLastClipboardAction(CItemData::URL);
  UpdateAccessTime(&ci);
}

void DboxMain::OnUpdateTrayRunCommand(CCmdUI *)
{
}

#endif /*  POCKET_PC */
