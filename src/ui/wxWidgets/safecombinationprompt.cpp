/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file safecombinationprompt.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "safecombinationprompt.h"
#include "os/file.h"

////@begin XPM images
#include "../graphics/cpane.xpm"
////@end XPM images


/*!
 * CSafeCombinationPrompt type definition
 */

IMPLEMENT_CLASS( CSafeCombinationPrompt, wxDialog )


/*!
 * CSafeCombinationPrompt event table definition
 */

BEGIN_EVENT_TABLE( CSafeCombinationPrompt, wxDialog )

////@begin CSafeCombinationPrompt event table entries
  EVT_BUTTON( wxID_OK, CSafeCombinationPrompt::OnOkClick )

  EVT_BUTTON( wxID_CANCEL, CSafeCombinationPrompt::OnCancelClick )

////@end CSafeCombinationPrompt event table entries

END_EVENT_TABLE()


/*!
 * CSafeCombinationPrompt constructors
 */

CSafeCombinationPrompt::CSafeCombinationPrompt(wxWindow* parent, PWScore &core,
                                               const wxString &fname, wxWindowID id,
                                               const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core), m_filename(fname), m_tries(0)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * CSafeCombinationPrompt creator
 */

bool CSafeCombinationPrompt::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSafeCombinationPrompt creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CSafeCombinationPrompt creation
  return true;
}


/*!
 * CSafeCombinationPrompt destructor
 */

CSafeCombinationPrompt::~CSafeCombinationPrompt()
{
////@begin CSafeCombinationPrompt destruction
////@end CSafeCombinationPrompt destruction
}


/*!
 * Member initialisation
 */

void CSafeCombinationPrompt::Init()
{
////@begin CSafeCombinationPrompt member initialisation
////@end CSafeCombinationPrompt member initialisation
}


/*!
 * Control creation for CSafeCombinationPrompt
 */

void CSafeCombinationPrompt::CreateControls()
{    
////@begin CSafeCombinationPrompt content construction
  CSafeCombinationPrompt* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticBitmap* itemStaticBitmap4 = new wxStaticBitmap( itemDialog1, wxID_STATIC, itemDialog1->GetBitmapResource(wxT("../graphics/cpane.xpm")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(49, 46)), 0 );
  itemBoxSizer3->Add(itemStaticBitmap4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please enter the safe combination for this password database."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("filename"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(itemBoxSizer8, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Safe\ncombination:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemDialog1, ID_PASSWORD, wxEmptyString, wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemBoxSizer8->Add(itemTextCtrl10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer11 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer11, 0, wxGROW|wxALL, 5);
  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton12->SetDefault();
  itemStdDialogButtonSizer11->AddButton(itemButton12);

  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer11->AddButton(itemButton13);

  wxButton* itemButton14 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer11->AddButton(itemButton14);

  itemStdDialogButtonSizer11->Realize();

  // Set validators
  itemStaticText7->SetValidator( wxGenericValidator(& m_filename) );
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_password) );
////@end CSafeCombinationPrompt content construction

  wxWindow* passwdCtrl = FindWindow(ID_PASSWORD);
  if (passwdCtrl)
    passwdCtrl->SetFocus();
}


/*!
 * Should we show tooltips?
 */

bool CSafeCombinationPrompt::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSafeCombinationPrompt::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CSafeCombinationPrompt bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("../graphics/cpane.xpm"))
  {
    wxBitmap bitmap(cpane_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end CSafeCombinationPrompt bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSafeCombinationPrompt::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CSafeCombinationPrompt icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CSafeCombinationPrompt icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CSafeCombinationPrompt::OnOkClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    if (m_password.empty()) {
      wxMessageDialog err(this, _("The combination cannot be blank."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
      return;
    }
    if (!pws_os::FileExists(m_filename.c_str())) {
      wxMessageDialog err(this, _("File or path not found."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
      return;
    }
    if (m_core.CheckPassword(m_filename.c_str(),
                             m_password.c_str()) != PWScore::SUCCESS) {
      wxString errmess;
      if (m_tries >= 2) {
        errmess = _("Three strikes - yer out!");
      } else {
        m_tries++;
        errmess = _("Incorrect passkey, not a PasswordSafe database, or a corrupt database. (Backup database has same name as original, ending with '~')");
      }
      wxMessageDialog err(this, errmess,
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
      wxTextCtrl *txt = (wxTextCtrl *)FindWindow(ID_PASSWORD);
      txt->SetSelection(-1,-1);
      txt->SetFocus();
      return;
    }
    // m_core.SetReadOnly(m_readOnly);
    m_core.SetCurFile(m_filename.c_str());
    EndModal(wxID_OK);
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CSafeCombinationPrompt::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationPrompt.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CANCEL);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationPrompt. 
}

