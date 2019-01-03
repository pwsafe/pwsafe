/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "GeneralMsgBox.h"

#include <errno.h>
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include "core/pwsprefs.h"
#include "core/pwscore.h"
#include "core/PWSAuxParse.h"
#include "core/RUEList.h"

#include "os/logit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool GetRUEntry(CRUEList &RUEList, size_t index, CItemData &ci)
{
  bool retval = RUEList.GetPWEntry(index, ci);

  if (!retval) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANTPROCESSENTRY);
  }
  return retval;
}

/////////////////////////////// New System Tray Commands /////////////////////
void DboxMain::OnTrayLockUnLock()
{
  PWS_LOGIT;

  switch(m_TrayLockedState) {
    case LOCKED:            // User clicked UnLock!
      // This only unlocks the database - it does not restore the window
      if (RestoreWindowsData(false, false))
        TellUserAboutExpiredPasswords();
      break;
    case UNLOCKED:          // User clicked Lock!
      UpdateSystemTray(LOCKED);
      ClearClipboardData();
      if (!IsIconic())
        m_vGroupDisplayState = GetGroupDisplayState();

      if (LockDataBase())  { // save db if needed, clear data
        // Hide everything
        CPWDialog::GetDialogTracker()->HideOpenDialogs();

        // Now hide/minimize main dialog
        // NOTE: Do not call OnMinimize if minimizing as this will overwrite
        // the scroll bar positions
        if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray))
          ShowWindow(SW_HIDE);
        else
          ShowWindow(SW_MINIMIZE);
      }
      break;
    case CLOSED:
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

  CItemData ci, *pbci(nullptr);

  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYCOPYUSERNAME1, ci))
    return;

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  const StringX sx_user = ci.GetEffectiveFieldValue(CItemData::USER, pbci);
  SetClipboardData(sx_user);
  UpdateLastClipboardAction(CItemData::USER);

  // Username always comes from the entry irrespective of type
  UpdateAccessTime(ci.GetUUID());
}

void DboxMain::OnUpdateTrayCopyUsername(CCmdUI *)
{
}

void DboxMain::OnTrayCopyPassword(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYPASSWORD1) && (nID <= ID_MENUITEM_TRAYCOPYPASSWORDMAX));

  CItemData ci, *pbci(nullptr);
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYCOPYPASSWORD1, ci))
    return;

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  const StringX sx_password = ci.GetEffectiveFieldValue(CItemData::PASSWORD, pbci);
  SetClipboardData(sx_password);
  UpdateLastClipboardAction(CItemData::PASSWORD);

  // Password always comes from a normal or alias/shortcut base entry
  UpdateAccessTime(pbci == nullptr ? ci.GetUUID() : pbci->GetUUID());
}

void DboxMain::OnUpdateTrayCopyPassword(CCmdUI *)
{
}

void DboxMain::OnTrayCopyNotes(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYNOTES1) && (nID <= ID_MENUITEM_TRAYCOPYNOTESMAX));

  CItemData ci, *pbci(nullptr);
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYCOPYNOTES1, ci))
    return;

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  const StringX sx_notes = ci.GetEffectiveFieldValue(CItemData::NOTES, pbci);
  SetClipboardData(sx_notes);
  UpdateLastClipboardAction(CItemData::NOTES);

  // Notes for a shortcut comes from base, otherwise from the entry
  UpdateAccessTime(ci.IsShortcut() ? pbci->GetUUID() : ci.GetUUID());
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
    if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYBROWSE1, ci))
      return;
  } else {
    if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYBROWSEPLUS1, ci))
      return;
  }

  const CItemData *pbci = ci.IsDependent() ? m_core.GetBaseEntry(&ci) : nullptr;
  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd;

  if (!PWSAuxParse::GetEffectiveValues(&ci, pbci, sx_group, sx_title, sx_user,
                                       sx_pswd, sx_lastpswd,
                                       sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd))
    return;

  if (!sx_url.empty()) {
    std::vector<size_t> vactionverboffsets;
    StringX sxAutotype = PWSAuxParse::GetAutoTypeString(sx_autotype,
                                  sx_group, sx_title, sx_user,
                                  sx_pswd, sx_lastpswd,
                                  sx_notes, sx_url, sx_email,
                                  vactionverboffsets);

    LaunchBrowser(sx_url.c_str(), sxAutotype, vactionverboffsets, bDoAutotype);

    if (PWSprefs::GetInstance()->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL)) {
      SetClipboardData(sx_pswd);
      UpdateLastClipboardAction(CItemData::PASSWORD);
    }
  }

  // URL for a shortcut comes from base, otherwise from the entry
  UpdateAccessTime(ci.IsShortcut() ? pbci->GetUUID() : ci.GetUUID());
}

