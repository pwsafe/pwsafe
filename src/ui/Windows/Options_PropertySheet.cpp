/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "Options_PropertySheet.h"
#include "Options_PropertyPage.h"
#include "Shortcut.h"

#include "core/PWSAuxParse.h"

IMPLEMENT_DYNAMIC(COptions_PropertySheet, CPWPropertySheet)

COptions_PropertySheet::COptions_PropertySheet(UINT nID, CWnd* pParent,
  const bool bLongPPs)
  : CPWPropertySheet(nID, pParent, bLongPPs),
  m_save_bSymbols(L""), m_save_iUseOwnSymbols(DEFAULT_SYMBOLS),
  m_save_iPreExpiryWarnDays(0),
  m_bIsModified(false), m_bChanged(false),
  m_bRefreshViews(false), m_bSaveGroupDisplayState(false), m_bUpdateShortcuts(false),
  m_bCheckExpired(false),
  m_save_bShowUsernameInTree(FALSE), m_save_bShowPasswordInTree(FALSE), 
  m_save_bExplorerTypeTree(FALSE), m_save_bPreExpiryWarn(FALSE),
  m_save_bLockOnWindowLock(FALSE), m_bStartupShortcutExists(FALSE),
  m_save_bSaveImmediately(TRUE), m_save_bHighlightChanges(FALSE),
  m_pp_backup(NULL), m_pp_display(NULL), m_pp_misc(NULL),
  m_pp_passwordhistory(NULL), m_pp_security(NULL),
  m_pp_shortcuts(NULL), m_pp_system(NULL)
{
  ASSERT(pParent != NULL);

  // Set up initial values
  SetupInitialValues();

  // Only now allocate the PropertyPages - after all data there
  // to be used by their c'tors
  m_OPTMD.bLongPPs = bLongPPs; // chooseResource();

  m_pp_backup          = new COptionsBackup(this, &m_OPTMD);
  m_pp_display         = new COptionsDisplay(this, &m_OPTMD);
  m_pp_misc            = new COptionsMisc(this, &m_OPTMD);
  m_pp_passwordhistory = new COptionsPasswordHistory(this, &m_OPTMD);
  m_pp_security        = new COptionsSecurity(this, &m_OPTMD);
  m_pp_shortcuts       = new COptionsShortcuts(this, &m_OPTMD);
  m_pp_system          = new COptionsSystem(this, &m_OPTMD);

  m_pp_shortcuts->InitialSetup(GetMainDlg()->GetMapMenuShortcuts(),
                               GetMainDlg()->GetExcludedMenuItems(),
                               GetMainDlg()->GetReservedShortcuts());

  AddPage(m_pp_backup);
  AddPage(m_pp_display);
  AddPage(m_pp_misc);
  AddPage(m_pp_passwordhistory);
  AddPage(m_pp_security);
  AddPage(m_pp_shortcuts);
  AddPage(m_pp_system);
 
  CString cs_caption(MAKEINTRESOURCE(nID));
  m_psh.pszCaption = _wcsdup(cs_caption);
}

COptions_PropertySheet::~COptions_PropertySheet()
{
  // Note: 'delete' handles NULL pointers
  delete m_pp_backup;
  delete m_pp_display;
  delete m_pp_misc;
  delete m_pp_passwordhistory;
  delete m_pp_security;
  delete m_pp_shortcuts;
  delete m_pp_system;

  free((void *)m_psh.pszCaption);
  m_psh.pszCaption = NULL;
}

