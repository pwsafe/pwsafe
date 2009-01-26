/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#ifndef __pwsgrid_H
#define __pwsgrid_H
/** \file
* 
*/

#endif /* __pwsgrid_H */

#ifndef _PWSGRID_H_
#define _PWSGRID_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"
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
#define ID_LISTBOX 10060
#define SYMBOL_PWSGRID_STYLE wxHSCROLL|wxVSCROLL
#define SYMBOL_PWSGRID_IDNAME ID_LISTBOX
#define SYMBOL_PWSGRID_SIZE wxDefaultSize
#define SYMBOL_PWSGRID_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * PWSGrid class declaration
 */

class PWSGrid: public wxGrid
{    
  DECLARE_DYNAMIC_CLASS( PWSGrid )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  PWSGrid();
  PWSGrid(wxWindow* parent, wxWindowID id = ID_LISTBOX, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHSCROLL|wxVSCROLL);

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id = ID_LISTBOX, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHSCROLL|wxVSCROLL);

  /// Destructor
  ~PWSGrid();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin PWSGrid event handler declarations

////@end PWSGrid event handler declarations

////@begin PWSGrid member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end PWSGrid member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin PWSGrid member variables
////@end PWSGrid member variables
};

#endif
  // _PWSGRID_H_
