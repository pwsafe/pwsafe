/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file yubicfg.cpp
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

#include <wx/timer.h>
#include "yubicfg.h"

////@begin XPM images
////@end XPM images


/*!
 * YubiCfgDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( YubiCfgDlg, wxDialog )


/*!
 * YubiCfgDlg event table definition
 */

BEGIN_EVENT_TABLE( YubiCfgDlg, wxDialog )

////@begin YubiCfgDlg event table entries
  EVT_BUTTON( ID_YK_HIDESHOW, YubiCfgDlg::OnYkHideshowClick )

  EVT_BUTTON( ID_YK_GENERATE, YubiCfgDlg::OnYkGenerateClick )

  EVT_BUTTON( ID_YK_SET, YubiCfgDlg::OnYkSetClick )

////@end YubiCfgDlg event table entries
EVT_TIMER(POLLING_TIMER_ID, YubiCfgDlg::OnPollingTimer)
END_EVENT_TABLE()


/*!
 * YubiCfgDlg constructors
 */

YubiCfgDlg::YubiCfgDlg()
{
  Init();
}

YubiCfgDlg::YubiCfgDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * YubiCfgDlg creator
 */

bool YubiCfgDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin YubiCfgDlg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end YubiCfgDlg creation
  return true;
}


/*!
 * YubiCfgDlg destructor
 */

YubiCfgDlg::~YubiCfgDlg()
{
////@begin YubiCfgDlg destruction
////@end YubiCfgDlg destruction
  delete m_pollingTimer;
}


/*!
 * Member initialisation
 */

void YubiCfgDlg::Init()
{
////@begin YubiCfgDlg member initialisation
////@end YubiCfgDlg member initialisation
  m_pollingTimer = new wxTimer(this, POLLING_TIMER_ID);
}


/*!
 * Control creation for YubiCfgDlg
 */

void YubiCfgDlg::CreateControls()
{    
////@begin YubiCfgDlg content construction
  YubiCfgDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, 0, 0);

  wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("YubiKey Serial Number: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemDialog1, ID_YK_SERNUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  itemBoxSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("YubiKey Secret Key (20 Byte Hex)"));
  wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer6, 0, wxGROW|wxALL, 5);

  wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemDialog1, ID_YKSK, _("Please insert your YubiKey"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  itemStaticBoxSizer6->Add(itemTextCtrl7, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer6->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, ID_YK_HIDESHOW, _("Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, ID_YK_GENERATE, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_YK_SET, _("Set YubiKey"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer12 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer12->AddButton(itemButton13);

  wxButton* itemButton14 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer12->AddButton(itemButton14);

  itemStdDialogButtonSizer12->Realize();

  // Set validators
  itemTextCtrl5->SetValidator( wxGenericValidator(& m_yksernum) );
  itemTextCtrl7->SetValidator( wxGenericValidator(& m_yksk) );
////@end YubiCfgDlg content construction
  m_pollingTimer->Start(250); // check for Yubikey every 250ms.
}


/*!
 * Should we show tooltips?
 */

bool YubiCfgDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap YubiCfgDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin YubiCfgDlg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end YubiCfgDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon YubiCfgDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin YubiCfgDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end YubiCfgDlg icon retrieval
}

void YubiCfgDlg::OnPollingTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == POLLING_TIMER_ID) {
    // TBD
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_HIDESHOW
 */

void YubiCfgDlg::OnYkHideshowClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_HIDESHOW in YubiCfgDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_HIDESHOW in YubiCfgDlg. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_GENERATE
 */

void YubiCfgDlg::OnYkGenerateClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_GENERATE in YubiCfgDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_GENERATE in YubiCfgDlg. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET
 */

void YubiCfgDlg::OnYkSetClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET in YubiCfgDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_YK_SET in YubiCfgDlg. 
}

