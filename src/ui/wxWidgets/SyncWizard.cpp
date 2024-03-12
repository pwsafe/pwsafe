/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SyncWizard.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/filename.h>
#include <wx/valgen.h>
#include <wx/statline.h>
#include <wx/collpane.h>

#include "core/core.h"
#include "core/PWScore.h"

#include "AdvancedSelectionDlg.h"
#include "DbSelectionPanel.h"
#include "OpenFilePickerValidator.h"
#include "SafeCombinationCtrl.h"
#include "SelectionCriteria.h"
#include "SyncWizard.h"

#include <algorithm>
#include <iterator>

/*!
 * SyncData class declaration
 *
 * Sync data and options shared and updated by all pages
 */
struct SyncData {
  SelectionCriteria selCriteria;
  wxFileName        otherDB;
  StringX           combination;
  PWScore*          core = nullptr;
  size_t            numUpdated;
  std::unique_ptr<CReport> syncReport = std::unique_ptr<CReport>(new CReport());
  bool              showReport;
  MultiCommands*    syncCmds = nullptr;
};

/*!
 * SyncWizardPage class declaration
 *
 * Base class from which all other Sync wizard pages are derived.  Each page
 * must overload the "SaveData" function
 */
class SyncWizardPage: public wxWizardPageSimple
{
protected:
  SyncData* m_syncData;
  enum class PageDirection { BACKWARD, FORWARD };
  wxBoxSizer* m_pageSizer;

public:
  SyncWizardPage(wxWizard* parent, SyncData* data, const wxString& pageHeader);
  virtual void SaveData(SyncData* data) = 0;

  void OnWizardPageChanging(wxWizardEvent& evt);
  void OnWizardPageChanged(wxWizardEvent& evt);

  virtual void OnPageEnter(PageDirection WXUNUSED(direction)) {}
  //return false to veto the page change
  virtual bool OnPageLeave(PageDirection WXUNUSED(direction)) { return true; }

  void SetChildWindowText(unsigned id, const wxString& str);

  DECLARE_EVENT_TABLE()
};

/*!
 * SyncStartPage class declaration
 *
 * First page of the synchronization wizard.  Only displays a
 * welcome message and explains the functionality
 */
class SyncStartPage : public SyncWizardPage
{
public:
  SyncStartPage(wxWizard* parent, SyncData* data);

  virtual void SaveData(SyncData* WXUNUSED(data)) {}
};

/*!
 * DbSelectionPage class declaration
 *
 * Second page of the synchronization wizard.  Lets the user chose
 * the DB to synchronize with and enter its combination
 */
class DbSelectionPage : public SyncWizardPage
{
  wxString m_safepath, m_combination;
  wxString m_currentDB;
  DbSelectionPanel* m_panel;

public:
  DbSelectionPage(wxWizard* parent, SyncData* data, const wxString filename = "");

  virtual bool OnPageLeave(PageDirection dir);
  virtual void SaveData(SyncData* data);
};

//helper class used by field selection page to construct the UI
struct SyncFieldSelection {
  static bool IsMandatoryField(CItemData::FieldType WXUNUSED(field)) {
    return false;
  }

  static bool IsPreselectedField(CItemData::FieldType WXUNUSED(field)) {
    return true;
  }

  static bool IsUsableField(CItemData::FieldType field) {
    switch (field) {
      case CItemData::GROUP:
      case CItemData::USER:
      case CItemData::TITLE:
        return false;
      default:
        return true;
    }
  }

  static bool ShowFieldSelection() {
    return true;
  }
  static wxString GetTaskWord() {
    return _("synchronize");
  }
};

/*!
 * SyncFieldSelectionPage class declaration
 *
 * Third page of the synchronization wizard.  Lets the user chose
 * which fields to synchronize
 */
class SyncFieldSelectionPage: public SyncWizardPage
{
  typedef AdvancedSelectionImpl<SyncFieldSelection> SyncFieldSelectionPanel;
  SyncFieldSelectionPanel* m_panel;

public:

  SyncFieldSelectionPage(wxWizard* parent, SyncData* data);

