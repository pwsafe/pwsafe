/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "../../corelib/ItemData.h"
#include "../../corelib/PWScore.h"

#define NumberOf(array) ((sizeof array)/sizeof(array[0]))

////@begin XPM images
////@end XPM images


/*!
 * PWSGridTable type definition
 */

IMPLEMENT_CLASS(PWSGridTable, wxGridTableBase)


static inline wxString towx(const StringX& str)
{
  return wxString(str.data(), str.size());
}

typedef StringX (CItemData::*ItemDataFuncT)() const;

struct PWSGridCellDataType {
  const charT* fieldname;
  ItemDataFuncT func;
} PWSGridCellData[] = {
                        {_S("Title"),                     &CItemData::GetTitle},
                        {_S("Username"),                  &CItemData::GetUser},
                        {_S("URL"),                       &CItemData::GetURL},
                        {_S("Creation Time"),             &CItemData::GetCTimeL},
                        {_S("Password Modified"),         &CItemData::GetPMTimeL},
                        {_S("Last Accessed"),             &CItemData::GetATimeL},
                        {_S("Password Expiry Date"),      &CItemData::GetXTimeL},
                        {_S("Last Modified"),             &CItemData::GetRMTimeL},
                      };

/*!
 * PWSGridTable constructor
 */

PWSGridTable::PWSGridTable(PWScore &core, PWSGrid* pwsgrid) : m_pwsgrid(pwsgrid)
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

  return val.empty() || val.IsSameAs(wxString(_S("Unknown")));
}

wxString PWSGridTable::GetColLabelValue(int col)
{    
  return (size_t(col) < NumberOf(PWSGridCellData)) ?
    wxString(PWSGridCellData[col].fieldname) : wxString();
}


wxString PWSGridTable::GetValue(int row, int col)
{
	if (size_t(row) < m_pwsgrid->GetNumItems() &&
      size_t(col) < NumberOf(PWSGridCellData)) {
		const CItemData* item = m_pwsgrid->GetItem(row);
    if (item != NULL) {
			return towx((item->*PWSGridCellData[col].func)());
		}
	}
	return wxString();
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
                 (unsigned int)pos,
                 (unsigned int)numRows,
                 (unsigned int)curNumRows
                 ) );
		return false;
	}

	if (numRows > curNumRows - pos)
		numRows = curNumRows - pos;

	if (numRows >= curNumRows) {
    	m_pwsgrid->DeleteAllItems();
	} else {
		m_pwsgrid->DeleteItems(pos, numRows);
		
		//This will actually remove the item from grid display
		wxGridTableMessage msg(this,
                           wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                           pos,
								numRows);
	}
    
	return true;  
}

