/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file TreeCtrl.h
* 
*/

#ifndef _TREECTRL_H_
#define _TREECTRL_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/treebase.h>
#include <wx/treectrl.h>
////@end includes

#include "core/ItemData.h"
#include "core/PWScore.h"
#include "core/UIinterface.h"
#include "os/UUID.h"

#include <map>

/*!
 * Forward declarations
 */

////@begin forward declarations
class TreeCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_TREECTRL 10061
#define SYMBOL_PWSTREECTRL_STYLE wxTR_EDIT_LABELS|wxTR_HAS_BUTTONS |wxTR_HIDE_ROOT|wxTR_SINGLE
#define SYMBOL_PWSTREECTRL_IDNAME ID_TREECTRL
#define SYMBOL_PWSTREECTRL_SIZE wxSize(100, 100)
#define SYMBOL_PWSTREECTRL_POSITION wxDefaultPosition
////@end control identifiers

typedef std::map<pws_os::CUUID, wxTreeItemId, std::less<pws_os::CUUID> > UUIDTIMapT;

/*!
 * TreeCtrl class declaration
 */

class TreeCtrl : public wxTreeCtrl, public Observer
{
  DECLARE_CLASS( TreeCtrl )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  TreeCtrl(); // Declared, never defined, as we don't support this!
  TreeCtrl(PWScore &core);
  TreeCtrl(wxWindow* parent, PWScore &core, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS);

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS);

  /// Destructor
  ~TreeCtrl();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  /* Observer Interface Implementation */

  /// Implements Observer::UpdateGUI(UpdateGUICommand::GUI_Action, const pws_os::CUUID&, CItemData::FieldType)
  void UpdateGUI(UpdateGUICommand::GUI_Action ga, const pws_os::CUUID &entry_uuid, CItemData::FieldType ft = CItemData::START) override;

  /// Implements Observer::GUIRefreshEntry(const CItemData&, bool)
  void GUIRefreshEntry(const CItemData &item, bool bAllowFail = false) override;

////@begin TreeCtrl event handler declarations

  /// wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
  void OnTreectrlSelChanged( wxTreeEvent& event );

  /// wxEVT_COMMAND_TREE_ITEM_ACTIVATED event handler for ID_TREECTRL
  void OnTreectrlItemActivated( wxTreeEvent& evt);

  /// wxEVT_TREE_ITEM_MENU event handler for ID_TREECTRL
  void OnContextMenu( wxTreeEvent& evt);

#if wxCHECK_VERSION(3, 1, 1)
  void OnContextMenu(wxContextMenuEvent& event);
#else
  /// wxEVT_RIGHT_DOWN event handler for mouse events
  void OnMouseRightClick(wxMouseEvent& event);
#endif // wxCHECK_VERSION(3, 1, 1)

  /// wxEVT_LEFT_DOWN event handler for mouse events
  void OnMouseLeftClick(wxMouseEvent& event);

////@end TreeCtrl event handler declarations

  void OnGetToolTip( wxTreeEvent& evt); // Added manually

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_ADDGROUP
  void OnAddGroup(wxCommandEvent& evt);

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_RENAME
  void OnRenameGroup(wxCommandEvent& evt);

  void OnEndLabelEdit( wxTreeEvent& evt );

  /// wxEVT_TREE_KEY_DOWN event handler for ID_TREECTRL
  void OnKeyDown(wxTreeEvent& evt);

////@begin TreeCtrl member function declarations
////@end TreeCtrl member function declarations

  void Clear(); // consistent name w/GridCtrl
  void AddItem(const CItemData &item);
  void UpdateItem(const CItemData &item);
  void UpdateItemField(const CItemData &item, CItemData::FieldType ft);
  CItemData *GetItem(const wxTreeItemId &id) const;
  wxTreeItemId Find(const pws_os::CUUID &uuid) const;
  wxTreeItemId Find(const CItemData &item) const;
  wxTreeItemId Find(const wxString &path, wxTreeItemId subtree) const;
  bool Remove(const pws_os::CUUID &uuid); // only remove from tree, not from m_core
  void SelectItem(const pws_os::CUUID& uuid);
  void SortChildrenRecursively(const wxTreeItemId& item);
  wxString GetItemGroup(const wxTreeItemId& item) const;
  bool IsGroupSelected() const;
  bool HasItems() const;
  bool HasSelection() const;
  bool ItemIsGroup(const wxTreeItemId& item) const;
  bool ItemIsGroupOrRoot(const wxTreeItemId& item) const;
  void AddEmptyGroup(const StringX& group) { AddGroup(group); }
  void AddRootItem();
  void SetFilterState(bool state);

  void SetGroupDisplayStateAllExpanded();
  void SetGroupDisplayStateAllCollapsed();
  void SaveGroupDisplayState();
  void RestoreGroupDisplayState();

private:
  void PreferencesChanged();

  virtual int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2) override;
  bool ExistsInTree(wxTreeItemId node, const StringX &s, wxTreeItemId &si) const;
  wxTreeItemId AddGroup(const StringX &group);
  wxString ItemDisplayString(const CItemData &item) const;
  wxString GetPath(const wxTreeItemId &node) const;
  void SetItemImage(const wxTreeItemId &node, const CItemData &item);
  void FinishAddingGroup(wxTreeEvent& evt, wxTreeItemId groupItem);
  void FinishRenamingGroup(wxTreeEvent& evt, wxTreeItemId groupItem, const wxString& oldPath);

  std::vector<bool> GetGroupDisplayState();
  void SetGroupDisplayState(const std::vector<bool> &groupstates);

  template<typename GroupItemConsumer>
  void TraverseTree(wxTreeItemId itemId, GroupItemConsumer&& consumer);

////@begin TreeCtrl member variables
////@end TreeCtrl member variables

  PWScore &m_core;
  UUIDTIMapT m_item_map; // given a uuid, find the tree item pronto!
};

#endif // _TREECTRL_H_