  virtual bool OnPageLeave(PageDirection dir);
  virtual void SaveData(SyncData* data);
};

/*!
 * SyncOptionsSummaryPage class declaration
 *
 * Fourth page of the synchronization wizard.  Presents the summary of selected
 * options in a language the user would understand
 */
class SyncOptionsSummaryPage: public SyncWizardPage
{
  enum {ID_DESC = 100, ID_UPDATED_TXT, ID_NOT_UPDATED_TXT};

  wxFlexGridSizer    *m_updatedFieldsGrid, *m_notUpdatedFieldsGrid;

public:
  SyncOptionsSummaryPage(wxWizard* parent, SyncData* data);

  virtual void OnPageEnter(PageDirection dir);
  virtual void SaveData(SyncData* WXUNUSED(data)) {}
};

/*!
 * SyncStatusPage class declaration
 *
 * Final page of the synchronization wizard, where all the synchronization happens
 * Either shows a progressbar followed by a success/error message, or an error
 * if the sync couldn't even be started
 */
class SyncStatusPage: public SyncWizardPage
{
  enum {ID_HEADER_TXT = 100, ID_PROGRESS_TXT, ID_GAUGE, ID_FINISH_TXT, ID_SHOW_REPORT};

  void SetSyncSummary(const wxString& str);
  void SetProgressText(const wxString& str);
  void SetHeaderText(const wxString& str);
  wxString GetReadErrorMessageTemplate(int rc);
  bool DbHasNoDuplicates(PWScore* core);
  void Synchronize(PWScore* currentCore, const PWScore* otherCore);
  void ReportAdvancedOptions(CReport* rpt, const wxString& operation);
  void OnSyncStartEvent(wxCommandEvent& evt);
public:
  SyncStatusPage(wxWizard* parent, SyncData* data);

  virtual void SaveData(SyncData* WXUNUSED(data)) {}
  virtual void OnPageEnter(PageDirection dir);
};

///////////////////////////////////////////////////
// SyncWizard Implementation
//
BEGIN_EVENT_TABLE(SyncWizard, wxWizard)
  EVT_WIZARD_PAGE_CHANGING(wxID_ANY, SyncWizard::OnWizardPageChanging)
  EVT_CLOSE( SyncWizard::OnClose )
  EVT_WIZARD_CANCEL(wxID_ANY, SyncWizard::OnWizardCancel)
END_EVENT_TABLE()

SyncWizard::SyncWizard(wxWindow* parent, PWScore* core, const wxString filename):
                wxWizard(parent, wxID_ANY, _("Synchronize another database with currently open database")),
                m_page1(nullptr), m_syncData(new SyncData)
{
  //select all fields, except those below
  m_syncData->selCriteria.SelectAllFields();

  //Other than these, all fields are selected for sync by default, as in
  m_syncData->selCriteria.ResetField(CItemData::NAME);
  m_syncData->selCriteria.ResetField(CItemData::UUID);
  m_syncData->selCriteria.ResetField(CItemData::GROUP);
  m_syncData->selCriteria.ResetField(CItemData::TITLE);
  m_syncData->selCriteria.ResetField(CItemData::USER);
  m_syncData->selCriteria.ResetField(CItemData::RESERVED);

  m_syncData->core = core;
  m_syncData->numUpdated = 0;
  m_syncData->showReport = false;

  m_page1 = new SyncStartPage(this, m_syncData);

  auto *page2 = new DbSelectionPage(this, m_syncData, filename);
  auto *page3 = new SyncFieldSelectionPage(this, m_syncData);
  auto *page4 = new SyncOptionsSummaryPage(this, m_syncData);
  auto *page5 = new SyncStatusPage(this, m_syncData);

  m_page1->SetNext(page2);
  page2->SetPrev(m_page1);

  page3->SetPrev(page2);
  page2->SetNext(page3);

  page3->SetNext(page4);
  page4->SetPrev(page3);

  page4->SetNext(page5);

  GetPageAreaSizer()->Add(m_page1);
}

SyncWizard::~SyncWizard()
{
  delete m_syncData;
  m_syncData = nullptr;
}

