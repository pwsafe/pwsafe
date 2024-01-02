/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersGrid.cpp
* 
*/

////@begin includes

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/grid.h"
#include <wx/fontdlg.h>
#include <wx/colour.h>

#include "core/core.h"
#include "core/PWScore.h"
#include "core/StringX.h"
#include "core/PWSprefs.h"
#include "PWFiltersGrid.h"
#include "PWFiltersTable.h"
#include "PWFiltersEditor.h"
#include "wxUtilities.h"
#include "SetFiltersDlg.h"
#include "PWFiltersBoolDlg.h"
#include "PWFiltersStringDlg.h"
#include "PWFiltersDCADlg.h"
#include "PWFiltersIntegerDlg.h"
#include "PWFiltersStatusDlg.h"
#include "PWFiltersTypeDlg.h"
#include "PWFiltersDateDlg.h"
#include "PWFiltersMediaDlg.h"
#include "PWFiltersPasswordDlg.h"


////@end includes

////@begin XPM images
////@end XPM images

/*!
 * pwFiltersGrid type definition
 */

IMPLEMENT_DYNAMIC_CLASS( pwFiltersGrid, wxGrid )

/*!
 * pwFiltersGrid event table definition
 */
DEFINE_EVENT_TYPE(EVT_SELECT_GRID_ROW)

BEGIN_EVENT_TABLE( pwFiltersGrid, wxGrid )

////@begin ManageFiltersGrid event table entries
#if wxCHECK_VERSION(3, 1, 5)
  EVT_GRID_RANGE_SELECTED(pwFiltersGrid::OnGridRangeSelect)
#else
  EVT_GRID_RANGE_SELECT(pwFiltersGrid::OnGridRangeSelect)
#endif
  EVT_COMMAND(wxID_ANY, EVT_SELECT_GRID_ROW, pwFiltersGrid::OnAutoSelectGridRow)
////@end ManageFiltersGrid event table entries

END_EVENT_TABLE()

bool pwFiltersGrid::m_subDialogCalled = false;

/*!
 * pwFiltersGrid constructors
 */

pwFiltersGrid::pwFiltersGrid(wxWindow *parent,
                             wxWindowID id,
                             st_filters *pfilters,
                             const FilterType &filtertype,
                             const FilterPool filterpool,
                             const bool bCanHaveAttachments,
                             const std::set<StringX> *psMediaTypes,
                             const bool bAllowSet,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name) : wxGrid(parent, id, pos, size, style, name),
                                                     m_pfilters(pfilters),
                                                     m_bCanHaveAttachments(bCanHaveAttachments),
                                                     m_psMediaTypes(psMediaTypes),
                                                     m_bAllowSet(bAllowSet),
                                                     m_filtertype(filtertype),
                                                     m_filterpool(filterpool)
{
  Init();
  CreateControls();
}

/*!
 * pwFiltersTable destructor
 */

pwFiltersGrid::~pwFiltersGrid()
{
}

/*!
 * Member initialisation
 */

void pwFiltersGrid::Init()
{
  m_currentSelection = -1;
  m_FontHeight = 15; // Default?
  if(m_pfilters) {
    switch(m_filtertype) {
      case DFTYPE_MAIN:
        m_currentFilter = &m_pfilters->vMfldata;
        m_currentCounter = &m_pfilters->num_Mactive;
        break;
      case DFTYPE_PWHISTORY:
        m_currentFilter = &m_pfilters->vHfldata;
        m_currentCounter = &m_pfilters->num_Hactive;
        break;
      case DFTYPE_PWPOLICY:
        m_currentFilter = &m_pfilters->vPfldata;
        m_currentCounter = &m_pfilters->num_Pactive;
        break;
      case DFTYPE_ATTACHMENT:
        m_currentFilter = &m_pfilters->vAfldata;
        m_currentCounter = &m_pfilters->num_Aactive;
        break;
      case DFTYPE_INVALID:
        /* FALLTHROUGH */
      default:
        ASSERT(false);
    }
    if(m_bAllowSet && m_currentFilter->empty()) {
      // On R/W and no entry at the beginning add an empty entry to have a first row to edit
      m_initialyEmpty = true;
      m_currentFilter->emplace_back();
      if(IsRowActive(0)) // When constructor is generting an active row count this one as active
        (*m_currentCounter) = 1;
    }
    else
      m_initialyEmpty = false;
  }
  else {
    m_currentFilter = nullptr;
    m_currentCounter = nullptr;
    m_initialyEmpty = false;
  }
}

