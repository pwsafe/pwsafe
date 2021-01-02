/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PasswordSubsetDlg.cpp
* 
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <sstream>
#include <wx/regex.h>
#include "PasswordSubsetDlg.h"
#include "Clipboard.h"

////@begin XPM images
#include "graphics/toolbar/new/copypassword.xpm"
#include "graphics/toolbar/new/copypassword_disabled.xpm"
////@end XPM images

/*!
 * PasswordSubsetDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( PasswordSubsetDlg, wxDialog )

/*!
 * PasswordSubsetDlg event table definition
 */

BEGIN_EVENT_TABLE( PasswordSubsetDlg, wxDialog )

////@begin PasswordSubsetDlg event table entries
  EVT_BUTTON( ID_BITMAPBUTTON, PasswordSubsetDlg::OnBitmapbuttonClick )
  EVT_BUTTON( wxID_CLOSE, PasswordSubsetDlg::OnCloseClick )
////@end PasswordSubsetDlg event table entries

END_EVENT_TABLE()

/*!
 * PasswordSubsetDlg constructors
 */

PasswordSubsetDlg::PasswordSubsetDlg()
: m_password(wxEmptyString)
{
  Init();
}

PasswordSubsetDlg::PasswordSubsetDlg( wxWindow* parent, const StringX &password,
                                  wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : m_password(password)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * PasswordSubsetDlg creator
 */

bool PasswordSubsetDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin PasswordSubsetDlg creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end PasswordSubsetDlg creation
  return true;
}

/*!
 * PasswordSubsetDlg destructor
 */

PasswordSubsetDlg::~PasswordSubsetDlg()
{
////@begin PasswordSubsetDlg destruction
////@end PasswordSubsetDlg destruction
}

/*!
 * Member initialisation
 */

void PasswordSubsetDlg::Init()
{
////@begin PasswordSubsetDlg member initialisation
  m_pos = nullptr;
  m_vals = nullptr;
  m_error = nullptr;
////@end PasswordSubsetDlg member initialisation
}

/*!
 * Control creation for PasswordSubsetDlg
 */

void PasswordSubsetDlg::CreateControls()
{    
////@begin PasswordSubsetDlg content construction
  PasswordSubsetDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Enter positions of password characters separated by spaces, commas or semi-colons.\nPosition 1 is the first character, 2 is the second, etc. up to N for the last character of a password of length N.\n-1 is the last character, -2 the next to last, etc. up to -N for the first password character."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxGridSizer* itemGridSizer4 = new wxGridSizer(2, 3, 0, 0);
  itemBoxSizer2->Add(itemGridSizer4, 0, wxEXPAND|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Positions:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_pos = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_POS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(m_pos, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemGridSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Values:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_vals = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_VAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  itemGridSizer4->Add(m_vals, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton10 = new wxBitmapButton( itemDialog1, ID_BITMAPBUTTON, itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  wxBitmap itemBitmapButton10BitmapSel(itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword.xpm")));
  itemBitmapButton10->SetBitmapSelected(itemBitmapButton10BitmapSel);
  wxBitmap itemBitmapButton10BitmapFocus(itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword.xpm")));
  itemBitmapButton10->SetBitmapFocus(itemBitmapButton10BitmapFocus);
  wxBitmap itemBitmapButton10BitmapDisabled(itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword_disabled.xpm")));
  itemBitmapButton10->SetBitmapDisabled(itemBitmapButton10BitmapDisabled);
  wxBitmap itemBitmapButton10BitmapHover(itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword.xpm")));
  itemBitmapButton10->SetBitmapHover(itemBitmapButton10BitmapHover);
  if (PasswordSubsetDlg::ShowToolTips())
    itemBitmapButton10->SetToolTip(_("Copy values"));
  itemGridSizer4->Add(itemBitmapButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_error = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_error->SetForegroundColour(wxColour(255, 0, 0));
  itemBoxSizer2->Add(m_error, 0, wxEXPAND|wxALL, 5);

  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // Connect events and objects
  m_pos->Connect(ID_TEXTCTRL_POS, wxEVT_CHAR, wxKeyEventHandler(PasswordSubsetDlg::OnChar), nullptr, this);
////@end PasswordSubsetDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool PasswordSubsetDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap PasswordSubsetDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin PasswordSubsetDlg bitmap retrieval
  wxUnusedVar(name);
  if (name == wxT("graphics/toolbar/new/copypassword.xpm"))
  {
    wxBitmap bitmap(copypassword_xpm);
    return bitmap;
  }
  else if (name == wxT("graphics/toolbar/new/copypassword_disabled.xpm"))
  {
    wxBitmap bitmap(copypassword_disabled_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end PasswordSubsetDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon PasswordSubsetDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin PasswordSubsetDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end PasswordSubsetDlg icon retrieval
}

/*!
 * wxEVT_CHAR event handler for ID_TEXTCTRL
 */

void PasswordSubsetDlg::OnChar( wxKeyEvent& event )
{
  static wxRegEx charSet("[[:digit:]]|[[:space:]]|,|-|;");
  static wxRegEx seps(",|;");

  wxASSERT(charSet.IsValid() && seps.IsValid());

  wxChar uc = event.GetUnicodeKey();
  if (uc != WXK_NONE) {
    const int N = static_cast<int>(m_password.length());
    if (charSet.Matches(wxString(uc, 1))) {
      event.Skip(true); // accept
      // Check against valid pos regexp and update vals accordingly
      wxString pos_str = m_pos->GetLineText(0);
      pos_str += uc; // since accepted char will only be added to control later
      // could have used xwStringTokenizer in following, but this way we also convert to int
      // and catch bad usage of '-'
      seps.Replace(&pos_str, wxT(" ")); // replace ';' and ',' with ' ' for stream tokenizing
      m_vals->Clear();
      m_error->SetLabel(wxEmptyString);
      
      std::wistringstream is(static_cast<const wchar_t*>(pos_str.wc_str()));
      int pos;
      while (is >> pos) {
	if (pos > 0 && pos <= N)
	  *m_vals << m_password[pos - 1] << wxT(" ");
	else if (pos < 0 && pos >= -N)
	  *m_vals << m_password[N + pos] << wxT(" ");
	else {
	  m_error->SetLabel(_("Invalid position"));
	}
      }
      if (!is.eof()) {
	m_error->SetLabel(_("Invalid position"));
      }
    } else {
      if (uc == WXK_BACK)
	event.Skip(true); // handle backspace
      else
	event.Skip(false); // not a char that we want to accept
    }
  } else { // process non-char key as usual
    event.Skip(true);
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
 */

void PasswordSubsetDlg::OnBitmapbuttonClick( wxCommandEvent& event )
{
  wxUnusedVar(event);
  wxString val_str = m_vals->GetLineText(0);
  wxString reduced_str;
  // we always put the char at the specified position plus a space.
  // so here we undo that.
  if (!val_str.empty()) {
    wxASSERT(val_str.length() % 2 == 0);
    for (size_t i = 0; i < val_str.length(); i += 2)
      reduced_str += val_str[i];
    Clipboard::GetInstance()->SetData(static_cast<const wchar_t*>(reduced_str.wc_str()));
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void PasswordSubsetDlg::OnCloseClick( wxCommandEvent& event )
{
  wxUnusedVar(event);
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in PasswordSubsetDlg.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CLOSE);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in PasswordSubsetDlg.
}