void SyncWizard::OnWizardPageChanging(wxWizardEvent& evt)
{
  if (evt.GetDirection()) {
    ;//wxMessageBox(wxT("In wizard: Going forward"));
  }
  else {
    ;//wxMessageBox(wxT("In wizard: Going backward"));
  }
  SyncWizardPage* page = wxDynamicCast(evt.GetPage(), SyncWizardPage);
  wxCHECK_RET(page, wxT("Wizard pages in Sync wizard not derived from SyncWizardPage"));
  page->SaveData(m_syncData);
}

size_t SyncWizard::GetNumUpdated() const
{
  return m_syncData->numUpdated;
}

bool SyncWizard::ShowReport() const {
  return m_syncData->showReport;
}

CReport* SyncWizard::GetReport() const {
  return m_syncData->syncReport.get();
}

MultiCommands* SyncWizard::GetSyncCommands() const {
  return m_syncData->syncCmds;
}

bool SyncWizard::QueryCancel(bool showDialog) {
  if (m_syncData->numUpdated > 0) {
    if (showDialog) {
      auto res = wxMessageDialog(
        nullptr,
        _("One or more items have been changed. Are you sure you wish to cancel?"), wxEmptyString,
        wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION
      ).ShowModal();
      if (res == wxID_YES) {
        return true;
      }
    }
    return false;
  }
  return true;
}

void SyncWizard::ResetSyncData() {
  // reset info about changes
  m_syncData->numUpdated = 0;
  m_syncData->showReport = false;
  m_syncData->syncReport = std::unique_ptr<CReport>(new CReport());
  if (m_syncData->syncCmds) {
    delete m_syncData->syncCmds;
  }
  m_syncData->syncCmds = nullptr;
}

void SyncWizard::OnClose(wxCloseEvent &event) {
  if (event.CanVeto()) {
    // when trying to closing app/db, don't ask questions when data changed
    if (!QueryCancel(!IsCloseInProgress())) {
      event.Veto();
      return;
    }
  }
  EndDialog(wxID_CANCEL); // cancel directly (if we skip event, OnCancel will be called and ask one more time)
}

void SyncWizard::OnWizardCancel(wxWizardEvent& event) {
  if (QueryCancel(true)) {
    event.Skip();
  }
  else {
    event.Veto();
  }
}

////////////////////////////////////////////
//SyncWizardPage implementation
//
BEGIN_EVENT_TABLE(SyncWizardPage, wxWizardPageSimple)
  EVT_WIZARD_PAGE_CHANGING(wxID_ANY, SyncWizardPage::OnWizardPageChanging)
  EVT_WIZARD_PAGE_CHANGED( wxID_ANY, SyncWizardPage::OnWizardPageChanged)
END_EVENT_TABLE()

SyncWizardPage::SyncWizardPage(wxWizard* parent, SyncData* data,
                                  const wxString& pageHeader): wxWizardPageSimple(parent),
                                                               m_syncData(data),
                                                               m_pageSizer(new wxBoxSizer(wxVERTICAL))
{
  wxStaticText* hdr = new wxStaticText(this, wxID_ANY, pageHeader);

  //make it bigger and bolder
  wxFont f = hdr->GetFont();
  f.SetWeight(wxFONTWEIGHT_BOLD);
  f.SetPointSize(f.GetPointSize()*2);
  hdr->SetFont(f);

  m_pageSizer->Add(hdr, wxSizerFlags().Expand().Proportion(0).Border());
  m_pageSizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Proportion(0).Border());
  m_pageSizer->AddSpacer(RowSeparation);
}

void SyncWizardPage::OnWizardPageChanging(wxWizardEvent& evt)
{
  SyncWizardPage* page = wxDynamicCast(evt.GetPage(), SyncWizardPage);
  wxASSERT_MSG(page, wxT("Sync wizard page not derived from SyncWizardPage class"));

  if (page && !page->OnPageLeave(evt.GetDirection() ? PageDirection::FORWARD : PageDirection::BACKWARD))
    evt.Veto();

  //must always do this, to let the wizard see the event as well
  evt.Skip();
}

