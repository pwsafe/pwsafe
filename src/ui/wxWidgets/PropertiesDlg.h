/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PropertiesDlg.h
* 
*/

#ifndef _PROPERTIESDLG_H_
#define _PROPERTIESDLG_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes
#include "core/PWScore.h"
#include "wxUtilities.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_PROPERTIESDLG 10064
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
#define SYMBOL_PROPERTIESDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE
#else
#define SYMBOL_PROPERTIESDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE
#endif
#define SYMBOL_PROPERTIESDLG_TITLE _("Database Properties")
#define SYMBOL_PROPERTIESDLG_IDNAME ID_PROPERTIESDLG
#define SYMBOL_PROPERTIESDLG_SIZE wxSize(400, 300)
#define SYMBOL_PROPERTIESDLG_POSITION wxDefaultPosition
////@end control identifiers

class PropertiesDlg;

/// The PropertiesModel retrieves the stored database properties
/// and provides functionality for processing these properties.
/// It thus contains the application logic for the PropertiesDlg view.
class PropertiesModel
{
public:
  PropertiesModel(PropertiesDlg &view, PWScore &core);
  ~PropertiesModel() = default;

  /// Retrieves the stored properties from the database,
  /// stores them internally, and uses them to initialize the view.
  void Init();

  /// Instructs the application to save the database properties
  /// if there are any changes.
  void Save();

  /// Checks for changes to all properties.
  bool HasChanges() const { return m_PropertiesDb != m_PropertiesNew; }

  /// Checks for changes to the databse 'name' property, only.
  bool HasDatabaseNameChanged() const { return m_PropertiesDb.db_name != m_PropertiesNew.db_name; }

  /// Checks for changes to the databse 'description' property, only.
  bool HasDatabaseDescriptionChanged() const { return m_PropertiesDb.db_description != m_PropertiesNew.db_description; }

  wxString GetDatabase() const { return towxstring(m_PropertiesNew.database); }
  void SetDatabase(const wxString& value) { m_PropertiesNew.database = tostringx(value); }

  wxString GetDatabaseformat() const { return towxstring(m_PropertiesNew.databaseformat); }
  void SetDatabaseformat(const wxString& value) { m_PropertiesNew.databaseformat = tostringx(value); }

  wxString GetNumOfGroups() const { return towxstring(m_PropertiesNew.numgroups); }
  void SetNumOfGroups(const wxString& value) { m_PropertiesNew.numgroups = tostringx(value); }

  wxString GetNumOfEntries() const { return towxstring(m_PropertiesNew.numentries); }
  void SetNumOfEntries(const wxString& value) { m_PropertiesNew.numentries = tostringx(value); }

  wxString GetWhenLastSaved() const { return towxstring(m_PropertiesNew.whenlastsaved); }
  void SetWhenLastSaved(const wxString& value) { m_PropertiesNew.whenlastsaved = tostringx(value); }

  wxString GetWhoLastSaved() const { return towxstring(m_PropertiesNew.wholastsaved); }
  void SetWhoLastSaved(const wxString& value) { m_PropertiesNew.wholastsaved = tostringx(value); }

  wxString GetWhatLastSaved() const { return towxstring(m_PropertiesNew.whatlastsaved); }
  void SetWhatLastSaved(const wxString& value) { m_PropertiesNew.whatlastsaved = tostringx(value); }

  wxString GetFileUuid() const { return towxstring(m_PropertiesNew.file_uuid); }
  void SetFileUuid(const wxString& value) { m_PropertiesNew.file_uuid = tostringx(value); }

  wxString GetUnknownFields() const { return towxstring(m_PropertiesNew.unknownfields); }
  void SetUnknownFields(const wxString& value) { m_PropertiesNew.unknownfields = tostringx(value); }

  StringX GetDatabaseName() const { return m_PropertiesNew.db_name; }
  void SetDatabaseName(const wxString& value) { m_PropertiesNew.db_name = tostringx(value); }

  StringX GetDatabaseDescription() const { return m_PropertiesNew.db_description; }
  void SetDatabaseDescription(const wxString& value) { m_PropertiesNew.db_description = tostringx(value); }

private:
  /// Represents the currently stored database properties.
  st_DBProperties m_PropertiesDb;

  /// Represents new, resp. modified database properties.
  st_DBProperties m_PropertiesNew;

