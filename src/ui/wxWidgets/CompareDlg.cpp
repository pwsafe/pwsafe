/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/filename.h>
#include <wx/grid.h>
#include <wx/ptr_scpd.h>
#include <wx/statline.h>

#include "core/core.h"
#include "core/PWScore.h"

#include "AddEditPropSheetDlg.h"
#include "AdvancedSelectionDlg.h"
#include "CompareDlg.h"
#include "ComparisonGridTable.h"
#include "DbSelectionPanel.h"
#include "FieldSelectionDlg.h"
#include "SelectionCriteria.h"
#include "SizeRestrictedPanel.h"
#include "core/StringX.h"
#include "ViewReportDlg.h"

enum {ID_BTN_COMPARE = 100,
      ID_BTN_REPORT = 102
};

enum {
  ID_COPY_FIELD_TO_CURRENT_DB = 100,
  ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB,
  ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB,
  ID_COPY_ITEMS_TO_CURRENT_DB,
  ID_EDIT_IN_CURRENT_DB,
  ID_VIEW_IN_COMPARISON_DB,
  ID_DELETE_ITEMS_FROM_CURRENT_DB
};

DEFINE_EVENT_TYPE(EVT_START_COMPARISON)

DEFINE_EVENT_TYPE(EVT_EXPAND_DATA_PANELS)

BEGIN_EVENT_TABLE( CompareDlg, wxDialog )
  EVT_BUTTON( ID_BTN_COMPARE,  CompareDlg::OnCompare )
  EVT_BUTTON( ID_BTN_REPORT,  CompareDlg::OnShowReport )
  EVT_GRID_CELL_RIGHT_CLICK(CompareDlg::OnGridCellRightClick)
  EVT_MENU(ID_EDIT_IN_CURRENT_DB, CompareDlg::OnEditInCurrentDB)
  EVT_MENU(ID_VIEW_IN_COMPARISON_DB, CompareDlg::OnViewInComparisonDB)
  EVT_COMMAND(wxID_ANY, EVT_EXPAND_DATA_PANELS, CompareDlg::OnExpandDataPanels)
  EVT_COMMAND(wxID_ANY, EVT_START_COMPARISON, CompareDlg::DoCompare)
  EVT_MENU(ID_COPY_ITEMS_TO_CURRENT_DB, CompareDlg::OnCopyItemsToCurrentDB)
  EVT_MENU(ID_DELETE_ITEMS_FROM_CURRENT_DB, CompareDlg::OnDeleteItemsFromCurrentDB)
  EVT_MENU(ID_COPY_FIELD_TO_CURRENT_DB, CompareDlg::OnCopyFieldsToCurrentDB)
  EVT_MENU(ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB, CompareDlg::OnSyncItemsWithCurrentDB)
  EVT_MENU(ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB, CompareDlg::OnSyncItemsWithCurrentDB)

  EVT_UPDATE_UI( ID_BTN_REPORT, CompareDlg::OnUpdateUI )
END_EVENT_TABLE()

struct ComparisonData {
  wxCollapsiblePane* pane;
  wxGrid* grid;              //the grid is linked back to this object using SetClientData()
  CompareData data;
  wxSizerItem* sizerBelow;  //for managing the spacers in between

  ComparisonData(): pane(nullptr), grid(nullptr), sizerBelow(nullptr){}
  ~ComparisonData() { /*nothing to do.  All window objects deleted automatically */ }
};

wxDEFINE_SCOPED_PTR_TYPE(MultiCommands)

CompareDlg::CompareDlg(wxWindow *parent, PWScore* currentCore): wxDialog(parent,
                                                        wxID_ANY,
                                                        _("Compare current database with another database"),
                                                        wxDefaultPosition,
                                                        wxDefaultSize,
                                                        wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX),
                                                        m_currentCore(currentCore),
                                                        m_otherCore(new PWSAuxCore),
                                                        m_selCriteria(new SelectionCriteria),
                                                        m_dbPanel(nullptr),
                                                        m_dbSelectionPane(nullptr),
                                                        m_optionsPane(nullptr),
                                                        m_current(new ComparisonData),
                                                        m_comparison(new ComparisonData),
                                                        m_conflicts(new ComparisonData),
                                                        m_identical(new ComparisonData)
{
  wxASSERT(!parent || parent->IsTopLevel());

  SetExtraStyle(GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);

  //compare all fields by default
  m_selCriteria->SelectAllFields();

  CreateControls();
}

