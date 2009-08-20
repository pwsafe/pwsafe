/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _PWSGRID_H_
#define _PWSGRID_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"
////@end includes
#include "corelib/ItemData.h"
#include "corelib/PWScore.h"
#include "corelib/UUIDGen.h"
#include <map>

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
#define ID_LISTBOX 10060
#define SYMBOL_PWSGRID_STYLE wxHSCROLL|wxVSCROLL
#define SYMBOL_PWSGRID_IDNAME ID_LISTBOX
#define SYMBOL_PWSGRID_SIZE wxDefaultSize
#define SYMBOL_PWSGRID_POSITION wxDefaultPosition
////@end control identifiers

typedef std::map<int, CUUIDGen> RowUUIDMapT;
typedef std::map<CUUIDGen, int, CUUIDGen::ltuuid> UUIDRowMapT;


/*!
 * PWSGrid class declaration
 */

class PWSGrid: public wxGrid
{    
  DECLARE_CLASS( PWSGrid )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  PWSGrid(PWScore &core);
  PWSGrid(wxWindow* parent, PWScore &core,
          wxWindowID id = ID_LISTBOX, const wxPoint& pos = wxDefaultPosition,
          const wxSize& size = wxDefaultSize, long style = wxHSCROLL|wxVSCROLL);

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id = ID_LISTBOX, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHSCROLL|wxVSCROLL);

  /// Destructor
  ~PWSGrid();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();
  
  // Notification from PWSCore when new data is loaded
  void OnPasswordListModified();

  //void Clear();
  void AddItem(const CItemData &item, int row);
  void UpdateItem(const CItemData &item);
  void Remove(const uuid_array_t &uuid);
  size_t GetNumItems() const;
  void DeleteItems(int row, size_t numItems);
  void DeleteAllItems();
  
////@begin PWSGrid event handler declarations

  /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_LISTBOX
  void OnCellRightClick( wxGridEvent& event );

  /// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_LISTBOX
  void OnLeftDClick( wxGridEvent& event );

////@end PWSGrid event handler declarations

////@begin PWSGrid member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end PWSGrid member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

  const CItemData *GetItem(int row) const;
  
////@begin PWSGrid member variables
////@end PWSGrid member variables

 private:
  void DisplayItem(const CItemData &item, int row);
  PWScore &m_core;
  RowUUIDMapT m_row_map;
  UUIDRowMapT m_uuid_map;
};

#endif
  // _PWSGRID_H_
