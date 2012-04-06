/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManagePwdPolicies.cpp
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

#include "ManagePwdPolicies.h"
#include "PasswordPolicy.h"
#include "pwsclip.h"
#include "core/PWCharPool.h"

////@begin XPM images
#include "graphics/toolbar/new/copypassword.xpm"
#include "graphics/toolbar/new/copypassword_disabled.xpm"
////@end XPM images


/*!
 * CManagePasswordPolicies type definition
 */

IMPLEMENT_CLASS( CManagePasswordPolicies, wxDialog )


/*!
 * CManagePasswordPolicies event table definition
 */

BEGIN_EVENT_TABLE( CManagePasswordPolicies, wxDialog )

////@begin CManagePasswordPolicies event table entries
  EVT_BUTTON( wxID_NEW, CManagePasswordPolicies::OnNewClick )

  EVT_BUTTON( ID_EDIT_PP, CManagePasswordPolicies::OnEditPpClick )

  EVT_BUTTON( wxID_DELETE, CManagePasswordPolicies::OnDeleteClick )

  EVT_BUTTON( ID_LIST, CManagePasswordPolicies::OnListClick )

  EVT_BUTTON( wxID_UNDO, CManagePasswordPolicies::OnUndoClick )

  EVT_BUTTON( wxID_REDO, CManagePasswordPolicies::OnRedoClick )

  EVT_BUTTON( ID_GENERATE_PASSWORD, CManagePasswordPolicies::OnGeneratePasswordClick )

  EVT_BUTTON( ID_BITMAPBUTTON, CManagePasswordPolicies::OnCopyPasswordClick )

  EVT_BUTTON( wxID_OK, CManagePasswordPolicies::OnOkClick )

  EVT_BUTTON( wxID_CANCEL, CManagePasswordPolicies::OnCancelClick )

  EVT_BUTTON( wxID_HELP, CManagePasswordPolicies::OnHelpClick )

////@end CManagePasswordPolicies event table entries

END_EVENT_TABLE()


/*!
 * CManagePasswordPolicies constructor
 */


CManagePasswordPolicies::CManagePasswordPolicies( wxWindow* parent,  PWScore &core, wxWindowID id,
						  const wxString& caption, const wxPoint& pos,
						  const wxSize& size, long style )
  : m_core(core), m_iundo_pos(-1), m_iSortNamesIndex(0), m_iSortEntriesIndex(0),
  m_bSortNamesAscending(true), m_bSortEntriesAscending(true), m_iSelectedItem(0),
  m_bChanged(false), m_bViewPolicy(true)
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * CManagePasswordPolicies creator
 */

bool CManagePasswordPolicies::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CManagePasswordPolicies creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CManagePasswordPolicies creation
  return true;
}


/*!
 * CManagePasswordPolicies destructor
 */

CManagePasswordPolicies::~CManagePasswordPolicies()
{
////@begin CManagePasswordPolicies destruction
////@end CManagePasswordPolicies destruction
}


/*!
 * Member initialisation
 */

void CManagePasswordPolicies::Init()
{
////@begin CManagePasswordPolicies member initialisation
  m_PolicyNames = NULL;
  m_passwordCtrl = NULL;
  m_lowerTableDesc = NULL;
  m_PolicyDetails = NULL;
  m_PolicyEntries = NULL;
////@end CManagePasswordPolicies member initialisation
  m_MapPSWDPLC = m_core.GetPasswordPolicies();

  m_st_default_pp.SetToDefaults();
}


/*!
 * Control creation for CManagePasswordPolicies
 */