CompareDlg* CompareDlg::Create(wxWindow *parent, PWScore* core)
{
  return new CompareDlg(parent, core);
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
  auto *dlgSizer = new wxBoxSizer(wxVERTICAL);
  wxASSERT(dlgSizer);
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

  dlgSizer->AddStretchSpacer();
  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(0));
  dlgSizer->AddSpacer(RowSeparation);
  auto *buttons = new wxStdDialogButtonSizer;
  wxASSERT(buttons);
  buttons->Add(new wxButton(this, wxID_CANCEL));
  wxButton* reportButton = new wxButton(this, ID_BTN_REPORT, _("&View Report")); // ID_BTN_REPORT is not in allowed list, but is working...
  wxASSERT(reportButton);
  reportButton->Enable(false);
  buttons->Add(reportButton);
  m_compareButton = new wxButton(this, ID_BTN_COMPARE, _("&Compare"));
  wxASSERT(m_compareButton);
  m_compareButton->SetDefault();
  buttons->SetAffirmativeButton(m_compareButton);
  buttons->Realize();
  dlgSizer->Add(buttons, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT, SideMargin).Proportion(0));
  dlgSizer->AddSpacer(BottomMargin);

  SetSizerAndFit(dlgSizer);
}

struct CompareDlgType {
  static bool IsMandatoryField(CItemData::FieldType field) {
    return field == CItemData::GROUP || field == CItemData::TITLE || field == CItemData::USER;
  }

  static bool IsPreselectedField(CItemData::FieldType field) {
    switch (field) {
      case CItemData::DCA:
      case CItemData::SHIFTDCA:
      case CItemData::CTIME:
      case CItemData::PMTIME:
      case CItemData::ATIME:
      case CItemData::RMTIME:
      case CItemData::XTIME:
      case CItemData::XTIME_INT:
        return false;
      default:
        return true;
    }
  }

  static bool IsUsableField(CItemData::FieldType WXUNUSED(field)) {
    return true;
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

  auto *dbPanelSizer = new wxBoxSizer(wxVERTICAL);
  m_dbPanel = new DbSelectionPanel(paneWindow, wxString(_("Password Database")),
                                               wxString(_("Select a PasswordSafe database to compare")),
                                               true,
                                               m_currentCore,
                                               1,
                                               ID_BTN_COMPARE);
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
                                                                               m_selCriteria,
                                                                               true);
  advPanel->CreateControls(optionsPane->GetPane());
  //Create a vertical box sizer
  auto *sizer = new wxBoxSizer(wxVERTICAL);
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
  auto *sizedPanel = new SizeRestrictedPanel(cd->pane->GetPane(), cd->pane);

  //create the grid with SizeRestrictedPanel as the parent
  if (customGrid)
    cd->grid = new ComparisonGrid(sizedPanel, wxID_ANY);
  else
    cd->grid = new wxGrid(sizedPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0); //don't have wxWANTS_CHARS

  // this grid is presentation only
  cd->grid->EnableEditing(false);

  //create a way to get to the ComparisonData object from the grid, which is the only thing we have in events
  wxASSERT_MSG(cd->grid->GetClientData() == nullptr, wxT("wxGrid::ClientData is not nullptr on creation.  Need to use that for our purposes"));
  cd->grid->SetClientData(cd);
#if defined(__WXMSW__) || defined(__WXOSX__)
  cd->grid->SetDefaultCellFont(wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT));
#else
  wxFont monospacedFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  if (monospacedFont.IsFixedWidth())
    cd->grid->SetDefaultCellFont(monospacedFont);
#endif
  cd->grid->SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_BOTTOM);
  auto *gridSizer = new wxBoxSizer(wxVERTICAL);
  gridSizer->Add(cd->grid, wxSizerFlags().Expand().Proportion(1));
  sizedPanel->SetSizer(gridSizer);

  wxSizer* sizedPanelSizer = new wxBoxSizer(wxVERTICAL);
  sizedPanelSizer->Add(sizedPanel, wxSizerFlags().Expand().Proportion(1));
  cd->pane->GetPane()->SetSizer(sizedPanelSizer);

  dlgSizer->Add(cd->pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));

  return cd->pane;
}

void CompareDlg::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId()) {
    case ID_BTN_REPORT:
      event.Enable(!m_compReport.StringEmpty());
      break;
  }
}

