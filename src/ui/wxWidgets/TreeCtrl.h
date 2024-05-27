/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include <wx/aui/auibar.h>
#include <wx/treebase.h>
#include <wx/treectrl.h>
#include <wx/dragimag.h>
#include <wx/dnd.h>

#include "core/ItemData.h"
#include "core/PWScore.h"
#include "core/UIinterface.h"
#include "os/UUID.h"

#include <map>

#include "DnDSupport.h"
////@end includes
///
///
/*!
 * Forward declarations
 */

////@begin forward declarations
class TreeCtrl;
class SelectTreeCtrl;
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

class TreeCtrl;

typedef void (TreeCtrl:: *TreeCtrlMemberFncPtr)(void);

class TreeCtrlTimer: public wxTimer
{
public:
  enum {
    // start scrolling 1/4 second (if the mouse hasn't been clicked)
    DELAY_SCROLLING = 250,
    // start Collapse or Expand after 1.5 second (if the mouse hasn't been clicked/moved)
    DELAY_COLLAPSE = 1500
  };
  
  TreeCtrlTimer(TreeCtrl *aowner, TreeCtrlMemberFncPtr acallback, int atime, bool aonetime = true) : m_owner(aowner), m_callback(acallback), m_time(atime), m_one_time(aonetime) { }
  TreeCtrlTimer(TreeCtrlMemberFncPtr acallback, int atime, bool aonetime = true) : m_owner(nullptr), m_callback(acallback), m_time(atime), m_one_time(aonetime) { }
  
  void setOwner(TreeCtrl *owner) { m_owner = owner; };
  bool Start() { return wxTimer::Start( m_time, m_one_time ); };

  virtual void Notify();

private:
  // Avoid -Wreorder warning when using the same order of variable declaration and order in constructor
  TreeCtrl *m_owner;
  TreeCtrlMemberFncPtr m_callback;
  int m_time;
  bool m_one_time;

  wxDECLARE_NO_COPY_CLASS(TreeCtrlTimer);
};

class TreeCtrlBase : public wxTreeCtrl
{
  friend TreeCtrl;
  friend SelectTreeCtrl;
private:
  enum class TreeSortType { GROUP, NAME, DATE };
public:
  
  TreeCtrlBase(PWScore &core) : m_core(core) {
    Init();
  };
  ~TreeCtrlBase() {
    m_item_map.clear();
  };
  
  /// Initialises member variables
  void Init();
  
  /// Creates the controls and sizers
  void CreateControls();
  
  void AddRootItem();
  
  void Clear(); // consistent name w/GridCtrl
  void AddItem(const CItemData &item);
  StringX GroupNameOfItem(const CItemData &item);
  
  CItemData *GetItem(const wxTreeItemId &id) const;
  
  wxTreeItemId Find(const pws_os::CUUID &uuid) const;
  
  bool HasSelection() const;
  bool ItemIsGroup(const wxTreeItemId& item) const;
  bool ItemIsGroupOrRoot(const wxTreeItemId& item) const;
  bool IsGroupSelected() const;
  bool HasItems() const;
  
  void SelectItem(const pws_os::CUUID& uuid);
  
  void SortChildrenRecursively(const wxTreeItemId& item);
  
  void SetShowGroup(bool v) { m_show_group = v; }
  bool IsShowGroup() const { return m_show_group; }
  
  void SetSorting(TreeSortType &v) { m_sort = v; }
  void SetSortingGroup() { m_sort = TreeSortType::GROUP; }
  void SetSortingName() { m_sort = TreeSortType::NAME; }
  void SetSortingDate() { m_sort = TreeSortType::DATE; }
  
  bool IsSortingGroup() const { return m_sort == TreeSortType::GROUP; }
  bool IsSortingName() const { return m_sort == TreeSortType::NAME; }
  bool IsSortingDate() const { return m_sort == TreeSortType::DATE; }
  
private:
  bool ExistsInTree(wxTreeItemId node, const StringX &s, wxTreeItemId &si) const;
  
  wxTreeItemId AddGroup(const StringX &group);
  wxString ItemDisplayString(const CItemData &item) const;
  wxString GetPath(const wxTreeItemId &node) const;
  
  void SetItemImage(const wxTreeItemId &node, const CItemData &item);
  void setNodeAsNotEmpty(const wxTreeItemId item);
  void setNodeAsEmptyIfNeeded(const wxTreeItemId item);
  
  TreeSortType m_sort;
  bool m_show_group;
  PWScore &m_core;
  UUIDTIMapT m_item_map; // given a uuid, find the tree item pronto!
};

/*!
 * TreeCtrl class declaration
 */

