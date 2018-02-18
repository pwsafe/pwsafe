/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PasswordSafeSearch.h"
#include "../../core/PwsPlatform.h"
#include "../../core/PWHistory.h"
#include "../../core/Util.h"
#include "passwordsafeframe.h"
#include "wxutils.h"
#include "AdvancedSelectionDlg.h"
#include "./SelectionCriteria.h"
#include "./SearchUtils.h"

////@begin XPM images
#include "./graphics/findtoolbar/new/find.xpm"
#include "./graphics/findtoolbar/new/findreport.xpm"
#include "./graphics/findtoolbar/new/find_disabled.xpm"
#include "./graphics/findtoolbar/new/findadvanced.xpm"
#include "./graphics/findtoolbar/new/findcase_i.xpm"
#include "./graphics/findtoolbar/new/findcase_s.xpm"
#include "./graphics/findtoolbar/new/findclear.xpm"
#include "./graphics/findtoolbar/new/findclose.xpm"
//-- classic bitmaps...
#include "./graphics/findtoolbar/classic/find.xpm"
#include "./graphics/findtoolbar/classic/findreport.xpm"
#include "./graphics/findtoolbar/classic/find_disabled.xpm"
#include "./graphics/findtoolbar/classic/findadvanced.xpm"
#include "./graphics/findtoolbar/classic/findcase_i.xpm"
#include "./graphics/findtoolbar/classic/findcase_s.xpm"
#include "./graphics/findtoolbar/classic/findclear.xpm"
#include "./graphics/findtoolbar/classic/findclose.xpm"
////@end XPM images
#include <wx/statline.h>
#include <wx/valgen.h>
#include <wx/srchctrl.h>

#include <functional>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

enum { FIND_MENU_POSITION = 4 } ;

////////////////////////////////////////////////////////////////////////////
// PasswordSafeSerach implementation
IMPLEMENT_CLASS( PasswordSafeSearch, wxEvtHandler )

enum {
  ID_FIND_CLOSE = 10061,
  ID_FIND_EDITBOX,
  ID_FIND_NEXT,
  ID_FIND_IGNORE_CASE,
  ID_FIND_ADVANCED_OPTIONS,
  ID_FIND_CREATE_REPORT,
  ID_FIND_CLEAR,
  ID_FIND_STATUS_AREA
};

PasswordSafeSearch::PasswordSafeSearch(PasswordSafeFrame* parent) : m_toolbar(0),
                                                                    m_parentFrame(parent),
                                                                    m_criteria(new SelectionCriteria)
{
}

PasswordSafeSearch::~PasswordSafeSearch(void)
{
  delete m_toolbar;
  m_toolbar = 0;
  delete m_criteria;
  m_criteria = 0;
}

