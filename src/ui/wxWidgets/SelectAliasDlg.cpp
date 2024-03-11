/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SelectAliasDlg.cpp
* 
*/

////@begin includes

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/stattext.h>

#include "core/core.h"
#include "core/ItemAtt.h"
#include "core/ItemData.h"
#include "core/PWScore.h"
#include "core/StringX.h"
#include "core/PWSprefs.h"

#include "wxUtilities.h"

#include "SelectAliasDlg.h"
#include "PasswordSafeFrame.h"
#include "AddEditPropSheetDlg.h"

////@end includes

////@begin XPM images
////@end XPM images

#define SELECT_DEFAULT_NUM_ROWS 8  // Number of Rows to be shown b default in selection tree

/*!
 * SelectAliasDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SelectAliasDlg, wxDialog )


/*!
 * SelectAliasDlg event table definition
 */

BEGIN_EVENT_TABLE( SelectAliasDlg, wxDialog )

////@begin SelectAliasDlg event table entries
  EVT_BUTTON( wxID_REMOVE, SelectAliasDlg::OnRemoveClick )
  EVT_BUTTON( wxID_OK, SelectAliasDlg::OnOkClick )
  EVT_BUTTON( wxID_CANCEL, SelectAliasDlg::OnCancelClick )
  EVT_BUTTON( wxID_HELP, SelectAliasDlg::OnHelpClick )
////@end SelectAliasDlg event table entries

END_EVENT_TABLE()



/*!
 * SelectTreeCtrl type definition
 */

IMPLEMENT_CLASS( SelectTreeCtrl, wxTreeCtrl )

/*!
 * SelectTreeCtrl event table definition
 */

BEGIN_EVENT_TABLE( SelectTreeCtrl, wxTreeCtrl )

////@begin SelectTreeCtrl event table entries
  EVT_TREE_SEL_CHANGED( ID_ENTRYTREE, SelectTreeCtrl::OnTreectrlSelChanged )
  EVT_TREE_ITEM_MENU( ID_ENTRYTREE, SelectTreeCtrl::OnContextMenu )
  /*
    In Linux environments context menus appear on Right-Down mouse click.
    Which mouse click type (Right-Down/Right-Up) is the right one on a platform
    is considered by 'EVT_CONTEXT_MENU(TreeCtrl::OnContextMenu)' which doesn't
    work for wxTreeCtrl prior wx version 3.1.1. See also the following URL.
    https://github.com/wxWidgets/wxWidgets/commit/caea08e6b2b9e4843e84c61abc879880a08634b0
  */
#if wxCHECK_VERSION(3, 1, 1)
  EVT_CONTEXT_MENU(SelectTreeCtrl::OnContextMenu)
#else
#ifdef __WINDOWS__
  EVT_RIGHT_UP(SelectTreeCtrl::OnMouseRightClick)
#else
  EVT_RIGHT_DOWN(SelectTreeCtrl::OnMouseRightClick)
#endif
#endif // wxCHECK_VERSION(3, 1, 1)

  EVT_MENU( ID_EXPANDALL_SELECT,   SelectTreeCtrl::OnExpandAll )
  EVT_MENU( ID_COLLAPSEALL_SELECT, SelectTreeCtrl::OnCollapseAll )
  EVT_MENU( ID_VIEW_SELECT,        SelectTreeCtrl::OnViewClick )
////@end SelectTreeCtrl event table entries
END_EVENT_TABLE()

/*!
 * SelectTreeCtrl constructors
 */

SelectTreeCtrl::SelectTreeCtrl(PWScore &core) : TreeCtrlBase(core)
{
  Init();
}

SelectTreeCtrl::SelectTreeCtrl(wxWindow* parent, PWScore &core,
                         wxWindowID id, const wxPoint& pos,
                         const wxSize& size, long style) : TreeCtrlBase(core)
{
  Init();
  Create(parent, id, pos, size, style);
}

/*!
 * SelectTreeCtrl creator
 */

