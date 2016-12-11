/*
 * Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
*
*/

#ifndef _ABOUT_H_
#define _ABOUT_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/hyperlink.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CABOUT 10078
#define wxID_VERSIONSTR 10079
#define ID_CHECKNEW 10081
#define ID_SITEHYPERLINK 10080
#define ID_TEXTCTRL 10082
#define SYMBOL_CABOUT_STYLE wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CABOUT_TITLE _("About Password Safe")
#define SYMBOL_CABOUT_IDNAME ID_CABOUT
#define SYMBOL_CABOUT_SIZE wxSize(400, 300)
#define SYMBOL_CABOUT_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CAbout class declaration
 */

class CAbout: public wxDialog
{
  DECLARE_CLASS( CAbout )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CAbout();
  CAbout( wxWindow* parent, wxWindowID id = SYMBOL_CABOUT_IDNAME, const wxString& caption = SYMBOL_CABOUT_TITLE, const wxPoint& pos = SYMBOL_CABOUT_POSITION, const wxSize& size = SYMBOL_CABOUT_SIZE, long style = SYMBOL_CABOUT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CABOUT_IDNAME, const wxString& caption = SYMBOL_CABOUT_TITLE, const wxPoint& pos = SYMBOL_CABOUT_POSITION, const wxSize& size = SYMBOL_CABOUT_SIZE, long style = SYMBOL_CABOUT_STYLE );

  /// Destructor
  ~CAbout();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  void CheckNewVersion();
////@begin CAbout event handler declarations

  /// event handler for ID_CHECKNEW
#if wxCHECK_VERSION(2,9,2)
  void OnCheckNewClicked(wxCommandEvent& WXUNUSED(event)) { CheckNewVersion(); };
#else
  void OnCheckNewClicked(wxHyperlinkEvent& WXUNUSED(event)) { CheckNewVersion(); };
#endif

  void OnVisitSiteClicked(wxHyperlinkEvent& event);
  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick( wxCommandEvent& event );

////@end CAbout event handler declarations

////@begin CAbout member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CAbout member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CAbout member variables
  wxTextCtrl* m_newVerStatus;
////@end CAbout member variables
};

#endif /* _ABOUT_H_ */