void CompareDlg::OnCompare(wxCommandEvent& )
{
  if ( Validate() && TransferDataFromWindow()) {
    if (wxFileName(m_dbPanel->m_filepath).SameAs(towxstring(m_otherCore->GetCurFile())) ||
        ReadCore(*m_otherCore, m_dbPanel->m_filepath, m_dbPanel->m_combination,
                 true, this, true) == PWScore::SUCCESS) {
      m_otherCore->SetCurFile(tostringx(m_dbPanel->m_filepath));
      m_otherCore->SetReadOnly(true);

      // TODO: must copy the selectionCriteria from advanced options pane.  Or else
      // the search would always be conducted on default field criteria
      m_current->data.clear();
      m_comparison->data.clear();
      m_conflicts->data.clear();
      m_identical->data.clear();

      m_conflicts->pane->Collapse();
      m_current->pane->Collapse();
      m_comparison->pane->Collapse();
      m_identical->pane->Collapse();

      wxCommandEvent cmdEvent(EVT_START_COMPARISON, GetId());
      GetEventHandler()->AddPendingEvent(cmdEvent);
    }
  }
  else {
    m_otherCore->SetCurFile(StringX());
  }
}

void CompareDlg::OnShowReport(wxCommandEvent& )
{
  CallAfter(&CompareDlg::DoShowReport);
}
void CompareDlg::DoShowReport()
{
  ShowModalAndGetResult<ViewReportDlg>(this, &m_compReport);
}

void CompareDlg::DoCompare(wxCommandEvent& WXUNUSED(evt))
{
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
  wxSizerItem* prevSizer = nullptr;
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
      
      if (sections[idx].cd->data.size() == 1) {
        // If only one row the slider will be too big, increase minimum size of window to add place for a slider
        wxScreenDC dc;
        wxCoord width, height;
        dc.SetFont(sections[idx].cd->grid->GetDefaultCellFont());
        dc.GetTextExtent("T", &width, &height);
        wxSize gridPanelSize(wxDefaultCoord, 11 * height); // Width, Height - have not found a better way to calculate
        pane->SetMinSize(gridPanelSize);
      }
    }
    else {
      pane->Collapse();
      pane->Hide();
    }
  }
  wxCommandEvent cmdEvent(EVT_EXPAND_DATA_PANELS, GetId());
  GetEventHandler()->AddPendingEvent(cmdEvent);
  
  WriteReport();
}

