/*
 * Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwsafeapp.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/cmdline.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <iostream> // currently for debugging

using namespace std;

#include "pwsafeapp.h"
#include "passwordsafeframe.h"
#include "version.h"
#include "wxMessages.h"
#include "core/SysInfo.h"
#include "core/PWSprefs.h"
#include "core/PWSrand.h"
#include <wx/timer.h>
#include <wx/html/helpctrl.h>
#include "../../os/dir.h"
#include <wx/tokenzr.h>
#include <wx/fs_arc.h>
#include <wx/propdlg.h>
#include <wx/textfile.h>
#if defined(__X__) || defined(__WXGTK__)
#include <wx/clipbrd.h>
#endif
#include <wx/snglinst.h>
#include "../../core/PWSLog.h"
#include "./pwsmenushortcuts.h"

#include <wx/spinctrl.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin XPM images
#include "./graphics/pwsafe16.xpm"
#include "./graphics/pwsafe32.xpm"
#include "./graphics/pwsafe48.xpm"

////@end XPM images

#if wxCHECK_VERSION(2,9,0)
#define STR(s) s
#else
#define STR(s) wxT(s)
#endif

static const wxCmdLineEntryDesc cmdLineDesc[] = {
  {wxCMD_LINE_SWITCH, STR("?"), STR("help"),
   STR("displays command line usage"),
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP},
  {wxCMD_LINE_SWITCH, STR("r"), STR("read-only"),
   STR("open database read-only"),
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("v"), STR("validate"),
   STR("validate (and repair) database"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("e"), STR("encrypt"),
   STR("encrypt a file"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("d"), STR("decrypt"),
   STR("decrypt a file"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_SWITCH, STR("c"), STR("close"),
   STR("start 'closed', without prompting for database"),
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_SWITCH, STR("s"), STR("silent"),
   STR("start 'silently', minimized and with no database"),
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_SWITCH, STR("m"), STR("minimized"),
   STR("like '-c', plus start minimized"),
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("u"), STR("username"),
   STR("use specified user preferences instead of current user"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("h"), STR("hostname"),
   STR("use specified host preferences instead of current host"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("g"), STR("config_file"),
   STR("use specified configuration file instead of default"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_PARAM, NULL, NULL, STR("database"),
   wxCMD_LINE_VAL_STRING,
   (wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE)},
  {wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, 0}
};

#undef STR

static wxReporter aReporter;
static wxAsker    anAsker;

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( PwsafeApp )
////@end implement app


/*!
 * PwsafeApp type definition
 */

IMPLEMENT_CLASS( PwsafeApp, wxApp )


/*!
 * PwsafeApp event table definition
 */

  BEGIN_EVENT_TABLE( PwsafeApp, wxApp )

////@begin PwsafeApp event table entries
////@end PwsafeApp event table entries
  EVT_TIMER(IDLE_TIMER_ID, PwsafeApp::OnIdleTimer)
  EVT_CUSTOM(wxEVT_GUI_DB_PREFS_CHANGE, wxID_ANY, PwsafeApp::OnDBGUIPrefsChange)
  END_EVENT_TABLE()


