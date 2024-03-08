/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SetFiltersDlg.cpp
* 
*/

////@begin includes

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/grid.h"

#include "core/core.h"
#include "core/PWScore.h"

#include "PWFiltersGrid.h"
#include "SetFiltersDlg.h"
#include "wxUtilities.h"
#include "PasswordSafeFrame.h"
#include "ManageFiltersTable.h"

////@end includes

////@begin XPM images
////@end XPM images

/*!
 * SetFiltersDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SetFiltersDlg, wxDialog )


/*!
 * SetFiltersDlg event table definition
 */

BEGIN_EVENT_TABLE( SetFiltersDlg, wxDialog )

////@begin SetFiltersDlg event table entries
  EVT_BUTTON( wxID_APPLY, SetFiltersDlg::OnApplyClick )
  EVT_BUTTON( wxID_OK, SetFiltersDlg::OnOkClick )
  EVT_BUTTON( wxID_CANCEL, SetFiltersDlg::OnCancelClick )
  EVT_BUTTON( wxID_HELP, SetFiltersDlg::OnHelpClick )
  EVT_CLOSE( SetFiltersDlg::OnClose )
////@end SetFiltersDlg event table entries

END_EVENT_TABLE()


/*!
 * SetFiltersDlg constructors
 */
SetFiltersDlg::SetFiltersDlg(wxWindow *parent, st_filters *pfilters,
                             st_filters *currentFilters,
                             bool *appliedCalled,
                             const FilterType filtertype,
                             const FilterPool filterpool,
                             const bool bCanHaveAttachments,
                             const std::set<StringX> *psMediaTypes,
                             wxWindowID id, const wxString& caption,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style ) : m_pfilters(pfilters),
                                            m_currentFilters(currentFilters),
                                            m_bCanHaveAttachments(bCanHaveAttachments || // If filter includes Attachment allow this
                                                                  ((filtertype == DFTYPE_MAIN) && pfilters && (pfilters->num_Aactive > 0))),
                                            m_psMediaTypes(psMediaTypes),
                                            m_filtertype(filtertype),
                                            m_filterpool(filterpool),
                                            m_AppliedCalled(appliedCalled)
{
  wxASSERT(!parent || parent->IsTopLevel());

  wxString heading(caption);
  
  ASSERT(m_pfilters);
  m_filterName = towxstring(m_pfilters->fname);
  
  switch(filtertype) {
    case DFTYPE_PWHISTORY:
      heading = SYMBOL_SETFILTERS_PWHIST_TITLE;
      break;
    case DFTYPE_PWPOLICY:
      heading = SYMBOL_SETFILTERS_POLICY_TITLE;
      break;
    case DFTYPE_ATTACHMENT:
      heading = SYMBOL_SETFILTERS_ATTACHMENT_TITLE;
      break;
    case DFTYPE_MAIN:
      /* FALLTHROUGH */
    default:
      if(heading.empty())
        heading = SYMBOL_SETFILTERS_TITLE;
      break;
  }

////@begin SetFiltersDlg creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
  
  // On second window of same type (history, policy or attachment) move small pixel number
  // down and right to leave view on below present original window
  if(m_filtertype != DFTYPE_MAIN) {
    // Move down and right to show lower window
    int x, y;
    GetPosition(&x, &y);
    Move(x + SET_FILTER_WINDOW_OFFSET, y + SET_FILTER_WINDOW_OFFSET);
  }
////@end SetFiltersDlg creation
}

SetFiltersDlg* SetFiltersDlg::Create(wxWindow *parent, st_filters *pfilters, st_filters *currentFilters, 
  bool *appliedCalled, const FilterType filtertype, FilterPool filterpool, 
  const bool bCanHaveAttachments, const std::set<StringX> *psMediaTypes, wxWindowID id, 
  const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
  return new SetFiltersDlg(parent, pfilters, currentFilters, appliedCalled, filtertype, filterpool, 
                           bCanHaveAttachments, psMediaTypes, id, caption, pos, size, style);
}

/*!
 * Control creation for SetFiltersDlg
 */

