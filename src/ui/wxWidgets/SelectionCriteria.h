/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __SELECTIONCRITERIA_H__
#define __SELECTIONCRITERIA_H__

#include "../../core/ItemData.h"
#include "./wxutils.h"

/*
 * SelectionCriteria data structure backs the AdvancedSelectionPanel widget
 * 
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
  CItemData::FieldType SubgroupObject() const     { return GetSubgroup(m_subgroupObject);}
  PWSMatch::MatchRule  SubgroupFunction() const   { return GetSubgroupFunction(m_subgroupFunction); }
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

  // static helpers for outsiders
  static size_t GetNumSubgroups(void);
  static CItemData::FieldType GetSubgroup(size_t idx);
  static size_t GetNumFieldsSelectable();
  static CItemData::FieldType GetSelectableField(size_t idx);
  static wxString GetSelectableFieldName(CItemData::FieldType ft);
  static size_t GetNumSubgroupFunctions();
  static wxString GetSubgroupFunctionName(size_t idx);
  static PWSMatch::MatchRule GetSubgroupFunction(size_t idx);

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

#endif