void CManagePasswordPolicies::CreateControls()
{    
////@begin CManagePasswordPolicies content construction
  CManagePasswordPolicies* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Available Password Policies:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_PolicyNames = new wxGrid( itemDialog1, ID_POLICYLIST, wxDefaultPosition, wxSize(269, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PolicyNames->SetDefaultColSize(100);
  m_PolicyNames->SetDefaultRowSize(25);
  m_PolicyNames->SetColLabelSize(25);
  m_PolicyNames->SetRowLabelSize(50);
  m_PolicyNames->CreateGrid(10, 2, wxGrid::wxGridSelectRows);
  itemBoxSizer4->Add(m_PolicyNames, 3, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer4->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxGridSizer* itemGridSizer7 = new wxGridSizer(0, 2, 0, 0);
  itemBoxSizer6->Add(itemGridSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton( itemDialog1, wxID_NEW, _("&New"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, ID_EDIT_PP, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton( itemDialog1, wxID_DELETE, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton11 = new wxButton( itemDialog1, ID_LIST, _("List"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton12 = new wxButton( itemDialog1, wxID_UNDO, _("&Undo"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton13 = new wxButton( itemDialog1, wxID_REDO, _("&Redo"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer7->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemDialog1, wxID_STATIC, _("Test Selected Policy"));
  wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
  itemBoxSizer6->Add(itemStaticBoxSizer14, 0, wxGROW|wxALL, 5);

  wxButton* itemButton15 = new wxButton( itemDialog1, ID_GENERATE_PASSWORD, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemStaticBoxSizer14->Add(itemBoxSizer16, 0, wxGROW|wxALL, 5);

  m_passwordCtrl = new wxTextCtrl( itemDialog1, ID_PASSWORD_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer16->Add(m_passwordCtrl, 5, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton18 = new wxBitmapButton( itemDialog1, ID_BITMAPBUTTON, itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  wxBitmap itemBitmapButton18BitmapDisabled(itemDialog1->GetBitmapResource(wxT("graphics/toolbar/new/copypassword_disabled.xpm")));
  itemBitmapButton18->SetBitmapDisabled(itemBitmapButton18BitmapDisabled);
  if (CManagePasswordPolicies::ShowToolTips())
    itemBitmapButton18->SetToolTip(_("Copy Password to clipboard"));
  itemBoxSizer16->Add(itemBitmapButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_lowerTableDesc = new wxStaticText( itemDialog1, wxID_STATIC, _("Selected policy details:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(m_lowerTableDesc, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer20, 0, wxGROW|wxALL, 5);

  m_PolicyDetails = new wxGrid( itemDialog1, ID_POLICYPROPERTIES, wxDefaultPosition, wxSize(-1, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PolicyDetails->SetDefaultColSize(210);
  m_PolicyDetails->SetDefaultRowSize(25);
  m_PolicyDetails->SetColLabelSize(25);
  m_PolicyDetails->SetRowLabelSize(50);
  m_PolicyDetails->CreateGrid(5, 2, wxGrid::wxGridSelectRows);
  itemBoxSizer20->Add(m_PolicyDetails, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_PolicyEntries = new wxGrid( itemDialog1, ID_POLICYENTRIES, wxDefaultPosition, wxSize(-1, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PolicyEntries->SetDefaultColSize(140);
  m_PolicyEntries->SetDefaultRowSize(25);
  m_PolicyEntries->SetColLabelSize(25);
  m_PolicyEntries->SetRowLabelSize(50);
  m_PolicyEntries->CreateGrid(5, 3, wxGrid::wxGridSelectRows);
  itemBoxSizer20->Add(m_PolicyEntries, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStdDialogButtonSizer* itemStdDialogButtonSizer23 = new wxStdDialogButtonSizer;

  itemBoxSizer2->Add(itemStdDialogButtonSizer23, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxButton* itemButton24 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer23->AddButton(itemButton24);

  wxButton* itemButton25 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer23->AddButton(itemButton25);

  wxButton* itemButton26 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStdDialogButtonSizer23->AddButton(itemButton26);

  itemStdDialogButtonSizer23->Realize();

////@end CManagePasswordPolicies content construction

  if (m_core.IsReadOnly()) {
    FindWindow(wxID_NEW)->Enable(false);
    FindWindow(wxID_DELETE)->Enable(false);

    // Hide cancel button & change OK button text
    FindWindow(wxID_CANCEL)->Enable(false);
    FindWindow(wxID_CANCEL)->Show(false);

    FindWindow(wxID_OK)->SetLabel(_("Close"));
    FindWindow(wxID_EDIT)->SetLabel(_("View"));
  }

  // We have 2 grids, but we show only one at a time,
  // toggle when user clicks on ID_LIST button.
  // Setting these up:
  m_PolicyNames->SetColLabelValue(0, _("Policy Name"));
  m_PolicyNames->SetColLabelValue(1, _("Use count"));
  UpdateNames();
  m_PolicyNames->SelectRow(0);

  // Since we select the default policy, disable List & Delete
  FindWindow(ID_LIST)->Enable(false);
  FindWindow(wxID_DELETE)->Enable(false);

  m_PolicyDetails->SetColLabelValue(0, _("Policy Field"));
  m_PolicyDetails->SetColLabelValue(1, _("Value"));
  UpdateDetails(); 

  m_PolicyEntries->SetColLabelValue(0, _("Group"));
  m_PolicyEntries->SetColLabelValue(1, _("Title"));
  m_PolicyEntries->SetColLabelValue(2, _("User Name"));

  ShowPolicyDetails();

  // Max. of 255 policy names allowed - only 2 hex digits used for number
  if (m_MapPSWDPLC.size() >= 255)
    FindWindow(wxID_NEW)->Enable(false);

  // No changes yet
  FindWindow(wxID_UNDO)->Enable(false);
  FindWindow(wxID_REDO)->Enable(false);

  m_PolicyNames->SetFocus();
}


/*!
 * Should we show tooltips?
 */

bool CManagePasswordPolicies::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CManagePasswordPolicies::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin CManagePasswordPolicies bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("graphics/toolbar/new/copypassword.xpm"))
  {
    wxBitmap bitmap(copypassword_xpm);
    return bitmap;
  }
  else if (name == _T("graphics/toolbar/new/copypassword_disabled.xpm"))
  {
    wxBitmap bitmap(copypassword_disabled_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end CManagePasswordPolicies bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CManagePasswordPolicies::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CManagePasswordPolicies icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CManagePasswordPolicies icon retrieval
}

bool CManagePasswordPolicies::Show(bool show)
{
  if (m_bViewPolicy)
    ShowPolicyDetails();
  else
    ShowPolicyEntries();
  return wxDialog::Show(show);
}


void CManagePasswordPolicies::ShowPolicyDetails()
{
  m_bViewPolicy = true;
  FindWindow(ID_LIST)->SetLabel(_("List"));
  m_lowerTableDesc->SetLabel(_("Selected policy details:"));
  m_PolicyDetails->Show();
  m_PolicyDetails->Enable(true);
  m_PolicyEntries->Hide();
  m_PolicyEntries->Enable(false);
  GetSizer()->Layout();
}

void CManagePasswordPolicies::ShowPolicyEntries()
{
  m_bViewPolicy = false;
  FindWindow(ID_LIST)->SetLabel(_("Details"));
  m_lowerTableDesc->SetLabel(_("Entries using selected policy:"));
  m_PolicyEntries->Show();
  m_PolicyEntries->Enable(true);
  m_PolicyDetails->Hide();
  m_PolicyDetails->Enable(false);
  GetSizer()->Layout();
}

void CManagePasswordPolicies::UpdateNames()
{
  int nPos = 0;
  m_PolicyNames->ClearGrid();

  // Add in the default policy as the first entry
  m_PolicyNames->SetCellValue(nPos, 0, _("Default Policy"));
  m_PolicyNames->SetCellValue(nPos, 1, _("N/A"));
  m_PolicyNames->SetReadOnly(nPos, 0); m_PolicyNames->SetReadOnly(nPos, 1);

  // Add in all other policies - ItemData == offset into map
  PSWDPolicyMapIter iter;
  nPos++;
  for (iter = m_MapPSWDPLC.begin(); iter != m_MapPSWDPLC.end(); iter++) {
    m_PolicyNames->InsertRows(nPos);
    m_PolicyNames->SetCellValue(nPos, 0, iter->first.c_str());
    wxString useCount;
    if (iter->second.usecount != 0)
      useCount.Printf(wxT("%d"),iter->second.usecount);
    else
      useCount = _("Not used");
    m_PolicyNames->SetCellValue(nPos, 1, useCount);
    m_PolicyNames->SetReadOnly(nPos, 0); m_PolicyNames->SetReadOnly(nPos, 1);
  }
}

static void wxRowPutter(int row, const stringT &name, const stringT &value,
                             void *table)
{
  // Callback function used by st_PSWDPolicy::Policy2Table
  wxGrid *tableControl = (wxGrid *)table;
  tableControl->InsertRows(row);
  tableControl->SetCellValue(row, 0, name.c_str());
  tableControl->SetCellValue(row, 1, value.c_str());
  tableControl->SetReadOnly(row, 0); tableControl->SetReadOnly(row, 1);
}

st_PSWDPolicy CManagePasswordPolicies::GetSelectedPolicy() const
{
  /*
    If m_iSelectedItem = 0, then fill in with the database default,
    otherwise use the name entry
  */

  if (m_iSelectedItem != 0) {
    const wxString policyname = m_PolicyNames->GetCellValue(m_iSelectedItem, 0);

    PSWDPolicyMapCIter iter = m_MapPSWDPLC.find(policyname.c_str());
    if (iter == m_MapPSWDPLC.end())
      return m_st_default_pp;

    return iter->second;
  } else {
    return m_st_default_pp;
  }
}

void CManagePasswordPolicies::UpdateDetails()
{
  if (m_iSelectedItem == -1)
    return;

  /*
    If m_iSelectedItem = 0, then fill in with the database default,
    otherwise use the name entry
  */

  st_PSWDPolicy st_pp = GetSelectedPolicy();

  m_PolicyDetails->ClearGrid();
  st_pp.Policy2Table(wxRowPutter, m_PolicyDetails);
}

void CManagePasswordPolicies::UpdatePolicy(const wxString &policyname, CPP_FLAGS mode)
{
    m_bChanged = true;
    // Save changes for Undo/Redo
    st_PSWDPolicyChange st_change;
    st_change.name = m_iSelectedItem != 0 ? policyname : wxT("");
    st_change.flags = mode;
    st_change.st_pp_save = m_iSelectedItem != 0 ? m_mapIter->second : m_st_default_pp;

    if (m_iSelectedItem != 0) {
      // Changed a named password policy
      PSWDPolicyMapIter iter_new = m_MapPSWDPLC.find(StringX(policyname.c_str()));
      if (iter_new == m_MapPSWDPLC.end())
        ASSERT(0);
      st_change.st_pp_new = iter_new->second;
    } else {
      // Changed the database default policy
      st_change.st_pp_new = m_st_default_pp;
    }

  if (m_iundo_pos != (int)m_vchanges.size() - 1) {
    // We did have changes that could have been redone
    // But not anymore - delete all these to add new change on the end
    m_vchanges.resize(m_iundo_pos + 1);
  }

  // Add new change
  m_vchanges.push_back(st_change);
  // Update pointer to the one that is next to be undone
  m_iundo_pos++;
  // Update buttons appropriately
  FindWindow(wxID_UNDO)->Enable(true);
  FindWindow(wxID_REDO)->Enable(false);

  // Update lists
  UpdateNames();
  int N = m_PolicyNames->GetNumberRows();
  for (int row = 0; row < N; row++) 
    if (m_PolicyNames->GetCellValue(row, 0) == policyname) {
      m_iSelectedItem = row;
      m_PolicyNames->SelectRow(row);
      break;
    }

  UpdateDetails();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEW
 */

void CManagePasswordPolicies::OnNewClick( wxCommandEvent& )
{
  CPasswordPolicy ppdlg(this, m_core);

  ppdlg.SetPolicyData(wxT(""), m_MapPSWDPLC);
  if (ppdlg.ShowModal() == wxID_OK) {
    wxString policyname;
    
    ppdlg.GetPolicyData(m_st_default_pp, policyname, m_MapPSWDPLC);
    UpdatePolicy(policyname, CPP_ADD);
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT
 */

void CManagePasswordPolicies::OnEditPpClick( wxCommandEvent& )
{
  wxString policyname = m_PolicyNames->GetCellValue(m_iSelectedItem, 0);

  m_mapIter = m_MapPSWDPLC.find(StringX(policyname.c_str()));
  if (m_iSelectedItem != 0 && m_mapIter == m_MapPSWDPLC.end())
    return;

  CPasswordPolicy ppdlg(this, m_core);

  ppdlg.SetPolicyData(policyname, m_MapPSWDPLC);
  if (ppdlg.ShowModal() == wxID_OK) {
    ppdlg.GetPolicyData(m_st_default_pp, policyname, m_MapPSWDPLC);
    UpdatePolicy(policyname, CPP_MODIFIED);
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE
 */

void CManagePasswordPolicies::OnDeleteClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LIST
 */

void CManagePasswordPolicies::OnListClick( wxCommandEvent&  )
{
  if (m_bViewPolicy)
    ShowPolicyEntries();
  else
    ShowPolicyDetails();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UNDO
 */

void CManagePasswordPolicies::OnUndoClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UNDO in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UNDO in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO
 */

void CManagePasswordPolicies::OnRedoClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CManagePasswordPolicies::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CManagePasswordPolicies::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void CManagePasswordPolicies::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in CManagePasswordPolicies.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in CManagePasswordPolicies. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERATE_PASSWORD
 */

void CManagePasswordPolicies::OnGeneratePasswordClick( wxCommandEvent& event )
{
  st_PSWDPolicy st_pp = GetSelectedPolicy();
  
  StringX passwd = st_pp.pwp.MakeRandomPassword(st_pp.symbols.c_str());
  m_passwordCtrl->SetValue(passwd.c_str());
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
 */

void CManagePasswordPolicies::OnCopyPasswordClick( wxCommandEvent& )
{
  PWSclip::SetData(m_passwordCtrl->GetValue().c_str());
}