BOOL COptions_PropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // There is no OnOK for classes derived from CPropertySheet,
  // so we make our own!
  if (LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED) {
    // First send a message to all loaded pages using base class function.
    // We want them all to update their variables in the Master Data area.
    // And call OnApply() rather than the default OnOK processing
    // Note: This message is only sent to PropertyPages that have been
    // loaded - i.e. the user has selected to view them, since obviously
    // the user would not have changed their values if not displayed. Duh!
    if (SendMessage(PSM_QUERYSIBLINGS,
                (WPARAM)CPWPropertyPage::PP_UPDATE_VARIABLES, 0L) != 0)
      return TRUE;

    // Now update a copy of the preferences as per user's wishes
    UpdateCopyPreferences();

    // Now end it all so that OnApply isn't called again
    CPWPropertySheet::EndDialog(IDOK);
    return TRUE;
  }
  return CPWPropertySheet::OnCommand(wParam, lParam);
}

BOOL COptions_PropertySheet::PreTranslateMessage(MSG *pMsg) 
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    COptions_PropertyPage *pp = (COptions_PropertyPage *)GetActivePage();
    pp->PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CPWPropertySheet::PreTranslateMessage(pMsg);
}

void COptions_PropertySheet::SetupInitialValues()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  
  // Set up a copy of the preferences
  prefs->SetupCopyPrefs();

  // Backup Data
  CString cs_backupPrefix, cs_backupDir;
  m_OPTMD.CurrentFile = GetMainDlg()->GetCurFile().c_str();
  m_OPTMD.SaveImmediately = m_save_bSaveImmediately = 
      prefs->GetPref(PWSprefs::SaveImmediately) ? TRUE : FALSE;
  m_OPTMD.BackupBeforeSave =
      prefs->GetPref(PWSprefs::BackupBeforeEverySave) ? TRUE : FALSE;
  cs_backupPrefix =
      prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
  m_OPTMD.BackupPrefix = cs_backupPrefix.IsEmpty() ? 0 : 1;
  m_OPTMD.UserBackupPrefix = (LPCWSTR)cs_backupPrefix;
  m_OPTMD.BackupSuffix =
      prefs->GetPref(PWSprefs::BackupSuffix);
  m_OPTMD.MaxNumIncBackups =
      prefs->GetPref(PWSprefs::BackupMaxIncremented);
  cs_backupDir =
      prefs->GetPref(PWSprefs::BackupDir).c_str();
  m_OPTMD.BackupLocation = cs_backupDir.IsEmpty() ? 0 : 1;
  m_OPTMD.UserBackupOtherLocation = (LPCWSTR)cs_backupDir;
  // Preferences min/max
  m_OPTMD.prefminBackupIncrement = (short)prefs->GetPrefMinVal(PWSprefs::BackupMaxIncremented);
  m_OPTMD.prefmaxBackupIncrement = (short)prefs->GetPrefMaxVal(PWSprefs::BackupMaxIncremented);

  // Display Data
  m_OPTMD.AlwaysOnTop =
      prefs->GetPref(PWSprefs::AlwaysOnTop) ? TRUE : FALSE;
  m_OPTMD.ShowPasswordInEdit =
      prefs->GetPref(PWSprefs::ShowPWDefault) ? TRUE : FALSE;
  m_OPTMD.ShowUsernameInTree = m_save_bShowUsernameInTree =
      prefs->GetPref(PWSprefs::ShowUsernameInTree) ? TRUE : FALSE;
  m_OPTMD.ShowPasswordInTree = m_save_bShowPasswordInTree =
      prefs->GetPref(PWSprefs::ShowPasswordInTree) ? TRUE : FALSE;
  m_OPTMD.ShowNotesAsTipsInViews =
      prefs->GetPref(PWSprefs::ShowNotesAsTooltipsInViews) ? TRUE : FALSE;
  m_OPTMD.ExplorerTypeTree = m_save_bExplorerTypeTree =
      prefs->GetPref(PWSprefs::ExplorerTypeTree) ? TRUE : FALSE;
  m_OPTMD.EnableGrid =
      prefs->GetPref(PWSprefs::ListViewGridLines) ? TRUE : FALSE;
  m_OPTMD.NotesShowInEdit =
      prefs->GetPref(PWSprefs::ShowNotesDefault) ? TRUE : FALSE;
  m_OPTMD.WordWrapNotes =
      prefs->GetPref(PWSprefs::NotesWordWrap) ? TRUE : FALSE;
  m_OPTMD.PreExpiryWarn = m_save_bPreExpiryWarn =
      prefs->GetPref(PWSprefs::PreExpiryWarn) ? TRUE : FALSE;
  m_OPTMD.PreExpiryWarnDays = m_save_iPreExpiryWarnDays =
      prefs->GetPref(PWSprefs::PreExpiryWarnDays);
  m_OPTMD.TreeDisplayStatusAtOpen =
      prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen);
  m_OPTMD.HighlightChanges = m_save_bHighlightChanges =
      prefs->GetPref(PWSprefs::HighlightChanges);
  m_OPTMD.EnableTransparency =
    prefs->GetPref(PWSprefs::EnableWindowTransparency) ? TRUE : FALSE;
  m_OPTMD.PercentTransparency =
      prefs->GetPref(PWSprefs::WindowTransparency);
  // Preferences min/max
  m_OPTMD.prefminExpiryDays = (short)prefs->GetPrefMinVal(PWSprefs::PreExpiryWarnDays);
  m_OPTMD.prefmaxExpiryDays = (short)prefs->GetPrefMaxVal(PWSprefs::PreExpiryWarnDays);
  m_OPTMD.prefminPercentTransparency = (short)prefs->GetPrefMinVal(PWSprefs::WindowTransparency);
  m_OPTMD.prefmaxPercentTransparency = (short)prefs->GetPrefMaxVal(PWSprefs::WindowTransparency);
  
  // Misc Data
  m_OPTMD.ConfirmDelete =
      prefs->GetPref(PWSprefs::DeleteQuestion) ? FALSE : TRUE;
  m_OPTMD.MaintainDatetimeStamps =
      prefs->GetPref(PWSprefs::MaintainDateTimeStamps) ? TRUE : FALSE;
  m_OPTMD.EscExits =
      prefs->GetPref(PWSprefs::EscExits) ? TRUE : FALSE;
  m_OPTMD.DoubleClickAction =
      prefs->GetPref(PWSprefs::DoubleClickAction);
  m_OPTMD.ShiftDoubleClickAction =
      prefs->GetPref(PWSprefs::ShiftDoubleClickAction);
  m_OPTMD.prefminAutotypeDelay = prefs->GetPrefMinVal(PWSprefs::DefaultAutotypeDelay);
  m_OPTMD.prefmaxAutotypeDelay = prefs->GetPrefMaxVal(PWSprefs::DefaultAutotypeDelay);

  m_OPTMD.UseDefuser =
      prefs->GetPref(PWSprefs::UseDefaultUser) ? TRUE : FALSE;
  m_OPTMD.DefUsername =
      prefs->GetPref(PWSprefs::DefaultUsername).c_str();
  m_OPTMD.QuerySetDef =
      prefs->GetPref(PWSprefs::QuerySetDef) ? TRUE : FALSE;
  m_OPTMD.OtherBrowserLocation =
      prefs->GetPref(PWSprefs::AltBrowser).c_str();
  m_OPTMD.OtherBrowserCmdLineParms =
      prefs->GetPref(PWSprefs::AltBrowserCmdLineParms).c_str();
  m_OPTMD.OtherEditorLocation =
      prefs->GetPref(PWSprefs::AltNotesEditor).c_str(); // 
  m_OPTMD.OtherEditorCmdLineParms =
      prefs->GetPref(PWSprefs::AltNotesEditorCmdLineParms).c_str();
  CString cs_dats =
      prefs->GetPref(PWSprefs::DefaultAutotypeString).c_str();
  if (cs_dats.IsEmpty())
    cs_dats = DEFAULT_AUTOTYPE;
  m_OPTMD.AutotypeText = (LPCWSTR)cs_dats;
  m_OPTMD.AutotypeDelay =
    prefs->GetPref(PWSprefs::DefaultAutotypeDelay);
  m_OPTMD.MinAuto =
      prefs->GetPref(PWSprefs::MinimizeOnAutotype) ? TRUE : FALSE;  
  
  // Password History Data
  m_OPTMD.SavePWHistory =
      prefs->GetPref(PWSprefs::SavePasswordHistory) ? TRUE : FALSE;
  m_OPTMD.PWHistoryNumDefault =
      prefs->GetPref(PWSprefs::NumPWHistoryDefault);
  m_OPTMD.PWHAction = 0;
  m_OPTMD.PWHDefExpDays = prefs->GetPref(PWSprefs::DefaultExpiryDays);
  // Preferences min/max values
  m_OPTMD.prefminPWHNumber = (short)prefs->GetPrefMinVal(PWSprefs::NumPWHistoryDefault);
  m_OPTMD.prefmaxPWHNumber = (short)prefs->GetPrefMaxVal(PWSprefs::NumPWHistoryDefault);

  // Security Data
  m_OPTMD.ClearClipboardOnMinimize =
      prefs->GetPref(PWSprefs::ClearClipboardOnMinimize) ? TRUE : FALSE;
  m_OPTMD.ClearClipboardOnExit =
      prefs->GetPref(PWSprefs::ClearClipboardOnExit) ? TRUE : FALSE;
  m_OPTMD.LockOnMinimize =
      prefs->GetPref(PWSprefs::DatabaseClear) ? TRUE : FALSE;
  m_OPTMD.ConfirmCopy =
      prefs->GetPref(PWSprefs::DontAskQuestion) ? FALSE : TRUE;
  m_OPTMD.LockOnWindowLock = m_save_bLockOnWindowLock =
      prefs->GetPref(PWSprefs::LockOnWindowLock) ? TRUE : FALSE;
  m_OPTMD.LockOnIdleTimeout =
      prefs->GetPref(PWSprefs::LockDBOnIdleTimeout) ? TRUE : FALSE;
  m_OPTMD.IdleTimeOut =
      prefs->GetPref(PWSprefs::IdleTimeout);
  m_OPTMD.HashIters = GetMainDlg()->GetHashIters();
  m_OPTMD.CopyPswdBrowseURL =
      prefs->GetPref(PWSprefs::CopyPasswordWhenBrowseToURL) ? TRUE : FALSE;
  // Preferences min/max values
  m_OPTMD.prefminIdleTimeout = (short)prefs->GetPrefMinVal(PWSprefs::IdleTimeout);
  m_OPTMD.prefmaxIdleTimeout = (short)prefs->GetPrefMaxVal(PWSprefs::IdleTimeout);
  
  // Shortcut Data
  m_OPTMD.AppHotKeyValue = int32(prefs->GetPref(PWSprefs::HotKey));
  // Can't be enabled if not set!
  if (m_OPTMD.AppHotKeyValue == 0)
    m_OPTMD.AppHotKeyEnabled = FALSE;
  else
    m_OPTMD.AppHotKeyEnabled =
      prefs->GetPref(PWSprefs::HotKeyEnabled) ? TRUE : FALSE;

  m_OPTMD.ColWidth =
      prefs->GetPref(PWSprefs::OptShortcutColumnWidth);
  m_OPTMD.DefColWidth =
      prefs->GetPrefDefVal(PWSprefs::OptShortcutColumnWidth);
  
  // System Data
  CShortcut pws_shortcut;
  m_OPTMD.MaxREItems =
      prefs->GetPref(PWSprefs::MaxREItems);
  m_OPTMD.UseSystemTray =
      prefs->GetPref(PWSprefs::UseSystemTray) ? TRUE : FALSE;
  m_OPTMD.HideSystemTray =
      prefs->GetPref(PWSprefs::HideSystemTray) ? TRUE : FALSE;
  m_OPTMD.MaxMRUItems =
      prefs->GetPref(PWSprefs::MaxMRUItems);
  m_OPTMD.MRUOnFileMenu =
      prefs->GetPref(PWSprefs::MRUOnFileMenu);
  const CString PWSLnkName(L"Password Safe"); // for startup shortcut
  m_OPTMD.Startup = m_bStartupShortcutExists =
      pws_shortcut.isLinkExist(PWSLnkName, CSIDL_STARTUP);
  m_OPTMD.DefaultOpenRO = prefs->GetPref(PWSprefs::DefaultOpenRO) ? TRUE : FALSE;
  m_OPTMD.MultipleInstances =
      prefs->GetPref(PWSprefs::MultipleInstances) ? TRUE : FALSE;
  // Preferences min/max values
  m_OPTMD.prefminREItems = (short)prefs->GetPrefMinVal(PWSprefs::MaxREItems);
  m_OPTMD.prefmaxREItems = (short)prefs->GetPrefMaxVal(PWSprefs::MaxREItems);
  m_OPTMD.prefminMRU = (short)prefs->GetPrefMinVal(PWSprefs::MaxMRUItems);
  m_OPTMD.prefmaxMRU = (short)prefs->GetPrefMaxVal(PWSprefs::MaxMRUItems);
}

