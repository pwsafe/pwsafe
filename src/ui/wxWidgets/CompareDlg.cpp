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
#include "./fieldselectiondlg.h"
#include <wx/statline.h>
#include <wx/grid.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

enum {ID_BTN_COMPARE = 100 };

enum {
  ID_COPY_FIELD_TO_CURRENT_DB = 100,
  ID_SYNC_ITEMS_WITH_CURRENT_DB,
  ID_COPY_ITEMS_TO_CURRENT_DB,
  ID_EDIT_IN_CURRENT_DB,
  ID_VIEW_IN_COMPARISON_DB,
  ID_DELETE_ITEMS_FROM_CURRENT_DB
};

DECLARE_EVENT_TYPE(EVT_EXPAND_DATA_PANELS, -1)
DEFINE_EVENT_TYPE(EVT_EXPAND_DATA_PANELS)

DECLARE_EVENT_TYPE(EVT_SELECT_GRID_ROW, -1)
DEFINE_EVENT_TYPE(EVT_SELECT_GRID_ROW)

BEGIN_EVENT_TABLE( CompareDlg, wxDialog )
  EVT_BUTTON( ID_BTN_COMPARE,  CompareDlg::OnCompare )
  EVT_GRID_CELL_RIGHT_CLICK(CompareDlg::OnGridCellRightClick)
  EVT_MENU(ID_EDIT_IN_CURRENT_DB, CompareDlg::OnEditInCurrentDB)
  EVT_MENU(ID_VIEW_IN_COMPARISON_DB, CompareDlg::OnViewInComparisonDB)
  EVT_COMMAND(wxID_ANY, EVT_EXPAND_DATA_PANELS, CompareDlg::OnExpandDataPanels)
  EVT_COMMAND(wxID_ANY, EVT_SELECT_GRID_ROW, CompareDlg::OnAutoSelectGridRow)
  EVT_MENU(ID_COPY_ITEMS_TO_CURRENT_DB, CompareDlg::OnCopyItemsToCurrentDB)
  EVT_MENU(ID_DELETE_ITEMS_FROM_CURRENT_DB, CompareDlg::OnDeleteItemsFromCurrentDB)
  EVT_MENU(ID_COPY_FIELD_TO_CURRENT_DB, CompareDlg::OnCopyFieldsToCurrentDB)
  EVT_MENU(ID_SYNC_ITEMS_WITH_CURRENT_DB, CompareDlg::OnSyncItemsWithCurrentDB)
END_EVENT_TABLE()

struct ComparisonData {
  wxCollapsiblePane* pane;
  wxGrid* grid;              //the grid is linked back to this object using SetClientData()
  CompareData data;
  wxSizerItem* sizerBelow;  //for managing the spacers in between
  
  ComparisonData(): pane(0), grid(0), sizerBelow(0){}
  ~ComparisonData() { /*nothing to do.  All window objects deleted automatically */ }
};

struct ContextMenuData {
  ComparisonData* cdata;
  wxArrayInt selectedRows;
  CItemData::FieldType field;
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

  m_conflicts->grid->Connect(wxEVT_GRID_RANGE_SELECT,
                             wxGridRangeSelectEventHandler(CompareDlg::OnGridRangeSelect),
                             NULL,
                             this);
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
  //create a way to get to the ComparisonData object from the grid, which is the only thing we have in events
  wxASSERT_MSG(cd->grid->GetClientData() == 0, wxT("wxGrid::ClientData is not NULL on creation.  Need to use that for our purposes"));
  cd->grid->SetClientData(cd);
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
      sections[idx].cd->grid->SetTable(table, true, wxGrid::wxGridSelectRows);
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
      if (!sections[idx].cd->data.empty()) {
        pane->Show();
        //if the next pane is displayed, show the sizer below this pane
        if (prevSizer)
          prevSizer->Show(true);
        prevSizer = sections[idx].cd->sizerBelow; 
      }
      else {
        pane->Collapse();
        pane->Hide();
      }
    }
    wxCommandEvent cmdEvent(EVT_EXPAND_DATA_PANELS, GetId());
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
}

