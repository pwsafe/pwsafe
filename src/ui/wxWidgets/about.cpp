/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file about.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/url.h>

////@begin includes
////@end includes

#include "about.h"
#include "version.h"
#include "passwordsafeframe.h"
#include "core/CheckVersion.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
#include "graphics/cpane.xpm"
////@end XPM images

/*!
 * CAbout type definition
 */

IMPLEMENT_CLASS( CAbout, wxDialog )

/*!
 * CAbout event table definition
 */

BEGIN_EVENT_TABLE( CAbout, wxDialog )

#if wxCHECK_VERSION(2,9,2)
  EVT_BUTTON( ID_CHECKNEW, CAbout::OnCheckNewClicked )
#else
  EVT_HYPERLINK( ID_CHECKNEW, CAbout::OnCheckNewClicked )
#endif
  EVT_HYPERLINK( ID_SITEHYPERLINK, CAbout::OnVisitSiteClicked )
  EVT_BUTTON( wxID_CLOSE, CAbout::OnCloseClick )

END_EVENT_TABLE()

/*!
 * CAbout constructors
 */

CAbout::CAbout()
{
  Init();
}

CAbout::CAbout( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}

/*!
 * CAbout creator
 */

bool CAbout::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
    // currently (wx 3.0.2 GTK+) after SetSizeHints() style flags are ignored
    // and maximize/minimize buttons reappear, so we need to force max size
    // to remove maximize and minimize buttons
    if (! (style & wxMAXIMIZE_BOX)) {
      SetMaxSize(GetSize());
    }
  }
  Centre();
  return true;
}

/*!
 * CAbout destructor
 */

CAbout::~CAbout()
{
////@begin CAbout destruction
////@end CAbout destruction
}

/*!
 * Member initialization
 */

void CAbout::Init()
{
  m_newVerStatus = nullptr;
}

/*!
 * Control creation for CAbout
 */

