/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwsgridtable.cpp
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

////@begin includes
////@end includes

#include <utility> // for make_pair
#include <limits> //for MAX_INT
#include "PWSgridtable.h"
#include "passwordsafeframe.h"
#include "PWSgrid.h"
#include "../../core/ItemData.h"
#include "../../core/PWScore.h"
#include <wx/tokenzr.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
////@end XPM images

/*!
 * PWSGridTable type definition
 */

IMPLEMENT_CLASS(PWSGridTable, wxGridTableBase)

typedef StringX (CItemData::*ItemDataFuncT)() const;

static struct PWSGridCellDataType {
    CItemData::FieldType ft;
    bool visible;
    int width;
    int position;
} PWSGridCellData[] = {
                        { CItemData::GROUP,     true,     wxDefaultCoord,     0},
                        { CItemData::TITLE,     true,     wxDefaultCoord,     1},
                        { CItemData::USER,      true,     wxDefaultCoord,     2},
                        { CItemData::URL,       true,     wxDefaultCoord,     3},
                        { CItemData::EMAIL,     true,     wxDefaultCoord,     4},
                        { CItemData::AUTOTYPE,  true,     wxDefaultCoord,     5},
                        { CItemData::RUNCMD,    true,     wxDefaultCoord,     6},
                        { CItemData::PROTECTED, true,     wxDefaultCoord,     7},
                        { CItemData::CTIME,     true,     wxDefaultCoord,     8},
                        { CItemData::PMTIME,    true,     wxDefaultCoord,     9},
                        { CItemData::ATIME,     true,     wxDefaultCoord,     10},
                        { CItemData::XTIME,     true,     wxDefaultCoord,     11},
                        { CItemData::XTIME_INT, true,     wxDefaultCoord,     12},
                        { CItemData::RMTIME,    true,     wxDefaultCoord,     13},
                        { CItemData::PASSWORD,  false,    wxDefaultCoord,     14},
                        { CItemData::PWHIST,    true,     wxDefaultCoord,     15},
                        { CItemData::POLICY,    true,     wxDefaultCoord,     16},
                        { CItemData::DCA,       true,     wxDefaultCoord,     17},
                      };

/*!
 * PWSGridTable constructor
 */

PWSGridTable::PWSGridTable(PWSGrid* pwsgrid) : m_pwsgrid(pwsgrid)
{
  //PWSGridTable could be created many times, but the above table should be initialized
  //only once to avoid losing the changes made during a session
  static bool initialized = false;
  if (!initialized) {
    RestoreSettings();
    initialized = true;
  }
}

/*!
 * PWSGridTable destructor
 */

PWSGridTable::~PWSGridTable()
{
}

/*!
 * wxGridTableBase override implementations
 */

int PWSGridTable::GetNumberRows()
{    
  const size_t N = m_pwsgrid->GetNumItems();
  assert(N <= size_t(std::numeric_limits<int>::max()));
  return int(N);
}

int PWSGridTable::GetNumberCols()
{    
  return NumberOf(PWSGridCellData);
}

bool PWSGridTable::IsEmptyCell(int row, int col)
{
  const wxString val = GetValue(row, col);

  return val == wxEmptyString || val.empty() || val.IsSameAs(wxT("Unknown"));
}

wxString PWSGridTable::GetColLabelValue(int col)
{    
  return (size_t(col) < NumberOf(PWSGridCellData)) ?
    towxstring(CItemData::FieldName(PWSGridCellData[col].ft)) : wxString();
}

wxString PWSGridTable::GetValue(int row, int col)
{
  if (size_t(row) < m_pwsgrid->GetNumItems() &&
      size_t(col) < NumberOf(PWSGridCellData)) {
    const CItemData *pItem = m_pwsgrid->GetItem(row);
    if (pItem != nullptr) {
      if (PWSGridCellData[col].ft != CItemData::POLICY) {
        return towxstring(pItem->GetFieldValue(PWSGridCellData[col].ft));
      } else {
        PWPolicy pwp;
        pItem->GetPWPolicy(pwp);
        return towxstring(pwp.GetDisplayString());
      }
    }
  }
  return wxEmptyString;
}

void PWSGridTable::SetValue(int /*row*/, int /*col*/, const wxString& /*value*/)
{
  //I think it comes here only if the grid is editable
}

void PWSGridTable::Clear()
{
  m_pwsgrid->DeleteAllItems();
}

