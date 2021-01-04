/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file AboutDlg.h
*
*/

#ifndef _ABOUTDLG_H_
#define _ABOUTDLG_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/hyperlink.h>
#include <wx/event.h>
#include <wx/thread.h>
////@end includes

#include "os/typedefs.h"

#ifndef __WINDOWS__
#define HAS_CURL
#endif // !__WINDOWS__

#ifdef HAS_CURL
#include <curl/curl.h>
#endif // HAS_CURL

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ABOUTDLG 10078
#define wxID_VERSIONSTR 10079
#define ID_CHECKNEW 10081
#define ID_SITEHYPERLINK 10080
#define ID_TEXTCTRL 10082
#define SYMBOL_ABOUTDLG_STYLE wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_ABOUTDLG_TITLE _("About Password Safe")
#define SYMBOL_ABOUTDLG_IDNAME ID_ABOUTDLG
#define SYMBOL_ABOUTDLG_SIZE wxSize(400, 300)
#define SYMBOL_ABOUTDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * AboutDlg class declaration
 */

class AboutDlg : public wxDialog, public wxThreadHelper
{
  DECLARE_CLASS( AboutDlg )
  DECLARE_EVENT_TABLE()

  void CompareVersionData();
  bool CheckDatabaseStatus();
  bool SetupConnection();
  void Cleanup();
  wxString GetLibCurlVersion();
  wxString GetLibWxVersion();
  static wxCriticalSection& CriticalSection();
  static size_t WriteCallback(char *receivedData, size_t size, size_t bytes, void *userData);

protected:
  virtual wxThread::ExitCode Entry();

public:
  /// Constructors
  AboutDlg();
  AboutDlg( wxWindow* parent, wxWindowID id = SYMBOL_ABOUTDLG_IDNAME, const wxString& caption = SYMBOL_ABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_ABOUTDLG_POSITION, const wxSize& size = SYMBOL_ABOUTDLG_SIZE, long style = SYMBOL_ABOUTDLG_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ABOUTDLG_IDNAME, const wxString& caption = SYMBOL_ABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_ABOUTDLG_POSITION, const wxSize& size = SYMBOL_ABOUTDLG_SIZE, long style = SYMBOL_ABOUTDLG_STYLE );

  /// Destructor
  ~AboutDlg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  void CheckNewVersion();

////@begin AboutDlg event handler declarations

  /// event handler for ID_CHECKNEW
  void OnCheckNewClicked(wxHyperlinkEvent& WXUNUSED(event)) { CheckNewVersion(); }

  /// event handler for ID_SITEHYPERLINK
  void OnVisitSiteClicked(wxHyperlinkEvent& event);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick( wxCommandEvent& event );

  /// wxEVT_CLOSE_WINDOW event handler
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_THREAD event handler for wxID_ANY
  void OnDownloadCompleted(wxThreadEvent& event);
////@end AboutDlg event handler declarations

////@begin AboutDlg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end AboutDlg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

private:
////@begin AboutDlg member variables
  wxTextCtrl* m_VersionStatus;
////@end AboutDlg member variables

  /// The CURL handle with connection specific options for request of version data
#ifdef HAS_CURL
  CURL *m_CurlHandle;
#else
  void *m_CurlHandle;
#endif // HAS_CURL

  /// Set to downloaded data by worker thread, resp. WriteCallback, and read by main thread for final version check
  static wxString s_VersionData;

  static const wstringT s_URL_HOME;
  static const cstringT s_URL_VERSION;
};

#endif /* _ABOUTDLG_H_ */
