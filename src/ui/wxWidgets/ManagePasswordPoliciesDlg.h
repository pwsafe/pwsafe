/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ManagePasswordPoliciesDlg.h
* 
*/

#ifndef _MANAGEPWDPOLICIESDLG_H_
#define _MANAGEPWDPOLICIESDLG_H_

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
#include "core/PolicyManager.h"
#include "QueryCancelDlg.h"
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

#ifndef wxDIALOG_MODAL
#define wxDIALOG_MODAL 0
#endif

////@begin control identifiers
#define ID_MANAGEPASSWORDPOLICIESDLG 10216
#define ID_POLICYLIST 10218
#define ID_EDIT_PP 10220
#define ID_LIST 10222
#define ID_GENERATE_PASSWORD 10225
#define ID_PASSWORD_TXT 10226
#define ID_BITMAPBUTTON 10227
#define ID_POLICYPROPERTIES 10217
#define ID_POLICYENTRIES 10219
#define SYMBOL_MANAGEPASSWORDPOLICIESDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxTAB_TRAVERSAL
#define SYMBOL_MANAGEPASSWORDPOLICIESDLG_TITLE _("Manage Password Policies")
#define SYMBOL_MANAGEPASSWORDPOLICIESDLG_IDNAME ID_MANAGEPASSWORDPOLICIESDLG
#define SYMBOL_MANAGEPASSWORDPOLICIESDLG_SIZE wxSize(400, 300)
#define SYMBOL_MANAGEPASSWORDPOLICIESDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * ManagePasswordPoliciesDlg class declaration
 */

class ManagePasswordPoliciesDlg: public QueryCancelDlg
{
  DECLARE_DYNAMIC_CLASS( ManagePasswordPoliciesDlg )
  DECLARE_EVENT_TABLE()

public:
  /// Creation
  static ManagePasswordPoliciesDlg* Create(wxWindow *parent, PWScore &core, wxWindowID id = SYMBOL_MANAGEPASSWORDPOLICIESDLG_IDNAME, const wxString& caption = SYMBOL_MANAGEPASSWORDPOLICIESDLG_TITLE, const wxPoint& pos = SYMBOL_MANAGEPASSWORDPOLICIESDLG_POSITION, const wxSize& size = SYMBOL_MANAGEPASSWORDPOLICIESDLG_SIZE, long style = SYMBOL_MANAGEPASSWORDPOLICIESDLG_STYLE );

  /// Destructor
  ~ManagePasswordPoliciesDlg() = default;

protected:
  /// Constructors
  ManagePasswordPoliciesDlg(wxWindow *parent, PWScore &core,
         wxWindowID id, const wxString& caption, const wxPoint& pos,
         const wxSize& size,long style);

  /// Creates the controls and sizers
  void CreateControls();

////@begin ManagePasswordPoliciesDlg event handler declarations

  /// wxEVT_GRID_SELECT_CELL event handler for ID_POLICYLIST
  void OnSelectCell( wxGridEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
  void OnNewClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT_PP
  void OnEditClick( wxCommandEvent& event );

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
  void OnCancelClick( wxCommandEvent& event ) override;

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );

////@end ManagePasswordPoliciesDlg event handler declarations

  void OnSize(wxSizeEvent& event);
  
  void OnMaximize(wxMaximizeEvent& event);

////@begin ManagePasswordPoliciesDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end ManagePasswordPoliciesDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

  // Overridden virtuals
  virtual bool Show(bool show = true) override;

  void DoNewClick();
  void DoEditClick();

////@begin ManagePasswordPoliciesDlg member variables
  wxGrid* m_PolicyNames = nullptr;
  wxTextCtrl* m_passwordCtrl = nullptr;
  wxStaticText* m_lowerTableDesc = nullptr;
  wxGrid* m_PolicyDetails = nullptr;
  wxGrid* m_PolicyEntries = nullptr;
////@end ManagePasswordPoliciesDlg member variables
 private:
  void UpdateNames();
  void UpdateDetails();
  void UpdateEntryList();
  void UpdateSelection(const wxString& policyname);
  void UpdateUndoRedoButtons();
  void ShowPolicyDetails();
  void ShowPolicyEntries();
  PWPolicy GetSelectedPolicy() const;
  int GetSelectedRow() const;
  void ResizeGridColumns();

  enum Changes : uint32_t {
    None = 0,
    DefaultPolicy = 1u,
    NamedPolices = 1u << 1,
  };

  uint32_t GetChanges() const;
  bool SyncAndQueryCancel(bool showDialog) override;
  bool IsChanged() const override;

  PWScore &m_core;

  int m_curPolRow;

  int m_iSortNamesIndex, m_iSortEntriesIndex;
  bool m_bSortNamesAscending, m_bSortEntriesAscending;

  bool m_bViewPolicy;
  
  bool m_bShowPolicyEntriesInitially;
  
  bool m_bUndoShortcut, m_bRedoShortcut;
  
  int m_ScrollbarWidth;
  
  std::unique_ptr<PolicyManager> m_PolicyManager;
};

#endif // _MANAGEPWDPOLICIESDLG_H_