void PasswordSafeSearch::OnSearchTextChanged(wxCommandEvent& evt)
{
  wxSearchCtrl *srchCtrl = wxDynamicCast(evt.GetEventObject(), wxSearchCtrl);
  srchCtrl->SetModified(true);
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_FIND_EDITBOX
 */

void PasswordSafeSearch::OnDoSearch(wxCommandEvent& /*evt*/)
{
  if (m_parentFrame->IsTreeView()) {
    OrderedItemList olist;
    olist.reserve(m_parentFrame->GetNumEntries());
    m_parentFrame->FlattenTree(olist);

    OnDoSearchT(olist.begin(), olist.end(), dereference<OrderedItemList>());
  }
  else
    OnDoSearchT(m_parentFrame->GetEntryIter(), m_parentFrame->GetEntryEndIter(), get_second<ItemList>());
}

template <class Iter, class Accessor>
void PasswordSafeSearch::OnDoSearchT(Iter begin, Iter end, Accessor afn)
{
  wxASSERT(m_toolbar);

  wxSearchCtrl* txtCtrl = wxDynamicCast(m_toolbar->FindControl(ID_FIND_EDITBOX), wxSearchCtrl);
  wxASSERT(txtCtrl);

  const wxString searchText = txtCtrl->GetLineText(0);

  if (searchText.IsEmpty())
    return;

  if (m_criteria->IsDirty() || txtCtrl->IsModified() || m_searchPointer.IsEmpty())  {
      m_searchPointer.Clear();

      if (!m_toolbar->GetToolState(ID_FIND_ADVANCED_OPTIONS))
        FindMatches(tostringx(searchText), m_toolbar->GetToolState(ID_FIND_IGNORE_CASE), m_searchPointer, begin, end, afn);
      else {
        m_searchPointer.Clear();
        ::FindMatches(tostringx(searchText), m_toolbar->GetToolState(ID_FIND_IGNORE_CASE), m_criteria->GetSelectedFields(),
                      m_criteria->HasSubgroupRestriction(), tostdstring(m_criteria->SubgroupSearchText()),
                      m_criteria->SubgroupObject(), m_criteria->SubgroupFunction(),
                    m_criteria->CaseSensitive(), begin, end, afn, [this, afn](Iter itr, bool *keep_going) {
                      uuid_array_t uuid;
                      afn(itr).GetUUID(uuid);
                      m_searchPointer.Add(pws_os::CUUID(uuid));
                      *keep_going = true;
                    });
      }

      m_criteria->Clean();
      txtCtrl->SetModified(false);
      m_searchPointer.InitIndex();
  }
  else {
      ++m_searchPointer;
  }

  UpdateView();

  // Replace the "Find" menu item under Edit menu by "Find Next" and "Find Previous"
  wxMenu* editMenu = 0;
  wxMenuItem* findItem = m_parentFrame->GetMenuBar()->FindItem(wxID_FIND, &editMenu);
  if (findItem && editMenu)  {
      //Is there a way to do this without hard-coding the insert position?
    if (!m_parentFrame->GetMenuBar()->FindItem(ID_EDITMENU_FIND_NEXT) ) {
      editMenu->Insert(FIND_MENU_POSITION, ID_EDITMENU_FIND_NEXT, _("&Find next...\tF3"), wxEmptyString, wxITEM_NORMAL);
    }
    if (!m_parentFrame->GetMenuBar()->FindItem(ID_EDITMENU_FIND_PREVIOUS) ) {
      editMenu->Insert(FIND_MENU_POSITION+1, ID_EDITMENU_FIND_PREVIOUS, _("&Find previous...\tSHIFT+F3"), wxEmptyString, wxITEM_NORMAL);
    }
  }
}

void PasswordSafeSearch::UpdateView()
{
  wxStaticText* statusArea = wxDynamicCast(m_toolbar->FindWindow(ID_FIND_STATUS_AREA), wxStaticText);
  wxASSERT(statusArea);

  if (!m_searchPointer.IsEmpty()) {
    m_parentFrame->SelectItem(*m_searchPointer);
  }
  statusArea->SetLabel(m_searchPointer.GetLabel());
}

void PasswordSafeSearch::FindNext()
{
    if (!m_searchPointer.IsEmpty()) {
      ++m_searchPointer;
      UpdateView();
    }
}

void PasswordSafeSearch::FindPrevious()
{
    if (!m_searchPointer.IsEmpty()) {
      --m_searchPointer;
      UpdateView();
    }
}

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_CLOSE
 */

void PasswordSafeSearch::OnSearchClose(wxCommandEvent& /* evt */)
{
  HideSearchToolbar();
}

void PasswordSafeSearch::OnSearchClear(wxCommandEvent& /* evt */)
{
  wxSearchCtrl* txtCtrl = wxDynamicCast(m_toolbar->FindControl(ID_FIND_EDITBOX), wxSearchCtrl);
  wxCHECK_RET(txtCtrl, wxT("Could not get search ctrl from toolbar"));
  txtCtrl->Clear();
  m_searchPointer.Clear();
  ClearToolbarStatusArea();
}

void PasswordSafeSearch::HideSearchToolbar()
{
  if (m_toolbar == nullptr)
    return;
  m_toolbar->Show(false);
  m_parentFrame->GetSizer()->Layout();
  m_parentFrame->SetFocus();

  wxMenu* editMenu = 0; // will be set by FindItem() below
  wxMenuItem* findNextItem = m_parentFrame->GetMenuBar()->FindItem(ID_EDITMENU_FIND_NEXT, &editMenu);
  if (editMenu) { //the menu might not have been modified if nothing was actually searched for
    if (findNextItem)
      editMenu->Delete(findNextItem);

    wxMenuItem* findPreviousItem = m_parentFrame->GetMenuBar()->FindItem(ID_EDITMENU_FIND_PREVIOUS, 0);
    if (findPreviousItem)
      editMenu->Delete(findPreviousItem);
  }
}

void PasswordSafeSearch::ClearToolbarStatusArea()
{
  wxStaticText* statusArea = wxDynamicCast(m_toolbar->FindWindow(ID_FIND_STATUS_AREA), wxStaticText);
  wxCHECK_RET(statusArea, wxT("Could not retrieve status area from search bar"));
  statusArea->SetLabel(wxEmptyString);
}

struct FindDlgType {
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced Find Options");
  }

  static bool IsMandatoryField(CItemData::FieldType /*field*/) {
    return false;
  }

  static bool IsPreselectedField(CItemData::FieldType /*field*/) {
    return true;
  }

  static bool IsUsableField(CItemData::FieldType field) {
    switch (field) {
      case CItemData::GROUP:
      case CItemData::TITLE:
      case CItemData::USER:
      case CItemData::NOTES:
      case CItemData::PASSWORD:
      case CItemData::URL:
      case CItemData::AUTOTYPE:
      case CItemData::PWHIST:
      case CItemData::RUNCMD:
      case CItemData::EMAIL:
      case CItemData::SYMBOLS:
        return true;
      default:
        return false;
    }
  }

  static bool ShowFieldSelection() {
    return true;
  }

  static wxString GetTaskWord() {
    return _("search");
  }
};