void CompareDlg::OnGridCellRightClick(wxGridEvent& evt)
{
  wxGrid* grid = wxDynamicCast(evt.GetEventObject(), wxGrid);
  if (!grid) {
    evt.Skip();
    return;
  }
  auto *cd = reinterpret_cast<ComparisonData*>(grid->GetClientData());
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
  menuContext.selectedItems = menuContext.selectedRows;
  size_t selectionCount = menuContext.selectedRows.GetCount();
  if (menuContext.cdata == m_conflicts) {
    selectionCount /= 2;
    wxCHECK_RET(menuContext.selectedItems.GetCount()%2 ==0, wxT("Conflicts grid should always select an even number of items"));
    //Our algorithm requires the indexes to be in order, and sometimes these are actually unsorted
    menuContext.selectedItems.Sort(pless);
    for( size_t idx = 1; idx <= selectionCount; ++idx) {
      wxCHECK_RET(menuContext.selectedItems[idx]%2 != 0, wxT("Selection indexes not in expected order"));
      wxLogDebug( wxString() << wxT("Removing index ") << menuContext.selectedItems.Item(idx) << wxT(" from selection at index ") << idx << wxT('\n'));
      menuContext.selectedItems.RemoveAt(idx, 1);
    }
    for( size_t idx = 0; idx < selectionCount; ++idx) {
      wxLogDebug(wxString() << wxT("Found index ") << menuContext.selectedItems.Item(idx) << wxT(" from selection at ") << idx << wxT('\n'));
      wxCHECK_RET(menuContext.selectedItems[idx]%2 == 0, wxT("Conflicts grid selection should only have even indexes after normalization"));
      menuContext.selectedItems[idx] /= 2;
    }
  }
  
  bool bDbIsRW = ! m_currentCore->IsReadOnly();

  stringT itemStr;
  LoadAString(itemStr, selectionCount > 1? IDSC_ENTRIES: IDSC_ENTRY);

  wxString selCountStr(wxT(" "));
  if (selectionCount > 1)
    selCountStr << selectionCount << wxT(" ");

  wxMenu itemEditMenu;

  wxString strSyncSelectedItemsMenu;
  if (selectionCount == 1)
    strSyncSelectedItemsMenu << _("Synchronize this item...");
  else
    strSyncSelectedItemsMenu << _("Synchronize") << selCountStr << _("selected ") << towxstring(itemStr) << _("...");
  if(bDbIsRW)
    itemEditMenu.Append(ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB, strSyncSelectedItemsMenu);

  if(bDbIsRW)
    itemEditMenu.Append(ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB, _("Synchronize all items..."));

  wxString strCopyItemsMenu;
  strCopyItemsMenu << _("Copy") << selCountStr << _("selected ") << towxstring(itemStr) << _(" to current db");
  if(bDbIsRW)
    itemEditMenu.Append(ID_COPY_ITEMS_TO_CURRENT_DB, strCopyItemsMenu);

  wxString strDeleteItemsMenu;
  strDeleteItemsMenu << _("Delete") << selCountStr << _("selected ") << towxstring(itemStr) << _(" from current db");
  if(bDbIsRW)
    itemEditMenu.Append(ID_DELETE_ITEMS_FROM_CURRENT_DB, strDeleteItemsMenu);

  if (selectionCount == 1) {
    if(bDbIsRW)
      itemEditMenu.AppendSeparator();

    itemEditMenu.Append(ID_EDIT_IN_CURRENT_DB, m_currentCore->IsReadOnly() ?   _("&View entry in current db") :  _("&Edit entry in current db"));
    itemEditMenu.Append(ID_VIEW_IN_COMPARISON_DB,   _("&View entry in comparison db"));
  }

  if (menuContext.cdata == m_conflicts) {
    wxString strCopyFieldMenu;
    ComparisonGridTable* table = wxDynamicCast(menuContext.cdata->grid->GetTable(), ComparisonGridTable);
    menuContext.field = table->ColumnToField(evt.GetCol());
    if (selectionCount > 1)
      strCopyFieldMenu << _("&Copy ") << selectionCount << _(" selected ") <<
        towxstring(CItemData::FieldName(menuContext.field))
                       << _(" fields to current db");
    else
      strCopyFieldMenu << _("&Copy this ") << towxstring(CItemData::FieldName(menuContext.field)) << _(" to current db");

    if(bDbIsRW) {
      itemEditMenu.Insert(0, ID_COPY_FIELD_TO_CURRENT_DB, strCopyFieldMenu);

      itemEditMenu.InsertSeparator(1);
      itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
    }
  }
  else if (menuContext.cdata == m_current) {
    if(bDbIsRW) {
      itemEditMenu.Delete(ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB);
      itemEditMenu.Delete(ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB);
      itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
    }
    if (selectionCount == 1)
      itemEditMenu.Delete(ID_VIEW_IN_COMPARISON_DB);
  }
  else if (menuContext.cdata == m_comparison) {
    if(bDbIsRW) {
      itemEditMenu.Delete(ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB);
      itemEditMenu.Delete(ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB);
      itemEditMenu.Delete(ID_DELETE_ITEMS_FROM_CURRENT_DB);
    }
    if (selectionCount == 1)
      itemEditMenu.Delete(ID_EDIT_IN_CURRENT_DB);
  }
  else if (menuContext.cdata == m_identical) {
    if(bDbIsRW) {
      itemEditMenu.Delete(ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB);
      itemEditMenu.Delete(ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB);
      itemEditMenu.Delete(ID_COPY_ITEMS_TO_CURRENT_DB);
    }
  }

  // Make the menuContext object available to the handlers
  EventDataInjector inject(&itemEditMenu, &menuContext, wxEVT_COMMAND_MENU_SELECTED);

  menuContext.cdata->grid->PopupMenu(&itemEditMenu);
}

void CompareDlg::OnEditInCurrentDB(wxCommandEvent& evt)
{
  auto *menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxCHECK_RET(menuContext, wxT("No menu context available"));
  CallAfter(&CompareDlg::DoEditInCurrentDB, *menuContext);
}

void CompareDlg::DoEditInCurrentDB(ContextMenuData menuContext) {
  const ComparisonGridTable& table = *wxDynamicCast(menuContext.cdata->grid->GetTable(), ComparisonGridTable);
  const pws_os::CUUID& uuid = table[menuContext.selectedRows[0]].uuid0;
  
  if (ViewEditEntry(m_currentCore, uuid, m_currentCore->IsReadOnly())) {
    int idx = menuContext.selectedRows[0];
    if (menuContext.cdata == m_conflicts)
      idx = idx/2;
    auto itr = m_currentCore->Find(uuid);
    if (itr != m_currentCore->GetEntryEndIter()) {
      const CItemData& item = itr->second;
      //we update these in the grid directly from st_CompareData, so keep track
      if (item.IsGroupSet())
        menuContext.cdata->data[idx].group = item.GetGroup();
      else
        menuContext.cdata->data[idx].group.clear();
      if (item.IsTitleSet())
        menuContext.cdata->data[idx].title = item.GetTitle();
      else
        menuContext.cdata->data[idx].title.clear();
      if (item.IsUserSet())
        menuContext.cdata->data[idx].user = item.GetUser();
      else
        menuContext.cdata->data[idx].user.clear();
      table.RefreshRow(table.GetItemRow(uuid));
    }
    else {
      wxFAIL_MSG(wxT("Could not find entry in core after editing it"));
    }
    WriteReport();
  }
}

