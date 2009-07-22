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
#include <vector>
#include "corelib/PWSprefs.h"
#include "corelib/PWCharPool.h"
#include "corelib/PWHistory.h"

#include "addeditpropsheet.h"
#include "PWSgrid.h"
#include "PWStree.h"
#include "pwsclip.h"

////@begin XPM images
////@end XPM images


/*!
 * AddEditPropSheet type definition
 */

IMPLEMENT_CLASS( AddEditPropSheet, wxPropertySheetDialog )


/*!
 * AddEditPropSheet event table definition
 */

BEGIN_EVENT_TABLE( AddEditPropSheet, wxPropertySheetDialog )

  EVT_BUTTON( wxID_OK, AddEditPropSheet::OnOk )
////@begin AddEditPropSheet event table entries
  EVT_BUTTON( ID_BUTTON2, AddEditPropSheet::OnShowHideClick )

  EVT_BUTTON( ID_BUTTON3, AddEditPropSheet::OnGenerateButtonClick )

  EVT_BUTTON( ID_GO_BTN, AddEditPropSheet::OnGoButtonClick )

  EVT_CHECKBOX( ID_CHECKBOX, AddEditPropSheet::OnOverrideDCAClick )

  EVT_CHECKBOX( ID_CHECKBOX1, AddEditPropSheet::OnKeepHistoryClick )

////@end AddEditPropSheet event table entries

END_EVENT_TABLE()


/*!
 * AddEditPropSheet constructors
 */

AddEditPropSheet::AddEditPropSheet(wxWindow* parent, PWScore &core,
                                   PWSGrid *grid, PWSTreeCtrl *tree,
                                   AddOrEdit type, const CItemData &item,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size,
                                   long style)
: m_core(core), m_grid(grid), m_tree(tree), m_type(type), m_item(item)
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
  ItemFieldsToPropSheet();
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

// following based on m_DCAcomboBoxStrings
// which is generated via DialogBlocks - unify these later
static struct {short pv; wxString name;}
  dcaMapping[] =
    {{PWSprefs::DoubleClickAutoType, _("Auto Type")},
     {PWSprefs::DoubleClickBrowse, _("Browse")},
     {PWSprefs::DoubleClickBrowsePlus, _("Browse + Auto Type")},
     {PWSprefs::DoubleClickCopyNotes, _("Copy Notes")},
     {PWSprefs::DoubleClickCopyPassword, _("Copy Password")},
     {PWSprefs::DoubleClickCopyPasswordMinimize, _("Copy Password + Minimize")},
     {PWSprefs::DoubleClickCopyUsername, _("Copy Username")},
     {PWSprefs::DoubleClickViewEdit, _("View/Edit Entry")},
     {PWSprefs::DoubleClickRun, _("Execute Run command")},
    };

/*!
 * Member initialisation
 */

