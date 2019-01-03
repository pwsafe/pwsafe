/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
*
*/

#ifndef _ABOUT_H_
#define _ABOUT_H_

/*!
 * Includes
 */

////@begin includes
#include <wx/hyperlink.h>
#include <wx/event.h>
#include <wx/thread.h>
////@end includes

#include "os/typedefs.h"

#include <curl/curl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CABOUT 10078
#define wxID_VERSIONSTR 10079
#define ID_CHECKNEW 10081
#define ID_SITEHYPERLINK 10080
#define ID_TEXTCTRL 10082
#define SYMBOL_CABOUT_STYLE wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CABOUT_TITLE _("About Password Safe")
#define SYMBOL_CABOUT_IDNAME ID_CABOUT
#define SYMBOL_CABOUT_SIZE wxSize(400, 300)
#define SYMBOL_CABOUT_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CAbout class declaration
 */

class CAbout: public wxDialog, public wxThreadHelper
{
  DECLARE_CLASS( CAbout )
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
  CAbout();
  CAbout( wxWindow* parent, wxWindowID id = SYMBOL_CABOUT_IDNAME, const wxString& caption = SYMBOL_CABOUT_TITLE, const wxPoint& pos = SYMBOL_CABOUT_POSITION, const wxSize& size = SYMBOL_CABOUT_SIZE, long style = SYMBOL_CABOUT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CABOUT_IDNAME, const wxString& caption = SYMBOL_CABOUT_TITLE, const wxPoint& pos = SYMBOL_CABOUT_POSITION, const wxSize& size = SYMBOL_CABOUT_SIZE, long style = SYMBOL_CABOUT_STYLE );

  /// Destructor
  ~CAbout();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  void CheckNewVersion();

////@begin CAbout event handler declarations

  /// event handler for ID_CHECKNEW
  void OnCheckNewClicked(wxHyperlinkEvent& WXUNUSED(event)) { CheckNewVersion(); };

  /// event handler for ID_SITEHYPERLINK
  void OnVisitSiteClicked(wxHyperlinkEvent& event);

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
  void OnCloseClick( wxCommandEvent& event );

  /// wxEVT_CLOSE_WINDOW event handler
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_THREAD event handler for wxID_ANY
  void OnDownloadCompleted(wxThreadEvent& event);
////@end CAbout event handler declarations

////@begin CAbout member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end CAbout member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

private:
////@begin CAbout member variables
  wxTextCtrl* m_VersionStatus;
////@end CAbout member variables

  /// The CURL handle with connection specific options for request of version data
  CURL *m_CurlHandle;

  /// Set to downloaded data by worker thread, resp. WriteCallback, and read by main thread for final version check
  static wxString s_VersionData;

  static const wstringT s_HOME_URL;
  static const cstringT s_VERSION_URL;
};

#endif /* _ABOUT_H_ */
