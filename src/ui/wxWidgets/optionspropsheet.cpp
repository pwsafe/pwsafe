/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file optionspropsheet.cpp
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
#include "wx/bookctrl.h"
////@end includes

#include "optionspropsheet.h"

////@begin XPM images
////@end XPM images


/*!
 * COptions type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COptions, wxPropertySheetDialog )


/*!
 * COptions event table definition
 */

BEGIN_EVENT_TABLE( COptions, wxPropertySheetDialog )

////@begin COptions event table entries
////@end COptions event table entries

END_EVENT_TABLE()


/*!
 * COptions constructors
 */

COptions::COptions()
{
  Init();
}

COptions::COptions( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * COptions creator
 */

bool COptions::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COptions creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  LayoutDialog();
  Centre();
////@end COptions creation
  return true;
}


/*!
 * COptions destructor
 */

COptions::~COptions()
{
////@begin COptions destruction
////@end COptions destruction
}


/*!
 * Member initialisation
 */

void COptions::Init()
{
////@begin COptions member initialisation
////@end COptions member initialisation
}


/*!
 * Control creation for COptions
 */

void COptions::CreateControls()
{    
////@begin COptions content construction
  COptions* itemPropertySheetDialog1 = this;

  wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxCheckBox* itemCheckBox4 = new wxCheckBox( itemPanel2, ID_CHECKBOX10, _("Save database immediately after Edit or Add"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox4->SetValue(false);
  itemBoxSizer3->Add(itemCheckBox4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Intermediate Backups"));
  wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
  itemBoxSizer3->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel2, ID_CHECKBOX11, _("Create intermediate backups (.ibak) before saving"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox6->SetValue(false);
  itemStaticBoxSizer5->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Backup Name"));
  wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
  itemStaticBoxSizer5->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText8 = new wxStaticText( itemPanel2, wxID_STATIC, _("Base:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton9 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON4, _("Database name"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton9->SetValue(false);
  itemStaticBoxSizer7->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);
  wxRadioButton* itemRadioButton11 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON5, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton11->SetValue(false);
  itemBoxSizer10->Add(itemRadioButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer10->Add(itemTextCtrl12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticLine* itemStaticLine13 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer7->Add(itemStaticLine13, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemPanel2, wxID_STATIC, _("Suffix:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer7->Add(itemBoxSizer15, 0, wxGROW|wxALL, 0);
  wxArrayString itemComboBox16Strings;
  wxComboBox* itemComboBox16 = new wxComboBox( itemPanel2, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox16Strings, wxCB_DROPDOWN );
  itemBoxSizer15->Add(itemComboBox16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText( itemPanel2, wxID_STATIC, _("Max."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl18 = new wxSpinCtrl( itemPanel2, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer15->Add(itemSpinCtrl18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText( itemPanel2, wxID_STATIC, _("Example:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer7->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticLine* itemStaticLine20 = new wxStaticLine( itemPanel2, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer5->Add(itemStaticLine20, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText21 = new wxStaticText( itemPanel2, wxID_STATIC, _("Backup directory:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer5->Add(itemStaticText21, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton22 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON6, _("Same as database's"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton22->SetValue(false);
  itemStaticBoxSizer5->Add(itemRadioButton22, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer5->Add(itemBoxSizer23, 0, wxGROW|wxALL, 0);
  wxRadioButton* itemRadioButton24 = new wxRadioButton( itemPanel2, ID_RADIOBUTTON7, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton24->SetValue(false);
  itemBoxSizer23->Add(itemRadioButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl25 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(itemPanel2->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer23->Add(itemTextCtrl25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton26 = new wxButton( itemPanel2, ID_BUTTON, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer23->Add(itemButton26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  GetBookCtrl()->AddPage(itemPanel2, _("Backups"));

  wxPanel* itemPanel27 = new wxPanel( GetBookCtrl(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
  itemPanel27->SetSizer(itemBoxSizer28);

  wxCheckBox* itemCheckBox29 = new wxCheckBox( itemPanel27, ID_CHECKBOX12, _("Always keep Password Safe on top"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox29->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox29, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox30 = new wxCheckBox( itemPanel27, ID_CHECKBOX13, _("Show Username in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox30->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox30, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox31 = new wxCheckBox( itemPanel27, ID_CHECKBOX14, _("Show Password in Tree View"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox31->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox31, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox32 = new wxCheckBox( itemPanel27, ID_CHECKBOX15, _("Show Notes as ToolTips in Tree && List views"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox32->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox32, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox33 = new wxCheckBox( itemPanel27, ID_CHECKBOX16, _("Show Password in Add & Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox33->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox33, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox34 = new wxCheckBox( itemPanel27, ID_CHECKBOX17, _("Show Notes in Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox34->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox34, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox35 = new wxCheckBox( itemPanel27, ID_CHECKBOX18, _("Word Wrap Notes in Add && Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox35->SetValue(false);
  itemBoxSizer28->Add(itemCheckBox35, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer28->Add(itemBoxSizer36, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox37 = new wxCheckBox( itemPanel27, ID_CHECKBOX19, _("Warn"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox37->SetValue(false);
  itemBoxSizer36->Add(itemCheckBox37, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl38 = new wxSpinCtrl( itemPanel27, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxSize(itemPanel27->ConvertDialogToPixels(wxSize(25, -1)).x, -1), wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer36->Add(itemSpinCtrl38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText39 = new wxStaticText( itemPanel27, wxID_STATIC, _("days before passwords expire"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer36->Add(itemStaticText39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemRadioBox40Strings;
  itemRadioBox40Strings.Add(_("&Fully collapsed"));
  itemRadioBox40Strings.Add(_("&Fully expanded"));
  itemRadioBox40Strings.Add(_("&Same as when last saved"));
  wxRadioBox* itemRadioBox40 = new wxRadioBox( itemPanel27, ID_RADIOBOX, _("Initial Tree View"), wxDefaultPosition, wxDefaultSize, itemRadioBox40Strings, 1, wxRA_SPECIFY_COLS );
  itemRadioBox40->SetSelection(0);
  itemBoxSizer28->Add(itemRadioBox40, 0, wxGROW|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel27, _("Display"));

  wxPanel* itemPanel41 = new wxPanel( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxVERTICAL);
  itemPanel41->SetSizer(itemBoxSizer42);

  wxCheckBox* itemCheckBox43 = new wxCheckBox( itemPanel41, ID_CHECKBOX20, _("Confirm deletion of items"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox43->SetValue(false);
  itemBoxSizer42->Add(itemCheckBox43, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox44 = new wxCheckBox( itemPanel41, ID_CHECKBOX21, _("Record last access times"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox44->SetValue(false);
  itemBoxSizer42->Add(itemCheckBox44, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox45 = new wxCheckBox( itemPanel41, ID_CHECKBOX22, _("Escape key closes application"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox45->SetValue(false);
  itemBoxSizer42->Add(itemCheckBox45, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer46 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer42->Add(itemBoxSizer46, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText47 = new wxStaticText( itemPanel41, wxID_STATIC, _("Double-click action"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer46->Add(itemStaticText47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemComboBox48Strings;
  wxComboBox* itemComboBox48 = new wxComboBox( itemPanel41, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox48Strings, wxCB_DROPDOWN );
  itemBoxSizer46->Add(itemComboBox48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer49Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Autotype"));
  wxStaticBoxSizer* itemStaticBoxSizer49 = new wxStaticBoxSizer(itemStaticBoxSizer49Static, wxVERTICAL);
  itemBoxSizer42->Add(itemStaticBoxSizer49, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox50 = new wxCheckBox( itemPanel41, ID_CHECKBOX23, _("Minimize after Autotype"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox50->SetValue(false);
  itemStaticBoxSizer49->Add(itemCheckBox50, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer51 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer49->Add(itemBoxSizer51, 0, wxGROW|wxALL, 0);
  wxStaticText* itemStaticText52 = new wxStaticText( itemPanel41, wxID_STATIC, _("Default Autotype string:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer51->Add(itemStaticText52, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl53 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer51->Add(itemTextCtrl53, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer54Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Default Username"));
  wxStaticBoxSizer* itemStaticBoxSizer54 = new wxStaticBoxSizer(itemStaticBoxSizer54Static, wxVERTICAL);
  itemBoxSizer42->Add(itemStaticBoxSizer54, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer55 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer54->Add(itemBoxSizer55, 0, wxGROW|wxALL, 0);
  wxCheckBox* itemCheckBox56 = new wxCheckBox( itemPanel41, ID_CHECKBOX24, _("Use"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox56->SetValue(false);
  itemBoxSizer55->Add(itemCheckBox56, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl57 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(90, -1)).x, -1), 0 );
  itemBoxSizer55->Add(itemTextCtrl57, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText58 = new wxStaticText( itemPanel41, wxID_STATIC, _("as default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer55->Add(itemStaticText58, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox59 = new wxCheckBox( itemPanel41, ID_CHECKBOX25, _("Query user to set default username"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox59->SetValue(false);
  itemStaticBoxSizer54->Add(itemCheckBox59, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer60Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Alternate Browser"));
  wxStaticBoxSizer* itemStaticBoxSizer60 = new wxStaticBoxSizer(itemStaticBoxSizer60Static, wxVERTICAL);
  itemBoxSizer42->Add(itemStaticBoxSizer60, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer61 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer60->Add(itemBoxSizer61, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl62 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(120, -1)).x, -1), 0 );
  itemBoxSizer61->Add(itemTextCtrl62, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton63 = new wxButton( itemPanel41, ID_BUTTON8, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer61->Add(itemButton63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer60->Add(itemBoxSizer64, 0, wxGROW|wxALL, 0);
  wxTextCtrl* itemTextCtrl65 = new wxTextCtrl( itemPanel41, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(itemPanel41->ConvertDialogToPixels(wxSize(60, -1)).x, -1), 0 );
  itemBoxSizer64->Add(itemTextCtrl65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText66 = new wxStaticText( itemPanel41, wxID_STATIC, _("Browser Command Line parameters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer64->Add(itemStaticText66, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel41, _("Misc."));

  wxPanel* itemPanel67 = new wxPanel( GetBookCtrl(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel67, _("Password Policy"));

  wxPanel* itemPanel68 = new wxPanel( GetBookCtrl(), ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel68, _("Password History"));

  wxPanel* itemPanel69 = new wxPanel( GetBookCtrl(), ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel69, _("Security"));

  wxPanel* itemPanel70 = new wxPanel( GetBookCtrl(), ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel70, _("System"));

  wxPanel* itemPanel71 = new wxPanel( GetBookCtrl(), ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  GetBookCtrl()->AddPage(itemPanel71, _("Shortcuts"));

////@end COptions content construction
}


/*!
 * Should we show tooltips?
 */

bool COptions::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap COptions::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin COptions bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end COptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon COptions::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin COptions icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end COptions icon retrieval
}
