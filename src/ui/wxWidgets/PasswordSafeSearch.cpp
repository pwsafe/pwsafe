#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PasswordSafeSearch.h"
#include "../../corelib/PwsPlatform.h"
#include "../../corelib/ItemData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////@begin XPM images
#include "../graphics/find.xpm"
#include "../graphics/findreport.xpm"
#include "../graphics/find_disabled.xpm"
#include "../graphics/findadvanced.xpm"
#include "../graphics/findcase_i.xpm"
#include "../graphics/findcase_s.xpm"
#include "../graphics/findclear.xpm"
#include "../graphics/findclose.xpm"
////@end XPM images
#include <wx/statline.h> 
#include <wx/valgen.h>

/*!
 * PasswordSafeSearchData class declaration
 */

class PasswordSafeSearchData 
{
  
  PasswordSafeSearchData(const PasswordSafeSearchData&);

public:
  PasswordSafeSearchData():  m_fCaseSensitive(false),
                             m_subgroupObject(0),
                             m_fUseSubgroups(false),
                             m_subgroupFunction(0)
  {}

  bool                  m_fCaseSensitive;
  wxString              m_searchText;
  CItemData::FieldBits  m_bsFields;
  wxString              m_subgroupName;
  bool                  m_fUseSubgroups;
  int                   m_subgroupObject;
  int                   m_subgroupFunction;
};


const charT* subgroupNames[] = { wxT("Group"), wxT("Group/Title"), wxT("Notes"), wxT("Title"), wxT("URL"), wxT("User Name") } ;

const charT* subgroupFunctionNames[] = { wxT("equals"), wxT("does not equal"), wxT("begins with"), wxT("does not begin with"), 
                                         wxT("ends with"), wxT("does not end with"), wxT("contains"), wxT("does not contain") } ;

struct {
    const charT* fieldName;
    CItemData::FieldType fieldType;
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
class AdvancedSearchOptionsDlg: public wxDialog
{
  DECLARE_CLASS(AdvancedSearchOptionsDlg)
  DECLARE_EVENT_TABLE()

  DECLARE_NO_COPY_CLASS(AdvancedSearchOptionsDlg);

  PasswordSafeSearchContext& m_context;

  enum {ID_SELECT_SOME = 101, ID_SELECT_ALL, ID_REMOVE_SOME, ID_REMOVE_ALL };

public:
  AdvancedSearchOptionsDlg(wxWindow* wnd, PasswordSafeSearchContext& context);
  ~AdvancedSearchOptionsDlg() {
    if (!m_context.IsSame(m_searchData))
        m_context.Set(m_searchData);
  }