void CompareDlg::OnViewInComparisonDB(wxCommandEvent& evt)
{
  auto *menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxCHECK_RET(menuContext, wxT("Empty client data"));
  CallAfter(&CompareDlg::DoViewInComparisonDB, *menuContext);
}

void CompareDlg::DoViewInComparisonDB(ContextMenuData menuContext)
{
  const ComparisonGridTable& table = *wxDynamicCast(menuContext.cdata->grid->GetTable(), ComparisonGridTable);
  const pws_os::CUUID& uuid = table[menuContext.selectedRows[0]].uuid1;

  wxCHECK_RET(!ViewEditEntry(m_otherCore, uuid, true), wxT("Should not need to refresh grid for just viewing entry"));
}

bool CompareDlg::ViewEditEntry(PWScore* core, const pws_os::CUUID& uuid, bool readOnly)
{
  int rc = ShowModalAndGetResult<AddEditPropSheetDlg>(this, *core,
                      readOnly? AddEditPropSheetDlg::SheetType::VIEW: AddEditPropSheetDlg::SheetType::EDIT,
                      &core->Find(uuid)->second);
  return rc == wxID_OK && !readOnly;
}

void CompareDlg::OnExpandDataPanels(wxCommandEvent& WXUNUSED(evt))
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
  auto *menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxGridTableBase* baseTable = menuContext->cdata->grid->GetTable();
  ComparisonGridTable* ptable = wxDynamicCast(baseTable, ComparisonGridTable);
  wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
  const ComparisonGridTable& table = *ptable;
  MultiCommandsPtr pmulticmds(MultiCommands::Create(m_currentCore));
  for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
    const int row = menuContext->selectedRows[idx];
    auto itrOther = m_otherCore->Find(table[row].uuid1);
    wxCHECK_RET(itrOther != m_otherCore->GetEntryEndIter(), wxT("Could not find item to be added in comparison core"));
    if (m_currentCore->Find(itrOther->second.GetUUID()) != m_currentCore->GetEntryEndIter()) {
      // if you copy an item from comparison grid to current db, edit the copy in current db and
      // change its GTU, it will appear in a different grid if you compare again, but have
      // the same UUID.  Then if you try to add the item from comparison db again, you'll get
      // an Assert failure.  I think it will work anyway in Release build, but abort in Debug
      CItemData newItem(itrOther->second);
      newItem.CreateUUID();
      AddEntryCommand* cmd = AddEntryCommand::Create(m_currentCore, newItem, newItem.GetBaseUUID());
      pmulticmds->Add(cmd);
    }
    else {
      const CItemData& item = itrOther->second;
      AddEntryCommand* cmd = AddEntryCommand::Create(m_currentCore, item, item.GetBaseUUID());
      pmulticmds->Add(cmd);
    }
  }
  if (pmulticmds->GetSize() > 0) {
    m_currentCore->Execute(pmulticmds.release());
    for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
      const int row = static_cast<int>(menuContext->selectedRows[idx]-idx);
      st_CompareData data = table[row];
      data.uuid0 = data.uuid1; //so far, uuid0 was nullptr since it was not found in current db
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
  
  WriteReport();
}

void CompareDlg::OnDeleteItemsFromCurrentDB(wxCommandEvent& evt)
{
  auto *menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxGridTableBase* baseTable = menuContext->cdata->grid->GetTable();
  ComparisonGridTable* ptable = wxDynamicCast(baseTable, ComparisonGridTable);
  wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
  const ComparisonGridTable& table = *ptable;
  MultiCommandsPtr pmulticmds(MultiCommands::Create(m_currentCore));
  for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
    const int row = menuContext->selectedRows[idx];
    auto itr = m_currentCore->Find(table[row].uuid0);
    wxCHECK_RET( itr != m_currentCore->GetEntryEndIter(), wxT("Could not find item to be deleted in current core"));
    DeleteEntryCommand* cmd = DeleteEntryCommand::Create(m_currentCore, itr->second);
    pmulticmds->Add(cmd);
  }
  if (pmulticmds->GetSize() > 0) {
    m_currentCore->Execute(pmulticmds.release());

    //need to delete the rows in order
    menuContext->selectedRows.Sort(&pless);

    bool relayout = false;
    if (menuContext->cdata == m_current) {
      //just delete them from the grid
      for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
        const int row = static_cast<int>(menuContext->selectedRows[idx] - idx);
        menuContext->cdata->grid->DeleteRows(row);
      }
    }
    else {
      wxCHECK_RET(menuContext->cdata == m_identical || menuContext->cdata == m_conflicts,
                      wxT("If deleted item was not in comparison grid, it should have been in conflicts or identicals grid"));
      // move items to comparison grid, and then delete the item
      for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
        const int row = static_cast<int>(menuContext->selectedRows[idx] - idx);
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
  
  WriteReport();
}

