/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
  AddEditPropSheet editDbox(this, m_core, m_grid, m_tree,
                            AddEditPropSheet::EDIT, &item);
  editDbox.ShowModal(); // update view if returned OK, all the rest done internally
  UpdateAccessTime(item);
}



/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ADD
 */

void PasswordSafeFrame::OnAddClick( wxCommandEvent& event )
{
  AddEditPropSheet addDbox(this, m_core, m_grid, m_tree,
                           AddEditPropSheet::ADD);
  if (addDbox.ShowModal() == wxID_OK) {
    const CItemData &item = addDbox.GetItem();
    m_core.Execute(AddEntryCommand::Create(&m_core, item));
    Show(true);
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
  int num_children = 0; // TBD: != 0 if group selected

  //Confirm whether to delete the item
  if (!dontaskquestion) {
    DeleteConfirmation deleteDlg(this, num_children);
    int rc = deleteDlg.ShowModal();
    if (rc != wxID_YES) {
      dodelete = false;
    }
  }

  if (dodelete) {
    const CItemData *item = GetSelectedEntry();
    if (item != NULL) {
      uuid_array_t uuid;
      item->GetUUID(uuid);
      Delete(uuid);
    }
  }
}

void PasswordSafeFrame::Delete(const uuid_array_t &uuid)
{
  if (m_grid->IsShown())
    m_grid->Remove(uuid);
  else
    m_tree->Remove(uuid);
  ItemListIter iter = m_core.Find(uuid);
  if (iter != m_core.GetEntryEndIter())
    m_core.Execute(DeleteEntryCommand::Create(&m_core,
                                              m_core.GetEntry(iter)));
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