void COptions_PropertySheet::UpdateCopyPreferences()
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Now update the Application preferences.
  // In PropertyPage alphabetic order
  // Note: Updating the COPY values - especially important for DB preferences!!!

  // Backup
  prefs->SetPref(PWSprefs::BackupBeforeEverySave,
                 m_OPTMD.BackupBeforeSave == TRUE, true);
  prefs->SetPref(PWSprefs::BackupPrefixValue,
                 LPCWSTR(m_OPTMD.UserBackupPrefix), true);
  prefs->SetPref(PWSprefs::BackupSuffix,
                 (unsigned int)m_OPTMD.BackupSuffix, true);
  prefs->SetPref(PWSprefs::BackupMaxIncremented,
                 m_OPTMD.MaxNumIncBackups, true);
  if (!m_OPTMD.UserBackupOtherLocation.IsEmpty()) {
    // Make sure it ends in a slash!
    if (m_OPTMD.UserBackupOtherLocation.Right(1) != CSecString(L"\\"))
      m_OPTMD.UserBackupOtherLocation += L'\\';
  }
  prefs->SetPref(PWSprefs::BackupDir,
                 LPCWSTR(m_OPTMD.UserBackupOtherLocation), true);

  // Display
  prefs->SetPref(PWSprefs::AlwaysOnTop,
                 m_OPTMD.AlwaysOnTop == TRUE, true);
  prefs->SetPref(PWSprefs::ShowNotesAsTooltipsInViews,
                 m_OPTMD.ShowNotesAsTipsInViews == TRUE, true);
  prefs->SetPref(PWSprefs::ExplorerTypeTree,
                 m_OPTMD.ExplorerTypeTree == TRUE, true);
  prefs->SetPref(PWSprefs::ListViewGridLines,
                 m_OPTMD.EnableGrid == TRUE, true);
  prefs->SetPref(PWSprefs::NotesWordWrap,
                 m_OPTMD.WordWrapNotes == TRUE, true);
  prefs->SetPref(PWSprefs::PreExpiryWarn,
                 m_OPTMD.PreExpiryWarn == TRUE, true);
  prefs->SetPref(PWSprefs::PreExpiryWarnDays,
                 m_OPTMD.PreExpiryWarnDays, true);
  prefs->SetPref(PWSprefs::HighlightChanges,
                  m_OPTMD.HighlightChanges == TRUE, true);
  prefs->SetPref(PWSprefs::EnableWindowTransparency,
                  m_OPTMD.EnableTransparency == TRUE, true);
  prefs->SetPref(PWSprefs::WindowTransparency,
                  m_OPTMD.PercentTransparency, true);
  
  // Changes are highlighted only if "hightlight changes" is true and 
  // "save immediately" is false.
  // So only need to refresh view if the new combination is different
  // to the original combination
  m_bRefreshViews = (m_save_bHighlightChanges && !m_save_bSaveImmediately) != 
                    (m_OPTMD.HighlightChanges && !m_OPTMD.SaveImmediately);

  // Misc
  prefs->SetPref(PWSprefs::DeleteQuestion,
                 m_OPTMD.ConfirmDelete == FALSE, true);
  prefs->SetPref(PWSprefs::EscExits,
                 m_OPTMD.EscExits == TRUE, true);
  // by strange coincidence, the values of the enums match the indices
  // of the radio buttons in the following :-)
  prefs->SetPref(PWSprefs::DoubleClickAction,
                 (unsigned int)m_OPTMD.DoubleClickAction, true);
  prefs->SetPref(PWSprefs::ShiftDoubleClickAction,
                 (unsigned int)m_OPTMD.ShiftDoubleClickAction, true);

  prefs->SetPref(PWSprefs::QuerySetDef,
                 m_OPTMD.QuerySetDef == TRUE, true);
  prefs->SetPref(PWSprefs::AltBrowser,
                 LPCWSTR(m_OPTMD.OtherBrowserLocation), true);
  prefs->SetPref(PWSprefs::AltBrowserCmdLineParms,
                 LPCWSTR(m_OPTMD.OtherBrowserCmdLineParms), true);
  prefs->SetPref(PWSprefs::AltNotesEditor,
                 LPCWSTR(m_OPTMD.OtherEditorLocation), true);
  prefs->SetPref(PWSprefs::AltNotesEditorCmdLineParms,
                 LPCWSTR(m_OPTMD.OtherEditorCmdLineParms), true);
  prefs->SetPref(PWSprefs::MinimizeOnAutotype,
                 m_OPTMD.MinAuto == TRUE, true);

  prefs->SetPref(PWSprefs::ClearClipboardOnMinimize,
                 m_OPTMD.ClearClipboardOnMinimize == TRUE, true);
  prefs->SetPref(PWSprefs::ClearClipboardOnExit,
                 m_OPTMD.ClearClipboardOnExit == TRUE, true);
  prefs->SetPref(PWSprefs::DatabaseClear,
                 m_OPTMD.LockOnMinimize == TRUE, true);
  prefs->SetPref(PWSprefs::DontAskQuestion,
                 m_OPTMD.ConfirmCopy == FALSE, true);
  prefs->SetPref(PWSprefs::LockOnWindowLock,
                 m_OPTMD.LockOnWindowLock == TRUE, true);
  prefs->SetPref(PWSprefs::CopyPasswordWhenBrowseToURL,
                 m_OPTMD.CopyPswdBrowseURL == TRUE, true);

  prefs->SetPref(PWSprefs::UseSystemTray,
                 m_OPTMD.UseSystemTray == TRUE, true);
  prefs->SetPref(PWSprefs::HideSystemTray,
                 m_OPTMD.HideSystemTray == TRUE, true);

  prefs->SetPref(PWSprefs::MaxREItems,
                 m_OPTMD.MaxREItems, true);
  prefs->SetPref(PWSprefs::MaxMRUItems,
                 m_OPTMD.MaxMRUItems, true);
  if (m_OPTMD.MaxMRUItems == 0) {
    // Put them on File menu where they don't take up any room
    prefs->SetPref(PWSprefs::MRUOnFileMenu, true, true);
  } else {
    prefs->SetPref(PWSprefs::MRUOnFileMenu,
                   m_OPTMD.MRUOnFileMenu == TRUE, true);
  }
  prefs->SetPref(PWSprefs::DefaultOpenRO,
                 m_OPTMD.DefaultOpenRO == TRUE, true);
  prefs->SetPref(PWSprefs::MultipleInstances,
                 m_OPTMD.MultipleInstances == TRUE, true);

  // Now update database preferences
  // In PropertyPage alphabetic order
  prefs->SetPref(PWSprefs::SaveImmediately,
                 m_OPTMD.SaveImmediately == TRUE, true);

  prefs->SetPref(PWSprefs::ShowPWDefault,
                 m_OPTMD.ShowPasswordInEdit == TRUE, true);
  prefs->SetPref(PWSprefs::ShowUsernameInTree,
                 m_OPTMD.ShowUsernameInTree == TRUE, true);
  prefs->SetPref(PWSprefs::ShowPasswordInTree,
                 m_OPTMD.ShowPasswordInTree == TRUE, true);
  prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen,
                 m_OPTMD.TreeDisplayStatusAtOpen, true);
  prefs->SetPref(PWSprefs::ShowNotesDefault,
                 m_OPTMD.NotesShowInEdit == TRUE, true);

  prefs->SetPref(PWSprefs::MaintainDateTimeStamps,
                 m_OPTMD.MaintainDatetimeStamps == TRUE, true);

  prefs->SetPref(PWSprefs::UseDefaultUser,
                 m_OPTMD.UseDefuser == TRUE, true);
  prefs->SetPref(PWSprefs::DefaultUsername,
                 LPCWSTR(m_OPTMD.DefUsername), true);

  if (m_OPTMD.AutotypeText.IsEmpty() || m_OPTMD.AutotypeText == DEFAULT_AUTOTYPE)
    prefs->SetPref(PWSprefs::DefaultAutotypeString, L"", true);
  else if (m_OPTMD.AutotypeText != DEFAULT_AUTOTYPE)
    prefs->SetPref(PWSprefs::DefaultAutotypeString,
                   LPCWSTR(m_OPTMD.AutotypeText), true);
  prefs->SetPref(PWSprefs::DefaultAutotypeDelay,
                 m_OPTMD.AutotypeDelay,
                 true);

  prefs->SetPref(PWSprefs::SavePasswordHistory,
                 m_OPTMD.SavePWHistory == TRUE, true);
  if (m_OPTMD.SavePWHistory == TRUE)
    prefs->SetPref(PWSprefs::NumPWHistoryDefault,
                   m_OPTMD.PWHistoryNumDefault, true);

  prefs->SetPref(PWSprefs::DefaultExpiryDays, m_OPTMD.PWHDefExpDays, true);

  prefs->SetPref(PWSprefs::LockDBOnIdleTimeout,
                 m_OPTMD.LockOnIdleTimeout == TRUE, true);
  prefs->SetPref(PWSprefs::IdleTimeout,
                 m_OPTMD.IdleTimeOut, true);

  // Changing ExplorerTypeTree changes order of items,
  // which DisplayStatus implicitly depends upon
  if (m_save_bExplorerTypeTree != m_OPTMD.ExplorerTypeTree)
    m_bSaveGroupDisplayState = m_bRefreshViews = true;

  // If user has turned on/changed warnings of expired passwords - check now
  if (m_OPTMD.PreExpiryWarn      == TRUE   &&
      (m_save_bPreExpiryWarn     == FALSE  ||
       m_save_iPreExpiryWarnDays != m_OPTMD.PreExpiryWarnDays))
    m_bCheckExpired = m_bRefreshViews = true;

  // Deal with shortcuts
  prefs->SetPref(PWSprefs::HotKey,
                 m_OPTMD.AppHotKeyValue, true);
  prefs->SetPref(PWSprefs::HotKeyEnabled,
                 m_OPTMD.AppHotKeyEnabled == TRUE, true);

  if (m_pp_shortcuts->HaveShortcutsChanged())
    m_bUpdateShortcuts = true;

  // Now copy across application preferences
  // Any changes via Database preferences done via call to UpdateGUI from Command
  prefs->UpdateFromCopyPrefs(PWSprefs::ptApplication);

  // Keep prefs file updated
  prefs->SaveApplicationPreferences();
}