void CompareDlg::OnGridRangeSelect(wxGridRangeSelectEvent& evt)
{
  wxCHECK_RET(evt.GetId() == m_conflicts->grid->GetId(), wxT("OnGridRangeSelect should be plugged-in only with conflicts grid"));
  //select grids asynchronously, or else we get in an infinite loop of selections & their notifications
  if (evt.GetTopRow()%2 != 0) {
    wxCommandEvent cmdEvent(EVT_SELECT_GRID_ROW);
    cmdEvent.SetEventObject(evt.GetEventObject());
    cmdEvent.SetInt(evt.GetTopRow()-1);
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
  if (evt.GetBottomRow()%2 == 0) {
    wxCommandEvent cmdEvent(EVT_SELECT_GRID_ROW);
    cmdEvent.SetEventObject(evt.GetEventObject());
    cmdEvent.SetInt(evt.GetBottomRow()+1);
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
}

void CompareDlg::OnAutoSelectGridRow(wxCommandEvent& evt)
{
  m_conflicts->grid->SelectRow(evt.GetInt(), true);
}

void CompareDlg::OnGridCellRightClick(wxGridEvent& evt)
{
  wxGrid* grid = wxDynamicCast(evt.GetEventObject(), wxGrid);
  if (!grid) {
    evt.Skip();
    return;
  }
  ComparisonData* cd = reinterpret_cast<ComparisonData*>(grid->GetClientData());
  wxCHECK_RET(cd, wxT("ClientData object not found in grid"));

  ContextMenuData menuContext;
  menuContext.cdata = cd;

  if (!menuContext.cdata->grid->IsInSelection(evt.GetRow(), evt.GetCol())) {
    menuContext.cdata->grid->SelectRow(evt.GetRow(), false);
  }

  if (menuContext.cdata == m_conflicts) {
    menuContext.cdata->grid->SelectRow(evt.GetRow()%2 == 0? evt.GetRow()+1: evt.GetRow()-1, true);
  }

  menuContext.cdata->grid->SetGridCursor(evt.GetRow(), evt.GetCol());

  menuContext.selectedRows = menuContext.cdata->grid->GetSelectedRows();
  int selectionCount = menuContext.selectedRows.GetCount();
  if (menuContext.cdata == m_conflicts) selectionCount /= 2;

  stringT itemStr;
  LoadAString(itemStr, selectionCount > 1? IDSC_ENTRIES: IDSC_ENTRY);

  wxString selCountStr(wxT(" "));
  if (selectionCount > 1)
    selCountStr << selectionCount << wxT(" ");

  wxMenu itemEditMenu;

  wxString strMergeItemsMenu;
  strMergeItemsMenu << _("Synchronize") << selCountStr << wxT("selected ") << towxstring(itemStr) << _(" with current db");
  itemEditMenu.Append(ID_SYNC_ITEMS_WITH_CURRENT_DB, strMergeItemsMenu);

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

  if (menuContext.cdata == m_conflicts) {
    wxString strCopyFieldMenu;
    ComparisonGridTable* table = wxDynamicCast(menuContext.cdata->grid->GetTable(), ComparisonGridTable);
    menuContext.field = table->ColumnToField(evt.GetCol());
    if (selectionCount > 1)
      strCopyFieldMenu << _("&Copy ") << selectionCount << wxT(" selected ") << 
                          towxstring(CItemData::FieldName(menuContext.field)) 
                          << _(" fields to current db");
    else
      strCopyFieldMenu << _("&Copy this ") << towxstring(CItemData::FieldName(menuContext.field)) << _(" to current db");
      
    itemEditMenu.Insert(0, ID_COPY_FIELD_TO_CURRENT_DB, strCopyFieldMenu);

    itemEditMenu.InsertSeparator(1);
    itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
  }
  else if (menuContext.cdata == m_current) {
    itemEditMenu.Delete(ID_SYNC_ITEMS_WITH_CURRENT_DB);
    itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
    if (selectionCount == 1)
      itemEditMenu.Delete(ID_VIEW_IN_COMPARISON_DB);
  }
  else if (menuContext.cdata == m_comparison) {
    itemEditMenu.Delete(ID_SYNC_ITEMS_WITH_CURRENT_DB);
    itemEditMenu.Delete(ID_DELETE_ITEMS_FROM_CURRENT_DB);
    if (selectionCount == 1)
      itemEditMenu.Delete(ID_EDIT_IN_CURRENT_DB);
  }
  else if (menuContext.cdata == m_identical) {
    itemEditMenu.Delete(ID_SYNC_ITEMS_WITH_CURRENT_DB);
    itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
  }

  // Make the menuContext object available to the handlers
  MenuEventModifier cem(&itemEditMenu, &menuContext);

  menuContext.cdata->grid->PopupMenu(&itemEditMenu);
}

void CompareDlg::OnEditInCurrentDB(wxCommandEvent& evt)
{
  ContextMenuData* menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  const ComparisonGridTable& table = *wxDynamicCast(menuContext->cdata->grid->GetTable(), ComparisonGridTable);
  const pws_os::CUUID& uuid = table[menuContext->selectedRows[0]].uuid0;
  if (ViewEditEntry(m_currentCore, uuid, false)) {
    int idx = menuContext->selectedRows[0];
    if (menuContext->cdata == m_conflicts)
      idx = idx/2;
    ItemListIter itr = m_currentCore->Find(uuid);
    if (itr != m_currentCore->GetEntryEndIter()) {
      const CItemData& item = itr->second;
      //we update these in the grid directly from st_CompareData, so keep track
      if (item.IsGroupSet())
        menuContext->cdata->data[idx].group = item.GetGroup();
      else
        menuContext->cdata->data[idx].group.clear();
      if (item.IsTitleSet())
        menuContext->cdata->data[idx].title = item.GetTitle();
      else
        menuContext->cdata->data[idx].title.clear();
      if (item.IsUserSet())
        menuContext->cdata->data[idx].user = item.GetUser();
      else
        menuContext->cdata->data[idx].user.clear();
      table.RefreshRow(table.GetItemRow(uuid));
    }
    else {
      wxFAIL_MSG(wxT("Could not find entry in core after editing it"));
    }
  }
}

void CompareDlg::OnViewInComparisonDB(wxCommandEvent& evt)
{
  ContextMenuData* menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  const ComparisonGridTable& table = *wxDynamicCast(menuContext->cdata->grid->GetTable(), ComparisonGridTable);
  const pws_os::CUUID& uuid = table[menuContext->selectedRows[0]].uuid1;
  wxCHECK_RET(ViewEditEntry(m_otherCore, uuid, true) == false, wxT("Should not need to refresh grid for just viewing entry"));
}

bool CompareDlg::ViewEditEntry(PWScore* core, const pws_os::CUUID& uuid, bool readOnly)
{
  AddEditPropSheet ae(this, 
                      *core,
                      readOnly? AddEditPropSheet::VIEW: AddEditPropSheet::EDIT,
                      &core->Find(uuid)->second);
  return ae.ShowModal() == wxID_OK && !readOnly;
}


void CompareDlg::OnExpandDataPanels(wxCommandEvent& /*evt*/)
{
  m_dbSelectionPane->Collapse();
  m_optionsPane->Collapse();

  if (m_conflicts->pane->IsShown())
    m_conflicts->pane->Expand();

  if (m_current->pane->IsShown())
    m_current->pane->Expand();

  if (m_comparison->pane->IsShown())
    m_comparison->pane->Expand();

  if (m_identical->pane->IsShown())
    m_identical->pane->Collapse();

  Layout();
}

void CompareDlg::OnCopyItemsToCurrentDB(wxCommandEvent& evt)
{
  ContextMenuData* menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxGridTableBase* baseTable = menuContext->cdata->grid->GetTable();
  ComparisonGridTable* ptable = wxDynamicCast(baseTable, ComparisonGridTable);
  wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
  const ComparisonGridTable& table = *ptable;
  MultiCommands *pmulticmds = MultiCommands::Create(m_currentCore);
  for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
    const int row = menuContext->selectedRows[idx];
    ItemListIter itrOther = m_otherCore->Find(table[row].uuid1);
    if (itrOther != m_otherCore->GetEntryEndIter()) {
      if (m_currentCore->Find(itrOther->second.GetUUID()) != m_currentCore->GetEntryEndIter()) {
        // if you copy an item from comparison grid to current db, edit the copy in current db and
        // change its GTU, it will appear in a different grid if you compare again, but have
        // the same UUID.  Then if you try to add the item from comparison db again, you'll get
        // an Assert failure.  I think it will work anyway in Release build, but abort in Debug
        CItemData newItem(itrOther->second);
        newItem.CreateUUID();
        AddEntryCommand* cmd = AddEntryCommand::Create(m_currentCore, newItem);
        pmulticmds->Add(cmd);
      }
      else {
        const CItemData& item = itrOther->second;
        AddEntryCommand* cmd = AddEntryCommand::Create(m_currentCore, item);
        pmulticmds->Add(cmd);
      }
    }
    else {
      wxFAIL_MSG(wxT("Could not find item to be added in comparison core"));
      delete pmulticmds;
      return;
    }
  }
  if (pmulticmds->GetSize() > 0) {
    m_currentCore->Execute(pmulticmds);
    for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
      const int row = menuContext->selectedRows[idx]-idx;
      st_CompareData data = table[row];
      data.uuid0 = data.uuid1; //so far, uuid0 was NULL since it was not found in current db
      m_identical->data.push_back(data);
      m_identical->grid->AppendRows(1);
      menuContext->cdata->grid->DeleteRows(row);
    }
    //Move copied items from comparison grid to identicals grid
    bool relayout = false;
    if (!m_identical->pane->IsShown()) {
      if (m_identical->sizerBelow)
        m_identical->sizerBelow->Show(true);
      m_identical->pane->Show();
      m_identical->pane->Expand();
      relayout = true;
    }
    if (m_comparison->grid->GetNumberRows() == 0) {
      if (m_comparison->sizerBelow)
        m_comparison->sizerBelow->Show(false);
      m_comparison->pane->Collapse();
      m_comparison->pane->Hide();
      relayout = true;
    }
    if (relayout)
      Layout();
  }
  else {
    delete pmulticmds;
  }
}