void SyncWizardPage::OnWizardPageChanged(wxWizardEvent& evt)
{
  SyncWizardPage* page = wxDynamicCast(evt.GetPage(), SyncWizardPage);
  wxASSERT_MSG(page, wxT("Sync wizard page not derived from SyncWizardPage class"));
  if (page) {
    page->OnPageEnter(evt.GetDirection() ? PageDirection::FORWARD : PageDirection::BACKWARD);
  }

  //must always do this, to let the wizard see the event as well
  evt.Skip();
}

void SyncWizardPage::SetChildWindowText(unsigned id, const wxString& str)
{
  FindWindow(id)->SetLabel(str);
}

////////////////////////////////////////////
//SyncStartPage implementation
//
SyncStartPage::SyncStartPage(wxWizard* parent, SyncData* data) : SyncWizardPage(parent, data, _("Introduction"))
{
  wxBoxSizer* sizer = m_pageSizer;

  const wxString explanation(_("Synchronizing with another database will update the entries in your\ndatabase with matching entries from the other database."));
  sizer->Add(new wxStaticText(this, wxID_ANY, explanation), wxSizerFlags().Expand().Proportion(0).Border());

  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, _("More Info"));

  const wxString helpItems[] = {
    _("1. Two entries from different databases match if their Group, Title\nand User fields match."),
    _("2. You can select the fields to update, as well as filter the entries\nfor synchronization."),
    _("3. Only existing entries in your database are updated.  No new entries are\nadded or existing entries removed during this process."),
    _("4. You can undo the operation once it is complete, but won't be\nable to abort it mid-way.")
  };

  auto *paneSizer = new wxBoxSizer(wxVERTICAL);
  for (size_t idx = 0; idx < NumberOf(helpItems); ++idx) {
    paneSizer->Add(new wxStaticText(pane->GetPane(), wxID_ANY, helpItems[idx]), wxSizerFlags().Expand().Border().Proportion(1));
  }
  pane->GetPane()->SetSizer(paneSizer);
  sizer->Add(pane, wxSizerFlags().Border().Proportion(1).Expand());

  SetSizerAndFit(sizer);
}

/////////////////////////////////////////
// DbSelectionPage implementation
//
DbSelectionPage::DbSelectionPage(wxWizard* parent, SyncData* data, const wxString filename):
                             SyncWizardPage(parent, data, _("Select another database"))
{
  const wxString filePrompt(wxString(_("Choose Database to Synchronize with \"")) << towxstring(data->core->GetCurFile()) << wxT("\""));
  const wxString filePickerCtrlTitle(_("Choose a Database to Synchronize with the current database"));

  wxBoxSizer* sizer = m_pageSizer;
  m_panel = new DbSelectionPanel(this, filePrompt, filePickerCtrlTitle, false, data->core, 5, wxID_OK, filename);
  sizer->Add(m_panel, wxSizerFlags().Expand().Proportion(1));
  SetSizerAndFit(sizer);
}

bool DbSelectionPage::OnPageLeave(PageDirection direction)
{
  return (direction == PageDirection::BACKWARD) || m_panel->DoValidation();
}

void DbSelectionPage::SaveData(SyncData* data)
{
  data->otherDB = m_panel->m_filepath;
  data->combination = m_panel->m_combination;
}

////////////////////////////////////////////////
// SyncFieldSelectionPage implementation
//
SyncFieldSelectionPage::SyncFieldSelectionPage(wxWizard* parent, SyncData* data):
                               SyncWizardPage(parent, data, _("Synchronization options"))
{
  wxBoxSizer* sizer = m_pageSizer;

  m_panel = new SyncFieldSelectionPanel(this, &data->selCriteria, false);
  m_panel->CreateControls(this);
  sizer->Add(m_panel, wxSizerFlags().Expand().Proportion(1));
  SetSizerAndFit(sizer);
}

bool SyncFieldSelectionPage::OnPageLeave(PageDirection direction)
{
  return (direction == PageDirection::BACKWARD) || m_panel->DoValidation();
}

void SyncFieldSelectionPage::SaveData(SyncData* data)
{
  data->selCriteria = *m_panel->m_criteria;
}

