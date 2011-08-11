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
#include "./SizeRestrictedPanel.h"
#include "../../core/core.h"
#include "./addeditpropsheet.h"
#include <wx/statline.h>
#include <wx/grid.h>

enum {ID_BTN_COMPARE = 100 };

enum {
  ID_COPY_FIELD_TO_CURRENT_DB = 100,
  ID_MERGE_ITEMS_WITH_CURRENT_DB,
  ID_COPY_ITEMS_TO_CURRENT_DB,
  ID_EDIT_IN_CURRENT_DB,
  ID_VIEW_IN_COMPARISON_DB,
  ID_DELETE_ITEMS_FROM_CURRENT_DB
};

DECLARE_EVENT_TYPE(EVT_EXPAND_DATA_PANELS, -1)
DEFINE_EVENT_TYPE(EVT_EXPAND_DATA_PANELS)

BEGIN_EVENT_TABLE( CompareDlg, wxDialog )
  EVT_BUTTON( ID_BTN_COMPARE,  CompareDlg::OnCompare )
  EVT_GRID_CELL_RIGHT_CLICK(CompareDlg::OnGridCellRightClick)
  EVT_MENU(ID_EDIT_IN_CURRENT_DB, CompareDlg::OnEditInCurrentDB)
  EVT_MENU(ID_VIEW_IN_COMPARISON_DB, CompareDlg::OnViewInComparisonDB)
  EVT_COMMAND(wxID_ANY, EVT_EXPAND_DATA_PANELS, CompareDlg::OnExpandDataPanels)
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
  wxButton* compareButton = new wxButton(this, ID_BTN_COMPARE, _("&Compare"));
  compareButton->SetDefault();
  buttons->SetAffirmativeButton(compareButton);
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
  SizeRestrictedPanel* sizedPanel = new SizeRestrictedPanel(cd->pane->GetPane(), cd->pane);

  //create the grid with SizeRestrictedPanel as the parent
  if (customGrid)
    cd->grid = new ComparisonGrid(sizedPanel, wxID_ANY);
  else
    cd->grid = new wxGrid(sizedPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0); //don't have wxWANTS_CHARS
#ifndef __WXMSW__
  wxFont monospacedFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  if (monospacedFont.IsFixedWidth())
    cd->grid->SetDefaultCellFont(monospacedFont);
#else
  cd->grid->SetDefaultCellFont(wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT));
#endif
  cd->grid->SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_BOTTOM);
  wxBoxSizer* gridSizer = new wxBoxSizer(wxVERTICAL);
  gridSizer->Add(cd->grid, wxSizerFlags().Expand().Proportion(1));
  sizedPanel->SetSizer(gridSizer);

  wxSizer* sizedPanelSizer = new wxBoxSizer(wxVERTICAL);
  sizedPanelSizer->Add(sizedPanel, wxSizerFlags().Expand().Proportion(1));
  cd->pane->GetPane()->SetSizer(sizedPanelSizer);

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
    m_current->data.clear();
    m_comparison->data.clear();
    m_conflicts->data.clear();
    m_identical->data.clear();
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
        sections[idx].cd->grid->SetTable(table, true);
        wxCollapsiblePane* pane = sections[idx].cd->pane;
        //expand the columns to show these fields fully, as these are usually small(er) strings
        table->AutoSizeField(CItemData::GROUP);
        table->AutoSizeField(CItemData::TITLE);
        table->AutoSizeField(CItemData::USER);
        table->AutoSizeField(CItemData::PASSWORD);
        /*
        // wxCollapsiblePane::GetLabel() doesn't work 
        wxString newLabel(pane->GetLabel());
        newLabel << wxT(" (") << sections[idx].cd->data.size() << wxT(")");
        pane->SetLabel(newLabel);
        */
        pane->Show();
        //if the next pane is displayed, show the sizer below this pane
        prevSizer = sections[idx].cd->sizerBelow; 
      }
      else {
        sections[idx].cd->pane->Hide();
      }
    }
    wxCommandEvent cmdEvent(EVT_EXPAND_DATA_PANELS, GetId());
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
}

wxGrid* CompareDlg::GetEventSourceGrid(int id)
{
  if (id == m_conflicts->grid->GetId())
    return m_conflicts->grid;
  else if (id == m_comparison->grid->GetId())
    return m_comparison->grid;
  else if (id == m_current->grid->GetId())
    return m_current->grid;
  else if (id == m_identical->grid->GetId())
    return m_identical->grid;
  else {
    wxFAIL_MSG(wxT("Unexpected grid id in CompareDlg's grid event handler"));
    return 0;
  }
}

