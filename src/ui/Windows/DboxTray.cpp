/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "core/pwsprefs.h"
#include "core/pwscore.h"
#include "core/PWSAuxParse.h"

#include "os/logit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool SafeGetBaseEntry(const DboxMain *pDbx, const CItemData &dep, CItemData &base)
{
  // Asserts in debug build if GetBaseEntry(dependent) fails
  // returns false in release build
  const CItemData *pBase = pDbx->GetBaseEntry(&dep);
  ASSERT(pBase != NULL);
  if (pBase != NULL) {
    base = *pBase;
    return true;
  } else
    return false;
}

/////////////////////////////// New System Tray Commands /////////////////////
#ifndef POCKET_PC
void DboxMain::OnTrayLockUnLock()
{
  PWS_LOGIT;

  switch(app.GetSystemTrayState()) {
    case ThisMfcApp::LOCKED:            // User clicked UnLock!
      // This only unlocks the database - it does not restore the window
      pws_os::Trace(L"OnTrayLockUnLock: User clicked Unlock\n");
      RestoreWindowsData(false, false);
      TellUserAboutExpiredPasswords();
      break;
    case ThisMfcApp::UNLOCKED:          // User clicked Lock!
      UpdateSystemTray(LOCKED);
      ClearClipboardData();
      if (!IsIconic())
        m_vGroupDisplayState = GetGroupDisplayState();
      if (LockDataBase())  { // save db if needed, clear data
        ShowWindow(SW_HIDE);
      }
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
    if (!SafeGetBaseEntry(this, ci, ci))
      return; // fail safely in release
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

  if (ci.IsDependent()) {
    if (!SafeGetBaseEntry(this, ci, ci))
      return; // fail safely in release
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

  if (ci.IsShortcut())
    if (!SafeGetBaseEntry(this, ci, ci))
      return;

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
    if (!SafeGetBaseEntry(this, ci, ci))
      return;
  }

  if (!ci.IsURLEmpty()) {
    std::vector<size_t> vactionverboffsets;
    StringX sxAutotype = PWSAuxParse::GetAutoTypeString(ci.GetAutoType(),
                                  ci.GetGroup(), ci.GetTitle(), 
                                  ci.GetUser(), ci.GetPassword(), 
                                  ci.GetNotes(),
                                  vactionverboffsets);

    LaunchBrowser(ci.GetURL().c_str(), sxAutotype, vactionverboffsets, bDoAutotype);

    if (PWSprefs::GetInstance()->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL)) {
      SetClipboardData(ci.GetPassword());
      UpdateLastClipboardAction(CItemData::PASSWORD);
    } else
      UpdateLastClipboardAction(CItemData::URL);
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
    if (!SafeGetBaseEntry(this, ci, ci))
      return;
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
    if (!SafeGetBaseEntry(this, ci, ci))
      return; // fail safely in release
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
    if (!SafeGetBaseEntry(this, ci, ci))
      return; // fail safely in release
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

void DboxMain::OnTraySelect(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYSELECT1) && (nID <= ID_MENUITEM_TRAYSELECTMAX));

  CItemData ci;
  if (!m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYSELECT1, ci))
      return;

  DisplayInfo *pdi = (DisplayInfo *)ci.GetDisplayInfo();
  if (pdi != NULL) {
    // Could be null if RefreshViews not called,
    // In which case we've no display to select to.
    // An alternate solution would be to force the main window
    // to display, along with a call to RefreshViews(), before
    // calling GetDisplayInfo().
    SelectEntry(pdi->list_index,TRUE);
  }
}

void DboxMain::OnUpdateTraySelect(CCmdUI *)
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
    if (!SafeGetBaseEntry(this, ci, ci))
      return; // fail safely in release
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
    if (!SafeGetBaseEntry(this, ci, ci))
      return; // fail safely in release
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
