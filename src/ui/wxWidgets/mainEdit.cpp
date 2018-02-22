/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file mainEdit.cpp
 *  This file contains implementations of PasswordSafeFrame
 *  member functions corresponding to actions under the 'Edit'
 *  menubar menu.
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/utils.h" // for wxLaunchDefaultBrowser

#include "PWSgrid.h"
#include "PWStree.h"

#include "passwordsafeframe.h"
#include "addeditpropsheet.h"
#include "pwsclip.h"
#include "PasswordSafeSearch.h"
#include "deleteconfirmation.h"
#include "editshortcut.h"
#include "createshortcutdlg.h"
#include "wxutils.h"
#include "guiinfo.h"
#include "passwordsubset.h"
#include "./pwsqrcodedlg.h"

#include "../../core/PWSAuxParse.h"
#include "../../core/Util.h"
#include "../../os/KeySend.h"
#include "../../os/run.h"
#include "../../os/sleep.h"
#include "../../os/utf8conv.h"
#include "./TimedTaskChain.h"
#include <wx/tokenzr.h>
#include <array>

#include <algorithm>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

using pws_os::CUUID;

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_EDIT
 */

void PasswordSafeFrame::OnEditClick( wxCommandEvent& /* evt */ )
{
  CItemData *item = GetSelectedEntry();
  if (item != nullptr)
    DoEdit(*item);
}

