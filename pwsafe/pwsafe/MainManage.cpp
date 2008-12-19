/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// file MainManage.cpp
//
// Manage-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "Shortcut.h"
#include "corelib/pwsprefs.h"
#include "corelib/PWSdirs.h"

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
#include "OptionsBackup.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Change the master password for the database.
void DboxMain::OnPasswordChange()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;
  CPasskeyChangeDlg changeDlg(this);
  app.DisableAccelerator();
  INT_PTR rc = changeDlg.DoModal();
  app.EnableAccelerator();
  if (rc == IDOK) {
    m_core.ChangePassword(changeDlg.m_newpasskey);
  }
}

void DboxMain::OnBackupSafe()
{
  BackupSafe();
}

int DboxMain::BackupSafe()
{
  INT_PTR rc;
  PWSprefs *prefs = PWSprefs::GetInstance();
  StringX tempname;
  StringX currbackup = prefs->GetPref(PWSprefs::CurrentBackup);

  CString cs_text(MAKEINTRESOURCE(IDS_PICKBACKUP));
  CString cs_temp, cs_title;
  //SaveAs-type dialog box
  while (1) {
    CFileDialog fd(FALSE,
                   _T("bak"),
                   currbackup.c_str(),
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   | OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   _T("Password Safe Backups (*.bak)|*.bak||"),
                   this);
    fd.m_ofn.lpstrTitle = cs_text;
    stringT dir = PWSdirs::GetSafeDir();
    if (!dir.empty())
      fd.m_ofn.lpstrInitialDir = dir.c_str();

    rc = fd.DoModal();
    if (m_inExit) {
      // If U3ExitNow called while in CFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling 
      PostQuitMessage(0);
      return PWScore::USER_CANCEL;
    }
    if (rc == IDOK) {
      tempname = fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  rc = m_core.WriteFile(tempname);
  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, tempname);
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  prefs->SetPref(PWSprefs::CurrentBackup, tempname);
  return PWScore::SUCCESS;
}

void DboxMain::OnRestore()
{
  if (!m_core.IsReadOnly()) // disable in read-only mode
    Restore();
}

int DboxMain::Restore()
{
  int rc;
  StringX backup, passkey, temp;
  StringX currbackup =
    PWSprefs::GetInstance()->GetPref(PWSprefs::CurrentBackup);

  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS)
    return rc;

  CString cs_text, cs_temp, cs_title;
  cs_text.LoadString(IDS_PICKRESTORE);
  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   _T("bak"),
                   currbackup.c_str(),
                   OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                   _T("Password Safe Backups (*.bak)|*.bak|")
                   _T("Password Safe Intermediate Backups (*.ibak)|*.ibak|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = cs_text;
    stringT dir = PWSdirs::GetSafeDir();
    if (!dir.empty())
      fd.m_ofn.lpstrInitialDir = dir.c_str();

    INT_PTR rc2 = fd.DoModal();
    if (m_inExit) {
      // If U3ExitNow called while in CFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling 
      PostQuitMessage(0);
      return PWScore::USER_CANCEL;
    }
    if (rc2 == IDOK) {
      backup = fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  rc = GetAndCheckPassword(backup, passkey, GCP_NORMAL);  // OK, CANCEL, HELP
  switch (rc) {
    case PWScore::SUCCESS:
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPEN, backup);
      cs_title.LoadString(IDS_FILEOPENERROR);
      MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
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
  if( !m_core.GetCurFile().empty() ) {
    m_core.UnlockFile(m_core.GetCurFile().c_str());
  }

  // clear the data before restoring
  ClearData();

  rc = m_core.ReadFile(backup, passkey);
  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENREADING, backup);
    cs_title.LoadString(IDS_FILEREADERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  m_core.SetCurFile(_T("")); //Force a Save As...
  m_core.SetChanged(Data); //So that the restored file will be saved
#if !defined(POCKET_PC)
  m_titlebar.LoadString(IDS_UNTITLEDRESTORE);
  app.SetTooltipText(_T("PasswordSafe"));
#endif
  ChangeOkUpdate();
  RefreshViews();

  return PWScore::SUCCESS;
}

void DboxMain::OnValidate() 
{
  stringT cs_msg;
  if (!m_core.Validate(cs_msg))
    LoadAString(cs_msg, IDS_VALIDATEOK);

  AfxMessageBox(cs_msg.c_str(), MB_OK);
}

void DboxMain::OnOptions() 
{
  const CString PWSLnkName(_T("Password Safe")); // for startup shortcut
  CPropertySheet optionsDlg(IDS_OPTIONS, this);
  COptionsDisplay         display;
  COptionsSecurity        security;
  COptionsPasswordPolicy  passwordpolicy;
  COptionsPasswordHistory passwordhistory;
  COptionsSystem          system;
  COptionsMisc            misc;
  COptionsBackup          backup;
  PWSprefs               *prefs = PWSprefs::GetInstance();
  BOOL                    prevLockOIT; // lock on idle timeout set?
  BOOL                    brc, save_hotkey_enabled;
  BOOL                    save_preexpirywarn;
  DWORD                   save_hotkey_value;
  int                     save_preexpirywarndays;
  CShortcut shortcut;
  BOOL StartupShortcutExists = shortcut.isLinkExist(PWSLnkName, CSIDL_STARTUP);

  // Need to compare pre-post values for some:
  const bool bOldShowUsernameInTree = 
               prefs->GetPref(PWSprefs::ShowUsernameInTree);
  const bool bOldShowPasswordInTree = 
               prefs->GetPref(PWSprefs::ShowPasswordInTree);
  const bool bOldExplorerTypeTree = 
               prefs->GetPref(PWSprefs::ExplorerTypeTree);
  /*
  **  Initialize the property pages values.
  */
  system.m_maxreitems = prefs->
    GetPref(PWSprefs::MaxREItems);
  system.m_usesystemtray = prefs->
    GetPref(PWSprefs::UseSystemTray) ? TRUE : FALSE;
  system.m_maxmruitems = prefs->
    GetPref(PWSprefs::MaxMRUItems);
  system.m_mruonfilemenu = prefs->
    GetPref(PWSprefs::MRUOnFileMenu);
  system.m_startup = StartupShortcutExists;
  system.m_defaultopenro = prefs->
    GetPref(PWSprefs::DefaultOpenRO) ? TRUE : FALSE;
  system.m_multipleinstances = prefs->
    GetPref(PWSprefs::MultipleInstances) ? TRUE : FALSE;

  display.m_alwaysontop = prefs->
    GetPref(PWSprefs::AlwaysOnTop) ? TRUE : FALSE;
  display.m_pwshowinedit = prefs->
    GetPref(PWSprefs::ShowPWDefault) ? TRUE : FALSE;
  display.m_showusernameintree = prefs->
    GetPref(PWSprefs::ShowUsernameInTree) ? TRUE : FALSE;
  display.m_showpasswordintree = prefs->
    GetPref(PWSprefs::ShowPasswordInTree) ? TRUE : FALSE;
  display.m_shownotesastipsinviews = prefs->
    GetPref(PWSprefs::ShowNotesAsTooltipsInViews) ? TRUE : FALSE;
  display.m_explorertree = prefs->
    GetPref(PWSprefs::ExplorerTypeTree) ? TRUE : FALSE;
  display.m_enablegrid = prefs->
    GetPref(PWSprefs::ListViewGridLines) ? TRUE : FALSE;
  display.m_notesshowinedit = prefs->
    GetPref(PWSprefs::ShowNotesDefault) ? TRUE : FALSE;
  display.m_preexpirywarn = prefs->
    GetPref(PWSprefs::PreExpiryWarn) ? TRUE : FALSE;
  display.m_preexpirywarndays = prefs->
    GetPref(PWSprefs::PreExpiryWarnDays);
  save_preexpirywarn = display.m_preexpirywarn;
  save_preexpirywarndays = display.m_preexpirywarndays;
#if defined(POCKET_PC)
  display.m_dcshowspassword = prefs->
    GetPref(PWSprefs::DCShowsPassword) ? TRUE : FALSE;
#endif
  // by strange coincidence, the values of the enums match the indices
  // of the radio buttons in the following :-)
  display.m_treedisplaystatusatopen = prefs->
    GetPref(PWSprefs::TreeDisplayStatusAtOpen);
  display.m_trayiconcolour = prefs->
    GetPref(PWSprefs::ClosedTrayIconColour);

  security.m_clearclipboardonminimize = prefs->
    GetPref(PWSprefs::ClearClipboardOnMinimize) ? TRUE : FALSE;
  security.m_clearclipboardonexit = prefs->
    GetPref(PWSprefs::ClearClipboardOnExit) ? TRUE : FALSE;
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
    GetPref(PWSprefs::PWUseEasyVision);
  passwordpolicy.m_pwmakepronounceable = prefs->
    GetPref(PWSprefs::PWMakePronounceable);
  passwordpolicy.m_pwdefaultlength = prefs->
    GetPref(PWSprefs::PWDefaultLength);
  passwordpolicy.m_pwdigitminlength = prefs->
    GetPref(PWSprefs::PWDigitMinLength);
  passwordpolicy.m_pwlowerminlength = prefs->
    GetPref(PWSprefs::PWLowercaseMinLength);
  passwordpolicy.m_pwsymbolminlength = prefs->
    GetPref(PWSprefs::PWSymbolMinLength);
  passwordpolicy.m_pwupperminlength = prefs->
    GetPref(PWSprefs::PWUppercaseMinLength);

  passwordhistory.m_savepwhistory = prefs->
    GetPref(PWSprefs::SavePasswordHistory) ? TRUE : FALSE;
  passwordhistory.m_pwhistorynumdefault = prefs->
    GetPref(PWSprefs::NumPWHistoryDefault);

  misc.m_confirmdelete = prefs->
    GetPref(PWSprefs::DeleteQuestion) ? FALSE : TRUE;
  misc.m_maintaindatetimestamps = prefs->
    GetPref(PWSprefs::MaintainDateTimeStamps) ? TRUE : FALSE;
  misc.m_escexits = prefs->
    GetPref(PWSprefs::EscExits) ? TRUE : FALSE;
  // by strange coincidence, the values of the enums match the indices
  // of the radio buttons in the following :-)
  misc.m_doubleclickaction = prefs->
    GetPref(PWSprefs::DoubleClickAction);

  save_hotkey_value = misc.m_hotkey_value = 
    DWORD(prefs->GetPref(PWSprefs::HotKey));
  // Can't be enabled if not set!
  if (misc.m_hotkey_value == 0)
    save_hotkey_enabled = misc.m_hotkey_enabled = FALSE;
  else
    save_hotkey_enabled = misc.m_hotkey_enabled = prefs->
      GetPref(PWSprefs::HotKeyEnabled) ? TRUE : FALSE;

  misc.m_usedefuser = prefs->
    GetPref(PWSprefs::UseDefaultUser) ? TRUE : FALSE;
  misc.m_defusername = prefs->
    GetPref(PWSprefs::DefaultUsername).c_str();
  misc.m_querysetdef = prefs->
    GetPref(PWSprefs::QuerySetDef) ? TRUE : FALSE;
  misc.m_csBrowser = prefs->
    GetPref(PWSprefs::AltBrowser).c_str();
  misc.m_csBrowserCmdLineParms = prefs->
    GetPref(PWSprefs::AltBrowserCmdLineParms).c_str();
  CString dats = prefs->
    GetPref(PWSprefs::DefaultAutotypeString).c_str();
  if (dats.IsEmpty())
    dats = DEFAULT_AUTOTYPE;
  misc.m_csAutotype = CString(dats);
  misc.m_minauto = prefs->
    GetPref(PWSprefs::MinimizeOnAutotype) ? TRUE : FALSE;                               

  backup.SetCurFile(m_core.GetCurFile().c_str());
  backup.m_saveimmediately = prefs->
    GetPref(PWSprefs::SaveImmediately) ? TRUE : FALSE;
  backup.m_backupbeforesave = prefs->
    GetPref(PWSprefs::BackupBeforeEverySave) ? TRUE : FALSE;
  CString backupPrefix(prefs->
                       GetPref(PWSprefs::BackupPrefixValue).c_str());
  backup.m_backupprefix = backupPrefix.IsEmpty() ? 0 : 1;
  backup.m_userbackupprefix = backupPrefix;
  backup.m_backupsuffix = prefs->
    GetPref(PWSprefs::BackupSuffix);
  backup.m_maxnumincbackups = prefs->
    GetPref(PWSprefs::BackupMaxIncremented);
  CString backupDir(prefs->GetPref(PWSprefs::BackupDir).c_str());
  backup.m_backuplocation = backupDir.IsEmpty() ? 0 : 1;
  backup.m_userbackupotherlocation = backupDir;

  optionsDlg.AddPage(&backup);
  optionsDlg.AddPage(&display);
  optionsDlg.AddPage(&misc);
  optionsDlg.AddPage(&passwordpolicy);
  optionsDlg.AddPage(&passwordhistory);
  optionsDlg.AddPage(&security);
  optionsDlg.AddPage(&system);

  /*
  **  Remove the "Apply Now" button.
  */
  optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  // Disable Hotkey around this as the user may press the current key when 
  // selecting the new key!

#if !defined(POCKET_PC)
  brc = UnregisterHotKey(m_hWnd, PWS_HOTKEY_ID); // clear last - never hurts
#endif

  passwordhistory.m_pDboxMain = this;
  app.DisableAccelerator();
  INT_PTR rc = optionsDlg.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    /*
    **  First check user hasn't introduced a conflict
    */
    if ((misc.m_doubleclickaction == PWSprefs::DoubleClickCopyPasswordMinimize) &&
         security.m_clearclipboardonminimize == TRUE) {
      misc.m_doubleclickaction = PWSprefs::DoubleClickCopyPassword;
      AfxMessageBox(IDS_MINIMIZECONFLICT);
    }
    /*
    **  Now save all the options.
    */
    prefs->SetPref(PWSprefs::AlwaysOnTop,
      display.m_alwaysontop == TRUE);
    prefs->SetPref(PWSprefs::ShowPWDefault,
      display.m_pwshowinedit == TRUE);
    prefs->SetPref(PWSprefs::ShowUsernameInTree,
      display.m_showusernameintree == TRUE);
    prefs->SetPref(PWSprefs::ShowPasswordInTree,
      display.m_showpasswordintree == TRUE);
    prefs->SetPref(PWSprefs::ShowNotesAsTooltipsInViews,
      display.m_shownotesastipsinviews == TRUE);
    prefs->SetPref(PWSprefs::ExplorerTypeTree,
      display.m_explorertree == TRUE);
    prefs->SetPref(PWSprefs::ListViewGridLines,
      display.m_enablegrid == TRUE);
    prefs->SetPref(PWSprefs::ShowNotesDefault,
      display.m_notesshowinedit == TRUE);
    prefs->SetPref(PWSprefs::PreExpiryWarn,
      display.m_preexpirywarn == TRUE);
    prefs->SetPref(PWSprefs::PreExpiryWarnDays,
      display.m_preexpirywarndays);
#if defined(POCKET_PC)
    prefs->SetPref(PWSprefs::DCShowsPassword,
      display.m_dcshowspassword == TRUE);
#endif
    // by strange coincidence, the values of the enums match the indices
    // of the radio buttons in the following :-)
    prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen,
      display.m_treedisplaystatusatopen);
    prefs->SetPref(PWSprefs::ClosedTrayIconColour,
      display.m_trayiconcolour);
    app.SetClosedTrayIcon(display.m_trayiconcolour);

    prefs->SetPref(PWSprefs::UseSystemTray,
      system.m_usesystemtray == TRUE);
    prefs->SetPref(PWSprefs::MaxREItems,
      system.m_maxreitems);
    if (system.m_maxreitems == 0) {
      // Put them on File menu where they don't take up any room
      prefs->SetPref(PWSprefs::MRUOnFileMenu, true);
      // Clear any currently saved
      app.ClearMRU();
    } else {
      // Otherwise use what the user wanted.
      prefs->SetPref(PWSprefs::MaxMRUItems,
        system.m_maxmruitems);
      prefs->SetPref(PWSprefs::MRUOnFileMenu,
        system.m_mruonfilemenu == TRUE);
    }
    prefs->SetPref(PWSprefs::DefaultOpenRO,
      system.m_defaultopenro == TRUE);
    prefs->SetPref(PWSprefs::MultipleInstances,
      system.m_multipleinstances == TRUE);

    prefs->SetPref(PWSprefs::ClearClipboardOnMinimize,
      security.m_clearclipboardonminimize == TRUE);
    prefs->SetPref(PWSprefs::ClearClipboardOnExit,
      security.m_clearclipboardonexit == TRUE);
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
    prefs->SetPref(PWSprefs::PWUseEasyVision,
      passwordpolicy.m_pweasyvision == TRUE);
    prefs->SetPref(PWSprefs::PWMakePronounceable,
      passwordpolicy.m_pwmakepronounceable == TRUE);
    prefs->SetPref(PWSprefs::PWDefaultLength,
      passwordpolicy.m_pwdefaultlength);
    prefs->SetPref(PWSprefs::PWDigitMinLength,
      passwordpolicy.m_pwdigitminlength);
    prefs->SetPref(PWSprefs::PWLowercaseMinLength,
      passwordpolicy.m_pwlowerminlength);
    prefs->SetPref(PWSprefs::PWSymbolMinLength,
      passwordpolicy.m_pwsymbolminlength);
    prefs->SetPref(PWSprefs::PWUppercaseMinLength,
      passwordpolicy.m_pwupperminlength);

    prefs->SetPref(PWSprefs::SavePasswordHistory,
      passwordhistory.m_savepwhistory == TRUE);
    if (passwordhistory.m_savepwhistory == TRUE)
      prefs->SetPref(PWSprefs::NumPWHistoryDefault,
      passwordhistory.m_pwhistorynumdefault);

    prefs->SetPref(PWSprefs::DeleteQuestion,
      misc.m_confirmdelete == FALSE);
    prefs->SetPref(PWSprefs::MaintainDateTimeStamps,
      misc.m_maintaindatetimestamps == TRUE);
    prefs->SetPref(PWSprefs::EscExits,
      misc.m_escexits == TRUE);
    // by strange coincidence, the values of the enums match the indices
    // of the radio buttons in the following :-)
    prefs->SetPref(PWSprefs::DoubleClickAction,
      (unsigned int)misc.m_doubleclickaction);

    // Need to update previous values as we use these variables to re-instate
    // the hotkey environment at the end whether the user changed it or not.
    prefs->SetPref(PWSprefs::HotKey,
      misc.m_hotkey_value);
    save_hotkey_value = misc.m_hotkey_value;
    // Can't be enabled if not set!
    if (misc.m_hotkey_value == 0)
      save_hotkey_enabled = misc.m_hotkey_enabled = FALSE;

    prefs->SetPref(PWSprefs::HotKeyEnabled,
      misc.m_hotkey_enabled == TRUE);
    prefs->SetPref(PWSprefs::UseDefaultUser,
      misc.m_usedefuser == TRUE);
    prefs->SetPref(PWSprefs::DefaultUsername,
                   LPCTSTR(misc.m_defusername));
    prefs->SetPref(PWSprefs::QuerySetDef,
      misc.m_querysetdef == TRUE);
    prefs->SetPref(PWSprefs::AltBrowser,
                   LPCTSTR(misc.m_csBrowser));
    prefs->SetPref(PWSprefs::AltBrowserCmdLineParms,
                   LPCTSTR(misc.m_csBrowserCmdLineParms));
    if (misc.m_csAutotype.IsEmpty() || misc.m_csAutotype == DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString, _T(""));
    else if (misc.m_csAutotype != DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString,
                     LPCTSTR(misc.m_csAutotype));
    prefs->SetPref(PWSprefs::MinimizeOnAutotype,
      misc.m_minauto == TRUE);

    prefs->SetPref(PWSprefs::SaveImmediately,
      backup.m_saveimmediately == TRUE);
    prefs->SetPref(PWSprefs::BackupBeforeEverySave,
      backup.m_backupbeforesave == TRUE);
    prefs->SetPref(PWSprefs::BackupPrefixValue,
                   LPCTSTR(backup.m_userbackupprefix));
    prefs->SetPref(PWSprefs::BackupSuffix,
      (unsigned int)backup.m_backupsuffix);
    prefs->SetPref(PWSprefs::BackupMaxIncremented,
      backup.m_maxnumincbackups);
    prefs->SetPref(PWSprefs::BackupDir,
                   LPCTSTR(backup.m_userbackupotherlocation));

    // JHF : no status bar under WinCE (was already so in the .h file !?!)
#if !defined(POCKET_PC)
    /* Update status bar */
    switch (misc.m_doubleclickaction) {
      case PWSprefs::DoubleClickAutoType:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATAUTOTYPE; break;
      case PWSprefs::DoubleClickBrowse:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATBROWSE; break;
      case PWSprefs::DoubleClickCopyNotes:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYNOTES; break;
      case PWSprefs::DoubleClickCopyPassword:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYPASSWORD; break;
      case PWSprefs::DoubleClickCopyUsername:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYUSERNAME; break;
      case PWSprefs::DoubleClickViewEdit:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATVIEWEDIT; break;
	  case PWSprefs::DoubleClickCopyPasswordMinimize:
		statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOPYPASSWORDMIN; break;
      default:
        statustext[CPWStatusBar::SB_DBLCLICK] = IDS_STATCOMPANY;
    }
    m_statusBar.SetIndicators(statustext, CPWStatusBar::SB_TOTAL);
    UpdateStatusBar();
    // Make a sunken or recessed border around the first pane
    m_statusBar.SetPaneInfo(CPWStatusBar::SB_DBLCLICK,
                            m_statusBar.GetItemID(CPWStatusBar::SB_DBLCLICK),
                            SBPS_STRETCH, NULL);
#endif

    /*
    ** Update string in database, if necessary & possible (i.e. ignore if R-O)
    */
    if (prefs->IsDBprefsChanged() && !m_core.GetCurFile().empty() &&
        m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
      if (!m_core.IsReadOnly()) {
        const StringX prefString(prefs->Store());
        SetChanged(m_core.HaveHeaderPreferencesChanged(prefString) ? DBPrefs : ClearDBPrefs);
        ChangeOkUpdate();
      }
    }
    /*
    **  Now update the application according to the options.
    */
    UpdateAlwaysOnTop();

    DWORD dwExtendedStyle = m_ctlItemList.GetExtendedStyle();
    BOOL bGridLines = ((dwExtendedStyle & LVS_EX_GRIDLINES) == LVS_EX_GRIDLINES) ? TRUE : FALSE;

    if (display.m_enablegrid != bGridLines) {
      if (display.m_enablegrid) {
        dwExtendedStyle |= LVS_EX_GRIDLINES;
      } else {
        dwExtendedStyle &= ~LVS_EX_GRIDLINES;
      }
      m_ctlItemList.SetExtendedStyle(dwExtendedStyle);
    }

    if ((bOldShowUsernameInTree !=
           prefs->GetPref(PWSprefs::ShowUsernameInTree) ||
         bOldShowPasswordInTree !=
           prefs->GetPref(PWSprefs::ShowPasswordInTree)) ||
        (bOldExplorerTypeTree !=
           prefs->GetPref(PWSprefs::ExplorerTypeTree)) ||
        (save_preexpirywarn != display.m_preexpirywarn) ||
        (save_preexpirywarndays != display.m_preexpirywarndays))
      RefreshViews();

    if (display.m_shownotesastipsinviews == TRUE) {
      m_ctlItemTree.ActivateND(true);
      m_ctlItemList.ActivateND(true);
    } else {
      m_ctlItemTree.ActivateND(false);
      m_ctlItemList.ActivateND(false);
    }

    // Changing ExplorerTypeTree changes order of items,
    // which DisplayStatus implicitly depends upon
    if (bOldExplorerTypeTree !=
        prefs->GetPref(PWSprefs::ExplorerTypeTree))
      SaveDisplayStatus();

    if (system.m_usesystemtray == TRUE) {
      if (app.IsIconVisible() == FALSE)
        app.ShowIcon();
    } else { // user doesn't want to display
      if (app.IsIconVisible() == TRUE)
        app.HideIcon();
    }
    m_RUEList.SetMax(system.m_maxreitems);

    if (system.m_startup != StartupShortcutExists) {
      if (system.m_startup == TRUE) {
        TCHAR exeName[MAX_PATH];
        GetModuleFileName(NULL, exeName, MAX_PATH);
        shortcut.SetCmdArguments(CString(_T("-s")));
        shortcut.CreateShortCut(exeName, PWSLnkName, CSIDL_STARTUP);
      } else { // remove existing startup shortcut
        shortcut.DeleteShortCut(PWSLnkName, CSIDL_STARTUP);
      }
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

      m_core.SetDefUsername(misc.m_defusername.GetString());
      m_core.SetUseDefUser(misc.m_usedefuser == TRUE ? true : false);
      // Finally, keep prefs file updated:
      prefs->SaveApplicationPreferences();
  }
  // JHF no hotkeys under WinCE
#if !defined(POCKET_PC)
  // Restore hotkey as it was or as user changed it - if he/she pressed OK
  if (save_hotkey_enabled == TRUE) {
    WORD wVirtualKeyCode = WORD(save_hotkey_value & 0xffff);
    WORD mod = WORD(save_hotkey_value >> 16);
    WORD wModifiers = 0;
    // Translate between CWnd & CHotKeyCtrl modifiers
    if (mod & HOTKEYF_ALT) 
      wModifiers |= MOD_ALT; 
    if (mod & HOTKEYF_CONTROL) 
      wModifiers |= MOD_CONTROL; 
    if (mod & HOTKEYF_SHIFT) 
      wModifiers |= MOD_SHIFT; 
    brc = RegisterHotKey(m_hWnd, PWS_HOTKEY_ID,
      UINT(wModifiers), UINT(wVirtualKeyCode));
    if (brc == FALSE)
      AfxMessageBox(IDS_NOHOTKEY, MB_OK);
  }
#endif
}

// functor objects for updating password history for each entry

struct HistoryUpdater {
  HistoryUpdater(int &num_altered) : m_num_altered(num_altered)
  {}
  virtual void operator() (CItemData &ci) = 0;
protected:
  int &m_num_altered;
};

struct HistoryUpdateResetOff : public HistoryUpdater {
  HistoryUpdateResetOff(int &num_altered) : HistoryUpdater(num_altered) {}
  void operator()(CItemData &ci)
  {
    StringX cs_tmp = ci.GetPWHistory();
    if (cs_tmp.length() >= 5 && cs_tmp[0] == _T('1')) {
      cs_tmp[0] = _T('0');
      ci.SetPWHistory(cs_tmp);
      m_num_altered++;
    }
  }
};

struct HistoryUpdateResetOn : public HistoryUpdater {
  HistoryUpdateResetOn(int &num_altered,
    int new_default_max) : HistoryUpdater(num_altered)
  {m_text.Format(_T("1%02x00"), new_default_max);}
  void operator()(CItemData &ci)
  {
    StringX cs_tmp = ci.GetPWHistory();
    if (cs_tmp.length() < 5) {
      ci.SetPWHistory(LPCTSTR(m_text));
      m_num_altered++;
    } else {
      if (cs_tmp[0] == _T('0')) {
        cs_tmp[0] = _T('1');
        ci.SetPWHistory(cs_tmp);
        m_num_altered++;
      }
    }
  }
private:
  CString m_text;
};

struct HistoryUpdateSetMax : public HistoryUpdater {
  HistoryUpdateSetMax(int &num_altered,
    int new_default_max) : HistoryUpdater(num_altered),
    m_new_default_max(new_default_max)
  {m_text.Format(_T("1%02x"), new_default_max);}
  void operator()(CItemData &ci)
  {
    StringX cs_tmp = ci.GetPWHistory();

    int len = cs_tmp.length();
    if (len >= 5) {
      int status, old_max, num_saved;
      const TCHAR *lpszPWHistory = cs_tmp.c_str();
#if _MSC_VER >= 1400
      int iread = _stscanf_s(lpszPWHistory, _T("%01d%02x%02x"), 
                             &status, &old_max, &num_saved);
#else
      int iread = _stscanf(lpszPWHistory, _T("%01d%02x%02x"),
                           &status, &old_max, &num_saved);
#endif
      if (iread == 3 && status == 1 && num_saved <= m_new_default_max) {
        cs_tmp = LPCTSTR(m_text) + cs_tmp.substr(3);
        ci.SetPWHistory(cs_tmp);
        m_num_altered++;
      }
    }
  }
private:
  int m_new_default_max;
  CString m_text;
};

void DboxMain::UpdatePasswordHistory(int iAction, int new_default_max)
{
  int ids = 0;
  int num_altered = 0;
  HistoryUpdater *updater = NULL;

  HistoryUpdateResetOff reset_off(num_altered);
  HistoryUpdateResetOn reset_on(num_altered, new_default_max);
  HistoryUpdateSetMax set_max(num_altered, new_default_max);

  switch (iAction) {
    case 1:   // reset off
      updater = &reset_off;
      ids = IDS_ENTRIESCHANGEDSTOP;
      break;
    case 2:   // reset on
      updater = &reset_on;
      ids = IDS_ENTRIESCHANGEDSAVE;
      break;
    case 3:   // setmax
      updater = &set_max;
      ids = IDS_ENTRIESRESETMAX;
      break;
    default:
      ASSERT(0);
      break;
  } // switch (iAction)

  /**
  * Interesting problem - a for_each iterator
  * cause a copy c'tor of the pair to be invoked, resulting
  * in a temporary copy of the CItemDatum being modified.
  * Couldn't find a handy way to workaround this (e.g.,
  * operator()(pair<...> &p) failed to compile
  * so reverted to slightly less elegant for loop
  * using polymorphism for the history updater
  * is an unrelated tweak.
  */

  if (updater != NULL) {
    ItemListIter listPos;
    for (listPos = m_core.GetEntryIter();
      listPos != m_core.GetEntryEndIter();
      listPos++) {
        CItemData &curitem = m_core.GetEntry(listPos);
        (*updater)(curitem);
    }

    CString cs_Msg;
    cs_Msg.Format(ids, num_altered);
    AfxMessageBox(cs_Msg);
  }
}
