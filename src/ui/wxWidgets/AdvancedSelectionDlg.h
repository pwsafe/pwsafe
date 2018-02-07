/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __ADVANCEDDSELECTIONDLG_H__
#define __ADVANCEDDSELECTIONDLG_H__

#include "../../core/ItemData.h"
#include "./wxutils.h"

/*
 *
 * AdvancedSelectionPanel is a composite UI widget which looks like this:
 * 
 *    +----------------------------------------------------------------------------+
 *    |    +------------------------------------------------------------------+    |
 *    |    |  [x] Restrict to a subset of entries                             |    |
 *    |    |         ___________      __________                              |    |
 *    |    |  where [___________[v]  [__________[v]                           |    |
 *    |    |                                                                  |    |
 *    |    |  the following text                                              |    |
 *    |    |  _____________________________                                   |    |
 *    |    | [_____________________________]                                  |    |
 *    |    |                                                                  |    |
 *    |    |  [x] Case Sensitive                                              |    |
 *    |    +------------------------------------------------------------------+    |
 *    |                                                                            |
 *    |    Available Fields                  Selected Fields                       |
 *    |    +-----------------+               +--------------------------------+    |
 *    |    | URL             |     [  > ]    | Group [Mandatory field]        |    |
 *    |    | Password        |     [ >> ]    | Title [Mandatory field]        |    |
 *    |    | Email           |     [ <  ]    | Username [Mandatory field]     |    |
 *    |    | Notes           |     [ << ]    | Autotype                       |    |
 *    |    |    .....        |               |    ...........                 |    |
 *    |    +-----------------+               +--------------------------------+    |
 *    |                                                                            |
 *    +----------------------------------------------------------------------------+
 *
 * This widget is used in multiple situations where users need to select
 * a subset of entries in their current db for some operation, while also
 * restricting the operation to a set of fields of those entries. There
 * are at least 5 such operations: Export to Text/XML, Compare, Merge, Synchronize
 * and Search. In some situations though, the user cannot choose the individual fields,
 * e.g while Merging a DB with another.  All fields of selected entries are merged.
 *
 * For example, the user might want to export the Group, Title, Username and Autotype
 * fields a set of entries where the URL contains the word "gmail".
 *
 * Or he might want to search for some text in a set of entries where the Title contains
 * "Bank", and also restrict the search to Email and Notes fields.
 *
 * The Widget is re-usable as a stand-alone dialog (Export to Text/XML), or as an embedded
 * child element in another dialog (Compare) or wizard (Synchronize).
 *
 * It uses the SelectionCriteria class as the backing data structure.
 *
 */

struct SelectionCriteria;
 
/*!
 * AdvancedSelectionDlg class declaration
 */

class AdvancedSelectionPanel: public wxPanel
{
  DECLARE_CLASS(AdvancedSelectionPanel)
  DECLARE_EVENT_TABLE()

  DECLARE_NO_COPY_CLASS(AdvancedSelectionPanel)

public:
  AdvancedSelectionPanel(wxWindow* wnd, SelectionCriteria* existingCriteria, bool autoValidate);

  bool Validate();               //overridden from wxWindow
  bool TransferDataToWindow();   //overridden from wxWindow
  bool TransferDataFromWindow(); //overridden from wxWindow
  void OnSelectSome( wxCommandEvent& evt );
  void OnSelectAll( wxCommandEvent& evt );
  void OnRemoveSome( wxCommandEvent& evt );
  void OnRemoveAll( wxCommandEvent& evt );
  void OnRestrictSearchItems(wxCommandEvent& evt);

  void CreateControls(wxWindow* parentWnd);
  bool DoValidation();               //actual validator

protected:
  virtual bool IsMandatoryField(CItemData::FieldType field) const = 0;
  virtual bool IsPreselectedField(CItemData::FieldType field) const = 0;
  virtual bool IsUsableField(CItemData::FieldType field) const = 0;
  virtual bool ShowFieldSelection() const = 0;
  virtual wxString GetTaskWord() const = 0;
  
public:
  SelectionCriteria* m_criteria;
  bool m_autoValidate;
};

template <class DlgType>
class AdvancedSelectionImpl: public AdvancedSelectionPanel
{
public:
  AdvancedSelectionImpl(wxWindow* wnd, SelectionCriteria* existingCriteria, bool autoValidate):
    AdvancedSelectionPanel(wnd, existingCriteria, autoValidate)
  {}

  virtual bool IsMandatoryField(CItemData::FieldType field) const {
    return DlgType::IsMandatoryField(field);
  }
  
  virtual bool IsPreselectedField(CItemData::FieldType field) const {
    return DlgType::IsPreselectedField(field);
  }

  virtual bool IsUsableField(CItemData::FieldType field) const {
    return DlgType::IsUsableField(field);
  }

  virtual bool ShowFieldSelection() const {
    return DlgType::ShowFieldSelection();
  }
  
  virtual wxString GetTaskWord() const {
    return DlgType::GetTaskWord();
  }
};

template <class DlgType>
class AdvancedSelectionDlg : public wxDialog
{
  DECLARE_CLASS(AdvancedSelectionDlg)

  typedef AdvancedSelectionImpl<DlgType> PanelType;
  PanelType* m_panel;

public:
  AdvancedSelectionDlg(wxWindow* parent, SelectionCriteria* existingCriteria): m_panel(0)
  {
    wxDialog::Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, 
                            wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
  

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddSpacer(TopMargin);

    m_panel = new PanelType(this, existingCriteria, true);
    m_panel->CreateControls(this);
    sizer->Add(m_panel, wxSizerFlags().Expand().Proportion(1));
    
    sizer->AddSpacer(RowSeparation);
    sizer->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL|wxHELP), 
                      wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand());
    sizer->AddSpacer(BottomMargin);
    
    SetTitle(DlgType::GetAdvancedSelectionTitle());
    
    SetSizerAndFit(sizer);
  }
};

#endif
