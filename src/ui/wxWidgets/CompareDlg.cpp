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
#include "./ComparisonGridTable.h"

#include <wx/statline.h>
#include <wx/grid.h>

enum {ID_BTN_COMPARE = 100 };

BEGIN_EVENT_TABLE( CompareDlg, wxDialog )
  EVT_BUTTON( ID_BTN_COMPARE,  CompareDlg::OnCompare )
END_EVENT_TABLE()

struct ComparisonData {
  wxCollapsiblePane* pane;
  wxGrid* grid;
  CompareData data;
  wxSizerItem* sizerBelow;  //for managing the spacers in between
  
  ComparisonData(): pane(0), grid(0), sizerBelow(0){}
  ~ComparisonData() { /*nothing to do.  All window objects deleted automatically */ }
};

CompareDlg::CompareDlg(wxWindow* parent, PWScore* currentCore): wxDialog(parent, 
                                                                wxID_ANY, 
                                                                wxT("Compare current database with another database"),
                                                                wxDefaultPosition,
                                                                wxDefaultSize,
                                                                wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX),
                                                                m_currentCore(currentCore),
                                                                m_otherCore(new PWScore),
                                                                m_selCriteria(new SelectionCriteria),
                                                                m_dbPanel(0),
                                                                m_dbSelectionPane(0),
                                                                m_optionsPane(0),
                                                                m_current(new ComparisonData),
                                                                m_comparison(new ComparisonData),
                                                                m_conflicts(new ComparisonData),
                                                                m_identical(new ComparisonData)
{
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

  //compare all fields by default
  m_selCriteria->SelectAllFields();

  CreateControls();
  wxSize screenSize = ::wxGetClientDisplayRect().GetSize();

  //assume taskbar/appbar/menubar is horizontal (at top or bottom)
  int captionHeight = ::wxSystemSettings::GetMetric(wxSYS_CAPTION_Y);
  if (captionHeight < 0)  captionHeight = 50;
  screenSize -= wxSize(0, captionHeight);
  SetMaxSize(screenSize);
}

CompareDlg::~CompareDlg()
{
  delete m_otherCore;
  delete m_selCriteria;
  delete m_current;
  delete m_comparison;
  delete m_conflicts;
  delete m_identical;
}

void CompareDlg::CreateControls()
{
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);  //add a margin at the top

  m_dbSelectionPane = CreateDBSelectionPanel(dlgSizer);
  dlgSizer->AddSpacer(RowSeparation);
  m_optionsPane = CreateOptionsPanel(dlgSizer);
  dlgSizer->AddSpacer(RowSeparation);

  CreateDataPanel(dlgSizer, _("Conflicting items"), m_conflicts, true)->Hide();
  m_conflicts->sizerBelow = dlgSizer->AddSpacer(RowSeparation);
  m_conflicts->sizerBelow->Show(false);

  CreateDataPanel(dlgSizer, wxString() << _("Only in current database: ") << m_currentCore->GetCurFile(),
                        m_current)->Hide();
  m_current->sizerBelow = dlgSizer->AddSpacer(RowSeparation);
  m_current->sizerBelow->Show(false);

  CreateDataPanel(dlgSizer, wxString() << _("Only in comparison database: ") << m_otherCore->GetCurFile(),
                        m_comparison)->Hide();
  m_comparison->sizerBelow = dlgSizer->AddSpacer(RowSeparation);
  m_comparison->sizerBelow->Show(false);

  CreateDataPanel(dlgSizer, _("Identical items"), m_identical)->Hide();

  dlgSizer->AddSpacer(RowSeparation);

  m_dbSelectionPane->Expand();

  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Center().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(0));
  dlgSizer->AddSpacer(RowSeparation);
  wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer;
  buttons->Add(new wxButton(this, wxID_CANCEL));
  buttons->SetAffirmativeButton(new wxButton(this, ID_BTN_COMPARE, _("&Compare")));
  buttons->Realize();
  dlgSizer->Add(buttons, wxSizerFlags().Center().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(0));
  dlgSizer->AddSpacer(BottomMargin);

  Connect(wxEVT_SIZE, wxSizeEventHandler(CompareDlg::OnSize));

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
  wxString paneTitle = wxString() << _("Select a database to compare with current database (")
                                    << m_currentCore->GetCurFile() << wxT(')');

  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, paneTitle);
  wxWindow* paneWindow = pane->GetPane();

  wxBoxSizer* dbPanelSizer = new wxBoxSizer(wxVERTICAL);
  m_dbPanel = new DbSelectionPanel(paneWindow, wxString(_("Password Database")),
                                               wxString(_("Select a PasswordSafe database to compare")),
                                               true,
                                               m_currentCore,
                                               1);
  dbPanelSizer->Add(m_dbPanel, wxSizerFlags().Expand().Proportion(1));

  dlgSizer->Add(pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));

  paneWindow->SetSizer(dbPanelSizer);

  return pane;
}