IMPLEMENT_CLASS_TEMPLATE( AdvancedSelectionDlg, wxDialog, FindDlgType )

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_ADVANCED_OPTIONS
 */
void PasswordSafeSearch::OnAdvancedSearchOptions(wxCommandEvent& evt)
{
  if (evt.IsChecked()) {
    m_criteria->Clean();
    AdvancedSelectionDlg<FindDlgType> dlg(m_parentFrame, m_criteria);
    if (dlg.ShowModal() == wxID_OK) {
      // No check for m_criteria.IsDirty() here because we want to start a new search
      // whether or not the group/field selection were modified because user just
      // toggled the "Advanced Options" on.  It was OFF before just now.
      m_searchPointer.Clear();
    }
    else {
      // No change, but need to toggle off "Advanced Options" button manually
      m_toolbar->ToggleTool(evt.GetId(), false);
    }
  }
  else {
    // Advanced Options were toggled off.  Start a new search next time
    m_searchPointer.Clear();
  }
}

void PasswordSafeSearch::RefreshButtons(void)
{
  if (!m_toolbar)
    return;

  static struct _SearchBarInfo{
    int id;
    const char** bitmap_normal;
    const char** bitmap_disabled;
    const char** bitmap_classic;
    const char** bitmap_classic_disabled;
  } SearchBarButtons[] = {
          { ID_FIND_CLOSE,            findclose_xpm,    nullptr,               classic_findclose_xpm,    nullptr                      },
          { ID_FIND_NEXT,             find_xpm,         find_disabled_xpm,  classic_find_xpm,         classic_find_disabled_xpm },
          { ID_FIND_IGNORE_CASE,      findcase_i_xpm,   findcase_s_xpm,     classic_findcase_i_xpm,   classic_findcase_s_xpm    },
          { ID_FIND_ADVANCED_OPTIONS, findadvanced_xpm, nullptr,               classic_findadvanced_xpm, nullptr                      },
          { ID_FIND_CREATE_REPORT,    findreport_xpm,   nullptr,               classic_findreport_xpm,   nullptr                      },
          { ID_FIND_CLEAR,            findclear_xpm,    nullptr,               classic_findclear_xpm,    nullptr                      },
  };

  const char** _SearchBarInfo::* bitmap_normal;
  const char** _SearchBarInfo::* bitmap_disabled;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar)) {
    bitmap_normal = &_SearchBarInfo::bitmap_normal;
    bitmap_disabled = &_SearchBarInfo::bitmap_normal;
  }
  else {
    bitmap_normal = &_SearchBarInfo::bitmap_classic;
    bitmap_disabled = &_SearchBarInfo::bitmap_classic_disabled;
  }

  for (size_t idx = 0; idx < NumberOf(SearchBarButtons); ++idx) {
    m_toolbar->SetToolNormalBitmap(SearchBarButtons[idx].id, wxBitmap(SearchBarButtons[idx].*bitmap_normal));
    if (SearchBarButtons[idx].*bitmap_disabled)
      m_toolbar->SetToolDisabledBitmap(SearchBarButtons[idx].id, wxBitmap(SearchBarButtons[idx].*bitmap_disabled));
  }
}

/**
 * Recreates the search bar with the last state 
 * regarding its visibility on the UI.
 */
void PasswordSafeSearch::ReCreateSearchBar(void)
{
  if (m_toolbar != nullptr) {
    // remember last status of search bar
    bool show = m_toolbar->IsShown();

    // destroy the existing search bar
    wxDELETE(m_toolbar);

    // here a new search bar is going to be created
    // right after creation it appears on the UI
    CreateSearchBar();

    // if the previous search bar was hidden then
    // hide also the new one
    if (show == false)
      HideSearchToolbar();
  }
}

