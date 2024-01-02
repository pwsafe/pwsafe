/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PasswordSafeSearch.cpp
*
*/

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/statline.h>
#include <wx/valgen.h>
#include <wx/srchctrl.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "core/PWHistory.h"
#include "core/Util.h"
#include "core/SearchUtils.h"
#include "core/core.h"
#include "wxUtilities.h"

#include "PasswordSafeSearch.h"
#include "PasswordSafeFrame.h"
#include "AdvancedSelectionDlg.h"
#include "PWSafeApp.h"
#include "SelectionCriteria.h"
#include "ViewReportDlg.h"

////@begin XPM images
#include "graphics/findtoolbar/new/find.xpm"
#include "graphics/findtoolbar/new/findreport.xpm"
#include "graphics/findtoolbar/new/find_disabled.xpm"
#include "graphics/findtoolbar/new/findadvanced.xpm"
#include "graphics/findtoolbar/new/findcase_i.xpm"
#include "graphics/findtoolbar/new/findcase_s.xpm"
#include "graphics/findtoolbar/new/findclear.xpm"
#include "graphics/findtoolbar/new/findclose.xpm"
//-- classic bitmaps...
#include "graphics/findtoolbar/classic/find.xpm"
#include "graphics/findtoolbar/classic/findreport.xpm"
#include "graphics/findtoolbar/classic/find_disabled.xpm"
#include "graphics/findtoolbar/classic/findadvanced.xpm"
#include "graphics/findtoolbar/classic/findcase_i.xpm"
#include "graphics/findtoolbar/classic/findcase_s.xpm"
#include "graphics/findtoolbar/classic/findclear.xpm"
#include "graphics/findtoolbar/classic/findclose.xpm"
////@end XPM images

#include <functional>

enum { FIND_MENU_POSITION = 4 } ;

////////////////////////////////////////////////////////////////////////////
// PasswordSafeSerach implementation

enum {
  ID_FIND_CLOSE = 10061,
  ID_FIND_EDITBOX,
  ID_FIND_NEXT,
  ID_FIND_IGNORE_CASE,
  ID_FIND_ADVANCED_OPTIONS,
  ID_FIND_CREATE_REPORT,
  ID_FIND_CLEAR,
  ID_FIND_STATUS_AREA
}; // see also PasswordSafeSearch::CalculateToolsWidth() in case of updates

struct SearchBarToolInfo {
  wxWindowID id;
  const wxString tooltip;
  const char* const* const bitmap_normal;
  const char* const* const bitmap_disabled;
  const char* const* const bitmap_classic;
  const char* const* const bitmap_classic_disabled;
  wxItemKind tool_type;

  SearchBarToolInfo() :
    id(0), tooltip(wxEmptyString),
    bitmap_normal(nullptr), bitmap_disabled(nullptr),
    bitmap_classic(nullptr), bitmap_classic_disabled(nullptr),
    tool_type(wxITEM_NORMAL) {}

  SearchBarToolInfo(
    wxWindowID id, const wxString &tooltip,
    const char* const* const bitmap_normal, const char* const* bitmap_disabled,
    const char* const* const bitmap_classic, const char* const* bitmap_classic_disabled,
    wxItemKind tool_type
  ) :
    id(id), tooltip(tooltip),
    bitmap_normal(bitmap_normal), bitmap_disabled(bitmap_disabled),
    bitmap_classic(bitmap_classic), bitmap_classic_disabled(bitmap_classic_disabled),
    tool_type(tool_type) {}

  bool UseNewToolbarStyle() const
  {
    return PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar);
  }

  /**
   * Provides the bitmap that represents an enabled toolbar item in the new or classic style, depending on user preferences.
   * @return toolbar item bitmap
   */
  wxBitmap GetBitmapForEnabledButton() const
  {
    return UseNewToolbarStyle() ? bitmap_normal ? bitmap_normal : wxNullBitmap : bitmap_classic ? bitmap_classic : wxNullBitmap;
  };

  /**
   * Provides the bitmap that represents an disabled toolbar item in the new or classic style, depending on user preferences.
   * @return toolbar item bitmap
   */
  wxBitmap GetBitmapForDisabledButton() const
  {
    return UseNewToolbarStyle() ? bitmap_disabled ? bitmap_disabled : wxNullBitmap : bitmap_classic_disabled ? bitmap_classic_disabled : wxNullBitmap;
  }
};

