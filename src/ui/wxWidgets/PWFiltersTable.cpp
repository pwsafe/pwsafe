/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersTable.cpp
* 
*/

////@begin includes

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "wx/string.h"
#include "wx/grid.h"

#include "core/core.h"
#include "core/ItemData.h"
#include "PWFiltersTable.h"
#include "PWFiltersGrid.h"
#include "PWFiltersEditor.h"
#include "core/PWSFilters.h"
#include "wxUtilities.h"

////@end includes

////@begin XPM images
////@end XPM images

/*!
 * pwFiltersTable type definition
 */

IMPLEMENT_CLASS(pwFiltersTable, wxGridTableBase)

/*!
 * pwFiltersTable constructor
 */

pwFiltersTable::pwFiltersTable(pwFiltersGrid* pGrid) : m_pwsgrid(pGrid)
{
}

/*!
 * pwFiltersTable destructor
 */

pwFiltersTable::~pwFiltersTable()
{
}

/*!
 * GetNumberRows Number of rows to be shown on the screen
 */

int pwFiltersTable::GetNumberRows()
{
  ASSERT(m_pwsgrid);
  if(IsReadOnly()) {
    // In read only we show each field and plus heading if history, policy and attachement is present
    return m_pwsgrid->GetNumMRows() +
           (m_pwsgrid->GetNumHRows() ? 1 : 0) + m_pwsgrid->GetNumHRows() +
           (m_pwsgrid->GetNumPRows() ? 1 : 0) + m_pwsgrid->GetNumPRows() +
           (m_pwsgrid->GetNumARows() ? 1 : 0) + m_pwsgrid->GetNumARows();
  }
  // In read write we have only the either main, history, policy or attachment
  return m_pwsgrid->GetNumRows();
}

/*!
 * GetNumberCols Number of columns to be shown on the screen (is fix)
 */

int pwFiltersTable::GetNumberCols()
{
  return FLC_NUM_COLUMNS;
}

/*!
 * IsEmptyCell Check on empty cell. Logic field of row 0 is left empty.
 */

bool pwFiltersTable::IsEmptyCell(int row, int col)
{
  if(col == FLC_LGC_COMBOBOX && row == 0) // First logic line symbol left empty
    return true;
  return false;
}

/*!
 * GetColLabelString Column heading label.
 */

wxString pwFiltersTable::GetColLabelString(int col)
{
  switch(col) {
    case FLC_FILTER_NUMBER:
      return wxString(L" # ");
    case FLC_ENABLE_BUTTON:
      return wxString(L" ? ");
    case FLC_ADD_BUTTON:
      return wxString(L" + ");
    case FLC_REM_BUTTON:
      return wxString(L" - ");
    case FLC_LGC_COMBOBOX:
      return wxString(_("And/Or"));
    case FLC_FLD_COMBOBOX:
      return wxString(_("Field"));
    case FLC_CRITERIA_TEXT:
      return wxString(_("Criteria"));
    default:
      break;
  }
  return wxEmptyString;
}

/*!
 * GetColLabelValue Column heading label. value
 */

wxString pwFiltersTable::GetColLabelValue(int col)
{
  return GetColLabelString(col);
}

/*!
 * GetTypeName Type name of column
 */

wxString pwFiltersTable::GetTypeName(int WXUNUSED(row), int col)
{
  if(IsReadOnly())
    return wxGRID_VALUE_STRING;
  
  switch(col) {
    case FLC_FILTER_NUMBER:
      return wxGRID_VALUE_NUMBER;
    case FLC_ENABLE_BUTTON:
      return wxGRID_VALUE_BOOL; // Specified as Bool, although value can be 0 to 3 (0 and 1 is bool)
    case FLC_ADD_BUTTON:
      return wxGRID_VALUE_STRING;
    case FLC_REM_BUTTON:
      return wxGRID_VALUE_STRING;
    case FLC_LGC_COMBOBOX:
      return wxGRID_VALUE_NUMBER; // Enumerated value
    case FLC_FLD_COMBOBOX:
      return wxGRID_VALUE_NUMBER; // Enumerated value
    case FLC_CRITERIA_TEXT:
      return wxGRID_VALUE_STRING;
    default:
      break;
  }
  return wxEmptyString;
}

