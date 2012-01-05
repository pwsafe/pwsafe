/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __ADVANCEDDSELECTIONDLG_H__
#define __ADVANCEDDSELECTIONDLG_H__


#include "../../core/ItemData.h"
#include "./wxutils.h"

extern CItemData::FieldType subgroups[];
size_t GetNumFieldsSelectable();
CItemData::FieldType GetSelectableField(size_t idx);

struct _subgroupFunctions {
  const charT* name;
  PWSMatch::MatchRule function;
};
extern struct _subgroupFunctions subgroupFunctions[];


/*!
 * SelectionCriteria class declaration
 */

struct SelectionCriteria 
{
  SelectionCriteria() : m_fCaseSensitive(false),
                        m_fUseSubgroups(false),
                        m_subgroupObject(0),            // index into subgroups array defined in .cpp
                        m_subgroupFunction(0),          // index into subgroupFunctions array defined in .cpp
                        m_fDirty(false)
  {
    SelectAllFields();
  }
  
  SelectionCriteria(const SelectionCriteria& other):  m_fCaseSensitive(other.m_fCaseSensitive),
                                                      m_bsFields(other.m_bsFields),
                                                      m_subgroupText(other.m_subgroupText),
                                                      m_fUseSubgroups(other.m_fUseSubgroups),
                                                      m_subgroupObject(other.m_subgroupObject),
                                                      m_subgroupFunction(other.m_subgroupFunction),
                                                      m_fDirty(false)
  {}

private:
  bool                  m_fCaseSensitive;
  CItemData::FieldBits  m_bsFields;
  wxString              m_subgroupText;
  bool                  m_fUseSubgroups;
  int                   m_subgroupObject;
  int                   m_subgroupFunction;
  bool                  m_fDirty;

public:
  bool IsDirty(void) const { return m_fDirty; }
  void Clean(void) { m_fDirty = false; }
  
  bool HasSubgroupRestriction() const             { return m_fUseSubgroups; }
  CItemData::FieldBits GetSelectedFields() const  { return m_bsFields; }
  size_t GetNumSelectedFields() const             { return m_bsFields.count(); }
  wxString SubgroupSearchText() const             { return m_subgroupText; }
  bool CaseSensitive() const                      { return m_fCaseSensitive; }
  CItemData::FieldType SubgroupObject() const     { return subgroups[m_subgroupObject];}
  PWSMatch::MatchRule  SubgroupFunction() const   { return subgroupFunctions[m_subgroupFunction].function; }
  int  SubgroupFunctionWithCase() const           { return m_fCaseSensitive? -SubgroupFunction(): SubgroupFunction(); }
  void SelectAllFields()                          { for(size_t idx = 0; idx < GetNumFieldsSelectable(); ++idx) 
                                                      m_bsFields.set(GetSelectableField(idx));
                                                  }
  void SelectField(CItemData::FieldType ft)       { m_bsFields.set(ft); }
  void ResetField(CItemData::FieldType ft)        { m_bsFields.reset(ft); }
  size_t SelectedFieldsCount() const              { return m_bsFields.count(); }
  size_t TotalFieldsCount() const                 { return m_bsFields.size(); }
  bool IsFieldSelected(CItemData::FieldType ft) const { return m_bsFields.test(ft); }

  bool MatchesSubgroupText(const CItemData& item) const {
    //could be very inefficient in a loop across the entire DB
    return !m_fUseSubgroups || item.Matches(tostdstring(m_subgroupText), SubgroupObject(), SubgroupFunction());
  }
  
  wxString GetGroupSelectionDescription() const;
  //returns true if all fields have been selected
  bool GetFieldSelection(wxArrayString& selectedFields, wxArrayString& unselectedFields);

SelectionCriteria& operator=(const SelectionCriteria& data) {
    m_fCaseSensitive    = data.m_fCaseSensitive;
    m_bsFields          = data.m_bsFields;
    m_subgroupText      = data.m_subgroupText;
    m_fUseSubgroups     = data.m_fUseSubgroups;
    m_subgroupObject    = data.m_subgroupObject;
    m_subgroupFunction  = data.m_subgroupFunction;
    
    m_fDirty = true;
    
    return *this;
  }
  friend class AdvancedSelectionPanel;
  friend bool operator!=(const SelectionCriteria& a, const SelectionCriteria& b);
};

inline bool operator!=(const SelectionCriteria& a, const SelectionCriteria& b)
{
  return a.m_bsFields         != b.m_bsFields         || 
         a.m_fCaseSensitive   != b.m_fCaseSensitive   ||
         a.m_fUseSubgroups    != b.m_fUseSubgroups    ||
         a.m_subgroupFunction != b.m_subgroupFunction ||
         a.m_subgroupText     != b.m_subgroupText     ||
         a.m_subgroupObject   != b.m_subgroupObject; 
}



/*!
 * AdvancedSelectionDlg class declaration
 */

class AdvancedSelectionPanel: public wxPanel
{
  DECLARE_CLASS(AdvancedSelectionPanel)
  DECLARE_EVENT_TABLE()

  DECLARE_NO_COPY_CLASS(AdvancedSelectionPanel)

public:
  AdvancedSelectionPanel(wxWindow* wnd, const SelectionCriteria& existingCriteria, bool autoValidate);

  bool Validate();               //overriden from wxWindow
  bool TransferDataToWindow();   //overriden from wxWindow
  bool TransferDataFromWindow(); //overriden from wxWindow
  void OnSelectSome( wxCommandEvent& evt );
  void OnSelectAll( wxCommandEvent& evt );
  void OnRemoveSome( wxCommandEvent& evt );
  void OnRemoveAll( wxCommandEvent& evt );

  void CreateControls(wxWindow* parentWnd);
  bool DoValidation();               //actual validator

protected:
  virtual bool IsMandatoryField(CItemData::FieldType field) const = 0;
  virtual bool ShowFieldSelection() const = 0;
  virtual wxString GetTaskWord() const = 0;
  
public:
  SelectionCriteria m_criteria;
  bool m_autoValidate;
};

template <class DlgType>
class AdvancedSelectionImpl: public AdvancedSelectionPanel
{
public:
  AdvancedSelectionImpl(wxWindow* wnd, const SelectionCriteria& existingCriteria, bool autoValidate):
    AdvancedSelectionPanel(wnd, existingCriteria, autoValidate)
  {}

  virtual bool IsMandatoryField(CItemData::FieldType field) const {
    return DlgType::IsMandatoryField(field);
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
  AdvancedSelectionDlg(wxWindow* parent, const SelectionCriteria& existingCriteria): m_panel(0)
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

  SelectionCriteria GetSelectionCriteria() const { return m_panel->m_criteria; }
  
  void GetSelectionCriteria(SelectionCriteria& other) const { 
    if (other != m_panel->m_criteria)
      other = m_panel->m_criteria;
  }

};


#endif
