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

#include <functional>

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
                                                                    m_fAdvancedSearch(false)
{
}

PasswordSafeSearch::~PasswordSafeSearch(void)
{
  delete m_toolbar;
  m_toolbar = 0;
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

  wxTextCtrl* txtCtrl = wxDynamicCast(m_toolbar->FindControl(ID_FIND_EDITBOX), wxTextCtrl);
  wxASSERT(txtCtrl);

  const wxString searchText = txtCtrl->GetLineText(0);
  
  if (searchText.IsEmpty())
    return;
    
  if (m_criteria.IsDirty() || txtCtrl->IsModified() || m_searchPointer.IsEmpty())  {
      m_searchPointer.Clear();
   
      if (!m_fAdvancedSearch)
        FindMatches(tostringx(searchText), m_toolbar->GetToolState(ID_FIND_IGNORE_CASE), m_searchPointer, begin, end, afn);
      else
        FindMatches(tostringx(searchText), m_toolbar->GetToolState(ID_FIND_IGNORE_CASE), m_searchPointer, 
                      m_criteria.GetSelectedFields(), m_criteria.HasSubgroupRestriction(), m_criteria.SubgroupSearchText(),
                      m_criteria.SubgroupObject(), m_criteria.SubgroupFunction(), 
                      m_criteria.CaseSensitive(), begin, end, afn);

      m_criteria.Clean();
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
      editMenu->Insert(FIND_MENU_POSITION, ID_EDITMENU_FIND_NEXT, _("&Find next...\tF3"), _T(""), wxITEM_NORMAL);
      editMenu->Insert(FIND_MENU_POSITION+1, ID_EDITMENU_FIND_PREVIOUS, _("&Find previous...\tSHIFT+F3"), _T(""), wxITEM_NORMAL);
      editMenu->Delete(findItem);
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

void PasswordSafeSearch::HideSearchToolbar()
{
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

    editMenu->Insert(FIND_MENU_POSITION, wxID_FIND, _("&Find Entry...\tCtrl+F"), _T(""), wxITEM_NORMAL);
  }
}

struct FindDlgType {
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced Find Options");
  }
  
  static bool IsMandatoryField(CItemData::FieldType /*field*/) {
    return false;
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
void PasswordSafeSearch::OnAdvancedSearchOptions(wxCommandEvent& /* evt */)
{
  m_criteria.Clean();
  AdvancedSelectionDlg<FindDlgType> dlg(m_parentFrame, m_criteria);
  if (dlg.ShowModal() == wxID_OK) {
    m_fAdvancedSearch = true;
    dlg.GetSelectionCriteria(m_criteria);
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
          { ID_FIND_CLOSE,            findclose_xpm,    NULL,               classic_findclose_xpm,    NULL                      },
          { ID_FIND_NEXT,             find_xpm,         find_disabled_xpm,  classic_find_xpm,         classic_find_disabled_xpm },
          { ID_FIND_IGNORE_CASE,      findcase_i_xpm,   findcase_s_xpm,     classic_findcase_i_xpm,   classic_findcase_s_xpm    },
          { ID_FIND_ADVANCED_OPTIONS, findadvanced_xpm, NULL,               classic_findadvanced_xpm, NULL                      },
          { ID_FIND_CREATE_REPORT,    findreport_xpm,   NULL,               classic_findreport_xpm,   NULL                      },
          { ID_FIND_CLEAR,            findclear_xpm,    NULL,               classic_findclear_xpm,    NULL                      },
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


/*!
 * Creates the search bar and keeps it hidden
 */
void PasswordSafeSearch::CreateSearchBar()
{
  wxASSERT(m_toolbar == 0);

  m_toolbar = new wxToolBar(m_parentFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTB_BOTTOM | wxTB_HORIZONTAL,  wxT("SearchBar"));

  m_toolbar->AddTool(ID_FIND_CLOSE, wxT(""), wxBitmap(findclose_xpm), wxNullBitmap, wxITEM_NORMAL, _("Close Find Bar"));
  wxTextCtrl* edit = new wxTextCtrl(m_toolbar, ID_FIND_EDITBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  m_toolbar->AddControl(edit);
  m_toolbar->AddTool(ID_FIND_NEXT, wxT(""), wxBitmap(find_xpm), wxBitmap(find_disabled_xpm), wxITEM_NORMAL, _("Find Next"));
  m_toolbar->AddCheckTool(ID_FIND_IGNORE_CASE, wxT(""), wxBitmap(findcase_i_xpm), wxBitmap(findcase_s_xpm), _("Case Insensitive Search"));
  m_toolbar->AddTool(ID_FIND_ADVANCED_OPTIONS, wxT(""), wxBitmap(findadvanced_xpm), wxNullBitmap, wxITEM_NORMAL, _("Advanced Find Options"));
  m_toolbar->AddTool(ID_FIND_CREATE_REPORT, wxT(""), wxBitmap(findreport_xpm), wxNullBitmap, wxITEM_NORMAL, _("Create report of previous Find search"));
  m_toolbar->AddTool(ID_FIND_CLEAR, wxT(""), wxBitmap(findclear_xpm), wxNullBitmap, wxITEM_NORMAL, _("Clear Find"));
  m_toolbar->AddControl(new wxStaticText(m_toolbar, ID_FIND_STATUS_AREA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY));
  
  RefreshButtons();
  
  if (!m_toolbar->Realize())
    wxMessageBox(wxT("Could not create Search Bar"), wxT("Password Safe"));

  wxSizer* origSizer = m_parentFrame->GetSizer();
  wxASSERT(origSizer);
  wxASSERT(origSizer->IsKindOf(wxBoxSizer(wxVERTICAL).GetClassInfo()));
  wxASSERT(((wxBoxSizer*)origSizer)->GetOrientation() == wxVERTICAL);
  origSizer->Add(m_toolbar, 0, wxEXPAND | wxALIGN_CENTER);
  origSizer->Layout();
  if (!m_toolbar->Show(true) && !m_toolbar->IsShownOnScreen())
    wxMessageBox(wxT("Could not display searchbar"));
 
  //we need to filter out the ESCAPE key and also receive the toolbar notifications 
  edit->Connect(wxEVT_CHAR, wxCharEventHandler(PasswordSafeSearch::OnSearchBarTextChar), NULL, this);
  edit->Connect(wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler(PasswordSafeSearch::OnDoSearch), NULL, this);
  m_toolbar->Connect(ID_FIND_CLOSE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnSearchClose), NULL, this);
  m_toolbar->Connect(ID_FIND_ADVANCED_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnAdvancedSearchOptions), NULL, this);
  m_toolbar->Connect(ID_FIND_NEXT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(PasswordSafeSearch::OnDoSearch), NULL, this);
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
    if (m_toolbar->Show(true)) {
      m_parentFrame->GetSizer()->Layout();
    }
    else {
      const wxPoint pt = m_toolbar->GetPosition();
      const wxSize sz = m_toolbar->GetSize();
      wxMessageBox(wxString() << wxT("Could not re-display searchbar at ") << pt << wxT(" of size ") << sz
                              << wxT(" because ") << (m_toolbar->IsShownOnScreen()? wxT("its already visible"): wxT(" of an error")));
    }
  }

  wxASSERT(m_toolbar);

  m_toolbar->FindControl(ID_FIND_EDITBOX)->SetFocus();
}


template <class Iter, class Accessor>
void PasswordSafeSearch::FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr, Iter begin, Iter end, Accessor afn)
{
  searchPtr.Clear();
  //As per original Windows code, default search is for all text fields
  CItemData::FieldBits bsFields;
  bsFields.set();

  return FindMatches(searchText, fCaseSensitive, searchPtr, bsFields, false, wxEmptyString, CItemData::END, PWSMatch::MR_INVALID, false, begin, end, afn);
}

bool FindNoCase( const StringX& src, const StringX& dest)
{
    StringX srcLower = src;
    ToLower(srcLower);

    StringX destLower = dest;
    ToLower(destLower);

    return destLower.find(srcLower) != StringX::npos;
}

template <class Iter, class Accessor>
void PasswordSafeSearch::FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr,
                                       const CItemData::FieldBits& bsFields, bool fUseSubgroups, const wxString& subgroupText,
                                       CItemData::FieldType subgroupObject, PWSMatch::MatchRule subgroupFunction, 
                                       bool subgroupFunctionCaseSensitive, Iter begin, Iter end, Accessor afn)
{
  if (searchText.empty())
      return;
  
  searchPtr.Clear();

  typedef StringX (CItemData::*ItemDataFuncT)() const;

  struct {
      CItemData::FieldType type;
      ItemDataFuncT        func;
  } ItemDataFields[] = {  {CItemData::GROUP,     &CItemData::GetGroup},
                          {CItemData::TITLE,     &CItemData::GetTitle},
                          {CItemData::USER,      &CItemData::GetUser},
                          {CItemData::PASSWORD,  &CItemData::GetPassword},
//                        {CItemData::NOTES,     &CItemData::GetNotes},
                          {CItemData::URL,       &CItemData::GetURL},
                          {CItemData::EMAIL,     &CItemData::GetEmail},
                          {CItemData::RUNCMD,    &CItemData::GetRunCommand},
                          {CItemData::AUTOTYPE,  &CItemData::GetAutoType},
                          {CItemData::XTIME_INT, &CItemData::GetXTimeInt},
 
                      };

  for ( Iter itr = begin; itr != end; ++itr) {
    
    const int fn = (subgroupFunctionCaseSensitive? -subgroupFunction: subgroupFunction);
    if (fUseSubgroups && !afn(itr).Matches(static_cast<const charT *>(subgroupText), subgroupObject, fn))
        continue;

    bool found = false;
    for (size_t idx = 0; idx < NumberOf(ItemDataFields) && !found; ++idx) {
      if (bsFields.test(ItemDataFields[idx].type)) {
          const StringX str = (afn(itr).*ItemDataFields[idx].func)();
          found = fCaseSensitive? str.find(searchText) != StringX::npos: FindNoCase(searchText, str);
      }
    }

    if (!found && bsFields.test(CItemData::NOTES)) {
        StringX str = afn(itr).GetNotes();
        found = fCaseSensitive? str.find(searchText) != StringX::npos: FindNoCase(searchText, str);
    }

    if (!found && bsFields.test(CItemData::PWHIST)) {
        size_t pwh_max, err_num;
        PWHistList pwhistlist;
        CreatePWHistoryList(afn(itr).GetPWHistory(), pwh_max, err_num, pwhistlist, TMC_XML);
        for (PWHistList::iterator iter = pwhistlist.begin(); iter != pwhistlist.end(); iter++) {
          PWHistEntry pwshe = *iter;
          found = fCaseSensitive? pwshe.password.find(searchText) != StringX::npos: FindNoCase(searchText, pwshe.password );
          if (found)
            break;  // break out of for loop
        }
        pwhistlist.clear();
    }

    if (found) {
        uuid_array_t uuid;
        afn(itr).GetUUID(uuid);
        searchPtr.Add(pws_os::CUUID(uuid));
    }
  }
}

/////////////////////////////////////////////////
// SearchPointer class definition
SearchPointer& SearchPointer::operator++()
{ //prefix operator, to prevent copying itself
  if (!m_indices.empty()) {
    m_currentIndex++;
    if (m_currentIndex == m_indices.end()) {
      m_currentIndex = m_indices.begin();
      m_label = wxT("Search hit bottom, continuing at top");
    }
    else {
      m_label.Printf(wxT("%d matches found"), m_indices.size());
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
      m_label = wxT("Search hit top, continuing at bottom");
    }
    else {
      m_currentIndex--;
      m_label.Printf(wxT("%d matches found"), m_indices.size());
    }
  }
  else
    m_currentIndex = m_indices.end();

  return *this;
}
