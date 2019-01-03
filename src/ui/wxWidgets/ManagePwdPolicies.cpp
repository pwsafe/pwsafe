/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "./wxutils.h"

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
  EVT_GRID_SELECT_CELL( CManagePasswordPolicies::OnSelectCell )

  EVT_BUTTON( wxID_NEW, CManagePasswordPolicies::OnNewClick )

  EVT_BUTTON( ID_EDIT_PP, CManagePasswordPolicies::OnEditClick )

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

  EVT_SIZE( CManagePasswordPolicies::OnSize )

  EVT_MAXIMIZE( CManagePasswordPolicies::OnMaximize )

END_EVENT_TABLE()

/*!
 * CManagePasswordPolicies constructor
 */

CManagePasswordPolicies::CManagePasswordPolicies( wxWindow* parent,  PWScore &core, wxWindowID id,
              const wxString& caption, const wxPoint& pos,
              const wxSize& size, long style )
: m_core(core), m_curPolRow(-1),
  m_iSortNamesIndex(0), m_iSortEntriesIndex(0),
  m_bSortNamesAscending(true), m_bSortEntriesAscending(true), m_bViewPolicy(true),
  m_bShowPolicyEntriesInitially(true)
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
  m_PolicyNames = nullptr;
  m_passwordCtrl = nullptr;
  m_lowerTableDesc = nullptr;
  m_PolicyDetails = nullptr;
  m_PolicyEntries = nullptr;
////@end CManagePasswordPolicies member initialisation

  m_PolicyManager = std::unique_ptr<PolicyManager>(new PolicyManager(m_core));
}

/*!
 * Control creation for CManagePasswordPolicies
 */

