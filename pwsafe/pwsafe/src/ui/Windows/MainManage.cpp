/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "GeneralMsgBox.h"
#include "Shortcut.h"
#include "PWFileDialog.h"
#include "PWPropertySheet.h"
#include "DboxMain.h"
#include "PasskeyChangeDlg.h"
#include "TryAgainDlg.h"
#include "Options_PropertySheet.h"
#include "OptionsSystem.h"
#include "OptionsSecurity.h"
#include "OptionsDisplay.h"
#include "OptionsPasswordPolicy.h"
#include "OptionsPasswordHistory.h"
#include "OptionsMisc.h"
#include "OptionsBackup.h"
#include "OptionsShortcuts.h"

#include "core/pwsprefs.h"
#include "core/PWSdirs.h"
#include "core/PWSAuxParse.h"

#include "os/dir.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Change the master password for the database.
void DboxMain::OnPassphraseChange()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;
  CPasskeyChangeDlg changeDlg(this);

  INT_PTR rc = changeDlg.DoModal();

  if (rc == IDOK) {
    m_core.ChangePasskey(changeDlg.m_newpasskey);
    ChangeOkUpdate();
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

  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  //SaveAs-type dialog box
  while (1) {
    CPWFileDialog fd(FALSE,
                     L"bak",
                     currbackup.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(IDS_FDF_BU)),
                     this);

    fd.m_ofn.lpstrTitle = cs_text;

    if (!dir.empty())
      fd.m_ofn.lpstrInitialDir = dir.c_str();

    rc = fd.DoModal();

    if (m_inExit) {
      // If U3ExitNow called while in CPWFileDialog,
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
    CGeneralMsgBox gmb;
    cs_temp.Format(IDS_CANTOPENWRITING, tempname);
    cs_title.LoadString(IDS_FILEWRITEERROR);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  prefs->SetPref(PWSprefs::CurrentBackup, tempname);
  return PWScore::SUCCESS;
}

void DboxMain::OnRestoreSafe()
{
  if (!m_core.IsReadOnly()) // disable in read-only mode
    RestoreSafe();
}

int DboxMain::RestoreSafe()
{
  int rc;
  StringX backup, passkey, temp;
  StringX currbackup =
    PWSprefs::GetInstance()->GetPref(PWSprefs::CurrentBackup);

  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
    return rc;
   
  // Reset changed flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
  SetChanged(Clear);

  CString cs_text, cs_temp, cs_title;
  cs_text.LoadString(IDS_PICKRESTORE);

  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  //Open-type dialog box
  while (1) {
    CPWFileDialog fd(TRUE,
                     L"bak",
                     currbackup.c_str(),
                     OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                     CString(MAKEINTRESOURCE(IDS_FDF_BUS)),
                     this);

    fd.m_ofn.lpstrTitle = cs_text;

    if (!dir.empty())
      fd.m_ofn.lpstrInitialDir = dir.c_str();

    INT_PTR rc2 = fd.DoModal();

    if (m_inExit) {
      // If U3ExitNow called while in CPWFileDialog,
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
  CGeneralMsgBox gmb;
  switch (rc) {
    case PWScore::SUCCESS:
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPEN, backup);
      cs_title.LoadString(IDS_FILEOPENERROR);
      gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
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
  if (!m_core.GetCurFile().empty()) {
    m_core.UnlockFile(m_core.GetCurFile().c_str());
  }

  // clear the data before restoring
  ClearData();

  rc = m_core.ReadFile(backup, passkey, MAXTEXTCHARS);
  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENREADING, backup);
    cs_title.LoadString(IDS_FILEREADERROR);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  m_core.SetCurFile(L"");    // Force a Save As...
  m_core.SetDBChanged(true); // So that the restored file will be saved
#if !defined(POCKET_PC)
  m_titlebar.LoadString(IDS_UNTITLEDRESTORE);
  app.SetTooltipText(L"PasswordSafe");
#endif
  ChangeOkUpdate();
  RefreshViews();

  return PWScore::SUCCESS;
}

void DboxMain::OnValidate() 
{
  CGeneralMsgBox gmb;
  if (!m_bValidate) {
    // We didn't get here via command line flag - so must be via the menu
    int rc = Open(IDS_CHOOSEDATABASEV);
    if (rc != PWScore::SUCCESS)
      return;
  }

  CReport rpt;
  std::wstring cs_title;
  LoadAString(cs_title, IDS_RPTVALIDATE);
  rpt.StartReport(cs_title.c_str(), m_core.GetCurFile().c_str());

  std::wstring cs_msg;
  bool bchanged = m_core.Validate(cs_msg, rpt, MAXTEXTCHARS);
  if (!bchanged)
    LoadAString(cs_msg, IDS_VALIDATEOK);
  else {
    SetChanged(Data);
    ChangeOkUpdate();
  }

  rpt.EndReport();

  gmb.SetTitle(cs_title.c_str());
  gmb.SetMsg(cs_msg.c_str());
  gmb.SetStandardIcon(bchanged ? MB_ICONEXCLAMATION : MB_ICONINFORMATION);
  gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
  if (bchanged)
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);

  INT_PTR rc = gmb.DoModal();
  if (rc == IDS_VIEWREPORT)
    ViewReport(rpt);

  // Show UUID in Edit Date/Time property sheet stats
  CAddEdit_DateTimes::m_bShowUUID = true;
}

void DboxMain::OnOptions() 
{
  const CString PWSLnkName(L"Password Safe"); // for startup shortcut

  COptions_PropertySheet optionsPS(IDS_OPTIONS, this);
  COptionsBackup backup;
  COptionsDisplay display;
  COptionsMisc misc;
  COptionsPasswordHistory passwordhistory;
  COptionsPasswordPolicy passwordpolicy;
  COptionsSecurity security;
  COptionsShortcuts shortcuts;
  COptionsSystem system;

  PWSprefs *prefs = PWSprefs::GetInstance();
  BOOL prevLockOIT; // lock On Idle Iimeout set?
  BOOL prevLockOWL; // lock On Window Lock set?
  BOOL brc, save_hotkey_enabled;
  BOOL save_preexpirywarn;
  BOOL save_highlightchanges;
  DWORD save_hotkey_value;
  int save_preexpirywarndays;
  UINT prevLockInterval;
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
  ** Initialize the property pages values.
  ** In PropertyPage alphabetic order
  */
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
  display.m_wordwrapnotes = prefs->
    GetPref(PWSprefs::NotesWordWrap) ? TRUE : FALSE;
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
  display.m_treedisplaystatusatopen = prefs->
    GetPref(PWSprefs::TreeDisplayStatusAtOpen);
  display.m_trayiconcolour = prefs->
    GetPref(PWSprefs::ClosedTrayIconColour);
  display.m_highlightchanges = prefs->
    GetPref(PWSprefs::HighlightChanges);
  save_highlightchanges = display.m_highlightchanges;

  misc.m_confirmdelete = prefs->
    GetPref(PWSprefs::DeleteQuestion) ? FALSE : TRUE;
  misc.m_maintaindatetimestamps = prefs->
    GetPref(PWSprefs::MaintainDateTimeStamps) ? TRUE : FALSE;
  misc.m_escexits = prefs->
    GetPref(PWSprefs::EscExits) ? TRUE : FALSE;
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
  misc.m_otherbrowserlocation = prefs->
    GetPref(PWSprefs::AltBrowser).c_str();
  misc.m_csBrowserCmdLineParms = prefs->
    GetPref(PWSprefs::AltBrowserCmdLineParms).c_str();
  misc.m_othereditorlocation = prefs->
    GetPref(PWSprefs::AltNotesEditor).c_str();
  CString dats = prefs->
    GetPref(PWSprefs::DefaultAutotypeString).c_str();
  if (dats.IsEmpty())
    dats = DEFAULT_AUTOTYPE;
  misc.m_csAutotype = CString(dats);
  misc.m_minauto = prefs->
    GetPref(PWSprefs::MinimizeOnAutotype) ? TRUE : FALSE;  

  passwordhistory.m_savepwhistory = prefs->
    GetPref(PWSprefs::SavePasswordHistory) ? TRUE : FALSE;
  passwordhistory.m_pwhistorynumdefault = prefs->
    GetPref(PWSprefs::NumPWHistoryDefault);

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

  CString cs_symbols(prefs->GetPref(PWSprefs::DefaultSymbols).c_str());
  passwordpolicy.m_cs_symbols = cs_symbols;
  passwordpolicy.m_useownsymbols = 
            (cs_symbols.GetLength() == 0) ? DEFAULT_SYMBOLS : OWN_SYMBOLS;

  security.m_clearclipboardonminimize = prefs->
    GetPref(PWSprefs::ClearClipboardOnMinimize) ? TRUE : FALSE;
  security.m_clearclipboardonexit = prefs->
    GetPref(PWSprefs::ClearClipboardOnExit) ? TRUE : FALSE;
  security.m_LockOnMinimize = prefs->
    GetPref(PWSprefs::DatabaseClear) ? TRUE : FALSE;
  security.m_confirmcopy = prefs->
    GetPref(PWSprefs::DontAskQuestion) ? FALSE : TRUE;
  security.m_LockOnWindowLock = prevLockOWL = prefs->
    GetPref(PWSprefs::LockOnWindowLock) ? TRUE : FALSE;
  security.m_LockOnIdleTimeout = prevLockOIT = prefs->
    GetPref(PWSprefs::LockDBOnIdleTimeout) ? TRUE : FALSE;
  security.m_IdleTimeOut = prevLockInterval = prefs->
    GetPref(PWSprefs::IdleTimeout);

  shortcuts.m_iColWidth = prefs->
    GetPref(PWSprefs::OptShortcutColumnWidth);
  shortcuts.m_iDefColWidth = prefs->
    GetPrefDefVal(PWSprefs::OptShortcutColumnWidth);
  shortcuts.InitialSetup(m_MapMenuShortcuts, m_MapKeyNameID,
                         m_ExcludedMenuItems,
                         m_ReservedShortcuts);

  system.m_maxreitems = prefs->
    GetPref(PWSprefs::MaxREItems);
  system.m_usesystemtray = prefs->
    GetPref(PWSprefs::UseSystemTray) ? TRUE : FALSE;
  system.m_hidesystemtray = prefs->
    GetPref(PWSprefs::HideSystemTray) ? TRUE : FALSE;
  system.m_maxmruitems = prefs->
    GetPref(PWSprefs::MaxMRUItems);
  system.m_mruonfilemenu = prefs->
    GetPref(PWSprefs::MRUOnFileMenu);
  system.m_startup = StartupShortcutExists;
  system.m_defaultopenro = prefs->
    GetPref(PWSprefs::DefaultOpenRO) ? TRUE : FALSE;
  system.m_multipleinstances = prefs->
    GetPref(PWSprefs::MultipleInstances) ? TRUE : FALSE;
  system.m_initialhotkeystate = save_hotkey_enabled;

  optionsPS.AddPage(&backup);
  optionsPS.AddPage(&display);
  optionsPS.AddPage(&misc);
  optionsPS.AddPage(&passwordpolicy);
  optionsPS.AddPage(&passwordhistory);
  optionsPS.AddPage(&security);
  optionsPS.AddPage(&system);
  optionsPS.AddPage(&shortcuts);

  // Remove the "Apply Now" button.
  optionsPS.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  // Disable Hotkey around this as the user may press the current key when 
  // selecting the new key!

#if !defined(POCKET_PC)
  brc = UnregisterHotKey(m_hWnd, PWS_HOTKEY_ID); // clear last - never hurts
#endif

  passwordhistory.m_pDboxMain = this;

  INT_PTR rc = optionsPS.DoModal();

  prefs->SetPref(PWSprefs::OptShortcutColumnWidth,
                 shortcuts.m_iColWidth);

  if (rc == IDOK) {
    /*
    **  Now save all the  Application preferences.
    ** Do not update Database preferences - Command will do that later
    */

    // Now update Application preferences
    // In PropertyPage alphabetic order
    prefs->SetPref(PWSprefs::BackupBeforeEverySave,
                   backup.m_backupbeforesave == TRUE);
    prefs->SetPref(PWSprefs::BackupPrefixValue,
                   LPCWSTR(backup.m_userbackupprefix));
    prefs->SetPref(PWSprefs::BackupSuffix,
                   (unsigned int)backup.m_backupsuffix);
    prefs->SetPref(PWSprefs::BackupMaxIncremented,
                   backup.m_maxnumincbackups);
    if (!backup.m_userbackupotherlocation.IsEmpty()) {
      // Make sure it ends in a slash!
      if (backup.m_userbackupotherlocation.Right(1) != L'\\')
        backup.m_userbackupotherlocation += L'\\';
    }
    prefs->SetPref(PWSprefs::BackupDir,
                   LPCWSTR(backup.m_userbackupotherlocation));

    prefs->SetPref(PWSprefs::AlwaysOnTop,
                   display.m_alwaysontop == TRUE);
    prefs->SetPref(PWSprefs::ShowNotesAsTooltipsInViews,
                   display.m_shownotesastipsinviews == TRUE);
    prefs->SetPref(PWSprefs::ExplorerTypeTree,
                   display.m_explorertree == TRUE);
    prefs->SetPref(PWSprefs::ListViewGridLines,
                   display.m_enablegrid == TRUE);
    prefs->SetPref(PWSprefs::NotesWordWrap,
                   display.m_wordwrapnotes == TRUE);
    prefs->SetPref(PWSprefs::PreExpiryWarn,
                   display.m_preexpirywarn == TRUE);
    prefs->SetPref(PWSprefs::PreExpiryWarnDays,
                   display.m_preexpirywarndays);
#if defined(POCKET_PC)
    prefs->SetPref(PWSprefs::DCShowsPassword,
                   display.m_dcshowspassword == TRUE);
#endif
    prefs->SetPref(PWSprefs::ClosedTrayIconColour,
                   display.m_trayiconcolour);
    app.SetClosedTrayIcon(display.m_trayiconcolour);
    if (save_highlightchanges != display.m_highlightchanges) {
      prefs->SetPref(PWSprefs::HighlightChanges,
                     display.m_highlightchanges == TRUE);
      m_ctlItemList.SetHighlightChanges(display.m_highlightchanges == TRUE);
      m_ctlItemTree.SetHighlightChanges(display.m_highlightchanges == TRUE);
      RefreshViews();
    }

    prefs->SetPref(PWSprefs::DeleteQuestion,
                   misc.m_confirmdelete == FALSE);
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
    prefs->SetPref(PWSprefs::QuerySetDef,
                   misc.m_querysetdef == TRUE);
    prefs->SetPref(PWSprefs::AltBrowser,
                   LPCWSTR(misc.m_otherbrowserlocation));
    prefs->SetPref(PWSprefs::AltBrowserCmdLineParms,
                   LPCWSTR(misc.m_csBrowserCmdLineParms));
    prefs->SetPref(PWSprefs::AltNotesEditor,
                   LPCWSTR(misc.m_othereditorlocation));
    prefs->SetPref(PWSprefs::MinimizeOnAutotype,
                   misc.m_minauto == TRUE);

    // JHF : no status bar under WinCE (was already so in the .h file !?!)
#if !defined(POCKET_PC)
    /* Update status bar */
    UINT uiMessage(IDS_STATCOMPANY);
    switch (misc.m_doubleclickaction) {
      case PWSprefs::DoubleClickAutoType:
        uiMessage = IDS_STATAUTOTYPE; break;
      case PWSprefs::DoubleClickBrowse:
        uiMessage = IDS_STATBROWSE; break;
      case PWSprefs::DoubleClickCopyNotes:
        uiMessage = IDS_STATCOPYNOTES; break;
      case PWSprefs::DoubleClickCopyPassword:
        uiMessage = IDS_STATCOPYPASSWORD; break;
      case PWSprefs::DoubleClickCopyUsername:
        uiMessage = IDS_STATCOPYUSERNAME; break;
      case PWSprefs::DoubleClickViewEdit:
        uiMessage = IDS_STATVIEWEDIT; break;
      case PWSprefs::DoubleClickCopyPasswordMinimize:
        uiMessage = IDS_STATCOPYPASSWORDMIN; break;
      case PWSprefs::DoubleClickBrowsePlus:
        uiMessage = IDS_STATBROWSEPLUS; break;
      case PWSprefs::DoubleClickRun:
        uiMessage = IDS_STATRUN; break;
      case PWSprefs::DoubleClickSendEmail:
        uiMessage = IDS_STATSENDEMAIL; break;
      default:
        uiMessage = IDS_STATCOMPANY;
    }
    statustext[CPWStatusBar::SB_DBLCLICK] = uiMessage;
    m_statusBar.SetIndicators(statustext, CPWStatusBar::SB_TOTAL);
    UpdateStatusBar();
    // Make a sunken or recessed border around the first pane
    m_statusBar.SetPaneInfo(CPWStatusBar::SB_DBLCLICK,
                            m_statusBar.GetItemID(CPWStatusBar::SB_DBLCLICK),
                            SBPS_STRETCH, NULL);
#endif

    prefs->SetPref(PWSprefs::ClearClipboardOnMinimize,
                   security.m_clearclipboardonminimize == TRUE);
    prefs->SetPref(PWSprefs::ClearClipboardOnExit,
                   security.m_clearclipboardonexit == TRUE);
    prefs->SetPref(PWSprefs::DatabaseClear,
                   security.m_LockOnMinimize == TRUE);
    prefs->SetPref(PWSprefs::DontAskQuestion,
                   security.m_confirmcopy == FALSE);
    prefs->SetPref(PWSprefs::LockOnWindowLock,
                   security.m_LockOnWindowLock == TRUE);

    prefs->SetPref(PWSprefs::UseSystemTray,
                   system.m_usesystemtray == TRUE);
    prefs->SetPref(PWSprefs::HideSystemTray,
                   system.m_hidesystemtray == TRUE);

    if (system.m_usesystemtray == FALSE)
      app.HideIcon();
    else
      app.ShowIcon();

    UpdateSystemMenu();
    prefs->SetPref(PWSprefs::MaxREItems,
                   system.m_maxreitems);
    prefs->SetPref(PWSprefs::MaxMRUItems,
                   system.m_maxmruitems);
    if (system.m_maxmruitems == 0) {
      // Put them on File menu where they don't take up any room
      prefs->SetPref(PWSprefs::MRUOnFileMenu, true);
      // Clear any currently saved
      app.ClearMRU();
    } else {
      prefs->SetPref(PWSprefs::MRUOnFileMenu,
                     system.m_mruonfilemenu == TRUE);
    }
    prefs->SetPref(PWSprefs::DefaultOpenRO,
                   system.m_defaultopenro == TRUE);
    prefs->SetPref(PWSprefs::MultipleInstances,
                   system.m_multipleinstances == TRUE);

    // Now do a bit of trickery to find the new preferences to be stored in
    // the database as a string but without updating the actual preferences
    // which needs to be done via a Command so that it can be Undone & Redone

    // Initialise a copy of the DB preferences
    prefs->SetUpCopyDBprefs();

    // Update them - last parameter of SetPref and Store is: "bUseCopy = true"
    // In PropertyPage alphabetic order
    prefs->SetPref(PWSprefs::SaveImmediately,
                   backup.m_saveimmediately == TRUE, true);

    prefs->SetPref(PWSprefs::ShowPWDefault,
                   display.m_pwshowinedit == TRUE, true);
    prefs->SetPref(PWSprefs::ShowUsernameInTree,
                   display.m_showusernameintree == TRUE, true);
    prefs->SetPref(PWSprefs::ShowPasswordInTree,
                   display.m_showpasswordintree == TRUE, true);
    prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen,
                   display.m_treedisplaystatusatopen, true);
    prefs->SetPref(PWSprefs::ShowNotesDefault,
                   display.m_notesshowinedit == TRUE, true);

    prefs->SetPref(PWSprefs::MaintainDateTimeStamps,
                   misc.m_maintaindatetimestamps == TRUE, true);

    prefs->SetPref(PWSprefs::UseDefaultUser,
                   misc.m_usedefuser == TRUE, true);
    prefs->SetPref(PWSprefs::DefaultUsername,
                   LPCWSTR(misc.m_defusername), true);

    if (misc.m_csAutotype.IsEmpty() || misc.m_csAutotype == DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString, L"", true);
    else
    if (misc.m_csAutotype != DEFAULT_AUTOTYPE)
      prefs->SetPref(PWSprefs::DefaultAutotypeString,
                     LPCWSTR(misc.m_csAutotype), true);

    prefs->SetPref(PWSprefs::SavePasswordHistory,
                   passwordhistory.m_savepwhistory == TRUE, true);
    if (passwordhistory.m_savepwhistory == TRUE)
      prefs->SetPref(PWSprefs::NumPWHistoryDefault,
                     passwordhistory.m_pwhistorynumdefault, true);

    prefs->SetPref(PWSprefs::PWUseLowercase,
                   passwordpolicy.m_pwuselowercase == TRUE, true);
    prefs->SetPref(PWSprefs::PWUseUppercase,
                   passwordpolicy.m_pwuseuppercase == TRUE, true);
    prefs->SetPref(PWSprefs::PWUseDigits,
                   passwordpolicy.m_pwusedigits == TRUE, true);
    prefs->SetPref(PWSprefs::PWUseSymbols,
                   passwordpolicy.m_pwusesymbols == TRUE, true);
    prefs->SetPref(PWSprefs::PWUseHexDigits,
                   passwordpolicy.m_pwusehexdigits == TRUE, true);
    prefs->SetPref(PWSprefs::PWUseEasyVision,
                   passwordpolicy.m_pweasyvision == TRUE, true);
    prefs->SetPref(PWSprefs::PWMakePronounceable,
                   passwordpolicy.m_pwmakepronounceable == TRUE, true);

    prefs->SetPref(PWSprefs::PWDefaultLength,
                   passwordpolicy.m_pwdefaultlength, true);
    prefs->SetPref(PWSprefs::PWDigitMinLength,
                   passwordpolicy.m_pwdigitminlength, true);
    prefs->SetPref(PWSprefs::PWLowercaseMinLength,
                   passwordpolicy.m_pwlowerminlength, true);
    prefs->SetPref(PWSprefs::PWSymbolMinLength,
                   passwordpolicy.m_pwsymbolminlength, true);
    prefs->SetPref(PWSprefs::PWUppercaseMinLength,
                   passwordpolicy.m_pwupperminlength, true);

    if (passwordpolicy.m_useownsymbols != passwordpolicy.m_saveuseownsymbols ||
        (passwordpolicy.m_useownsymbols == OWN_SYMBOLS &&
         passwordpolicy.m_cs_symbols != passwordpolicy.m_cs_savesymbols))
      prefs->SetPref(PWSprefs::DefaultSymbols,
                     LPCWSTR(passwordpolicy.m_cs_symbols), true);

    prefs->SetPref(PWSprefs::LockDBOnIdleTimeout,
                   security.m_LockOnIdleTimeout == TRUE, true);
    prefs->SetPref(PWSprefs::IdleTimeout,
                   security.m_IdleTimeOut, true);

    // Get new and old DB preferences String value
    const StringX sxOldDBPrefsString(prefs->Store());
    StringX sxNewDBPrefsString(prefs->Store(true));

    // Maybe needed if this causes changes to database
    // (currently only DB preferences + updating PWHistory in exisiting entries)
    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

    /*
    ** Set up Command to update string in database, if necessary & possible
    ** (i.e. ignore if R-O)
    */
    if (!m_core.GetCurFile().empty() && !m_core.IsReadOnly() &&
        m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
      if (sxOldDBPrefsString != sxNewDBPrefsString) {
        // Determine whether Tree needs redisplayng due to change
        // in what is shown (e.g. usernames/passwords)
        bool bUserDisplayChanged = (bOldShowUsernameInTree != 
                                    (display.m_showusernameintree == TRUE));
        // Note: passwords are only shown in the Tree is usernames are also shown
        bool bPswdDisplayChanged = (bOldShowPasswordInTree != 
                                    (display.m_showpasswordintree == TRUE));
        bool bNeedGUITreeUpdate = bUserDisplayChanged || 
            ((display.m_showusernameintree == TRUE) && bPswdDisplayChanged);
        if (bNeedGUITreeUpdate) {
          Command *pcmd = UpdateGUICommand::Create(&m_core,
                                                   UpdateGUICommand::WN_UNDO,
                                                   UpdateGUICommand::GUI_REFRESH_TREE);
          pmulticmds->Add(pcmd);
        }
        Command *pcmd = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
        pmulticmds->Add(pcmd);
        if (bNeedGUITreeUpdate) {
          Command *pcmd = UpdateGUICommand::Create(&m_core,
                                                   UpdateGUICommand::WN_EXECUTE_REDO,
                                                   UpdateGUICommand::GUI_REFRESH_TREE);
          pmulticmds->Add(pcmd);
        }
      }
    }

    const int iAction = passwordhistory.m_pwhaction;
    const int new_default_max = passwordhistory.m_pwhistorynumdefault;
    size_t ipwh_exec(0);

    if (iAction != 0) {
      Command *pcmd = UpdatePasswordHistoryCommand::Create(&m_core,
                                                           iAction,
                                                           new_default_max);
      pmulticmds->Add(pcmd);
      ipwh_exec = pmulticmds->GetSize();
    }

    /*
    ** Now update the application according to the options.
    ** Any changes via Database preferences done via call to UpdateGUI from Command
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
      SaveGroupDisplayState();

    m_RUEList.SetMax(system.m_maxreitems);

    if (system.m_startup != StartupShortcutExists) {
      if (system.m_startup == TRUE) {
        wchar_t exeName[MAX_PATH];
        GetModuleFileName(NULL, exeName, MAX_PATH);
        shortcut.SetCmdArguments(CString(L"-s"));
        shortcut.CreateShortCut(exeName, PWSLnkName, CSIDL_STARTUP);
      } else { // remove existing startup shortcut
        shortcut.DeleteShortCut(PWSLnkName, CSIDL_STARTUP);
      }
    }

    // Update Lock on Window Lock
    if (security.m_LockOnWindowLock != prevLockOWL) {
      if (security.m_LockOnWindowLock == TRUE) {
        startLockCheckTimer();
      } else {
        KillTimer(TIMER_LOCKONWTSLOCK);
      }
    }

    // Keep prefs file updated:
    prefs->SaveApplicationPreferences();

    if (shortcuts.HaveShortcutsChanged()) {
      // Create vector of shortcuts for user's config file
      std::vector<st_prefShortcut> vShortcuts;
      MapMenuShortcutsIter iter, iter_entry, iter_group;
      m_MapMenuShortcuts = shortcuts.GetMaps();

      for (iter = m_MapMenuShortcuts.begin(); iter != m_MapMenuShortcuts.end();
        iter++) {
        // User should not have these sub-entries in their config file
        if (iter->first == ID_MENUITEM_GROUPENTER ||
            iter->first == ID_MENUITEM_VIEW || 
            iter->first == ID_MENUITEM_DELETEENTRY ||
            iter->first == ID_MENUITEM_DELETEGROUP ||
            iter->first == ID_MENUITEM_RENAMEENTRY ||
            iter->first == ID_MENUITEM_RENAMEGROUP) {
          continue;
        }
        // Now only those different from default
        if (iter->second.cVirtKey  != iter->second.cdefVirtKey  ||
            iter->second.cModifier != iter->second.cdefModifier) {
          st_prefShortcut stxst;
          stxst.id = iter->first;
          stxst.cVirtKey = iter->second.cVirtKey;
          stxst.cModifier = iter->second.cModifier;
          vShortcuts.push_back(stxst);
        }
      }
      prefs->SetPrefShortcuts(vShortcuts);
      prefs->SaveShortcuts();

      // Set up the shortcuts based on the main entry
      // for View, Delete and Rename
      iter = m_MapMenuShortcuts.find(ID_MENUITEM_EDIT);
      iter_entry = m_MapMenuShortcuts.find(ID_MENUITEM_VIEW);
      iter_entry->second.SetKeyFlags(iter->second);

      SetupSpecialShortcuts();

      UpdateAccelTable();

      // Set menus to be rebuilt with user's changed shortcuts
      for (int i = 0; i < NUMPOPUPMENUS; i++) {
        m_bDoShortcuts[i] = true;
      }
    }

    // If DB preferences changed and/or password history options
    if (pmulticmds != NULL) {
      if (pmulticmds->GetSize() > 0) {
        Execute(pmulticmds);
        if (ipwh_exec > 0) {
          // We did do PWHistory update
          int num_altered(0);
          if (pmulticmds->GetRC(ipwh_exec, num_altered)) {
            UINT uimsg_id(0);
            switch (iAction) {
              case 1:   // reset off
                uimsg_id = IDS_ENTRIESCHANGEDSTOP;
                break;
              case 2:   // reset on
                uimsg_id = IDS_ENTRIESCHANGEDSAVE;
                break;
              case 3:   // setmax
                uimsg_id = IDS_ENTRIESRESETMAX;
                break;
              default:
                ASSERT(0);
                break;
            } // switch (iAction)

            if (uimsg_id > 0) {
              CGeneralMsgBox gmb;
              CString cs_Msg;
              cs_Msg.Format(uimsg_id, num_altered);
              gmb.AfxMessageBox(cs_Msg);
            }
          }
        } 
      } else {
        // Was created but no commands added in the end.
        delete pmulticmds;
      }
    }

    if ((bOldExplorerTypeTree !=
           prefs->GetPref(PWSprefs::ExplorerTypeTree)) ||
        (save_preexpirywarn != display.m_preexpirywarn) ||
        (save_preexpirywarndays != display.m_preexpirywarndays)) {
      RefreshViews();
    }

    if (m_core.HaveDBPrefsChanged()) {
      ChangeOkUpdate();
    }

    brc = FALSE;
    // JHF no hotkeys under WinCE
#if !defined(POCKET_PC)
    // Restore hotkey as it was or as user changed it - if user pressed OK
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
      if (brc == FALSE) {
        CGeneralMsgBox gmb;
        gmb.AfxMessageBox(IDS_NOHOTKEY, MB_OK);
      }
    }
#endif
    if (prefs->GetPref(PWSprefs::UseSystemTray)) { 
      if (prefs->GetPref(PWSprefs::HideSystemTray) && 
          prefs->GetPref(PWSprefs::HotKeyEnabled) &&
          prefs->GetPref(PWSprefs::HotKey) > 0)
        app.HideIcon();
      else if (app.IsIconVisible() == FALSE)
        app.ShowIcon();
    } else {
      app.HideIcon();
    }

    // If user has turned on/changed warnings of expired passwords - check now
    if (display.m_preexpirywarn == TRUE &&
        (save_preexpirywarn == FALSE ||
         save_preexpirywarndays != display.m_preexpirywarndays)) {
      CheckExpireList();
      TellUserAboutExpiredPasswords();
    }
    
  }  // rc == IDOK
}

void DboxMain::OnGeneratePassword()
{
  COptions_PropertySheet GenPswdPS(IDS_OPTIONS, this);
  COptionsPasswordPolicy pp(false);
  pp.m_pDbx = this;

  PWSprefs *prefs = PWSprefs::GetInstance();

  pp.m_pwuselowercase = prefs->GetPref(PWSprefs::PWUseLowercase);
  pp.m_pwuseuppercase = prefs->GetPref(PWSprefs::PWUseUppercase);
  pp.m_pwusedigits = prefs->GetPref(PWSprefs::PWUseDigits);
  pp.m_pwusesymbols = prefs->GetPref(PWSprefs::PWUseSymbols);
  pp.m_pwusehexdigits = prefs->GetPref(PWSprefs::PWUseHexDigits);
  pp.m_pweasyvision = prefs->GetPref(PWSprefs::PWUseEasyVision);
  pp.m_pwmakepronounceable = prefs->GetPref(PWSprefs::PWMakePronounceable);
  pp.m_pwdefaultlength = prefs->GetPref(PWSprefs::PWDefaultLength);
  pp.m_pwdigitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
  pp.m_pwlowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
  pp.m_pwsymbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
  pp.m_pwupperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);

  CString cs_symbols(prefs->GetPref(PWSprefs::DefaultSymbols).c_str());
  pp.m_cs_symbols = pp.m_cs_savesymbols = cs_symbols;
  pp.m_useownsymbols = pp.m_saveuseownsymbols =
                (cs_symbols.GetLength() == 0) ? DEFAULT_SYMBOLS : OWN_SYMBOLS;

  CString cs_caption(MAKEINTRESOURCE(IDS_GENERATEPASSWORD));
  GenPswdPS.AddPage(&pp);
  GenPswdPS.m_psh.dwFlags |= PSH_NOAPPLYNOW;
  GenPswdPS.m_psh.pszCaption = cs_caption;

  GenPswdPS.DoModal();
}
