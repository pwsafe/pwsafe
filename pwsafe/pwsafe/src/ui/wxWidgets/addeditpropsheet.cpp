/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file addeditpropsheet.cpp
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

#include "addeditpropsheet.h"

////@begin XPM images
////@end XPM images


/*!
 * AddEditPropSheet type definition
 */

IMPLEMENT_DYNAMIC_CLASS( AddEditPropSheet, wxPropertySheetDialog )


/*!
 * AddEditPropSheet event table definition
 */

BEGIN_EVENT_TABLE( AddEditPropSheet, wxPropertySheetDialog )

////@begin AddEditPropSheet event table entries
////@end AddEditPropSheet event table entries

END_EVENT_TABLE()


/*!
 * AddEditPropSheet constructors
 */

AddEditPropSheet::AddEditPropSheet()
{
  Init();
}

AddEditPropSheet::AddEditPropSheet( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * AddEditPropSheet creator
 */

bool AddEditPropSheet::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin AddEditPropSheet creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  LayoutDialog();
  Centre();
////@end AddEditPropSheet creation
  return true;
}


/*!
 * AddEditPropSheet destructor
 */

AddEditPropSheet::~AddEditPropSheet()
{
////@begin AddEditPropSheet destruction
////@end AddEditPropSheet destruction
}


/*!
 * Member initialisation
 */

void AddEditPropSheet::Init()
{
////@begin AddEditPropSheet member initialisation
////@end AddEditPropSheet member initialisation
}


/*!
 * Control creation for AddEditPropSheet
 */

