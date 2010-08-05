#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PasswordSafeSearch.h"
#include "../../corelib/PwsPlatform.h"
#include "../../corelib/PWHistory.h"
#include "../../corelib/Util.h"
#include "passwordsafeframe.h"
#include "wxutils.h"
#include "AdvancedSelectionDlg.h"

////@begin XPM images
#include "../graphics/wxWidgets/find.xpm"
#include "../graphics/wxWidgets/findreport.xpm"
#include "../graphics/wxWidgets/find_disabled.xpm"
#include "../graphics/wxWidgets/findadvanced.xpm"
#include "../graphics/wxWidgets/findcase_i.xpm"
#include "../graphics/wxWidgets/findcase_s.xpm"
#include "../graphics/wxWidgets/findclear.xpm"
#include "../graphics/wxWidgets/findclose.xpm"
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

BEGIN_EVENT_TABLE( PasswordSafeSearch, wxEvtHandler )
////@begin PasswordSafeSearch event table entries
  EVT_TEXT_ENTER( ID_FIND_EDITBOX, PasswordSafeSearch::OnDoSearch )
  EVT_TOOL( ID_FIND_CLOSE, PasswordSafeSearch::OnSearchClose )
  EVT_TOOL( ID_FIND_ADVANCED_OPTIONS, PasswordSafeSearch::OnAdvancedSearchOptions )
  EVT_TOOL( ID_FIND_NEXT, PasswordSafeSearch::OnDoSearch )
////@end PasswordSafeSearch event table entries
END_EVENT_TABLE()


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
    
  if (m_criteria.IsDirty() || txtCtrl->IsModified())  {
      m_searchPointer.Clear();
   
      if (!m_fAdvancedSearch)
        FindMatches(tostringx(searchText), m_toolbar->GetToolState(ID_FIND_IGNORE_CASE), m_searchPointer, begin, end, afn);
      else
        FindMatches(tostringx(searchText), m_toolbar->GetToolState(ID_FIND_IGNORE_CASE), m_searchPointer, 
                      m_criteria.m_bsFields, m_criteria.m_fUseSubgroups, m_criteria.m_subgroupText,
                      m_criteria.SubgroupObject(), m_criteria.SubgroupFunction(), 
                      m_criteria.m_fCaseSensitive, begin, end, afn);

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
};

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_ADVANCED_OPTIONS
 */
void PasswordSafeSearch::OnAdvancedSearchOptions(wxCommandEvent& /* evt */)
{
  m_criteria.Clean();
  AdvancedSelectionDlg<FindDlgType> dlg(m_parentFrame, m_criteria);
  if (dlg.ShowModal() == wxID_OK) {
    m_fAdvancedSearch = true;
    if (m_criteria != dlg.m_criteria)
      m_criteria = dlg.m_criteria;
  }
}

/*!
 * Creates the search bar and keeps it hidden
 */
void PasswordSafeSearch::CreateSearchBar()
{
  wxASSERT(m_toolbar == 0);

  m_toolbar = new wxToolBar(m_parentFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTB_BOTTOM | wxTB_HORIZONTAL,  wxT("SearchBar"));

  m_toolbar->AddTool(ID_FIND_CLOSE, wxT(""), wxBitmap(findclose), wxNullBitmap, wxITEM_NORMAL, wxT("Close Find Bar"));
  wxTextCtrl* edit = new wxTextCtrl(m_toolbar, ID_FIND_EDITBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  m_toolbar->AddControl(edit);
  m_toolbar->AddTool(ID_FIND_NEXT, wxT(""), wxBitmap(find), wxBitmap(find_disabled), wxITEM_NORMAL, wxT("Find Next"));
  m_toolbar->AddCheckTool(ID_FIND_IGNORE_CASE, wxT(""), wxBitmap(findcase_i), wxBitmap(findcase_s), wxT("Case Insensitive Search"));
  m_toolbar->AddTool(ID_FIND_ADVANCED_OPTIONS, wxT(""), wxBitmap(findadvanced), wxNullBitmap, wxITEM_NORMAL, wxT("Advanced Find Options"));
  m_toolbar->AddTool(ID_FIND_CREATE_REPORT, wxT(""), wxBitmap(findreport), wxNullBitmap, wxITEM_NORMAL, wxT("Create report of previous Find search"));
  m_toolbar->AddTool(ID_FIND_CLEAR, wxT(""), wxBitmap(findclear), wxNullBitmap, wxITEM_NORMAL, wxT("Clear Find"));
  m_toolbar->AddControl(new wxStaticText(m_toolbar, ID_FIND_STATUS_AREA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY));
  
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
  edit->PushEventHandler(this);
  m_toolbar->PushEventHandler(this);
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

bool PasswordSafeSearch::ProcessEvent(wxEvent& evt)
{
  if (evt.GetId() == ID_FIND_EDITBOX && (evt.GetEventType() == wxEVT_KEY_DOWN || evt.GetEventType() == wxEVT_CHAR)) {
    wxKeyEvent& keyEvent = dynamic_cast<wxKeyEvent&>(evt);
    if (keyEvent.GetKeyCode() == WXK_ESCAPE) {
      HideSearchToolbar();
      return true;
    }
    else if (keyEvent.GetKeyCode() == WXK_DELETE) {
      //never gets here.  Delete key is translated by a much lower
      //layer like gtk, into the menu accelerator key
      return true;
    }
    else if (keyEvent.GetKeyCode() == WXK_RETURN) {
      return m_toolbar->FindControl(ID_FIND_EDITBOX)->ProcessEvent(evt);
    }
  }
  
  // Handle the EVT_TEXT_ENTER that the editCtrl would have sent us
  // due to our processing of WXK_RETURN above
  if (GetEvtHandlerEnabled()) { 
    //copied from wxWidgets source: event.cpp
    if ( GetEventHashTable().HandleEvent(evt, this) )
      return true;
  }
  
  if (GetNextHandler())
    return GetNextHandler()->ProcessEvent(evt);

  return false;
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
    if (fUseSubgroups && !afn(itr).Matches((const charT*)subgroupText, subgroupObject, fn))
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
        searchPtr.Add(CUUIDGen(uuid));
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