std::vector<SearchBarToolInfo> SearchBarToolInfos =
  {
    { ID_FIND_CLOSE,            _("Close"),                                 findclose_xpm,    nullptr,           classic_findclose_xpm,    nullptr,                   wxITEM_NORMAL },
    { ID_FIND_EDITBOX,          _("Search text input field"),               nullptr,          nullptr,           nullptr,                  nullptr,                   wxITEM_MAX    },
    { ID_FIND_NEXT,             _("Find Next"),                             find_xpm,         find_disabled_xpm, classic_find_xpm,         classic_find_disabled_xpm, wxITEM_NORMAL },
    { ID_FIND_IGNORE_CASE,      _("Case Insensitive Search"),               findcase_i_xpm,   findcase_s_xpm,    classic_findcase_i_xpm,   classic_findcase_s_xpm,    wxITEM_CHECK  },
    { ID_FIND_ADVANCED_OPTIONS, _("Advanced Find Options"),                 findadvanced_xpm, nullptr,           classic_findadvanced_xpm, nullptr,                   wxITEM_NORMAL },
    { ID_FIND_CREATE_REPORT,    _("Create report of previous Find search"), findreport_xpm,   nullptr,           classic_findreport_xpm,   nullptr,                   wxITEM_NORMAL },
    { ID_FIND_CLEAR,            _("Clear Find"),                            findclear_xpm,    nullptr,           classic_findclear_xpm,    nullptr,                   wxITEM_NORMAL },
    { ID_FIND_STATUS_AREA,      _("Search status"),                         nullptr,          nullptr,           nullptr,                  nullptr,                   wxITEM_MAX    }
  };

PasswordSafeSearch::PasswordSafeSearch(wxWindow *parent, wxWindowID id, const wxPoint &position, const wxSize &size, long style)
: wxAuiToolBar(parent, id, position, size, style)
{
  m_parentFrame = wxDynamicCast(parent, PasswordSafeFrame);
  m_criteria = new SelectionCriteria();
  m_modified = false;
}

PasswordSafeSearch::~PasswordSafeSearch()
{
  Unbind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnAdvancedSearchOptions, this, ID_FIND_ADVANCED_OPTIONS);
  Unbind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnSearchClose,           this, ID_FIND_CLOSE);
  Unbind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnDoSearch,              this, ID_FIND_NEXT);
  Unbind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnSearchClear,           this, ID_FIND_CLEAR);
  Unbind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnToolBarFindReport,     this, ID_FIND_CREATE_REPORT);
  Unbind(wxEVT_CHAR_HOOK,                                 &PasswordSafeSearch::OnChar,                  this);

  delete m_criteria;
  m_criteria = nullptr;
}