/*!
 * Control creation for pwFiltersGrid
 */

void pwFiltersGrid::CreateControls()
{
  SetTable(new pwFiltersTable(this), true, wxGrid::wxGridSelectRows); // true => auto-delete
#ifdef PW_GRID_USE_NATIVE_HEADER
  //column picker is free if built with wx2.9.1
#if wxCHECK_VERSION(2, 9, 1)
  UseNativeColHeader(true);
#endif
#endif
  
  wxFont font(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::TreeFont)));
  if (font.IsOk())
    SetDefaultCellFont(font);

  font = GetDefaultCellFont();
  if(font.IsOk()) {
    wxSize size = font.GetPixelSize();
    m_FontHeight = size.GetHeight();
    // Update Label font to have similar size
    SetLabelFont(font.Italic());
  }
  
  SetDefaultColSize(50, true);
  SetDefaultRowSize(m_FontHeight + 10);
  SetColLabelSize(25);
  SetRowLabelSize(0); // Do not show row numbers, we want to manage by ower own

  DisableDragRowSize();
  SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  SetSelectionForeground(*wxRED);

  for(int i = 0; i < FLC_NUM_COLUMNS; i++) {
    AutoSizeColLabelSize(i);
  }
  // Make +/- columns same size ('+' is bigger than '-')
  SetColSize(FLC_REM_BUTTON, GetColSize(FLC_ADD_BUTTON));
  
  AutoSize();
  
  SetGridColReadOnly(FLC_FILTER_NUMBER);
  SetGridColFormat(FLC_ENABLE_BUTTON, new pwFiltersActiveRenderer(m_FontHeight));
  SetGridColReadOnly(FLC_ADD_BUTTON);
  SetGridColReadOnly(FLC_REM_BUTTON);
  SetGridColFormat(FLC_LGC_COMBOBOX, new pwFiltersLCChoiceRenderer, new pwFiltersLCChoiceEditor);
  SetGridColFormat(FLC_FLD_COMBOBOX, new pwFiltersFTChoiceRenderer, new pwFiltersFTChoiceEditor(m_currentFilter, m_filtertype, m_bCanHaveAttachments));
  SetGridColReadOnly(FLC_CRITERIA_TEXT);
 
  // Set size to a minimum
  wxClientDC dc(GetGridWindow());
  dc.SetFont(GetDefaultCellFont());
  wxSize size = pwFiltersTable::GetCriteriaMinSize(dc);
  if(size.GetWidth() > GetColSize(FLC_CRITERIA_TEXT))
    SetColSize(FLC_CRITERIA_TEXT, size.GetWidth() + 6);
  
  if(!m_bAllowSet) {
    SetColSize(FLC_ADD_BUTTON, 0);
    SetColSize(FLC_REM_BUTTON, 0);
    EnableEditing(false);
  }
  else {
    SetReadOnly(0, FLC_LGC_COMBOBOX); // First line logic is left empty and cannot be selected, is alway LC_OR
    Bind(wxEVT_GRID_CELL_LEFT_CLICK, &pwFiltersGrid::OnCellLeftClick, this);
    EnableEditing(true);
  }
  // Determine minimal size of the Grid
  int width = GetRowLabelSize();
  for(int i = 0; i < FLC_NUM_COLUMNS; i++) {
    width += GetColSize(i);
  }
  wxSize minSize(width + 6, (m_FontHeight + 10) * FLC_DEFAULT_NUM_ROWS);
  SetMinClientSize(minSize);
}

/*!
 * SetGridColFormat is setting format,  renderer and editor for the column. On no editor, set to read only.
 * Set column best size
 */

void pwFiltersGrid::SetGridColFormat(int col, wxGridCellRenderer *renderer, wxGridCellEditor *editor)
{
  ASSERT(renderer);
  wxGridCellAttr *attr = GetTable()->GetAttr(-1, col, wxGridCellAttr::Col);
  if(!attr)
      attr = new wxGridCellAttr;
  attr->SetRenderer(renderer);
  if(editor)
    attr->SetEditor(editor);
  else
    attr->SetReadOnly();
  
  attr->SetFont(GetDefaultCellFont());
  if(col == FLC_FLD_COMBOBOX)
    attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
  else
    attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  
  SetColAttr(col, attr);
  
  // Determine best Coll width
  wxClientDC dc(GetGridColLabelWindow());
#ifdef PW_GRID_USE_NATIVE_HEADER
  dc.SetFont(GetDefaultCellFont());
#else
  dc.SetFont(GetLabelFont());
#endif
  wxSize size = dc.GetTextExtent(GetColLabelValue(col)); // Size of header
  wxClientDC dc_cell(GetGridWindow());
  dc_cell.SetFont(GetDefaultCellFont());
  size.IncTo(renderer->GetBestSize(*this, *attr, dc_cell, 0, col)); // Size of entries
  
  SetColSize(col, size.GetWidth() + PW_COLL_SIZE_EXTEND);
}