bool SelectTreeCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin SelectTreeCtrl creation
  wxTreeCtrl::Create(parent, id, pos, size, style);
  CreateControls();
////@end SelectTreeCtrl creation
  return true;
}

/*!
 * SelectTreeCtrl destructor
 */

SelectTreeCtrl::~SelectTreeCtrl()
{
////@begin SelectTreeCtrl destruction
////@end SelectTreeCtrl destruction
}

/*!
 * Member initialisation
 */

void SelectTreeCtrl::Init()
{
  TreeCtrlBase::Init();
////@begin SelectTreeCtrl member initialisation
  m_menu_item = nullptr;
////@end SelectTreeCtrl member initialisation
}



/*!
 * wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
 */

void SelectTreeCtrl::OnTreectrlSelChanged( wxTreeEvent& evt )
{
  CItemData *pci = GetItem(evt.GetItem());

  dynamic_cast<SelectAliasDlg*>(GetParent()->GetParent())->UpdateSelChanged(pci);
}


/*!
 * wxEVT_TREE_ITEM_MENU event handler for ID_TREECTRL
 */

void SelectTreeCtrl::OnContextMenu( wxTreeEvent& evt )
{
  dynamic_cast<SelectAliasDlg*>(GetParent()->GetParent())->OnContextMenu(GetItem(evt.GetItem()));
}

#if wxCHECK_VERSION(3, 1, 1)
void SelectTreeCtrl::OnContextMenu(wxContextMenuEvent& event)
#else
void SelectTreeCtrl::OnMouseRightClick(wxMouseEvent& event)
#endif // wxCHECK_VERSION(3, 1, 1)
{
  wxPoint mouseClickPosition = event.GetPosition();
  int positionInfo;

#if wxCHECK_VERSION(3, 1, 1)
  HitTest(wxWindow::ScreenToClient(mouseClickPosition), positionInfo);
#else
  HitTest(mouseClickPosition, positionInfo);
#endif // wxCHECK_VERSION(3, 1, 1)

  if ((positionInfo & wxTREE_HITTEST_NOWHERE) == wxTREE_HITTEST_NOWHERE) {
    auto *parentWindow = dynamic_cast<SelectAliasDlg*>(GetParent()->GetParent());
    wxASSERT(parentWindow != nullptr);
    Unselect();
    parentWindow->OnContextMenu(nullptr);
  }
  else {
    event.Skip();
  }
}


/*!
 * SelectAliasDlg constructors
 */

SelectAliasDlg::SelectAliasDlg(wxWindow *parent, PWScore *core,
                             CItemData *item,
                             CItemData **pbci,
                             wxWindowID id, const wxString& caption,
                             const wxPoint& pos, const wxSize& size,
                             long style ) : m_Core(core),
                                            m_Item(item),
                                            m_BaseItem(pbci)
{
  wxASSERT(!parent || parent->IsTopLevel());

  if(m_Core && m_Item && m_BaseItem) {
    CItemData *pbci = m_Core->GetBaseEntry(m_Item);
    if(pbci) {
      StringX pwd = L"[" +
                  pbci->GetGroup() + L":" +
                  pbci->GetTitle() + L":" +
                  pbci->GetUser()  + L"]";
      m_AliasName = pwd.c_str();
      *m_BaseItem = pbci;
    }
  }
  ////@begin SelectAliasDlg creation
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end SelectAliasDlg creation
}

SelectAliasDlg* SelectAliasDlg::Create(wxWindow *parent, PWScore *core,
                             CItemData *item,
                             CItemData **pbci,
                             wxWindowID id, const wxString& caption,
                             const wxPoint& pos, const wxSize& size,
                             long style)
{
  return new SelectAliasDlg(parent, core, item, pbci, id, caption, pos, size, style);
}

/*!
 * Control creation for SelectAliasDlg
 */