void CompareDlg::OnGridCellRightClick(wxGridEvent& evt)
{
  wxGrid* sourceGrid = GetEventSourceGrid(evt.GetId());
  if (!sourceGrid) {
    evt.Skip();
    return;
  }

  if (!sourceGrid->IsInSelection(evt.GetRow(), evt.GetCol())) {
    sourceGrid->SelectRow(evt.GetRow(), false);
  }
  sourceGrid->SetGridCursor(evt.GetRow(), evt.GetCol());

  const int selectionCount = sourceGrid->GetSelectedRows().GetCount();
  stringT itemStr;
  LoadAString(itemStr, selectionCount > 1? IDSC_ENTRIES: IDSC_ENTRY);

  wxString selCountStr(wxT(" "));
  if (selectionCount > 1)
    selCountStr << selectionCount << wxT(" ");

  wxMenu itemEditMenu;

  wxString strMergeItemsMenu;
  strMergeItemsMenu << _("Merge") << selCountStr << wxT("selected ") << towxstring(itemStr) << _(" with current db");
  itemEditMenu.Append(ID_MERGE_ITEMS_WITH_CURRENT_DB, strMergeItemsMenu);

  wxString strCopyItemsMenu;
  strCopyItemsMenu << _("Copy") << selCountStr << wxT("selected ") << towxstring(itemStr) << _(" to current db");
  itemEditMenu.Append(ID_COPY_ITEMS_TO_CURRENT_DB, strCopyItemsMenu);

  wxString strDeleteItemsMenu;
  strDeleteItemsMenu << _("Delete") << selCountStr << wxT("selected ") << towxstring(itemStr) << _(" from current db");
  itemEditMenu.Append(ID_DELETE_ITEMS_FROM_CURRENT_DB, strDeleteItemsMenu);

  if (selectionCount == 1) {
    itemEditMenu.AppendSeparator();

    itemEditMenu.Append(ID_EDIT_IN_CURRENT_DB,   wxT("&Edit entry in current db"));
    itemEditMenu.Append(ID_VIEW_IN_COMPARISON_DB,   wxT("&View entry in comparison db"));
  }

  if (sourceGrid == m_conflicts->grid) {
    wxString strCopyFieldMenu;
    ComparisonGridTable* table = wxDynamicCast(sourceGrid->GetTable(), ComparisonGridTable);
    strCopyFieldMenu << _("&Copy ") << towxstring(CItemData::FieldName(table->ColumnToField(evt.GetCol()))) << _(" to current db");
    itemEditMenu.Insert(0, ID_COPY_FIELD_TO_CURRENT_DB, strCopyFieldMenu);

    itemEditMenu.InsertSeparator(1);
  }
  else if (sourceGrid == m_current->grid) {
    itemEditMenu.Delete(ID_MERGE_ITEMS_WITH_CURRENT_DB);
    itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
    itemEditMenu.Delete(ID_VIEW_IN_COMPARISON_DB);
  }
  else if (sourceGrid == m_comparison->grid) {
    itemEditMenu.Delete(ID_MERGE_ITEMS_WITH_CURRENT_DB);
    itemEditMenu.Delete(ID_DELETE_ITEMS_FROM_CURRENT_DB);
    itemEditMenu.Delete(ID_EDIT_IN_CURRENT_DB);
  }
  else if (sourceGrid == m_identical->grid) {
    itemEditMenu.Delete(ID_MERGE_ITEMS_WITH_CURRENT_DB);
    itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
  }
  sourceGrid->PopupMenu(&itemEditMenu);
}

wxGrid* GetGridFromEvent(wxCommandEvent& evt)
{
  wxMenu* sourceMenu = wxDynamicCast(evt.GetEventObject(), wxMenu);
  if (sourceMenu)
    return wxDynamicCast(sourceMenu->GetInvokingWindow(), wxGrid);
  else {
    wxFAIL_MSG(wxT("Could not determine source grid from event"));
    return 0;
  }
}

pws_os::CUUID CompareDlg::GetSelectedItemId(const wxGrid* grid, bool readOnly) const
{
  ComparisonGridTable* table = wxDynamicCast(grid->GetTable(), ComparisonGridTable);
  wxCHECK_MSG(table, pws_os::CUUID::NullUUID(), wxT("Comparison grid doesnot have ComparisonGridTable derived grid table"));
  return table->GetSelectedItemId(readOnly);
}

void CompareDlg::OnEditInCurrentDB(wxCommandEvent& evt)
{
  ViewEditSelectedEntry(GetGridFromEvent(evt), m_currentCore, false);
}

void CompareDlg::OnViewInComparisonDB(wxCommandEvent& evt)
{
  ViewEditSelectedEntry(GetGridFromEvent(evt), m_otherCore, true);
}

void CompareDlg::ViewEditSelectedEntry(wxGrid* sourceGrid, PWScore* core, bool readOnly)
{
  AddEditPropSheet ae(this, 
                      *core,
                      readOnly? AddEditPropSheet::VIEW: AddEditPropSheet::EDIT,
                      &core->Find(GetSelectedItemId(sourceGrid, readOnly))->second);
  ae.ShowModal();
}


void CompareDlg::OnExpandDataPanels(wxCommandEvent& evt)
{
  m_dbSelectionPane->Collapse();
  m_optionsPane->Collapse();

  if (m_conflicts->pane->IsShown())
    m_conflicts->pane->Expand();
  else
    m_conflicts->pane->Collapse();

  if (m_current->pane->IsShown())
    m_current->pane->Expand();
  else
    m_current->pane->Collapse();

  if (m_comparison->pane->IsShown())
    m_comparison->pane->Expand();
  else
    m_comparison->pane->Collapse();

  if (m_identical->pane->IsShown())
    m_identical->pane->Collapse();

  Layout();
}