//This function intentionally takes the argument by value and not by
//reference to avoid touching an item invalidated by idle timeout
void PasswordSafeFrame::DoEdit(CItemData item)
{
  int rc = 0;
  if (!item.IsShortcut()) {
    bool read_only = m_core.IsReadOnly() || item.IsProtected();
    AddEditPropSheet editDbox(this, m_core,
                              read_only ? AddEditPropSheet::SheetType::VIEW : AddEditPropSheet::SheetType::EDIT,
                              &item, this);
    rc = editDbox.ShowModal();
  } else {
    EditShortcut editDbox(this, m_core, &item);
    rc = editDbox.ShowModal();
  }
  if (rc == wxID_OK) {
    uuid_array_t uuid;
    item.GetUUID(uuid);
    //Find the item in the database, which might have been loaded afresh
    //after lock/unlock, so the old data structures are no longer valid
    auto iter = m_core.Find(uuid);
    if ( iter != m_core.GetEntryEndIter()) {
      CItemData& origItem = m_core.GetEntry(iter);
      //The Item is updated in DB by AddEditPropSheet
      UpdateAccessTime(origItem);
      UpdateSelChanged(&origItem);
      UpdateStatusBar();
    }
    else {
      wxFAIL_MSG(wxT("Item being edited not found in currently loaded DB"));
    }
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ADD
 */

void PasswordSafeFrame::OnAddClick( wxCommandEvent& /* evt */ )
{
  wxString selectedGroup = wxEmptyString;
  wxTreeItemId selection;
  if (IsTreeView() && (selection = m_tree->GetSelection()).IsOk() && m_tree->ItemIsGroup(selection)) {
    selectedGroup = m_tree->GetItemGroup(selection);
  }

  AddEditPropSheet addDbox(this, m_core, AddEditPropSheet::SheetType::ADD, nullptr, this, selectedGroup);
  if (addDbox.ShowModal() == wxID_OK) {
    const CItemData &item = addDbox.GetItem();
    m_core.Execute(AddEntryCommand::Create(&m_core, item, item.GetBaseUUID()));
    UpdateStatusBar();
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
 */

void PasswordSafeFrame::OnDeleteClick( wxCommandEvent& /* evt */ )
{
  bool dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);

  bool dodelete = true;
  int num_children = 0;
  // If tree view, check if group selected
  if (m_tree->IsShown()) {
    wxTreeItemId sel = m_tree->GetSelection();
    num_children = static_cast<int>(m_tree->GetChildrenCount(sel));
    if (num_children > 0) // ALWAYS confirm group delete
      dontaskquestion = false;
  }

  //Confirm whether to delete the item
  if (!dontaskquestion) {
    DeleteConfirmation deleteDlg(this, num_children);
    deleteDlg.SetConfirmdelete(PWSprefs::GetInstance()->GetPref(PWSprefs::DeleteQuestion));
    int rc = deleteDlg.ShowModal();
    if (rc != wxID_YES) {
      dodelete = false;
    }
    PWSprefs::GetInstance()->SetPref(PWSprefs::DeleteQuestion, deleteDlg.GetConfirmdelete());
  }

  if (dodelete) {
    Command *doit = nullptr;
    CItemData *item = GetSelectedEntry();
    if (item != nullptr) {
      doit = Delete(item);
    } else if (m_tree->GetSelection() != m_tree->GetRootItem()) {
      doit = Delete(m_tree->GetSelection());
    }
    if (doit != nullptr)
      m_core.Execute(doit);
  } // dodelete
}

Command *PasswordSafeFrame::Delete(CItemData *pci)
{
  ASSERT(pci != nullptr);
  // ConfirmDelete asks for user confirmation
  // when deleting a shortcut or alias base.
  // Otherwise it just returns true
  if (m_core.ConfirmDelete(pci))
    return DeleteEntryCommand::Create(&m_core, *pci);
  else
    return nullptr;
}

Command *PasswordSafeFrame::Delete(wxTreeItemId tid)
{
  // Called for deleting a group
  // Recursively build the appropriate multi-command

  if (!tid) return nullptr;
  MultiCommands *retval = MultiCommands::Create(&m_core);
  if (m_tree->GetChildrenCount(tid) > 0) {
    wxTreeItemIdValue cookie;
    wxTreeItemId ti = m_tree->GetFirstChild(tid, cookie);

    while (ti.IsOk()) {
      Command *delCmd = Delete(ti);
      if (delCmd != nullptr)
        retval->Add(delCmd);
      ti = m_tree->GetNextChild(tid, cookie);
    } // while children
    // Explicitly delete any empty groups coinciding with this wxTreeItem's group hierarchy
    // Otherwise the user will see a these empty groups still hanging around in spite
    // of just deleting the parent/ancestor
    StringX sxGroup = tostringx(m_tree->GetItemGroup(tid));
    if (m_core.IsEmptyGroup(sxGroup)) {
      Command *delGrp = DBEmptyGroupsCommand::Create(&m_core, sxGroup, DBEmptyGroupsCommand::EG_DELETE);
      if (delGrp)
        retval->Add(delGrp);
    }
  } else { // leaf
    CItemData *leaf = m_tree->GetItem(tid);
    if (leaf != nullptr) {
      Command *delLeafCmd = Delete(leaf); // gets user conf. if needed
      if (delLeafCmd != nullptr)
        retval->Add(delLeafCmd);
    }
    else {
      wxASSERT_MSG(m_tree->ItemIsGroup(tid), wxT("Childless item without CItemData must be an empty group"));
      StringX sxGroup = tostringx(m_tree->GetItemGroup(tid));
      Command *delGrp = DBEmptyGroupsCommand::Create(&m_core, sxGroup, DBEmptyGroupsCommand::EG_DELETE);
      if (delGrp)
        retval->Add(delGrp);
    }
  }

  // If MultiCommands is empty, delete and return nullptr
  if (retval->GetSize() == 0) {
    delete retval;
    retval = nullptr;
  }
  return retval;
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_FIND
 */

void PasswordSafeFrame::OnFindClick( wxCommandEvent& /* evt */ )
{
  m_search->Activate();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EDITMENU_FIND_NEXT
 */

void PasswordSafeFrame::OnFindNext( wxCommandEvent& /* evt */ )
{
  m_search->FindNext();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EDITMENU_FIND_PREVIOUS
 */

void PasswordSafeFrame::OnFindPrevious( wxCommandEvent& /* evt */ )
{
  m_search->FindPrevious();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CLEARCLIPBOARD
 */

void PasswordSafeFrame::OnClearclipboardClick( wxCommandEvent& /* evt */ )
{
  PWSclipboard::GetInstance()->ClearCBData();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYPASSWORD
 */

void PasswordSafeFrame::OnCopypasswordClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyPassword(*item);
}

void PasswordSafeFrame::DoCopyPassword(CItemData &item)
{
  if (!item.IsDependent())
    PWSclipboard::GetInstance()->SetData(item.GetPassword());
  else {
    const CUUID &base = item.GetBaseUUID();
    const StringX &passwd = m_core.GetEntry(m_core.Find(base)).GetPassword();
    PWSclipboard::GetInstance()->SetData(passwd);
  }
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYRUNCOMMAND
 */

void PasswordSafeFrame::OnCopyRunCmd(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyRunCmd(*item);
}

void PasswordSafeFrame::DoCopyRunCmd(CItemData &item)
{
  PWSclipboard::GetInstance()->SetData(item.GetRunCommand());
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYUSERNAME
 */

void PasswordSafeFrame::OnCopyusernameClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyUsername(*item);
}

void PasswordSafeFrame::DoCopyUsername(CItemData &item)
{
  PWSclipboard::GetInstance()->SetData(item.GetUser());
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYNOTESFLD
 */

void PasswordSafeFrame::OnCopynotesfldClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyNotes(*item);
}

void PasswordSafeFrame::DoCopyNotes(CItemData &item)
{
  PWSclipboard::GetInstance()->SetData(item.GetNotes());
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYURL
 */

void PasswordSafeFrame::OnCopyurlClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyURL(*item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYEMAIL
 */

void PasswordSafeFrame::OnCopyEmailClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyEmail(*item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CREATESHORTCUT
 */

void PasswordSafeFrame::OnCreateShortcut(wxCommandEvent& WXUNUSED(event))
{
  CItemData* item = GetSelectedEntry();
  if (item && !item->IsDependent()) {
    CreateShortcutDlg dlg(this, m_core, item);
    int rc = dlg.ShowModal();
    if (rc == wxID_OK) {
      UpdateAccessTime(*item);
      Show(true);
      UpdateStatusBar();
    }
  }
}

// Duplicate selected entry but make title unique
void PasswordSafeFrame::OnDuplicateEntry(wxCommandEvent& WXUNUSED(event))
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;
//  if (SelItemOk() == TRUE) {
    CItemData *pci = GetSelectedEntry();
    ASSERT(pci != nullptr);
//    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
//    ASSERT(pdi != nullptr);

    // Get information from current selected entry
    const StringX ci2_group = pci->GetGroup();
    const StringX ci2_user = pci->GetUser();
    const StringX ci2_title0 = pci->GetTitle();
    StringX ci2_title;

    // Find a unique "Title"
    ItemListConstIter listpos;
    int i = 0;
    wxString s_copy;
    do {
      s_copy.clear();
      i++;
      s_copy << _(" Copy # ") << i;
      ci2_title = ci2_title0 + tostringx(s_copy);
      listpos = m_core.Find(ci2_group, ci2_title, ci2_user);
    } while (listpos != m_core.GetEntryEndIter());

    // Set up new entry
    CItemData ci2(*pci);
    ci2.CreateUUID();
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetStatus(CItemData::ES_ADDED);

    Command *pcmd = nullptr;
    if (pci->IsDependent()) {
      if (pci->IsAlias()) {
        ci2.SetAlias();
      } else {
        ci2.SetShortcut();
      }

      const CItemData *pbci = m_core.GetBaseEntry(pci);
      if (pbci != nullptr) {
        StringX cs_tmp;
        cs_tmp = wxT("[") +
          pbci->GetGroup() + wxT(":") +
          pbci->GetTitle() + wxT(":") +
          pbci->GetUser()  + wxT("]");
        ci2.SetPassword(cs_tmp);
        pcmd = AddEntryCommand::Create(&m_core, ci2, pbci->GetUUID());
      }
    } else { // not alias or shortcut
      ci2.SetNormal();
      pcmd = AddEntryCommand::Create(&m_core, ci2);
    }

    Execute(pcmd);

//    pdi->list_index = -1; // so that InsertItemIntoGUITreeList will set new values

    CUUID uuid = ci2.GetUUID();
    auto iter = m_core.Find(uuid);
    ASSERT(iter != m_core.GetEntryEndIter());
    wxUnusedVar(iter); // used in assert only
//    InsertItemIntoGUITreeList(m_core.GetEntry(iter));
//    FixListIndexes();
    UpdateStatusBar();

//    int rc = SelectEntry(pdi->list_index);
//    if (rc == 0) {
//      SelectEntry(m_ctlItemList.GetItemCount() - 1);
//    }
//    ChangeOkUpdate();
    m_RUEList.AddRUEntry(uuid);
//  }
}

void PasswordSafeFrame::DoCopyURL(CItemData &item)
{
  PWSclipboard::GetInstance()->SetData(item.GetURL());
  UpdateAccessTime(item);
}

void PasswordSafeFrame::DoCopyEmail(CItemData &item)
{
  const StringX mailto = item.IsEmailEmpty()?
                      (item.IsURLEmail()? item.GetURL(): StringX())
                      : item.GetEmail();

  if (!mailto.empty()) {
    PWSclipboard::GetInstance()->SetData(mailto);
    UpdateAccessTime(item);
  }
}

/*
 * The entire logic is borrowed from ui/Windows/MainEdit.cpp
 */
void PasswordSafeFrame::DoAutotype(CItemData &ci)
{
  std::vector<size_t> vactionverboffsets;
  const StringX sxautotype = PWSAuxParse::GetAutoTypeString(ci, m_core,
                                                            vactionverboffsets);
  // even though we only need them in one of the *later* tasks, its safer to
  // get sxautotype and vactionverboffsets set up before the reference to
  // CItemData becomes invalid in some way

  UpdateAccessTime(ci);

  const wxString intervalStr = towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::AutotypeTaskDelays));
  const wxArrayString tokens = wxStringTokenize(intervalStr, wxT(" ,:;\t"), wxTOKEN_STRTOK);
  const int stdInterval = PWSprefs::GetInstance()->GetPref(PWSprefs::TimedTaskChainDelay);

  std::array<int, 3> intervals;
  for (unsigned idx = 0; idx < intervals.size(); idx++)
      intervals[idx] = (tokens.GetCount() > idx && tokens[idx].IsNumber())? wxAtoi(tokens[idx]) : stdInterval;

  TimedTaskChain::CreateTaskChain(
                    {{std::bind(&PasswordSafeFrame::MinimizeOrHideBeforeAutotyping, this), intervals[0]}}
                  )
                  .then( [this, sxautotype, vactionverboffsets]() {
                     // those lambda args should not be captured by reference
                     // since they are on the stack
                     DoAutotype(sxautotype, vactionverboffsets);
                  }, intervals[1])
                  .then( [this]() {
                     MaybeRestoreUI(false, wxEmptyString);
                  }, intervals[2])
                  .OnError( [this](const std::exception& e) {
                     MaybeRestoreUI(true, e.what());
                  });

}

void PasswordSafeFrame::MinimizeOrHideBeforeAutotyping()
{
  // Rules are ("Minimize on Autotype" takes precedence):
  // 1. If "MinimizeOnAutotype" - minimize PWS during Autotype but do
  //    not restore it (previous default action - but a pain if locked
  //    in the system tray!)
  // 2. If "Always on Top" - hide PWS during Autotype and then make it
  //    "AlwaysOnTop" again, unless minimized!
  // 3. If not "Always on Top" - hide PWS during Autotype and show
  //    it again once finished - but behind other windows.
  // NOTE: If "Lock on Minimize" is set and "MinimizeOnAutotype" then
  //       the window will be locked once minimized.

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::MinimizeOnAutotype)) {
    // Need to save display status for when we return from minimize
    //m_vGroupDisplayState = GetGroupDisplayState();
    TryIconize();
  } else {
    m_guiInfo->Save(this);
    Hide();
  }
}

void PasswordSafeFrame::MaybeRestoreUI(bool autotype_err, wxString autotype_err_msg)
{
  // Restore the UI if
  //   1. there was an error autotyping
  //   2. We're supposed to be always-on-top
  //   3. We hid the UI instead of minimizing while autotyping
  if (autotype_err || PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)
                   || !PWSprefs::GetInstance()->GetPref(PWSprefs::MinimizeOnAutotype)) {
    /* TODO - figure out how to keep a wxWidgets window always on top */
    if (IsIconized())
      Iconize(false);
    else {
      Show();
      m_guiInfo->Restore(this);
    }
  }
  if (autotype_err)
    wxMessageBox(_("There was an error autotyping.  ") + autotype_err_msg, _T("Autotype error"), wxOK|wxICON_ERROR, this);
}

/*
 * The entire logic is borrowed from ui/Windows/MainEdit.cpp
 */
void PasswordSafeFrame::DoAutotype(const StringX& sx_autotype,
          const std::vector<size_t>& vactionverboffsets)
{
  // All parsing of AutoType command done in one place: PWSAuxParse::GetAutoTypeString
  // Except for anything involving time (\d, \w, \W) or use older method (\z)
  StringX sxtmp(wxEmptyString);
  StringX sxautotype(sx_autotype);
  wchar_t curChar;

  const int N = static_cast<int>(sxautotype.length());

  CKeySend ks;
#ifdef __WXMAC__
  if (!ks.SimulateApplicationSwitch()) {
    wxMessageBox(_("Error switching to another application before autotyping. Switch manually within 5 seconds"),
                  wxTheApp->GetAppName(), wxOK|wxICON_ERROR, this);
    pws_os::sleep_ms(5000);
  }
  else {
    wxYield();
  }
#endif
  //sleep for 1 second
  pws_os::sleep_ms(1000); // Karl Student's suggestion, to ensure focus set correctly on minimize.

  int gNumIts;
  for (int n = 0; n < N; n++){
    curChar = sxautotype[n];
    if (curChar == L'\\') {
      n++;
      if (n < N)
        curChar = sxautotype[n];

      // Only need to process fields left in there by PWSAuxParse::GetAutoTypeString
      // for later processing
      switch (curChar) {
        case L'd':
        case L'w':
        case L'W':
        {
           if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), (size_t)(n - 1)) ==
               vactionverboffsets.end()) {
             // Not in the list of found action verbs - treat as-is
             sxtmp += L'\\';
             sxtmp += curChar;
             break;
           }

          /*
           'd' means value is in milli-seconds, max value = 0.999s
           and is the delay between sending each character

           'w' means value is in milli-seconds, max value = 0.999s
           'W' means value is in seconds, max value = 16m 39s
           and is the wait time before sending the next character.
           Use of this field does not change any current delay value.

           User needs to understand that PasswordSafe will be unresponsive
           for the whole of this wait period!
          */

          // Delay is going to change - send what we have with old delay
          ks.SendString(sxtmp);
          // start collecting new delay
          sxtmp = wxEmptyString;
          int newdelay = 0;
          gNumIts = 0;
          for (n++; n < N && (gNumIts < 3); ++gNumIts, n++) {
            if (_istdigit(sxautotype[n])) {
              newdelay *= 10;
              newdelay += (sxautotype[n] - L'0');
            } else
              break; // for loop
          }

          n--;
          // Either set new character delay time or wait specified time
          if (curChar == L'd') {
            ks.SetAndDelay(newdelay);
          }
          else
            pws_os::sleep_ms(newdelay * (curChar == L'w' ? 1 : 1000));

          break; // case 'd', 'w' & 'W'
        }
        case L'c':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), (size_t)(n - 1)) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          }
          else {
            ks.SelectAll();
          }
          break;
        case L'j':
        case L'k':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), (size_t)(n - 1)) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          }
          else {
            ks.EmulateMods(curChar == L'j');
          }
          break;
        case L'z':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), (size_t)(n - 1)) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          }
          break;
        case L'b':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), (size_t)(n - 1)) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          } else {
            sxtmp += L'\b';
          }
          break;
        default:
          sxtmp += L'\\';
          sxtmp += curChar;
          break;
      }
    } else
      sxtmp += curChar;
  }
  ks.SendString(sxtmp);

  pws_os::sleep_ms(1000);

}