void DboxMain::OnUpdateTrayBrowse(CCmdUI *pCmdUI)
{
  int nID = pCmdUI->m_nID;

  ASSERT(((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX)) ||
         ((nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX)));

  CItemData ci, *pbci(nullptr);
  const bool bDoAutotype = (nID >= ID_MENUITEM_TRAYBROWSEPLUS1) && 
                           (nID <= ID_MENUITEM_TRAYBROWSEPLUSMAX);
  if (!bDoAutotype) {
    if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYBROWSE1, ci))
      return;
  } else {
    if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYBROWSEPLUS1, ci))
      return;
  }

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  // Has it an embedded URL
  if (ci.IsFieldValueEmpty(CItemData::URL, pbci)) {
    pCmdUI->Enable(FALSE);
  } else {
    const bool bIsEmail = ci.IsURLEmail(pbci);
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

  CItemData ci, *pbci(nullptr);
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYCOPYEMAIL1, ci))
    return;

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  const StringX sx_email = ci.GetEffectiveFieldValue(CItemData::EMAIL, pbci);
  SetClipboardData(sx_email);
  UpdateLastClipboardAction(CItemData::EMAIL);

  // Email for a shortcut comes from base, otherwise from the entry
  UpdateAccessTime(ci.IsShortcut() ? pbci->GetUUID() : ci.GetUUID());
}

void DboxMain::OnUpdateTrayCopyEmail(CCmdUI *)
{
}

void DboxMain::OnTraySendEmail(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYSENDEMAIL1) && (nID <= ID_MENUITEM_TRAYSENDEMAILMAX));

  CItemData ci, *pbci(nullptr);
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYSENDEMAIL1, ci))
      return;

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  const StringX sx_email = ci.GetEffectiveFieldValue(CItemData::EMAIL, pbci);

  CString cs_command;
  if (!sx_email.empty()) {
    cs_command = L"mailto:";
    cs_command += sx_email.c_str();
  } else {
    cs_command = ci.GetEffectiveFieldValue(CItemData::URL, pbci).c_str();
  }

  if (!cs_command.IsEmpty()) {
    std::vector<size_t> vactionverboffsets;
    LaunchBrowser(cs_command, L"", vactionverboffsets, false);

    // Email/URL for a shortcut comes from base, otherwise from the entry
    UpdateAccessTime(ci.IsShortcut() ? pbci->GetUUID() : ci.GetUUID());
  }
}

void DboxMain::OnUpdateTraySendEmail(CCmdUI *)
{
}

void DboxMain::OnTraySelect(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYSELECT1) && (nID <= ID_MENUITEM_TRAYSELECTMAX));

  CItemData ci;
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYSELECT1, ci))
      return;

  DisplayInfo *pdi = GetEntryGUIInfo(ci, true);

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
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYAUTOTYPE1, ci))
    return;

  m_bInAT = true;
  AutoType(ci);

  // Unclear what AutoType is going to reference from entry, if a shortcut,
  // and what from the base so just update the entry access time
  UpdateAccessTime(ci.GetUUID());
  m_bInAT = false;
}

void DboxMain::OnUpdateTrayAutoType(CCmdUI *)
{
}

void DboxMain::OnTrayCopyURL(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYURL1) && (nID <= ID_MENUITEM_TRAYCOPYURLMAX));

  CItemData ci, *pbci(nullptr);
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYCOPYURL1, ci))
    return;

  if (ci.IsDependent())
    pbci = m_core.GetBaseEntry(&ci);

  StringX sx_URL = ci.GetEffectiveFieldValue(CItemData::URL, pbci);

  if (!sx_URL.empty()) {
    StringX::size_type ipos;
    ipos = sx_URL.find(L"[alt]");
    if (ipos != StringX::npos)
      sx_URL.replace(ipos, 5, L"");
    ipos = sx_URL.find(L"[ssh]");
    if (ipos != StringX::npos)
      sx_URL.replace(ipos, 5, L"");
    ipos = sx_URL.find(L"{alt}");
    if (ipos != StringX::npos)
      sx_URL.replace(ipos, 5, L"");
  }

  SetClipboardData(sx_URL);
  UpdateLastClipboardAction(CItemData::URL);

  // URL for a shortcut comes from base, otherwise from the entry
  UpdateAccessTime(ci.IsShortcut() ? pbci->GetUUID() : ci.GetUUID());
}

