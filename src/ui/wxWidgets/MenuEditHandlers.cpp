/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file MenuEditHandlers.cpp
 *  This file contains implementations of PasswordSafeFrame
 *  member functions corresponding to actions under the 'Edit'
 *  menubar menu.
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/utils.h" // for wxLaunchDefaultBrowser

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/richmsgdlg.h>
#include <wx/tokenzr.h>

#include "core/PWSAuxParse.h"
#include "core/Util.h"
#include "os/KeySend.h"
#include "os/run.h"
#include "os/sleep.h"
#include "os/utf8conv.h"

#include "AddEditPropSheetDlg.h"
#include "Clipboard.h"
#include "CreateShortcutDlg.h"
#include "DeleteConfirmationDlg.h"
#include "EditShortcutDlg.h"
#include "GridCtrl.h"
#include "GuiInfo.h"
#include "PasswordSafeFrame.h"
#include "PasswordSafeSearch.h"
#include "PasswordSubsetDlg.h"
#include "QRCodeDlg.h"
#include "TimedTaskChain.h"
#include "TreeCtrl.h"
#include "ViewAttachmentDlg.h"
#include "wxUtilities.h"

#include <array>
#include <algorithm>

using pws_os::CUUID;

/**
 * This function intentionally takes the argument by value and not by
 * reference to avoid touching an item invalidated by idle timeout.
 */