class TreeCtrl : public TreeCtrlBase, public Observer
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
  
  /// wxEVT_MOTION event handler for mouse events
  void OnMouseMove(wxMouseEvent& event);

  void OnGetToolTip( wxTreeEvent& evt); // Added manually

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_ADDGROUP
  void OnAddGroup(wxCommandEvent& evt);

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_RENAME
  void OnRenameGroup(wxCommandEvent& evt);
  
  /// EVT_TREE_START_LABEL_EDIT event handler for ID_TREECTRL
  void OnStartLabelEdit( wxTreeEvent& evt );

  /// EVT_TREE_END_LABEL_EDIT event handler for ID_TREECTRL and ID_TREECTRL_1
  void OnEndLabelEdit( wxTreeEvent& evt );

  /// wxEVT_TREE_KEY_DOWN event handler for ID_TREECTRL
  void OnKeyDown(wxTreeEvent& evt);

  /// wxEVT_TREE_BEGIN_DRAG event handler for ID_TREECTRL
  void OnBeginDrag(wxTreeEvent& evt);

  /// wxEVT_TREE_END_DRAG event handler for ID_TREECTRL
  void OnEndDrag(wxTreeEvent& evt);
  
  /// when draging is started
  void OnDrag(wxAuiToolBarEvent& event);

////@end TreeCtrl event handler declarations

////@begin TreeCtrl member function declarations
////@end TreeCtrl member function declarations

  void UpdateItem(const CItemData &item);
  void UpdateItemField(const CItemData &item, CItemData::FieldType ft);
  wxTreeItemId Find(const CItemData &item) const;
  wxTreeItemId Find(const wxString &path, wxTreeItemId subtree) const;
  bool Remove(const pws_os::CUUID &uuid); // only remove from tree, not from m_core
  wxString GetItemGroup(const wxTreeItemId& item) const;
  void AddEmptyGroup(const StringX& group) { AddGroup(group); }
  void SetFilterState(bool state);

  void SetGroupDisplayStateAllExpanded();
  void SetGroupDisplayStateAllCollapsed();
  void SaveGroupDisplayState();
  void RestoreGroupDisplayState();
  
  void SetFilterActive(bool v) { m_bFilterActive = v; }
  
  void CheckScrollList();
  void CheckCollapseEntry();
  
  void SetDndEntry(wxTreeItemId item) { m_last_dnd_item = item; }
  bool IsReadOnly() { return m_core.IsReadOnly(); }
  wxDragResult OnDrop(wxCoord x, wxCoord y, wxMemoryBuffer *inDDmem);

private:
  void PreferencesChanged();

  virtual int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2) override;
  void FinishAddingGroup(wxTreeEvent& evt, wxTreeItemId groupItem);
  void FinishRenamingGroup(wxTreeEvent& evt, wxTreeItemId groupItem, const wxString& oldPath);
  CItemData CreateNewItemAsCopy(const CItemData *dataSrc, StringX sxNewPath, bool checkName, bool newEntry = false);
  void ExtendCommandCopyGroup(MultiCommands* pmCmd, wxTreeItemId itemSrc, StringX sxNewPath, bool checkName);
  MultiCommands* CreateCommandRenamingGroup(StringX sxNewPath, StringX sxOldPath);
  MultiCommands* CreateCommandCopyGroup(wxTreeItemId itemSrc, StringX sxNewPath, StringX sxOldPath, bool checkName);
  void ExecuteMultiCommands(MultiCommands* commands);
  bool IsDescendant(const wxTreeItemId itemDst, const wxTreeItemId itemSrc);
  void markDragItem(const wxTreeItemId itemSrc, bool markIt = true);
  void resetDragItems(bool initSize = false);
  void resetScrolling();

  std::vector<bool> GetGroupDisplayState();
  void SetGroupDisplayState(const std::vector<bool> &groupstates);

  template<typename GroupItemConsumer>
  void TraverseTree(wxTreeItemId itemId, GroupItemConsumer&& consumer);
  
  void CollectDnDData(wxMemoryBuffer &outDDmem, wxString &fileName);
  void GetGroupEntriesData(DnDObList &dnd_oblist, wxTreeItemId item);
  void GetEntryData(DnDObList &dnd_oblist, CItemData *pci);
  bool ProcessDnDData(StringX &sxDropPath, wxMemoryBuffer *inDDmem);
  void AddDnDEntries(MultiCommands *pmCmd, DnDObList &dnd_oblist, StringX &sxDropPath);

  void EditTreeLabel(wxTreeCtrl* tree, const wxTreeItemId& id);

////@begin TreeCtrl member variables
  wxTreeItemId m_drag_item;
  wxColour m_drag_text_colour;
  wxColour m_drag_background_colour;
  wxTreeItemId m_last_dnd_item;
  bool m_run_dnd;

  TreeCtrlTimer m_collapse_timer;
  wxTreeItemId m_last_mice_item_in_drag_and_drop;
  
  int m_lower_scroll_limit, m_upper_scroll_limit;
  bool m_had_been_out;
  
  TreeCtrlTimer m_scroll_timer;
  
  wxDragImage *m_drag_image;
  
  long m_style;
  
  bool m_bFilterActive;
  ////@end TreeCtrl member variables
};

#endif // _TREECTRL_H_
