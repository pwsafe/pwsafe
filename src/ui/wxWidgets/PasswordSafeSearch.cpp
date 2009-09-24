#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PasswordSafeSearch.h"
#include "../../corelib/PwsPlatform.h"

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

void PasswordSafeSearch::OnDoSearch(wxCommandEvent& evt)
{
  wxASSERT(m_toolbar);

  wxControl* ctrl = m_toolbar->FindControl(ID_FIND_EDITBOX);
  if (ctrl) {
    wxTextCtrl* txtCtrl = wxDynamicCast(ctrl, wxTextCtrl);
    if (txtCtrl) {
      wxMessageBox(txtCtrl->GetLineText(0), wxT("Password Safe"));
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
  m_toolbar->AddControl(new wxTextCtrl(m_toolbar, ID_FIND_STATUS_AREA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY));

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
