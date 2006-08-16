/// file MainManage.cpp
//
// Manage-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

#include "corelib/pwsprefs.h"

// dialog boxen
#include "DboxMain.h"

#include "PasskeyChangeDlg.h"
#include "TryAgainDlg.h"
#include "OptionsSystem.h"
#include "OptionsSecurity.h"
#include "OptionsDisplay.h"
#include "OptionsPasswordPolicy.h"
#include "OptionsPasswordHistory.h"
#include "OptionsMisc.h"

#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Change the master password for the database.
void
DboxMain::OnPasswordChange()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;
  CPasskeyChangeDlg changeDlg(this);
  app.DisableAccelerator();
  int rc = changeDlg.DoModal();
  app.EnableAccelerator();
  if (rc == IDOK) {
    m_core.ChangePassword(changeDlg.m_newpasskey);
  }
}

void
DboxMain::OnBackupSafe()
{
  BackupSafe();
}

int
DboxMain::BackupSafe()
{
  int rc;
  PWSprefs *prefs = PWSprefs::GetInstance();
  CMyString tempname;
  CMyString currbackup =
    prefs->GetPref(PWSprefs::CurrentBackup);


  //SaveAs-type dialog box
  while (1) {
    CFileDialog fd(FALSE,
                   _T("bak"),
                   currbackup,
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   | OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   _T("Password Safe Backups (*.bak)|*.bak||"),
                   this);
    fd.m_ofn.lpstrTitle = _T("Please Choose a Name for this Backup:");

    rc = fd.DoModal();
    if (rc == IDOK) {
      tempname = (CMyString)fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  rc = m_core.WriteFile(tempname);
  if (rc == PWScore::CANT_OPEN_FILE) {
    CMyString temp = tempname + _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  prefs->SetPref(PWSprefs::CurrentBackup, tempname);
  return PWScore::SUCCESS;
}

void
DboxMain::OnRestore()
{
  if (!m_IsReadOnly) // disable in read-only mode
    Restore();
}

int
DboxMain::Restore()
{
  int rc;
  CMyString backup, passkey, temp;
  CMyString currbackup =
    PWSprefs::GetInstance()->GetPref(PWSprefs::CurrentBackup);

  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS)
    return rc;

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   _T("bak"),
                   currbackup,
                   OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                   _T("Password Safe Backups (*.bak)|*.bak||"),
                   this);
    fd.m_ofn.lpstrTitle = _T("Please Choose a Backup to restore:");
    rc = fd.DoModal();
    if (rc == IDOK) {
      backup = (CMyString)fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  rc = GetAndCheckPassword(backup, passkey, GCP_NORMAL);  // OK, CANCEL, HELP
  switch (rc) {
  case PWScore::SUCCESS:
    break; // Keep going...
  case PWScore::CANT_OPEN_FILE:
    temp = backup + _T("\n\nCan't open file. Please choose another.");
    MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
  case TAR_OPEN:
    ASSERT(0);
    return PWScore::FAILURE; // shouldn't be an option here
  case TAR_NEW:
    ASSERT(0);
    return PWScore::FAILURE; // shouldn't be an option here
  case PWScore::WRONG_PASSWORD:
  case PWScore::USER_CANCEL:
    /*
      If the user just cancelled out of the password dialog,
      assume they want to return to where they were before...
    */
    return PWScore::USER_CANCEL;
  }

  // unlock the file we're leaving
  if( !m_core.GetCurFile().IsEmpty() ) {
    m_core.UnlockFile(m_core.GetCurFile());
  }

  // clear the data before restoring
  ClearData();

  rc = m_core.ReadFile(backup, passkey);
  if (rc == PWScore::CANT_OPEN_FILE) {
    temp = backup + _T("\n\nCould not open file for reading!");
    MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  m_core.SetCurFile(_T("")); //Force a Save As...
  m_core.SetChanged(Data); //So that the restored file will be saved
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - <Untitled Restored Backup>");
  app.SetTooltipText(_T("PasswordSafe"));
#endif
  ChangeOkUpdate();
  RefreshList();

  return PWScore::SUCCESS;
}

void
DboxMain::OnOptions() 
{
  CPropertySheet optionsDlg(_T("Options"), this);
  COptionsDisplay         display;
  COptionsSecurity        security;
  COptionsPasswordPolicy  passwordpolicy;
  COptionsPasswordHistory passwordhistory;
  COptionsSystem          system;
  COptionsMisc            misc;
  PWSprefs               *prefs = PWSprefs::GetInstance();
  BOOL                   prevLockOIT; // lock on idle timeout set?
  /*
  **  Initialize the property pages values.
  */
  system.m_maxreitems = prefs->
    GetPref(PWSprefs::MaxREItems);
  system.m_usesystemtray = prefs->
    GetPref(PWSprefs::UseSystemTray) ? TRUE : FALSE;
  system.m_maxmruitems = prefs->
    GetPref(PWSprefs::MaxMRUItems);
  system.m_mruonfilemenu = PWSprefs::GetInstance()->
    GetPref(PWSprefs::MRUOnFileMenu);

  display.m_alwaysontop = m_bAlwaysOnTop;
  display.m_pwshowinedit = prefs->
    GetPref(PWSprefs::ShowPWDefault) ? TRUE : FALSE;
  display.m_pwshowinlist = prefs->
    GetPref(PWSprefs::ShowPWInList) ? TRUE : FALSE;
#if defined(POCKET_PC)
  display.m_dcshowspassword = prefs->
    GetPref(PWSprefs::DCShowsPassword) ? TRUE : FALSE;
#endif
  // by strange coincidence, the values of the enums match the indices
  // of the radio buttons in the following :-)
  display.m_treedisplaystatusatopen = prefs->
    GetPref(PWSprefs::TreeDisplayStatusAtOpen);

  security.m_clearclipboard = prefs->
    GetPref(PWSprefs::DontAskMinimizeClearYesNo) ? TRUE : FALSE;
  security.m_lockdatabase = prefs->
    GetPref(PWSprefs::DatabaseClear) ? TRUE : FALSE;
  security.m_confirmcopy = prefs->
    GetPref(PWSprefs::DontAskQuestion) ? FALSE : TRUE;
  security.m_LockOnWindowLock = prefs->
    GetPref(PWSprefs::LockOnWindowLock) ? TRUE : FALSE;
  security.m_LockOnIdleTimeout = prevLockOIT = prefs->
    GetPref(PWSprefs::LockOnIdleTimeout) ? TRUE : FALSE;
  security.m_IdleTimeOut = prefs->
    GetPref(PWSprefs::IdleTimeout);
  security.m_bAllowWeakPassphrases = prefs->
    GetPref(PWSprefs::AllowWeakPassphrases) ? TRUE : FALSE;

  passwordpolicy.m_pwlendefault = prefs->
    GetPref(PWSprefs::PWLenDefault);
  passwordpolicy.m_pwuselowercase = prefs->
    GetPref(PWSprefs::PWUseLowercase);
  passwordpolicy.m_pwuseuppercase = prefs->
    GetPref(PWSprefs::PWUseUppercase);
  passwordpolicy.m_pwusedigits = prefs->
    GetPref(PWSprefs::PWUseDigits);
  passwordpolicy.m_pwusesymbols = prefs->
    GetPref(PWSprefs::PWUseSymbols);
  passwordpolicy.m_pwusehexdigits = prefs->
    GetPref(PWSprefs::PWUseHexDigits);
  passwordpolicy.m_pweasyvision = prefs->
    GetPref(PWSprefs::PWEasyVision);

  passwordhistory.m_savepwhistory = prefs->
    GetPref(PWSprefs::SavePasswordHistory) ? TRUE : FALSE;
  passwordhistory.m_pwhistorynumdefault = prefs->
    GetPref(PWSprefs::NumPWHistoryDefault);

  misc.m_confirmdelete = prefs->
    GetPref(PWSprefs::DeleteQuestion) ? FALSE : TRUE;
  misc.m_saveimmediately = prefs->
    GetPref(PWSprefs::SaveImmediately) ? TRUE : FALSE;
  misc.m_maintaindatetimestamps = prefs->
    GetPref(PWSprefs::MaintainDateTimeStamps) ? TRUE : FALSE;
  misc.m_escexits = prefs->
    GetPref(PWSprefs::EscExits) ? TRUE : FALSE;
  // by strange coincidence, the values of the enums match the indices
  // of the radio buttons in the following :-)
  misc.m_doubleclickaction = prefs->
    GetPref(PWSprefs::DoubleClickAction);
  misc.m_hotkey_value = DWORD(prefs->GetPref(PWSprefs::HotKey));
  misc.m_hotkey_enabled = prefs->
    GetPref(PWSprefs::HotKeyEnabled) ? TRUE : FALSE;
  misc.m_usedefuser = prefs->
    GetPref(PWSprefs::UseDefUser) ? TRUE : FALSE;
  misc.m_defusername = CString(prefs->
    GetPref(PWSprefs::DefUserName));
  misc.m_querysetdef = prefs->
    GetPref(PWSprefs::QuerySetDef) ? TRUE : FALSE;
  misc.m_continuefindateodb = prefs->
    GetPref(PWSprefs::ContinueFindAtEODB) ? TRUE : FALSE;

  optionsDlg.AddPage( &display );
  optionsDlg.AddPage( &misc );
  optionsDlg.AddPage( &passwordpolicy );
  optionsDlg.AddPage( &passwordhistory );
  optionsDlg.AddPage( &security );
  optionsDlg.AddPage( &system );

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
      prefs->SetPref(PWSprefs::AlwaysOnTop,
                     display.m_alwaysontop == TRUE);
      prefs->SetPref(PWSprefs::ShowPWDefault,
                     display.m_pwshowinedit == TRUE);
      prefs->SetPref(PWSprefs::ShowPWInList,
                     display.m_pwshowinlist == TRUE);
#if defined(POCKET_PC)
      prefs->SetPref(PWSprefs::DCShowsPassword,
                     display.m_dcshowspassword == TRUE);
#endif
      // by strange coincidence, the values of the enums match the indices
      // of the radio buttons in the following :-)
      prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen,
                     display.m_treedisplaystatusatopen);

      prefs->SetPref(PWSprefs::UseSystemTray,
                     system.m_usesystemtray == TRUE);
      prefs->SetPref(PWSprefs::MaxREItems,
                     system.m_maxreitems);
      prefs->SetPref(PWSprefs::MaxMRUItems,
                     system.m_maxmruitems);
      prefs->SetPref(PWSprefs::MRUOnFileMenu,
                     system.m_mruonfilemenu == TRUE);

      prefs->SetPref(PWSprefs::DontAskMinimizeClearYesNo,
                     security.m_clearclipboard == TRUE);
      prefs->SetPref(PWSprefs::DatabaseClear,
                     security.m_lockdatabase == TRUE);
      prefs->SetPref(PWSprefs::DontAskQuestion,
                     security.m_confirmcopy == FALSE);
      prefs->SetPref(PWSprefs::LockOnWindowLock,
                     security.m_LockOnWindowLock == TRUE);
      prefs->SetPref(PWSprefs::LockOnIdleTimeout,
                     security.m_LockOnIdleTimeout == TRUE);
      prefs->SetPref(PWSprefs::IdleTimeout,
                     security.m_IdleTimeOut);
      prefs->SetPref(PWSprefs::AllowWeakPassphrases,
                     security.m_bAllowWeakPassphrases == TRUE);

      prefs->SetPref(PWSprefs::PWLenDefault,
                     passwordpolicy.m_pwlendefault);
      prefs->SetPref(PWSprefs::PWUseLowercase,
                     passwordpolicy.m_pwuselowercase == TRUE);
      prefs->SetPref(PWSprefs::PWUseUppercase,
                     passwordpolicy.m_pwuseuppercase == TRUE);
      prefs->SetPref(PWSprefs::PWUseDigits,
                     passwordpolicy.m_pwusedigits == TRUE);
      prefs->SetPref(PWSprefs::PWUseSymbols,
                     passwordpolicy.m_pwusesymbols == TRUE);
      prefs->SetPref(PWSprefs::PWUseHexDigits,
                     passwordpolicy.m_pwusehexdigits == TRUE);
      prefs-> SetPref(PWSprefs::PWEasyVision,
                      passwordpolicy.m_pweasyvision == TRUE);

      prefs->SetPref(PWSprefs::SavePasswordHistory,
      				 passwordhistory.m_savepwhistory == TRUE);
      if (passwordhistory.m_savepwhistory == TRUE)
          prefs->SetPref(PWSprefs::NumPWHistoryDefault,
                     passwordhistory.m_pwhistorynumdefault);

      prefs->SetPref(PWSprefs::DeleteQuestion,
                     misc.m_confirmdelete == FALSE);
      prefs->SetPref(PWSprefs::SaveImmediately,
                     misc.m_saveimmediately == TRUE);
      prefs->SetPref(PWSprefs::MaintainDateTimeStamps,
                     misc.m_maintaindatetimestamps == TRUE);
      prefs->SetPref(PWSprefs::EscExits,
                     misc.m_escexits == TRUE);
      // by strange coincidence, the values of the enums match the indices
      // of the radio buttons in the following :-)
      prefs->SetPref(PWSprefs::DoubleClickAction,
                     misc.m_doubleclickaction);
      prefs->SetPref(PWSprefs::HotKeyEnabled,
                     misc.m_hotkey_enabled == TRUE);
      prefs->SetPref(PWSprefs::HotKey,
                     misc.m_hotkey_value);
      prefs->SetPref(PWSprefs::UseDefUser,
                     misc.m_usedefuser == TRUE);
      prefs->SetPref(PWSprefs::DefUserName,
                     misc.m_defusername);
      prefs->SetPref(PWSprefs::QuerySetDef,
                     misc.m_querysetdef == TRUE);
      prefs->SetPref(PWSprefs::ContinueFindAtEODB,
                     misc.m_continuefindateodb == TRUE);

      if (passwordhistory.m_applypwhistory == TRUE) {
          TCHAR buffer[6];
#if _MSC_VER >= 1400
          _stprintf_s(buffer, 6, "1%02x00", passwordhistory.m_pwhistorynumdefault);
#else
          _stprintf(buffer, "1%02x00", passwordhistory.m_pwhistorynumdefault);
#endif
          POSITION listPos = m_core.GetFirstEntryPosition();
          int i = 0;
          while (listPos != NULL) {
              CItemData &ci = m_core.GetEntryAt(listPos);
              const CMyString tmp = ci.GetPWHistory();
              if (tmp.GetLength() < 5 || tmp == _T("00000")) {
                  ci.SetPWHistory(buffer);
                  i++;
              }
              m_core.GetNextEntry(listPos);
          } // while
          CString csMsg;
          csMsg.Format(_T("%d entries have had their settings changed to save password history."), i);
          AfxMessageBox(csMsg);
      }

      /* Update status bar */
      switch (misc.m_doubleclickaction) {
      	case PWSprefs::DoubleClickCopy: statustext[0] = IDS_STATCOPY; break;
      	case PWSprefs::DoubleClickEdit: statustext[0] = IDS_STATEDIT; break;
      	case PWSprefs::DoubleClickAutoType: statustext[0] = IDS_STATAUTOTYPE; break;
      	case PWSprefs::DoubleClickBrowse: statustext[0] = IDS_STATBROWSE; break;
      	default: ASSERT(0);
      }
      // JHF : no status bar under WinCE (was already so in the .h file !?!)
#if !defined(POCKET_PC)
      m_statusBar.SetIndicators(statustext, 3);
	  UpdateStatusBar();
	  // Make a sunken or recessed border around the first pane
      m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
#endif
      /*
      ** Update string in database, if necessary & possible
      */
      if (prefs->IsChanged() && !app.m_core.GetCurFile().IsEmpty() &&
          m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
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
        GetPref(PWSprefs::ShowPWInList);

      if (bOldShowPasswordInList != m_bShowPasswordInList)
		RefreshList();
	
      if (system.m_usesystemtray == TRUE) {
		if (app.IsIconVisible() == FALSE)
          app.ShowIcon();
	  } else { // user doesn't want to display
		if (app.IsIconVisible() == TRUE)
          app.HideIcon();
      }
      m_RUEList.SetMax(system.m_maxreitems);

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

      m_core.SetDefUsername(misc.m_defusername);
      m_core.SetUseDefUser(misc.m_usedefuser == TRUE ? true : false);
    }
}