void PasswordSafeFrame::DoBrowse(CItemData &item, bool bAutotype)
{
  CItemData* pci = &item;

  StringX sx_pswd;
  if (pci->IsDependent()) {
    CItemData *pbci = m_core.GetBaseEntry(pci);
    ASSERT(pbci != nullptr);
    sx_pswd = pbci->GetPassword();
    if (pci->IsShortcut())
      pci = pbci;
  } else
    sx_pswd = pci->GetPassword();

  wxString cs_command = towxstring(pci->GetURL());

  if (!cs_command.IsEmpty()) {
    std::vector<size_t> vactionverboffsets;
    StringX sxautotype = PWSAuxParse::GetAutoTypeString(*pci, m_core,
                                                        vactionverboffsets);
    LaunchBrowser(cs_command, sxautotype, vactionverboffsets, bAutotype);
#ifdef NOT_YET
    SetClipboardData(sx_pswd);
    UpdateLastClipboardAction(CItemData::PASSWORD);
#endif
    UpdateAccessTime(item);
  }
}

bool PasswordSafeFrame::LaunchBrowser(const wxString &csURL, const StringX &/*sxAutotype*/,
                             const std::vector<size_t> &/*vactionverboffsets*/,
                             bool /*bDoAutotype*/)
{
  /*
   * This is a straight port of DBoxMain::LaunchBrowser.  See the comments in that function
   * to understand what this code is doing, and why.
   */
  wxString theURL(csURL);
  theURL.Replace(wxT("\n\t\r"), wxEmptyString, true); //true => replace all

  const bool isMailto = (theURL.Find(wxT("mailto:")) != wxNOT_FOUND);
  const wxString errMsg = isMailto ? _("Unable to send email") : _("Unable to display URL");

  const size_t altReplacements = theURL.Replace(wxT("[alt]"), wxEmptyString);
  const size_t alt2Replacements = (theURL.Replace(wxT("[ssh]"), wxEmptyString) +
                          theURL.Replace(wxT("{alt}"), wxEmptyString));
#ifdef NOT_YET
  const size_t autotypeReplacements = theURL.Replace(wxT("[autotype]"), wxEmptyString);
  const size_t no_autotype = theURL.Replace(wxT("[xa]"), wxEmptyString);
#endif

  if (alt2Replacements == 0 && !isMailto && theURL.Find(wxT("://")) == wxNOT_FOUND)
    theURL = wxT("http://") + theURL;

  const wxString sxAltBrowser(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::AltBrowser)));
  const bool useAltBrowser = ((altReplacements > 0 || alt2Replacements > 0) && !sxAltBrowser.empty());

  wxString sxFile, sxParameters;
  if (!useAltBrowser) {
    sxFile = theURL;
  }
  else { // alternate browser specified, invoke w/optional args
    sxFile = sxAltBrowser;
    wxString sxCmdLineParms(towxstring(PWSprefs::GetInstance()->
                           GetPref(PWSprefs::AltBrowserCmdLineParms)));

    if (!sxCmdLineParms.empty())
      sxParameters = sxCmdLineParms + wxT(" ") + theURL;
    else
      sxParameters = theURL;
  }

  sxFile.Trim(false); //false => from left