//////////////////////////////////////////////////////
// SyncOptionsSummaryPage implementation
//
SyncOptionsSummaryPage::SyncOptionsSummaryPage(wxWizard* parent, SyncData* data)
                              : SyncWizardPage(parent, data, _("Options Summary")),
                                m_updatedFieldsGrid(nullptr),
                                m_notUpdatedFieldsGrid(nullptr)
{
  wxSizerFlags flags = wxSizerFlags().Expand().Proportion(0).Border(wxLEFT+wxRIGHT, SideMargin);
  wxSizerFlags gridFlags = wxSizerFlags().Expand().Proportion(1).Border(wxLEFT+wxRIGHT, SideMargin*2);
  wxBoxSizer* sizer = m_pageSizer;

  sizer->Add(new wxStaticText(this, ID_DESC, wxEmptyString), flags.Proportion(1));
  sizer->AddSpacer(RowSeparation);

  sizer->Add(new wxStaticText(this, ID_UPDATED_TXT, wxEmptyString), flags.Proportion(0));
  sizer->AddSpacer(RowSeparation);

  m_updatedFieldsGrid = new wxFlexGridSizer(0, 3, RowSeparation, ColSeparation);
  sizer->Add(m_updatedFieldsGrid, gridFlags.Proportion(1));
  sizer->AddSpacer(RowSeparation*2);

  sizer->Add(new wxStaticText(this, ID_NOT_UPDATED_TXT, wxEmptyString), flags.Proportion(0));
  sizer->AddSpacer(RowSeparation);

  m_notUpdatedFieldsGrid = new wxFlexGridSizer(0, 3, RowSeparation, ColSeparation);
  sizer->Add(m_notUpdatedFieldsGrid, gridFlags.Proportion(1));
  sizer->AddSpacer(RowSeparation*2);

  const wxString warning(_("WARNING!!\n\nIf you continue, fields will be updated in your existing database\nfrom your selected input database"));
  wxStaticText* txtWarn = new wxStaticText(this, wxID_ANY, warning);
  txtWarn->SetForegroundColour(*wxRED);
  sizer->Add(txtWarn, flags.Proportion(0));

  SetSizerAndFit(sizer);
}

void SyncOptionsSummaryPage::OnPageEnter(PageDirection direction)
{
  if (direction == PageDirection::BACKWARD)
    return;

  m_updatedFieldsGrid->Clear(true);
  m_notUpdatedFieldsGrid->Clear(true);

  wxString description = m_syncData->selCriteria.GetGroupSelectionDescription();
  description << _(" will be updated with corresponding entries from \"")
              << m_syncData->otherDB.GetFullPath() << wxT('"');
  FindWindow(ID_DESC)->SetLabel( description);

  wxArrayString fieldsSelected, fieldsNotSelected;
  const bool allSelected = m_syncData->selCriteria.GetFieldSelection(fieldsSelected, fieldsNotSelected);
  if (allSelected) {
    FindWindow(ID_UPDATED_TXT)->SetLabel(_("All fields in matching entries will be updated"));
    FindWindow(ID_NOT_UPDATED_TXT)->SetLabel(wxEmptyString);
  }
  else {
    wxCHECK_RET(fieldsSelected.Count() > 0, wxT("None of the fields have been selected"));
    wxCHECK_RET(fieldsNotSelected.Count() > 0, wxT("None of the fields have been un-selected"));
    FindWindow(ID_UPDATED_TXT)->SetLabel(_("Following fields in matching entries will be updated:"));
    for( size_t idx = 0; idx < fieldsSelected.Count(); ++idx) {
      m_updatedFieldsGrid->Add(new wxStaticText(this, wxID_ANY, wxT("* ") + fieldsSelected[idx]));
    }
    FindWindow(ID_NOT_UPDATED_TXT)->SetLabel(_("Following fields will not be updated:"));
    for( size_t idx = 0; idx < fieldsNotSelected.Count(); ++idx) {
      m_notUpdatedFieldsGrid->Add(new wxStaticText(this, wxID_ANY, wxT("* ") + fieldsNotSelected[idx]));
    }

    // Set the wxSizer Proportion of the two grids of field lists to the number
    // of rows they have now, or else they overlap each other
    m_pageSizer->GetItem(m_updatedFieldsGrid)->SetProportion(m_updatedFieldsGrid->GetRows());
    m_pageSizer->GetItem(m_notUpdatedFieldsGrid)->SetProportion(m_notUpdatedFieldsGrid->GetRows());
  }
  GetSizer()->Layout();
}

