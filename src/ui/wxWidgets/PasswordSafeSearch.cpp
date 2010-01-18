#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PasswordSafeSearch.h"
#include "../../corelib/PwsPlatform.h"
#include "../../corelib/PWHistory.h"
#include "../../corelib/Util.h"
#include "passwordsafeframe.h"
#include "wxutils.h"

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

struct _subgroups {
  const charT* name;
  CItemData::FieldType type;
} subgroups[] = { {wxT("Group"),       CItemData::GROUP},
                  {wxT("Group/Title"), CItemData::GROUPTITLE},
                  {wxT("Notes"),       CItemData::NOTES},
                  {wxT("Title"),       CItemData::TITLE},
                  {wxT("URL"),         CItemData::URL},
                  {wxT("User Name"),   CItemData::USER} } ;

struct _subgroupFunctions {
  const charT* name;
  PWSMatch::MatchRule function;
} subgroupFunctions[] = { {wxT("equals"),              PWSMatch::MR_EQUALS},
                          {wxT("does not equal"),      PWSMatch::MR_NOTEQUAL},
                          {wxT("begins with"),         PWSMatch::MR_BEGINS},
                          {wxT("does not begin with"), PWSMatch::MR_NOTBEGIN},
                          {wxT("ends with"),           PWSMatch::MR_ENDS},
                          {wxT("does not end with"),   PWSMatch::MR_NOTEND},
                          {wxT("contains"),            PWSMatch::MR_CONTAINS},
                          {wxT("does not contain"),    PWSMatch::MR_NOTCONTAIN} } ;

struct _fieldNames {
    const charT* name;
    CItemData::FieldType type;
} fieldNames[] = {  {wxT("Group"),              CItemData::GROUP},
                    {wxT("Title"),              CItemData::TITLE},
                    {wxT("User Name"),          CItemData::USER},
                    {wxT("Notes"),              CItemData::NOTES},
                    {wxT("Password"),           CItemData::PASSWORD},
                    {wxT("URL"),                CItemData::URL},
                    {wxT("Autotype"),           CItemData::AUTOTYPE},
                    {wxT("Password History"),   CItemData::PWHIST},
                    {wxT("Run Command"),        CItemData::RUNCMD},
                    {wxT("Email"),              CItemData::EMAIL}
                 };


////////////////////////////////////////////////////////////////////////////
// AdvancedSearchOptionsDlg implementation

IMPLEMENT_CLASS( AdvancedSearchOptionsDlg, wxDialog )

BEGIN_EVENT_TABLE( AdvancedSearchOptionsDlg, wxDialog )
  EVT_BUTTON( wxID_OK, AdvancedSearchOptionsDlg::OnOk )
  EVT_BUTTON( ID_SELECT_SOME, AdvancedSearchOptionsDlg::OnSelectSome )
  EVT_BUTTON( ID_SELECT_ALL, AdvancedSearchOptionsDlg::OnSelectAll )
  EVT_BUTTON( ID_REMOVE_SOME, AdvancedSearchOptionsDlg::OnRemoveSome )
  EVT_BUTTON( ID_REMOVE_ALL, AdvancedSearchOptionsDlg::OnRemoveAll )
END_EVENT_TABLE()

AdvancedSearchOptionsDlg::AdvancedSearchOptionsDlg(wxWindow* parentWnd, 
                                                   PasswordSafeSearchContext& context): m_context(context)
{
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
  m_searchData = m_context.Get();
  CreateControls(parentWnd);
}

