/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file ComparisonGridTable.h
* 
*/

#ifndef _COMPARISONGRIDTABLE_H_
#define _COMPARISONGRIDTABLE_H_

#include <wx/grid.h>

#include "../../core/DBCompareData.h"

struct SelectionCriteria;
class PWScore;

class ComparisonGridTable: public wxGridTableBase
{
  
  SelectionCriteria*      m_criteria;
  CompareData*            m_compData;
  PWScore*                m_currentCore;
  PWScore*                m_otherCore;
  CItemData::FieldType*   m_colFields;
  
public:
  ComparisonGridTable(SelectionCriteria* criteria, CompareData* data,
                                                   PWScore* current,
                                                   PWScore* other);
  
  //virtual overrides
  int GetNumberRows();
  int GetNumberCols();
  bool IsEmptyCell(int row, int col);
  virtual wxString GetValue(int row, int col);
  virtual void SetValue(int row, int col, const wxString& value);
  
};

#endif
