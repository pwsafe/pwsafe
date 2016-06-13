/*
 * Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file mainView.cpp
 *  This file contains implementations of PasswordSafeFrame
 *  member functions corresponding to actions under the 'View'
 *  menubar menu.
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/fontdlg.h> 

#include "passwordsafeframe.h"
#include "PWSgrid.h"
#include "PWStree.h"
#include "PWSDragBar.h"
#include "PasswordSafeSearch.h"
#include "core/PWSprefs.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

void PasswordSafeFrame::OnChangeToolbarType(wxCommandEvent& evt)
{
  //This assumes the menu item is checked before it comes here
  if (GetMenuBar()->IsChecked(evt.GetId())) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::UseNewToolbar, evt.GetId() == ID_TOOLBAR_NEW);
    RefreshToolbarButtons();
    PWSDragBar* dragbar = GetDragBar();
    wxCHECK_RET(dragbar, wxT("Could not find dragbar"));
    dragbar->RefreshButtons();
    wxCHECK_RET(m_search, wxT("Search object not created as expected"));
    m_search->RefreshButtons();
  }
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_LIST_VIEW
 */

void PasswordSafeFrame::OnListViewClick( wxCommandEvent& /* evt */ )
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView, _T("list"));
  ShowTree(false);
  ShowGrid(true);
  m_currentView = GRID;
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_TREE_VIEW
 */

void PasswordSafeFrame::OnTreeViewClick( wxCommandEvent& /* evt */ )
{
  PWSprefs::GetInstance()->SetPref(PWSprefs::LastView, _T("tree"));
  ShowGrid(false);
  ShowTree(true);
  m_currentView = TREE;
}

void PasswordSafeFrame::OnExpandAll(wxCommandEvent& /*evt*/)
{
  wxASSERT(m_currentView == TREE);
  m_tree->ExpandAll();
}

void PasswordSafeFrame::OnCollapseAll(wxCommandEvent& /*evt*/)
{
  wxASSERT(m_currentView == TREE);

  //we cannot just call wxTreeCtrl::CollapseAll(), since it tries to
  //collapse the invisible root item also, and thus ASSERTs
  wxTreeItemIdValue cookie;
  for ( wxTreeItemId root = m_tree->GetRootItem(), idCurr = m_tree->GetFirstChild(root, cookie);
        idCurr.IsOk();
        idCurr = m_tree->GetNextChild(root, cookie) )
  {
      m_tree->CollapseAllChildren(idCurr);
  }
}

void PasswordSafeFrame::OnChangeTreeFont(wxCommandEvent& /*evt*/)
{
  wxFont currentFont(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::TreeFont)));

  if (!currentFont.IsOk()) {
    currentFont = IsTreeView() ? m_tree->GetFont() : m_grid->GetDefaultCellFont();
  }

  wxFont newFont = ::wxGetFontFromUser(this, currentFont, _("Select Tree/List display font"));

  if (newFont.IsOk()) {
    if (IsTreeView()) {
      m_tree->SetFont(newFont);
    }
    else {
      m_grid->SetDefaultCellFont(newFont);
    }
    PWSprefs::GetInstance()->SetPref(PWSprefs::TreeFont, tostringx(newFont.GetNativeFontInfoDesc()));
  }
}

void PasswordSafeFrame::OnChangePasswordFont(wxCommandEvent& /*evt*/)
{
  wxFont passwordFont(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::PasswordFont)));

  wxFont newFont = ::wxGetFontFromUser(this, passwordFont, _("Set Password display font"));
  if (newFont.IsOk()) {
    PWSprefs::GetInstance()->SetPref(PWSprefs::PasswordFont, tostringx(newFont.GetNativeFontInfoDesc()));
  }
}

void PasswordSafeFrame::OnShowHideToolBar(wxCommandEvent& evt)
{
  GetToolBar()->Show(evt.IsChecked());
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowToolbar, evt.IsChecked());
  DoLayout();
  SendSizeEvent();
}

void PasswordSafeFrame::OnShowHideDragBar(wxCommandEvent& evt)
{
  PWSDragBar* dragbar = GetDragBar();
  wxCHECK_RET(dragbar, wxT("Could not find dragbar"));

  dragbar->Show(evt.IsChecked());
  PWSprefs::GetInstance()->SetPref(PWSprefs::ShowDragbar, evt.IsChecked());
  DoLayout();
}

//-----------------------------------------------------------------
// Filters related
// (Starting with ShowExpiry, implementing top-down as needed)
//-----------------------------------------------------------------
void PasswordSafeFrame::OnShowAllExpiryClick( wxCommandEvent& event )
{
  m_bShowExpiry = event.IsChecked();
  m_bFilterActive = m_bShowExpiry; //for now these are synonymous
  if (m_bShowExpiry) {
    st_filters showexpirefilter;
    showexpirefilter.Empty();
    showexpirefilter.fname = _("Entries with a password expiry");

    st_FilterRow fr;

    fr.bFilterComplete = true;
    fr.ftype = FT_XTIME;
    fr.mtype = PWSMatch::MT_DATE;
    fr.rule = PWSMatch::MR_NOTEQUAL;
    fr.ltype = LC_OR;

    fr.fdate1 = 0;
    showexpirefilter.vMfldata.push_back(fr);
    showexpirefilter.num_Mactive = (int)showexpirefilter.vMfldata.size();
  } else {
  }
  ApplyFilters();
}

void PasswordSafeFrame::ApplyFilters()
{
  m_tree->SetFilterState(m_bFilterActive);
  m_grid->SetFilterState(m_bFilterActive);
  UpdateStatusBar();
}