/*!
 * Creates the search bar and keeps it hidden
 */
void PasswordSafeSearch::CreateSearchBar()
{
  wxASSERT(m_toolbar == 0);
  wxPanel *panel = new wxPanel(m_parentFrame, wxID_ANY);
  wxBoxSizer *panelSizer = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(panelSizer);
  m_toolbar = new wxToolBar(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTB_BOTTOM | wxTB_HORIZONTAL,  wxT("SearchBar"));
  panelSizer->Add(m_toolbar, wxSizerFlags().Proportion(1).Expand());

  m_toolbar->AddTool(ID_FIND_CLOSE, wxEmptyString, wxBitmap(findclose_xpm), wxNullBitmap, wxITEM_NORMAL, _("Close SearchBar"));
  wxSize srchCtrlSize(m_parentFrame->GetSize().GetWidth()/3, wxDefaultSize.GetHeight());
  wxSearchCtrl* srchCtrl = new wxSearchCtrl(m_toolbar, ID_FIND_EDITBOX, wxEmptyString, wxDefaultPosition, srchCtrlSize, wxTE_PROCESS_ENTER);
  srchCtrl->ShowCancelButton(true);
  srchCtrl->ShowSearchButton(true);

  m_toolbar->AddControl(srchCtrl);
  m_toolbar->AddTool(ID_FIND_NEXT, wxEmptyString, wxBitmap(find_xpm), wxBitmap(find_disabled_xpm), wxITEM_NORMAL, _("Find Next"));
  m_toolbar->AddCheckTool(ID_FIND_IGNORE_CASE, wxEmptyString, wxBitmap(findcase_i_xpm), wxBitmap(findcase_s_xpm), _("Case Insensitive Search"));
  m_toolbar->AddTool(ID_FIND_ADVANCED_OPTIONS, wxEmptyString, wxBitmap(findadvanced_xpm), wxNullBitmap, wxITEM_CHECK, _("Advanced Find Options"));
  m_toolbar->AddTool(ID_FIND_CREATE_REPORT, wxEmptyString, wxBitmap(findreport_xpm), wxNullBitmap, wxITEM_NORMAL, _("Create report of previous Find search"));
  m_toolbar->AddTool(ID_FIND_CLEAR, wxEmptyString, wxBitmap(findclear_xpm), wxNullBitmap, wxITEM_NORMAL, _("Clear Find"));
  m_toolbar->AddControl(new wxStaticText(m_toolbar, ID_FIND_STATUS_AREA, wxEmptyString, wxDefaultPosition, srchCtrlSize.Scale(3,1), wxTE_READONLY));

  RefreshButtons();

  if (!m_toolbar->Realize())
    wxMessageBox(_("Could not create Search Bar"), _("Password Safe"));

  wxSizer* origSizer = m_parentFrame->GetSizer();
  wxASSERT(origSizer);
  wxASSERT(origSizer->IsKindOf(wxBoxSizer(wxVERTICAL).GetClassInfo()));
  wxASSERT(((wxBoxSizer*)origSizer)->GetOrientation() == wxVERTICAL);
  origSizer->Add(panel, 0, wxEXPAND);
  origSizer->Layout();
  if (!m_toolbar->Show(true) && !m_toolbar->IsShownOnScreen())
    wxMessageBox(_("Could not display searchbar"));

  //This gross hack is the only way I could think of to get ESC keystrokes from the text ctrl user is typing into
  if (wxDynamicCast(static_cast<wxControl*>(srchCtrl), wxTextCtrl)) {
    //searchCtrl is a wxTextCtrl derivative, like on Mac OS X 10.3+
    wxDynamicCast(static_cast<wxControl*>(srchCtrl), wxTextCtrl)->Connect(wxEVT_CHAR, wxCharEventHandler(PasswordSafeSearch::OnSearchBarTextChar), nullptr, this);
  }
  else {
    //The wxTextCtrl is buried inside the wxSearchCtrl
    wxWindowList& srchChildren = srchCtrl->GetChildren();
    for( wxWindowList::const_iterator itr = srchChildren.begin(); itr != srchChildren.end(); ++itr) {
      wxTextCtrl* txtCtrl = wxDynamicCast(*itr, wxTextCtrl);
      if (txtCtrl) {
        txtCtrl->Connect(wxEVT_CHAR, wxCharEventHandler(PasswordSafeSearch::OnSearchBarTextChar), nullptr, this);
        break;
      }
    }
  }
  srchCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(PasswordSafeSearch::OnSearchTextChanged), nullptr, this);
  srchCtrl->Connect(wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler(PasswordSafeSearch::OnDoSearch), nullptr, this);
  srchCtrl->Connect(wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler(PasswordSafeSearch::OnDoSearch), nullptr, this);
  m_toolbar->Connect(ID_FIND_CLOSE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnSearchClose), nullptr, this);
  m_toolbar->Connect(ID_FIND_ADVANCED_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnAdvancedSearchOptions), nullptr, this);
  m_toolbar->Connect(ID_FIND_NEXT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnDoSearch), nullptr, this);
  m_toolbar->Connect(ID_FIND_CLEAR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnSearchClear), nullptr, this);
}