/*!
 * SetGridColReadOnly is setting format and set to read only.
 * Set column best size.
 */

void pwFiltersGrid::SetGridColReadOnly(int col)
{
  wxGridCellAttr *attr = GetTable()->GetAttr(-1, col, wxGridCellAttr::Col);
  if(!attr)
      attr = new wxGridCellAttr;

  if(col == FLC_CRITERIA_TEXT)
    attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
  else
    attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr->SetReadOnly();
  
  SetColAttr(col, attr);
    
  // Determine best Coll width
  wxClientDC dc(GetGridColLabelWindow());
#ifdef PW_GRID_USE_NATIVE_HEADER
  dc.SetFont(GetDefaultCellFont());
#else
  dc.SetFont(GetLabelFont());
#endif
  wxSize size = dc.GetTextExtent(GetColLabelValue(col)); // Size of header
  if(GetColSize(col) < (size.GetWidth() + PW_COLL_SIZE_EXTEND))
    SetColSize(col, size.GetWidth() + PW_COLL_SIZE_EXTEND);
}

/*!
 * ResetFilter is setting back all filter content and if R/W add first line for editing.
 */

void pwFiltersGrid::ResetFilter()
{
  if(m_pfilters) {
    m_pfilters->vMfldata.clear();
    m_pfilters->num_Mactive = 0;
    m_pfilters->vHfldata.clear();
    m_pfilters->num_Hactive = 0;
    m_pfilters->vPfldata.clear();
    m_pfilters->num_Pactive = 0;
    m_pfilters->vAfldata.clear();
    m_pfilters->num_Aactive = 0;
    if(m_bAllowSet && m_currentFilter) {
      m_currentFilter->emplace_back();
      if(IsRowActive(0))
        (*m_currentCounter)++;
    }
  }
}

/*!
 * ClearFilter is clearing the table content.
 */

void pwFiltersGrid::ClearFilter()
{
  GetTable()->Clear();
}

/*!
 * RefreshFilter is updating the table with new content. The criteria text column width is updated according new content.
 */

void pwFiltersGrid::RefreshFilter()
{
  static_cast<pwFiltersTable *>(GetTable())->RefreshRowsCount();
  int numrows = GetTable()->GetNumberRows();

  // Update dynamic width critera field size
  for(int row = 0; row < numrows; ++row) {
    // Increase size if needed
    wxClientDC dc(GetGridWindow());
    dc.SetFont(GetDefaultCellFont());
    wxSize size = dc.GetTextExtent(GetTable()->GetValue(row, FLC_CRITERIA_TEXT));
    if(size.GetWidth() > GetColSize(FLC_CRITERIA_TEXT))
      SetColSize(FLC_CRITERIA_TEXT, size.GetWidth() + 6);
  }
}

/*!
 * DoRowAppend is adding new empty line at the end of current table.
 */

void pwFiltersGrid::DoRowAppend(size_t numRows)
{
  if(numRows == 0) return;
  ASSERT(m_currentFilter);
  while(numRows--) {
    m_currentFilter->emplace_back();
    if(IsRowActive(static_cast<int>(m_currentFilter->size() - 1)))
      (*m_currentCounter)++; // Increase counter when new active line is active in default constructor
  }
}

/*!
 * DoRowInsert is adding new empty line(s) at the demanded place of current table (at actual demanded position).
 */

void pwFiltersGrid::DoRowInsert(size_t row, size_t numRows)
{
  if(numRows == 0) return;
  ASSERT(m_currentFilter && row < m_currentFilter->size());
  while(numRows--) {
    m_currentFilter->emplace(m_currentFilter->begin() + row);
    if(IsRowActive(static_cast<int>(row)))
      (*m_currentCounter)++; // Increase counter when new active line is active in default constructor
  }
}

/*!
 * DoRowsDelete is deleting demaded row(s).
 */

