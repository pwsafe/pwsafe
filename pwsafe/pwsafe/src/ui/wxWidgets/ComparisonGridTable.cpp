/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ComparisonGridTable.cpp
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

#include "./ComparisonGridTable.h"

#include "./AdvancedSelectionDlg.h"
#include "../../core/PWScore.h"
#include "./wxutils.h"


ComparisonGridTable::ComparisonGridTable(SelectionCriteria* criteria,
                                         CompareData* data,
                                         PWScore* current,
                                         PWScore* other): m_criteria(criteria),
                                                          m_compData(data),
                                                          m_currentCore(current),
                                                          m_otherCore(other),
                                                          m_colFields(new CItemData::FieldType[criteria->GetNumSelectedFields()-1])
{
  //this is the order in which we want to display the comparison grids
  const CItemData::FieldType fields[] = { CItemData::PASSWORD, CItemData::URL, 
                                          CItemData::AUTOTYPE, CItemData::PWHIST, 
                                          CItemData::RUNCMD, CItemData::EMAIL, CItemData::NOTES };
  const size_t ncols = size_t(GetNumberCols());
  for(size_t col = 0, idx = 0; (idx < WXSIZEOF(fields)) && (col < ncols); idx++) {
    if (m_criteria->IsFieldSelected(fields[idx]))
      m_colFields[col++] = fields[idx];
  }
}


int ComparisonGridTable::GetNumberRows()
{
  const size_t N = m_compData->size();
  assert(N <= size_t(std::numeric_limits<int>::max()));
  return int(N);
}

int ComparisonGridTable::GetNumberCols()
{
  //GetSelectedFields() will return at-least G+T+U
  //We club T & U and display only Group + title[user] + fields
  //Hence return one less
  return m_criteria->GetNumSelectedFields() - 1; 
}

bool ComparisonGridTable::IsEmptyCell(int row, int col)
{
  if (row < 0 || size_t(row) > m_compData->size())
    return true;
  if (col < 0 || col > GetNumberCols())
    return true;

  st_CompareData cd = m_compData->at(row);

  if (cd.indatabase == CURRENT || cd.indatabase == COMPARE) {
    PWScore* core = cd.indatabase == CURRENT? m_currentCore: m_otherCore;
    ItemListConstIter itr = core->Find(cd.uuid0);
    if (itr != core->GetEntryEndIter()) {
      if (col == 0)
        return itr->second.IsGroupEmpty();
      else if (col == 1)
        return false; //title is always present
      else
        return itr->second.GetFieldValue(m_colFields[col-2]).empty();
    }
  }
  else{
    ItemListConstIter itr0 = m_currentCore->Find(cd.uuid0);
    ItemListConstIter itr1 = m_otherCore->Find(cd.uuid1);
    wxString val;
    if (itr0 != m_currentCore->GetEntryEndIter()) {
      if (col == 0)
        return itr0->second.IsGroupEmpty() && (itr1 == m_otherCore->GetEntryEndIter() || 
                                                itr0->second.IsGroupEmpty());
      else if (col == 1)
        return false; //title is mandatory
      else
        return itr0->second.GetFieldValue(m_colFields[col-2]).empty() &&
                (itr1 == m_otherCore->GetEntryEndIter() || 
                    itr1->second.GetFieldValue(m_colFields[col-2]).empty());
    }
  }
  return true;
}

wxString ComparisonGridTable::GetValue(int row, int col)
{
  if (size_t(row) < m_compData->size() && col < GetNumberCols()) {
    switch(col) {
      case 0:
        return towxstring(m_compData->at(row).group);
      case 1:
      {
        wxString ret;
        return ret << m_compData->at(row).title << wxT('[') <<  m_compData->at(row).user << wxT(']');
      }
      default:
      {
        st_CompareData cd = m_compData->at(row);
        if (cd.indatabase == CURRENT || cd.indatabase == COMPARE) {
          PWScore* core = cd.indatabase == CURRENT? m_currentCore: m_otherCore;
          ItemListConstIter itr = core->Find(cd.uuid0);
          if (itr != core->GetEntryEndIter())
            return towxstring(itr->second.GetFieldValue(m_colFields[col-2]));
          break;
        }
        else{
          ItemListConstIter itr0 = m_currentCore->Find(cd.uuid0);
          ItemListConstIter itr1 = m_otherCore->Find(cd.uuid1);
          wxString val;
          if (itr0 != m_currentCore->GetEntryEndIter())
            val << itr0->second.GetFieldValue(m_colFields[col-2]);
          if (itr1 != m_otherCore->GetEntryEndIter())
            val << wxT('\n') << itr1->second.GetFieldValue(m_colFields[col-2]);
          return val;
        }
      }
    }
  }
  return wxEmptyString;
}

void ComparisonGridTable::SetValue(int /*row*/, int /*col*/, const wxString& /*value*/)
{
}

wxString ComparisonGridTable::GetColLabelValue(int col)
{
  switch(col) {
    case 0:
      return towxstring(CItemData::FieldName(CItemData::GROUP));
    case 1:
    {
      wxString label;
      label << CItemData::FieldName(CItemData::TITLE) << wxT('[') 
                        << CItemData::FieldName(CItemData::USER) << wxT(']');
      return label;
    }
    default:
      return towxstring(CItemData::FieldName(m_colFields[col-2]));
  }
}
