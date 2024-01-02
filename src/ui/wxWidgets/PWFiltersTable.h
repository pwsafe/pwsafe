/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersTable.h
* 
*/

#ifndef _PWFILTERSTABLE_H_
#define _PWFILTERSTABLE_H_

/*!
 * Includes
 */

////@begin includes
#include <map>
#include <wx/string.h>
#include <wx/grid.h>
#include "core/PWSFilters.h"
#include "PWFiltersGrid.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class pwFiltersGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

#define NOCRITERIADEFINED _("No criteria defined")

/*!
 * pwFiltersTable class declaration
 */

class pwFiltersTable : public wxGridTableBase
{
  DECLARE_CLASS( pwFiltersTable )

  DECLARE_NO_COPY_CLASS(pwFiltersTable)
public:
  /// Constructors
  pwFiltersTable(pwFiltersGrid* pGrid);

  /// Destructor
  ~pwFiltersTable();

  /// overrides from wxGridTableBase
  virtual int GetNumberRows();
  virtual int GetNumberCols();
  virtual bool IsEmptyCell(int row, int col);
  virtual wxString GetValue(int row, int col);
  // virtual wxString GetRowLabelValue(int row);
  virtual wxString GetColLabelValue(int col);
  virtual void SetValue(int row, int col, const wxString& value);
  virtual bool DeleteRows(size_t pos = 0, size_t numRows = 1);
  virtual bool InsertRows(size_t pos = 0, size_t numRows = 1);
  virtual bool AppendRows(size_t numRows = 1);
  
  /// Supporting functions to handle label values
  static wxString GetColLabelString(int col);
  static wxSize GetCriteriaMinSize(wxClientDC &dc); // Minimum size of default criteria strings
  
  ///optional overrides
  virtual void Clear();
          void RefreshRowsCount(); // Update rows content count, after filter has Cleared (Clear() called) and filled with new content
  
  virtual wxString GetTypeName(int row, int col );
  virtual bool CanGetValueAs(int row, int col, const wxString& typeName);
  virtual bool CanSetValueAs(int row, int col, const wxString& typeName);

  virtual long GetValueAsLong(int row, int col); // typeName is wxGRID_VALUE_NUMBER
  virtual bool GetValueAsBool(int row, int col); // typeName is wxGRID_VALUE_BOOL

  virtual void SetValueAsLong(int row, int col, long value); // typeName is wxGRID_VALUE_NUMBER
  virtual void SetValueAsBool(int row, int col, bool value); // typeName is wxGRID_VALUE_BOOL
  
private:
  // Supporting functions to handled read only and read write grid differently
  bool IsReadOnly() { wxASSERT(m_pwsgrid != nullptr); return m_pwsgrid->IsReadOnly(); };
  bool IsReadWrite() { wxASSERT(m_pwsgrid != nullptr); return m_pwsgrid->IsReadWrite(); };
  
  pwFiltersGrid* m_pwsgrid;
};

#endif // _PWFILTERSTABLE_H_
