/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PwsSync.cpp
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

#include "./PwsSync.h"
#include "./OpenFilePickerValidator.h"
#include "./SafeCombinationCtrl.h"
#include "./DbSelectionPanel.h"
#include "./AdvancedSelectionDlg.h"

//#include <wx/richtext/richtextctrl.h>
#include <wx/filename.h>

/*!
 * SyncStartPage class declaration
 * 
 * First page of the synchronization wizard.  Only displays a 
 * welcome message and explains the functionality
 */
class SyncStartPage : public wxWizardPageSimple
{
public:
  SyncStartPage(wxWizard* parent);
};

/*!
 * DbSelectionPage class declaration
 * 
 * Second page of the synchronization wizard.  Lets the user chose
 * the DB to synchronize with and enter its combination
 */
class DbSelectionPage : public wxWizardPageSimple
{
  wxString m_safepath, m_combination;
  wxString m_currentDB;
  DbSelectionPanel* m_panel;

public:
  DbSelectionPage(wxWizard* parent, const wxString& currentDB);
  void OnWizardPageChanging(wxWizardEvent& evt);

  DECLARE_EVENT_TABLE()
};


//helper class used by field selection page to construct the UI
struct SyncFieldSelection {
  static bool IsMandatoryField(CItemData::FieldType /*field*/) {
    return false;
  }
  
  static bool ShowFieldSelection() {
    return true;
  }
};

/*!
 * SyncFieldSelectionPage class declaration
 * 
 * Third page of the synchronization wizard.  Lets the user chose
 * which fields to synchronize
 */
class SyncFieldSelectionPage: public wxWizardPageSimple
{
  
  typedef AdvancedSelectionImpl<SyncFieldSelection> SyncFieldSelectionPanel;
  SyncFieldSelectionPanel* m_panel;
  
public:
  SyncFieldSelectionPage(wxWizard* parent);
  void OnWizardPageChanging(wxWizardEvent& evt);

  DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////
// PwsSyncWizard Implementation
//
BEGIN_EVENT_TABLE(PwsSyncWizard, wxWizard)
  EVT_WIZARD_PAGE_CHANGING(wxID_ANY, PwsSyncWizard::OnWizardPageChanging)
END_EVENT_TABLE()

PwsSyncWizard::PwsSyncWizard(wxWindow* parent, const wxString& currentDB): 
                wxWizard(parent, wxID_ANY, _("Synchronize another database with currently open database")),
                m_page1(0)
{
  m_page1 = new SyncStartPage(this);
  
  DbSelectionPage* page2 = new DbSelectionPage(this, currentDB);
  SyncFieldSelectionPage* page3 = new SyncFieldSelectionPage(this);

  m_page1->SetNext(page2);
  page2->SetPrev(m_page1);

  page3->SetPrev(page2);
  page2->SetNext(page3);
  
  GetPageAreaSizer()->Add(m_page1);
}

PwsSyncWizard::~PwsSyncWizard()
{
}

void PwsSyncWizard::OnWizardPageChanging(wxWizardEvent& evt)
{
  if (evt.GetDirection())
    ;//wxMessageBox(wxT("Going forward"));
  else
    ;//wxMessageBox(wxT("Going backward"));
}

////////////////////////////////////////////
//SyncStartPage implementation
//
SyncStartPage::SyncStartPage(wxWizard* parent) : wxWizardPageSimple(parent)
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  
  const wxString text(_("WARNING!!\r\n\r\nIf you continue, fields will be updated in your existing database\r\nfrom your selected input database"));
  wxStaticText* txtCtrl = new wxStaticText(this, wxID_ANY, text);
  /*
  wxRichTextCtrl* txtCtrl = new wxRichTextCtrl(this);
  
  txtCtrl->BeginTextColour(*wxRED);
  txtCtrl->BeginBold();
  txtCtrl->WriteText(_("WARNING!!\r\n\r\n"));
  txtCtrl->EndBold();
  txtCtrl->EndTextColour();
  txtCtrl->WriteText(_("If you continue, fields will be updated in your existing database\r\nfrom your selected input database"));
  */
  sizer->Add(txtCtrl, wxSizerFlags().Expand().Proportion(1).Border());
  SetSizerAndFit(sizer);
}

/////////////////////////////////////////
// DbSelectionPage implementation
//
BEGIN_EVENT_TABLE(DbSelectionPage, wxWizardPageSimple)
  EVT_WIZARD_PAGE_CHANGING(wxID_ANY, DbSelectionPage::OnWizardPageChanging)
END_EVENT_TABLE()

DbSelectionPage::DbSelectionPage(wxWizard* parent, const wxString& currentDB):
                            wxWizardPageSimple(parent), m_currentDB(currentDB),
                                                        m_panel(0)
{
  const wxString filePrompt(wxString(_("Choose Database to Synchronize with \"")) << currentDB << _("\""));
  const wxString filePickerCtrlTitle(_("Please Choose a Database to Synchronize with current database"));

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  m_panel = new DbSelectionPanel(this, filePrompt, filePickerCtrlTitle, 5);
  sizer->Add(m_panel, wxSizerFlags().Expand().Proportion(1));
  SetSizerAndFit(sizer);
}

void DbSelectionPage::OnWizardPageChanging(wxWizardEvent& evt)
{
  if (evt.GetDirection()) {
    //Check that this file isn't already open
    if (wxFileName(m_panel->m_filepath).SameAs(wxFileName(m_currentDB))) {
      // It is the same damn file
      wxMessageBox(_("That file is already open."), _("Synchronize error"), wxOK | wxICON_WARNING, this);
      evt.Veto();
    }
  }
  else
    ;//wxMessageBox(wxT("Going backward"), wxT("DbSelectionPage"), wxOK, this);
}

////////////////////////////////////////////////
// SyncFieldSelectionPage implementation
//
BEGIN_EVENT_TABLE(SyncFieldSelectionPage, wxWizardPageSimple)
  EVT_WIZARD_PAGE_CHANGING(wxID_ANY, SyncFieldSelectionPage::OnWizardPageChanging)
END_EVENT_TABLE()

SyncFieldSelectionPage::SyncFieldSelectionPage(wxWizard* parent) : wxWizardPageSimple(parent)
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  
  SelectionCriteria selCriteria;

  //Don't auto-select any fields to synchronize by default
  selCriteria.m_bsFields.reset();
  
  m_panel = new SyncFieldSelectionPanel(this, selCriteria);
  m_panel->CreateControls(this);
  sizer->Add(m_panel, wxSizerFlags().Expand().Proportion(1));
  SetSizerAndFit(sizer);
}

void SyncFieldSelectionPage::OnWizardPageChanging(wxWizardEvent& evt)
{
  if (evt.GetDirection() && !m_panel->m_criteria.m_bsFields.count()) {
    wxMessageBox(_("You must select some of the fields to synchronize"), 
                      wxT("Synchronize"), wxOK|wxICON_INFORMATION, this);
    evt.Veto();
  }
  else
    ;//wxMessageBox(wxT("Going backward"), wxT("DbSelectionPage"), wxOK, this);
}