/*
bool PwsafeApp::activateLanguageX(wxLanguage language)
{
  wxLanguage lang = wxLANGUAGE_ENGLISH;

  // for the English language there is no translation file available and
  // needed. The locale should be available but English is also our fall
  // back language, hence we don't care.
  if ( language == wxLANGUAGE_ENGLISH ) {
    lang = wxLANGUAGE_ENGLISH;
    wxDELETE (m_locale);
    return false;
  }

  // the selected language was not English and the corresponding
  // locale does not exist
  else if ( !wxLocale::IsAvailable(language) ) {
    std::wcerr <<
      L"The selected language is not supported by your system. "
      L"Try installing support for this language." << std::endl;

    // should we use wxLANGUAGE_SYSTEM which garanties us the
    // corresponding locale but not a translation?
    //lang = wxLANGUAGE_ENGLISH;
    lang = language;
  }

  // if the language package resp. locale for the desired language
  // is available on the system we take this one and try to load
  // the corresponding translation file
  else if ( wxLocale::IsAvailable(language) ) {
    lang = language;
  }

  // we are going to create a new locale so the old one is not needed anymore
  wxDELETE( m_locale );

#if wxCHECK_VERSION( 2, 9, 0 )
  m_locale = new wxLocale( lang );
#else
  m_locale = new wxLocale( lang, wxLOCALE_CONV_ENCODING );
#endif

  // add locale search paths
  m_locale->AddCatalogLookupPathPrefix(wxT("/usr"));
  m_locale->AddCatalogLookupPathPrefix(wxT("/usr/local"));
#if defined(__WXDEBUG__) || defined(_DEBUG) || defined(DEBUG)
  m_locale->AddCatalogLookupPathPrefix(wxT("../I18N/mos"));
#endif

  if ( language != wxLANGUAGE_ENGLISH ) {
    // add translation file
    if ( !m_locale->AddCatalog(wxT("pwsafe")))
      std::wcerr << L"Couldn't load text for "
        << m_locale->GetLanguageName(language).c_str() << std::endl;
  }

  // let's check the state of our new locale instance
  if ( !m_locale->IsOk() )
    std::wcerr << L"Failed to set locale for selected language." << std::endl;

  return true;
  //add translation for standard resources
  //m_locale->AddStdCatalog();
}
*/
/*!
 * Constructor for PwsafeApp
 */

PwsafeApp::PwsafeApp() : m_idleTimer(new wxTimer(this, IDLE_TIMER_ID)),
                         m_idleFlag(true), m_frame(0), m_recentDatabases(0),
       m_controller(new wxHtmlHelpController), m_locale(NULL)
{
  Init();
}

/*!
 * Destructor for PwsafeApp
 */
PwsafeApp::~PwsafeApp()
{
  delete m_idleTimer;
  delete m_recentDatabases;

  PWSprefs::DeleteInstance();
  PWSrand::DeleteInstance();
  PWSLog::DeleteLog();

  delete m_controller;
  delete m_locale;
}

/*!
 * Member initialisation
 */

void PwsafeApp::Init()
{
  m_locale = new wxLocale;
  wxLocale::AddCatalogLookupPathPrefix( wxT("/usr") );
  wxLocale::AddCatalogLookupPathPrefix( wxT("/usr/local") );
#if defined(__WXDEBUG__) || defined(_DEBUG) || defined(DEBUG)
  wxLocale::AddCatalogLookupPathPrefix( wxT("../I18N/mos") );
#endif

////@begin PwsafeApp member initialisation
////@end PwsafeApp member initialisation
}

#ifdef __WXDEBUG__
void PwsafeApp::OnAssertFailure(const wxChar *file, int line, const wxChar *func,
                const wxChar *cond, const wxChar *msg)
{
  if (m_locale)
    wxApp::OnAssertFailure(file, line, func, cond, msg);
  else
      std::wcerr << file << L'(' << line << L"):"
                 << L" assert \"" << cond << L"\" failed in " << (func? func: L"") << L"(): "
                 << (msg? msg: L"") << endl;
}
#endif

/*!
 * Initialisation for PwsafeApp
 */