void PasswordSafeSearch::OnSearchTextChanged(wxCommandEvent& WXUNUSED(event))
{
  SetModified(true);
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_FIND_EDITBOX
 */

void PasswordSafeSearch::OnDoSearch(wxCommandEvent& WXUNUSED(event))
{
  if (m_parentFrame->IsTreeView()) {
    OrderedItemList olist;
    olist.reserve(m_parentFrame->GetNumEntries());
    m_parentFrame->FlattenTree(olist);

    OnDoSearchT(olist.begin(), olist.end(), dereference<OrderedItemList>());
  }
  else {
    OnDoSearchT(m_parentFrame->GetEntryIter(), m_parentFrame->GetEntryEndIter(), get_second<ItemList>());
  }
}

template <class Iter, class Accessor>
void PasswordSafeSearch::OnDoSearchT(Iter begin, Iter end, Accessor afn)
{
  wxSearchCtrl* txtCtrl = wxDynamicCast(FindControl(ID_FIND_EDITBOX), wxSearchCtrl);
  wxCHECK_RET(txtCtrl, wxT("Could not get search control of toolbar"));

  const wxString searchText = txtCtrl->GetValue();

  if (searchText.IsEmpty()) {
    return;
  }

  if (m_criteria->IsDirty() || IsModified() || m_searchPointer.IsEmpty()) {
    m_searchPointer.Clear();

    if (!GetToolToggled(ID_FIND_ADVANCED_OPTIONS)) {
      FindMatches(tostringx(searchText), GetToolToggled(ID_FIND_IGNORE_CASE), m_searchPointer, begin, end, afn);
    }
    else {
      m_searchPointer.Clear();
      ::FindMatches(
        tostringx(searchText),
          GetToolToggled(ID_FIND_IGNORE_CASE), m_criteria->GetSelectedFields(),
          m_criteria->HasSubgroupRestriction(), tostdstring(m_criteria->SubgroupSearchText()),
          m_criteria->SubgroupObject(), m_criteria->SubgroupFunction(),
          m_criteria->CaseSensitive(), begin, end, afn, [this, afn](Iter itr, bool *keep_going) {
            uuid_array_t uuid;
            afn(itr).GetUUID(uuid);
            m_searchPointer.Add(pws_os::CUUID(uuid));
            *keep_going = true;
          }
       );
    }

    m_criteria->Clean();
    SetModified(false);
    m_searchPointer.InitIndex();

    // Set last find filter
    UUIDVector res(m_searchPointer);
    m_parentFrame->SetFilterFindEntries(&res);
  }
  else {
    ++m_searchPointer;
  }

  UpdateView();
  txtCtrl->SelectNone();

  // Replace the "Find" menu item under Edit menu by "Find Next" and "Find Previous"
  wxMenu* editMenu = nullptr;
  wxMenuItem* findItem = m_parentFrame->GetMenuBar()->FindItem(wxID_FIND, &editMenu);
  if (findItem && editMenu) {
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
  auto statusArea = FindControl(ID_FIND_STATUS_AREA);
  wxASSERT(statusArea);

  if (!m_searchPointer.IsEmpty()) {
    m_parentFrame->SelectItem(*m_searchPointer);
  }
  statusArea->SetLabel(m_searchPointer.GetLabel());

  SetFocusIntoEditField();
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

void PasswordSafeSearch::OnSearchClose(wxCommandEvent& WXUNUSED(event))
{
  HideSearchToolbar();
}

void PasswordSafeSearch::OnSearchClear(wxCommandEvent& WXUNUSED(event))
{
  wxSearchCtrl* txtCtrl = wxDynamicCast(FindControl(ID_FIND_EDITBOX), wxSearchCtrl);
  wxCHECK_RET(txtCtrl, wxT("Could not get search control from toolbar"));
  txtCtrl->Clear();
  m_searchPointer.Clear();
  ClearToolbarStatusArea();
  m_parentFrame->SetFilterFindEntries(nullptr);
}

/**
 * Event handler (EVT_SIZE) that will be called when the window has been resized.
 *
 * @param event holds information about size change events.
 * @see <a href="http://docs.wxwidgets.org/3.0/classwx_size_event.html">wxSizeEvent Class Reference</a>
 */
void PasswordSafeSearch::OnSize(wxSizeEvent& event)
{
  UpdateStatusAreaWidth();
  event.Skip();
}

/**
 * Updates the width of status area depending on the available space of the toolbar.
 * 
 * @see PasswordSafeSearch::CalculateToolsWidth()
 */
void PasswordSafeSearch::UpdateStatusAreaWidth()
{
  auto control = FindControl(ID_FIND_STATUS_AREA);

  if (control) {
    int statusAreaWidth = (GetParent()->GetClientSize()).GetWidth() - static_cast<int>(m_ToolsWidth);

    if (statusAreaWidth > 0) {
      control->SetSizeHints(statusAreaWidth, wxDefaultSize.GetHeight());
    }
  }
}

/**
 * Calculates the total width of all controls of toolbar without status area.
 * 
 * @note This calculation needs to be done only once, because all controls that 
 *       are located left from status area have a fixed size, which won't change.
 */
void PasswordSafeSearch::CalculateToolsWidth()
{
  m_ToolsWidth = 0;

  std::vector<int> ids = {
    ID_FIND_CLOSE,
    ID_FIND_EDITBOX,
    ID_FIND_NEXT,
    ID_FIND_IGNORE_CASE,
    ID_FIND_ADVANCED_OPTIONS,
    ID_FIND_CREATE_REPORT,
    ID_FIND_CLEAR
  };

  for (const auto& id : ids) {
    auto control = FindControl(id);

    if (control) {
      m_ToolsWidth += (control->GetSize()).GetWidth();
    }
  }
}

/**
 * Calculates the width for the search control.
 * 
 * The search control shall only grow up to a maximum of 200 pixels,
 * which should be enough for a long search text. Limited to this width 
 * more and more space can be made available by the user for the status 
 * area if applications main frame is further resized.
 */
wxSize PasswordSafeSearch::CalculateSearchWidth()
{
  auto width = m_parentFrame->GetSize().GetWidth() < 570 ? (m_parentFrame->GetSize().GetWidth() / 3) : 200;

  return wxSize(width, 30);
}

void PasswordSafeSearch::HideSearchToolbar()
{
  m_parentFrame->HideSearchBar();

  wxMenu* editMenu = nullptr; // will be set by FindItem() below
  wxMenuItem* findNextItem = m_parentFrame->GetMenuBar()->FindItem(ID_EDITMENU_FIND_NEXT, &editMenu);
  if (editMenu) {       // the menu might not have been modified if nothing was actually searched for
    if (findNextItem) {
      editMenu->Delete(findNextItem);
    }
    wxMenuItem* findPreviousItem = m_parentFrame->GetMenuBar()->FindItem(ID_EDITMENU_FIND_PREVIOUS, nullptr);
    if (findPreviousItem) {
      editMenu->Delete(findPreviousItem);
    }
  }
}

void PasswordSafeSearch::ClearToolbarStatusArea()
{
  auto statusArea = FindControl(ID_FIND_STATUS_AREA);
  wxCHECK_RET(statusArea, wxT("Could not retrieve status area from search bar"));
  statusArea->SetLabel(wxEmptyString);
}

struct FindDlgType {
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced Find Options");
  }

  static bool IsMandatoryField(CItemData::FieldType WXUNUSED(field)) {
    return false;
  }

  static bool IsPreselectedField(CItemData::FieldType WXUNUSED(field)) {
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
void PasswordSafeSearch::OnAdvancedSearchOptions(wxCommandEvent& event)
{
  if (event.IsChecked()) {
    CallAfter(&PasswordSafeSearch::GetAdvancedSearchOptions, event.GetId());
  }
  else {
    // Advanced Options were toggled off.  Start a new search next time
    m_searchPointer.Clear();
  }
}

void PasswordSafeSearch::GetAdvancedSearchOptions(int controlId)
{
  m_criteria->Clean();
  if (ShowModalAndGetResult<AdvancedSelectionDlg<FindDlgType>>(wxGetTopLevelParent(this), m_criteria) == wxID_OK) {
    // No check for m_criteria.IsDirty() here because we want to start a new search
    // whether or not the group/field selection were modified because user just
    // toggled the "Advanced Options" on.  It was OFF before just now.
    m_searchPointer.Clear();
  }
  else if (!IsCloseInProgress()) {
    // No change, but need to toggle off "Advanced Options" button manually
    ToggleTool(controlId, false);
  }
}
/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_CREATE_REPORT
 */
void PasswordSafeSearch::OnToolBarFindReport(wxCommandEvent& event)
{
  CallAfter(&PasswordSafeSearch::DoToolBarFindReport, event.GetId());
}

void PasswordSafeSearch::DoToolBarFindReport(int controlId)
{
  wxSearchCtrl* txtCtrl = wxDynamicCast(FindControl(ID_FIND_EDITBOX), wxSearchCtrl);
  wxCHECK_RET(txtCtrl, wxT("Could not get search control of toolbar"));

  const wxString searchText = txtCtrl->GetValue();

  if (searchText.IsEmpty()) {
    return;
  }
  
  CReport report;
  report.StartReport(IDSC_RPTFIND, m_parentFrame->GetCurrentFile().c_str());
  
  if(m_criteria) {
    m_criteria->ReportAdvancedOptions(&report, _("found"), m_parentFrame->GetCurrentFile().c_str());
  }
  
  stringT line, searchT(searchText.c_str()), searchCaseT(GetToolToggled(ID_FIND_IGNORE_CASE) ? _("(case-sensitive)"): _("(not case-sensitive)"));
  
  if (m_searchPointer.IsEmpty()) {
    Format(line, IDSC_SEARCHRESULTS1, searchT.c_str(), searchCaseT.c_str());
    report.WriteLine(line);
  }
  else {
    Format(line, IDSC_SEARCHRESULTS2, searchT.c_str(), searchCaseT.c_str());
    report.WriteLine(line);
    
    size_t numEntries = m_searchPointer.Size();
    size_t idx;
    
    for(idx = 0; idx < numEntries; ++idx) {
      pws_os::CUUID uuid = m_searchPointer.GetUUID(idx);
      wxASSERT(uuid != pws_os::CUUID::NullUUID());
      ItemListConstIter itemIterator = m_parentFrame->FindEntry(uuid);
      if (itemIterator != m_parentFrame->GetEntryEndIter()) {
        const CItemData *item = &itemIterator->second;
        Format(line, IDSC_COMPARESTATS, item->GetGroup().c_str(), item->GetTitle().c_str(), item->GetUser().c_str());
        report.WriteLine(line);
      }
    }
  }
  
  report.WriteLine();
  report.EndReport();
  
  ShowModalAndGetResult<ViewReportDlg>(wxGetTopLevelParent(this), &report);
  if (!IsCloseInProgress()) {
    // set back toggle
    ToggleTool(controlId, false);
  }
}

/**
 * Updates the bitmaps of the tool elements after the user changed the icon style.
 */
void PasswordSafeSearch::UpdateBitmaps()
{
  if (HasTools()) {
    for (const auto & toolInfo : SearchBarToolInfos)
    {
      auto tool = FindTool(toolInfo.id);
      if (tool) {
        tool->SetBitmap(toolInfo.GetBitmapForEnabledButton());
        tool->SetDisabledBitmap(toolInfo.GetBitmapForDisabledButton());
      }
    }

    Realize();
  }
}

/**
 * Recreates the search bar with the last state regarding its visibility on the UI.
 */
void PasswordSafeSearch::ReCreateSearchBar()
{
    // remember last status of search bar
    bool show = IsShown();

    // here a new search bar is going to be created
    // right after creation it appears on the UI
    CreateSearchBar();
    UpdateStatusAreaWidth();

    // if the previous search bar was hidden then
    // hide also the new one
    if (show == false)
      HideSearchToolbar();
}

/**
 * Creates the search bar and keeps it hidden.
 */
bool PasswordSafeSearch::CreateSearchBar()
{
  ClearTools();

  auto searchCtrl = new wxSearchCtrl(this, ID_FIND_EDITBOX, wxEmptyString, wxDefaultPosition, CalculateSearchWidth(), wxTE_PROCESS_ENTER);
  searchCtrl->ShowCancelButton(true);
  searchCtrl->ShowSearchButton(true);

  auto statusText = new wxStaticText(this, ID_FIND_STATUS_AREA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxST_ELLIPSIZE_END);

  for (const auto & toolInfo : SearchBarToolInfos)
  {
    if (toolInfo.id == ID_FIND_EDITBOX) {
      AddControl(searchCtrl);
    }
    else if (toolInfo.id == ID_FIND_STATUS_AREA) {
      AddSeparator();
      AddControl(statusText);
    }
    else {
      AddTool(
        toolInfo.id,
        toolInfo.GetBitmapForEnabledButton(),
        toolInfo.GetBitmapForDisabledButton(),
        (toolInfo.id == ID_FIND_IGNORE_CASE) || (toolInfo.id == ID_FIND_ADVANCED_OPTIONS) /* toogle element? */,
        nullptr,
        wxGetTranslation(toolInfo.tooltip)
      );
    }
  }

  searchCtrl->Bind(wxEVT_COMMAND_TEXT_UPDATED,          &PasswordSafeSearch::OnSearchTextChanged,     this);
  searchCtrl->Bind(wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, &PasswordSafeSearch::OnDoSearch,              this);
  searchCtrl->Bind(wxEVT_COMMAND_TEXT_ENTER,            &PasswordSafeSearch::OnDoSearch,              this);
  Bind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnAdvancedSearchOptions, this, ID_FIND_ADVANCED_OPTIONS);
  Bind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnSearchClose,           this, ID_FIND_CLOSE);
  Bind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnDoSearch,              this, ID_FIND_NEXT);
  Bind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnSearchClear,           this, ID_FIND_CLEAR);
  Bind(wxEVT_COMMAND_TOOL_CLICKED,                      &PasswordSafeSearch::OnToolBarFindReport,     this, ID_FIND_CREATE_REPORT);
  Bind(wxEVT_CHAR_HOOK,                                 &PasswordSafeSearch::OnChar,                  this);
  m_parentFrame->Bind(wxEVT_SIZE,                       &PasswordSafeSearch::OnSize,                  this);

  CalculateToolsWidth();
  return Realize();
}