/*!
 * CanGetValueAs Check on type string matching to column, type can be used for GetValue()
 */

bool pwFiltersTable::CanGetValueAs(int WXUNUSED(row), int col, const wxString& typeName)
{
  if(IsReadWrite() && (typeName.CompareTo(wxGRID_VALUE_BOOL) == 0) && (col == FLC_ENABLE_BUTTON)) // Get enable as 3 enum value, but when ask as boolean return activation only
    return true;
  if(IsReadWrite() && (typeName.CompareTo(wxGRID_VALUE_NUMBER) == 0) && ((col == FLC_ENABLE_BUTTON) ||
                                                        (col == FLC_LGC_COMBOBOX) ||
                                                        (col == FLC_FILTER_NUMBER) ||
                                                        (col == FLC_FLD_COMBOBOX)))
    return true;
  if((typeName.CompareTo(wxGRID_VALUE_STRING) == 0) && (IsReadOnly() ||
                                                        (col == FLC_ADD_BUTTON) ||
                                                        (col == FLC_REM_BUTTON) ||
                                                        (col == FLC_CRITERIA_TEXT)))
    return true;
  return false;
}

/*!
 * CanSetValueAs Check on type string matching to column, type can be used for SetValue()
 */

bool pwFiltersTable::CanSetValueAs(int WXUNUSED(row), int col, const wxString& typeName)
{
  if(IsReadOnly()) // Read only cannot set any value
    return false;
  if((typeName.CompareTo(wxGRID_VALUE_BOOL) == 0) && (col == FLC_ENABLE_BUTTON))
    return true;
  if((typeName.CompareTo(wxGRID_VALUE_NUMBER) == 0) && ((col == FLC_ENABLE_BUTTON) ||
                                                        (col == FLC_LGC_COMBOBOX) ||
                                                        (col == FLC_FLD_COMBOBOX)))
    return true; // Filter Number cannot be set
  if((typeName.CompareTo(wxGRID_VALUE_STRING) == 0) && ((col == FLC_ADD_BUTTON) ||
                                                        (col == FLC_REM_BUTTON) ||
                                                        (col == FLC_CRITERIA_TEXT)))
    return false; // We cannot set a value on such entries
  return false;
}

/*!
 * GetValue Returns value to be shown on screen, value given as string
 * With read only mode all grid content is shown in one grid, intermediate header is shown with logic and criteria as "-" and field type as name of following group
 */

