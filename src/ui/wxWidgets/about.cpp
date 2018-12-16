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
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
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

  EVT_CLOSE(                       CAbout::OnCloseWindow       )
  EVT_HYPERLINK( ID_CHECKNEW     , CAbout::OnCheckNewClicked   )
  EVT_HYPERLINK( ID_SITEHYPERLINK, CAbout::OnVisitSiteClicked  )
  EVT_BUTTON(    wxID_CLOSE      , CAbout::OnCloseClick        )
  EVT_THREAD(    wxID_ANY        , CAbout::OnDownloadCompleted )

END_EVENT_TABLE()

wxString CAbout::s_VersionData = wxEmptyString;

const wstringT s_URL_HOME      = L"https://pwsafe.org";
const cstringT s_URL_VERSION   =  "https://pwsafe.org/latest.xml";

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

  // Print version information on standard output which might be useful for error reports.
  pws_os::Trace(GetLibWxVersion());
  pws_os::Trace(GetLibCurlVersion());
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
  m_VersionStatus = nullptr;
  m_CurlHandle = nullptr;
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

  wxStaticText* latestStaticTextBegin = new wxStaticText(aboutDialog, wxID_STATIC, _("Latest version? Click "), wxDefaultPosition, wxDefaultSize, 0 );
  verCheckSizer->Add(latestStaticTextBegin, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5);

  wxGenericHyperlinkCtrl* latestCheckButton = new wxGenericHyperlinkCtrl(aboutDialog, ID_CHECKNEW, _("here"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  verCheckSizer->Add(latestCheckButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

  wxStaticText* latestStaticTextEnd = new wxStaticText(aboutDialog, wxID_STATIC, _(" to check."), wxDefaultPosition, wxDefaultSize, 0);
  verCheckSizer->Add(latestStaticTextEnd, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5);

  wxBoxSizer* visitSiteSizer = new wxBoxSizer(wxHORIZONTAL);
  rightSizer->Add(visitSiteSizer, 0, wxALIGN_LEFT|wxALL, 0);

  wxStaticText* visitSiteStaticTextBegin = new wxStaticText(aboutDialog, wxID_STATIC, _("Please visit the "), wxDefaultPosition, wxDefaultSize, 0);
  visitSiteSizer->Add(visitSiteStaticTextBegin, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5);

  wxGenericHyperlinkCtrl* visitSiteHyperlinkCtrl = new wxGenericHyperlinkCtrl(aboutDialog, ID_SITEHYPERLINK, _("PasswordSafe website"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  visitSiteSizer->Add(visitSiteHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

  wxStaticText* visitSiteStaticTextEnd = new wxStaticText(aboutDialog, wxID_STATIC, _("."), wxDefaultPosition, wxDefaultSize, 0);
  visitSiteSizer->Add(visitSiteStaticTextEnd, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5);

  wxStaticText* licenseStaticTextEnd = new wxStaticText(aboutDialog, wxID_STATIC, _("See LICENSE for open source details."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  rightSizer->Add(licenseStaticTextEnd, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* copyrightStaticText = new wxStaticText(aboutDialog, wxID_STATIC, _("Copyright (c) 2003-2018 by Rony Shapiro"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  rightSizer->Add(copyrightStaticText, 0, wxALIGN_LEFT|wxALL, 5);

  m_VersionStatus = new wxTextCtrl(aboutDialog, ID_TEXTCTRL, wxT("\n\n"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER);
  rightSizer->Add(m_VersionStatus, 0, wxALIGN_LEFT|wxALL|wxEXPAND|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
  m_VersionStatus->Hide();

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
 * wxEVT_CLOSE_WINDOW event handler
 */

void CAbout::OnCloseWindow( wxCloseEvent& WXUNUSED(event) )
{
  Cleanup();
  EndModal(wxID_CLOSE);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void CAbout::OnCloseClick( wxCommandEvent& WXUNUSED(event) )
{
  Cleanup();
  EndModal(wxID_CLOSE);
}

/**
 * Returns a <code>wxCriticalSection</code> object that is used to protect
 * the shared data <code>s_VersionData</code>, which is accessed by worker
 * thread and its parent thread.
 *
 * @return a static <code>wxCriticalSection</code> instance.
 */
wxCriticalSection& CAbout::CriticalSection()
{
  static wxCriticalSection criticalSectionObject;

  return criticalSectionObject;
}

/**
 * Prepare resources that are needed to request version data from server.
 *
 * The download of the version information (latest.xml) is performed by the external 
 * library Curl (libcurl). For the purpose of only downloading a single file the Easy 
 * API of libcurl is used, which already provides everything needed for this 
 * task and which keeps the implementation simple.
 *
 * The following settings are used.
 *
 * - Url      (CURLOPT_URL):           https://pwsafe.org/latest.xml
 * - Timeout  (CURLOPT_TIMEOUT):       120 seconds
 * - Callback (CURLOPT_WRITEFUNCTION): WriteCallback
 *
 * Once libcurl was successfully initialized and all needed options set a handle 
 * represents the connection and can be used for further activities during the 
 * lifetime of the <code>CAbout</code> instance.
 *
 * @see https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
 */
bool CAbout::SetupConnection()
{
  //
  // Setup Curl by creating a handle and setting options to configure how Curl should behave.
  // If we already have a handle from a previous call we reuse it.
  //
  if (m_CurlHandle == nullptr) {

    //
    // Curl initialization
    //
    CURLcode curlResult = curl_global_init(CURL_GLOBAL_ALL);

    if (curlResult != CURLE_OK) {
      m_VersionStatus->Clear();
      *m_VersionStatus << _("Could not initialize the Curl library.\n");
      *m_VersionStatus << curl_easy_strerror(curlResult);
      m_VersionStatus->Show();
      return false;
    }

    //
    // Curl handle creation
    //
    m_CurlHandle = curl_easy_init();

    if (m_CurlHandle == nullptr) {
      m_VersionStatus->Clear();
      *m_VersionStatus << _("Could not create a connection session.\n");
      *m_VersionStatus << curl_easy_strerror(curlResult);
      m_VersionStatus->Show();
      return false;
    }

    //
    // Curl options regarding the data connection
    //
    curlResult = curl_easy_setopt(m_CurlHandle, CURLOPT_URL, s_URL_VERSION.c_str());

    if (curlResult != CURLE_OK) {
      m_VersionStatus->Clear();
      *m_VersionStatus << _("Could not set 'URL' option.\n");
      *m_VersionStatus << curl_easy_strerror(curlResult);
      m_VersionStatus->Show();
      return false;
    }

    curlResult = curl_easy_setopt(m_CurlHandle, CURLOPT_TIMEOUT, 120L /*sec.*/);

    if (curlResult != CURLE_OK) {
      m_VersionStatus->Clear();
      *m_VersionStatus << _("Could not set 'Timeout' option.\n");
      *m_VersionStatus << curl_easy_strerror(curlResult);
      m_VersionStatus->Show();
      return false;
    }

    //
    // Callback function to handle received data
    //
    curlResult = curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEFUNCTION, CAbout::WriteCallback);

    if (curlResult != CURLE_OK) {
      m_VersionStatus->Clear();
      *m_VersionStatus << _("Could not set 'Write Function' option.\n");
      *m_VersionStatus << curl_easy_strerror(curlResult);
      m_VersionStatus->Show();
      return false;
    }
  }

  return true;
}

/**
 * Releases all resources that has been established to request version data from server.
 */
void CAbout::Cleanup()
{
  // Stop the worker thread if it is still active
  if (GetThread() && GetThread()->IsRunning()) {
    GetThread()->Delete();
  }

  // Free resources concerning the server connection
  if (m_CurlHandle != nullptr) {
    curl_easy_cleanup(m_CurlHandle);
    curl_global_cleanup();
    m_CurlHandle = nullptr;
  }
}

/**
 * Provides version information about Curl library.
 */
wxString CAbout::GetLibCurlVersion()
{
  wxString versionInfo;

  auto curlVersion = curl_version_info(CURLVERSION_NOW);

  versionInfo << "[libcurl] Curl Version: " << curlVersion->version << "\n";

  if (curlVersion->ssl_version != nullptr) {
    versionInfo << "[libcurl] SSL Version: " << curlVersion->ssl_version << "\n";
  }
  else {
    versionInfo << "[libcurl] SSL Version: no SSL support\n";
  }

  wxString protocols;

  for (size_t i = 0; i < (sizeof(curlVersion->protocols) / sizeof(curlVersion->protocols[0])); i++) {
    i == 0 ? protocols = (curlVersion->protocols)[i] : protocols << ", " << (curlVersion->protocols)[i];
  }

  versionInfo << "[libcurl] Supported Protocols: " << protocols << "\n";

  return versionInfo;
}

/**
 * Provides version information about wxWidgets framework.
 */
wxString CAbout::GetLibWxVersion()
{
  return wxString::Format("[wx] Wx Version:\n%s\n", wxGetLibraryVersionInfo().ToString());
}

/**
 * Checks whether database is closed.
 *
 * If database is open user is prompted to close the database.
 *
 * @return true if database is closed, otherwise false.
 */
bool CAbout::CheckDatabaseStatus()
{
  PasswordSafeFrame *pwsafe = static_cast<PasswordSafeFrame *>(GetParent());

  if (!pwsafe->IsClosed()) {

    const wxString cs_txt(_(
      "For security, the database must be closed before connecting to the Internet.\n"
      "Press OK to close database and continue (Changes will be saved)")
    );

    const wxString cs_title(
      _("Confirm Close Dialog")
    );

    wxMessageDialog dialog(this, cs_txt, cs_title, (wxICON_QUESTION | wxOK | wxCANCEL));

    if (dialog.ShowModal() == wxID_CANCEL) {
      return false;
    }

    // Notify PasswordSafeFrame to close database.
    // If there are any unsaved changes PasswordSafeFrame 
    // will prompt the user to save them.
    wxCommandEvent closeEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_CLOSE);
#if wxCHECK_VERSION(2,9,0)
    pwsafe->GetEventHandler()->ProcessEvent(closeEvent);
#else
    pwsafe->ProcessEvent(closeEvent);
#endif

    // Check database once again, because user could have cancelled to save changes.
    if (!pwsafe->IsClosed()) {
      return false;
    }
  }

  // Update UI accordingly to show user that database is closed.
  pwsafe->Update();
  ASSERT(pwsafe->GetNumEntries() == 0);

  // Now, database is closed.
  return true;
}

/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCHECK
 */
void CAbout::CheckNewVersion()
{
  //
  // Version check might have been issued already and data transfer in worker thread is still ongoing.
  // In this case we wait until thread terminates, due to successfully performed transfer, failed transfer
  // or connection timeout.
  //
  if (GetThread() && GetThread()->IsRunning()) {
    pws_os::Trace(L"Worker thread already created and still running.");
    return;
  }

  //
  // For security reasons database must be closed before opening connection to server.
  //
  if (CheckDatabaseStatus() == false) {
    return;
  }

  //
  // Configure connection to server
  //
  if (SetupConnection() == false) {
    return;
  }

  // Now, the database should be closed and we should have a valid Curl handle to perform
  // the data transfer, thus worker thread will be created and started to fetch the data.
  // If thread creation fails the Curl handle still remains valid and can be (re)used.
  // It will be finally cleaned up when dialog gets closed.

  if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR) {
    pws_os::Trace(L"Could not create worker thread.");
    return;
  }

  m_VersionStatus->Clear();
  *m_VersionStatus << _("Trying to contact server...");
  m_VersionStatus->Show();

  //
  // The download starts right away.
  //
  if (GetThread() && (GetThread()->Run() != wxTHREAD_NO_ERROR)) {
    pws_os::Trace(L"Could not run worker thread.");
    return;
  }

  pws_os::Trace(L"Started worker thread to fetch version data.");
}

/**
 * Compares the downloaded version data against the one of the application.
 *
 * @see core routine <code>CheckVersion::CheckLatestVersion</code> for version 
 *      check algorithm and details about format of the downloaded xml file.
 */
void CAbout::CompareVersionData()
{
  CheckVersion::CheckStatus status = CheckVersion::CheckStatus::UP2DATE;
  stringT latest_xml;

  //
  // Get the downloaded data that worker thread has gathered in static wxString object.
  //
  if (CriticalSection().TryEnter()) {
    latest_xml = s_VersionData;
    s_VersionData.Empty();
    CriticalSection().Leave();
  }
  else {
    pws_os::Trace(L"CheckVersion - couldn't enter critical section to access version data");
    status = CheckVersion::CheckStatus::CANT_READ;
  }

  //
  // Compare between current version in use and latest available version
  //
  stringT latest;
  if (status == CheckVersion::CheckStatus::UP2DATE) {
    CheckVersion cv(MAJORVERSION, MINORVERSION, REVISION);
    status = cv.CheckLatestVersion(latest_xml, latest);
  }

  //
  // Update UI with determined version status
  //
  m_VersionStatus->Clear();
  switch (status) {
    case CheckVersion::CheckStatus::CANT_CONNECT:
      *m_VersionStatus << _("Couldn't contact server.");
      pws_os::Trace(wxString::Format("Server URL: %s", s_URL_VERSION));
      break;

    case CheckVersion::CheckStatus::UP2DATE:
      *m_VersionStatus << _("This is the latest release!");
      break;

    case CheckVersion::CheckStatus::NEWER_AVAILABLE:
    {
      wxString newer(_("Current version: "));
      newer << pwsafeVersionString << L"\n";
      newer << _("Latest version:\t") << latest.c_str() << L"\n\n";
      newer << _("Please visit the PasswordSafe website to download the latest version.");
      const wxString cs_title(_("Newer Version Found!"));
      *m_VersionStatus << cs_title;
      wxMessageDialog dlg(this, newer, cs_title, wxOK);
      dlg.ShowModal();
      break;
    }
    case CheckVersion::CheckStatus::CANT_READ:
      *m_VersionStatus << _("Could not read server version data.");
      pws_os::Trace(wxString::Format("parsed version data: \n'%s'\n", latest_xml));
      break;

    default:
      break;
  }
  m_VersionStatus->Show();
}

/**
 * This is the entry point of the worker thread which handles the file download.
 *
 * This function gets executed in the secondary thread context and will use the
 * blocking function <code>curl_easy_perform</code> to perform the download,
 * which returns when download was successful, it failed or the configured timeout
 * occured. After <code>curl_easy_perform</code> completed the worker thread sends
 * a notification event to its parent thread to inform it about the result. The
 * parent thread will decide on the received exit code how to proceed.
 *
 * @return ExitCode 0 indicates success,
 *                 -1 indicates that no Curl handle exists,
 *                 positive values correspond to Curl error codes
 *
 * @see method <code>CAbout::WriteCallback(char *receivedData, size_t size, size_t bytes, void *userData)</code>
 * @see https://curl.haxx.se/libcurl/c/libcurl-errors.html
 * @see https://curl.haxx.se/libcurl/c/curl_easy_perform.html
 */
wxThread::ExitCode CAbout::Entry()
{
  CURLcode curlResult;
  auto event = new wxThreadEvent();

  if (m_CurlHandle != nullptr) {
    pws_os::Trace(L"Fetching version data...");

    curlResult = curl_easy_perform(m_CurlHandle);

    event->SetInt(curlResult);                        // error code
    event->SetString(curl_easy_strerror(curlResult)); // description of the error code

    // We are done. Let's inform the parent thread about the status.
    wxQueueEvent(GetEventHandler(), event);

    return (wxThread::ExitCode)curlResult;
  }
  else {
    // Unexpected case that thread was started while no Curl handle exists
    event->SetInt(-1);
    event->SetString(_("No Curl handle to perform download."));

    // We are done. Let's inform the parent thread about the status.
    wxQueueEvent(GetEventHandler(), event);

    return (wxThread::ExitCode)-1;
  }
}

/**
 * This function is called by the Curl library each time new data is received after
 * <code>curl_easy_perform</code> was triggered.
 *
 * The version data might be received in chunks, which are collected in the static
 * variable <code>s_VersionData</code>, until data transfer is accomplished.
 *
 * @see method <code>CAbout::Entry()</code>
 * @see https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
 */
size_t CAbout::WriteCallback(char *receivedData, size_t size, size_t bytes, void *userData)
{
  size_t receivedDataSize = size * bytes;

  if (CriticalSection().TryEnter()) {

    if (receivedDataSize > 0) {
      s_VersionData += receivedData;
    }
    CriticalSection().Leave();
  }
  else {
    pws_os::Trace(L"WriteCallback - couldn't enter critical section to access version data");
  }

  return receivedDataSize;
}

/**
 * wxEVT_THREAD event handler for wxID_ANY
 *
 * This event handler is called by worker thread when file download is accomplished.
 * CURLE_OK is the only acceptable exit code from worker thread to continue with version check.
 * All other exit codes indicate some sort of occured problem.
 *
 * @see method <code>CAbout::Entry()</code> regarding worker thread and its exit codes.
 */
void CAbout::OnDownloadCompleted(wxThreadEvent& event)
{
  pws_os::Trace(wxString::Format("Got notification from worker thread. Exit Code = %d ; Result = %s", event.GetInt(), event.GetString()));

  if (event.GetInt() == 0 /*CURLE_OK*/) {
    CompareVersionData();
  }
  else {
    m_VersionStatus->Clear();
    *m_VersionStatus << _("Could not download version data.\n");
    *m_VersionStatus << event.GetString();
    m_VersionStatus->Show();

    // Show what we've received so far
    pws_os::Trace(wxString::Format("received data: \n'%s'\n", s_VersionData));

    // Forget about all the received data in error case
    s_VersionData.Empty();
  }
}

/**
 * wxEVT_HYPERLINK event handler for ID_SITEHYPERLINK
 */
void CAbout::OnVisitSiteClicked(wxHyperlinkEvent& WXUNUSED(event)) {
  wxLaunchDefaultBrowser(s_URL_HOME);
}
