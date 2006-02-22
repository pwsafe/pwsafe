/// \file DboxOptions.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "corelib/PWSprefs.h"

// dialog boxen
#include "DboxMain.h"
#include "OptionsSecurity.h"
#include "OptionsDisplay.h"
#include "OptionsUsername.h"
#include "OptionsPasswordPolicy.h"
#include "OptionsMisc.h"


#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void
DboxMain::OnOptions() 
{
  CPropertySheet optionsDlg(_T("Options"), this);
  COptionsDisplay         display;
  COptionsSecurity        security;
  COptionsPasswordPolicy  passwordpolicy;
  COptionsUsername        username;
  COptionsMisc            misc;
  PWSprefs               *prefs = PWSprefs::GetInstance();
  BOOL                   prevLockOIT; // lock on idle timeout set?
  /*
  **  Initialize the property pages values.
  */
  display.m_alwaysontop = m_bAlwaysOnTop;
  display.m_pwshowinedit = prefs->
    GetPref(PWSprefs::BoolPrefs::ShowPWDefault) ? TRUE : FALSE;
  display.m_pwshowinlist = prefs->
    GetPref(PWSprefs::BoolPrefs::ShowPWInList) ? TRUE : FALSE;
#if defined(POCKET_PC)
  display.m_dcshowspassword = prefs->
    GetPref(PWSprefs::BoolPrefs::DCShowsPassword) ? TRUE : FALSE;
#endif
  display.m_usesystemtray = prefs->
    GetPref(PWSprefs::BoolPrefs::UseSystemTray) ? TRUE : FALSE;
  display.m_maxmruitems = prefs->
    GetPref(PWSprefs::IntPrefs::MaxMRUItems);
  display.m_mruonfilemenu = PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::MRUOnFileMenu);

  display.m_Display_Expanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::DisplayExpandedAddEditDlg);

  security.m_clearclipboard = prefs->
    GetPref(PWSprefs::BoolPrefs::DontAskMinimizeClearYesNo) ? TRUE : FALSE;
  security.m_lockdatabase = prefs->
    GetPref(PWSprefs::BoolPrefs::DatabaseClear) ? TRUE : FALSE;
  security.m_confirmsaveonminimize = prefs->
    GetPref(PWSprefs::BoolPrefs::DontAskSaveMinimize) ? FALSE : TRUE;
  security.m_confirmcopy = prefs->
    GetPref(PWSprefs::BoolPrefs::DontAskQuestion) ? FALSE : TRUE;
  security.m_LockOnWindowLock = prefs->
    GetPref(PWSprefs::BoolPrefs::LockOnWindowLock) ? TRUE : FALSE;
  security.m_LockOnIdleTimeout = prevLockOIT = prefs->
    GetPref(PWSprefs::BoolPrefs::LockOnIdleTimeout) ? TRUE : FALSE;
  security.m_IdleTimeOut = prefs->
    GetPref(PWSprefs::IntPrefs::IdleTimeout);

  passwordpolicy.m_pwlendefault = prefs->
    GetPref(PWSprefs::IntPrefs::PWLenDefault);
  passwordpolicy.m_pwuselowercase = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseLowercase);
  passwordpolicy.m_pwuseuppercase = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseUppercase);
  passwordpolicy.m_pwusedigits = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseDigits);
  passwordpolicy.m_pwusesymbols = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseSymbols);
  passwordpolicy.m_pwusehexdigits = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseHexDigits);
  passwordpolicy.m_pweasyvision = prefs->
    GetPref(PWSprefs::BoolPrefs::PWEasyVision);

  username.m_usedefuser = prefs->
    GetPref(PWSprefs::BoolPrefs::UseDefUser);
  username.m_defusername = CString(prefs->
                                   GetPref(PWSprefs::StringPrefs::DefUserName));
  username.m_querysetdef = prefs->
    GetPref(PWSprefs::BoolPrefs::QuerySetDef);

  misc.m_confirmdelete = prefs->
    GetPref(PWSprefs::BoolPrefs::DeleteQuestion) ? FALSE : TRUE;
  misc.m_saveimmediately = prefs->
    GetPref(PWSprefs::BoolPrefs::SaveImmediately) ? TRUE : FALSE;
  misc.m_escexits = prefs->
    GetPref(PWSprefs::BoolPrefs::EscExits) ? TRUE : FALSE;
  // by strange coincidence, the values of the enums match the indices
  // of the radio buttons in the following :-)
  misc.m_doubleclickaction = prefs->
    GetPref(PWSprefs::IntPrefs::DoubleClickAction);
  misc.m_hotkey_value = DWORD(prefs->GetPref(PWSprefs::IntPrefs::HotKey));
  misc.m_hotkey_enabled = prefs->GetPref(PWSprefs::BoolPrefs::HotKeyEnabled) ? TRUE : FALSE;

  optionsDlg.AddPage( &display );
  optionsDlg.AddPage( &security );
  optionsDlg.AddPage( &passwordpolicy );
  optionsDlg.AddPage( &username );
  optionsDlg.AddPage( &misc );


  /*
  **  Remove the "Apply Now" button.
  */
  optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW;
  app.DisableAccelerator();
  int rc = optionsDlg.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK)
    {
      /*
      **  First save all the options.
      */
      prefs->SetPref(PWSprefs::BoolPrefs::AlwaysOnTop,
                     display.m_alwaysontop == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::ShowPWDefault,
                     display.m_pwshowinedit == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::ShowPWInList,
                     display.m_pwshowinlist == TRUE);
#if defined(POCKET_PC)
      prefs->SetPref(PWSprefs::BoolPrefs::DCShowsPassword,
                     display.m_dcshowspassword == TRUE);
#endif
      prefs->SetPref(PWSprefs::BoolPrefs::UseSystemTray,
                     display.m_usesystemtray == TRUE);
      prefs->SetPref(PWSprefs::IntPrefs::MaxMRUItems,
                     display.m_maxmruitems);
      prefs->SetPref(PWSprefs::BoolPrefs::MRUOnFileMenu,
                     display.m_mruonfilemenu == TRUE);

      prefs->SetPref(PWSprefs::BoolPrefs::DisplayExpandedAddEditDlg,
                     display.m_Display_Expanded == TRUE);


      prefs->SetPref(PWSprefs::BoolPrefs::DontAskMinimizeClearYesNo,
                     security.m_clearclipboard == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::DatabaseClear,
                     security.m_lockdatabase == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::DontAskSaveMinimize,
                     security.m_confirmsaveonminimize == FALSE);
      prefs->SetPref(PWSprefs::BoolPrefs::DontAskQuestion,
                     security.m_confirmcopy == FALSE);
      prefs->SetPref(PWSprefs::BoolPrefs::LockOnWindowLock,
                     security.m_LockOnWindowLock == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::LockOnIdleTimeout,
                     security.m_LockOnIdleTimeout == TRUE);
      prefs->SetPref(PWSprefs::IntPrefs::IdleTimeout,
                     security.m_IdleTimeOut);

      prefs->SetPref(PWSprefs::IntPrefs::PWLenDefault,
                     passwordpolicy.m_pwlendefault);
      prefs->SetPref(PWSprefs::BoolPrefs::PWUseLowercase,
                     passwordpolicy.m_pwuselowercase == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::PWUseUppercase,
                     passwordpolicy.m_pwuseuppercase == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::PWUseDigits,
                     passwordpolicy.m_pwusedigits == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::PWUseSymbols,
                     passwordpolicy.m_pwusesymbols == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::PWUseHexDigits,
                     passwordpolicy.m_pwusehexdigits == TRUE);
      prefs-> SetPref(PWSprefs::BoolPrefs::PWEasyVision,
                      passwordpolicy.m_pweasyvision == TRUE);

      prefs->SetPref(PWSprefs::BoolPrefs::UseDefUser,
                     username.m_usedefuser == TRUE);
      prefs->SetPref(PWSprefs::StringPrefs::DefUserName,
                     username.m_defusername);
      prefs->SetPref(PWSprefs::BoolPrefs::QuerySetDef,
                     username.m_querysetdef == TRUE);

      prefs->SetPref(PWSprefs::BoolPrefs::DeleteQuestion,
                     misc.m_confirmdelete == FALSE);
      prefs->SetPref(PWSprefs::BoolPrefs::SaveImmediately,
                     misc.m_saveimmediately == TRUE);
      prefs->SetPref(PWSprefs::BoolPrefs::EscExits,
                     misc.m_escexits == TRUE);
      // by strange coincidence, the values of the enums match the indices
      // of the radio buttons in the following :-)
      prefs->SetPref(PWSprefs::IntPrefs::DoubleClickAction,
                     misc.m_doubleclickaction);

      prefs->SetPref(PWSprefs::BoolPrefs::HotKeyEnabled,
                     misc.m_hotkey_enabled == TRUE);
      prefs->SetPref(PWSprefs::IntPrefs::HotKey,
                     misc.m_hotkey_value);

      /* Update status bar */
      UINT statustext;
      switch (misc.m_doubleclickaction) {
      case PWSprefs::DoubleClickCopy: statustext = IDS_STATCOPY; break;
      case PWSprefs::DoubleClickEdit: statustext = IDS_STATEDIT; break;
      case PWSprefs::DoubleClickAutoType: statustext = IDS_STATAUTOTYPE; break;
      case PWSprefs::DoubleClickBrowse: statustext = IDS_STATBROWSE; break;
      default: ASSERT(0);
      }
      // JHF : no status bar under WinCE (was already so in the .h file !?!)
#if !defined(POCKET_PC)
      m_statusBar.SetIndicators(&statustext, 1);	
#endif
      /*
      ** Update string in database, if necessary & possible
      */
      if (prefs->IsChanged() && !app.m_core.GetCurFile().IsEmpty()) {
        // save changed preferences to file
        // Note that we currently can only write the entire file, so any changes
        // the user made to the database are also saved here
        m_core.BackupCurFile(); // try to save previous version
        if (app.m_core.WriteCurFile() != PWScore::SUCCESS)
          MessageBox(_T("Failed to save changed preferences"), AfxGetAppName());
        else
          prefs->ClearChanged();
      }
      /*
      **  Now update the application according to the options.
      */
      m_bAlwaysOnTop = display.m_alwaysontop == TRUE;
      UpdateAlwaysOnTop();
      bool bOldShowPasswordInList = m_bShowPasswordInList;
      m_bShowPasswordInList = prefs->
        GetPref(PWSprefs::BoolPrefs::ShowPWInList);

      if (bOldShowPasswordInList != m_bShowPasswordInList)
		RefreshList();
	
      if (display.m_usesystemtray == TRUE) {
		if (app.m_TrayIcon.Visible() == FALSE)
          app.m_TrayIcon.ShowIcon();
	  } else { // user doesn't want to display
		if (app.m_TrayIcon.Visible() == TRUE)
          app.m_TrayIcon.HideIcon();
      }

      // update idle timeout values, if changed
      if (security.m_LockOnIdleTimeout != prevLockOIT)
        if (security.m_LockOnIdleTimeout == TRUE) {
          const UINT MINUTE = 60*1000;
          SetTimer(TIMER_USERLOCK, MINUTE, NULL);
        } else {
          KillTimer(TIMER_USERLOCK);
        }
      SetIdleLockCounter(security.m_IdleTimeOut);

      // JHF no hotkeys under WinCE
#if !defined(POCKET_PC)
      // Handle HotKey setting
      if (misc.m_hotkey_enabled == TRUE) {
        WORD v;
        v = WORD((misc.m_hotkey_value & 0xff) |
                 ((misc.m_hotkey_value & 0xff0000) >> 8));
        SendMessage(WM_SETHOTKEY, v);
      } else {
        SendMessage(WM_SETHOTKEY, 0);
      }
#endif
      /*
       * Here are the old (pre 2.0) semantics:
       * The username entered in this dialog box will be added to all the entries
       * in the username-less database that you just opened. Click Ok to add the
       * username or Cancel to leave them as is.
       *
       * You can also set this username to be the default username by clicking the
       * check box.  In this case, you will not see the username that you just added
       * in the main dialog (though it is still part of the entries), and it will
       * automatically be inserted in the Add dialog for new entries.
       *
       * To me (ronys), these seem too complicated, and not useful once password files
       * have been converted to the old (username-less) format to 1.9 (with usernames).
       * (Not to mention 2.0).
       * Therefore, the username will now only be a default value to be used in new entries,
       * and in converting pre-2.0 databases.
       */

      m_core.SetDefUsername(username.m_defusername);
      m_core.SetUseDefUser(username.m_usedefuser == TRUE ? true : false);
    }
}

