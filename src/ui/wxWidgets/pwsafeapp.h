/////////////////////////////////////////////////////////////////////////////
// Name:        pwsafeapp.h
// Purpose:     
// Author:      Rony Shapiro
// Modified by: 
// Created:     Wed 14 Jan 2009 10:11:39 PM IST
// RCS-ID:      
// Copyright:   Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _PWSAFEAPP_H_
#define _PWSAFEAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "safecombinationentry.h"
////@end includes
#include "core/PWScore.h"
#include "./RecentDBList.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class wxTimer;
class PasswordSafeFrame;
class wxHtmlHelpController;
class wxLocale;

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * PwsafeApp class declaration
 */

class PwsafeApp: public wxApp
{    
    DECLARE_CLASS( PwsafeApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    PwsafeApp();

    /// Destructor
    ~PwsafeApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin PwsafeApp event handler declarations

////@end PwsafeApp event handler declarations

////@begin PwsafeApp member function declarations

////@end PwsafeApp member function declarations

////@begin PwsafeApp member variables
////@end PwsafeApp member variables

	void OnActivate(wxActivateEvent& actEvent);
	void OnActivityTimer(wxTimerEvent& timerEvent);
  void OnHelp(wxCommandEvent& evt);
  void OnDBGUIPrefsChange(wxEvent& evt);

  void SaveFrameCoords(void);
  void RestoreFrameCoords(void);
  
  //virtual override from some ancestor, to handle Help commands from all windows
  virtual int FilterEvent(wxEvent& evt);

  wxIconBundle GetAppIcons() const { return m_appIcons; }
  
 private:
    PWScore m_core;
    wxTimer* m_activityTimer;
    PasswordSafeFrame* m_frame;
    enum { ACTIVITY_TIMER_ID = 33 } ; 
    CRecentDBList *m_recentDatabases;

    //A map of dialog titles (or tab names) vs help sections
    WX_DECLARE_STRING_HASH_MAP( wxString, StringToStringMap );
    StringToStringMap& GetHelpMap();
    wxHtmlHelpController* m_controller;

    wxIconBundle m_appIcons;
    wxLocale *m_locale; // set in initLanguageSupport(), deleted in d'tor, unused elsewhere
    void initLanguageSupport();
    
 public:
    CRecentDBList &recentDatabases();
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(PwsafeApp)
////@end declare app

#endif
    // _PWSAFEAPP_H_