void PasswordSafeSearch::OnSearchBarTextChar(wxKeyEvent& evt)
{
  if (evt.GetKeyCode() == WXK_ESCAPE) {
    HideSearchToolbar();
  }
  else {
    evt.Skip();
  }
}

/*!
 * Called when user clicks Find from Edit menu, or presses Ctrl-F
 */
void PasswordSafeSearch::Activate(void)
{
  if (!m_toolbar)
    CreateSearchBar();
  else {
    if ( m_toolbar->IsShownOnScreen() ) {
        m_toolbar->FindControl(ID_FIND_EDITBOX)->SetFocus();
    }
    else if (m_toolbar->Show(true)) {
      wxSize srchCtrlSize(m_parentFrame->GetSize().GetWidth()/5, wxDefaultSize.GetHeight());
      m_toolbar->FindControl(ID_FIND_EDITBOX)->SetSize(srchCtrlSize);
      m_parentFrame->GetSizer()->Layout();
    }
    else {
      const wxPoint pt = m_toolbar->GetPosition();
      const wxSize sz = m_toolbar->GetSize();
      wxMessageBox(wxString() << _("Could not re-display searchbar at ") << pt << _(" of size ") << sz
                              << _(" because ") << (m_toolbar->IsShownOnScreen()? _("it's already visible"): _(" of an error")));
    }
  }

  wxCHECK_RET(m_toolbar, wxT("Could not create or retrieve search bar"));

  m_toolbar->FindControl(ID_FIND_EDITBOX)->SetFocus();
  ClearToolbarStatusArea();
}

template <class Iter, class Accessor>
void PasswordSafeSearch::FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr, Iter begin, Iter end, Accessor afn)
{
  searchPtr.Clear();
  //As per original Windows code, default search is for all text fields
  CItemData::FieldBits bsFields;
  bsFields.set();

  return ::FindMatches(searchText, fCaseSensitive, bsFields, false, stringT{}, CItemData::END, PWSMatch::MR_INVALID, false, begin, end, afn,
                     [&searchPtr, afn](Iter itr, bool *keep_going) {
                       uuid_array_t uuid;
                       afn(itr).GetUUID(uuid);
                       searchPtr.Add(pws_os::CUUID(uuid));
                       *keep_going = true;
                     });
}

/////////////////////////////////////////////////
// SearchPointer class definition
SearchPointer& SearchPointer::operator++()
{ //prefix operator, to prevent copying itself
  if (!m_indices.empty()) {
    m_currentIndex++;
    if (m_currentIndex == m_indices.end()) {
      m_currentIndex = m_indices.begin();
      PrintLabel(_("Search hit bottom, continuing at top").c_str());
    }
    else {
      PrintLabel();
    }
  }
  else {
    m_currentIndex = m_indices.end();
  }

  return *this;
}

SearchPointer& SearchPointer::operator--()
{ //prefix operator, to prevent copying itself
  if (!m_indices.empty()) {
    if (m_currentIndex == m_indices.begin()) {
      m_currentIndex = --m_indices.end();
      PrintLabel(_("Search hit top, continuing at bottom").c_str());
    }
    else {
      m_currentIndex--;
      PrintLabel();
    }
  }
  else
    m_currentIndex = m_indices.end();

  return *this;
}

void SearchPointer::PrintLabel(const TCHAR* prefix /*= 0*/)
{
  if (m_indices.empty())
    m_label = _("No matches found");
  else if (m_indices.size() == 1)
    m_label = _("1 match");
  else {
    // need a const object so we get both args to distance() as const iterators
    const SearchIndices& idx = m_indices;
    m_label.Clear();
    m_label << std::distance(idx.begin(), m_currentIndex)+1 << '/' << m_indices.size() << wxT(" matches");
    if (prefix)
      m_label = wxString(prefix) + wxT(".  ") + m_label;
  }
}