#ifdef NOT_YET
  // Obey user's No Autotype flag [xa]
  if (no_autotype > 0) {
    m_bDoAutoType = false;
    m_AutoType.clear();
    m_vactionverboffsets.clear();
  }
  else {
    // Either do it because they pressed the right menu/shortcut
    // or they had specified Do Autotype flag [autotype]
    m_bDoAutoType = bDoAutotype || autotypeReplacements > 0;
    m_AutoType = m_bDoAutoType ? sxAutotype : wxEmptyString;
    if (m_bDoAutoType)
      m_vactionverboffsets = vactionverboffsets;
  }
#endif

#ifdef NOT_YET
  bool rc = m_runner.issuecmd(sxFile, sxParameters, !m_AutoType.empty());
#else
  bool rc;
  if (useAltBrowser) {
    const wxString cmdLine(sxFile + wxT(" ") + sxParameters);
    rc = (::wxExecute(cmdLine, wxEXEC_ASYNC) != 0);
  }
  else {
    rc= ::wxLaunchDefaultBrowser(sxFile);
  }
#endif

  if (!rc) {
    wxMessageBox(errMsg, wxTheApp->GetAppName(), wxOK|wxICON_STOP, this);
  }
  return rc;
}

void PasswordSafeFrame::DoRun(CItemData& item)
{
  const StringX runee = item.GetRunCommand();
  PWSRun runner;
  if (runner.runcmd(runee, false))
    UpdateAccessTime(item);
}