void pwFiltersGrid::DoRowsDelete(size_t row, size_t numRows)
{
  if(numRows == 0) return;
  ASSERT(m_currentFilter && ((row + numRows) <= m_currentFilter->size()));
  for(int i = static_cast<int>(row); i < static_cast<int>(row + numRows); ++i) {
    if(IsRowActive(i) && *m_currentCounter)
      (*m_currentCounter)--; // Decrease counter when active line is removed
    if(RowFieldType(i) == FT_PWHIST) {
      ClearPWHist(); // When history is removed, clear filter on password history
    }
    else if(RowFieldType(i) == FT_POLICY) {
      ClearPolicy(); // When policy is removed, clear filter on password policy
    }
    else if(RowFieldType(i) == FT_ATTACHMENT) {
      ClearAttachment(); // When attachement is removed, clear filter on password attachment
    }
  }
  m_currentFilter->erase(m_currentFilter->begin() + row, m_currentFilter->begin() + row + numRows);
}

/*!
 * RefreshRow is updating comlete row on the screen.
 */

void pwFiltersGrid::RefreshRow(int row)
{
  // Calculate rectangle ib screen and ask for referesh
  wxRect rect(CellToRect( row, 0 ));
  rect.x = 0;
  rect.width = GetGridWindow()->GetClientSize().GetWidth();
  int dummy;
  CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
  GetGridWindow()->Refresh( false, &rect );
}

/*!
 * RefreshCell content of a single cell on the screen.
 */

void pwFiltersGrid::RefreshCell(int row, int col)
{
  wxRect rect(CellToRect(row, col));
  CalcScrolledPosition(rect.x, rect.y, &rect.x, &rect.y);
  GetGridWindow()->Refresh(false, &rect);
}

/*!
 * DoCheckFilterIsComplete check if filter can be set to complete.
 */

void pwFiltersGrid::DoCheckFilterIsComplete(int row)
{
  bool completed = IsRowComplete(row);
  if(RowLC(row) != LC_INVALID && RowFieldType(row) != FT_INVALID &&
     ((RowMatchType(row) != PWSMatch::MT_INVALID && RowMatchRule(row) != PWSMatch::MR_INVALID) ||
      (RowFieldType(row) == FT_PWHIST && IsSetPWHist()) ||
      (RowFieldType(row) == FT_POLICY && IsSetPolicy()) ||
      (RowFieldType(row) == FT_ATTACHMENT && IsSetAttachment()))) {
    SetRowComplete(row);
    if(! completed) { // If value changed to true
      RefreshCell(row, FLC_ENABLE_BUTTON);
    }
  }
  else if((RowFieldType(row) == FT_PWHIST && ! IsSetPWHist()) ||
          (RowFieldType(row) == FT_POLICY && ! IsSetPolicy()) ||
          (RowFieldType(row) == FT_ATTACHMENT && ! IsSetAttachment())) {
    // When a history, policy or or attachment field is to be checked and no related part is stored the complete flag must be reset
    ResetRowComplete(row);
    if(completed) { // If value changed to false
      RefreshCell(row, FLC_ENABLE_BUTTON);
    }
  }
}

/*!
 * wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_FILTERGRID
 */

