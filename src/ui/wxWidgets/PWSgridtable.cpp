/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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


////@begin XPM images
////@end XPM images


/*!
 * PWSGridTable type definition
 */

IMPLEMENT_CLASS(PWSGridTable, wxGridTableBase)


typedef StringX (CItemData::*ItemDataFuncT)() const;

struct PWSGridCellDataType {
  CItemData::FieldType ft;
  ItemDataFuncT func;
} PWSGridCellData[] = {
                        { CItemData::GROUP,                 &CItemData::GetGroup},
                        { CItemData::TITLE,                 &CItemData::GetTitle},
                        { CItemData::USER,                  &CItemData::GetUser},
                        { CItemData::URL,                   &CItemData::GetURL},
                        { CItemData::EMAIL,                 &CItemData::GetEmail},
                        { CItemData::AUTOTYPE,              &CItemData::GetAutoType},
                        { CItemData::RUNCMD,                &CItemData::GetRunCommand},
                        { CItemData::PROTECTED,             &CItemData::GetProtected},
                        { CItemData::CTIME,                 &CItemData::GetCTimeL},
                        { CItemData::PMTIME,                &CItemData::GetPMTimeL},
                        { CItemData::ATIME,                 &CItemData::GetATimeL},
                        { CItemData::XTIME,                 &CItemData::GetXTimeL},
                        { CItemData::XTIME_INT,             &CItemData::GetXTimeInt},
                        { CItemData::RMTIME,                &CItemData::GetRMTimeL},
//                        { CItemData::PASSWORD,              &CItemData::GetPassword},
                        { CItemData::PWHIST,                &CItemData::GetPWHistory},
                        { CItemData::POLICY,              &CItemData::GetPWPolicy},
                        { CItemData::DCA,                   &CItemData::GetDCA},
                      };

/*!
 * PWSGridTable constructor
 */

PWSGridTable::PWSGridTable(PWSGrid* pwsgrid) : m_pwsgrid(pwsgrid)
{
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
		const CItemData* item = m_pwsgrid->GetItem(row);
    if (item != NULL) {
			return towxstring((item->*PWSGridCellData[col].func)());
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

bool PWSGridTable::DeleteRows(size_t pos, size_t numRows)
{
	size_t curNumRows = m_pwsgrid->GetNumItems();
  
	if (pos >= curNumRows) {
		wxFAIL_MSG( wxString::Format 
                (
                 wxT("Called PWSGridTable::DeleteRows(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu rows"),
                 static_cast<unsigned int>(pos),
                 static_cast<unsigned int>(numRows),
                 static_cast<unsigned int>(curNumRows)
                 ) );
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
