/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "../../corelib/PWSAuxParse.h"
#include "../../corelib/Util.h"
#include "../../os/KeySend.h"
#include "../../os/sleep.h"

#include <algorithm>

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_EDIT
 */

void PasswordSafeFrame::OnEditClick( wxCommandEvent& event )
{
  CItemData *item = GetSelectedEntry();
  if (item != NULL)
    DoEdit(*item);
}

void PasswordSafeFrame::DoEdit(CItemData &item)
{
  int rc = 0;
  if (!item.IsShortcut()) {
    AddEditPropSheet editDbox(this, m_core, AddEditPropSheet::EDIT, &item);
    rc = editDbox.ShowModal();
  } else {
    EditShortcut editDbox(this, m_core, &item);
    rc = editDbox.ShowModal();
  }
  if (rc != 0)
    UpdateAccessTime(item);
}



/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ADD
 */

void PasswordSafeFrame::OnAddClick( wxCommandEvent& event )
{
  AddEditPropSheet addDbox(this, m_core, AddEditPropSheet::ADD);
  if (addDbox.ShowModal() == wxID_OK) {
    const CItemData &item = addDbox.GetItem();
    m_core.Execute(AddEntryCommand::Create(&m_core, item));
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
 */

void PasswordSafeFrame::OnDeleteClick( wxCommandEvent& event )
{
  bool dontaskquestion = !PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);

  bool dodelete = true;
  int num_children = 0;
  // If tree view, check if group selected
  if (m_tree->IsShown()) {
    wxTreeItemId sel = m_tree->GetSelection();
    num_children = m_tree->GetChildrenCount(sel);
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

void PasswordSafeFrame::OnFindClick( wxCommandEvent& event )
{
  m_search->Activate();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EDITMENU_FIND_NEXT
 */

void PasswordSafeFrame::OnFindNext( wxCommandEvent& event )
{
  m_search->FindNext();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EDITMENU_FIND_PREVIOUS
 */

void PasswordSafeFrame::OnFindPrevious( wxCommandEvent& event )
{
  m_search->FindPrevious();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CLEARCLIPBOARD
 */

void PasswordSafeFrame::OnClearclipboardClick( wxCommandEvent& event )
{
  PWSclip::ClearData();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYPASSWORD
 */

void PasswordSafeFrame::OnCopypasswordClick( wxCommandEvent& event )
{
  CItemData *item = GetSelectedEntry();
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

void PasswordSafeFrame::OnCopyusernameClick( wxCommandEvent& event )
{
  CItemData *item = GetSelectedEntry();
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

void PasswordSafeFrame::OnCopynotesfldClick( wxCommandEvent& event )
{
  CItemData *item = GetSelectedEntry();
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

void PasswordSafeFrame::OnCopyurlClick( wxCommandEvent& event )
{
  CItemData *item = GetSelectedEntry();
  if (item != NULL)
    DoCopyURL(*item);
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
    if (rc != 0) {
      UpdateAccessTime(*item);
      Show(true);
    }
  }
}

void PasswordSafeFrame::DoCopyURL(CItemData &item)
{
  PWSclip::SetData(item.GetURL());
  UpdateAccessTime(item);
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
  } else
    Hide();

  std::vector<size_t> vactionverboffsets;
  sxautotype = PWSAuxParse::GetAutoTypeString(ci, vactionverboffsets);
  DoAutotype(sxautotype, vactionverboffsets);

  // If we minimized it, exit. If we only hid it, now show it
  if (bMinOnAuto)
    return;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
    /* TODO - figure out how to keep a wxWidgets window always on top */
    if (IsIconized())
      Iconize(false);
    else
      Show();
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
 
  const int N = sxautotype.length();

  //sleep for 1 second
  pws_os::sleep_ms(1000); // Karl Student's suggestion, to ensure focus set correctly on minimize.

  CKeySend ks;

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

void PasswordSafeFrame::DoBrowse(CItemData &item)
{
  const wxString url = item.GetURL().c_str();

  if (!url.empty())
    ::wxLaunchDefaultBrowser(url, wxBROWSER_NEW_WINDOW);
}

void PasswordSafeFrame::DoRun(CItemData &item)
{
}

void PasswordSafeFrame::DoEmail(CItemData &item)
{
}

