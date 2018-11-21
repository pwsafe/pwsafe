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

#ifndef _PWSGRID_H_
#define _PWSGRID_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/grid.h>
#include <wx/headerctrl.h>
////@end includes

#include "core/ItemData.h"
#include "core/PWScore.h"
#include "os/UUID.h"

#include <functional>
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

typedef std::map<int, pws_os::CUUID> RowUUIDMapT;
typedef std::map<pws_os::CUUID, int, std::less<pws_os::CUUID> > UUIDRowMapT;

/*!
 * PWSGrid class declaration
 */

class PWSGrid: public wxGrid
{
  typedef std::multimap<wxString, const CItemData*, std::greater<wxString> > DescendingSortedMultimap;
  typedef std::multimap<wxString, const CItemData*, std::less<wxString> >    AscendingSortedMultimap;

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

  // Notification from PWScore when new data is loaded
  void OnPasswordListModified();

  void AddItem(const CItemData &item, int row = -1);
  void UpdateItem(const CItemData &item);
  void RefreshRow(int row);
  void RefreshItem(const CItemData &item, int row = -1);
  void RefreshItemRow(const pws_os::CUUID& uuid);
  void RefreshItemField(const pws_os::CUUID& uuid, CItemData::FieldType ft);
  void Remove(const pws_os::CUUID &uuid);
  size_t GetNumItems() const;
  void DeleteItems(int row, size_t numItems);
  void DeleteAllItems();
  void Clear();

////@begin PWSGrid event handler declarations

  /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_LISTBOX
  void OnCellRightClick( wxGridEvent& evt);

  void OnContextMenu(wxContextMenuEvent& evt);

  /// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_LISTBOX
  void OnLeftDClick( wxGridEvent& evt);

  /// wxEVT_GRID_SELECT_CELL event handler for ID_LISTBOX
  void OnSelectCell( wxGridEvent& event );

  void OnDBGUIPrefsChange(wxEvent& evt);

  /// EVT_HEADER_CLICK
  void OnHeaderClick(wxHeaderCtrlEvent& event);

////@end PWSGrid event handler declarations

////@begin PWSGrid member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end PWSGrid member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

  CItemData *GetItem(int row) const;

  void SelectItem(const pws_os::CUUID& uuid);

  int  FindItemRow(const pws_os::CUUID& uu);

  void SaveSettings() const;

  void SetFilterState(bool state);

  void UpdateSorting();

////@begin PWSGrid member variables
////@end PWSGrid member variables

 private:
  void SortByColumn(int column, bool ascending);

  template<typename ItemsCollection>
  void RearrangeItems(ItemsCollection& collection, int column);

  PWScore &m_core;
  RowUUIDMapT m_row_map;
  UUIDRowMapT m_uuid_map;
};

#endif
  // _PWSGRID_H_