wxString pwFiltersTable::GetValue(int row, int col)
{
  wxString result(wxEmptyString);
  FilterType ro_ft = DFTYPE_INVALID;
  vFilterRows *ro_filter = nullptr;
  int ro_row = -1;
  
  // Determine values in ReadOnly mode
  if(IsReadOnly()) {
    ro_ft = m_pwsgrid->GetFilterAndRow(row, &ro_filter, ro_row);
    ASSERT(ro_ft != DFTYPE_INVALID);
    wxASSERT(ro_filter && ((ro_row == -1) || (ro_row < static_cast<int>(ro_filter->size()))));
  }
  
  switch(col) {
    case FLC_FILTER_NUMBER:
      if(IsReadOnly()) {
        if(ro_row >= 0)
          result = wxString::Format("%d", ro_row + 1);
      }
      else
        result = wxString::Format("%d", row + 1);
      break;
    case FLC_ENABLE_BUTTON:
      // Logic is not 100% the same as in windows, but looks better for me:
      // - UNCHECKED when not active,
      // - CHECKED when active and complete,
      // - DISABLED when active but not complete
      if(IsReadOnly()) {
        if(ro_row >= 0) {
          wxASSERT(ro_filter);
          if((*ro_filter)[ro_row].bFilterActive) {
            if((*ro_filter)[ro_row].bFilterComplete) {
              result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_CHECKED);
            }
            else
              result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_DISABLED);
          }
          else
            result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_UNCHECKED);
        }
        else if(ro_row)
          result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_UNCHECKED_DISABLED); // Special value for disabled
      }
      else if(m_pwsgrid->IsRowActive(row)) {
        if(m_pwsgrid->IsRowComplete(row)) {
          result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_CHECKED);
        }
        else
          result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_DISABLED);
      }
      else
        result = pwFiltersActiveRenderer::GetCellValueOfCheckType(PWF_UNCHECKED);
      break;
    case FLC_ADD_BUTTON:
      result = L" + ";
      break;
    case FLC_REM_BUTTON:
      result = L" - ";
      break;
    case FLC_LGC_COMBOBOX:
      if(IsReadOnly()) {
        if(ro_row > 0) {
          wxASSERT(ro_filter);
          LogicConnect ltype = (*ro_filter)[ro_row].ltype;
          if(ltype == LC_AND) {
            result = wxString(_("And"));
          }
          else if(ltype == LC_OR) {
            result = wxString(_("Or"));
          }
        }
        else if(ro_row == -1)
          result = L"-----";
      }
      else if(row != 0) {
        LogicConnect ltype = m_pwsgrid->RowLC(row);
        if(ltype == LC_AND) {
          result = wxString(_("And"));
        }
        else if(ltype == LC_OR) {
          result = wxString(_("Or"));
        }
        /* else wxEmptString */
      }
      break;
    case FLC_FLD_COMBOBOX:
    {
      if(IsReadOnly()) {
        if(ro_row >= 0) {
          wxASSERT(ro_filter);
          result = pwFiltersFTChoiceRenderer::getFieldTypeString((*ro_filter)[ro_row].ftype);
        }
        else {
          switch(ro_ft) {
            case DFTYPE_MAIN:
              wxASSERT(false); // Should never happen
              break;
            case DFTYPE_PWHISTORY:
              result = _T("---") + _("Password History") + _T("---");
              break;
            case DFTYPE_PWPOLICY:
              result = _T("---") + _("Password Policy") + _T("---");
              break;
            case DFTYPE_ATTACHMENT:
              result = _T("---") + _("Attachment") + _T("---");
              break;
            default:
              wxASSERT(false);
              break;
          }
        }
      }
      else {
        FieldType ft = m_pwsgrid->RowFieldType(row);

        result = pwFiltersFTChoiceRenderer::getFieldTypeString(ft);
      }
      break;
    }
    case FLC_CRITERIA_TEXT:
      if(IsReadOnly()) {
        if(ro_row >= 0) {
          result = wxString(PWSFilters::GetFilterDescription((*ro_filter)[ro_row]));
        }
        else
          result = L"----------------------------";
      }
      else if((m_pwsgrid->RowFieldType(row) == FT_PWHIST) && m_pwsgrid->IsSetPWHist()) {
        result = LoadAString(IDSC_SEEPWHISTORYFILTERS);
      } else if((m_pwsgrid->RowFieldType(row) == FT_POLICY) && m_pwsgrid->IsSetPolicy()) {
        result = LoadAString(IDSC_SEEPWPOLICYFILTERS);
      } else if((m_pwsgrid->RowFieldType(row) == FT_ATTACHMENT) && m_pwsgrid->IsSetAttachment()) {
        result = LoadAString(IDSC_SEEATTACHMENTFILTERS);
      } else if(m_pwsgrid->RowMatchType(row) == PWSMatch::MT_INVALID || m_pwsgrid->RowMatchRule(row) == PWSMatch::MR_INVALID) {
        result = wxString(NOCRITERIADEFINED);
      }
      else
        result = wxString(PWSFilters::GetFilterDescription(m_pwsgrid->FilterRow(row)));
      break;
    default:
      break;
  }
  return result;
}

/*!
 * GetCriteriaMinSize Get minimum size of pre-defined criteria strings, using the drawing context for formatting.
 */

wxSize pwFiltersTable::GetCriteriaMinSize(wxClientDC &dc)
{
  wxSize size;
  
  size = dc.GetTextExtent(wxString(NOCRITERIADEFINED));
  size.IncTo(dc.GetTextExtent(LoadAString(IDSC_SEEPWHISTORYFILTERS)));
  size.IncTo(dc.GetTextExtent(LoadAString(IDSC_SEEPWPOLICYFILTERS)));
  size.IncTo(dc.GetTextExtent(LoadAString(IDSC_SEEATTACHMENTFILTERS)));
  return size;
}