  void OnOk( wxCommandEvent& event );

private:
  void CreateControls(wxWindow* parentWnd);
  PasswordSafeSearchData m_searchData;
};

IMPLEMENT_CLASS( AdvancedSearchOptionsDlg, wxDialog )

BEGIN_EVENT_TABLE( AdvancedSearchOptionsDlg, wxDialog )
  EVT_BUTTON( wxID_OK, AdvancedSearchOptionsDlg::OnOk )
END_EVENT_TABLE()

AdvancedSearchOptionsDlg::AdvancedSearchOptionsDlg(wxWindow* parentWnd, 
                                                   PasswordSafeSearchContext& context): m_context(context)
{
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
  CreateControls(parentWnd);
  m_searchData = m_context.Get();
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
    for (size_t idx = 0 ; idx < NumberOf(subgroupNames); ++idx) comboSubgroup->AppendString(subgroupNames[idx]);
    comboSubgroup->SetValidator(wxGenericValidator(&m_searchData.m_subgroupObject));
    hbox->Add(comboSubgroup, wxSizerFlags(1).Expand());
    
    hbox->AddSpacer(20);
    
    wxComboBox* comboFunctions = new wxComboBox(panel, wxID_ANY);
    for( size_t idx = 0; idx < NumberOf(subgroupFunctionNames); ++idx) comboFunctions->AppendString(subgroupFunctionNames[idx]);
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
    wxListBox* lbFields = new wxListBox(panel, wxID_ANY, wxDefaultPosition, wxSize(-1, 200));
    for (size_t idx = 0; idx < NumberOf(fieldNames); ++idx)
        if (!m_searchData.m_bsFields.test(fieldNames[idx].fieldType))
            lbFields->AppendString(fieldNames[idx].fieldName);

    vbox1->Add(lbFields, wxSizerFlags(1).Expand());
    hbox->Add(vbox1);

    hbox->AddSpacer(15);

    wxBoxSizer* buttonBox = new wxBoxSizer(wxVERTICAL);
    buttonBox->Add( new wxButton(panel, ID_SELECT_SOME, wxT(">")) );
    buttonBox->AddSpacer(5);
    buttonBox->Add( new wxButton(panel, ID_SELECT_ALL, wxT(">>")) );
    buttonBox->AddSpacer(30);
    buttonBox->Add( new wxButton(panel, wxID_ANY, wxT("<")) );
    buttonBox->Add( new wxButton(panel, wxID_ANY, wxT("<<")) );
    buttonBox->AddSpacer(5);
    hbox->Add(buttonBox, wxSizerFlags().Center());

    hbox->AddSpacer(15);

    wxBoxSizer* vbox2 = new wxBoxSizer(wxVERTICAL);
    vbox2->Add(new wxStaticText(panel, wxID_ANY, wxT("&Selected Fields:")));
    vbox2->AddSpacer(10);
    wxListBox* lbSelectedFields = new wxListBox(panel, wxID_ANY, wxDefaultPosition, wxSize(-1, 200));
    for (size_t idx=0; idx < NumberOf(fieldNames); ++idx)
        if (m_searchData.m_bsFields.test(fieldNames[idx].fieldType))
            lbSelectedFields->AppendString(fieldNames[idx].fieldName);

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
  if (!m_context.IsSame(m_searchData))
      m_context.Set(m_searchData);

  //Let wxDialog handle it as well
  evt.Skip(true);
}


////////////////////////////////////////////////////////////////////////////
// PasswordSafeSerachContext implementation
PasswordSafeSearchContext::PasswordSafeSearchContext(): m_searchData(new PasswordSafeSearchData), 
                                                        m_fDirty(false)
{}

PasswordSafeSearchContext::~PasswordSafeSearchContext()
{
  delete m_searchData;
  m_searchData = 0;
}

inline bool PasswordSafeSearchContext::IsSame(const PasswordSafeSearchData& data) const 
{ 
    return data.m_bsFields == m_searchData->m_bsFields && 
           data.m_fCaseSensitive == m_searchData->m_fCaseSensitive &&
           data.m_fUseSubgroups == m_searchData->m_fUseSubgroups &&
           data.m_searchText == m_searchData->m_searchText &&
           data.m_subgroupFunction == m_searchData->m_subgroupFunction &&
           data.m_subgroupName == m_searchData->m_subgroupName &&
           data.m_subgroupObject == m_searchData->m_subgroupObject; 
}

inline void PasswordSafeSearchContext::Set(const PasswordSafeSearchData& data) 
{ 
    *m_searchData = data; 
    m_fDirty = true;
}

inline const PasswordSafeSearchData& PasswordSafeSearchContext::Get(void) const 
{ 
    return *m_searchData; 
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


PasswordSafeSearch::PasswordSafeSearch(wxFrame* parent) : m_toolbar(0), m_parentFrame(parent)
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
  wxASSERT(m_toolbar);

  wxControl* ctrl = m_toolbar->FindControl(ID_FIND_EDITBOX);
  if (ctrl) {
    wxTextCtrl* txtCtrl = wxDynamicCast(ctrl, wxTextCtrl);
    if (txtCtrl) {
      wxString searchText = txtCtrl->GetLineText(0);
      if (m_searchContext->m_searchText != searchText)
          m_searchContext->m_searchText = searchText;

      wxMessageBox(wxString(wxT("Will search for ")) + m_searchContext->m_searchText, wxT("Search clicked"));
    }
  }
}


/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_CLOSE
 */

void PasswordSafeSearch::OnSearchClose(wxCommandEvent& evt)
{
  m_parentFrame->SetToolBar(NULL);
  m_toolbar->Show(false);
}

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_ADVANCED_OPTIONS
 */
void PasswordSafeSearch::OnAdvancedSearchOptions(wxCommandEvent& evt)
{
  AdvancedSearchOptionsDlg dlg(m_parentFrame, m_searchContext);
  dlg.ShowModal();
}

/*!
 * wxEVT_COMMAND_TOOL_CLICKED event handler for ID_FIND_IGNORE_CASE
 */
void PasswordSafeSearch::OnToggleCaseSensitivity(wxCommandEvent& evt)
{
    m_searchContext->m_fCaseSensitive = !m_searchContext->m_fCaseSensitive;
}


/*!
 * Creates the search bar and keeps it hidden
 */
void PasswordSafeSearch::CreateSearchBar()
{
  wxASSERT(m_toolbar == 0);

  m_toolbar = m_parentFrame->CreateToolBar(wxBORDER_NONE | wxTB_BOTTOM | wxTB_HORIZONTAL, wxID_ANY, wxT("SearchBar"));

  m_toolbar->AddTool(ID_FIND_CLOSE, wxT(""), wxBitmap(findclose), wxNullBitmap, wxITEM_NORMAL, wxT("Close Find Bar"));
  m_toolbar->AddControl(new wxTextCtrl(m_toolbar, ID_FIND_EDITBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER));
  m_toolbar->AddTool(ID_FIND_NEXT, wxT(""), wxBitmap(find), wxBitmap(find_disabled), wxITEM_NORMAL, wxT("Find Next"));
  m_toolbar->AddCheckTool(ID_FIND_IGNORE_CASE, wxT(""), wxBitmap(findcase_i), wxBitmap(findcase_s), wxT("Case Insensitive Search"));
  m_toolbar->AddTool(ID_FIND_ADVANCED_OPTIONS, wxT(""), wxBitmap(findadvanced), wxNullBitmap, wxITEM_NORMAL, wxT("Advanced Find Options"));
  m_toolbar->AddTool(ID_FIND_CREATE_REPORT, wxT(""), wxBitmap(findreport), wxNullBitmap, wxITEM_NORMAL, wxT("Create report of previous Find search"));
  m_toolbar->AddTool(ID_FIND_CLEAR, wxT(""), wxBitmap(findclear), wxNullBitmap, wxITEM_NORMAL, wxT("Clear Find"));
  m_toolbar->AddControl(new wxStaticText(m_toolbar, ID_FIND_STATUS_AREA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY));

  if (!m_toolbar->Realize())
    wxMessageBox(wxT("SearcBar::Realize failed"), wxT("Password Safe"));
 
  m_toolbar->PushEventHandler(this);
}

void PasswordSafeSearch::Activate(void)
{
  if (!m_toolbar)
    CreateSearchBar();
  else {
    m_parentFrame->SetToolBar(m_toolbar);
    m_toolbar->Show(true);
  }

  wxASSERT(m_toolbar);

  m_toolbar->FindControl(ID_FIND_EDITBOX)->SetFocus();
}