//////////////////////////////////////////////////////
// SyncStatusPage implementation
//
DECLARE_EVENT_TYPE(wxEVT_SYNC_START, -1)
DEFINE_EVENT_TYPE(wxEVT_SYNC_START)

SyncStatusPage::SyncStatusPage(wxWizard* parent, SyncData* data): SyncWizardPage(parent, data, _("Synchronization status"))
{
  wxBoxSizer* sizer = m_pageSizer;
  wxSizerFlags flags = wxSizerFlags().Expand().Proportion(1).Border(wxLEFT|wxRIGHT, SideMargin);

  sizer->Add(new wxStaticText(this, ID_HEADER_TXT, wxEmptyString), flags.Proportion(1));
  sizer->AddSpacer(RowSeparation);

  auto *midSizer = new wxBoxSizer(wxVERTICAL);
  midSizer->Add(new wxStaticText(this, ID_PROGRESS_TXT, wxEmptyString), wxSizerFlags().Expand().Proportion(1));
  midSizer->AddSpacer(RowSeparation);
  size_t range = data->core->GetNumEntries();
  wxCHECK2_MSG(range <= INT_MAX, range = INT_MAX, wxT("Too many entries in db for wxGauge"));
  midSizer->Add(new wxGauge(this, ID_GAUGE, int(range)), wxSizerFlags().Expand().Proportion(0));
  sizer->Add(midSizer, flags.Proportion(1));

  auto *horizSizer = new wxBoxSizer(wxHORIZONTAL);
  horizSizer->Add(new wxStaticText(this, ID_FINISH_TXT, wxEmptyString), wxSizerFlags().Expand().Proportion(1));
  horizSizer->Add(new wxCheckBox(this, ID_SHOW_REPORT, _("See a detailed report"), wxDefaultPosition,
                      wxDefaultSize, 0, wxGenericValidator(&m_syncData->showReport)),
                    wxSizerFlags().Proportion(0));
  sizer->Add(horizSizer, flags.Proportion(1));

  SetSizerAndFit(sizer);
}

void SyncStatusPage::SetSyncSummary(const wxString& str)
{
  SetChildWindowText(ID_FINISH_TXT, str);
}

void SyncStatusPage::SetProgressText(const wxString& str)
{
  SetChildWindowText(ID_PROGRESS_TXT, str);
}

void SyncStatusPage::SetHeaderText(const wxString& str)
{
  SetChildWindowText(ID_HEADER_TXT, str);
}

void SyncStatusPage::OnPageEnter(PageDirection direction)
{
  if (direction == PageDirection::FORWARD) {
    //we came here from the previous page

    FindWindow(ID_SHOW_REPORT)->Hide();

    auto *othercore = new PWSAuxCore;
    const wxString otherDBPath = m_syncData->otherDB.GetFullPath();
    const int rc = ReadCore(*othercore, otherDBPath, m_syncData->combination,
                                    false, this);
    if (rc == PWScore::SUCCESS) {
      if (DbHasNoDuplicates(othercore) && DbHasNoDuplicates(m_syncData->core)) {
        SetHeaderText(_("Your database is being synchronized with \"") + otherDBPath + _T('"'));
        Connect(GetId(), wxEVT_SYNC_START, wxCommandEventHandler(SyncStatusPage::OnSyncStartEvent));
        wxCommandEvent evt(wxEVT_SYNC_START, GetId());
        evt.SetClientData(reinterpret_cast<wxClientData*>(othercore));
        AddPendingEvent(evt);
      }
    }
    else {
      SetProgressText(wxString::Format(GetReadErrorMessageTemplate(rc),
                                       otherDBPath));
      SetHeaderText(_("There was an error during synchronization"));
      SetSyncSummary(_("File Read Error"));
    }
  }

  GetSizer()->Layout();
}

