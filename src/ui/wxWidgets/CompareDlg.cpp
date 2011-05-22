/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file CompareDlg.cpp
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

#include "./CompareDlg.h"
#include "./DbSelectionPanel.h"
#include "./wxutils.h"
#include "../../core/PWScore.h"
#include "./AdvancedSelectionDlg.h"

#include <wx/statline.h>

DECLARE_EVENT_TYPE(wxEVT_RELAYOUT, -1)
DEFINE_EVENT_TYPE(wxEVT_RELAYOUT)

enum {ID_BTN_COMPARE = 100 };

BEGIN_EVENT_TABLE( CompareDlg, wxDialog )
  EVT_BUTTON( ID_BTN_COMPARE,  CompareDlg::OnCompare )
END_EVENT_TABLE()

CompareDlg::CompareDlg(wxWindow* parent, PWScore* currentCore): wxDialog(parent, 
                                                                wxID_ANY, 
                                                                wxT("Compare current database with another database"),
                                                                wxDefaultPosition,
                                                                wxDefaultSize,
                                                                wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX),
                                                                m_currentCore(currentCore),
                                                                m_selCriteria(new SelectionCriteria)
{
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

  //compare all fields by default
  m_selCriteria->m_bsFields.set();

  CreateControls();
  Connect(GetId(), wxEVT_RELAYOUT, wxCommandEventHandler(CompareDlg::OnReLayout));
}

CompareDlg::~CompareDlg()
{
  delete m_selCriteria;
}

void CompareDlg::CreateControls()
{
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);  //add a margin at the top

  wxCollapsiblePane* topPane = CreateDBSelectionPanel(dlgSizer);
  topPane->Expand();

  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Center().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(0));
  dlgSizer->AddSpacer(RowSeparation);
  wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer;
  buttons->Add(new wxButton(this, wxID_CANCEL));
  buttons->SetAffirmativeButton(new wxButton(this, ID_BTN_COMPARE, _("&Compare")));
  buttons->Realize();
  dlgSizer->Add(buttons, wxSizerFlags().Center().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(0));
  dlgSizer->AddSpacer(BottomMargin);

  SetSizerAndFit(dlgSizer);
}

struct CompareDlgType {
  static bool IsMandatoryField(CItemData::FieldType field) {
    return field == CItemData::GROUP || field == CItemData::TITLE || field == CItemData::USER;
  }
  
  static bool ShowFieldSelection() {
    return true;
  }
  
  static wxString GetTaskWord() {
    return _("compare");
  }
};

wxCollapsiblePane* CompareDlg::CreateDBSelectionPanel(wxSizer* dlgSizer)
{
  wxString paneTitle = wxString::Format(_("Select a database to compare with current database (%s)"),
                                                towxstring(m_currentCore->GetCurFile()).c_str());

  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, paneTitle);
  wxWindow* paneWindow = pane->GetPane();

  wxBoxSizer* dbPanelSizer = new wxBoxSizer(wxVERTICAL);
  dbPanelSizer->Add(new DbSelectionPanel(paneWindow, wxString(_("Password Database")),
                                               wxString(_("Select a PasswordSafe database to compare")),
                                               true,
                                               m_currentCore,
                                               1), wxSizerFlags().Expand().Proportion(1));

  dlgSizer->Add(pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  dlgSizer->AddSpacer(RowSeparation);
  
  //Create another collapsible pane to be nested within the top collapsible pane
  wxCollapsiblePane* nestedPane = new wxCollapsiblePane(this, wxID_ANY, _("Advanced Options..."));
  //Create the Advanced Selection Options panel with the pane's window as parent
  AdvancedSelectionPanel* advPanel = new AdvancedSelectionImpl<CompareDlgType>(nestedPane->GetPane(), *m_selCriteria, true);
  advPanel->CreateControls(nestedPane->GetPane());
  //Create a vertical box sizer
  wxBoxSizer* nestedSizer = new wxBoxSizer(wxVERTICAL);
  //and add the panel to it
  nestedSizer->Add(advPanel, wxSizerFlags().Expand().Proportion(1));
  //Set it as the nested pane's sizer
  nestedPane->GetPane()->SetSizer(nestedSizer);
  //Connect our event handler to resize the dialog when the inner pane collapses/expands
  nestedPane->Connect(nestedPane->GetId(), wxEVT_COMMAND_COLLPANE_CHANGED, 
                      wxCollapsiblePaneEventHandler(CompareDlg::OnPaneCollapse), 
                      NULL, this);
  
  pane->Connect(pane->GetId(), wxEVT_COMMAND_COLLPANE_CHANGED, 
                      wxCollapsiblePaneEventHandler(CompareDlg::OnTopPaneCollapse), 
                      NULL, this);

  dlgSizer->Add(nestedPane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));

  paneWindow->SetSizer(dbPanelSizer);
  dbPanelSizer->SetSizeHints(paneWindow);


  return pane;
}

void CompareDlg::OnTopPaneCollapse(wxCollapsiblePaneEvent& evt)
{
  evt.Skip();
}

void CompareDlg::OnPaneCollapse(wxCollapsiblePaneEvent& evt)
{
  wxCommandEvent cmdEvt(wxEVT_RELAYOUT, GetId());
  GetEventHandler()->AddPendingEvent(cmdEvt);
  evt.Skip();
}

void CompareDlg::OnReLayout(wxCommandEvent& evt)
{
  GetSizer()->RecalcSizes();
  Fit();
  /*
  const wxSize size = GetSize();
  wxSizeEvent event( size, GetId() );
  event.SetEventObject( this );
  GetEventHandler()->AddPendingEvent( event );
   */
//  GetSizer()->Layout();
}

void CompareDlg::OnCompare(wxCommandEvent& )
{
  if ( Validate() && TransferDataFromWindow())
    DoCompare();
}

void CompareDlg::DoCompare()
{
  
}
