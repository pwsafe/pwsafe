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

void PasswordSafeFrame::DoAutotype(CItemData &item)
{
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

