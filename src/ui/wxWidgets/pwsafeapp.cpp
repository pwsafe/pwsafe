/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

#if !defined(_WIN32)
#include <unistd.h> // for fork()
#endif

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
   STR("encrypt a file (deprecated)"),
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
  {wxCMD_LINE_OPTION, STR("d"), STR("decrypt"),
   STR("decrypt a file (deprecated)"),
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
    EVT_ACTIVATE_APP(PwsafeApp::OnActivate)
    EVT_TIMER(ACTIVITY_TIMER_ID, PwsafeApp::OnActivityTimer)
    EVT_CUSTOM(wxEVT_GUI_DB_PREFS_CHANGE, wxID_ANY, PwsafeApp::OnDBGUIPrefsChange)
END_EVENT_TABLE()


/*!
 * Constructor for PwsafeApp
 */

PwsafeApp::PwsafeApp() : m_activityTimer(new wxTimer(this, ACTIVITY_TIMER_ID)),
  m_frame(0), m_recentDatabases(0),
  m_controller(new wxHtmlHelpController)
{
    Init();
}

/*!
 * Destructor for PwsafeApp
 */
PwsafeApp::~PwsafeApp()
{
  delete m_activityTimer;
  delete m_recentDatabases;

  PWSprefs::DeleteInstance();
  PWSrand::DeleteInstance();
  
  delete m_controller;
}

/*!
 * Member initialisation
 */

void PwsafeApp::Init()
{
////@begin PwsafeApp member initialisation
////@end PwsafeApp member initialisation
}

/*!
 * Initialisation for PwsafeApp
 */

bool PwsafeApp::OnInit()
{    
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
  bool cmd_validate = cmdParser.Found(wxT("v"), &filename);
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

#if !defined(__WXDEBUG__) && !defined(__WXMAC__) && !defined(_WIN32)
  // Now's a good time to fork
  // and exit the parent process, returning the command prompt to the user
  // (but not for debug builds - just make debugging harder)
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork"); // should never happen!
    exit(1);
  } else if (pid != 0) { // parent
    exit(0);
  }
#endif /* _DEBUG */

  wxSingleInstanceChecker appInstance;
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
    // dbox.SetStartClosed(true);
  }
  // dbox.SetStartSilent(cmd_silent);
  if (cmd_minimized) {
    m_core.SetCurFile(wxT(""));
    //dbox.SetStartClosed(true);
    // dbox.SetStartSilent(true);
  }
  // dbox.SetValidate(cmd_validate);

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

  if (!cmd_closed) {
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
    if (cmd_validate)
      m_frame->ValidateCurrentDatabase();
  } 
  else {
    wxASSERT_MSG(!m_frame, wxT("Frame window created unexpectedly"));
    m_frame = new PasswordSafeFrame(NULL, m_core);
  }

  RestoreFrameCoords();
  m_frame->Show();
  return true;
}


/*!
 * Cleanup for PwsafeApp
 */

int PwsafeApp::OnExit()
{    
  recentDatabases().Save();
  PWSprefs *prefs = PWSprefs::GetInstance();
  if (!m_core.GetCurFile().empty())
    prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());
  // Save Application related preferences
  prefs->SaveApplicationPreferences();
  m_activityTimer->Stop();
////@begin PwsafeApp cleanup
  return wxApp::OnExit();
////@end PwsafeApp cleanup
}

void PwsafeApp::OnActivate(wxActivateEvent& actEvent)
{
  if (actEvent.GetActive()) {
    m_activityTimer->Stop();
  }
  else {
    m_activityTimer->Stop();
    m_activityTimer->Start(PWSprefs::GetInstance()->GetPref(PWSprefs::IdleTimeout)*60*1000, true);
  }
  actEvent.Skip();
}

void PwsafeApp::OnActivityTimer(wxTimerEvent& /* timerEvent */)
{
  if (!m_frame->GetCurrentSafe().IsEmpty())
    m_frame->HideUI(true);  //true => lock
}

void PwsafeApp::OnDBGUIPrefsChange(wxEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  if (m_activityTimer->IsRunning()) {
    m_activityTimer->Stop();
    m_activityTimer->Start(PWSprefs::GetInstance()->GetPref(PWSprefs::IdleTimeout)*60*1000, true);
  }
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
  if (left == -1 && top == -1 && right == -1 && bottom == -1) {
    m_frame->Maximize();
  }
  else {
    wxRect rcApp(left, top, right - left, bottom - top);
    
    int displayWidth, displayHeight;
    ::wxDisplaySize(&displayWidth, &displayHeight);
    wxRect rcDisplay(0, 0, displayWidth, displayHeight);
    
    if (!rcApp.IsEmpty() && rcDisplay.Contains(rcApp))
      m_frame->SetSize(rcApp);
  }
}

int PwsafeApp::FilterEvent(wxEvent& evt) {
  if (evt.IsCommandEvent() && evt.GetId() == wxID_HELP && 
          (evt.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED || 
            evt.GetEventType() == wxEVT_COMMAND_MENU_SELECTED)) {
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
      msg << wxT("Missing help definition for page \"") << pageName
          << wxT("\" of \"") << dlgName 
          << wxT("\".\n");
    } else { // !propSheet
      keyName = win->GetClassInfo()->GetClassName();
      msg << wxT("Missing help definition for window \"") << keyName
          << wxT("\".\n");
    }
    
    StringToStringMap& helpmap = GetHelpMap();
    StringToStringMap::iterator itr = helpmap.find(keyName);
    if (itr != helpmap.end())
      m_controller->DisplaySection(itr->second);
    else {
#ifdef __WXDEBUG__
      msg << wxT("Please inform the developers.");
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