void AddEditPropSheet::CreateControls()
{    
////@begin AddEditPropSheet content construction
  AddEditPropSheet* itemPropertySheetDialog1 = this;

  wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), ID_PANEL_BASIC, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxStaticText* itemStaticText4 = new wxStaticText( itemPanel2, wxID_STATIC, _("To add a new entry, simply fill in the fields below. At least a title and a\npassword are required. If you have set a default username, it will appear in the\nusername field."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
  itemBoxSizer3->Add(itemFlexGridSizer5, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText6 = new wxStaticText( itemPanel2, wxID_STATIC, _("Group:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemComboBox7Strings;
  wxComboBox* itemComboBox7 = new wxComboBox( itemPanel2, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox7Strings, wxCB_DROPDOWN );
  itemFlexGridSizer5->Add(itemComboBox7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer5->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemPanel2, wxID_STATIC, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemTextCtrl10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer5->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText12 = new wxStaticText( itemPanel2, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemTextCtrl13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer5->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText15 = new wxStaticText( itemPanel2, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl16 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemTextCtrl16, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer5->Add(itemBoxSizer17, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  itemBoxSizer17->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton19 = new wxButton( itemPanel2, ID_BUTTON2, _("&Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  itemBoxSizer17->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton21 = new wxButton( itemPanel2, ID_BUTTON3, _("&Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText22 = new wxStaticText( itemPanel2, wxID_STATIC, _("Confirm:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl23 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
  itemFlexGridSizer5->Add(itemTextCtrl23, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer5->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText25 = new wxStaticText( itemPanel2, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl26 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemTextCtrl26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer5->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  itemBoxSizer27->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton29 = new wxButton( itemPanel2, ID_BUTTON, _("Go"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemButton29, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer30, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText31 = new wxStaticText( itemPanel2, wxID_STATIC, _("Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(itemStaticText31, 1, wxALIGN_TOP|wxALL, 5);

  wxTextCtrl* itemTextCtrl32 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
  itemBoxSizer30->Add(itemTextCtrl32, 5, wxALIGN_CENTER_VERTICAL|wxALL, 3);

  GetBookCtrl()->AddPage(itemPanel2, _("Basic"));

  wxPanel* itemPanel33 = new wxPanel( GetBookCtrl(), ID_PANEL_ADDITIONAL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxVERTICAL);
  itemPanel33->SetSizer(itemBoxSizer34);

  wxFlexGridSizer* itemFlexGridSizer35 = new wxFlexGridSizer(0, 2, 0, 0);
  itemBoxSizer34->Add(itemFlexGridSizer35, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText36 = new wxStaticText( itemPanel33, wxID_STATIC, _("Autotype:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer35->Add(itemStaticText36, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl37 = new wxTextCtrl( itemPanel33, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer35->Add(itemTextCtrl37, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText38 = new wxStaticText( itemPanel33, wxID_STATIC, _("Run Cmd:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer35->Add(itemStaticText38, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl39 = new wxTextCtrl( itemPanel33, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer35->Add(itemTextCtrl39, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText40 = new wxStaticText( itemPanel33, wxID_STATIC, _("Double-Click\nAction:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer35->Add(itemStaticText40, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer35->Add(itemBoxSizer41, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxCheckBox* itemCheckBox42 = new wxCheckBox( itemPanel33, ID_CHECKBOX, _("Use Default"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox42->SetValue(false);
  itemBoxSizer41->Add(itemCheckBox42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString itemComboBox43Strings;
  wxComboBox* itemComboBox43 = new wxComboBox( itemPanel33, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox43Strings, wxCB_DROPDOWN );
  itemBoxSizer41->Add(itemComboBox43, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer44Static = new wxStaticBox(itemPanel33, wxID_ANY, _("Password History"));
  wxStaticBoxSizer* itemStaticBoxSizer44 = new wxStaticBoxSizer(itemStaticBoxSizer44Static, wxVERTICAL);
  itemBoxSizer34->Add(itemStaticBoxSizer44, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer44->Add(itemBoxSizer45, 0, wxGROW|wxALL, 5);
  wxCheckBox* itemCheckBox46 = new wxCheckBox( itemPanel33, ID_CHECKBOX1, _("Keep"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox46->SetValue(false);
  itemBoxSizer45->Add(itemCheckBox46, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl47 = new wxSpinCtrl( itemPanel33, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer45->Add(itemSpinCtrl47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText48 = new wxStaticText( itemPanel33, wxID_STATIC, _("last passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer45->Add(itemStaticText48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxGrid* itemGrid49 = new wxGrid( itemPanel33, ID_GRID, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemGrid49->SetDefaultColSize(150);
  itemGrid49->SetDefaultRowSize(25);
  itemGrid49->SetColLabelSize(25);
  itemGrid49->SetRowLabelSize(0);
  itemGrid49->CreateGrid(5, 2, wxGrid::wxGridSelectCells);
  itemStaticBoxSizer44->Add(itemGrid49, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer44->Add(itemBoxSizer50, 0, wxGROW|wxALL, 5);
  wxButton* itemButton51 = new wxButton( itemPanel33, ID_BUTTON1, _("Clear History"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(itemButton51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer50->Add(10, 10, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton53 = new wxButton( itemPanel33, ID_BUTTON4, _("Copy All"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(itemButton53, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel33, _("Additional"));

  wxPanel* itemPanel54 = new wxPanel( GetBookCtrl(), ID_PANEL_DTIME, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer55 = new wxBoxSizer(wxVERTICAL);
  itemPanel54->SetSizer(itemBoxSizer55);

  wxStaticBox* itemStaticBoxSizer56Static = new wxStaticBox(itemPanel54, wxID_ANY, _("Password Expiry"));
  wxStaticBoxSizer* itemStaticBoxSizer56 = new wxStaticBoxSizer(itemStaticBoxSizer56Static, wxVERTICAL);
  itemBoxSizer55->Add(itemStaticBoxSizer56, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
  itemStaticBoxSizer56->Add(itemBoxSizer57, 0, wxGROW|wxALL, 5);
  wxBoxSizer* itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer57->Add(itemBoxSizer58, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText59 = new wxStaticText( itemPanel54, wxID_STATIC, _("Password Expires on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemStaticText59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText60 = new wxStaticText( itemPanel54, wxID_STATIC, _("Whenever"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemStaticText60, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxFlexGridSizer* itemFlexGridSizer61 = new wxFlexGridSizer(0, 3, 0, 0);
  itemBoxSizer57->Add(itemFlexGridSizer61, 0, wxGROW|wxALL, 5);
  wxRadioButton* itemRadioButton62 = new wxRadioButton( itemPanel54, ID_RADIOBUTTON, _("On"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton62->SetValue(false);
  itemFlexGridSizer61->Add(itemRadioButton62, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxDatePickerCtrl* itemDatePickerCtrl63 = new wxDatePickerCtrl( itemPanel54, ID_DATECTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
  itemFlexGridSizer61->Add(itemDatePickerCtrl63, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl64 = new wxSpinCtrl( itemPanel54, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  itemFlexGridSizer61->Add(itemSpinCtrl64, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxRadioButton* itemRadioButton65 = new wxRadioButton( itemPanel54, ID_RADIOBUTTON1, _("In"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton65->SetValue(false);
  itemFlexGridSizer61->Add(itemRadioButton65, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer61->Add(itemBoxSizer66, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxSpinCtrl* itemSpinCtrl67 = new wxSpinCtrl( itemPanel54, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
  itemBoxSizer66->Add(itemSpinCtrl67, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText68 = new wxStaticText( itemPanel54, wxID_STATIC, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer66->Add(itemStaticText68, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox69 = new wxCheckBox( itemPanel54, ID_CHECKBOX2, _("Recurring"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox69->SetValue(false);
  itemFlexGridSizer61->Add(itemCheckBox69, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer57->Add(itemBoxSizer70, 0, wxGROW|wxALL, 5);
  wxStaticText* itemStaticText71 = new wxStaticText( itemPanel54, wxID_STATIC, _("Current Value:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer70->Add(itemStaticText71, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText72 = new wxStaticText( itemPanel54, wxID_STATIC, _("Never"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer70->Add(itemStaticText72, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer73 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer57->Add(itemBoxSizer73, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton74 = new wxButton( itemPanel54, ID_BUTTON5, _("&Set"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer73->Add(itemButton74, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer73->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton76 = new wxButton( itemPanel54, ID_BUTTON6, _("&Clear"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer73->Add(itemButton76, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer77Static = new wxStaticBox(itemPanel54, wxID_ANY, _("Statistics"));
  wxStaticBoxSizer* itemStaticBoxSizer77 = new wxStaticBoxSizer(itemStaticBoxSizer77Static, wxVERTICAL);
  itemBoxSizer55->Add(itemStaticBoxSizer77, 0, wxGROW|wxALL, 5);
  wxFlexGridSizer* itemFlexGridSizer78 = new wxFlexGridSizer(0, 2, 0, 0);
  itemStaticBoxSizer77->Add(itemFlexGridSizer78, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText79 = new wxStaticText( itemPanel54, wxID_STATIC, _("Created on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText79, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText80 = new wxStaticText( itemPanel54, wxID_STATIC, _("10/06/2009 23:19:25"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText80, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText81 = new wxStaticText( itemPanel54, wxID_STATIC, _("Password last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText81, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText82 = new wxStaticText( itemPanel54, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText82, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText83 = new wxStaticText( itemPanel54, wxID_STATIC, _("Last accessed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText83, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText84 = new wxStaticText( itemPanel54, wxID_STATIC, _("N/A"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText84, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText85 = new wxStaticText( itemPanel54, wxID_STATIC, _("Any field last changed on:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText85, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText86 = new wxStaticText( itemPanel54, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer78->Add(itemStaticText86, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel54, _("Dates and Times"));

  wxPanel* itemPanel87 = new wxPanel( GetBookCtrl(), ID_PANEL_PPOLICY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxStaticBox* itemStaticBoxSizer88Static = new wxStaticBox(itemPanel87, wxID_ANY, _("Random password generation rules"));
  wxStaticBoxSizer* itemStaticBoxSizer88 = new wxStaticBoxSizer(itemStaticBoxSizer88Static, wxVERTICAL);
  itemPanel87->SetSizer(itemStaticBoxSizer88);

  wxRadioButton* itemRadioButton89 = new wxRadioButton( itemPanel87, ID_RADIOBUTTON2, _("Use Database Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton89->SetValue(false);
  itemStaticBoxSizer88->Add(itemRadioButton89, 0, wxALIGN_LEFT|wxALL, 5);

  wxRadioButton* itemRadioButton90 = new wxRadioButton( itemPanel87, ID_RADIOBUTTON3, _("Use the policy below:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemRadioButton90->SetValue(false);
  itemStaticBoxSizer88->Add(itemRadioButton90, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticLine* itemStaticLine91 = new wxStaticLine( itemPanel87, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  itemStaticBoxSizer88->Add(itemStaticLine91, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer92 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer88->Add(itemBoxSizer92, 0, wxALIGN_LEFT|wxALL, 5);
  wxStaticText* itemStaticText93 = new wxStaticText( itemPanel87, wxID_STATIC, _("Password length: "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer92->Add(itemStaticText93, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxSpinCtrl* itemSpinCtrl94 = new wxSpinCtrl( itemPanel87, ID_SPINCTRL3, _T("8"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 4, 100, 8 );
  itemBoxSizer92->Add(itemSpinCtrl94, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxCheckBox* itemCheckBox95 = new wxCheckBox( itemPanel87, ID_CHECKBOX3, _("Use lowercase letters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox95->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox95, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox96 = new wxCheckBox( itemPanel87, ID_CHECKBOX4, _("Use UPPERCASE letters"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox96->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox96, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox97 = new wxCheckBox( itemPanel87, ID_CHECKBOX5, _("Use digits"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox97->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox97, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox98 = new wxCheckBox( itemPanel87, ID_CHECKBOX6, _("Use symbols (i.e., ., %, $, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox98->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox98, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox99 = new wxCheckBox( itemPanel87, ID_CHECKBOX7, _("Use only easy-to-read charachters (i.e., no 'l', '1', etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox99->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox99, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox100 = new wxCheckBox( itemPanel87, ID_CHECKBOX8, _("Generate pronounceable passwords"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox100->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox100, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText101 = new wxStaticText( itemPanel87, wxID_STATIC, _("Or"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer88->Add(itemStaticText101, 0, wxALIGN_LEFT|wxALL, 5);

  wxCheckBox* itemCheckBox102 = new wxCheckBox( itemPanel87, ID_CHECKBOX9, _("Use hexadecimal digits only (0-9, a-f)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemCheckBox102->SetValue(false);
  itemStaticBoxSizer88->Add(itemCheckBox102, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton103 = new wxButton( itemPanel87, ID_BUTTON7, _("Reset to Database Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer88->Add(itemButton103, 0, wxALIGN_RIGHT|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel87, _("Password Policy"));

////@end AddEditPropSheet content construction
}


/*!
 * Should we show tooltips?
 */

bool AddEditPropSheet::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap AddEditPropSheet::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin AddEditPropSheet bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end AddEditPropSheet bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AddEditPropSheet::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin AddEditPropSheet icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end AddEditPropSheet icon retrieval
}
