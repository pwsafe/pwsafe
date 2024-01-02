/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SelectAliasDlg.h
* 
*/


#ifndef _SELECTALIASDLG_H_
#define _SELECTALIASDLG_H_


/*!
 * Includes
 */
////@begin includes
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/event.h>

#include "core/core.h"
#include "core/ItemData.h"
#include "core/PWScore.h"

#include "TreeCtrl.h"
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
#define ID_SELECTALIAS 10600
#define ID_ALIASNAME 10601
#define ID_ENTRYTREE 10602
#define ID_VIEW_SELECT 10603
#define ID_EXPANDALL_SELECT 10604
#define ID_COLLAPSEALL_SELECT 10605
#define SYMBOL_SELECTALIAS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SELECTALIAS_TITLE _("Select Alias")
#define SYMBOL_SELECTALIAS_IDNAME ID_SELECTALIAS
#define SYMBOL_SELECTALIAS_SIZE wxSize(600, 400)
#define SYMBOL_SELECTALIAS_POSITION wxDefaultPosition
////@end control identifiers

class SelectTreeCtrl : public TreeCtrlBase
{
  DECLARE_CLASS( SelectTreeCtrl )
  DECLARE_EVENT_TABLE()
  
public:
  /// Constructors
  SelectTreeCtrl(); // Declared, never defined, as we don't support this!
  SelectTreeCtrl(PWScore &core);
  SelectTreeCtrl(wxWindow* parent, PWScore &core, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS);

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS);

  /// Destructor
  ~SelectTreeCtrl();

  /// Initialises member variables
  void Init();
  
  /// wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
  void OnTreectrlSelChanged( wxTreeEvent& event );

  /// wxEVT_TREE_ITEM_MENU event handler for ID_TREECTRL
  void OnContextMenu( wxTreeEvent& evt);

#if wxCHECK_VERSION(3, 1, 1)
  void OnContextMenu(wxContextMenuEvent& event);
#else
  /// wxEVT_RIGHT_DOWN event handler for mouse events
  void OnMouseRightClick(wxMouseEvent& event);
#endif // wxCHECK_VERSION(3, 1, 1)
  
  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_EXPANDALL
  void OnExpandAll(wxCommandEvent& evt);

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_COLLAPSEALL
  void OnCollapseAll(wxCommandEvent& evt);
  
  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_VIEW_SELECT
  void OnViewClick(wxCommandEvent& evt);
  
  void SetCurrentMenuItem(const CItemData *pci) { m_menu_item = pci; };

  void DoViewClick();
private:
  const CItemData* m_menu_item;
};

/*!
 * SelectAliasDlg class declaration
 */

class SelectAliasDlg : public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( SelectAlias )
  DECLARE_EVENT_TABLE()

public:
  static SelectAliasDlg* Create(wxWindow *parent, PWScore *core, CItemData *item, CItemData **pbci,
    wxWindowID id = SYMBOL_SELECTALIAS_IDNAME, const wxString& caption = SYMBOL_SELECTALIAS_TITLE, const wxPoint& pos = SYMBOL_SELECTALIAS_POSITION, const wxSize& size = SYMBOL_SELECTALIAS_SIZE, long style = SYMBOL_SELECTALIAS_STYLE );

  /// Destructor
  ~SelectAliasDlg() = default;
  void UpdateSelChanged(CItemData *pci);
    /// Centralized handling of right click in the grid or the tree view
  void OnContextMenu(const CItemData* item);
protected:
  /// Constructors
  SelectAliasDlg() = default;
  SelectAliasDlg(wxWindow *parent, PWScore *core, CItemData *item, CItemData **pbci,
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style);

  /// Creates the controls and sizers
  void CreateControls();

////@begin SelectAliasDlg event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REMOVE
  void OnRemoveClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCancelClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
  void OnHelpClick( wxCommandEvent& event );

////@end SelectAliasDlg event handler declarations

////@begin SelectAliasDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );

  /// Should we show tooltips?
  static bool ShowToolTips();
////@end SelectAliasDlg member function declarations

////@begin SelectAliasDlg member variables
private:
  void InitDialog();
  
  PWScore *m_Core = nullptr;
  CItemData *m_Item = nullptr;
  CItemData **m_BaseItem = nullptr;

  wxString m_AliasName;
  wxTextCtrl *m_AliasBaseTextCtrl = nullptr;

  SelectTreeCtrl *m_Tree = nullptr;
////@end SelectAliasDlg member variables
};

#endif
  // _SELECTALIASDLG_H_