void PasswordSafeFrame::DoEdit(CItemData item)
{
  int returnCode = 0;

  if (!item.IsShortcut()) {
    bool isItemReadOnly = m_core.IsReadOnly() || item.IsProtected();

    returnCode = ShowModalAndGetResult<AddEditPropSheetDlg>(this,
      m_core,
      isItemReadOnly ? AddEditPropSheetDlg::SheetType::VIEW : AddEditPropSheetDlg::SheetType::EDIT,
      &item
    );
  }
  else {
    returnCode = ShowModalAndGetResult<EditShortcutDlg>(this, m_core, &item);
  }

  if (returnCode == wxID_OK) {
    uuid_array_t uuid;
    item.GetUUID(uuid);
    //Find the item in the database, which might have been loaded afresh
    //after lock/unlock, so the old data structures are no longer valid
    const auto iter = m_core.Find(uuid);
    if (iter != m_core.GetEntryEndIter()) {
      CItemData& origItem = m_core.GetEntry(iter);
      //The Item is updated in DB by AddEditPropSheetDlg
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

void PasswordSafeFrame::OnAddClick(wxCommandEvent& WXUNUSED(evt))
{
  wxString selectedGroup = wxEmptyString;
  wxTreeItemId selection;

  if (IsTreeView() && (selection = m_tree->GetSelection()).IsOk() && m_tree->ItemIsGroup(selection)) {
    selectedGroup = m_tree->GetItemGroup(selection);
  }

  int rc = ShowModalAndGetResult<AddEditPropSheetDlg>(this, m_core, AddEditPropSheetDlg::SheetType::ADD, nullptr, selectedGroup);
  if (rc == wxID_OK) {
    UpdateStatusBar();
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_EDIT
 */

void PasswordSafeFrame::OnEditClick(wxCommandEvent& WXUNUSED(evt))
{
  CItemData *item = GetSelectedEntry();
  if (item) {
    CallAfter(&PasswordSafeFrame::DoEdit, *item);
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
 */

void PasswordSafeFrame::OnDeleteClick(wxCommandEvent& WXUNUSED(evt))
{
  bool dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);

  bool isGroup = false;
  // If tree view, check if group selected
  if (m_tree->IsShown()) {
    wxTreeItemId sel = m_tree->GetSelection();
    isGroup = m_tree->ItemIsGroup(sel);
    if (isGroup) // ALWAYS confirm group delete
      dontaskquestion = false;
  }
  CallAfter(&PasswordSafeFrame::DoDeleteItems, !dontaskquestion, isGroup);
}

void PasswordSafeFrame::DoDeleteItems(bool askConfirmation, bool isGroup)
{
  bool dodelete = true;
  //Confirm whether to delete the item
  if (askConfirmation) {
    DestroyWrapper<DeleteConfirmationDlg> deleteDlgWrapper(this, isGroup);
    DeleteConfirmationDlg* deleteDlg = deleteDlgWrapper.Get();
    deleteDlg->SetConfirmdelete(PWSprefs::GetInstance()->GetPref(PWSprefs::DeleteQuestion));
    int rc = deleteDlg->ShowModal();
    if (rc != wxID_YES) {
      dodelete = false;
    }
    if (!IsCloseInProgress()) {
      PWSprefs::GetInstance()->SetPref(PWSprefs::DeleteQuestion, deleteDlg->GetConfirmdelete());
    }
  }

  if (dodelete) {
    auto *commands = MultiCommands::Create(&m_core);
    Command *deleteCommand = nullptr;
    CItemData *item = GetSelectedEntry();
    wxTreeItemId tid;
    if (item != nullptr) {
      tid = m_tree->Find(*item);
      deleteCommand = DeleteItem(item);
    } else if (m_tree->GetSelection() != m_tree->GetRootItem()) {
      tid = m_tree->GetSelection();
      deleteCommand = Delete(tid);
    }
    if (deleteCommand != nullptr) {
      commands->Add(deleteCommand);
    }
    // If the item was the only child of its parent we make the parent empty.
    // See src/ui/Windows/MainEdit::OnDelete
    if (tid.IsOk()) {
      auto root = m_tree->GetRootItem();
      auto parent = m_tree->GetItemParent(tid);
      auto children = m_tree->GetChildrenCount(parent, false);
      if (parent != nullptr && root != nullptr && parent != root && children == 1) {
        StringX sxPath = tostringx(m_tree->GetItemGroup(parent));
        commands->Add(
          DBEmptyGroupsCommand::Create(&m_core, sxPath, DBEmptyGroupsCommand::EG_ADD)
        );
      }
    }
    // If MultiCommands is empty, delete
    if (commands->GetSize() == 0) {
      delete commands;
    }
    else {
      commands->Add(
        UpdateGUICommand::Create(
          &m_core,
          UpdateGUICommand::ExecuteFn::WN_EXECUTE,
          UpdateGUICommand::GUI_Action::GUI_REFRESH_TREE
        )
      );
      m_core.Execute(commands);
    }
  } // dodelete
}

Command *PasswordSafeFrame::DeleteItem(CItemData *pci, wxTreeItemId root)
{
  ASSERT(pci != nullptr);
  // Alias in same group, which will be deleted, are not given warning for
  StringX sxGroup = L"";
  if(root)
    sxGroup = tostringx(m_tree->GetItemGroup(root));
  // ConfirmDelete asks for user confirmation
  // when deleting a shortcut or alias base.
  // Otherwise it just returns true
  if (m_core.ConfirmDelete(pci, sxGroup))
    return DeleteEntryCommand::Create(&m_core, *pci);
  else
    return nullptr;
}

Command *PasswordSafeFrame::Delete(wxTreeItemId tid, wxTreeItemId root)
{
  // Called for deleting a group
  // Recursively build the appropriate multi-command

  if (!tid) return nullptr;
  if (!root)
    root = tid;
  MultiCommands *retval = MultiCommands::Create(&m_core);
  if (m_tree->ItemIsGroup(tid) && (m_tree->GetChildrenCount(tid) > 0)) {
    wxTreeItemIdValue cookie;
    wxTreeItemId ti = m_tree->GetFirstChild(tid, cookie);

    while (ti.IsOk()) {
      Command *delCmd = Delete(ti, root);
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
      Command *delLeafCmd = DeleteItem(leaf, root); // gets user conf. if needed
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

void PasswordSafeFrame::OnFindClick(wxCommandEvent& WXUNUSED(evt))
{
  wxASSERT(m_search);
  m_search->Activate();
  ShowSearchBar();
  m_search->SetFocusIntoEditField();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EDITMENU_FIND_NEXT
 */

void PasswordSafeFrame::OnFindNext(wxCommandEvent& WXUNUSED(evt))
{
  m_search->FindNext();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EDITMENU_FIND_PREVIOUS
 */

void PasswordSafeFrame::OnFindPrevious(wxCommandEvent& WXUNUSED(evt))
{
  m_search->FindPrevious();
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

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CLEARCLIPBOARD
 */

void PasswordSafeFrame::OnClearClipboardClick(wxCommandEvent& WXUNUSED(evt))
{
  UpdateLastClipboardAction(CItemData::FieldType::END);
  Clipboard::GetInstance()->ClearCBData();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYPASSWORD
 */

void PasswordSafeFrame::OnCopyPasswordClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyPassword(*item);
}

void PasswordSafeFrame::DoCopyPassword(CItemData &item)
{
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::DontAskQuestion)) {
    wxRichMessageDialog dialog(this, 
      _("Pressing OK will copy the password of the selected item\nto the clipboard. The clipboard will be securely cleared\nwhen Password Safe is closed.\n\nPressing Cancel stops the password from being copied."), 
      _("Clear Clipboard"), 
      wxOK | wxCANCEL | wxICON_INFORMATION);

    dialog.ShowCheckBox(_("&Don't remind me again"));

    if(dialog.ShowModal() == wxID_CANCEL) {
      return;
    }

    PWSprefs::GetInstance()->SetPref(PWSprefs::DontAskQuestion, !dialog.IsCheckBoxChecked());
  }
  
  if (!item.IsDependent())
    Clipboard::GetInstance()->SetData(item.GetPassword());
  else {
    const CUUID &base = item.GetBaseUUID();
    const StringX &passwd = m_core.GetEntry(m_core.Find(base)).GetPassword();
    Clipboard::GetInstance()->SetData(passwd);
  }
  UpdateLastClipboardAction(CItemData::FieldType::PASSWORD);
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
  Clipboard::GetInstance()->SetData(item.GetRunCommand());
  UpdateLastClipboardAction(CItemData::FieldType::RUNCMD);
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYUSERNAME
 */

void PasswordSafeFrame::OnCopyUsernameClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyUsername(*item);
}

void PasswordSafeFrame::DoCopyUsername(CItemData &item)
{
  Clipboard::GetInstance()->SetData(item.GetUser());
  UpdateLastClipboardAction(CItemData::FieldType::USER);
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYNOTESFLD
 */

void PasswordSafeFrame::OnCopyNotesFieldClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item != nullptr)
    DoCopyNotes(*item);
}

void PasswordSafeFrame::DoCopyNotes(CItemData &item)
{
  Clipboard::GetInstance()->SetData(item.GetNotes());
  UpdateLastClipboardAction(CItemData::FieldType::NOTES);
  UpdateAccessTime(item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYURL
 */

void PasswordSafeFrame::OnCopyUrlClick(wxCommandEvent& evt)
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
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_BROWSEURL
 */

void PasswordSafeFrame::OnBrowseUrl(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item)
    DoBrowse(*item, false); //false => no autotype
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_BROWSEURLPLUS
 */

void PasswordSafeFrame::OnBrowseUrlAndAutotype(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item) {
    DoBrowse(*item, true); //true => autotype
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_AUTOTYPE
 */

void PasswordSafeFrame::OnAutoType(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = GetSelectedEntry(evt, rueItem);
  if (item) {
#ifdef __WXMAC__
    Lower();
#endif
    DoAutotype(*item);
  }
}

void PasswordSafeFrame::OnViewAttachment(wxCommandEvent& WXUNUSED(evt))
{
  CItemData* item = GetSelectedEntry();

  if (item == nullptr) {
    return;
  }

  if (!item->HasAttRef()) {
    return;
  }

  CallAfter(&PasswordSafeFrame::DoViewAttachment, item);
}

void PasswordSafeFrame::DoViewAttachment(CItemData* item)
{
  ASSERT(m_core.HasAtt(item->GetAttUUID()));

  CItemAtt itemAttachment = m_core.GetAtt(item->GetAttUUID());

  // Shouldn't be here if no content
  if (!itemAttachment.HasContent()) {
    return;
  }

  // Get media type before we find we can't load it
  auto mediaTypeDescription = stringx2std(itemAttachment.GetMediaType());

  if (!IsMimeTypeImage(mediaTypeDescription)) {
    wxMessageDialog(
      this,
      _("There is no view available for attachments that are not of image media type.\n"),
      _("View Attachment"),
      wxICON_INFORMATION
    ).ShowModal();

    return;
  }

  DestroyWrapper<ViewAttachmentDlg> viewAttachmentDlgWrapper(this);
  ViewAttachmentDlg *viewAttachmentDlg = viewAttachmentDlgWrapper.Get();

  if (viewAttachmentDlg->LoadImage(itemAttachment)) {
    viewAttachmentDlg->ShowModal();
  }
  else {
    wxMessageDialog(
      this,
      _("No preview available due to an error."),
      _("View Attachment"),
      wxICON_ERROR
    ).ShowModal();
  }
}

void PasswordSafeFrame::OnGotoBase(wxCommandEvent& WXUNUSED(evt))
{
  CItemData* item = GetSelectedEntry();
  if (item && (item->IsAlias() || item->IsShortcut())) {
    item = m_core.GetBaseEntry(item);
    CUUID base_uuid = item->GetUUID();
    SelectItem(base_uuid);
    UpdateAccessTime(*item);
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CREATESHORTCUT
 */

void PasswordSafeFrame::OnCreateShortcut(wxCommandEvent& WXUNUSED(event))
{
  CItemData* item = GetSelectedEntry();
  if (item && !item->IsDependent()) {
    CallAfter(&PasswordSafeFrame::DoCreateShortcut, item);
  }
}

void PasswordSafeFrame::DoCreateShortcut(CItemData* item)
{
  if (ShowModalAndGetResult<CreateShortcutDlg>(this, m_core, item) == wxID_OK) {
    UpdateAccessTime(*item);
    Show(true);
    UpdateStatusBar();
  }
}

void PasswordSafeFrame::DoCopyURL(CItemData &item)
{
  Clipboard::GetInstance()->SetData(item.GetURL());
  UpdateLastClipboardAction(CItemData::FieldType::URL);
  UpdateAccessTime(item);
}

void PasswordSafeFrame::DoCopyEmail(CItemData &item)
{
  const StringX mailto = item.IsEmailEmpty()?
                      (item.IsURLEmail()? item.GetURL(): StringX())
                      : item.GetEmail();

  if (!mailto.empty()) {
    Clipboard::GetInstance()->SetData(mailto);
    UpdateLastClipboardAction(CItemData::FieldType::EMAIL);
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
    wxMessageBox(_("There was an error autotyping.  ") + autotype_err_msg, _("Autotype error"), wxOK|wxICON_ERROR, this);
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
           if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), static_cast<size_t>(n - 1)) ==
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
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), static_cast<size_t>(n - 1)) ==
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
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), static_cast<size_t>(n - 1)) ==
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
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), static_cast<size_t>(n - 1)) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          }
          break;
        case L'b':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), static_cast<size_t>(n - 1)) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          } else {
            sxtmp += L'\b';
          }
          break;
        default:
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

    if (PWSprefs::GetInstance()->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL)) {
      Clipboard::GetInstance()->SetData(sx_pswd);
      UpdateLastClipboardAction(CItemData::FieldType::PASSWORD);
    }

    // TODO: Minimize depending on PWSprefs::MinimizeOnAutotype

    UpdateAccessTime(item);
  }
}

bool PasswordSafeFrame::LaunchBrowser(const wxString &csURL, const StringX & WXUNUSED(sxAutotype),
                             const std::vector<size_t> & WXUNUSED(vactionverboffsets),
                             bool WXUNUSED(bDoAutotype))
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
  const StringX runee   = item.GetRunCommand();
  const CItemData *pci  = &item;
  const CItemData *pbci = pci->IsDependent() ? m_core.GetBaseEntry(pci) : nullptr;

  CItemData effci;
  StringX lastpassword, totpauthcode;

  PWSAuxParse::GetEffectiveValues(pci, pbci, effci, lastpassword, totpauthcode);

  if (effci.GetRunCommand().empty()) {
    return;
  }

  stringT errorMessage;
  StringX::size_type columnPosition;
  bool isSpecialUrl, doAutoType;
  StringX expandedAutoType;

  StringX expandedES(PWSAuxParse::GetExpandedString(effci.GetRunCommand(),
                                                    m_core.GetCurFile(), pci, pbci,
                                                    doAutoType, expandedAutoType,
                                                    errorMessage, columnPosition, isSpecialUrl));

  if (!errorMessage.empty()) {
    wxMessageBox(
      wxString::Format(_("Error at column %d:\n\n%s"), static_cast<int>(columnPosition), errorMessage.c_str()), 
      _("Error parsing Run Command"), 
      wxOK|wxICON_ERROR, this);

    return;
  }

  pws_os::CUUID uuid = pci->GetUUID();

  std::vector<size_t> vactionverboffsets;

  // if no autotype value in run command's $a(value), start with item's (bug #1078)
  if (expandedAutoType.empty()) {
    expandedAutoType = pci->GetAutoType();
  }

  expandedAutoType = PWSAuxParse::GetAutoTypeString(expandedAutoType,
                                                    effci.GetGroup(), effci.GetTitle(), effci.GetUser(),
                                                    effci.GetPassword(), lastpassword,
                                                    effci.GetNotes(), effci.GetURL(), effci.GetEmail(),
                                                    totpauthcode,
                                                    vactionverboffsets);

  // Now honour presence of [alt], {alt} or [ssh] in the url if present
  // in the RunCommand field.  Note: they are all treated the same (unlike
  // in 'Browse to'.
  StringX altBrowser(PWSprefs::GetInstance()->GetPref(PWSprefs::AltBrowser));

  if (isSpecialUrl && !altBrowser.empty()) {
    StringX cmdLineParams(PWSprefs::GetInstance()->GetPref(PWSprefs::AltBrowserCmdLineParms));

    if (altBrowser[0] != L'\'' && altBrowser[0] != L'"') {
      altBrowser = L"\"" + altBrowser + L"\"";
    }
    if (!cmdLineParams.empty()) {
      expandedES = altBrowser + StringX(L" ") + cmdLineParams + StringX(L" ") + expandedES;
    }
    else {
      expandedES = altBrowser + StringX(L" ") + expandedES;
    }
  }

  // FR856 - Copy Password to Clipboard on Run-Command When copy-on-browse set.
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL)) {
    Clipboard::GetInstance()->SetData(effci.GetPassword());
    UpdateLastClipboardAction(CItemData::FieldType::PASSWORD);
  }

  PWSRun runner;

  if (runner.runcmd(expandedES, !expandedAutoType.empty())) {
    UpdateAccessTime(item);
  }
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

void PasswordSafeFrame::DoPasswordSubset(CItemData* item)
{
  if (item) {
    ShowModalAndGetResult<PasswordSubsetDlg>(this, item->GetPassword());
    UpdateAccessTime(*item);
  }
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
  if (item) {
    CallAfter(&PasswordSafeFrame::DoPasswordSubset, item);
  }
}

void PasswordSafeFrame::OnPasswordQRCode(wxCommandEvent &evt)
{
  if ( /* constexpr */ HasQRCode() ) {
    CItemData rueItem;
    CItemData* item = GetSelectedEntry(evt, rueItem);
    if (item != nullptr) {
      CallAfter(&PasswordSafeFrame::DoPasswordQRCode, item);
    }
  }
}

void PasswordSafeFrame::DoPasswordQRCode(CItemData* item)
{
#ifndef NO_QR
  if (item) {
    ShowModalAndGetResult<QRCodeDlg>(this, item->GetPassword(),
            towxstring(CItemData::FieldName(CItem::PASSWORD)) + _T(" of ") +
            towxstring(item->GetGroup()) +
            _T('[') + towxstring(item->GetTitle()) + _T(']') +
            _T(':') + towxstring(item->GetUser()));
  }
#endif
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