/*!
 * GetValueAsLong Returns value to be shown on screen, value given as long value.
 * Used in read write only.
 */

long pwFiltersTable::GetValueAsLong(int row, int col)
{
  long result = 0;
  
  if(col == FLC_FILTER_NUMBER) {
    result = static_cast<long>(row + 1);
  }
  else if(col == FLC_ENABLE_BUTTON) {
    if(m_pwsgrid->IsRowActive(row)) {
      if(m_pwsgrid->IsRowComplete(row)) {
        result = static_cast<long>(PWF_CHECKED);
      }
      else
        result = static_cast<long>(PWF_DISABLED);
    }
    else
      result = static_cast<long>(PWF_UNCHECKED);
  }
  else if(col == FLC_LGC_COMBOBOX) {
    if(row == 0) {
      result = static_cast<long>(LC_INVALID);
    }
    else
      result = static_cast<long>(m_pwsgrid->RowLC(row));
  }
  else if(col == FLC_FLD_COMBOBOX) {
    result = static_cast<long>(m_pwsgrid->RowFieldType(row));
  }
  else {
    ASSERT(false);
  }
  
  return result;
}

/*!
 * GetValueAsBool Returns value to be shown on screen, value given as boolean value.
 * Used in read write only.
 */

bool pwFiltersTable::GetValueAsBool(int row, int col)
{
  bool result = false;
  
  if(col == FLC_ENABLE_BUTTON) {
    result = m_pwsgrid->IsRowActive(row);
  }
  else {
    ASSERT(false);
  }
  return result;
}

/*!
 * SetValue Set value from screen editing screen, value given as string value.
 * Used in read write only.
 */

void pwFiltersTable::SetValue(int row, int col, const wxString& value)
{
  if(col == FLC_ENABLE_BUTTON) {
    m_pwsgrid->SetRowActive(row, ((value.CompareTo(L"0") == 0) || value.empty()) ? false : true);
  }
  else if(col == FLC_LGC_COMBOBOX) {
    long num = 0;
    
    if(row == 0) { // Should not been done, as read only, but in case...
      m_pwsgrid->SetRowLC(row, LC_OR);
    }
    else if(value.empty()) {
      m_pwsgrid->SetRowLC(row, LC_INVALID);
    }
    else if(! value.ToLong(&num)) {
      if(wxString(_("And")).CompareTo(value.c_str()) == 0) {
        m_pwsgrid->SetRowLC(row, LC_AND);
      }
      else if(wxString(_("Or")).CompareTo(value.c_str()) == 0) {
        m_pwsgrid->SetRowLC(row, LC_OR);
      }
      else {
        m_pwsgrid->SetRowLC(row, LC_INVALID);
      }
    }
    else {
      m_pwsgrid->SetRowLC(row, static_cast<LogicConnect>(num));
    }
  }
  else if(col == FLC_FLD_COMBOBOX) {
    long num = 0;
    
    if(value.empty()) {
      num = 0;
    }
    else if(! value.ToLong(&num)) {
      wxFAIL_MSG( wxT("this cell doesn't have numeric value") );
      num = 0;
    }
    m_pwsgrid->SetRowFieldType(row, static_cast<FieldType>(num));
    if(row == 0)
      m_pwsgrid->SetRowLC(row, LC_OR);
  }
  // At the end check if row is complete
  m_pwsgrid->DoCheckFilterIsComplete(row);
}

/*!
 * SetValueAsLong Set value from screen editing screen, value given as long value.
 * Used in read write only.
 */

void pwFiltersTable::SetValueAsLong(int row, int col, long value)
{
  if(col == FLC_ENABLE_BUTTON) {
    m_pwsgrid->SetRowActive(row, value ? true : false);
  }
  else if(col == FLC_LGC_COMBOBOX) {
    if(row == 0) { // Should not been done, as read only, but in case...
      m_pwsgrid->SetRowLC(row, LC_OR);
    }
    else
      m_pwsgrid->SetRowLC(row, static_cast<LogicConnect>(value));
  }
  else if(col == FLC_FLD_COMBOBOX) {
    m_pwsgrid->SetRowFieldType(row, static_cast<FieldType>(value));
    if(row == 0)
      m_pwsgrid->SetRowLC(row, LC_OR);
  }
  else {
    ASSERT(false);
  }
}

