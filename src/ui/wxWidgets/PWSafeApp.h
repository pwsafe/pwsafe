/////////////////////////////////////////////////////////////////////////////
// Name:        PWSafeApp.h
// Purpose:
// Author:      Rony Shapiro
// Modified by:
// Created:     Wed 14 Jan 2009 10:11:39 PM IST
// RCS-ID:
// Copyright:   Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>
// Licence:
/////////////////////////////////////////////////////////////////////////////

/** \file PWSafeApp.h
*
*/

#ifndef _PWSAFEAPP_H_
#define _PWSAFEAPP_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "SafeCombinationEntryDlg.h"
////@end includes
#include "core/PWScore.h"
#include "./RecentDbList.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class wxTimer;
class PasswordSafeFrame;
class wxLocale;

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * PWSafeApp class declaration
 */

class PWSafeApp : public wxApp
{
    DECLARE_CLASS( PWSafeApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    PWSafeApp();

    /// Destructor
    ~PWSafeApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

  /// Handle asserts without showing the assert dialog until locale is initialized.
#ifdef __WXDEBUG__
  virtual void OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg);
#endif
    /// Called on exit
    virtual int OnExit();

////@begin PWSafeApp event handler declarations

////@end PWSafeApp event handler declarations

////@begin PWSafeApp member function declarations

////@end PWSafeApp member function declarations

////@begin PWSafeApp member variables
////@end PWSafeApp member variables

  void OnIdleTimer(wxTimerEvent& timerEvent);
  void ConfigureIdleTimer();
  void RestartIdleTimer();
  void OnHelp(wxCommandEvent& evt);
  void OnDBGUIPrefsChange(wxEvent& evt);

  void SaveFrameCoords(void);
  void RestoreFrameCoords(void);

  //virtual override from some ancestor, to handle Help commands from all windows
  virtual int FilterEvent(wxEvent& evt);

  wxIconBundle GetAppIcons() const { return m_appIcons; }

 private:
    PWScore m_core;
    wxTimer* m_idleTimer;
    PasswordSafeFrame* m_frame;
    enum { IDLE_TIMER_ID = 33 } ;
    CRecentDBList *m_recentDatabases;

    //A map of dialog titles (or tab names) vs help sections
    WX_DECLARE_STRING_HASH_MAP( wxString, StringToStringMap );
    StringToStringMap& GetHelpMap();
    wxIconBundle m_appIcons;
    wxLocale *m_locale; // set in Init(), deleted in d'tor, unused elsewhere
    wxString helpFileNamePath;
    bool isHelpActivated;
    bool ActivateHelp(wxLanguage language);
    
 public:
    CRecentDBList &recentDatabases();
    uint32 GetHashIters() const {return m_core.GetHashIters();}
    bool ActivateLanguage(wxLanguage language, bool tryOnly);
    wxLanguage GetSystemLanguage();
    wxLanguage GetSelectedLanguage();
};

/*!
 * Application instance declaration
 */

////@begin declare app
DECLARE_APP(PWSafeApp)
////@end declare app

#endif // _PWSAFEAPP_H_
