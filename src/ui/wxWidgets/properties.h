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

#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes
#include "core/PWScore.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CPROPERTIES 10064
#define wxID_DATABASE 10065
#define wxID_DATABASEFORMAT 10066
#define wxID_NUMGROUPS 10067
#define wxID_NUMENTRIES 10068
#define wxID_NUMATTACHMENTS 10300
#define wxID_WHOLASTSAVED 10069
#define wxID_WHENLASTSAVED 10070
#define wxID_WHATLASTSAVED 10071
#define wxID_PWDLASTCHANGED 10301
#define wxID_FILEUUID 10072
#define wxID_UNKNOWFIELDS 10073
#define wxID_DBNAME 10302
#define wxID_DBDESCRIPTION 10303
#define wxID_CHANGE_NAME 10304
#define wxID_CHANGE_DESCRIPTION 10305
#if WXWIN_COMPATIBILITY_2_6
#define SYMBOL_CPROPERTIES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE
#else
#define SYMBOL_CPROPERTIES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE
#endif
#define SYMBOL_CPROPERTIES_TITLE _("Properties")
#define SYMBOL_CPROPERTIES_IDNAME ID_CPROPERTIES
#define SYMBOL_CPROPERTIES_SIZE wxSize(400, 300)
#define SYMBOL_CPROPERTIES_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CProperties class declaration
 */

class CProperties: public wxDialog
{
  DECLARE_CLASS( CProperties )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CProperties(wxWindow* parent, const PWScore &core,
              wxWindowID id = SYMBOL_CPROPERTIES_IDNAME,
              const wxString& caption = SYMBOL_CPROPERTIES_TITLE,
              const wxPoint& pos = SYMBOL_CPROPERTIES_POSITION,
              const wxSize& size = SYMBOL_CPROPERTIES_SIZE,
              long style = SYMBOL_CPROPERTIES_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROPERTIES_IDNAME, const wxString& caption = SYMBOL_CPROPERTIES_TITLE, const wxPoint& pos = SYMBOL_CPROPERTIES_POSITION, const wxSize& size = SYMBOL_CPROPERTIES_SIZE, long style = SYMBOL_CPROPERTIES_STYLE );

  /// Destructor
  ~CProperties();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin CProperties event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick(wxCommandEvent& evt);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CHANGE_NAME
  void OnEditName(wxCommandEvent& evt);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CHANGE_DESCRIPTION
  void OnEditDescription(wxCommandEvent& evt);

////@end CProperties event handler declarations

////@begin CProperties member function declarations

  wxString GetDatabase() const { return m_database ; }
  void SetDatabase(wxString value) { m_database = value ; }

  wxString GetDatabaseformat() const { return m_databaseformat ; }
  void SetDatabaseformat(wxString value) { m_databaseformat = value ; }

  wxString GetNumgroups() const { return m_numgroups ; }
  void SetNumgroups(wxString value) { m_numgroups = value ; }

  wxString GetNumentries() const { return m_numentries ; }
  void SetNumentries(wxString value) { m_numentries = value ; }

  wxString GetWhenlastsaved() const { return m_whenlastsaved ; }
  void SetWhenlastsaved(wxString value) { m_whenlastsaved = value ; }

  wxString GetWholastsaved() const { return m_wholastsaved ; }
  void SetWholastsaved(wxString value) { m_wholastsaved = value ; }

  wxString GetWhatlastsaved() const { return m_whatlastsaved ; }
  void SetWhatlastsaved(wxString value) { m_whatlastsaved = value ; }

  wxString GetFileUuid() const { return m_file_uuid ; }
  void SetFileUuid(wxString value) { m_file_uuid = value ; }

  wxString GetUnknownfields() const { return m_unknownfields ; }
  void SetUnknownfields(wxString value) { m_unknownfields = value ; }

  StringX GetNewDbName() const { return m_NewDbName; }
  StringX GetNewDbDescription() const { return m_NewDbDescription; }

  bool HasDbNameChanged() const { return m_NewDbName != m_core.GetHeader().m_DB_Name; }
  bool HasDbDescriptionChanged() const { return m_NewDbDescription != m_core.GetHeader().m_DB_Description; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CProperties member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin CProperties member variables
private:
  wxString m_database;
  wxString m_databaseformat;
  wxString m_numgroups;
  wxString m_numentries;
  wxString m_numattachments;
  wxString m_whenlastsaved;
  wxString m_wholastsaved;
  wxString m_whatlastsaved;
  wxString m_whenpwdlastchanged;
  wxString m_file_uuid;
  wxString m_unknownfields;
  wxString m_DbName;
  wxString m_DbDescription;
////@end CProperties member variables
  StringX m_NewDbName;
  StringX m_NewDbDescription;
  const PWScore &m_core;
};

#endif
  // _PROPERTIES_H_