void SyncStatusPage::OnSyncStartEvent(wxCommandEvent& evt)
{
  auto *otherCore = reinterpret_cast<PWScore*>(evt.GetClientData());
  wxASSERT_MSG(otherCore, wxT("Sync Start Event did not arrive with the other PWScore"));
  Synchronize(m_syncData->core, otherCore);

  SetHeaderText(wxString::Format(_("Your database has been synchronized with \"%ls\""), otherCore->GetCurFile().c_str()));
  SetProgressText(wxString::Format(_("%ld %ls updated"), m_syncData->numUpdated,
                      m_syncData->numUpdated == 1? _("entry"): _("entries")));
  SetSyncSummary(_("Synchronization completed successfully"));

  delete otherCore;

  FindWindow(ID_GAUGE)->Hide();
  FindWindow(ID_SHOW_REPORT)->Show();

  GetSizer()->Layout();
}

wxString SyncStatusPage::GetReadErrorMessageTemplate(int rc)
{
  switch (rc) {
    case PWScore::CANT_OPEN_FILE:
      return _("%ls\n\nCould not open file for reading!");
    case PWScore::BAD_DIGEST:
      return _("%ls\n\nFile corrupt or truncated!\nData may have been lost or modified.");
    default:
      return _("%ls\n\nUnknown error");
  }
}

bool SyncStatusPage::DbHasNoDuplicates(PWScore* core)
{
  GTUSet setGTU;

  // First check other database
  if (!core->GetUniqueGTUValidated() && !core->InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    SetSyncSummary(_("Synchronization failed"));
    SetProgressText(wxString::Format(_("The database:\n\n%ls\n\nhas duplicate entries with the same group/title/user combination. Fix by validating database."), core->GetCurFile().c_str()));
    FindWindow(ID_GAUGE)->Hide();
    return false;
  }
  return true;
}