bool PwsafeApp::OnInit()
{
  m_locale->Init( GetSystemLanguage() );
  ActivateLanguage( GetSystemLanguage() );

  SetAppName(pwsafeAppName);
  m_core.SetApplicationNameAndVersion(tostdstring(pwsafeAppName),
                                      DWORD((MINORVERSION << 16) | MAJORVERSION));
  PWSprefs::SetReporter(&aReporter);
  PWScore::SetReporter(&aReporter);
  PWScore::SetAsker(&anAsker);

#if wxUSE_XPM
  wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
  wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
  wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
  wxImage::AddHandler(new wxGIFHandler);
#endif
  // Get progname
  wxString progName(argv[0]);
  progName = progName.AfterLast(wxChar('/'));
  // Parse command line
  wxCmdLineParser cmdParser(cmdLineDesc, argc, argv);
  int res = cmdParser.Parse();
  if (res != 0)
    return false;

  // Parse command line options:
  wxString filename, user, host, cfg_file;
  bool cmd_ro = cmdParser.Found(wxT("r"));
  // Next variable currently not referenced
  bool cmd_encrypt = cmdParser.Found(wxT("e"), &filename);
  bool cmd_decrypt = cmdParser.Found(wxT("d"), &filename);
  bool cmd_closed = cmdParser.Found(wxT("c"));
  bool cmd_silent = cmdParser.Found(wxT("s"));
  bool cmd_minimized = cmdParser.Found(wxT("m"));
  bool cmd_user = cmdParser.Found(wxT("u"), &user);
  bool cmd_host = cmdParser.Found(wxT("h"), &host);
  bool cmd_cfg = cmdParser.Found(wxT("g"), &cfg_file);
  size_t count = cmdParser.GetParamCount();
  if (count == 1)
    filename = cmdParser.GetParam();
  else if (count > 1) {
    cmdParser.Usage();
    return false;
  }
  // check for mutually exclusive options
  if (((cmd_encrypt + cmd_decrypt) > 1) ||
      ((cmd_closed + cmd_silent + cmd_minimized) > 1)) {
    cmdParser.Usage();
    return false;
  }

  if (cmd_user)
    SysInfo::GetInstance()->SetEffectiveUser(tostdstring(user));
  if (cmd_host)
    SysInfo::GetInstance()->SetEffectiveHost(tostdstring(host));
  if (cmd_cfg)
    PWSprefs::SetConfigFile(tostdstring(cfg_file));

  m_core.SetReadOnly(cmd_ro);
  // OK to load prefs now
  PWSprefs *prefs = PWSprefs::GetInstance();

  // if filename passed in command line, it takes precedence
  // over that in preference:
  if (filename.empty()) {
    filename =  prefs->GetPref(PWSprefs::CurrentFile).c_str();
  } else {
    recentDatabases().AddFileToHistory(filename);
  }
  m_core.SetCurFile(tostringx(filename));
  m_core.SetApplicationNameAndVersion(tostdstring(progName),
                                      MAKEWORD(MINORVERSION, MAJORVERSION));

  static wxSingleInstanceChecker appInstance;
  if (!prefs->GetPref(PWSprefs::MultipleInstances) &&
        (appInstance.Create(wxT("pwsafe.lck"), towxstring(pws_os::getuserprefsdir())) &&
         appInstance.IsAnotherRunning()))
  {
    wxMessageBox(_("Another instance of Password Safe is already running"), _("Password Safe"),
                          wxOK|wxICON_INFORMATION);
    return false;
  }

#if defined(__X__) || defined(__WXGTK__)
  wxTheClipboard->UsePrimarySelection(prefs->GetPref(PWSprefs::UsePrimarySelectionForClipboard));
#endif

  // here if we're the child
  recentDatabases().Load();

  if (cmd_closed) {
    m_core.SetCurFile(wxT(""));
  }
  if (cmd_minimized) {
    m_core.SetCurFile(wxT(""));
  }
  if (cmd_silent) {
    // start silent implies use system tray.
    PWSprefs::GetInstance()->SetPref(PWSprefs::UseSystemTray, true);
  }

  //Initialize help subsystem
  wxFileSystem::AddHandler(new wxArchiveFSHandler);

  wxString helpfile(wxFileName(towxstring(pws_os::gethelpdir()), wxT("help.zip")).GetFullPath());
  if (!m_controller->Initialize(helpfile))
    wxMessageBox(_("Could not initialize help subsystem. Help will not be available"),
    _("Error initializing help"), wxOK | wxICON_ERROR);
  m_controller->SetParentWindow(NULL); // try to de-modalize. Partially (?) successful

  m_appIcons.AddIcon(pwsafe16);
  m_appIcons.AddIcon(pwsafe32);
  m_appIcons.AddIcon(pwsafe48);

  if (!cmd_closed && !cmd_silent && !cmd_minimized) {
    // Get the file, r/w mode and password from user
    // Note that file may be new
    CSafeCombinationEntry* initWindow = new CSafeCombinationEntry(NULL, m_core);
    int returnValue = initWindow->ShowModal();
    initWindow->Destroy();

    if (returnValue != wxID_OK) {
      return false;
    }
    wxASSERT_MSG(!m_frame, wxT("Frame window created unexpectedly"));
    m_frame = new PasswordSafeFrame(NULL, m_core);
    m_frame->Load(initWindow->GetPassword());
  }
  else {
    wxASSERT_MSG(!m_frame, wxT("Frame window created unexpectedly"));
    m_frame = new PasswordSafeFrame(NULL, m_core);
  }

  RestoreFrameCoords();
  m_frame->Show();
  if (cmd_minimized)
    m_frame->Iconize(true);
  else if (cmd_silent) {
    m_frame->SetTrayStatus(true);
    m_frame->HideUI(true);
  } else
    SetTopWindow(m_frame);
  return true;
}

