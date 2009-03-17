/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "corelib/CheckVersion.h"
////@begin XPM images
////@end XPM images


/*!
 * CAbout type definition
 */

IMPLEMENT_CLASS( CAbout, wxDialog )


/*!
 * CAbout event table definition
 */

BEGIN_EVENT_TABLE( CAbout, wxDialog )

////@begin CAbout event table entries
  EVT_HYPERLINK( ID_HYPERLINKCTRL1, CAbout::OnHyperlinkctrl1HyperlinkClicked )

  EVT_BUTTON( wxID_CLOSE, CAbout::OnCloseClick )

////@end CAbout event table entries

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
////@begin CAbout creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end CAbout creation
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
 * Member initialisation
 */

void CAbout::Init()
{
////@begin CAbout member initialisation
  m_newVerStatus = NULL;
////@end CAbout member initialisation
}


/*!
 * Control creation for CAbout
 */

void CAbout::CreateControls()
{    
////@begin CAbout content construction
  CAbout* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticBitmap* itemStaticBitmap3 = new wxStaticBitmap( itemDialog1, wxID_STATIC, itemDialog1->GetBitmapResource(wxT("../graphics/cpane.bmp")), wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(49, 46)), 0 );
  itemBoxSizer2->Add(itemStaticBitmap3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_VERSIONSTR, _("Password Safe vx.yy (abcd)"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer4->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 0);

  wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Latest version? Click"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxHyperlinkCtrl* itemHyperlinkCtrl8 = new wxHyperlinkCtrl( itemDialog1, ID_HYPERLINKCTRL1, _("here"), _T(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
  itemBoxSizer6->Add(itemHyperlinkCtrl8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("to check."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer6->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer4->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALL, 0);

  wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Please visit the "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxHyperlinkCtrl* itemHyperlinkCtrl12 = new wxHyperlinkCtrl( itemDialog1, ID_HYPERLINKCTRL, _("PasswordSafe website"), _T("http://pwsafe.org/"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
  itemBoxSizer10->Add(itemHyperlinkCtrl12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("See LICENSE for open souce details."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  itemBoxSizer4->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText14 = new wxStaticText( itemDialog1, wxID_STATIC, _("Copyright (c) 2003-2009 by Rony Shapiro"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  itemBoxSizer4->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL, 5);

  m_newVerStatus = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(itemDialog1->ConvertDialogToPixels(wxSize(120, -1)).x, -1), wxTE_READONLY|wxNO_BORDER );
  m_newVerStatus->SetBackgroundColour(wxColour(230, 231, 232));
  itemBoxSizer4->Add(m_newVerStatus, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton16 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer4->Add(itemButton16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CAbout content construction
  const wxString vstring = pwsafeAppName + _T(" ") + pwsafeVersionString;
  itemStaticText5->SetLabel(vstring);
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
////@begin CAbout bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("../graphics/cpane.bmp"))
  {
    wxBitmap bitmap(_T("../graphics/cpane.bmp"), wxBITMAP_TYPE_BMP);
    return bitmap;
  }
  return wxNullBitmap;
////@end CAbout bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAbout::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin CAbout icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end CAbout icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void CAbout::OnCloseClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in CAbout.
  // Before editing this code, remove the block markers.
  EndModal(wxID_CLOSE);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE in CAbout. 
}


/*
 * The latest version information is in
 * http://pwsafe.org/latest.xml
 *
 * And is of the form:
 * <VersionInfo>
 *  <Product name=PasswordSafe variant=PC major=3 minor=10 build=2 rev=1710 />
 *  <Product name=PasswordSafe variant=PPc major=1 minor=9 build=2
 *    rev=100 />
 *  <Product name=PasswordSafe variant=U3 major=3 minor=10 build=2
 *    rev=1710 />
 *  <Product name=SWTPasswordSafe variant=Java major=0 minor=6
 *    build=0 rev=1230 />
 * </VersionInfo>
 *
 * Note: The "rev" is the svn commit number. Not using it (for now),
 *       as I think it's too volatile.
 */

/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL1
 */

void CAbout::OnHyperlinkctrl1HyperlinkClicked( wxHyperlinkEvent& event )
{
  // Get the latest.xml file from our site, compare to version,
  // and notify the user
  // First, make sure database is closed: Sensitive data with an
  // open socket makes me uneasy...
  PasswordSafeFrame *pFrm = static_cast<PasswordSafeFrame *>(GetParent());

  if (pFrm->GetNumEntries() != 0) {
    const wxString cs_txt(_("For security, the database must be closed before "
                            "connecting to the Internet.\r\nPress OK to close "
                            "database and continue (Changes will be saved)"));
    const wxString cs_title(_("Confirm Close Dialog"));
    wxMessageDialog dlg(this, cs_txt, cs_title,
                        (wxICON_QUESTION | wxOK | wxCANCEL));
    int rc = dlg.ShowModal();
    if (rc == wxCANCEL)
      return; // no hard feelings
    // Close database, prompt for save if changed
    wxCommandEvent closeEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_CLOSE);
    pFrm->ProcessEvent(closeEvent);
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
  stringT latext_xml;
  wxURL url(_("http://pwsafe.org/latest.xml"));
  wxInputStream *in_stream = url.GetInputStream();
  unsigned char buff[BUFSIZ+1];
  StringX chunk;
  stringT latest_xml;
  CUTF8Conv conv;
  CheckVersion::CheckStatus status = CheckVersion::UP2DATE;
  do {
    in_stream->Read(buff, BUFSIZ);
    size_t nRead = in_stream->LastRead();
    if (nRead != 0) {
      buff[nRead] = '\0';
      // change to widechar representation
      if (!conv.FromUTF8(buff, nRead, chunk)) {
        delete in_stream;
        status = CheckVersion::CANT_READ;
        break;
      } else {
        latest_xml += chunk.c_str();
      }
    }
  } while (!in_stream->Eof());
  delete in_stream;
  if (url.GetError() != wxURL_NOERR)
    status = CheckVersion::CANT_CONNECT;

  stringT latest;
  if (status == CheckVersion::UP2DATE) {
    CheckVersion cv(MAJORVERSION, MINORVERSION, 0);
    status = cv.CheckLatestVersion(latest_xml, latest);
  }
  m_newVerStatus->Clear();
  switch (status) {
    case CheckVersion::CANT_CONNECT:
      *m_newVerStatus << _("Couldn't contact server.");
      break;
    case CheckVersion::UP2DATE:
      *m_newVerStatus << _("This is the latest release!");
      break;
    case CheckVersion::NEWER_AVAILABLE:
    {
      wxString newer(_("Current version: "));
      newer += pwsafeVersionString;
      newer += _("\r\nLatest version:\t"); newer += latest.c_str();
      newer += _("\r\n\r\nPlease visit the PasswordSafe website to download the latest version.");
      const wxString cs_title(_("Newer Version Found!"));
      *m_newVerStatus << cs_title;
      wxMessageDialog dlg(this, newer, cs_title, wxOK);
      dlg.ShowModal();
      break;
    }
    case CheckVersion::CANT_READ:
      *m_newVerStatus << _("Could not read server version data.");
      break;
    default:
      break;
  }
  m_newVerStatus->Show();
}