void pwFiltersGrid::OnCellLeftClick(wxGridEvent& event)
{
  int row = event.GetRow(), col = event.GetCol();
  
  if(m_bAllowSet && row < GetNumRows()) { // Check read write and range of row
    if(m_currentSelection != row) { // On change of selected row, mark the row first time, second click will active function
      if(m_currentSelection != -1) {
        ClearSelection();  // No specific clear of row, due to error in wxWidgets 3.1.4
        RefreshRow(m_currentSelection);
      }
      m_currentSelection = row;
      SelectRow(row);
      RefreshRow(row);
    }
    else { // When row is already selected one, check on column to perform related action
      if(col == FLC_ENABLE_BUTTON) {
        // Toogle active field
        bool bIsActive = GetTable()->GetValueAsBool(row, FLC_ENABLE_BUTTON);
        GetTable()->SetValueAsBool(row, col, !bIsActive);
        RefreshCell(row, FLC_ENABLE_BUTTON);
        // Update counter
        if(bIsActive) {
          if(*m_currentCounter)
            (*m_currentCounter)--;
        } else
          (*m_currentCounter)++;
      }
      else if(col == FLC_ADD_BUTTON) {
        ++row; // Append after the actual row, so add one
        if(row == GetNumRows()) {
          GetTable()->AppendRows();
        }
        else {
          GetTable()->InsertRows(row); // Append below current entry
        }
        SetRowLC(row, LC_AND); // New row is always beneath one, and therefore not the first one  - default is AND
        RefreshCell(row, FLC_LGC_COMBOBOX);
        if(! IsVisible(row, FLC_ADD_BUTTON)) {
          MakeCellVisible(row, FLC_ADD_BUTTON);
        }
      }
      else if(col == FLC_REM_BUTTON) {
        // Delete the actual selected row
        GetTable()->DeleteRows(row);
        int num = GetNumRows();
        if(num == 0) {
          // When all columns had been removed, add an empty one
          GetTable()->AppendRows();
          SetReadOnly(0, FLC_LGC_COMBOBOX); // Set the first rows logic field to not changable
        }
        else if(row == 0) {
          // After deleting the first line set this one to OR
          if((RowFieldType(row) != FT_INVALID) || RowLC(row) != LC_INVALID)
            SetRowLC(row, LC_OR);
          SetReadOnly(0, FLC_LGC_COMBOBOX); // Set the first rows logic field to not changable
          RefreshCell(row, FLC_LGC_COMBOBOX);
        }
      }
      else if(col == FLC_CRITERIA_TEXT) {
        CallAfter(&pwFiltersGrid::DoEditCriteria, row);
      }
    }
  }
  event.Skip();
}
void pwFiltersGrid::DoEditCriteria(int row)
{
  // Call editor on criteria field
  if (GetCriterion(row)) { 
    RefreshCell(row, FLC_CRITERIA_TEXT);
    DoCheckFilterIsComplete(row);

    // Increase size if needed
    wxClientDC dc(GetGridWindow());
    dc.SetFont(GetDefaultCellFont());
    wxSize size = dc.GetTextExtent(GetTable()->GetValue(row, FLC_CRITERIA_TEXT));
    if(size.GetWidth() > GetColSize(FLC_CRITERIA_TEXT)) {
      SetColSize(FLC_CRITERIA_TEXT, size.GetWidth() + 6);
    }
  }
}

/*!
 * EVT_GRID_RANGE_SELECT event handler
 * Double check on more than one line or wrong line selection. In case of correction needed add an asynchronous event to call OnAutoSelectGridRow()
 */

void pwFiltersGrid::OnGridRangeSelect(wxGridRangeSelectEvent& evt)
{
  //select grids asynchronously, or else we get in an infinite loop of selections & their notifications
  if (evt.Selecting() && m_currentSelection != -1 && evt.GetTopRow() != evt.GetBottomRow() && m_currentSelection != evt.GetTopRow()) { // More than one row selected
    wxCommandEvent cmdEvent(EVT_SELECT_GRID_ROW);
    cmdEvent.SetEventObject(evt.GetEventObject());
    cmdEvent.SetInt(m_currentSelection);
    GetEventHandler()->AddPendingEvent(cmdEvent);
  }
}

/*!
 * EVT_COMMAND for event handler for EVT_SELECT_GRID_ROW
 * Clear selection and set selected row to the demanded one
 */

void pwFiltersGrid::OnAutoSelectGridRow(wxCommandEvent& evt)
{
  ClearSelection();
  SelectRow(evt.GetInt());
}

/*!
 * wxEVT_CHAR_HOOK event handler for WXK_UP, WXK_DOWN, WXK_LEFT, WXK_RIGHT
 * Interpret key events for changing the selected row
 */

void pwFiltersGrid::OnChar(wxKeyEvent& evt)
{
  int nKey = evt.GetKeyCode();
  
  switch(nKey) {
    case WXK_LEFT:
    case WXK_RIGHT:
    case WXK_UP:
    case WXK_DOWN:
      if(m_currentSelection != -1) {
        int newRow = -1;
        if(nKey == WXK_LEFT || nKey == WXK_UP) {
          // Select prvious row
          if(m_currentSelection > 0)
            newRow = m_currentSelection - 1;
        }
        else if (nKey == WXK_RIGHT || nKey == WXK_DOWN) {
          // Select next row
          if(m_currentSelection < (GetTable()->GetNumberRows() - 1))
            newRow = m_currentSelection + 1;
        }
        if(newRow != -1) {
          // Simulate click on first field to select the demanded row
          wxRect rect = CellToRect(newRow, 0);
          wxRect devRect = BlockToDeviceRect(XYToCell(rect.GetTopLeft()), XYToCell(rect.GetTopRight()));

          wxGridEvent event(GetId(), wxEVT_GRID_CELL_LEFT_CLICK, this, newRow, 0, devRect.GetX()+(devRect.GetWidth()/ 2), devRect.GetY()+(devRect.GetHeight()/2));
          
          GetEventHandler()->AddPendingEvent(event);
        }
      }
      break;
    default:
      // All other keys are handled by default action
      evt.Skip();
  }
}