/*!
 * Initializing the language support means currently
 * to determine the system default language and
 * activate this one for the application.
 */
wxLanguage PwsafeApp::GetSystemLanguage()
{
  int language = wxLocale::GetSystemLanguage();

#if defined(__UNIX__) && !defined(__WXMAC__) && wxCHECK_VERSION( 2, 8, 0 )
  // Workaround for wx bug 15006 that has been fixed in wxWidgets 2.8
  if (language == wxLANGUAGE_UNKNOWN) {
    std::wcerr << L"Couldn't detect locale. Trying to skip empty env. variables." << std::endl;
    // Undefine empty environment variables and try again
    wxString langFull;
    if (wxGetEnv(wxT("LC_ALL"), &langFull) && !langFull) {
      wxUnsetEnv(wxT("LC_ALL"));
      std::wcerr << L"Empty LC_ALL variable was dropped" << std::endl;
    }
    if (wxGetEnv(wxT("LC_MESSAGES"), &langFull) && !langFull) {
      wxUnsetEnv(wxT("LC_MESSAGES"));
      std::wcerr << L"Empty LC_MESSAGES variable was dropped" << std::endl;
    }
    if (wxGetEnv(wxT("LANG"), &langFull) && !langFull) {
      wxUnsetEnv(wxT("LANG"));
      std::wcerr << L"Empty LANG variable was dropped" << std::endl;
    }
    language = wxLocale::GetSystemLanguage();
  }
#endif

  return static_cast<wxLanguage>(language);
}

/*!
 * Activates a language.
 *
 * \param language the wxWidgets specific enumeration of type wxLanguage representing a supported language
 * \see http://docs.wxwidgets.org/trunk/language_8h.html#a7d1c74ce43b2fb7acf7a6fa438c0ee86
 */
bool PwsafeApp::ActivateLanguage(wxLanguage language)
{
  static wxString DOMAIN_("pwsafe");

  wxTranslations *translations = new wxTranslations;
  translations->SetLanguage( language );
  translations->AddStdCatalog();
  wxTranslations::Set( translations ); // takes care of occupied memory by wxTranslations

  if ( language == wxLANGUAGE_ENGLISH) {
    translations->SetLanguage( wxLANGUAGE_ENGLISH );
    return true;
  }

  if ( !translations->AddCatalog( DOMAIN_ ) ) {
    std::wcerr << L"Couldn't load language catalog for " << wxLocale::GetLanguageName(language) << std::endl;
    translations->SetLanguage( wxLANGUAGE_ENGLISH );
  }

  return translations->IsLoaded( DOMAIN_ );
}