void CompareDlg::OnDeleteItemsFromCurrentDB(wxCommandEvent& evt)
{
  ContextMenuData* menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxGridTableBase* baseTable = menuContext->cdata->grid->GetTable();
  ComparisonGridTable* ptable = wxDynamicCast(baseTable, ComparisonGridTable);
  wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
  const ComparisonGridTable& table = *ptable;
  MultiCommands *pmulticmds = MultiCommands::Create(m_currentCore);
  for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
    const int row = menuContext->selectedRows[idx];
    ItemListIter itr = m_currentCore->Find(table[row].uuid0);
    if ( itr != m_currentCore->GetEntryEndIter()) {
      DeleteEntryCommand* cmd = DeleteEntryCommand::Create(m_currentCore, itr->second);
      pmulticmds->Add(cmd);
    }
    else {
      wxFAIL_MSG(wxT("Could not find item to be deleted in current core"));
      //something wrong - don't delete anything
      delete pmulticmds;
      return;
    }
  }
  if (pmulticmds->GetSize() > 0) {
    m_currentCore->Execute(pmulticmds);

    //need to delete the rows in order
    menuContext->selectedRows.Sort(&pless);

    bool relayout = false;
    if (menuContext->cdata == m_current) {
      //just delete them from the grid
      for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
        const int row = menuContext->selectedRows[idx] - idx;
        menuContext->cdata->grid->DeleteRows(row);
      }
    }
    else {
      wxCHECK_RET(menuContext->cdata == m_identical || menuContext->cdata == m_conflicts,
                      wxT("If deleted item was not in comparison grid, it should have been in conflicts or identicals grid"));
      // move items to comparison grid, and then delete the item
      for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
        const int row = menuContext->selectedRows[idx] - idx;
        st_CompareData data = table[row];
        data.uuid0 = pws_os::CUUID::NullUUID(); //removed from current db, so make it null
        m_comparison->data.push_back(data);
        m_comparison->grid->AppendRows(1);
        menuContext->cdata->grid->DeleteRows(row);
      }
      if (!m_comparison->pane->IsShown()) {
        m_comparison->pane->Show();
        m_comparison->pane->Expand();
        relayout = true;
      }
    }
    if (menuContext->cdata->grid->GetNumberRows() == 0) {
      relayout = true;
      //hide the pane
      menuContext->cdata->pane->Collapse();
      menuContext->cdata->pane->Hide();
    }
    if (relayout)
      Layout();
  }
  else {
    delete pmulticmds;
  }
}

