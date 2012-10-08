/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "core/PWCharPool.h" // for CheckPassword()
#include "./wxutils.h"          // for ApplyPasswordFont
#include "./ExternalKeyboardButton.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
#include "graphics/Yubikey-button.xpm"
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
  m_YubiBtn = NULL;
  m_YubiBtn2 = NULL;
  m_yubiStatusCtrl = NULL;
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

  wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(4, 3, 0, 0);
  itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Old safe combination:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemDialog1, ID_OLDPASSWD, wxEmptyString, wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemFlexGridSizer4->Add(itemTextCtrl6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_YubiBtn = new wxBitmapButton( itemDialog1, ID_YUBIBTN, itemDialog1->GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  itemFlexGridSizer4->Add(m_YubiBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("New safe combination:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemDialog1, ID_NEWPASSWD, wxEmptyString, wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemFlexGridSizer4->Add(itemTextCtrl9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_YubiBtn2 = new wxBitmapButton( itemDialog1, ID_YUBIBTN2, itemDialog1->GetBitmapResource(wxT("graphics/Yubikey-button.xpm")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(40, 15)), wxBU_AUTODRAW );
  itemFlexGridSizer4->Add(m_YubiBtn2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxSHAPED, 5);

  wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Confirmation:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemDialog1, ID_CONFIRM, wxEmptyString, wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), wxTE_PASSWORD );
  itemFlexGridSizer4->Add(itemTextCtrl12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer4->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer4->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_yubiStatusCtrl = new wxStaticText( itemDialog1, ID_YUBISTATUS, _("Please insert your YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer4->Add(m_yubiStatusCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer4->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer17 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer17, 0, wxGROW|wxALL, 5);
  wxButton* itemButton18 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton18->SetDefault();
  itemStdDialogButtonSizer17->AddButton(itemButton18);

  wxButton* itemButton19 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer17->AddButton(itemButton19);

  wxButton* itemButton20 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer17->AddButton(itemButton20);

  itemStdDialogButtonSizer17->Realize();

  // Set validators
  itemTextCtrl6->SetValidator( wxGenericValidator(& m_oldpasswd) );
  itemTextCtrl9->SetValidator( wxGenericValidator(& m_newpasswd) );
  itemTextCtrl12->SetValidator( wxGenericValidator(& m_confirm) );
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
  if (name == _T("graphics/Yubikey-button.xpm"))
  {
    wxBitmap bitmap(Yubikey_button_xpm);
    return bitmap;
  }
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

void CSafeCombinationChange::OnOkClick( wxCommandEvent& /* evt */ )
{
  if (Validate() && TransferDataFromWindow()) {
    StringX errmess;
    int rc = m_core.CheckPasskey(m_core.GetCurFile(), tostringx(m_oldpasswd));
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
    } else if (!CPasswordCharPool::CheckPassword(tostringx(m_newpasswd), errmess)) {
      wxString msg = _("Weak passphrase:\n\n");
      msg += errmess.c_str();
#ifndef PWS_FORCE_STRONG_PASSPHRASE
      msg += _("\nUse it anyway?");
      wxMessageDialog err(this, msg,
                          _("Error"), wxYES_NO | wxICON_HAND);
      int rc1 = err.ShowModal();
      if (rc1 == wxID_YES)
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

void CSafeCombinationChange::OnCancelClick( wxCommandEvent& /* evt */ )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationChange.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CANCEL);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CSafeCombinationChange. 
}