//overridden
void PWSGridTable::SetView(wxGrid* newGrid)
{
  wxGrid* oldGrid = GetView();
  wxGridTableBase::SetView(newGrid);
  if (newGrid) {
    //A new gridtable is being installed.  Update the grid with our settings
    for (size_t idx = 0; idx < WXSIZEOF(PWSGridCellData); ++idx) {

#if wxCHECK_VERSION(2, 9, 1)
      if (PWSGridCellData[idx].visible)
        newGrid->ShowCol(idx);
      else
        newGrid->HideCol(idx);
#endif

      //calling SetColSize, SetColPos would make them visible, so don't call them
      //unless they are really visible
      if (PWSGridCellData[idx].visible) {
        if (PWSGridCellData[idx].width != wxDefaultCoord)
          newGrid->SetColSize(idx, PWSGridCellData[idx].width);
          
        newGrid->SetColPos(idx, PWSGridCellData[idx].position);
      }
    }
  }
  else {
    wxCHECK_RET(oldGrid, wxT("Both old and new grid views are nullptr"));
    //This gridtable is about to be deleted.  Save current settings
    for (size_t idx = 0; idx < WXSIZEOF(PWSGridCellData); ++idx) {

      bool visible = true;

#if wxCHECK_VERSION(2, 9, 1)
      visible = PWSGridCellData[idx].visible = oldGrid->IsColShown(idx);
#endif

      if (visible) {
        PWSGridCellData[idx].width = oldGrid->GetColSize(idx);
        PWSGridCellData[idx].position = oldGrid->GetColPos(idx);
      }
    }
  }
}

bool PWSGridTable::DeleteRows(size_t pos, size_t numRows)
{
  size_t curNumRows = m_pwsgrid->GetNumItems();
  
  if (pos >= curNumRows) {
    wxFAIL_MSG( wxString(wxT("PWSGridTable::DeleteRows(")) << "pos= " << pos << ", numRows= " << numRows << ") call is invalid\nPos value is invalid for present table with " << curNumRows << " rows");
    return false;
  }

  if (numRows > curNumRows - pos)
    numRows = curNumRows - pos;

  if (GetView()) {
    //This will actually remove the item from grid display
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           reinterpret_cast<int &>(pos),
                           reinterpret_cast<int &>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
    
  return true;
}

bool PWSGridTable::AppendRows(size_t numRows/*=1*/)
{
  if (GetView()) {
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                           reinterpret_cast<int &>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  return true;
}

bool PWSGridTable::InsertRows(size_t pos/*=0*/, size_t numRows/*=1*/)
{
  if (GetView()) {
    wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                           reinterpret_cast<int &>(pos),
                           reinterpret_cast<int &>(numRows));
    GetView()->ProcessTableMessage(msg);
  }
  return true;
}

//static
int PWSGridTable::GetColumnFieldType(int colID)
{
  wxCHECK_MSG(colID >= 0 && size_t(colID) < WXSIZEOF(PWSGridCellData), CItemData::END,
                wxT("column ID is greater than the number of columns in PWSGrid"));
  return PWSGridCellData[colID].ft;
}

//static
int PWSGridTable::Field2Column(int fieldType)
{
  for(int n = 0; n < int(WXSIZEOF(PWSGridCellData)); ++n) {
    if (PWSGridCellData[n].ft == fieldType)
      return n; //or it might be the position: PWSGridCellData[n].position
  }
  return wxNOT_FOUND;
}

void PWSGridTable::SaveSettings(void) const
{
  wxString colWidths, colShown;
  wxGrid* grid = GetView();
  const int nCols = grid->GetNumberCols();

  for(int idx = 0; idx < nCols; ++idx) {

    const int colID = grid->GetColAt(idx);

#if wxCHECK_VERSION(2, 9, 1)
    if (!grid->IsColShown(colID))
      continue;
#endif

    colShown << GetColumnFieldType(colID) << wxT(',');
    colWidths << grid->GetColSize(colID) << wxT(',');
  }

  if (!colShown.IsEmpty())
    colShown.RemoveLast();

  if (!colWidths.IsEmpty())
    colWidths.RemoveLast();

  //write these, even if colWidth and colShown are empty
  PWSprefs::GetInstance()->SetPref(PWSprefs::ListColumns, tostringx(colShown));
  PWSprefs::GetInstance()->SetPref(PWSprefs::ColumnWidths, tostringx(colWidths));
}

void PWSGridTable::RestoreSettings(void) const
{
  wxString colShown = towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::ListColumns));
  wxString colWidths = towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::ColumnWidths));

  wxArrayString colShownArray = wxStringTokenize(colShown, wxT(" \r\n\t,"), wxTOKEN_STRTOK);
  wxArrayString colWidthArray = wxStringTokenize(colWidths, wxT(" \r\n\t,"), wxTOKEN_STRTOK);
  
  if (colShownArray.Count() != colWidthArray.Count() || colShownArray.Count() == 0)
    return;

  //turn off all the columns first
  for(size_t n = 0; n < WXSIZEOF(PWSGridCellData); ++n) {
    PWSGridCellData[n].visible = false;
  }

  //now turn on the selected columns
  for( size_t idx = 0; idx < colShownArray.Count(); ++idx) {
    const int fieldType = wxAtoi(colShownArray[idx]);
    const int fieldWidth = wxAtoi(colWidthArray[idx]);
    for(size_t n = 0; n < WXSIZEOF(PWSGridCellData); ++n) {
      if (PWSGridCellData[n].ft == fieldType) {
        PWSGridCellData[n].visible = true;
        PWSGridCellData[n].width = fieldWidth;
        PWSGridCellData[n].position = idx;
        break;
      }
    }
  }
}

int PWSGridTable::GetNumHeaderCols()
{
  // XXX Should probably reflect number of columns
  // as selected by user
  return NumberOf(PWSGridCellData);
}
