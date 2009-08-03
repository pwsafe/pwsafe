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

#include "PWSgrid.h"
#include "PWStree.h"

#include "passwordsafeframe.h"
#include "addeditpropsheet.h"
#include "pwsclip.h"


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_EDIT
 */

void PasswordSafeFrame::OnEditClick( wxCommandEvent& event )
{
  const CItemData *item = GetSelectedEntry();
  if (item != NULL) {
    AddEditPropSheet editDbox(this, m_core, m_grid, m_tree,
                              AddEditPropSheet::EDIT, item);
    editDbox.ShowModal(); // update view if returned OK, all the rest done internally
  }
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
    m_core.AddEntry(item);
    Show(true);
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
 */

void PasswordSafeFrame::OnDeleteClick( wxCommandEvent& event )
{
  bool dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);

  bool dodelete = true;
  int num_children = 0; // TBD: != 0 if group selected

  //Confirm whether to delete the item
  if (!dontaskquestion) {
#ifdef NOTYET
    CConfirmDeleteDlg deleteDlg(this, num_children);
    INT_PTR rc = deleteDlg.DoModal();
    if (rc == IDCANCEL) {
      dodelete = false;
    }
#endif
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
  m_grid->Remove(uuid);
  m_tree->Remove(uuid);
  m_core.RemoveEntryAt(m_core.Find(uuid));
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
  const CItemData *item = GetSelectedEntry();
  if (item != NULL)
    PWSclip::SetData(item->GetPassword());
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYUSERNAME
 */

void PasswordSafeFrame::OnCopyusernameClick( wxCommandEvent& event )
{
  const CItemData *item = GetSelectedEntry();
  if (item != NULL)
    PWSclip::SetData(item->GetUser());
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYNOTESFLD
 */

void PasswordSafeFrame::OnCopynotesfldClick( wxCommandEvent& event )
{
  const CItemData *item = GetSelectedEntry();
  if (item != NULL)
    PWSclip::SetData(item->GetNotes());
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_COPYURL
 */

void PasswordSafeFrame::OnCopyurlClick( wxCommandEvent& event )
{
  const CItemData *item = GetSelectedEntry();
  if (item != NULL)
    PWSclip::SetData(item->GetURL());
}