/*!
 * Cleanup for PwsafeApp
 */

int PwsafeApp::OnExit()
{
  m_idleTimer->Stop();
  recentDatabases().Save();
  PWSprefs *prefs = PWSprefs::GetInstance();
  if (!m_core.GetCurFile().empty())
    prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());
  // Save Application related preferences
  prefs->SaveApplicationPreferences();
  // Save shortcuts, if changed
  PWSMenuShortcuts::GetShortcutsManager()->SaveUserShortcuts();

  PWSMenuShortcuts::DestroyShortcutsManager();
////@begin PwsafeApp cleanup
  return wxApp::OnExit();
////@end PwsafeApp cleanup
}

void PwsafeApp::ConfigureIdleTimer()
{
  const PWSprefs *prefs =   PWSprefs::GetInstance();

  bool isRunning = m_idleTimer->IsRunning();
  bool shouldBeRunning = prefs->GetPref(PWSprefs::LockDBOnIdleTimeout);
  int timeOut = shouldBeRunning ? prefs->GetPref(PWSprefs::IdleTimeout) : 0;

  if (!isRunning) {
    if (shouldBeRunning && timeOut != 0) {
      m_idleTimer->Start(timeOut * 60 * 1000, wxTIMER_CONTINUOUS);
    } else {
      // not running, nor should it be - nop
    }
  } else { // running
    if (!shouldBeRunning || timeOut == 0) {
      m_idleTimer->Stop();
    } else if (timeOut != 0) { // Restart
      m_idleTimer->Start(timeOut * 60 * 1000, wxTIMER_CONTINUOUS);
    }
  }
}

void PwsafeApp::OnIdleTimer(wxTimerEvent &evt)
{
  if (evt.GetId() == IDLE_TIMER_ID && PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
    if (m_frame != NULL && !m_frame->GetCurrentSafe().IsEmpty()) {
      if (m_idleFlag) // cleared if a user event occurred via FilterEvent()
        m_frame->HideUI(true);  //true => lock
      else
        m_idleFlag = true; // arm for next interval
    }
  }
}

void PwsafeApp::OnDBGUIPrefsChange(wxEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  ConfigureIdleTimer();
}

CRecentDBList &PwsafeApp::recentDatabases()
{
  // we create an instance of m_recentDatabases
  // as late as possible in order to make
  // sure that prefs' is set correcly (user, machine, etc.)
  if (m_recentDatabases == NULL)
    m_recentDatabases = new CRecentDBList;
  return *m_recentDatabases;
}

void PwsafeApp::SaveFrameCoords(void)
{
  if (m_frame->IsMaximized()) {
    PWSprefs::GetInstance()->SetPrefRect(-1, -1, -1, -1);
  }
  else if (m_frame->IsIconized() || !m_frame->IsShown()) {
    //if we save coords when minimized/hidden, pwsafe could be hidden or off
    //screen next time it runs
    return;
  }
  else {
    wxRect rc = m_frame->GetScreenRect();
    PWSprefs::GetInstance()->SetPrefRect(rc.GetTop(), rc.GetBottom(), rc.GetLeft(), rc.GetRight());
  }
}

void PwsafeApp::RestoreFrameCoords(void)
{
  long top, bottom, left, right;
  PWSprefs::GetInstance()->GetPrefRect(top, bottom, left, right);
  if (!(left == -1 && top == -1 && right == -1 && bottom == -1)) {
    wxRect rcApp(left, top, right - left, bottom - top);

    int displayWidth, displayHeight;
    ::wxDisplaySize(&displayWidth, &displayHeight);
    wxRect rcDisplay(0, 0, displayWidth, displayHeight);

    if (!rcApp.IsEmpty() && rcDisplay.Contains(rcApp))
      m_frame->SetSize(rcApp);
  }
}