void AddEditPropSheet::Init()
{
////@begin AddEditPropSheet member initialisation
  m_groupCtrl = NULL;
  m_PasswordCtrl = NULL;
  m_ShowHideCtrl = NULL;
  m_Password2Ctrl = NULL;
  m_DCAcomboBox = NULL;
  m_PWHgrid = NULL;
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

  wxArrayString m_groupCtrlStrings;
  m_groupCtrl = new wxComboBox( itemPanel2, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_groupCtrlStrings, wxCB_DROPDOWN );
  itemFlexGridSizer5->Add(m_groupCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

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

  m_PasswordCtrl = new wxTextCtrl( itemPanel2, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(m_PasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer5->Add(itemBoxSizer17, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  itemBoxSizer17->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_ShowHideCtrl = new wxButton( itemPanel2, ID_BUTTON2, _("&Hide"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(m_ShowHideCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  itemBoxSizer17->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton21 = new wxButton( itemPanel2, ID_BUTTON3, _("&Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText22 = new wxStaticText( itemPanel2, wxID_STATIC, _("Confirm:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Password2Ctrl = new wxTextCtrl( itemPanel2, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
  itemFlexGridSizer5->Add(m_Password2Ctrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer5->Add(10, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText25 = new wxStaticText( itemPanel2, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxTextCtrl* itemTextCtrl26 = new wxTextCtrl( itemPanel2, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer5->Add(itemTextCtrl26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer5->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  itemBoxSizer27->Add(10, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton29 = new wxButton( itemPanel2, ID_GO_BTN, _("Go"), wxDefaultPosition, wxDefaultSize, 0 );
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

  wxArrayString m_DCAcomboBoxStrings;
  m_DCAcomboBoxStrings.Add(_("Auto Type"));
  m_DCAcomboBoxStrings.Add(_("Browse"));
  m_DCAcomboBoxStrings.Add(_("Browse + Auto Type"));
  m_DCAcomboBoxStrings.Add(_("Copy Notes"));
  m_DCAcomboBoxStrings.Add(_("Copy Password"));
  m_DCAcomboBoxStrings.Add(_("Copy Password + Minimize"));
  m_DCAcomboBoxStrings.Add(_("Copy Username"));
  m_DCAcomboBoxStrings.Add(_("View/Edit Entry"));
  m_DCAcomboBoxStrings.Add(_("Execute Run command"));
  m_DCAcomboBox = new wxComboBox( itemPanel33, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_DCAcomboBoxStrings, wxCB_READONLY );
  itemBoxSizer41->Add(m_DCAcomboBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

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

  m_PWHgrid = new wxGrid( itemPanel33, ID_GRID, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PWHgrid->SetDefaultColSize(150);
  m_PWHgrid->SetDefaultRowSize(25);
  m_PWHgrid->SetColLabelSize(25);
  m_PWHgrid->SetRowLabelSize(0);
  m_PWHgrid->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemStaticBoxSizer44->Add(m_PWHgrid, 0, wxGROW|wxALL, 5);

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

  // Set validators
  itemTextCtrl10->SetValidator( wxGenericValidator(& m_title) );
  itemTextCtrl13->SetValidator( wxGenericValidator(& m_user) );
  itemTextCtrl26->SetValidator( wxGenericValidator(& m_url) );
  itemTextCtrl32->SetValidator( wxGenericValidator(& m_notes) );
  itemTextCtrl37->SetValidator( wxGenericValidator(& m_autotype) );
  itemTextCtrl39->SetValidator( wxGenericValidator(& m_runcmd) );
  itemCheckBox42->SetValidator( wxGenericValidator(& m_useDefaultDCA) );
  itemCheckBox46->SetValidator( wxGenericValidator(& m_keepPWHist) );
  itemSpinCtrl47->SetValidator( wxGenericValidator(& m_maxPWHist) );
////@end AddEditPropSheet content construction
  m_PWHgrid->SetColLabelValue(0, _("Set Date/Time"));
  m_PWHgrid->SetColLabelValue(1, _("Password"));
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

void AddEditPropSheet::ItemFieldsToPropSheet()
{
  // Populate the combo box
  std::vector<stringT> aryGroups;
  m_core.GetUniqueGroups(aryGroups);
  for (size_t igrp = 0; igrp < aryGroups.size(); igrp++) {
    m_groupCtrl->Append(aryGroups[igrp].c_str());
  }
  // select relevant group
  const StringX group = m_item.GetGroup();
  if (!group.empty())
    for (size_t igrp = 0; igrp < aryGroups.size(); igrp++)
      if (group == aryGroups[igrp].c_str()) {
        m_groupCtrl->SetSelection(igrp);
        break;
      }
  
  m_title = m_item.GetTitle().c_str();
  m_user = m_item.GetUser().c_str();
  m_url = m_item.GetURL().c_str();
  m_password = m_item.GetPassword();
  PWSprefs *prefs = PWSprefs::GetInstance();
  if (prefs->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  m_PasswordCtrl->ChangeValue(m_password.c_str());
  // Enable Go button iff m_url isn't empty
  wxWindow *goBtn = FindWindow(ID_GO_BTN);
  goBtn->Enable(!m_url.empty());
  m_notes = m_item.GetNotes().c_str();
  m_autotype = m_item.GetAutoType().c_str();
  m_runcmd = m_item.GetRunCommand().c_str();

  // double-click action:
  short iDCA;
  m_item.GetDCA(iDCA);
  m_useDefaultDCA = (iDCA < PWSprefs::minDCA || iDCA > PWSprefs::maxDCA);
  m_DCAcomboBox->Enable(!m_useDefaultDCA);
  if (m_useDefaultDCA) {
    m_DCA = short(PWSprefs::GetInstance()->
                  GetPref(PWSprefs::DoubleClickAction));
  } else {
    m_DCA = iDCA;
  }
  for (size_t i = 0; i < sizeof(dcaMapping)/sizeof(dcaMapping[0]); i++)
    if (m_DCA == dcaMapping[i].pv) {
      m_DCAcomboBox->SetValue(dcaMapping[i].name);
      break;
    }
  // History: If we're adding, use preferences, otherwise,
  // get values from m_item
  if (m_type == ADD) {
    // Get history preferences
    PWSprefs *prefs = PWSprefs::GetInstance();
    m_keepPWHist = prefs->GetPref(PWSprefs::SavePasswordHistory);
    m_maxPWHist = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
  } else {
    PWHistList pwhl;
    size_t pwh_max, num_err;

    m_keepPWHist = CreatePWHistoryList(m_item.GetPWHistory(),
                                       pwh_max, num_err,
                                       pwhl, TMC_LOCALE);
    m_maxPWHist = int(pwh_max);
    int row = 0;
    for (PWHistList::iterator iter = pwhl.begin(); iter != pwhl.end(); ++iter) {
      m_PWHgrid->SetCellValue(row, 0, iter->changedate.c_str());
      m_PWHgrid->SetCellValue(row, 1, iter->password.c_str());
      row++;
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN
 */

void AddEditPropSheet::OnGoButtonClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN in AddEditPropSheet.
  // Before editing this code, remove the block markers.
  wxMessageBox(_("'Go' placeholder"));
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GO_BTN in AddEditPropSheet. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void AddEditPropSheet::OnGenerateButtonClick( wxCommandEvent& event )
{
  PWPolicy pwp;
  m_item.GetPWPolicy(pwp);
  StringX password = pwp.MakeRandomPassword();


  PWSclip::SetData(password);
  m_password = password.c_str();
  m_PasswordCtrl->ChangeValue(m_password.c_str());
  if (m_isPWHidden) {
    m_Password2Ctrl->ChangeValue(m_password.c_str());
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void AddEditPropSheet::OnShowHideClick( wxCommandEvent& event )
{
  if (m_isPWHidden) {
    ShowPassword();
  } else {
    m_password = m_PasswordCtrl->GetValue(); // save visible password
    HidePassword();
  }
}

void AddEditPropSheet::ShowPassword()
{
  m_isPWHidden = false;
  m_ShowHideCtrl->SetLabel(_("&Hide"));

  m_PasswordCtrl->ChangeValue(m_password.c_str());
  // XXX Can't change wxTE_PASSWORD style - need to think of something else.
  // Disable confirmation Ctrl, as the user can see the password entered
  m_Password2Ctrl->ChangeValue(_(""));
  m_Password2Ctrl->Enable(false);
}

void AddEditPropSheet::HidePassword()
{
  m_isPWHidden = true;
  m_ShowHideCtrl->SetLabel(_("&Show"));

  // XXX Can't change wxTE_PASSWORD style - need to think of something else.

  // Need verification as the user can not see the password entered
  m_Password2Ctrl->ChangeValue(m_password.c_str());
  m_Password2Ctrl->Enable(true);
}

void AddEditPropSheet::OnOk(wxCommandEvent& event)
{
  if (Validate() && TransferDataFromWindow()) {
    time_t t;
    const wxString group = m_groupCtrl->GetValue();
    const StringX password = m_PasswordCtrl->GetValue().c_str();

    switch (m_type) {
    case EDIT: {
      bool bIsModified, bIsPSWDModified;
      short lastDCA;
      m_item.GetDCA(lastDCA);
      if (m_useDefaultDCA) { // get value from global pref
        m_DCA = short(PWSprefs::GetInstance()->
                      GetPref(PWSprefs::DoubleClickAction));
      } else { // get value from field
        const wxString cv = m_DCAcomboBox->GetValue();      
        for (size_t i = 0; i < sizeof(dcaMapping)/sizeof(dcaMapping[0]); i++)
          if (cv == dcaMapping[i].name) {
            m_DCA = dcaMapping[i].pv;
            break;
          }
      }
      
      // Check if modified
      bIsModified = (group        != m_item.GetGroup().c_str()      ||
                     m_title      != m_item.GetTitle().c_str()      ||
                     m_user       != m_item.GetUser().c_str()       ||
                     m_notes      != m_item.GetNotes().c_str()      ||
                     m_url        != m_item.GetURL().c_str()        ||
                     m_autotype   != m_item.GetAutoType().c_str()   ||
                     m_runcmd != m_item.GetRunCommand().c_str()     ||
                     m_DCA        != lastDCA                        ||
                     m_PWHistory  != m_item.GetPWHistory().c_str()  ||
#ifdef NOTYET
                     m_AEMD.locXTime    != m_AEMD.oldlocXTime          ||
                     m_AEMD.XTimeInt    != m_AEMD.oldXTimeInt          ||
                     m_AEMD.ipolicy     != m_AEMD.oldipolicy           ||
                     (m_AEMD.ipolicy     == SPECIFIC_POLICY &&
                      m_AEMD.pwp         != m_AEMD.oldpwp)
#else 
                     0
#endif
                     );

      bIsPSWDModified = (password != m_item.GetPassword());

      if (bIsModified) {
        // Just modify all - even though only 1 may have actually been modified
        m_item.SetGroup(group.c_str());
        m_item.SetTitle(m_title.c_str());
        m_item.SetUser(m_user.empty() ?
                       m_core.GetDefUsername().c_str() : m_user.c_str());
        m_item.SetNotes(m_notes.c_str());
        m_item.SetURL(m_url.c_str());
        m_item.SetAutoType(m_autotype.c_str());
        m_item.SetRunCommand(m_runcmd.c_str());
        m_item.SetPWHistory(m_PWHistory.c_str());
#ifdef NOTYET
        if (m_AEMD.ipolicy == DEFAULT_POLICY)
          m_item.SetPWPolicy(_T(""));
        else
          m_item.SetPWPolicy(m_AEMD.pwp);
#endif
        m_item.SetDCA(m_useDefaultDCA ? -1 : m_DCA);
      } // bIsModified

      time(&t);
      if (bIsPSWDModified) {
        m_item.SetPassword(password);
#ifdef NOTYET
        if (SavePWHistory)
          UpdateHistory();
#endif
        m_item.SetPMTime(t);
      }
      if (bIsModified || bIsPSWDModified)
        m_item.SetRMTime(t);
#ifdef NOTYET
      if (m_AEMD.oldlocXTime != m_AEMD.locXTime)
        m_item.SetXTime(m_AEMD.tttXTime);
      if (m_AEMD.oldXTimeInt != m_AEMD.XTimeInt)
        m_item.SetXTimeInt(m_AEMD.XTimeInt);
#endif
      // All fields in m_item now reflect user's edits
      // Let's update the core's data
      uuid_array_t uuid;
      m_item.GetUUID(uuid);
      ItemListIter listpos = m_core.Find(uuid);
      ASSERT(listpos != m_core.GetEntryEndIter());
      m_core.RemoveEntryAt(listpos);
      m_core.AddEntry(m_item);
      // refresh tree view
      m_tree->UpdateItem(m_item);
      m_grid->UpdateItem(m_item);
    }
      break;

    case ADD:
      m_item.SetGroup(group.c_str());
      m_item.SetTitle(m_title.c_str());
      m_item.SetUser(m_user.empty() ?
                     m_core.GetDefUsername().c_str() : m_user.c_str());
      m_item.SetNotes(m_notes.c_str());
      m_item.SetURL(m_url.c_str());
      m_item.SetPassword(password);
      m_item.SetAutoType(m_autotype.c_str());
      m_item.SetRunCommand(m_runcmd.c_str());
      m_item.SetDCA(m_DCA);
      time(&t);
      m_item.SetCTime(t);
      if (m_keepPWHist)
        m_item.SetPWHistory(MakePWHistoryHeader(TRUE, m_maxPWHist, 0));
      
#ifdef NOTYET
      if (m_AEMD.XTimeInt > 0 && m_AEMD.XTimeInt <= 3650)
        m_item.SetXTimeInt(m_AEMD.XTimeInt);

      if (m_AEMD.ibasedata > 0) {
        // Password in alias format AND base entry exists
        // No need to check if base is an alias as already done in
        // call to PWScore::GetBaseEntry
        uuid_array_t alias_uuid;
        m_item.GetUUID(alias_uuid);
        m_AEMD.pcore->AddDependentEntry(m_AEMD.base_uuid, alias_uuid, CItemData::ET_ALIAS);
        m_item.SetPassword(_T("[Alias]"));
        m_item.SetAlias();
        ItemListIter iter = m_AEMD.pcore->Find(m_AEMD.base_uuid);
        if (iter != m_AEMD.pDbx->End()) {
          const CItemData &cibase = iter->second;
          DisplayInfo *di = (DisplayInfo *)cibase.GetDisplayInfo();
          int nImage = m_AEMD.pDbx->GetEntryImage(cibase);
          m_AEMD.pDbx->SetEntryImage(di->list_index, nImage, true);
          m_AEMD.pDbx->SetEntryImage(di->tree_item, nImage, true);
        }
      } else {
        m_item.SetPassword(m_AEMD.realpassword);
        m_item.SetNormal();
      }

      if (m_item.IsAlias()) {
        m_item.SetXTime((time_t)0);
        m_item.SetPWPolicy(_T(""));
      } else {
        m_item.SetXTime(m_AEMD.tttXTime);
        if (m_AEMD.ipolicy == DEFAULT_POLICY)
          m_item.SetPWPolicy(_T(""));
        else
          m_item.SetPWPolicy(m_AEMD.pwp);
      }
#endif
      break;
    case VIEW:
      // No Update
      break;
    default:
      ASSERT(0);
      break;
    }
  }
  EndModal(wxID_OK);
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1
 */

void AddEditPropSheet::OnKeepHistoryClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1 in AddEditPropSheet.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1 in AddEditPropSheet. 
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX
 */

void AddEditPropSheet::OnOverrideDCAClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    m_DCAcomboBox->Enable(!m_useDefaultDCA);
    if (m_useDefaultDCA) { // restore default
      short dca = short(PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DoubleClickAction));
      for (size_t i = 0; i < sizeof(dcaMapping)/sizeof(dcaMapping[0]); i++)
        if (dca == dcaMapping[i].pv) {
          m_DCAcomboBox->SetValue(dcaMapping[i].name);
          break;
        }
      m_DCA = -1; // 'use default' value
    }
  }
}