void CompareDlg::OnCopyFieldsToCurrentDB(wxCommandEvent& evt)
{
  auto *menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxCHECK_RET(menuContext, wxT("No menu context available"));
  ComparisonGridTable* ptable = wxDynamicCast(menuContext->cdata->grid->GetTable(), ComparisonGridTable);
  wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
  const ComparisonGridTable& table = *ptable;
  MultiCommandsPtr pmulticmds(MultiCommands::Create(m_currentCore));
  for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
    const int row = menuContext->selectedRows[idx];
    auto itrOther = m_otherCore->Find(table[row].uuid1);
    wxCHECK_RET( itrOther != m_otherCore->GetEntryEndIter(), wxT("Could not find item to be modified in current core"));
    const CItemData& otherItem = itrOther->second;
    auto itrCurrent = m_currentCore->Find(table[row].uuid0);
    wxCHECK_RET(itrCurrent != m_currentCore->GetEntryEndIter(), wxT("Could not find item to be modified in current core"));
    const CItemData& currentItem = itrCurrent->second;
    UpdateEntryCommand* cmd = UpdateEntryCommand::Create(m_currentCore, currentItem,
                                                        menuContext->field,
                                                        otherItem.GetFieldValue(menuContext->field));
    pmulticmds->Add(cmd);
  }
  if (pmulticmds->GetSize() > 0) {
    m_currentCore->Execute(pmulticmds.release());
    for( size_t idx = 0; idx < menuContext->selectedRows.Count(); ++idx) {
      //refresh the row just above the selected one
      table.RefreshRow(menuContext->selectedRows[idx]-1);
    }
  }
  
  WriteReport();
}

void CompareDlg::OnSyncItemsWithCurrentDB(wxCommandEvent& evt)
{
  auto *menuContext = reinterpret_cast<ContextMenuData*>(evt.GetClientData());
  wxCHECK_RET(menuContext, wxT("No menu context available"));
  CallAfter(&CompareDlg::DoSyncItemsWithCurrentDB, evt.GetId(), *menuContext);
}

