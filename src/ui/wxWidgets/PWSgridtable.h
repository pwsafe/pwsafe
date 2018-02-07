/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _PWSGRIDTABLE_H_
#define _PWSGRIDTABLE_H_

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
class PWSGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
//#define ID_LISTBOX 10060
//#define SYMBOL_PWSGRID_STYLE wxHSCROLL|wxVSCROLL
//#define SYMBOL_PWSGRID_IDNAME ID_LISTBOX
//#define SYMBOL_PWSGRID_SIZE wxDefaultSize
//#define SYMBOL_PWSGRID_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * PWSGridTable class declaration
 */

class PWSGridTable: public wxGridTableBase
{    
  DECLARE_CLASS( PWSGridTable )

  DECLARE_NO_COPY_CLASS(PWSGridTable)
public:
  /// Constructors
  PWSGridTable(PWSGrid* pwsgrid);

  /// Destructor
  ~PWSGridTable();

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
  PWSGrid* m_pwsgrid;
};

#endif
  // _PWSGRIDTABLE_H_