/*!
 * IsSameAsDefault is checking if actual row is havin the same content as a default constructed row - no change performed.
 */

bool pwFiltersGrid::IsSameAsDefault(int row)
{
  st_FilterRow emptyRow;
  
  ASSERT(m_currentFilter && row < static_cast<int>(m_currentFilter->size()));
  return (*m_currentFilter)[row] == emptyRow;
}

/*!
 * ClearIfEmpty remove single line if initally the grid had been empty and the content is the unchanged.
 */

void pwFiltersGrid::ClearIfEmpty()
{
  if(m_initialyEmpty && (GetNumRows() == 1) && IsSameAsDefault(0)) {
    m_currentFilter->clear(); // Remove entry that had been entered in Init()
    (*m_currentCounter) = 0;
  }
}

/*!
 * UpdateMatchType set matching type (field mtype) defpending from actual field type.
 */

void pwFiltersGrid::UpdateMatchType(int row)
{
  FieldType ft = RowFieldType(row);
  PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
  
  if(ft == FT_INVALID)
    return;
  
  // Now get associated match type
  switch (m_filtertype) {
    case DFTYPE_MAIN:
      switch (ft) {
        case FT_GROUPTITLE:
        case FT_TITLE:
          mt = PWSMatch::MT_STRING;
          break;
        case FT_PASSWORD:
          mt = PWSMatch::MT_PASSWORD;
          break;
        case FT_PASSWORDLEN:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_GROUP:
        case FT_USER:
        case FT_NOTES:
        case FT_URL:
        case FT_AUTOTYPE:
        case FT_RUNCMD:
        case FT_EMAIL:
        case FT_SYMBOLS:
        case FT_POLICYNAME:
          mt = PWSMatch::MT_STRING;
          break;
        case FT_DCA:
        case FT_SHIFTDCA:
          mt = PWSMatch::MT_DCA;
          break;
        case FT_CTIME:
        case FT_PMTIME:
        case FT_ATIME:
        case FT_XTIME:
        case FT_RMTIME:
          mt = PWSMatch::MT_DATE;
          break;
        case FT_PWHIST:
          mt = PWSMatch::MT_PWHIST;
          break;
        case FT_POLICY:
          mt = PWSMatch::MT_POLICY;
          break;
        case FT_XTIME_INT:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_PROTECTED:
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_KBSHORTCUT:
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_UNKNOWNFIELDS:
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_ENTRYTYPE:
          mt = PWSMatch::MT_ENTRYTYPE;
          break;
        case FT_ENTRYSTATUS:
          mt = PWSMatch::MT_ENTRYSTATUS;
          break;
        case FT_ENTRYSIZE:
          mt = PWSMatch::MT_ENTRYSIZE;
          break;
        case FT_ATTACHMENT:
          mt = PWSMatch::MT_ATTACHMENT;
          break;
        default:
          ASSERT(0);
      }
      break;

    case DFTYPE_PWHISTORY:
      switch (ft) {
        case HT_PRESENT:
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_ACTIVE:
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_NUM:
        case HT_MAX:
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_CHANGEDATE:
          mt = PWSMatch::MT_DATE;
          break;
        case HT_PASSWORDS:
          mt = PWSMatch::MT_PASSWORD;
          break;
        default:
          ASSERT(0);
      }
      break;

    case DFTYPE_PWPOLICY:
      switch (ft) {
        case PT_PRESENT:
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_EASYVISION:
        case PT_PRONOUNCEABLE:
        case PT_HEXADECIMAL:
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_LENGTH:
        case PT_LOWERCASE:
        case PT_UPPERCASE:
        case PT_DIGITS:
        case PT_SYMBOLS:
          mt = PWSMatch::MT_INTEGER;
          break;
        default:
          ASSERT(0);
      }
      break;
    case DFTYPE_ATTACHMENT:
      switch (ft) {
        case AT_PRESENT:
          mt = PWSMatch::MT_BOOL;
          break;
        case AT_FILENAME:
          mt = PWSMatch::MT_STRING;
          break;
        case AT_TITLE:
        case AT_FILEPATH:
          mt = PWSMatch::MT_STRING;
          break;
        case AT_MEDIATYPE:
          mt = PWSMatch::MT_MEDIATYPE;
          break;
        case AT_CTIME:
        case AT_FILECTIME:
        case AT_FILEMTIME:
        case AT_FILEATIME:
          mt = PWSMatch::MT_DATE;
          break;
        default:
          ASSERT(0);
      }
      break;
    default:
      ASSERT(0);
  }

  // When assignment is changing the match rule must be reset
  if(RowMatchType(row) != mt) {
    SetRowMatchRule(row, PWSMatch::MR_INVALID);
    RefreshCell(row, FLC_CRITERIA_TEXT);
  }
  
  SetRowMatchType(row, mt);
}