/*
Purpose:
Merge entries from otherCore to currentCore

Algorithm:
Foreach entry in otherCore
  Find in m_core
  if find a match
    update requested fields
*/
void SyncStatusPage::Synchronize(PWScore* currentCore, const PWScore *otherCore)
{
  CReport& rpt = *m_syncData->syncReport;

  rpt.StartReport(IDSC_RPTSYNCH, currentCore->GetCurFile().c_str());
  wxString line = wxString::Format(_("Synchronizing from database: %ls\n"), otherCore->GetCurFile().c_str());
  rpt.WriteLine(line.c_str());

  ReportAdvancedOptions(&rpt, _("synchronized"));

  std::vector<StringX> vs_updated;
  int numUpdated = 0;

  MultiCommands *pmulticmds = MultiCommands::Create(currentCore);
  Command *pcmd1 = UpdateGUICommand::Create(currentCore, UpdateGUICommand::WN_UNDO,
                                            UpdateGUICommand::GUI_UNDO_MERGESYNC);
  pmulticmds->Add(pcmd1);
  const SelectionCriteria& criteria = m_syncData->selCriteria;
  const stringT subgroup_name = tostdstring(criteria.SubgroupSearchText());

  wxGauge* gauge = wxDynamicCast(FindWindow(ID_GAUGE), wxGauge);
  gauge->SetRange(int(otherCore->GetNumEntries()));

  ItemListConstIter otherPos;
  for (otherPos = otherCore->GetEntryIter();
       otherPos != otherCore->GetEntryEndIter();
       otherPos++) {
    const CItemData &otherItem = otherCore->GetEntry(otherPos);
    CItemData::EntryType et = otherItem.GetEntryType();

    const auto currentIndex = std::distance(otherCore->GetEntryIter(), otherPos);
    gauge->SetValue(int(currentIndex));

    // Do not process Aliases and Shortcuts
    if (et == CItemData::ET_ALIAS || et == CItemData::ET_SHORTCUT)
      continue;

    const StringX otherGroup = otherItem.GetGroup();
    const StringX otherTitle = otherItem.GetTitle();
    const StringX otherUser = otherItem.GetUser();

    const StringX sx_updated = StringX(wxT("\xab")) +
                           otherGroup + StringX(wxT("\xbb \xab")) +
                           otherTitle + StringX(wxT("\xbb \xab")) +
                           otherUser  + StringX(wxT("\xbb"));

    SetProgressText((wxString() << currentIndex << wxT(": ")) + towxstring(sx_updated));
    wxSafeYield();

    if (criteria.HasSubgroupRestriction() && !otherItem.Matches(subgroup_name, criteria.SubgroupObject(), criteria.SubgroupFunctionWithCase()))
      continue;

    ItemListConstIter foundPos = currentCore->Find(otherGroup, otherTitle, otherUser);

    if (foundPos != currentCore->GetEntryEndIter()) {
      // found a match
      CItemData curItem = currentCore->GetEntry(foundPos);
      CItemData updItem(curItem);

      uuid_array_t current_uuid, other_uuid;
      curItem.GetUUID(current_uuid);
      otherItem.GetUUID(other_uuid);
      if (memcmp(static_cast<void *>(current_uuid), static_cast<void *>(other_uuid), sizeof(uuid_array_t)) != 0) {
        pws_os::Trace(wxT("Synchronize: Mis-match UUIDs for [%ls:%ls:%ls]\n"), otherGroup.c_str(), otherTitle.c_str(), otherUser.c_str());
      }

      bool bUpdated(false);
      for (size_t i = 0; i < criteria.TotalFieldsCount(); i++) {
        auto ft = static_cast<CItemData::FieldType>(i);
        if (criteria.IsFieldSelected(ft)) {
          const StringX sxValue = otherItem.GetFieldValue(ft);
          if (sxValue != updItem.GetFieldValue(ft)) {
            bUpdated = true;
            updItem.SetFieldValue(ft, sxValue);
          }
        }
      }

      if (!bUpdated)
        continue;

      updItem.SetStatus(CItemData::ES_MODIFIED);

      vs_updated.push_back(sx_updated);

      Command *pcmd = EditEntryCommand::Create(currentCore, curItem, updItem);
      pcmd->SetNoGUINotify();
      pmulticmds->Add(pcmd);

      numUpdated++;
    }  // Found match via [g:t:u]
  } // iteration over other core's entries

  if (numUpdated > 0) {
    std::sort(vs_updated.begin(), vs_updated.end(), MergeSyncGTUCompare);
    const wxString cs_singular_plural_type = (numUpdated == 1 ? _("entry") : _("entries"));
    const wxString cs_singular_plural_verb = (numUpdated == 1 ? _("was") : _("were"));
    const wxString resultStr = wxString::Format(_("\nThe following %ls %ls updated:"), cs_singular_plural_type,
                                                cs_singular_plural_verb);
    rpt.WriteLine(resultStr.c_str());
    for (size_t i = 0; i < vs_updated.size(); i++) {
      const wxString fieldName = wxString::Format(wxT("\t%ls"), vs_updated[i].c_str());
      rpt.WriteLine(fieldName.c_str());
    }
  }

  Command *pcmd2 = UpdateGUICommand::Create(currentCore, UpdateGUICommand::WN_REDO,
                                            UpdateGUICommand::GUI_REDO_MERGESYNC);
  pmulticmds->Add(pcmd2);
  m_syncData->syncCmds = pmulticmds; // will execute later

  /* tell the user we're done & provide short merge report */
  const wxString cs_entries = (numUpdated == 1 ? _("entry") : _("entries"));
  wxString resultStr = wxString::Format(_("\nSynchronize completed: %d %ls updated"), numUpdated, cs_entries);
  rpt.WriteLine(resultStr.c_str());
  rpt.EndReport();

  m_syncData->numUpdated = numUpdated;
}

void SyncStatusPage::ReportAdvancedOptions(CReport* rpt, const wxString& operation)
{
  m_syncData->selCriteria.ReportAdvancedOptions(rpt, operation, m_syncData->otherDB.GetFullPath().c_str());
}