void DboxMain::OnUpdateTrayCopyURL(CCmdUI *)
{
}

void DboxMain::OnTrayRunCommand(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYRUNCMD1) && (nID <= ID_MENUITEM_TRAYRUNCMDMAX));

  CItemData ci;
  if (!GetRUEntry(m_RUEList, nID - ID_MENUITEM_TRAYRUNCMD1, ci))
    return;

  const CItemData *pbci = ci.IsDependent() ? m_core.GetBaseEntry(&ci) : nullptr;
  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd;

  if (!PWSAuxParse::GetEffectiveValues(&ci, pbci, sx_group, sx_title, sx_user,
                                       sx_pswd, sx_lastpswd,
                                       sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd))
    return;

  StringX sx_Expanded_ES;
  if (sx_runcmd.empty())
    return;

  std::wstring errmsg;
  StringX::size_type st_column;
  bool bURLSpecial;
  sx_Expanded_ES = PWSAuxParse::GetExpandedString(sx_runcmd,
                                                  m_core.GetCurFile(), &ci, pbci,
                                                  m_bDoAutoType, m_sxAutoType,
                                                  errmsg, st_column, bURLSpecial);

  if (!errmsg.empty()) {
    CGeneralMsgBox gmb;
    CString cs_title, cs_errmsg;
    cs_title.LoadString(IDS_RUNCOMMAND_ERROR);
    cs_errmsg.Format(IDS_RUN_ERRORMSG, (int)st_column, errmsg.c_str());
    gmb.MessageBox(cs_errmsg, cs_title, MB_OK | MB_ICONQUESTION);
    return;
  }

  // if no autotype value in run command's $a(value), start with item's (bug #1078)
  if (m_sxAutoType.empty())
    m_sxAutoType = ci.GetAutoType();

  m_sxAutoType = PWSAuxParse::GetAutoTypeString(m_sxAutoType,
                                                sx_group, sx_title, sx_user,
                                                sx_pswd, sx_lastpswd,
                                                sx_notes, sx_url, sx_email,
                                                m_vactionverboffsets);
  SetClipboardData(sx_pswd);
  UpdateLastClipboardAction(CItemData::PASSWORD);

  // Password always comes from a normal or base entry
  UpdateAccessTime(pbci == nullptr ? ci.GetUUID() : pbci->GetUUID());

  // Now honour presence of [alt], {alt} or [ssh] in the url if present
  // in the RunCommand field.  Note: they are all treated the same (unlike
  // in 'Browse to'.
  StringX sxAltBrowser(PWSprefs::GetInstance()->
    GetPref(PWSprefs::AltBrowser));

  if (bURLSpecial && !sxAltBrowser.empty()) {
    StringX sxCmdLineParms(PWSprefs::GetInstance()->
      GetPref(PWSprefs::AltBrowserCmdLineParms));

    if (sxAltBrowser[0] != L'\'' && sxAltBrowser[0] != L'"')
      sxAltBrowser = L"\"" + sxAltBrowser + L"\"";
    if (!sxCmdLineParms.empty())
      sx_Expanded_ES = sxAltBrowser + StringX(L" ") +
      sxCmdLineParms + StringX(L" ") + sx_Expanded_ES;
    else
      sx_Expanded_ES = sxAltBrowser + StringX(L" ") + sx_Expanded_ES;
  }

  bool rc = m_runner.runcmd(sx_Expanded_ES, !m_sxAutoType.empty());
  if (!rc) {
    m_bDoAutoType = false;
    m_sxAutoType = L"";
    return;
  }
}

void DboxMain::OnUpdateTrayRunCommand(CCmdUI *)
{
}

void DboxMain::OnGotoDependant(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_GOTODEPENDANT1) && (nID <= ID_MENUITEM_GOTODEPENDANTMAX));

  if ((nID - ID_MENUITEM_GOTODEPENDANT1 + 1) > m_vGotoDependants.size())
    return;

  SelectEntry(m_vGotoDependants[nID - ID_MENUITEM_GOTODEPENDANT1], TRUE);
}

void DboxMain::OnUpdateGotoDependant(CCmdUI *)
{
}