int PwsafeApp::FilterEvent(wxEvent& evt) {
  const wxEventType et = evt.GetEventType();

  // Clear idle flag for lock-on-idle timer
  // We need a whitelist rather than a blacklist because
  // undocumented events are passed through here as well...
  if ((et == wxEVT_COMMAND_BUTTON_CLICKED) ||
      (et == wxEVT_COMMAND_CHECKBOX_CLICKED) ||
      (et == wxEVT_COMMAND_CHOICE_SELECTED) ||
      (et == wxEVT_COMMAND_LISTBOX_SELECTED) ||
      (et == wxEVT_COMMAND_LISTBOX_DOUBLECLICKED) ||
      (et == wxEVT_COMMAND_TEXT_UPDATED) ||
      (et == wxEVT_COMMAND_TEXT_ENTER) ||
      (et == wxEVT_COMMAND_MENU_SELECTED) ||
      (et == wxEVT_COMMAND_SLIDER_UPDATED) ||
      (et == wxEVT_COMMAND_RADIOBOX_SELECTED) ||
      (et == wxEVT_COMMAND_RADIOBUTTON_SELECTED) ||
      (et == wxEVT_COMMAND_SCROLLBAR_UPDATED) ||
      (et == wxEVT_COMMAND_VLBOX_SELECTED) ||
      (et == wxEVT_COMMAND_COMBOBOX_SELECTED) ||
      (et == wxEVT_COMMAND_TOOL_RCLICKED) ||
      (et == wxEVT_COMMAND_TOOL_ENTER) ||
      (et == wxEVT_COMMAND_SPINCTRL_UPDATED) ||
      (et == wxEVT_LEFT_DOWN) ||
      (et == wxEVT_LEFT_UP) ||
      (et == wxEVT_MIDDLE_DOWN) ||
      (et == wxEVT_MIDDLE_UP) ||
      (et == wxEVT_RIGHT_DOWN) ||
      (et == wxEVT_RIGHT_UP) ||
      (et == wxEVT_MOTION) ||
      (et == wxEVT_ENTER_WINDOW) ||
      (et == wxEVT_LEAVE_WINDOW) ||
      (et == wxEVT_LEFT_DCLICK) ||
      (et == wxEVT_MIDDLE_DCLICK) ||
      (et == wxEVT_RIGHT_DCLICK) ||
      (et == wxEVT_SET_FOCUS) ||
      (et == wxEVT_MOUSEWHEEL) ||
      (et == wxEVT_NAVIGATION_KEY) ||
      (et == wxEVT_KEY_DOWN) ||
      (et == wxEVT_KEY_UP) ||
      (et == wxEVT_SCROLL_TOP) ||
      (et == wxEVT_SCROLL_BOTTOM) ||
      (et == wxEVT_SCROLL_LINEUP) ||
      (et == wxEVT_SCROLL_LINEDOWN) ||
      (et == wxEVT_SCROLL_PAGEUP) ||
      (et == wxEVT_SCROLL_PAGEDOWN) ||
      (et == wxEVT_SCROLL_THUMBTRACK) ||
      (et == wxEVT_SCROLL_THUMBRELEASE) ||
      (et == wxEVT_SCROLL_CHANGED) ||
      (et == wxEVT_SIZE) ||
      (et == wxEVT_MOVE) ||
      (et == wxEVT_ACTIVATE_APP) ||
      (et == wxEVT_ACTIVATE) ||
      (et == wxEVT_SHOW) ||
      (et == wxEVT_ICONIZE) ||
      (et == wxEVT_MAXIMIZE) ||
      (et == wxEVT_MENU_OPEN) ||
      (et == wxEVT_MENU_CLOSE) ||
      (et == wxEVT_MENU_HIGHLIGHT) ||
      (et == wxEVT_CONTEXT_MENU) ||
      (et == wxEVT_JOY_BUTTON_DOWN) ||
      (et == wxEVT_JOY_BUTTON_UP) ||
      (et == wxEVT_JOY_MOVE) ||
      (et == wxEVT_JOY_ZMOVE) ||
      (et == wxEVT_DROP_FILES) ||
      (et == wxEVT_COMMAND_TEXT_COPY) ||
      (et == wxEVT_COMMAND_TEXT_CUT) ||
      (et == wxEVT_COMMAND_TEXT_PASTE) ||
      (et == wxEVT_COMMAND_LEFT_CLICK) ||
      (et == wxEVT_COMMAND_LEFT_DCLICK) ||
      (et == wxEVT_COMMAND_RIGHT_CLICK) ||
      (et == wxEVT_COMMAND_RIGHT_DCLICK) ||
      (et == wxEVT_COMMAND_ENTER) ||
      (et == wxEVT_HELP) ||
      (et == wxEVT_DETAILED_HELP))
    m_idleFlag = false; // for lock on idle timer

  if (evt.IsCommandEvent() && evt.GetId() == wxID_HELP &&
      (et == wxEVT_COMMAND_BUTTON_CLICKED ||
       et == wxEVT_COMMAND_MENU_SELECTED)) {
    OnHelp(*wxDynamicCast(&evt, wxCommandEvent));
    return int(true);
  }
  return wxApp::FilterEvent(evt);
}

