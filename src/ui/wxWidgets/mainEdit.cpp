/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "../../core/PWSAuxParse.h"
#include "../../core/Util.h"
#include "../../os/KeySend.h"
#include "../../os/sleep.h"

#include <algorithm>

using pws_os::CUUID;

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_EDIT
 */

void PasswordSafeFrame::OnEditClick( wxCommandEvent& /* evt */ )
{
  CItemData *item = GetSelectedEntry();
  if (item != NULL)
    DoEdit(*item);
}

//This function intentionally takes the argument by value and not by
//reference to avoid touching an item invalidated by idle timeout
void PasswordSafeFrame::DoEdit(CItemData item)
{
  int rc = 0;
  if (!item.IsShortcut()) {
    AddEditPropSheet editDbox(this, m_core, AddEditPropSheet::EDIT, &item);
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
    ItemListIter iter = m_core.Find(uuid);
    if ( iter != m_core.GetEntryEndIter()) {
      CItemData& origItem = m_core.GetEntry(iter);
      //The Item is updated in DB by AddEditPropSheet
      UpdateAccessTime(origItem);
      SetChanged(Data);
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
  if (IsTreeView() && (selection = m_tree->GetSelection()).IsOk() && m_tree->ItemHasChildren(selection)) {
    selectedGroup = m_tree->GetItemText(selection);
  }

  AddEditPropSheet addDbox(this, m_core, AddEditPropSheet::ADD, NULL, selectedGroup);
  if (addDbox.ShowModal() == wxID_OK) {
    const CItemData &item = addDbox.GetItem();
    m_core.Execute(AddEntryCommand::Create(&m_core, item));
    SetChanged(Data);
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
 */

void PasswordSafeFrame::OnDeleteClick( wxCommandEvent& /* evt */ )
{
  bool dontaskquestion = !PWSprefs::GetInstance()->
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
    int rc = deleteDlg.ShowModal();
    if (rc != wxID_YES) {
      dodelete = false;
    }
  }

  if (dodelete) {
    Command *doit = NULL;
    CItemData *item = GetSelectedEntry();
    if (item != NULL) {
      doit = Delete(item);
    } else if (num_children > 0) {
      doit = Delete(m_tree->GetSelection());
    }
    if (doit != NULL)
      m_core.Execute(doit);
  } // dodelete
}

Command *PasswordSafeFrame::Delete(CItemData *pci)
{
  ASSERT(pci != NULL);
  // ConfirmDelete asks for user confirmation
  // when deleting a shortcut or alias base.
  // Otherwise it just returns true
  if (m_core.ConfirmDelete(pci))
    return DeleteEntryCommand::Create(&m_core, *pci);
  else
    return NULL;
}

Command *PasswordSafeFrame::Delete(wxTreeItemId tid)
{
  // Called for deleting a group
  // Recursively build the appropriate multi-command

  MultiCommands *retval = MultiCommands::Create(&m_core);
  ASSERT(tid.IsOk());
  if (m_tree->GetChildrenCount(tid) > 0) {
    wxTreeItemIdValue cookie;
    wxTreeItemId ti = m_tree->GetFirstChild(tid, cookie);
    
    while (ti) {
      Command *delCmd = Delete(ti);
      if (delCmd != NULL)
        retval->Add(delCmd);
      // go through all siblings too
      wxTreeItemId sibling = m_tree->GetNextSibling(ti);
      while (sibling.IsOk()) {
        Command *delSibCmd = Delete(sibling);
        if (delSibCmd != NULL)
          retval->Add(delSibCmd);
        sibling = m_tree->GetNextSibling(sibling);
      } // while siblings
      ti = m_tree->GetNextChild(ti, cookie);
    } // while children
  } else { // leaf
    CItemData *leaf = m_tree->GetItem(tid);
    ASSERT(leaf != NULL);
    Command *delLeafCmd = Delete(leaf); // gets user conf. if needed
    if (delLeafCmd != NULL)
      retval->Add(delLeafCmd);
  }

  // If MultiCommands is empty, delete and return NULL
  if (retval->GetSize() == 0) {
    delete retval;
    retval = NULL;
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
  PWSclip::ClearData();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYPASSWORD
 */

void PasswordSafeFrame::OnCopypasswordClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = IsRUEEvent(evt)? (m_RUEList.GetPWEntry(GetEventRUEIndex(evt), rueItem)? &rueItem: NULL) : GetSelectedEntry();
  if (item != NULL)
    DoCopyPassword(*item);
}

void PasswordSafeFrame::DoCopyPassword(CItemData &item)
{
  PWSclip::SetData(item.GetPassword());
  UpdateAccessTime(item);
}



/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYUSERNAME
 */

void PasswordSafeFrame::OnCopyusernameClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = IsRUEEvent(evt)? (m_RUEList.GetPWEntry(GetEventRUEIndex(evt), rueItem)? &rueItem: NULL) : GetSelectedEntry();
  if (item != NULL)
    DoCopyUsername(*item);
}

void PasswordSafeFrame::DoCopyUsername(CItemData &item)
{
  PWSclip::SetData(item.GetUser());
  UpdateAccessTime(item);
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYNOTESFLD
 */

void PasswordSafeFrame::OnCopynotesfldClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = IsRUEEvent(evt)? (m_RUEList.GetPWEntry(GetEventRUEIndex(evt), rueItem)? &rueItem: NULL) : GetSelectedEntry();
  if (item != NULL)
    DoCopyNotes(*item);
}

void PasswordSafeFrame::DoCopyNotes(CItemData &item)
{
  PWSclip::SetData(item.GetNotes());
  UpdateAccessTime(item);
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYURL
 */

void PasswordSafeFrame::OnCopyurlClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = IsRUEEvent(evt)? (m_RUEList.GetPWEntry(GetEventRUEIndex(evt), rueItem)? &rueItem: NULL) : GetSelectedEntry();
  if (item != NULL)
    DoCopyURL(*item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYEMAIL
 */

void PasswordSafeFrame::OnCopyEmailClick(wxCommandEvent& evt)
{
  CItemData rueItem;
  CItemData* item = IsRUEEvent(evt)? (m_RUEList.GetPWEntry(GetEventRUEIndex(evt), rueItem)? &rueItem: NULL) : GetSelectedEntry();
  if (item != NULL)
    DoCopyEmail(*item);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CREATESHORTCUT
 */

void PasswordSafeFrame::OnCreateShortcut(wxCommandEvent& /*evt*/)
{
  CItemData* item = GetSelectedEntry();
  if (item && !item->IsDependent()) {
    CreateShortcutDlg dlg(this, m_core, item);
    int rc = dlg.ShowModal();
    if (rc == wxID_OK) {
      UpdateAccessTime(*item);
      Show(true);
      SetChanged(Data);
    }
  }
}

// Duplicate selected entry but make title unique
void PasswordSafeFrame::OnDuplicateEntry(wxCommandEvent& /*evt*/)
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;
//  if (SelItemOk() == TRUE) {
    CItemData *pci = GetSelectedEntry();
    ASSERT(pci != NULL);
//    DisplayInfo *pdi = (DisplayInfo *)pci->GetDisplayInfo();
//    ASSERT(pdi != NULL);

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
    ci2.SetDisplayInfo(NULL);
    ci2.CreateUUID();
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetStatus(CItemData::ES_ADDED);

    Command *pcmd = NULL;
    if (pci->IsDependent()) {
      if (pci->IsAlias()) {
        ci2.SetAlias();
      } else {
        ci2.SetShortcut();
      }

      const CItemData *pbci = m_core.GetBaseEntry(pci);
      if (pbci != NULL) {
        CUUID base_uuid = pbci->GetUUID();
        StringX cs_tmp;
        cs_tmp = L"[" +
          pbci->GetGroup() + L":" +
          pbci->GetTitle() + L":" +
          pbci->GetUser()  + L"]";
        ci2.SetPassword(cs_tmp);
        pcmd = AddEntryCommand::Create(&m_core, ci2, base_uuid);
      }
    } else { // not alias or shortcut
      ci2.SetNormal();
      pcmd = AddEntryCommand::Create(&m_core, ci2);
    }

    Execute(pcmd);

//    pdi->list_index = -1; // so that InsertItemIntoGUITreeList will set new values

    CUUID uuid = ci2.GetUUID();
    ItemListIter iter = m_core.Find(uuid);
    ASSERT(iter != m_core.GetEntryEndIter());

//    InsertItemIntoGUITreeList(m_core.GetEntry(iter));
//    FixListIndexes();
    SetChanged(Data);

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
  PWSclip::SetData(item.GetURL());
  UpdateAccessTime(item);
}

void PasswordSafeFrame::DoCopyEmail(CItemData &item)
{
  const StringX mailto = item.IsEmailEmpty()? 
                      (item.IsURLEmail()? item.GetURL(): StringX()) 
                      : item.GetEmail();
  
  if (!mailto.empty()) {
    PWSclip::SetData(mailto);
    UpdateAccessTime(item);
  }
}

/*
 * The entire logic is borrowed from ui/Windows/MainEdit.cpp
 */
void PasswordSafeFrame::DoAutotype(CItemData &ci)
{
  // Called from OnAutoType and OnTrayAutoType

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
  bool bMinOnAuto = PWSprefs::GetInstance()->
                    GetPref(PWSprefs::MinimizeOnAutotype);

  if (bMinOnAuto) {
    // Need to save display status for when we return from minimize
    //m_vGroupDisplayState = GetGroupDisplayState();
    Iconize(true);
    while (!IsIconized())
      wxSafeYield();
  } else {
    m_guiInfo->Save(this);
    Hide();
  }
 
  std::vector<size_t> vactionverboffsets;
  const StringX sxautotype = PWSAuxParse::GetAutoTypeString(ci, m_core,
                                                            vactionverboffsets);
  DoAutotype(sxautotype, vactionverboffsets);
  UpdateAccessTime(ci);

  // If we minimized it, exit. If we only hid it, now show it
  if (bMinOnAuto)
    return;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
    /* TODO - figure out how to keep a wxWidgets window always on top */
    if (IsIconized())
      Iconize(false);
    else {
      Show();
      m_guiInfo->Restore(this);
    }
  }
}

/*
 * The entire logic is borrowed from ui/Windows/MainEdit.cpp
 */
void PasswordSafeFrame::DoAutotype(const StringX& sx_autotype, 
					const std::vector<size_t>& vactionverboffsets)
{
  // All parsing of AutoType command done in one place: PWSAuxParse::GetAutoTypeString
  // Except for anything involving time (\d, \w, \W) or use older method (\z)
  StringX sxtmp(L"");
  StringX sxautotype(sx_autotype);
  wchar_t curChar;
 
  const int N = static_cast<int>(sxautotype.length());

  CKeySend ks;
#ifdef __WXMAC__
  if (!ks.SimulateApplicationSwitch()) {
    wxMessageBox(wxT("Error switching to another application before autotyping. Switch manually within 5 seconds"), 
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
           if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), n - 1) ==
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
          sxtmp = L"";
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
        case L'z':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), n - 1) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          }
          break;
        case L'b':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), n - 1) ==
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
    ASSERT(pbci != NULL);
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