/*!
 * SetValueAsBool Set value from screen editing screen, value given as bool value.
 * Used in read write only.
 */

void pwFiltersTable::SetValueAsBool(int row, int col, bool value)
{
  if(col == FLC_ENABLE_BUTTON) {
    m_pwsgrid->SetRowActive(row, value);
  }
  else {
    ASSERT(false);
  }
}

/*!
 * Clear the screen and remove all filter from table and screen.
 * Used in read only to update the shown filter.
 */

void pwFiltersTable::Clear()
{
  size_t curNumRows = GetNumberRows();

  if (GetView() && curNumRows > 0) {
    //This will actually remove the item from grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           static_cast<int>(0),
                           static_cast<int>(curNumRows));
    GetView()->ProcessTableMessage(msg);
  }
  m_pwsgrid->ResetFilter();
}

/*!
 * RefreshRowsCount Update the count of rows on the screen to fit the new filter.
 * Used in read only to update the shown filter.
 */

void pwFiltersTable::RefreshRowsCount()
{
  size_t curNumRows = GetNumberRows();

  if (GetView() && curNumRows > 0) {
    //This will actually add the item to grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                           static_cast<int>(curNumRows));
    GetView()->ProcessTableMessage(msg);
  }
}

/*!
 * DeleteRows Delete demanded rows on screen and in filter table
 * Used in read write only.
 */

bool pwFiltersTable::DeleteRows(size_t pos, size_t numRows)
{
  ASSERT(m_pwsgrid);
  size_t curNumRows = GetNumberRows();
  
  if (pos >= curNumRows) {
    // Position is behind the length of list
    wxFAIL_MSG( wxString(wxT("pwFiltersTable::DeleteRows(")) << "pos= " << pos << ", numRows= " << numRows << ") call is invalid\nPos value is invalid for present table with " << curNumRows << " rows");
    return false;
  }
  // Limit the number for rows to the existing number (should ne be happen, but in case it is needed left here)
  if (numRows > curNumRows - pos)
    numRows = curNumRows - pos;
  // Delete demanded row in table
  m_pwsgrid->DoRowsDelete(static_cast<int>(pos), static_cast<int>(numRows));
  // Update screen
  if (GetView()) {
    //This will actually remove the item from grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           static_cast<int>(pos),
                           static_cast<int>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  // Update logic field of first line to be read only, when first line had been removed before
  if(pos == 0 && GetNumberRows() > 0)
    m_pwsgrid->SetReadOnly(0, FLC_LGC_COMBOBOX);
  // Positive end
  return true;
}

/*!
 * AppendRows Append demanded rows on screen and in filter table
 * Used in read write only.
 */

bool pwFiltersTable::AppendRows(size_t numRows/*=1*/)
{
  ASSERT(m_pwsgrid);
  bool bFirst = (GetNumberRows() == 0);
  // Append row at end of the table
  m_pwsgrid->DoRowAppend(numRows);
  // Update screen
  if (GetView()) {
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                           static_cast<int>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  // Update logic field of first line to be read only, when first line had been added
  if(bFirst)
    m_pwsgrid->SetReadOnly(0, FLC_LGC_COMBOBOX);
  // Allways positive end
  return true;
}

/*!
 * InsertRows Insert demanded rows on screen and in filter table
 * Used in read write only.
 */

bool pwFiltersTable::InsertRows(size_t pos/*=0*/, size_t numRows/*=1*/)
{
  ASSERT(m_pwsgrid);
  // Update logic field of first line to be read write, when entry added before
  if(pos == 0)
    m_pwsgrid->SetReadOnly(0, FLC_LGC_COMBOBOX, false);
  // Insert row at demanded position
  m_pwsgrid->DoRowInsert(pos, numRows);
  // Update screen
  if (GetView()) {
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                           static_cast<int>(pos),
                           static_cast<int>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  // Update logic field of first line to be read only, when first line had been added
  if(pos == 0)
    m_pwsgrid->SetReadOnly(0, FLC_LGC_COMBOBOX);
  // Allways positive end
  return true;
}