void CompareDlg::DoSyncItemsWithCurrentDB(int menuId, ContextMenuData menuContext)
{
  if (m_currentCore->IsReadOnly()) {
    wxMessageBox(_("Current safe was opened read-only"), _("Synchronize"), wxOK|wxICON_INFORMATION, this);
    return;
  }

  GTUSet setGTU;
  if (!m_currentCore->GetUniqueGTUValidated() && !m_currentCore->InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    wxMessageBox(wxString::Format(_("The database:\n\n%ls\n\nhas duplicate entries with the same group/title/user combination. You can fix this by validating the database.").c_str(), m_currentCore->GetCurFile().c_str()),
                  _("Synchronization failed"), wxOK|wxICON_EXCLAMATION, this);
    return;
  }
  setGTU.clear();  // Don't need it anymore - so clear it now

  //we only synchronize these fields
  const CItemData::FieldType syncFields[] = {
    CItemData::PASSWORD, CItemData::URL, CItemData::EMAIL, CItemData::AUTOTYPE, CItemData::NOTES,
    CItemData::PWHIST, CItemData::POLICY, CItemData::CTIME, CItemData::PMTIME, CItemData::ATIME,
    CItemData::XTIME, CItemData::RMTIME, CItemData::XTIME_INT,
    CItemData::RUNCMD, CItemData::DCA, CItemData::PROTECTED, CItemData::SYMBOLS, CItemData::SHIFTDCA,
  };

  FieldSet userSelection(syncFields, syncFields + WXSIZEOF(syncFields));

  //let the user choose which fields to synchronize
  int rc = ShowModalAndGetResult<FieldSelectionDlg>(this,
                        nullptr, 0, //no fields are left unselected by default
                        nullptr, 0, //But no fields are mandatory
                        userSelection,
                        _T("Synchronize"),
                        _("Synchronize items"),
                        _("Select fields to synchronize"),
                        _("You must select some fields to synchronize"),
                        _("Synchronize items"));
  if (rc == wxID_OK) {
    wxCHECK_RET(!userSelection.empty(), wxT("User did not select any fields to sync?"));
    //start with the selected items
    wxArrayInt syncIndexes(menuContext.selectedItems);
    if (menuId == ID_SYNC_ALL_ITEMS_WITH_CURRENT_DB) {
      //add all items to the sync Index list
      syncIndexes.Empty();
      const size_t numIndexes = menuContext.cdata->data.size();
      syncIndexes.Alloc(numIndexes);
      for(size_t i = 0; i < numIndexes; ++i)
        syncIndexes.Add(static_cast<int>(i));
    }
    else {
      wxCHECK_RET(menuId == ID_SYNC_SELECTED_ITEMS_WITH_CURRENT_DB, wxT("Sync menu id is neither for all nor for selected items"));
    }

    //use a wxScopedPtr to clean up the heap object if we trip on any of the wxCHECK_RETs below
    MultiCommandsPtr pMultiCmds(MultiCommands::Create(m_currentCore));
    for (size_t idx = 0; idx < syncIndexes.Count(); ++idx) {
      auto fromPos = m_otherCore->Find(menuContext.cdata->data[syncIndexes[idx]].uuid1);
      wxCHECK_RET(fromPos != m_otherCore->GetEntryEndIter(), wxT("Could not find sync item in other db"));
      const CItemData *pfromEntry = &fromPos->second;

      auto toPos = m_currentCore->Find(menuContext.cdata->data[syncIndexes[idx]].uuid0);
      wxCHECK_RET(toPos != m_currentCore->GetEntryEndIter(), wxT("Could not find sync item in current db"));
      CItemData *ptoEntry = &toPos->second;
      CItemData updtEntry(*ptoEntry);

      //create a copy of the "to" object, with only the to-be-sync'ed fields changed
      bool bUpdated(false);
      for (FieldSet::const_iterator itr = userSelection.begin(); itr != userSelection.end(); ++itr) {
        const StringX sxValue = pfromEntry->GetFieldValue(*itr);
        if (sxValue != updtEntry.GetFieldValue(*itr)) {
          bUpdated = true;
          updtEntry.SetFieldValue(*itr, sxValue);
        }
      }
      //Don't change anything yet.  Just keep track of the differences.
      if (bUpdated) {
        updtEntry.SetStatus(CItemData::ES_MODIFIED);
        Command *pcmd = EditEntryCommand::Create(m_currentCore, *ptoEntry, updtEntry);
        pMultiCmds->Add(pcmd);
      }
    }
    //Update all or nothing.  And there's no way of knowing if any of the sub-commands
    //inside MultiCommands failed
    if (pMultiCmds->GetSize() > 0) {
      m_currentCore->Execute(pMultiCmds.release());
      ComparisonGridTable* ptable = wxDynamicCast(menuContext.cdata->grid->GetTable(), ComparisonGridTable);
      wxCHECK_RET(ptable, wxT("Could not find ComparisonGridTable derived object in comparison grid"));
      const ComparisonGridTable& table = *ptable;
      wxCHECK_RET(menuContext.cdata == m_conflicts, wxT("Sync happened in unexpected grid"));
      for( size_t idx = 0; idx < syncIndexes.Count(); ++idx) {
        //refresh every even-numbered row
        table.RefreshRow(syncIndexes[idx]*2);
      }
    }
    WriteReport();
  }
}