void SelectAliasDlg::CreateControls()
{    
  auto itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(itemBoxSizer2);

  auto itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5);

  auto itemStaticText2 = new wxStaticText(this, wxID_STATIC, _("Alias name:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_AliasBaseTextCtrl = new wxTextCtrl(this, ID_ALIASNAME, m_AliasName, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  itemBoxSizer1->Add(m_AliasBaseTextCtrl, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 5);
  // Set validators
  m_AliasBaseTextCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, &m_AliasName) );

  // Selection Tree inside a box
  auto staticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this);
  itemBoxSizer2->Add(staticBoxSizer1, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);

  m_Tree = new SelectTreeCtrl(staticBoxSizer1->GetStaticBox(), *m_Core, ID_ENTRYTREE, 
    wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS |wxTR_HIDE_ROOT|wxTR_SINGLE
  );
  m_Tree->SetSortingGroup();
  m_Tree->SetShowGroup(false);
  staticBoxSizer1->Add(m_Tree, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 0);

  auto itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  auto itemButton6 = new wxButton(this, wxID_REMOVE);
  itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto itemButton7 = new wxButton(this, wxID_OK, _("Select"));
  itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto itemButton8 = new wxButton(this, wxID_CANCEL);
  itemBoxSizer5->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  auto itemButton9 = new wxButton(this, wxID_HELP);
  itemBoxSizer5->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  if (*m_BaseItem == nullptr || m_Core->IsReadOnly()) {
    itemButton6->Disable();
    itemButton7->Disable();
  }
}


/*!
 * Fill content int Tree for SelectAliasDlg
 */

void SelectAliasDlg::InitDialog()
{
  int fontHeight = 15;
  wxCoord width, height;
  wxSize actSize = m_Tree->GetSize();
  
  m_Tree->Clear();
  wxFont font(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::TreeFont)));
  if (font.IsOk()) {
    m_Tree->SetFont(font);
    
    wxSize size = font.GetPixelSize();
    fontHeight = size.GetHeight();
  }
  
  wxSize minSize(actSize.GetWidth(), (fontHeight + 10) * SELECT_DEFAULT_NUM_ROWS);
  m_Tree->SetMinClientSize(minSize);
  
  GetSize(&width, &height);
  height += minSize.GetHeight() - actSize.GetHeight();
  int displayWidth, displayHeight;
  ::wxDisplaySize(&displayWidth, &displayHeight);
  if(height > displayHeight) height = displayHeight;
  SetSize(width, height);
  
  const pws_os::CUUID item_uuid = m_Item->GetUUID();
  
  ItemListConstIter iter;
  for (iter = m_Core->GetEntryIter();
       iter != m_Core->GetEntryEndIter();
       iter++) {
    if((iter->second).IsShortcut() || (iter->second).IsAlias()) {
      const pws_os::CUUID base_uuid = (iter->second).GetBaseUUID();
      if (base_uuid == item_uuid) // Do not include alias or shortcut to the base entry
        continue;
    }
    const pws_os::CUUID uuid = (iter->second).GetUUID();
    if (item_uuid != uuid) // Do not include own item as selectable
      m_Tree->AddItem(iter->second);
  }
  
  if (m_Tree->HasItems()) {// avoid assertion!
    m_Tree->SortChildrenRecursively(m_Tree->GetRootItem());
  }
  
  if(*m_BaseItem) {
    uuid_array_t uuid;
    (*m_BaseItem)->GetUUID(uuid);
    m_Tree->SelectItem(uuid);
  }
  
  m_Tree->SetFocus();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REMOVE
 */

