/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file GridTable.h
* 
*/

#ifndef _GRIDTABLE_H_
#define _GRIDTABLE_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/grid.h>
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class GridCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
//#define ID_LISTBOX 10060
//#define SYMBOL_GRIDTABLE_STYLE wxHSCROLL|wxVSCROLL
//#define SYMBOL_GRIDTABLE_IDNAME ID_LISTBOX
//#define SYMBOL_GRIDTABLE_SIZE wxDefaultSize
//#define SYMBOL_GRIDTABLE_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * GridTable class declaration
 */

class GridTable : public wxGridTableBase
{
  DECLARE_CLASS( GridTable )

  DECLARE_NO_COPY_CLASS(GridTable)
public:
  /// Constructors
  GridTable(GridCtrl* GridCtrl);

  /// Destructor
  ~GridTable();

  /// overrides from wxGridTableBase
  virtual int GetNumberRows();
  virtual int GetNumberCols();
  virtual bool IsEmptyCell(int row, int col);
  virtual wxString GetValue(int row, int col);
  //override this to suppress the row numbering in the grid
  //virtual wxString GetRowLabelValue(int row);
  virtual wxString GetColLabelValue(int col);
  virtual void SetValue(int row, int col, const wxString& value);
  virtual bool DeleteRows(size_t pos, size_t numRows);
  virtual bool InsertRows(size_t pos, size_t numRows);
  virtual bool AppendRows(size_t numRows = 1);
  
  ///optional overrides
  virtual void Clear();
  virtual void SetView(wxGrid* grid);

  static int GetColumnFieldType(int colID);
  static int Field2Column(int fieldType);
  static int GetNumHeaderCols();
  void SaveSettings(void) const;
  void RestoreSettings(void) const;

private:
  GridCtrl* m_pwsgrid;
};

#endif // _GRIDTABLE_H_