void PasswordSafeFrame::DoEmail(CItemData& item )
{
  const wxString mailto = item.IsEmailEmpty()?
                      (item.IsURLEmail()? towxstring(item.GetURL()): wxString())
                      : towxstring(item.GetEmail());
  if (!mailto.empty()) {
    ::wxLaunchDefaultBrowser(mailto, wxBROWSER_NEW_WINDOW);
    UpdateAccessTime(item);
  }
}

void PasswordSafeFrame::DoPasswordSubset(CItemData& item )
{
  CPasswordSubset psDlg(this, item.GetPassword());
  psDlg.ShowModal();
  UpdateAccessTime(item);
}

void PasswordSafeFrame::OnUndo(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  m_core.Undo();
//  RestoreGUIStatus();
}

void PasswordSafeFrame::OnRedo(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
//  SaveGUIStatus();
  m_core.Redo();
}

void PasswordSafeFrame::OnPasswordSubset(wxCommandEvent &evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoPasswordSubset(*item);
}

void PasswordSafeFrame::OnPasswordQRCode(wxCommandEvent &evt)
{
  if ( /* constexpr */ HasQRCode() ) {
    CItemData rueItem;
    CItemData* item = GetSelectedEntry(evt, rueItem);
    if (item != nullptr) {
#ifndef NO_QR
    PWSQRCodeDlg dlg(this, item->GetPassword(),
              towxstring(CItemData::FieldName(CItem::PASSWORD)) + _T(" of ") +
              towxstring(item->GetGroup()) +
              _T('[') + towxstring(item->GetTitle()) + _T(']') +
              _T(':') + towxstring(item->GetUser()));
      dlg.ShowModal();
#endif
    }
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_PROTECT
 */

void PasswordSafeFrame::OnProtectUnprotectClick( wxCommandEvent& event )
{
  CItemData *item = GetSelectedEntry();
  if (item != nullptr) {
    bool protect = event.IsChecked();
    m_core.Execute(UpdateEntryCommand::Create(&m_core, *item,
                                              CItemData::PROTECTED,
                                              protect ? L"1" : L"0"));
  }
}
