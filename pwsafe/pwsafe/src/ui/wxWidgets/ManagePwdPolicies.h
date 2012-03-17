/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManagePwdPolicies.h
* 
*/


#ifndef _MANAGEPWDPOLICIES_H_
#define _MANAGEPWDPOLICIES_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"
////@end includes
#include "core/coredefs.h"
#include "core/PWPolicy.h"
#include "core/StringX.h"
#include "core/PWScore.h"

#include <vector>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CMANAGEPASSWORDPOLICIES 10216
#define ID_POLICYLIST 10218
#define ID_EDIT_PP 10220
#define ID_LIST 10222
#define ID_GENERATE_PASSWORD 10225
#define ID_PASSWORD_TXT 10226
#define ID_BITMAPBUTTON 10227
#define ID_POLICYPROPERTIES 10217
#define ID_POLICYENTRIES 10219
#define SYMBOL_CMANAGEPASSWORDPOLICIES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_CMANAGEPASSWORDPOLICIES_TITLE _("Manage Password Policies")
#define SYMBOL_CMANAGEPASSWORDPOLICIES_IDNAME ID_CMANAGEPASSWORDPOLICIES
#define SYMBOL_CMANAGEPASSWORDPOLICIES_SIZE wxSize(400, 300)
#define SYMBOL_CMANAGEPASSWORDPOLICIES_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CManagePasswordPolicies class declaration
 */

class CManagePasswordPolicies: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( CManagePasswordPolicies )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  CManagePasswordPolicies( wxWindow* parent,  PWScore &core,
			   wxWindowID id = SYMBOL_CMANAGEPASSWORDPOLICIES_IDNAME,
			   const wxString& caption = SYMBOL_CMANAGEPASSWORDPOLICIES_TITLE,
			   const wxPoint& pos = SYMBOL_CMANAGEPASSWORDPOLICIES_POSITION,
			   const wxSize& size = SYMBOL_CMANAGEPASSWORDPOLICIES_SIZE,
			   long style = SYMBOL_CMANAGEPASSWORDPOLICIES_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMANAGEPASSWORDPOLICIES_IDNAME, const wxString& caption = SYMBOL_CMANAGEPASSWORDPOLICIES_TITLE, const wxPoint& pos = SYMBOL_CMANAGEPASSWORDPOLICIES_POSITION, const wxSize& size = SYMBOL_CMANAGEPASSWORDPOLICIES_SIZE, long style = SYMBOL_CMANAGEPASSWORDPOLICIES_STYLE );

  /// Destructor
  ~CManagePasswordPolicies();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin CManagePasswordPolicies event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
  void OnNewClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT_PP
  void OnEditPpClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
  void OnDeleteClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LIST
  void OnListClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_UNDO
  void OnUndoClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REDO
  void OnRedoClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERATE_PASSWORD
  void OnGeneratePasswordClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
  void OnCopyPasswordClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );

////@end CManagePasswordPolicies event handler declarations

////@begin CManagePasswordPolicies member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CManagePasswordPolicies member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

  // Overriden virtuals
  virtual bool Show(bool show = true);

////@begin CManagePasswordPolicies member variables
  wxGrid* m_PolicyNames;
  wxTextCtrl* m_passwordCtrl;
  wxStaticText* m_lowerTableDesc;
  wxGrid* m_PolicyDetails;
  wxGrid* m_PolicyEntries;
////@end CManagePasswordPolicies member variables
 private:
  void UpdateNames();
  void UpdateDetails();
  void ShowPolicyDetails();
  void ShowPolicyEntries();
  st_PSWDPolicy GetSelectedPolicy() const;

  PWScore &m_core;
  // History of current changes for Undo/Redo and index to current change
  // that can be undone. Note: if this is less that the size of the vector
  // of saved changes, then there are changes that can be redone.
  std::vector<st_PSWDPolicyChange> m_vchanges;
  int m_iundo_pos;

  PSWDPolicyMap m_MapPSWDPLC;
  st_PSWDPolicy m_st_default_pp;

  GTUSet m_setGTU;

  int m_iSortNamesIndex, m_iSortEntriesIndex;
  bool m_bSortNamesAscending, m_bSortEntriesAscending;

  int m_iSelectedItem;
  bool m_bChanged, m_bViewPolicy, m_bLongPPs, m_bReadOnly;
  
  bool m_bUndoShortcut, m_bRedoShortcut;
  unsigned short int m_siUndoVirtKey, m_siRedoVirtKey;
  unsigned char m_cUndoModifier, m_cRedoModifier;
};

#endif
  // _MANAGEPWDPOLICIES_H_
