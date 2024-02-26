/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DeleteConfirmationDlg.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "DeleteConfirmationDlg.h"
#include "core/PWSprefs.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * DeleteConfirmationDlg type definition
 */

IMPLEMENT_CLASS( DeleteConfirmationDlg, wxDialog )

/*!
 * DeleteConfirmationDlg event table definition
 */

BEGIN_EVENT_TABLE( DeleteConfirmationDlg, wxDialog )

////@begin DeleteConfirmationDlg event table entries
  EVT_BUTTON( wxID_YES, DeleteConfirmationDlg::OnYesClick )

  EVT_BUTTON( wxID_NO, DeleteConfirmationDlg::OnNoClick )

////@end DeleteConfirmationDlg event table entries

END_EVENT_TABLE()

/*!
 * DeleteConfirmationDlg constructors
 */

DeleteConfirmationDlg::DeleteConfirmationDlg(wxWindow *parent, bool isGroup, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : m_isGroup(isGroup)
{
  wxASSERT(!parent || parent->IsTopLevel());
////@begin DeleteConfirmationDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end DeleteConfirmationDlg creation
}

DeleteConfirmationDlg* DeleteConfirmationDlg::Create(wxWindow *parent, bool isGroup, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  return new DeleteConfirmationDlg(parent, isGroup, id, caption, pos, size, style);
}

/*!
 * Control creation for DeleteConfirmationDlg
 */

void DeleteConfirmationDlg::CreateControls()
{
  DeleteConfirmationDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  m_areyousure = new wxStaticText( itemDialog1, wxID_STATIC,
                                  m_isGroup ?
                                   _("Are you sure you want to delete the selected group?") :
                                   _("Are you sure you want to delete the selected entry?"),
                                   wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(m_areyousure, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxCheckBox* itemCheckBox4 = new wxCheckBox( itemDialog1, ID_CHECKBOX37, _("Don't ask me again"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox4->SetValue(false);

  itemBoxSizer2->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, 5);
  itemCheckBox4->Show(!m_isGroup); // Don't show the "Don't ask" checkbox for group deletes

  wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton6 = new wxButton( itemDialog1, wxID_YES, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer5->AddButton(itemButton6);

  wxButton* itemButton7 = new wxButton( itemDialog1, wxID_NO, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton7->SetDefault();
  itemStdDialogButtonSizer5->AddButton(itemButton7);

  itemStdDialogButtonSizer5->Realize();

  // Set validators
  itemCheckBox4->SetValidator( wxGenericValidator(& m_confirmdelete) );

}

/*!
 * Should we show tooltips?
 */

bool DeleteConfirmationDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap DeleteConfirmationDlg::GetBitmapResource( const wxString& WXUNUSED(name) )
{
  // Bitmap retrieval
////@begin DeleteConfirmationDlg bitmap retrieval
  return wxNullBitmap;
////@end DeleteConfirmationDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DeleteConfirmationDlg::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin DeleteConfirmationDlg icon retrieval
  return wxNullIcon;
////@end DeleteConfirmationDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NO
 */

void DeleteConfirmationDlg::OnNoClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::DeleteQuestion,
                                     m_confirmdelete);

  }
  EndModal(wxID_NO);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_YES
 */

void DeleteConfirmationDlg::OnYesClick(wxCommandEvent& WXUNUSED(evt))
{
  if (Validate() && TransferDataFromWindow()) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::DeleteQuestion,
                                     !m_confirmdelete);

  }
  EndModal(wxID_YES);
}