void AdvancedSearchOptionsDlg::CreateControls(wxWindow* parentWnd)
{
  wxDialog::Create(parentWnd, wxID_ANY, wxT("Advanced Find Options"));
  
  wxPanel* panel = new wxPanel(this);
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  //Subset entries
  {
    wxStaticBoxSizer* sizer = new wxStaticBoxSizer(wxVERTICAL, panel);

    sizer->AddSpacer(5);
    wxCheckBox* check = new wxCheckBox(panel, wxID_ANY, wxT("&Restrict to a subset of entries:"));
    check->SetValidator(wxGenericValidator(&m_searchData.m_fUseSubgroups));
    sizer->Add(check);
    sizer->Add(10, 10);

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(panel, wxID_ANY, wxT("&Where")), wxSizerFlags(0));
    hbox->AddSpacer(20);
    
    wxComboBox* comboSubgroup = new wxComboBox(panel, wxID_ANY);
    for (size_t idx = 0 ; idx < NumberOf(subgroups); ++idx) comboSubgroup->AppendString(subgroups[idx].name);
    comboSubgroup->SetValidator(wxGenericValidator(&m_searchData.m_subgroupObject));
    hbox->Add(comboSubgroup, wxSizerFlags(1).Expand());
    
    hbox->AddSpacer(20);
    
    wxComboBox* comboFunctions = new wxComboBox(panel, wxID_ANY);
    for( size_t idx = 0; idx < NumberOf(subgroupFunctions); ++idx) comboFunctions->AppendString(subgroupFunctions[idx].name);
    comboFunctions->SetValidator(wxGenericValidator(&m_searchData.m_subgroupFunction));
    hbox->Add(comboFunctions, wxSizerFlags(1).Expand());
    
    sizer->Add(hbox, wxSizerFlags(1).Border(wxLEFT|wxRIGHT, 15).Expand());

    sizer->AddSpacer(5);

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add( new wxStaticText(panel, wxID_ANY, wxT("the &following text:")) );
    vbox->Add(0, 3);
    wxBoxSizer* hsizer = new wxBoxSizer(wxHORIZONTAL);

    wxTextCtrl* txtCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1));
    txtCtrl->SetValidator(wxGenericValidator(&m_searchData.m_searchText));
    hsizer->Add(txtCtrl, wxSizerFlags(1).Expand().FixedMinSize());

    vbox->Add(hsizer);
    vbox->Add(0, 3);
    wxCheckBox* checkCaseSensitivity = new wxCheckBox(panel, wxID_ANY, wxT("&Case Sensitive"));
    checkCaseSensitivity->SetValidator(wxGenericValidator(&m_searchData.m_fCaseSensitive));
    vbox->Add( checkCaseSensitivity, 1, wxEXPAND );
    
    sizer->Add(vbox, wxSizerFlags().Border(wxLEFT, 15));

    dlgSizer->Add(sizer, wxSizerFlags(0).Border(wxALL, 10).Center());
  }

  {
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);
    vbox1->Add(new wxStaticText(panel, wxID_ANY, wxT("&Available Fields:")));
    vbox1->AddSpacer(10);
    wxListBox* lbFields = new wxListBox(panel, ID_LB_AVAILABLE_FIELDS, wxDefaultPosition, 
              wxSize(100, 200), 0, NULL, wxLB_EXTENDED);
    for (size_t idx = 0; idx < NumberOf(fieldNames); ++idx)
        if (!m_searchData.m_bsFields.test(fieldNames[idx].type))
            lbFields->Append(fieldNames[idx].name, (void*)(idx));

    vbox1->Add(lbFields, wxSizerFlags(1).Expand());
    hbox->Add(vbox1);

    hbox->AddSpacer(15);

    wxBoxSizer* buttonBox = new wxBoxSizer(wxVERTICAL);
    buttonBox->Add( new wxButton(panel, ID_SELECT_SOME, wxT(">")) );
    buttonBox->AddSpacer(5);
    buttonBox->Add( new wxButton(panel, ID_SELECT_ALL, wxT(">>")) );
    buttonBox->AddSpacer(30);
    buttonBox->Add( new wxButton(panel, ID_REMOVE_SOME, wxT("<")) );
    buttonBox->Add( new wxButton(panel, ID_REMOVE_ALL, wxT("<<")) );
    buttonBox->AddSpacer(5);
    hbox->Add(buttonBox, wxSizerFlags().Center());

    hbox->AddSpacer(15);

    wxBoxSizer* vbox2 = new wxBoxSizer(wxVERTICAL);
    vbox2->Add(new wxStaticText(panel, wxID_ANY, wxT("&Selected Fields:")));
    vbox2->AddSpacer(10);
    wxListBox* lbSelectedFields = new wxListBox(panel, ID_LB_SELECTED_FIELDS, wxDefaultPosition, 
                  wxSize(100, 200), 0, NULL, wxLB_EXTENDED);
    for (size_t idx=0; idx < NumberOf(fieldNames); ++idx)
        if (m_searchData.m_bsFields.test(fieldNames[idx].type))
            lbSelectedFields->Append(fieldNames[idx].name, (void*)(idx));

    vbox2->Add(lbSelectedFields, wxSizerFlags(1).Expand());
    hbox->Add(vbox2);

    dlgSizer->Add(hbox, wxSizerFlags(1).Expand().Border(wxALL, 10));
  }
  
  dlgSizer->Add( new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, wxSize(300, -1)), wxSizerFlags().Border(wxALL, 10).Center() );