void CAbout::CreateControls()
{
  CAbout* aboutDialog = this;

  wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
  aboutDialog->SetSizer(mainSizer);

  wxStaticBitmap* logoBitmap = new wxStaticBitmap(aboutDialog, wxID_STATIC, aboutDialog->GetBitmapResource(L"graphics/cpane.xpm"), wxDefaultPosition, wxDefaultSize, 0);
  mainSizer->Add(logoBitmap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(rightSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* versionStaticText = new wxStaticText(aboutDialog, wxID_VERSIONSTR, _("Password Safe")+wxT(" vx.yy (abcd)"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  rightSizer->Add(versionStaticText, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* buildStaticText = new wxStaticText(aboutDialog, wxID_STATIC, _("Build date:")+wxT(" Mon dd yyyy hh:mm:ss"), wxDefaultPosition, wxDefaultSize, 0);
  rightSizer->Add(buildStaticText, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* verCheckSizer = new wxBoxSizer(wxHORIZONTAL);
  rightSizer->Add(verCheckSizer, 0, wxALIGN_LEFT|wxALL, 0);

  wxStaticText* latestStaticTextBegin = new wxStaticText(aboutDialog, wxID_STATIC, _("Latest version? Click"), wxDefaultPosition, wxDefaultSize, 0 );
  verCheckSizer->Add(latestStaticTextBegin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#if wxCHECK_VERSION(2,9,2)
  // using simple button to prevent Gtk-WARNING and other link processing overhead
  wxButton* latestCheckButton = new wxButton(aboutDialog, ID_CHECKNEW, _("here"), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE|wxBU_EXACTFIT);
  wxString markup = wxString(L"<span color='blue'><u>") + _("here") + L"</u></span>";
  latestCheckButton->SetLabelMarkup(markup);
#else
  wxHyperlinkCtrl* latestCheckButton = new wxHyperlinkCtrl(aboutDialog, ID_CHECKNEW, _("here"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxHL_ALIGN_LEFT);
  // Force empty URL, because wxHyperlinkCtrl constructor set URL to label if ti's empty
  // This doesn't prevent "Gtk-WARNING **: Unable to show ", but at lease we don't try to open label text
  latestCheckButton->SetURL(wxEmptyString);
#endif
  verCheckSizer->Add(latestCheckButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* latestStaticTextEnd = new wxStaticText(aboutDialog, wxID_STATIC, _("to check."), wxDefaultPosition, wxDefaultSize, 0);
  verCheckSizer->Add(latestStaticTextEnd, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* visitSiteSizer = new wxBoxSizer(wxHORIZONTAL);
  rightSizer->Add(visitSiteSizer, 0, wxALIGN_LEFT|wxALL, 0);

  wxStaticText* visitSiteStaticTextBegin = new wxStaticText(aboutDialog, wxID_STATIC, _("Please visit the "), wxDefaultPosition, wxDefaultSize, 0);
  visitSiteSizer->Add(visitSiteStaticTextBegin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxHyperlinkCtrl* visitSiteHyperlinkCtrl = new wxHyperlinkCtrl(aboutDialog, ID_SITEHYPERLINK, _("PasswordSafe website"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  visitSiteSizer->Add(visitSiteHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* visitSiteStaticTextEnd = new wxStaticText(aboutDialog, wxID_STATIC, _("See LICENSE for open source details."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  rightSizer->Add(visitSiteStaticTextEnd, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* copyrightStaticText = new wxStaticText(aboutDialog, wxID_STATIC, _("Copyright (c) 2003-2018 by Rony Shapiro"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  rightSizer->Add(copyrightStaticText, 0, wxALIGN_LEFT|wxALL, 5);

  m_newVerStatus = new wxTextCtrl(aboutDialog, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxNO_BORDER); //wxSize(aboutDialog->ConvertDialogToPixels(wxSize(120, -1)).x, -1)
  m_newVerStatus->SetBackgroundColour(wxColour(230, 231, 232));
  rightSizer->Add(m_newVerStatus, 0, wxALIGN_LEFT|wxALL|wxEXPAND|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
  m_newVerStatus->Hide();

  wxButton* closeButton = new wxButton(aboutDialog, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0);
  rightSizer->Add(closeButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  const wxString vstring = pwsafeAppName + L" " + pwsafeVersionString;
  versionStaticText->SetLabel(vstring);
  const wxString dstring = _("Build date:") + wxT(" ") + wxT(__DATE__) + wxT(" ") + wxT(__TIME__);
  buildStaticText->SetLabel(dstring);
}

/*!
 * Should we show tooltips?
 */

bool CAbout::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAbout::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
  if (name == L"graphics/cpane.xpm")
  {
    wxBitmap bitmap(cpane_xpm);
    return bitmap;
  }
  return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CAbout::GetIconResource( const wxString& WXUNUSED(name) )
{
  // Icon retrieval
////@begin CAbout icon retrieval
  return wxNullIcon;
////@end CAbout icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void CAbout::OnCloseClick( wxCommandEvent& WXUNUSED(event) )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in CAbout.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CLOSE);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in CAbout.
}

/*!
 * core routine "CheckVersion::CheckLatestVersion" checks if there is a later version
 * see details in that routine as to format of the downloaded xml file
 */

/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCHECK
 */

void CAbout::CheckNewVersion()
{
  // Get the latest.xml file from our site, compare to version,
  // and notify the user
  // First, make sure database is closed: Sensitive data with an
  // open socket makes me uneasy...
  PasswordSafeFrame *pFrm = static_cast<PasswordSafeFrame *>(GetParent());

  if (pFrm->GetNumEntries() != 0) {
    const wxString cs_txt(_("For security, the database must be closed before connecting to the Internet.\nPress OK to close database and continue (Changes will be saved)"));
    const wxString cs_title(_("Confirm Close Dialog"));
    wxMessageDialog dlg(this, cs_txt, cs_title,
                        (wxICON_QUESTION | wxOK | wxCANCEL));
    int rc = dlg.ShowModal();
    if (rc == wxID_CANCEL)
      return; // no hard feelings
    // Close database, prompt for save if changed
    wxCommandEvent closeEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_CLOSE);
#if wxCHECK_VERSION(2,9,0)
    pFrm->GetEventHandler()->ProcessEvent(closeEvent);
#else
    pFrm->ProcessEvent(closeEvent);
#endif
    // User could have cancelled save, need to check if really closed:
    if (pFrm->GetNumEntries() != 0)
      return;
  }
  pFrm->Update(); // show user that we closed database
  ASSERT(pFrm->GetNumEntries() == 0);
  // safe to open external connection
  m_newVerStatus->Clear();
  *m_newVerStatus << _("Trying to contact server...");
  m_newVerStatus->Show();
  wxURL url(L"https://pwsafe.org/latest.xml");
  CheckVersion::CheckStatus status = CheckVersion::CheckStatus::UP2DATE;
  stringT latest_xml;
  if (!url.IsOk()) {
    wxURLError err = url.GetError();
    pws_os::Trace(wxT("Err:%d\n"),err);
    status = CheckVersion::CheckStatus::CANT_READ;
  }
  wxInputStream *in_stream = url.GetInputStream();
  if (in_stream != nullptr) {
    unsigned char buff[BUFSIZ+1];
    StringX chunk;
    CUTF8Conv conv;
    do {
      in_stream->Read(buff, BUFSIZ);
      size_t nRead = in_stream->LastRead();
      if (nRead != 0) {
        buff[nRead] = '\0';
        // change to widechar representation
        if (!conv.FromUTF8(buff, nRead, chunk)) {
          delete in_stream;
          in_stream = 0;
          status = CheckVersion::CheckStatus::CANT_READ;
          break;
        } else {
          latest_xml += chunk.c_str();
        }
      }
    } while (!in_stream->Eof());
    delete in_stream;
    if (url.GetError() != wxURL_NOERR)
      status = CheckVersion::CheckStatus::CANT_CONNECT;
  }
  stringT latest;
  if (status == CheckVersion::CheckStatus::UP2DATE) {
    CheckVersion cv(MAJORVERSION, MINORVERSION, REVISION);
    status = cv.CheckLatestVersion(latest_xml, latest);
  }
  m_newVerStatus->Clear();
  switch (status) {
  case CheckVersion::CheckStatus::CANT_CONNECT:
    *m_newVerStatus << _("Couldn't contact server.");
    break;
  case CheckVersion::CheckStatus::UP2DATE:
    *m_newVerStatus << _("This is the latest release!");
    break;
  case CheckVersion::CheckStatus::NEWER_AVAILABLE:
    {
      wxString newer(_("Current version: "));
      newer += pwsafeVersionString + L"\n";
      newer += _("Latest version:\t"); newer += latest.c_str();
      newer += L"\n\n";
      newer += _("Please visit the PasswordSafe website to download the latest version.");
      const wxString cs_title(_("Newer Version Found!"));
      *m_newVerStatus << cs_title;
      wxMessageDialog dlg(this, newer, cs_title, wxOK);
      dlg.ShowModal();
      break;
    }
  case CheckVersion::CheckStatus::CANT_READ:
    *m_newVerStatus << _("Could not read server version data.");
    break;
  default:
    break;
  }
  m_newVerStatus->Show();
}

void CAbout::OnVisitSiteClicked(wxHyperlinkEvent& WXUNUSED(event)) {
  wxLaunchDefaultBrowser(L"https://pwsafe.org");
}