/*!
 * GetCriterion calls the related editor dialog to fetch the parameter depending on the matching type
 * @return true, if dialog wasn't canceled
 */

bool pwFiltersGrid::GetCriterion(int row)
{
  FieldType ft = RowFieldType(row);
  PWSMatch::MatchType mt(RowMatchType(row));
  
  if(ft == FT_INVALID) {
    // Do nothing as long field type is not set (also no message dialog)
    return false;
  }
  int rc = wxID_CANCEL;
  switch(mt) {
    case PWSMatch::MT_STRING:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      wxString fstring = towxstring(FilterRow(row).fstring);
      bool fcase = FilterRow(row).fcase;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersStringDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fstring, &fcase);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fstring = tostringx(fstring);
        FilterRow(row).fcase = fcase;
      }
      break;
    }
    case PWSMatch::MT_PASSWORD:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      wxString fstring = towxstring(FilterRow(row).fstring);
      bool fcase = FilterRow(row).fcase;
      int fnum1 = FilterRow(row).fnum1;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersPasswordDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fstring, &fcase, &fnum1);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fstring = tostringx(fstring);
        FilterRow(row).fcase = fcase;
        FilterRow(row).fnum1 = fnum1;
        return true;
      }
      break;
    }
    case PWSMatch::MT_INTEGER:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      int fnum1 = FilterRow(row).fnum1;
      int fnum2 = FilterRow(row).fnum2;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersIntegerDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fnum1, &fnum2);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fnum1 = fnum1;
        FilterRow(row).fnum2 = fnum2;
      }
      break;
    }
    case PWSMatch::MT_DATE:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      int fnum1 = FilterRow(row).fnum1;
      int fnum2 = FilterRow(row).fnum2;
      time_t fdate1 = FilterRow(row).fdate1;
      time_t fdate2 = FilterRow(row).fdate2;
      int fdatetype = FilterRow(row).fdatetype;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersDateDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fdate1, &fdate2, &fnum1, &fnum2, &fdatetype);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fnum1 = fnum1;
        FilterRow(row).fnum2 = fnum2;
        FilterRow(row).fdate1 = fdate1;
        FilterRow(row).fdate2 = fdate2;
        FilterRow(row).fdatetype = fdatetype;
      }
      break;
    }
    case PWSMatch::MT_BOOL:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersBoolDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
      }
      break;
    }
    case PWSMatch::MT_PWHIST:
    {
      if(m_subDialogCalled) break;
      m_subDialogCalled = true;
      bool bAppliedCalled = false;
      st_filters filters(*m_pfilters);
      // Set filter Name to current value
      filters.fname = static_cast<wxTextCtrl *>(GetParent()->FindWindow(ID_FILTERNAME))->GetValue().c_str();
      // Call Dialog
      rc = ShowModalAndGetResult<SetFiltersDlg>(wxGetTopLevelParent(this), &filters, nullptr, &bAppliedCalled, DFTYPE_PWHISTORY, m_filterpool, false, m_psMediaTypes);
      if(rc == wxID_OK) {
        m_pfilters->vHfldata = filters.vHfldata;
        m_pfilters->num_Hactive = filters.num_Hactive;
      }
      m_subDialogCalled = false;
      break;
    }
    case PWSMatch::MT_POLICY:
    {
      if(m_subDialogCalled) break;
      m_subDialogCalled = true;
      bool bAppliedCalled = false;
      st_filters filters(*m_pfilters);
      // Set filter Name to current value
      filters.fname = static_cast<wxTextCtrl *>(GetParent()->FindWindow(ID_FILTERNAME))->GetValue().c_str();
      // Call Dialog
      rc = ShowModalAndGetResult<SetFiltersDlg>(wxGetTopLevelParent(this), &filters, nullptr, &bAppliedCalled, DFTYPE_PWPOLICY, m_filterpool, false, m_psMediaTypes);
      if(rc == wxID_OK) {
        m_pfilters->vPfldata = filters.vPfldata;
        m_pfilters->num_Pactive = filters.num_Pactive;
      }
      m_subDialogCalled = false;
      break;
    }
    case PWSMatch::MT_ENTRYTYPE:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      CItemData::EntryType etype = FilterRow(row).etype;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersTypeDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &etype);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).etype = etype;
      }
      break;
    }
    case PWSMatch::MT_DCA:
    case PWSMatch::MT_SHIFTDCA:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      short fdca = FilterRow(row).fdca;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersDCADlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fdca);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fdca = fdca;
      }
      break;
    }
    case PWSMatch::MT_ENTRYSTATUS:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      CItemData::EntryStatus estatus = FilterRow(row).estatus;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersStatusDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &estatus);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).estatus = estatus;
      }
      break;
    }
    case PWSMatch::MT_ENTRYSIZE:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      int fnum1 = FilterRow(row).fnum1;
      int fnum2 = FilterRow(row).fnum2;
      int funit = FilterRow(row).funit;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersIntegerDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fnum1, &fnum2, &funit);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fnum1 = fnum1;
        FilterRow(row).fnum2 = fnum2;
        FilterRow(row).funit = funit;
      }
      break;
    }
    case PWSMatch::MT_ATTACHMENT:
    {
      if(m_subDialogCalled) break;
      m_subDialogCalled = true;
      bool bAppliedCalled = false;
      st_filters filters(*m_pfilters);
      // Set filter Name to current value
      filters.fname = static_cast<wxTextCtrl *>(GetParent()->FindWindow(ID_FILTERNAME))->GetValue().c_str();
      // Call Dialog
      rc = ShowModalAndGetResult<SetFiltersDlg>(wxGetTopLevelParent(this), &filters, nullptr, &bAppliedCalled, DFTYPE_ATTACHMENT, m_filterpool, false, m_psMediaTypes);
      if(rc == wxID_OK) {
        m_pfilters->vAfldata = filters.vAfldata;
        m_pfilters->num_Aactive = filters.num_Aactive;
      }
      m_subDialogCalled = false;
      break;
    }
    case PWSMatch::MT_MEDIATYPE:
    {
      PWSMatch::MatchRule value = RowMatchRule(row);
      wxString fstring = towxstring(FilterRow(row).fstring);
      bool fcase = FilterRow(row).fcase;
      // Call Dialog
      rc = ShowModalAndGetResult<pwFiltersMediaTypesDlg>(wxGetTopLevelParent(this), RowFieldType(row), &value, &fstring, &fcase, m_psMediaTypes);
      if(rc == wxID_OK) {
        SetRowMatchRule(row, value);
        FilterRow(row).fstring = tostringx(fstring);
        FilterRow(row).fcase = fcase;
      }
      break;
    }
    default:
      break;
  }
  return rc == wxID_OK;
}