/**
 * Event handler (EVT_CHAR_HOOK) that will be called on keystroke events.
 * 
 * The following keystroke events are handled specially.
 * - Escape Key: Hides the search toolbar.
 * - Ctrl-C Key: Copies marked text from search text field or password of selected item.
 * - Ctrl-U Key: Copies the username of selected item.
 * - Ctrl-X Key: Cuts marked text from search text field or clears the search text field.
 * 
 * @param event holds information about key event.
 * @see <a href="https://docs.wxwidgets.org/3.1/classwx_key_event.html">wxKeyEvent Class Reference</a>
 */
void PasswordSafeSearch::OnChar(wxKeyEvent& event)
{
  auto keyName = event.GetUnicodeKey();

  if (keyName == WXK_ESCAPE) {
    HideSearchToolbar();
  }
  else if ((event.GetModifiers() == wxMOD_CONTROL) && 
            wxString("cCuUxX").Contains(keyName)) {

    auto control = wxDynamicCast(FindControl(ID_FIND_EDITBOX), wxSearchCtrl);

    if (control) {
      if (control->CanCopy() || control->CanCut()) {
        // If the user has marked some text in the search text field,
        // then normal copy and cut event shall be handled.
        event.Skip();
      }
      else if (wxString("cC").Contains(keyName)) {
        // If nothing is marked in search text field,
        // the item's password shall be copied.
        wxCommandEvent copy_password_event(wxEVT_MENU, ID_COPYPASSWORD);
        m_parentFrame->GetEventHandler()->AddPendingEvent(copy_password_event);
      }
      else if (wxString("uU").Contains(keyName)) {
        // If nothing is marked in search text field,
        // the item's username shall be copied.
        wxCommandEvent copy_username_event(wxEVT_MENU, ID_COPYUSERNAME);
        m_parentFrame->GetEventHandler()->AddPendingEvent(copy_username_event);
      }
      else if (wxString("xX").Contains(keyName)) {
        // If nothing is marked in search text field,
        // the search text field shall be cleared.
        wxCommandEvent search_clear_event(wxEVT_MENU, ID_FIND_CLEAR);
        GetEventHandler()->AddPendingEvent(search_clear_event);
      }
    }
  }
  else {
    event.Skip();
  }
}

/*!
 * Called when user clicks Find from Edit menu, or presses Ctrl-F
 */
void PasswordSafeSearch::Activate()
{
  static bool IsCreated = false;

  if (!IsCreated) {
    IsCreated = CreateSearchBar();
  }

  UpdateStatusAreaWidth();
}

void PasswordSafeSearch::SetFocusIntoEditField()
{
  FindControl(ID_FIND_EDITBOX)->SetFocus();
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
  else {
    m_currentIndex = m_indices.end();
  }

  return *this;
}

void SearchPointer::PrintLabel(const TCHAR* prefix /*= 0*/)
{
  if (m_indices.empty()) {
    m_label = _("No matches found");
  }
  else if (m_indices.size() == 1) {
    m_label = _("1 match");
  }
  else {
    // need a const object so we get both args to distance() as const iterators
    const SearchIndices& idx = m_indices;
    m_label.Clear();
    m_label << std::distance(idx.begin(), m_currentIndex)+1 << '/' << m_indices.size() << wxT(" matches");
    if (prefix) {
      m_label = wxString(prefix) + wxT(".  ") + m_label;
    }
  }
}