  /// The application's user interface.
  PropertiesDlg &m_View;

  /// The application's core functionality.
  PWScore &m_Core;
};

/*!
 * PropertiesDlg class declaration
 */

class PropertiesDlg : public wxDialog
{
  DECLARE_CLASS( PropertiesDlg )
  DECLARE_EVENT_TABLE()

public:
  
  static PropertiesDlg* Create(wxWindow *parent, PWScore &core,
              wxWindowID id = SYMBOL_PROPERTIESDLG_IDNAME,
              const wxString& caption = SYMBOL_PROPERTIESDLG_TITLE,
              const wxPoint& pos = SYMBOL_PROPERTIESDLG_POSITION,
              const wxSize& size = SYMBOL_PROPERTIESDLG_SIZE,
              long style = SYMBOL_PROPERTIESDLG_STYLE );
  /// Destructor
  ~PropertiesDlg() { delete m_Model; }

protected:
  /// Constructors
  PropertiesDlg(wxWindow *parent, PWScore &core,
              wxWindowID id, const wxString& caption,
              const wxPoint& pos, const wxSize& size, long style);

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin PropertiesDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick(wxCommandEvent& evt);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_SAVE
  void OnSaveClick(wxCommandEvent& evt);

  /// wxEVT_LEFT_DCLICK event handler for wxID_DBNAME
  void OnDoubleClickNameTextCtrl(wxMouseEvent& evt);

  /// wxEVT_LEFT_DCLICK event handler for wxID_DBDESCRIPTION
  void OnDoubleClickDescriptionTextCtrl(wxMouseEvent& evt);

  /// wxEVT_TEXT event handler for wxID_CHANGE_NAME and wxID_CHANGE_DESCRIPTION
  void OnNameOrDescriptionChanged(wxCommandEvent& evt);

////@end PropertiesDlg event handler declarations
public:
////@begin PropertiesDlg member function declarations

  wxString GetDatabase() const { return m_database ; }
  void SetDatabase(const wxString& value) { m_database = value ; }

  wxString GetDatabaseformat() const { return m_databaseformat ; }
  void SetDatabaseformat(const wxString& value) { m_databaseformat = value ; }

  wxString GetNumOfGroups() const { return m_numgroups ; }
  void SetNumOfGroups(const wxString& value) { m_numgroups = value ; }

  wxString GetNumOfEntries() const { return m_numentries ; }
  void SetNumOfEntries(const wxString& value) { m_numentries = value ; }

  wxString GetNumOfAttachments() const { return m_numattachments ; }
  void SetNumOfAttachments(const wxString& value) { m_numattachments = value ; }

  wxString GetWhenLastSaved() const { return m_whenlastsaved ; }
  void SetWhenLastSaved(const wxString& value) { m_whenlastsaved = value ; }

  wxString GetWhoLastSaved() const { return m_wholastsaved ; }
  void SetWhoLastSaved(const wxString& value) { m_wholastsaved = value ; }

  wxString GetWhatLastSaved() const { return m_whatlastsaved ; }
  void SetWhatLastSaved(const wxString& value) { m_whatlastsaved = value ; }

  wxString GetWhenPwdLastChanged() const { return m_whenpwdlastchanged ; }
  void SetWhenPwdLastChanged(const wxString& value) { m_whenpwdlastchanged = value ; }

  wxString GetFileUuid() const { return m_file_uuid ; }
  void SetFileUuid(const wxString& value) { m_file_uuid = value ; }

  wxString GetUnknownFields() const { return m_unknownfields ; }
  void SetUnknownFields(const wxString& value) { m_unknownfields = value ; }

  void SetDatabaseName(const wxString& value) { m_DbName = value; }
  void SetDatabaseDescription(const wxString& value) { m_DbDescription = value; }

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end PropertiesDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

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

  wxButton *m_saveButton = nullptr;
  wxButton *m_closeButton = nullptr;
  wxTextCtrl *m_dbNameTextCtrl = nullptr;
  wxTextCtrl *m_dbDescriptionTextCtrl = nullptr;

  /// The dialog's functionality.
  PropertiesModel *m_Model = nullptr;

  /// The application's core functionality.
  PWScore &m_core;
};

#endif // _PROPERTIESDLG_H_