void SelectAliasDlg::OnRemoveClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    *m_BaseItem = nullptr; // Mark base entry as NULL and return from dialog
    EndModal(wxID_OK);
  }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SelectAliasDlg::OnOkClick( wxCommandEvent& event )
{
  if (Validate() && TransferDataFromWindow()) {
    if(m_AliasName.IsEmpty()) {
      wxMessageBox(_("No Alias name given."), _("Name the alias."), wxOK|wxICON_ERROR);
      return;
    }
  }
  EndModal(wxID_OK);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void SelectAliasDlg::OnCancelClick( wxCommandEvent& event )
{
  EndModal(wxID_CANCEL);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP
 */

void SelectAliasDlg::OnHelpClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in SelectAliasDlg.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_HELP in SelectAliasDlg.
}


/*!
 * Should we show tooltips?
 */

bool SelectAliasDlg::ShowToolTips()
{
  return true;
}


/*!
 * Get bitmap resources
 */

wxBitmap SelectAliasDlg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin SelectAliasDlg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end SelectAliasDlg bitmap retrieval
}


/*!
 * Get icon resources
 */

wxIcon SelectAliasDlg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin SelectAliasDlg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end SelectAliasDlg icon retrieval
}


/*!
 * Selection is changing
 */

void SelectAliasDlg::UpdateSelChanged(CItemData *pci)
{
  if(pci && !m_Core->IsReadOnly()) {
    *m_BaseItem = pci;
    StringX pwd = L"[" +
                pci->GetGroup() + L":" +
                pci->GetTitle() + L":" +
                pci->GetUser()  + L"]";
    m_AliasName = pwd.c_str();
    m_AliasBaseTextCtrl->SetValue(m_AliasName);
    
    FindWindow(wxID_REMOVE)->Enable();
    FindWindow(wxID_OK)->Enable();
  }
  if(!pci) {
    FindWindow(wxID_REMOVE)->Disable();
    FindWindow(wxID_OK)->Disable();
  }
  if(m_Core->IsReadOnly() && *m_BaseItem) {
    uuid_array_t uuid;
    (*m_BaseItem)->GetUUID(uuid);
    m_Tree->SelectItem(uuid);
  }
}


///////////////////////////////////////////
// Handles right-click event forwarded by the tree and list views
void SelectAliasDlg::OnContextMenu(const CItemData* item)
{
  wxMenu selectMenu;
  if (item != nullptr) {
    selectMenu.Append(ID_VIEW_SELECT, _("&View Entry..."));
  }
  selectMenu.Append(ID_EXPANDALL_SELECT, _("Expand"));
  selectMenu.Append(ID_COLLAPSEALL_SELECT, _("Collapse"));

  m_Tree->SetCurrentMenuItem(item);
  m_Tree->PopupMenu(&selectMenu);
}

void SelectTreeCtrl::OnExpandAll(wxCommandEvent& evt)
{
  if (!IsEmpty()) {
    ExpandAll();
  }
}

void SelectTreeCtrl::OnCollapseAll(wxCommandEvent& evt)
{
  if (IsEmpty()) {
    return;
  }
  
  wxTreeItemId selItem = GetSelection();

  //we cannot just call wxTreeCtrl::CollapseAll(), since it tries to
  //collapse the invisible root item also, and thus ASSERTs
  wxTreeItemIdValue cookie;
  for ( wxTreeItemId root = GetRootItem(), idCurr = GetFirstChild(root, cookie);
        idCurr.IsOk();
        idCurr = GetNextChild(root, cookie) )
  {
      CollapseAllChildren(idCurr);
  }
  // Show selected item
  if(selItem.IsOk()) {
    wxTreeItemId parent = GetItemParent(selItem);
    if(parent.IsOk() && (parent != GetRootItem()) && ! IsExpanded(parent))
      Expand(parent);
    ::wxSafeYield();
    EnsureVisible(selItem);
    ::wxSafeYield();
    
    wxTreeCtrl::SelectItem(selItem);
  }
}

void SelectTreeCtrl::OnViewClick(wxCommandEvent&)
{
  CallAfter(&SelectTreeCtrl::DoViewClick);
}

void SelectTreeCtrl::DoViewClick()
{
  if(m_menu_item) {
    CItemData item = *m_menu_item;
    item.SetProtected(true);
    ShowModalAndGetResult<AddEditPropSheetDlg>(this, m_core, AddEditPropSheetDlg::SheetType::VIEW, &item);
  }
}
