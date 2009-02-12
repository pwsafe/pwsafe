/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file safecombinationchange.cpp
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

#include "safecombinationchange.h"
#include "corelib/PWCharPool.h" // for CheckPassword()

////@begin XPM images
////@end XPM images


/*!
 * CSafeCombinationChange type definition
 */

IMPLEMENT_CLASS( CSafeCombinationChange, wxDialog )


/*!
 * CSafeCombinationChange event table definition
 */

BEGIN_EVENT_TABLE( CSafeCombinationChange, wxDialog )

////@begin CSafeCombinationChange event table entries
  EVT_BUTTON( wxID_OK, CSafeCombinationChange::OnOkClick )

  EVT_BUTTON( wxID_CANCEL, CSafeCombinationChange::OnCancelClick )

////@end CSafeCombinationChange event table entries

END_EVENT_TABLE()


/*!
 * CSafeCombinationChange constructors
 */


CSafeCombinationChange::CSafeCombinationChange(wxWindow* parent, PWScore &core,
                                               wxWindowID id, const wxString& caption,
                                               const wxPoint& pos,
                                               const wxSize& size, long style)
: m_core(core)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * CSafeCombinationChange creator
 */

bool CSafeCombinationChange::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSafeCombinationChange creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CSafeCombinationChange creation
  return true;
}


/*!
 * CSafeCombinationChange destructor
 */

CSafeCombinationChange::~CSafeCombinationChange()
{
////@begin CSafeCombinationChange destruction
////@end CSafeCombinationChange destruction
}


/*!
 * Member initialisation
 */

void CSafeCombinationChange::Init()
{
////@begin CSafeCombinationChange member initialisation
////@end CSafeCombinationChange member initialisation
}


/*!
 * Control creation for CSafeCombinationChange
 */

void CSafeCombinationChange::CreateControls()
{    
////@begin CSafeCombinationChange content construction
  CSafeCombinationChange* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please enter the current combination, followed by a new combination.\nType the new combination once again to confirm it."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

  wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(3, 2, 0, 0);
  itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Old safe combination:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemDialog1, ID_OLDPASSWD, _T(""), wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemFlexGridSizer4->Add(itemTextCtrl6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  itemTextCtrl6->SetFocus();

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("New safe combination:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemDialog1, ID_NEWPASSWD, _T(""), wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemFlexGridSizer4->Add(itemTextCtrl8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Confirmation:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemDialog1, ID_CONFIRM, _T(""), wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemFlexGridSizer4->Add(itemTextCtrl10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

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
  itemTextCtrl6->SetValidator( wxGenericValidator(& m_oldpasswd) );
  itemTextCtrl8->SetValidator( wxGenericValidator(& m_newpasswd) );
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_confirm) );
////@end CSafeCombinationChange content construction
}


/*!
 * Should we show tooltips?
 */

bool CSafeCombinationChange::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSafeCombinationChange::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CSafeCombinationChange bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end CSafeCombinationChange bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSafeCombinationChange::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CSafeCombinationChange icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CSafeCombinationChange icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CSafeCombinationChange::OnOkClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    StringX errmess;
    int rc = m_core.CheckPassword(m_core.GetCurFile(), m_oldpasswd.c_str());
    if (rc == PWScore::WRONG_PASSWORD) {
      wxMessageDialog err(this, _("The old safe combination is not correct"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (rc == PWScore::CANT_OPEN_FILE) {
      wxMessageDialog err(this, _("Cannot verify old safe combination - file gone?"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (m_confirm != m_newpasswd) {
      wxMessageDialog err(this, _("New safe combination and confirmation do not match"),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    } else if (m_newpasswd.empty()) {
      wxMessageDialog err(this, _("The combination cannot be blank."),
                          _("Error"), wxOK | wxICON_EXCLAMATION);
      err.ShowModal();
    // Vox populi vox dei - folks want the ability to use a weak
    // passphrase, best we can do is warn them...
    // If someone want to build a version that insists on proper
    // passphrases, then just define the preprocessor macro
    // PWS_FORCE_STRONG_PASSPHRASE in the build properties/Makefile
    // (also used in CPasskeySetup)
    } else if (!CPasswordCharPool::CheckPassword(m_newpasswd.c_str(), errmess)) {
      wxString msg = _("Weak passphrase:\n\n");
      msg += errmess.c_str();
#ifndef PWS_FORCE_STRONG_PASSPHRASE
      msg += _("\nUse it anyway?");
      wxMessageDialog err(this, msg,
                          _("Error"), wxYES_NO | wxICON_HAND);
      int rc = err.ShowModal();
      if (rc == wxID_YES)
        EndModal(wxID_OK);
#else
      wxMessageDialog err(this, msg,
                          _("Error"), wxOK | wxICON_HAND);
      err.ShowModal();
#endif // PWS_FORCE_STRONG_PASSPHRASE
    } else { // password checks out OK.
      EndModal(wxID_OK);
    }
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CSafeCombinationChange::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationChange.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CANCEL);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationChange. 
}

