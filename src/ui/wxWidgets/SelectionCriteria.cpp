/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SelectionCriteria.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "./SelectionCriteria.h"

CItemData::FieldType subgroups[] = {  CItemData::GROUP,
                                      CItemData::GROUPTITLE,
                                      CItemData::NOTES,
                                      CItemData::TITLE,
                                      CItemData::URL,
                                      CItemData::USER
                                  } ;

struct _subgroupFunctions {
  const wxString name;
  PWSMatch::MatchRule function;
  // Following ctor's required to shut up some compiler warnings
  _subgroupFunctions() : name(wxEmptyString), function(PWSMatch::MR_INVALID) {}
  _subgroupFunctions(const wxString &aname, PWSMatch::MatchRule afunction) :
    name(aname), function(afunction) {}
} subgroupFunctions[] = {                         {_("equals"),              PWSMatch::MR_EQUALS},
                                                  {_("does not equal"),      PWSMatch::MR_NOTEQUAL},
                                                  {_("begins with"),         PWSMatch::MR_BEGINS},
                                                  {_("does not begin with"), PWSMatch::MR_NOTBEGIN},
                                                  {_("ends with"),           PWSMatch::MR_ENDS},
                                                  {_("does not end with"),   PWSMatch::MR_NOTEND},
                                                  {_("contains"),            PWSMatch::MR_CONTAINS},
                                                  {_("does not contain"),    PWSMatch::MR_NOTCONTAIN} } ;

CItemData::FieldType selectableFields[] = { CItemData::GROUP,
                                            CItemData::TITLE,
                                            CItemData::USER,
                                            CItemData::PASSWORD,
                                            CItemData::NOTES,
                                            CItemData::URL,
                                            CItemData::AUTOTYPE,
                                            CItemData::PWHIST,
                                            CItemData::POLICY,
                                            CItemData::POLICYNAME,
                                            CItemData::SYMBOLS,
                                            CItemData::RUNCMD,
                                            CItemData::EMAIL,
                                            CItemData::DCA,
                                            CItemData::SHIFTDCA,
                                            CItemData::PROTECTED,
                                            CItemData::CTIME,
                                            CItemData::ATIME,
                                            CItemData::XTIME,
                                            CItemData::XTIME_INT,
                                            CItemData::PMTIME,
                                            CItemData::RMTIME,
                                      };

////////////////////////////////////////////////////////////////////////////
// SelectionCriteria implementation

//static
size_t SelectionCriteria::GetNumSubgroups(void) {
  return WXSIZEOF(subgroups);
}

//static
CItemData::FieldType SelectionCriteria::GetSubgroup(size_t idx) {
  wxASSERT_MSG(idx < GetNumSubgroups(), wxT("Invalid index for GetSubgroups"));
  return subgroups[idx];
}

//static
wxString SelectionCriteria::GetSelectableFieldName(CItemData::FieldType ft) {
  return towxstring(CItemData::FieldName(ft));
}

//static
size_t SelectionCriteria::GetNumFieldsSelectable() {
  return WXSIZEOF(selectableFields);
}

CItemData::FieldType SelectionCriteria::GetSelectableField(size_t idx) {
  wxASSERT_MSG(idx < GetNumFieldsSelectable(), wxT("Invalid index for GetSelectableField"));
  return selectableFields[idx];
}

wxString SelectionCriteria::GetGroupSelectionDescription() const
{
  if (!m_fUseSubgroups)
    return _("All entries");
  else
    return wxString(_("Entries whose ")) << GetSelectableFieldName(subgroups[m_subgroupObject]) << wxS(' ')
            << subgroupFunctions[m_subgroupFunction].name << wxS(" \"") << m_subgroupText
                                         << wxS("\" [") << (m_fCaseSensitive? wxS("") : _("not ")) << _("case-sensitive]");
}

//static
size_t SelectionCriteria::GetNumSubgroupFunctions()
{
  return WXSIZEOF(subgroupFunctions);
}

//static
wxString SelectionCriteria::GetSubgroupFunctionName(size_t idx)
{
  wxASSERT_MSG(idx < GetNumSubgroupFunctions(), wxT("Invalid index for GetSubgroupFunctionName"));
  return subgroupFunctions[idx].name;
}
//static
PWSMatch::MatchRule SelectionCriteria::GetSubgroupFunction(size_t idx)
{
  wxASSERT_MSG(idx < GetNumSubgroupFunctions(), wxT("Invalid index for GetSubgroupFunction"));
  return subgroupFunctions[idx].function;
}

//returns true if all fields have been selected
bool SelectionCriteria::GetFieldSelection(wxArrayString& selectedFields, wxArrayString& unselectedFields)
{
  for (size_t idx = 0; idx < GetNumFieldsSelectable(); ++idx) {
    if (m_bsFields.test(selectableFields[idx]))
      selectedFields.Add(GetSelectableFieldName(selectableFields[idx]));
    else
      unselectedFields.Add(GetSelectableFieldName(selectableFields[idx]));
  }
  return m_bsFields.count() == NumberOf(selectableFields);
}