BOOL PasswordSafeFrame::LaunchBrowser(const wxString &csURL, const StringX &/*sxAutotype*/,
                             const std::vector<size_t> &/*vactionverboffsets*/,
                             bool /*bDoAutotype*/)
{
  /*
   * This is a straight port of DBoxMain::LaunchBrowser.  See the comments in that function
   * to understand what this code is doing, and why.
   */
  wxString theURL(csURL);
  theURL.Replace(wxT("\n\t\r"), wxT(""), true); //true => replace all
  
  const bool isMailto = (theURL.Find(wxT("mailto:")) != wxNOT_FOUND);
  const wxString errMsg = isMailto ? _("Unable to send email") : _("Unable to display URL");
  
  const size_t altReplacements = theURL.Replace(wxT("[alt]"), wxT(""));
  const size_t alt2Replacements = (theURL.Replace(wxT("[ssh]"), wxT("")) +
                          theURL.Replace(wxT("{alt}"), wxT("")));
#ifdef NOT_YET
  const size_t autotypeReplacements = theURL.Replace(wxT("[autotype]"), wxT(""));
  const size_t no_autotype = theURL.Replace(wxT("[xa]"), wxT(""));
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
    // or they had specified Do Auotype flag [autotype]
    m_bDoAutoType = bDoAutotype || autotypeReplacements > 0;
    m_AutoType = m_bDoAutoType ? sxAutotype : wxT("");
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
  return rc ? TRUE : FALSE;
}

void PasswordSafeFrame::DoRun(CItemData& item)
{
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