//  why doesn't this work?
//  dlgSizer->Add( CreateButtonSizer(wxOK | wxCANCEL | wxHELP) );
  {
      wxBoxSizer* bbox = new wxBoxSizer(wxHORIZONTAL);
      bbox->Add(new wxButton(panel, wxID_OK, wxT("&Ok")));
      bbox->AddSpacer(20);
      bbox->Add(new wxButton(panel, wxID_CANCEL, wxT("&Cancel")));
      bbox->AddSpacer(20);
      bbox->Add(new wxButton(panel, wxID_HELP, wxT("&Help")));

      dlgSizer->Add(bbox, wxSizerFlags().Center().Border(wxBOTTOM, 10));
  }

  panel->SetSizer(dlgSizer);
  dlgSizer->Fit(this);
  dlgSizer->SetSizeHints(this);

}

void AdvancedSearchOptionsDlg::OnOk( wxCommandEvent& evt )
{
  TransferDataFromWindow();

  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  wxASSERT(lbSelected);

  //reset the selected field bits 
  m_searchData.m_bsFields.reset();
  const size_t count = lbSelected->GetCount();
  
  for (size_t idx = 0; idx < count; ++idx) {
      const size_t which = (size_t)lbSelected->GetClientData((unsigned int)idx);
      m_searchData.m_bsFields.set(fieldNames[which].type, true);
  }

  if (!m_context.IsSame(m_searchData))
      m_context.Set(m_searchData);

  //Let wxDialog handle it as well, to close the window
  evt.Skip(true);
}

void AdvancedSearchOptionsDlg::OnSelectSome( wxCommandEvent& evt )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  wxArrayInt aSelected;
  if (lbAvailable->GetSelections(aSelected)) {
    for (size_t idx = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = (size_t)lbAvailable->GetClientData((unsigned int)(aSelected[idx] - idx));
      wxASSERT(which < NumberOf(fieldNames));
      lbAvailable->Delete((unsigned int)(aSelected[idx] - idx));
      lbSelected->Append(fieldNames[which].name, (void *)which);
    }
  }
}

void AdvancedSearchOptionsDlg::OnSelectAll( wxCommandEvent& evt )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  while (lbAvailable->GetCount()) {
      size_t which = (size_t)lbAvailable->GetClientData(0);
      lbAvailable->Delete(0);
      lbSelected->Append(fieldNames[which].name, (void*)which);
  }
}

void AdvancedSearchOptionsDlg::OnRemoveSome( wxCommandEvent& evt )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  wxArrayInt aSelected;
  if (lbSelected->GetSelections(aSelected)) {
    for (size_t idx = 0; idx < aSelected.GetCount(); ++idx) {
      size_t which = (size_t)lbSelected->GetClientData((unsigned int)(aSelected[idx] - idx));
      wxASSERT(which < NumberOf(fieldNames));
      lbSelected->Delete((unsigned int)(aSelected[idx] - idx));
      lbAvailable->Append(fieldNames[which].name, (void *)which);
    }
  }
}