void SetFiltersDlg::CreateControls()
{    
////@begin SetFiltersDlg content construction
  SetFiltersDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

  wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Filter name:")+_T(" "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  // Only the main window may edit the filter name, while the history, policy and attachment gets actual value shown
  if(m_filtertype == DFTYPE_MAIN) {
    wxTextCtrl* itemTextCtrl3 = new wxTextCtrl( itemDialog1, ID_FILTERNAME, m_filterName, wxDefaultPosition, wxDefaultSize, (m_filtertype != DFTYPE_MAIN) ? wxTE_READONLY : 0 );
    itemBoxSizer1->Add(itemTextCtrl3, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
    // Set validators
    itemTextCtrl3->SetValidator( wxTextValidator(wxFILTER_NONE, & m_filterName) );
    
    itemBoxSizer1->AddSpacer(itemStaticText2->GetSize().GetWidth() / 2);
  }
  else {
    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, ID_FILTERNAME, m_filterName+_T(" "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
  }

  m_filterGrid = new pwFiltersGrid( itemDialog1, ID_FILTERGRID,
                                   m_pfilters, m_filtertype, m_filterpool, m_bCanHaveAttachments, m_psMediaTypes, true,
                                   wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_origFilters = *m_pfilters; // set origFilters here, because pwFiltersGrid insert empty item, if list is empty
  itemBoxSizer2->Add(m_filterGrid, 1, wxGROW|wxALL|wxEXPAND, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton6 = new wxButton( itemDialog1, wxID_APPLY, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton9 = new wxButton( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end SetFiltersDlg content construction
  
  if(m_filtertype != DFTYPE_MAIN) {
    itemButton6->Disable(); // Do not allow Apply in case History, Policy or Attachment filter
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
 */

void SetFiltersDlg::OnApplyClick( wxCommandEvent& /*event*/ )
{
  if ((m_filtertype == DFTYPE_MAIN) && Validate() && TransferDataFromWindow()) {
    // Second call will clear and remove active filter
    if(*m_AppliedCalled) {
      // If filter is already applied, this one is call of Clear and filter is taken back
      m_currentFilters->Empty();
      // Update view
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_APPLYFILTER);
      GetParent()->GetEventHandler()->ProcessEvent(event);
      // Mark as not applied
      *m_AppliedCalled = false;
      // Renew button label
      FindWindow(wxID_APPLY)->SetLabel(_("&Apply"));
      return;
    }
    // At first time check parameter
    if(m_filterName.IsEmpty()) {
      wxMessageBox(_("No filter name given."), _("Name this filter."), wxOK|wxICON_ERROR);
      return;
    }
    if(! VerifyFilters()) {
      // Message to user is given in VeryFilters()
      return;
    }
    // When correct copy filter and apply menu item to force start of filter in view
    m_pfilters->fname = m_filterName.c_str();
    m_currentFilters->Empty();
    *m_currentFilters = *m_pfilters;
    // Update the view on filtered entries
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_APPLYFILTER);
    event.SetString(pwManageFiltersTable::getSourcePoolLabel(m_filterpool));
    GetParent()->GetEventHandler()->ProcessEvent(event);
    // Mark applied is called
    *m_AppliedCalled = true;
    // Set button label as "Clear" to revert filter application on next press
    FindWindow(wxID_APPLY)->SetLabel(_("Clear"));
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SetFiltersDlg::OnOkClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    if((m_filtertype == DFTYPE_MAIN) && m_filterName.IsEmpty()) {
      wxMessageBox(_("No filter name given."), _("Name this filter."), wxOK|wxICON_ERROR);
      return;
    }
    if(! VerifyFilters()) {
      // Message to user is given in VeryFilters()
      return;
    }    
    m_filterGrid->ClearIfEmpty();
    if(m_filtertype == DFTYPE_MAIN)
      m_pfilters->fname = m_filterName.c_str();
  }
  EndModal(wxID_OK);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void SetFiltersDlg::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in SetFiltersDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in SetFiltersDlg.
}


/*!
 * Should we show tooltips?
 */

bool SetFiltersDlg::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap SetFiltersDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin SetFiltersDlg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end SetFiltersDlg bitmap retrieval
}


/*!
 * Get icon resources
 */

wxIcon SetFiltersDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin SetFiltersDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end SetFiltersDlg icon retrieval
}


/*!
 * Verify filter and return true when correct
 */

bool SetFiltersDlg::VerifyFilters()
{
  vFilterRows *currentFilter;
  
  // Select applicable filter group
  switch(m_filtertype) {
    case DFTYPE_MAIN:
      currentFilter = &m_pfilters->vMfldata;
      break;
    case DFTYPE_PWHISTORY:
      currentFilter = &m_pfilters->vHfldata;
      break;
    case DFTYPE_PWPOLICY:
      currentFilter = &m_pfilters->vPfldata;
      break;
    case DFTYPE_ATTACHMENT:
      currentFilter = &m_pfilters->vAfldata;
      break;
    case DFTYPE_INVALID:
      /* FALLTHROUGH */
    default:
      ASSERT(false);
      return false;
  }
  int i = 0;
  int iError = -1;
  int iHistory = -1;
  int iPolicy = -1;
  int iAttachment = -1;
  // Check on complete filter. History, Policy and Attachment need related group filled
  for_each(currentFilter->begin(), currentFilter->end(), [&i, &iError, &iHistory, &iPolicy, &iAttachment] (st_FilterRow st_fldata) {
    ++i; // User is starting it's count by 1 for users view
    if((st_fldata.mtype != PWSMatch::MT_PWHIST && st_fldata.mtype != PWSMatch::MT_POLICY && st_fldata.mtype != PWSMatch::MT_ATTACHMENT) &&
       (st_fldata.mtype == PWSMatch::MT_INVALID || st_fldata.rule  == PWSMatch::MR_INVALID)) {
      iError = i;
      return false;
    }
    if(st_fldata.mtype == PWSMatch::MT_PWHIST)
      iHistory = i;
    if(st_fldata.mtype == PWSMatch::MT_POLICY)
      iPolicy = i;
    if(st_fldata.mtype == PWSMatch::MT_ATTACHMENT)
      iAttachment = i;
    return true;
  } );
  
  if(iError != -1) {
    stringT msg;
    Format(msg, _("Row %d is incomplete.").c_str(), iError);
    wxMessageBox(_("Set both Field and Criteria."), wxString(msg), wxOK|wxICON_ERROR);
    return false;
  }
  
  if(m_filtertype == DFTYPE_MAIN) {
    if((iHistory != -1) && (m_pfilters->vHfldata.empty() || ! m_pfilters->num_Hactive)) {
      stringT msg;
      Format(msg, _("Row %d is incomplete.").c_str(), iHistory);
      wxMessageBox(_("Set both Field and Criteria or update History filters."), wxString(msg), wxOK|wxICON_ERROR);
      return false;
    }
    if((iPolicy != -1) && (m_pfilters->vPfldata.empty() || ! m_pfilters->num_Pactive)) {
      stringT msg;
      Format(msg, _("Row %d is incomplete.").c_str(), iPolicy);
      wxMessageBox(_("Set both Field and Criteria or update Policy filters."), wxString(msg), wxOK|wxICON_ERROR);
      return false;
    }
    if((iAttachment != -1) && (m_pfilters->vAfldata.empty() || ! m_pfilters->num_Aactive)) {
      stringT msg;
      Format(msg, _("Row %d is incomplete.").c_str(), iAttachment);
      wxMessageBox(_("Set both Field and Criteria or update Attachment filters."), wxString(msg), wxOK|wxICON_ERROR);
      return false;
    }
  }
  
  return true;
}

bool SetFiltersDlg::IsChanged() const {
  return *m_pfilters != m_origFilters || m_filterName != m_origFilters.fname;
}

bool SetFiltersDlg::SyncAndQueryCancel(bool showDialog) {
  if((m_filtertype == DFTYPE_MAIN) && *m_AppliedCalled) {
    if (showDialog) {
      wxMessageDialog dialog(this, _("Applied pressed before Cancel"), _("Do you wish to overtake applied filter?"), wxYES_NO | wxICON_EXCLAMATION);
      if(dialog.ShowModal() == wxID_NO) {
        m_currentFilters->Empty();

        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_APPLYFILTER);
        GetParent()->GetEventHandler()->ProcessEvent(event);

        *m_AppliedCalled = false;
      }
    }
    // don't block forced close in this case
  }

  return QueryCancelDlg::SyncAndQueryCancel(showDialog);
}