void PwsafeApp::OnHelp(wxCommandEvent& evt)
{
  wxWindow* win = wxDynamicCast(evt.GetEventObject(), wxWindow);
  if (win) {
    //The window associated with the event is typically the Help button.  Fail if
    //we can't get to its parent
    if (win->GetId() == wxID_HELP && ((win = win->GetParent()) == NULL))
      return;

    wxString keyName, msg;
    //Is this a property sheet?
    wxPropertySheetDialog* propSheet = wxDynamicCast(win, wxPropertySheetDialog);
    if (propSheet) {
      const wxString dlgName = win->GetClassInfo()->GetClassName();
      const wxString pageName = propSheet->GetBookCtrl()->GetPageText(propSheet->GetBookCtrl()->GetSelection());
      keyName = dlgName + wxT('#') + pageName;
      msg << _("Missing help definition for page \"") << pageName
          << _("\" of \"") << dlgName
          << wxT("\".\n");
    } else { // !propSheet
      keyName = win->GetClassInfo()->GetClassName();
      msg << _("Missing help definition for window \"") << keyName
          << wxT("\".\n");
    }

    StringToStringMap& helpmap = GetHelpMap();
    StringToStringMap::iterator itr = helpmap.find(keyName);
    if (itr != helpmap.end())
      m_controller->DisplaySection(itr->second);
    else {
#ifdef __WXDEBUG__
      msg << _("Please inform the developers.");
      wxMessageBox(msg, _("Help Undefined"), wxOK | wxICON_EXCLAMATION);
#endif
    } // keyName not found in map
  } else {
    //just display the main page.  Could happen if the click came from a menu instead of
    //a button, like for the top-level frame
    m_controller->DisplayContents();
  }
}

PwsafeApp::StringToStringMap& PwsafeApp::GetHelpMap()
{
  //may be its worth defining a class and doing all this in its ctor, but
  //I don't want to introduce yet another set of source/header files just for this
  static StringToStringMap helpMap;
  static bool initialized = false;

  if (!initialized) {
#define DLG_HELP(dlgname, htmlfile) helpMap[wxSTRINGIZE_T(dlgname)] = wxSTRINGIZE_T(htmlfile);
#define PROPSHEET_HELP(sheet, page, htmlfile) helpMap[wxString(wxSTRINGIZE_T(sheet) wxT("#")) + page] = wxSTRINGIZE_T(htmlfile);
#include "./helpmap.h"
#undef DLG_HELP
#undef PROPSHEET_HELP
    initialized = true;
  }
  return helpMap;
}