void CompareDlg::WriteReportData()
{
  CompareData::iterator cd_iter;
  stringT line, delimiter = L"";
  
  ReportAdvancedOptions();
  
  if (m_current && m_current->data.empty() &&
      m_comparison && m_comparison->data.empty() &&
      m_conflicts && m_conflicts->data.empty()) {
    m_compReport.WriteLine(_("\n\nThe databases are identical when comparing the selected or default fields!"));
    m_compReport.EndReport();
    return;
  }
  
  if (m_current && !m_current->data.empty()) {

    Format(line, _("Entries only in current database (%ls):").c_str(), m_currentCore->GetCurFile().c_str());
    m_compReport.WriteLine(line);
    for (cd_iter = m_current->data.begin(); cd_iter != m_current->data.end(); cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      Format(line, IDSC_COMPARESTATS, st_data.group.c_str(), st_data.title.c_str(), st_data.user.c_str());
      m_compReport.WriteLine(line);
    }
    m_compReport.WriteLine();
  }

  if (m_comparison && !m_comparison->data.empty()) {

   Format(line, _("Entries only in comparison database (%ls):").c_str(), m_otherCore->GetCurFile().c_str());
   m_compReport.WriteLine(line);
    for (cd_iter = m_comparison->data.begin(); cd_iter != m_comparison->data.end(); cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      Format(line, IDSC_COMPARESTATS, st_data.group.c_str(), st_data.title.c_str(), st_data.user.c_str());
      m_compReport.WriteLine(line);
    }
    m_compReport.WriteLine();
  }

  if (m_conflicts && !m_conflicts->data.empty()) {

   m_compReport.WriteLine(_("Entries in both databases but with differences:"));

    for (cd_iter = m_conflicts->data.begin(); cd_iter != m_conflicts->data.end(); cd_iter++) {
      const st_CompareData &st_data = *cd_iter;

      Format(line, IDSC_COMPARESTATS, st_data.group.c_str(), st_data.title.c_str(), st_data.user.c_str());
      m_compReport.WriteLine(line);
      m_compReport.WriteLine(_("\t\thas differences in the following fields: "), false);

      // Non-time fields
      if (st_data.bsDiffs.test(CItemData::PASSWORD)) { line += CItemData::FieldName(CItemData::PASSWORD); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::NOTES)) { line += delimiter + CItemData::FieldName(CItemData::NOTES); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::URL)) { line += delimiter + CItemData::FieldName(CItemData::URL); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::AUTOTYPE)) { line += delimiter + CItemData::FieldName(CItemData::AUTOTYPE); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::PWHIST)) { line += delimiter + CItemData::FieldName(CItemData::PWHIST); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::POLICY)) { line += delimiter + CItemData::FieldName(CItemData::POLICY); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::RUNCMD)) { line += delimiter + CItemData::FieldName(CItemData::RUNCMD); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::DCA)) { line += delimiter + CItemData::FieldName(CItemData::DCA); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::SHIFTDCA)) { line += delimiter + CItemData::FieldName(CItemData::SHIFTDCA); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::EMAIL)) { line += delimiter + CItemData::FieldName(CItemData::EMAIL); delimiter = L", "; };
      if (st_data.bsDiffs.test(CItemData::PROTECTED)) { line += delimiter + CItemData::FieldName(CItemData::PROTECTED); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::SYMBOLS)) { line += delimiter + CItemData::FieldName(CItemData::SYMBOLS); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::POLICYNAME)) { line += delimiter + CItemData::FieldName(CItemData::POLICYNAME); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::KBSHORTCUT)) { line += delimiter + CItemData::FieldName(CItemData::KBSHORTCUT); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::ATTREF)) { line += delimiter + CItemData::FieldName(CItemData::ATTREF); delimiter = L", "; }

      // Time fields
      if (st_data.bsDiffs.test(CItemData::CTIME)) { line += delimiter + CItemData::FieldName(CItemData::CTIME); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::PMTIME)) { line += delimiter + CItemData::FieldName(CItemData::PMTIME); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::ATIME)) { line += delimiter + CItemData::FieldName(CItemData::ATIME); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::XTIME)) { line += delimiter + CItemData::FieldName(CItemData::XTIME); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::RMTIME)) { line += delimiter + CItemData::FieldName(CItemData::RMTIME); delimiter = L", "; }
      if (st_data.bsDiffs.test(CItemData::XTIME_INT)) { line += delimiter + CItemData::FieldName(CItemData::XTIME_INT); delimiter = L", "; }

      m_compReport.WriteLine(line);
    }
    m_compReport.WriteLine();
  }
  m_compReport.EndReport();
}

void CompareDlg::ReportAdvancedOptions()
{
  m_selCriteria->ReportAdvancedOptions(&m_compReport, _("compared"), m_otherCore->GetCurFile().c_str());
}

void CompareDlg::WriteReport()
{
  m_compReport.StartReport(IDSC_RPTCOMPARE, m_currentCore->GetCurFile().c_str());
  m_compReport.WriteLine(_("Comparing current database with: "), false);
  m_compReport.WriteLine(m_otherCore->GetCurFile().c_str());
  m_compReport.WriteLine();
  
  stringT line;
  Format(line, IDSC_COMPARESTATISTICS, m_currentCore->GetCurFile().c_str(), m_otherCore->GetCurFile().c_str());
  m_compReport.WriteLine(line);
  
  WriteReportData();
  
  ASSERT(m_compareButton);
  m_compareButton->Enable(true);
  Refresh();
}