void CompareDlg::OnCopyFieldsToCurrentDB(wxCommandEvent& evt)
{
  ContextMenuData* menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  ComparisonGridTable* ptable = wxDynamicCast(menuContext->cdata->grid->GetTable(), ComparisonGridTable);
  wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
  const ComparisonGridTable& table = *ptable;
  MultiCommands *pmulticmds = MultiCommands::Create(m_currentCore);
  for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
    const int row = menuContext->selectedRows[idx];
    ItemListIter itrOther = m_otherCore->Find(table[row].uuid1);
    if ( itrOther != m_otherCore->GetEntryEndIter()) {
      const CItemData& otherItem = itrOther->second;
      ItemListIter itrCurrent = m_currentCore->Find(table[row].uuid0);
      if (itrCurrent != m_currentCore->GetEntryEndIter()) {
        const CItemData& currentItem = itrCurrent->second;
        UpdateEntryCommand* cmd = UpdateEntryCommand::Create(m_currentCore, currentItem,
                                                            menuContext->field,
                                                            otherItem.GetFieldValue(menuContext->field));
        pmulticmds->Add(cmd);
      }
      else {
        wxFAIL_MSG(wxT("Could not find item to be modified in current core"));
        delete pmulticmds;
        return;
      }
    }
    else {
      wxFAIL_MSG(wxT("Could not find item to be modified in current core"));
      delete pmulticmds;
      return;
    }
  }
  if (pmulticmds->GetSize() > 0) {
    m_currentCore->Execute(pmulticmds);
    for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
      //refresh the row just above the selected one
      table.RefreshRow(menuContext->selectedRows[idx]-1);
    }
  }
  else {
    delete pmulticmds;
  }
}

void CompareDlg::OnSyncItemsWithCurrentDB(wxCommandEvent& evt)
{
  CItemData::FieldType syncFields[] = {
    CItemData::NOTES, CItemData::PASSWORD, CItemData::CTIME, CItemData::PMTIME, CItemData::ATIME, CItemData::XTIME,
    CItemData::RMTIME, CItemData::URL, CItemData::AUTOTYPE, CItemData::PWHIST, CItemData::POLICY, CItemData::XTIME_INT,
    CItemData::RUNCMD, CItemData::DCA, CItemData::EMAIL, CItemData::PROTECTED, CItemData::SYMBOLS, CItemData::SHIFTDCA,
    
  };
  
  FieldSet userSelection(syncFields, syncFields + WXSIZEOF(syncFields));
  
  FieldSelectionDlg dlg(this,
                        NULL, 0, //nothing is unselected by default
                        NULL, 0, //nothing is mandatory, either
                        userSelection,
                        _T("Synchronize"));
  if (dlg.ShowModal() == wxID_OK) {
  }
}