void AdvancedSearchOptionsDlg::OnRemoveAll( wxCommandEvent& evt )
{
  wxListBox* lbAvailable = wxDynamicCast(FindWindow(ID_LB_AVAILABLE_FIELDS), wxListBox);
  wxListBox* lbSelected  = wxDynamicCast(FindWindow(ID_LB_SELECTED_FIELDS), wxListBox);
  
  wxASSERT(lbAvailable);
  wxASSERT(lbSelected);

  while (lbSelected->GetCount()) {
      size_t which = (size_t)lbSelected->GetClientData(0);
      lbSelected->Delete(0);
      lbAvailable->Append(fieldNames[which].name, (void*)which);
  }
}


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
  EVT_TOOL( ID_FIND_IGNORE_CASE, PasswordSafeSearch::OnToggleCaseSensitivity )
  EVT_TOOL( ID_FIND_NEXT, PasswordSafeSearch::OnDoSearch )
////@end PasswordSafeSearch event table entries
END_EVENT_TABLE()


PasswordSafeSearch::PasswordSafeSearch(PasswordSafeFrame* parent) : m_toolbar(0), m_parentFrame(parent), m_fAdvancedSearch(false)
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

  wxString searchText = txtCtrl->GetLineText(0);
  if (m_searchContext->m_searchText != searchText)
      m_searchContext.SetSearchText(searchText);

  if (m_searchContext.IsDirty())  {
      m_searchPointer.Clear();
   
      if (!m_fAdvancedSearch)
        FindMatches(StringX(m_searchContext->m_searchText), m_searchContext->m_fCaseSensitive, m_searchPointer, begin, end, afn);
      else
        FindMatches(StringX(m_searchContext->m_searchText), m_searchContext->m_fCaseSensitive, m_searchPointer, 
                      m_searchContext->m_bsFields, m_searchContext->m_fUseSubgroups, m_searchContext->m_subgroupText,
                      subgroups[m_searchContext->m_subgroupObject].type, subgroupFunctions[m_searchContext->m_subgroupFunction].function, begin, end, afn);

      m_searchContext.Reset();
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

void PasswordSafeSearch::OnSearchClose(wxCommandEvent& evt)
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

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_ADVANCED_OPTIONS
 */
void PasswordSafeSearch::OnAdvancedSearchOptions(wxCommandEvent& evt)
{
  m_searchContext.Reset();
  AdvancedSearchOptionsDlg dlg(m_parentFrame, m_searchContext);
  m_fAdvancedSearch = (dlg.ShowModal() == wxID_OK);
}

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_IGNORE_CASE
 */
void PasswordSafeSearch::OnToggleCaseSensitivity(wxCommandEvent& evt)
{
    m_searchContext.SetCaseSensitivity(!m_searchContext->m_fCaseSensitive);
}


/*!
 * Creates the search bar and keeps it hidden
 */
void PasswordSafeSearch::CreateSearchBar()
{
  wxASSERT(m_toolbar == 0);

  m_toolbar = new wxToolBar(m_parentFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTB_BOTTOM | wxTB_HORIZONTAL,  wxT("SearchBar"));

  m_toolbar->AddTool(ID_FIND_CLOSE, wxT(""), wxBitmap(findclose), wxNullBitmap, wxITEM_NORMAL, wxT("Close Find Bar"));
  m_toolbar->AddControl(new wxTextCtrl(m_toolbar, ID_FIND_EDITBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER));
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

template <class Iter, class Accessor>
void PasswordSafeSearch::FindMatches(const StringX& searchText, bool fCaseSensitive, SearchPointer& searchPtr, Iter begin, Iter end, Accessor afn)
{
  searchPtr.Clear();
  //As per original Windows code, default search is for all text fields
  CItemData::FieldBits bsFields;
  bsFields.set();

  return FindMatches(searchText, fCaseSensitive, searchPtr, bsFields, false, wxEmptyString, CItemData::END, PWSMatch::MR_INVALID, begin, end, afn);
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
                                       CItemData::FieldType subgroupObject, PWSMatch::MatchRule subgroupFunction, Iter begin, Iter end, Accessor afn)
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
    
    if (fUseSubgroups && afn(itr).Matches((const charT*)subgroupText, subgroupObject, subgroupFunction))
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