wxCollapsiblePane* CompareDlg::CreateOptionsPanel(wxSizer* dlgSizer)
{
  //Create another collapsible pane to be nested within the top collapsible pane
  wxCollapsiblePane* optionsPane = new wxCollapsiblePane(this, wxID_ANY, _("Advanced Options..."));
  //Create the Advanced Selection Options panel with the pane's window as parent
  AdvancedSelectionPanel* advPanel = new AdvancedSelectionImpl<CompareDlgType>(optionsPane->GetPane(),
                                                                               *m_selCriteria,
                                                                               true);
  advPanel->CreateControls(optionsPane->GetPane());
  //Create a vertical box sizer
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  //and add the panel to it
  sizer->Add(advPanel, wxSizerFlags().Expand().Proportion(1));
  //Set it as the nested pane's sizer
  optionsPane->GetPane()->SetSizer(sizer);

  dlgSizer->Add(optionsPane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  return optionsPane;
}

wxCollapsiblePane* CompareDlg::CreateDataPanel(wxSizer* dlgSizer, const wxString& title, ComparisonData* cd,
                                                      bool customGrid /*=false*/)
{
  cd->pane = new wxCollapsiblePane(this, wxID_ANY, title);
  if (customGrid)
    cd->grid = new ComparisonGrid(cd->pane->GetPane(), wxID_ANY);
  else
    cd->grid = new wxGrid(cd->pane->GetPane(), wxID_ANY);
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(cd->grid, wxSizerFlags().Expand().Proportion(1));
  cd->pane->GetPane()->SetSizer(sizer);

  dlgSizer->Add(cd->pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));

  return cd->pane;
}

void CompareDlg::OnCompare(wxCommandEvent& )
{
  if ( Validate() && TransferDataFromWindow())
    DoCompare();
}

void CompareDlg::DoCompare()
{
  if (ReadCore(*m_otherCore, m_dbPanel->m_filepath,
                          m_dbPanel->m_combination,
                          true,
                          this) == PWScore::SUCCESS) {
    bool treatWhitespacesAsEmpty = false;
    m_currentCore->Compare(m_otherCore, 
                           m_selCriteria->GetSelectedFields(),
                           m_selCriteria->HasSubgroupRestriction(),
                           treatWhitespacesAsEmpty,
                           tostdstring(m_selCriteria->SubgroupSearchText()),
                           m_selCriteria->SubgroupObject(),
                           m_selCriteria->SubgroupFunction(),
                           m_current->data,
                           m_comparison->data,
                           m_conflicts->data,
                           m_identical->data);

    m_dbSelectionPane->Collapse();
    m_optionsPane->Collapse();
    struct {
      ComparisonData* cd;
      bool expand;
      bool multiSource;
      bool useComparisonSafe; ////unused if multisource is true
    } sections[] = { {m_conflicts,  true,  true,  false}, 
                     {m_current,    true,  false, false},
                     {m_comparison, true,  false, true},
                     {m_identical,  false, false, false}
                };
    wxSizerItem* prevSizer = 0;
    for(size_t idx =0; idx < WXSIZEOF(sections); ++idx) {
      if (!sections[idx].cd->data.empty()) {
        if (prevSizer)
          prevSizer->Show(true);
        ComparisonGridTable* table;
        if (sections[idx].multiSource) {
          table = new MultiSafeCompareGridTable(m_selCriteria,
                                                &sections[idx].cd->data,
                                                m_currentCore,
                                                m_otherCore);
        }
        else {
          table = new UniSafeCompareGridTable(m_selCriteria,
                                              &sections[idx].cd->data,
                                              sections[idx].useComparisonSafe? m_otherCore: m_currentCore,
                                              sections[idx].useComparisonSafe? &st_CompareData::uuid1: &st_CompareData::uuid0,
                                              sections[idx].useComparisonSafe? ComparisonBackgroundColor: CurrentBackgroundColor);
        }
        sections[idx].cd->grid->SetTable(table);
        wxCollapsiblePane* pane = sections[idx].cd->pane;
        /*
        // wxCollapsiblePane::GetLabel() doesn't work 
        wxString newLabel(pane->GetLabel());
        newLabel << wxT(" (") << sections[idx].cd->data.size() << wxT(")");
        pane->SetLabel(newLabel);
        */
        pane->Show();
        if (sections[idx].expand) {
          pane->Expand();
        }
        else {
          pane->Collapse();
        }
        //if the next pane is displayed, show the sizer below this pane
        prevSizer = sections[idx].cd->sizerBelow; 
      }
    }
    Layout();
  }
}

void CompareDlg::OnSize(wxSizeEvent& evt)
{
  wxSize screenSize = ::wxGetClientDisplayRect().GetSize();

  //assume taskbar/appbar/menubar is horizontal (at top or bottom)
  int captionHeight = ::wxSystemSettings::GetMetric(wxSYS_CAPTION_Y);
  if (captionHeight < 0)  captionHeight = 50;
  screenSize -= wxSize(0, captionHeight);

  const wxSize evtSize = evt.GetSize();
  if (evtSize.GetWidth() < screenSize.GetWidth() && evtSize.GetHeight() < screenSize.GetHeight())
    evt.Skip();
  else {
    wxSize bestSize = wxDialog::DoGetBestSize();

    if (bestSize.GetWidth() > screenSize.GetWidth())
      bestSize.SetWidth(screenSize.GetWidth()-1);

    if (bestSize.GetHeight() > screenSize.GetHeight())
      bestSize.SetHeight(screenSize.GetHeight()-1);

    wxSize windowSize = GetSize(), clientSize = GetClientSize();
    int horizOffset = windowSize.GetWidth() - clientSize.GetWidth();
    int vertOffset = windowSize.GetHeight() - clientSize.GetHeight();
    //GetSizer()->SetDimension(0, 0, bestSize.GetWidth() - horizOffset, bestSize.GetHeight() - vertOffset);
//    wxRect winRect = this->GetRect();
//    winRect.SetSize(bestSize);
//    SetSize(winRect);
    
    wxSizeEvent szEvt(bestSize, GetId());
    AddPendingEvent(szEvt);
//    Layout();
  }
}