void CManagePasswordPolicies::CreateControls()
{
////@begin CManagePasswordPolicies content construction
  CManagePasswordPolicies* itemDialog1 = this;

  auto *itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Available Password Policies:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

  auto *itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_PolicyNames = new wxGrid( itemDialog1, ID_POLICYLIST, wxDefaultPosition, wxSize(-1, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PolicyNames->SetDefaultColSize(100);
  m_PolicyNames->SetDefaultRowSize(25);
  m_PolicyNames->SetColLabelSize(25);
  m_PolicyNames->SetRowLabelSize(0);
  m_PolicyNames->CreateGrid(10, 2, wxGrid::wxGridSelectRows);
  m_PolicyNames->EnableEditing(false);
  itemBoxSizer4->Add(m_PolicyNames, 3, wxEXPAND|wxALL, 5);

  auto *itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer4->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemGridSizer7 = new wxGridSizer(0, 2, 0, 0);
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
  auto *itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
  itemBoxSizer6->Add(itemStaticBoxSizer14, 0, wxGROW|wxALL, 5);

  wxButton* itemButton15 = new wxButton( itemDialog1, ID_GENERATE_PASSWORD, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  auto *itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
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

  auto *itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer20, 1, wxEXPAND|wxALL, 5);

  m_PolicyDetails = new wxGrid( itemDialog1, ID_POLICYPROPERTIES, wxDefaultPosition, wxSize(-1, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PolicyDetails->SetDefaultColSize(220);
  m_PolicyDetails->SetDefaultRowSize(25);
  m_PolicyDetails->SetColLabelSize(25);
  m_PolicyDetails->SetRowLabelSize(0);
  m_PolicyDetails->CreateGrid(8, 2, wxGrid::wxGridSelectRows);
  m_PolicyDetails->EnableEditing(false);
  itemBoxSizer20->Add(m_PolicyDetails, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_PolicyEntries = new wxGrid( itemDialog1, ID_POLICYENTRIES, wxDefaultPosition, wxSize(-1, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_PolicyEntries->SetDefaultColSize(140);
  m_PolicyEntries->SetDefaultRowSize(25);
  m_PolicyEntries->SetColLabelSize(25);
  m_PolicyEntries->SetRowLabelSize(0);
  m_PolicyEntries->CreateGrid(8, 3, wxGrid::wxGridSelectRows);
  m_PolicyEntries->EnableEditing(false);
  itemBoxSizer20->Add(m_PolicyEntries, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto *itemStdDialogButtonSizer23 = new wxStdDialogButtonSizer;

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
    FindWindow(ID_EDIT_PP)->SetLabel(_("View"));
  }

  // We have 2 grids, but we show only one at a time,
  // toggle when user clicks on ID_LIST button.
  // Setting these up:
  m_PolicyNames->SetColLabelValue(0, _("Policy Name"));
  m_PolicyNames->SetColLabelValue(1, _("Use count"));
  m_PolicyNames->InvalidateBestSize();
  m_PolicyNames->SetClientSize(m_PolicyNames->GetBestSize());
  UpdateNames();
  m_PolicyNames->Fit();
  m_PolicyNames->SelectRow(0);

  // Since we select the default policy, disable List & Delete
  FindWindow(ID_LIST)->Enable(false);
  FindWindow(wxID_DELETE)->Enable(false);

  m_PolicyDetails->SetColLabelValue(0, _("Policy Field"));
  m_PolicyDetails->SetColLabelValue(1, _("Value"));
  UpdateDetails();
  m_PolicyDetails->Fit();

  m_PolicyEntries->SetColLabelValue(0, _("Group"));
  m_PolicyEntries->SetColLabelValue(1, _("Title"));
  m_PolicyEntries->SetColLabelValue(2, _("User Name"));
  m_PolicyEntries->InvalidateBestSize();
  m_PolicyEntries->SetClientSize(m_PolicyEntries->GetBestSize());
  m_PolicyEntries->Fit();

  ShowPolicyDetails(); // Show initially table with policy details.

  m_ScrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this) - 10;


  // Max. of 255 policy names allowed - only 2 hex digits used for number
  if (m_PolicyManager->HasMaxPolicies()) {
    FindWindow(wxID_NEW)->Enable(false);
  }

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

wxIcon CManagePasswordPolicies::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin CManagePasswordPolicies icon retrieval
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

  if (m_bShowPolicyEntriesInitially) {
    m_bShowPolicyEntriesInitially = false;

    // All columns of policy entries shall get the same space (see also CManagePasswordPolicies::ResizeGridColumns)
    int width = m_PolicyEntries->GetClientSize().GetWidth() - m_PolicyEntries->GetRowLabelSize() - m_ScrollbarWidth;

    if (width > 0) {
      m_PolicyEntries->SetColSize(0, width/3);
      m_PolicyEntries->SetColSize(1, width/3);
      m_PolicyEntries->SetColSize(2, width/3);
    }
  }
}

void CManagePasswordPolicies::UpdateNames()
{
  int row = 0;
  const auto& policies      = m_PolicyManager->GetPolicies();
  const auto& defaultPolicy = m_PolicyManager->GetDefaultPolicy();

  m_PolicyNames->ClearGrid();

  // Add in the default policy as the first entry
  m_PolicyNames->SetCellValue(row, 0, PolicyManager::GetDefaultPolicyName());
  m_PolicyNames->SetCellValue(row, 1, _("N/A"));

  for (const auto& policy : policies) {

    row++;

    // Re-use existing rows and only add a new one if needed
    if (m_PolicyNames->GetNumberRows() <= row) {
      m_PolicyNames->InsertRows(row);
    }

    // Add policy name
    m_PolicyNames->SetCellValue(row, 0, policy.first.c_str());

    // Add policy usage counter
    wxString useCount;
    if (policy.second.usecount != 0) {
      useCount << policy.second.usecount;
    }
    else {
      useCount = _("Not used");
    }

    m_PolicyNames->SetCellValue(row, 1, useCount);
  }
}

/**
 * Callback function used by PWPolicy::Policy2Table
 */
static void wxRowPutter(int row, const stringT &name, const stringT &value, void *table)
{
  auto *tableControl = static_cast<wxGrid *>(table);

  if (tableControl) {
    if (tableControl->GetNumberRows() <= row) {
      tableControl->InsertRows(row);
    }

    tableControl->SetCellValue(row, 0, name.c_str());
    tableControl->SetCellValue(row, 1, value.c_str());
  }
}

int CManagePasswordPolicies::GetSelectedRow() const
{
  wxArrayInt ai = m_PolicyNames->GetSelectedRows();
  if (!ai.IsEmpty())
    return ai[0];
  else {
    return m_curPolRow;
  }
}

/**
 * Provides the selected policy.
 *
 * If first row or no row selected, then fill in with the database default,
 * otherwise use the name entry.
 */
PWPolicy CManagePasswordPolicies::GetSelectedPolicy() const
{
  int row = GetSelectedRow();

  if (row > 0) {
    const wxString policyname = m_PolicyNames->GetCellValue(row, 0);

    if (m_PolicyManager->HasPolicy(policyname.ToStdWstring())) {
      return m_PolicyManager->GetPolicy(policyname.ToStdWstring());
    }
  }

  return m_PolicyManager->GetDefaultPolicy();
}

void CManagePasswordPolicies::UpdateDetails()
{
  // Update details table to reflect selected policy, if any
  if (GetSelectedRow() < 0) {
    return;
  }

  PWPolicy st_pp = GetSelectedPolicy();

  m_PolicyDetails->ClearGrid();
  st_pp.Policy2Table(wxRowPutter, m_PolicyDetails);
}

void CManagePasswordPolicies::UpdateEntryList()
{
  int row = GetSelectedRow();

  if (row < 0) {
    return;
  }

  m_PolicyEntries->ClearGrid();

  GTUSet gtuSet;

  if (m_core.InitialiseGTU(gtuSet, StringX(m_PolicyNames->GetCellValue(row, 0).c_str()))) {

    row = 0;

    for (const auto& gtuItem : gtuSet) {

      // Re-use existing rows and only add a new one if needed
      if (m_PolicyEntries->GetNumberRows() <= row) {
        m_PolicyEntries->InsertRows(row);
      }

      m_PolicyEntries->SetCellValue(row, 0, gtuItem.group.c_str());
      m_PolicyEntries->SetCellValue(row, 1, gtuItem.title.c_str());
      m_PolicyEntries->SetCellValue(row, 2, gtuItem.user.c_str());
      row++;
    }
  }
}

void CManagePasswordPolicies::UpdateSelection(const wxString& policyname)
{
  int rows = m_PolicyNames->GetNumberRows();

  for (int row = 0; row < rows; ++row) {

    if (m_PolicyNames->GetCellValue(row, 0) == policyname) {
      m_PolicyNames->SetFocus();
      m_PolicyNames->SelectRow(row);
      m_PolicyNames->SetGridCursor(row, 0);
    }
  }
}

void CManagePasswordPolicies::UpdateUndoRedoButtons()
{
  FindWindow(wxID_UNDO)->Enable(m_PolicyManager->CanUndo());
  FindWindow(wxID_REDO)->Enable(m_PolicyManager->CanRedo());
}

void CManagePasswordPolicies::ResizeGridColumns()
{
  int width = 0;

  //
  // Table with policy names
  //

  // First column of policy names grid shall get available space, whereas the second column has fixed size
  width = m_PolicyNames->GetClientSize().GetWidth() - m_PolicyNames->GetRowLabelSize() - m_PolicyNames->GetColSize(1) - m_ScrollbarWidth;

  if (width > 0) {
    m_PolicyNames->SetColSize(0, width);
  }

  //
  // Table with policy details
  //

  // Second column of policy details grid shall get available space, whereas the first column has fixed size
  width = m_PolicyDetails->GetClientSize().GetWidth() - m_PolicyDetails->GetRowLabelSize() - m_PolicyDetails->GetColSize(0) - m_ScrollbarWidth;

  if (width > 0) {
    m_PolicyDetails->SetColSize(1, width);
  }

  //
  // Table with policy entries
  //

  // All columns of policy entries shall get the same space
  width = m_PolicyEntries->GetClientSize().GetWidth() - m_PolicyEntries->GetRowLabelSize() - m_ScrollbarWidth;

  if (width > 0) {
    m_PolicyEntries->SetColSize(0, width/3);
    m_PolicyEntries->SetColSize(1, width/3);
    m_PolicyEntries->SetColSize(2, width/3);
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEW
 */

void CManagePasswordPolicies::OnNewClick( wxCommandEvent& )
{
  auto policies = m_PolicyManager->GetPolicies();
  auto policy   = m_PolicyManager->GetDefaultPolicy();

  CPasswordPolicy ppdlg(this, m_core, policies);
  ppdlg.SetPolicyData(wxEmptyString, policy);

  if (ppdlg.ShowModal() == wxID_OK) {
    wxString policyname;

    ppdlg.GetPolicyData(policyname, policy);

    m_PolicyManager->PolicyAdded(policyname.ToStdWstring(), policy);

    UpdateNames();
    UpdateSelection(policyname);
    UpdateUndoRedoButtons();
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT
 */

void CManagePasswordPolicies::OnEditClick( wxCommandEvent& )
{
  int row = GetSelectedRow();

  if (row < 0) {
    return;
  }

  const auto& policies = m_PolicyManager->GetPolicies();

  wxString originalPolicyname;
  wxString modifiedPolicyname;

  PWPolicy originalPolicy;
  PWPolicy modifiedPolicy;

  if (row == 0) { // 1st row is default
    originalPolicyname = PolicyManager::GetDefaultPolicyName();
    originalPolicy     = m_PolicyManager->GetDefaultPolicy();
  }
  else {          // All other rows hold user individual policy names

    originalPolicyname = m_PolicyNames->GetCellValue(row, 0);

    if (!(m_PolicyManager->HasPolicy(originalPolicyname.ToStdWstring()))) {
      ASSERT(0);
      return;
    }

    originalPolicy     = m_PolicyManager->GetPolicy(originalPolicyname.ToStdWstring());
  }

  CPasswordPolicy ppdlg(this, m_core, policies);

  ppdlg.SetPolicyData(originalPolicyname, originalPolicy);

  if (ppdlg.ShowModal() == wxID_OK) {

    ppdlg.GetPolicyData(modifiedPolicyname, modifiedPolicy);

    if (originalPolicyname != modifiedPolicyname) {

      m_PolicyManager->PolicyRenamed(
        originalPolicyname.ToStdWstring(), modifiedPolicyname.ToStdWstring(),
        originalPolicy, modifiedPolicy
      );

      UpdateNames();
      UpdateSelection(modifiedPolicyname);
      UpdateUndoRedoButtons();
    }
    else if (originalPolicy != modifiedPolicy) {

      m_PolicyManager->PolicyModified(
        originalPolicyname.ToStdWstring(),
        originalPolicy, modifiedPolicy
      );

      UpdateNames();
      UpdateSelection(originalPolicyname);
      UpdateUndoRedoButtons();
    }
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE
 */

void CManagePasswordPolicies::OnDeleteClick( wxCommandEvent& )
{
  int row = GetSelectedRow();

  if (row > 0) {
    wxString policyname = m_PolicyNames->GetCellValue(row, 0);

    m_PolicyManager->PolicyRemoved(policyname.ToStdWstring());

    UpdateNames();
    UpdateSelection(PolicyManager::GetDefaultPolicyName());
    UpdateUndoRedoButtons();
  }
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

void CManagePasswordPolicies::OnUndoClick( wxCommandEvent& )
{
  m_PolicyManager->Undo();
  UpdateUndoRedoButtons();
  UpdateNames();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REDO
 */

void CManagePasswordPolicies::OnRedoClick( wxCommandEvent& )
{
  m_PolicyManager->Redo();
  UpdateUndoRedoButtons();
  UpdateNames();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CManagePasswordPolicies::OnOkClick( wxCommandEvent& )
{
  /*
   * User may have changed default policy, named policy, none or both.
   * If anything has changed, we treat the change as atomic, creating a multicommand
   * s.t. Undo/Redo will work as expected.
   */
  const auto& olddefpol = PWSprefs::GetInstance()->GetDefaultPolicy();
  const auto& newdefpol = m_PolicyManager->GetDefaultPolicy();
  bool defChanged = (olddefpol != newdefpol);

  auto policies = m_PolicyManager->GetPolicies();
  bool namedChanged = (policies != m_core.GetPasswordPolicies());

  if (defChanged || namedChanged) {
    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

    if (defChanged) {
      // User has changed database default policy - need to update preferences
      // Update the copy only!
      PWSprefs::GetInstance()->SetupCopyPrefs();
      PWSprefs::GetInstance()->SetDefaultPolicy(newdefpol, true);

      // Now get new DB preferences String value
      StringX sxNewDBPrefsString(PWSprefs::GetInstance()->Store(true));

      // Set up Command to update string in database
      if (m_core.GetReadFileVersion() == PWSfile::VCURRENT)
        pmulticmds->Add(DBPrefsCommand::Create(&m_core, sxNewDBPrefsString));
    } // defChanged

    if (namedChanged) {
      pmulticmds->Add(DBPolicyNamesCommand::Create(&m_core, policies,
                                                   DBPolicyNamesCommand::NP_REPLACEALL));
    }
    m_core.Execute(pmulticmds);
  } // defChanged || namedChanged
  m_bShowPolicyEntriesInitially = true;
  EndModal(wxID_OK);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CManagePasswordPolicies::OnCancelClick( wxCommandEvent& event )
{
  m_bShowPolicyEntriesInitially = true;

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
  UNREFERENCED_PARAMETER(event);
  PWPolicy st_pp = GetSelectedPolicy();

  StringX passwd = st_pp.MakeRandomPassword();
  m_passwordCtrl->SetValue(passwd.c_str());
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
 */

void CManagePasswordPolicies::OnCopyPasswordClick( wxCommandEvent& )
{
  PWSclipboard::GetInstance()->SetData(tostringx(m_passwordCtrl->GetValue()));
}

/*!
 * wxEVT_GRID_SELECT_CELL event handler for ID_POLICYLIST
 */

void CManagePasswordPolicies::OnSelectCell( wxGridEvent& event )
{
  if (event.GetEventObject() == m_PolicyNames) {

    auto cellValue = m_PolicyNames->GetCellValue(event.GetRow(), event.GetCol());

    if (event.GetRow() == 0) { /* First row contains the default policy */

      // Update button states
      if (!m_core.IsReadOnly()) {
        FindWindow(wxID_DELETE)->Enable(false);
        FindWindow(ID_EDIT_PP)->Enable(true);
      }

      FindWindow(ID_LIST)->Enable(false);

      // Update details of selected policy
      m_curPolRow = event.GetRow();
      UpdateDetails();
      UpdateEntryList();
    }
    else if (cellValue.IsEmpty()) { /* Row with an empty cell */

      // Update button states
      if (!m_core.IsReadOnly()) {
        FindWindow(wxID_DELETE)->Enable(false);
        FindWindow(ID_EDIT_PP)->Enable(false);
      }

      FindWindow(ID_LIST)->Enable(false);

      m_PolicyDetails->ClearGrid();
      m_PolicyEntries->ClearGrid();

      // Indicates to all other functions that no policy was selected (e.g. empty row)
      m_curPolRow = -1;
    }
    else {

      // Update button states
      if (!m_core.IsReadOnly()) {
        FindWindow(wxID_DELETE)->Enable(true);
        FindWindow(ID_EDIT_PP)->Enable(true);
      }

      FindWindow(ID_LIST)->Enable(true);

      // Update details of selected policy
      m_curPolRow = event.GetRow();
      UpdateDetails();
      UpdateEntryList();
    }
  }
}

/**
 * Event handler (EVT_SIZE) that will be called when the window has been resized.
 *
 * @param event holds information about size change events.
 * @see <a href="http://docs.wxwidgets.org/3.0/classwx_size_event.html">wxSizeEvent Class Reference</a>
 */
void CManagePasswordPolicies::OnSize(wxSizeEvent& event)
{
  ResizeGridColumns();

  event.Skip();
}

/**
 * Event handler (EVT_MAXIMIZE) that will be called when the window has been maximized.
 *
 * @param event holds information about size change events.
 * @see <a href="http://docs.wxwidgets.org/3.0/classwx_maximize_event.html">wxMaximizeEvent Class Reference</a>
 */
void CManagePasswordPolicies::OnMaximize(wxMaximizeEvent& event)
{
  CallAfter(&CManagePasswordPolicies::ResizeGridColumns); // delayed execution of resizing, until dialog is completely layout

  event.Skip();
}