/*!
 * GetFilterAndRow Determine filter, row inside filter and type of filter for mapping read only data
 * The read only data are shown in one grid, with intermediate header for each filter area, e.g. like the following example
 *  MAIN 1
 *  MAIN 2
 *   ...
 *    --- Heading History ----
 *  HISTORY 1
 *  HISTORY 2
 *   ...
 *    --- Heading Policy ----
 *  POLICY 1
 *  POLICY 2
 *   ...
 * "filter" of type "flt" (return value) will point to filter vector and "frow" to the related index inside vector.
 */

FilterType pwFiltersGrid::GetFilterAndRow(int row, vFilterRows **filter, int &frow)
{
  int flt = DFTYPE_MAIN;
  vFilterRows *standardFilter[DFTYPE_ATTACHMENT+1] = { nullptr, &m_pfilters->vMfldata, &m_pfilters->vHfldata, &m_pfilters->vPfldata, &m_pfilters->vAfldata };
  
  do {
    wxASSERT(standardFilter[flt]);
    // If row is inside filter group
    if(row < static_cast<int>(standardFilter[flt]->size())) {
      *filter = standardFilter[flt];
      frow = row;
      return static_cast<FilterType>(flt);
    }
    // Skip filter group
    row -= static_cast<int>(standardFilter[flt]->size());
    ++flt;
    // Skip empty filter group
    while(flt <= DFTYPE_ATTACHMENT && ! standardFilter[flt]->size()) ++flt;
    // If still valid group
    if(flt <= DFTYPE_ATTACHMENT) {
      if(row == 0) {
        // Row heading
        *filter = standardFilter[flt];
        frow = -1;
        return static_cast<FilterType>(flt);
      }
      --row; // Skip row heading
    }
  } while(row >= 0 && flt <= DFTYPE_ATTACHMENT);
  wxASSERT(false); // Should never happen
  // No entry found?
  *filter = nullptr;
  frow = 0;
  return DFTYPE_INVALID;
}
