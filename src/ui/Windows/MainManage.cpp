/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "OptionsPasswordHistory.h"
#include "OptionsMisc.h"
#include "OptionsBackup.h"
#include "OptionsShortcuts.h"
#include "AddEdit_DateTimes.h"
#include "PasswordPolicyDlg.h"
#include "ManagePSWDPolices.h"

#include "core/pwsprefs.h"
#include "core/PWSdirs.h"
#include "core/PWSAuxParse.h"

#include "os/dir.h"
#include "os/logit.h"

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

  rc = m_core.WriteFile(tempname, false);
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
  PWS_LOGIT;

  PWSprefs *prefs = PWSprefs::GetInstance();

  // Get old DB preferences String value for use later
  const StringX sxOldDBPrefsString(prefs->Store());

  bool bLongPPs = LongPPs();

  COptions_PropertySheet optionsPS(IDS_OPTIONS, this, bLongPPs);

  // Remove the "Apply Now" button.
  optionsPS.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  // Save Hotkey info
  DWORD hotkey_value;
  BOOL hotkey_enabled;
  
  hotkey_value = DWORD(prefs->GetPref(PWSprefs::HotKey));
  // Can't be enabled if not set!
  if (hotkey_value == 0)
    hotkey_enabled = FALSE;
  else
    hotkey_enabled = prefs->GetPref(PWSprefs::HotKeyEnabled) ? TRUE : FALSE;

  // Disable Hotkey around this as the user may press the current key when 
  // selecting the new key!
  UnregisterHotKey(m_hWnd, PWS_HOTKEY_ID); // clear last - never hurts

  INT_PTR rc = optionsPS.DoModal();

  if (rc == IDOK) {
    // Now update the application look and feel as appropriate

    // Get updated Hotkey information as we will either re-instate the original or
    // set these new values
    hotkey_enabled = optionsPS.GetHotKeyState();
    hotkey_value = optionsPS.GetHotKeyValue();

    // Update status bar
    UINT uiMessage(IDS_STATCOMPANY);
    switch (optionsPS.GetDCA()) {
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

    int iTrayColour = optionsPS.GetTrayIconColour();
    app.SetClosedTrayIcon(iTrayColour);

    UpdateSystemMenu();
    
    if (optionsPS.GetMaxMRUItems() == 0)
      app.ClearMRU();  // Clear any currently saved
    
    UpdateAlwaysOnTop();
    
    DWORD dwExtendedStyle = m_ctlItemList.GetExtendedStyle();
    bool bGridLines = ((dwExtendedStyle & LVS_EX_GRIDLINES) == LVS_EX_GRIDLINES);

    if (optionsPS.GetEnableGrid() != bGridLines) {
      if (optionsPS.GetEnableGrid()) {
        dwExtendedStyle |= LVS_EX_GRIDLINES;
      } else {
        dwExtendedStyle &= ~LVS_EX_GRIDLINES;
      }
      m_ctlItemList.SetExtendedStyle(dwExtendedStyle);
    }

    m_ctlItemTree.ActivateND(optionsPS.GetNotesAsTips());
    m_ctlItemList.ActivateND(optionsPS.GetNotesAsTips());

    m_RUEList.SetMax(optionsPS.GetMaxREItems());
    
    if (optionsPS.StartupShortcutChanged()) {
      CShortcut pws_shortcut;
      const CString PWSLnkName(L"Password Safe"); // for startup shortcut
      if (optionsPS.StartupShortcut()) {
        // Put it there
        wchar_t exeName[MAX_PATH];
        GetModuleFileName(NULL, exeName, MAX_PATH);
        pws_shortcut.SetCmdArguments(CString(L"-s"));
        pws_shortcut.CreateShortCut(exeName, PWSLnkName, CSIDL_STARTUP);
      } else {
        // remove existing startup shortcut
        pws_shortcut.DeleteShortCut(PWSLnkName, CSIDL_STARTUP);
      }
    }

    // Update Lock on Window Lock
    if (optionsPS.LockOnWindowLockChanged()) {
      if (optionsPS.LockOnWindowLock()) {
        startLockCheckTimer();
      } else {
        KillTimer(TIMER_LOCKONWTSLOCK);
      }
    }

    if (optionsPS.RefreshViews()) {
      m_ctlItemList.SetHighlightChanges(optionsPS.HighlightChanges());
      m_ctlItemTree.SetHighlightChanges(optionsPS.HighlightChanges());
      RefreshViews();
    }

    if (optionsPS.SaveGroupDisplayState())
      SaveGroupDisplayState();

    if (optionsPS.UpdateShortcuts()) {
      // Create vector of shortcuts for user's config file
      std::vector<st_prefShortcut> vShortcuts;
      MapMenuShortcutsIter iter, iter_entry, iter_group;
      m_MapMenuShortcuts = optionsPS.GetMaps();

      for (iter = m_MapMenuShortcuts.begin(); iter != m_MapMenuShortcuts.end();
           iter++) {
        // User should not have these sub-entries in their config file
        if (iter->first == ID_MENUITEM_GROUPENTER  ||
            iter->first == ID_MENUITEM_VIEW        || 
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

    if (optionsPS.CheckExpired()) {
      CheckExpireList();
      TellUserAboutExpiredPasswords();
    }

    // Get new DB preferences String value
    StringX sxNewDBPrefsString(prefs->Store(true));

    // Maybe needed if this causes changes to database
    // (currently only DB preferences + updating PWHistory in existing entries)
    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

    // Set up Command to update string in database, if necessary & possible
    // Allow even if no entries (yet) in database and if the database is R-O.
    // In the latter case - just won't be saved but will do what the user wants
    // in this session with this database, until the database is closed.
    if (m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
      if (sxOldDBPrefsString != sxNewDBPrefsString) {
        // Determine whether Tree needs redisplayng due to change
        // in what is shown (e.g. usernames/passwords)
        bool bUserDisplayChanged = optionsPS.UserDisplayChanged();
        // Note: passwords are only shown in the Tree if usernames are also shown
        bool bPswdDisplayChanged = optionsPS.PswdDisplayChanged();
        bool bNeedGUITreeUpdate = bUserDisplayChanged || 
                 (optionsPS.ShowUsernameInTree() && bPswdDisplayChanged);
        if (bNeedGUITreeUpdate) {
          Command *pcmd = UpdateGUICommand::Create(&m_core,
                                                   UpdateGUICommand::WN_UNDO,
                                                   UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED);
          pmulticmds->Add(pcmd);
        }
        Command *pcmd = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
        pmulticmds->Add(pcmd);
        if (bNeedGUITreeUpdate) {
          Command *pcmd = UpdateGUICommand::Create(&m_core,
                                                   UpdateGUICommand::WN_EXECUTE_REDO,
                                                   UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED);
          pmulticmds->Add(pcmd);
        }
      }
    }

    const int iAction = optionsPS.GetPWHAction();
    const int new_default_max = optionsPS.GetPWHistoryMax();
    size_t ipwh_exec(0);
    int num_altered(0);

    if (iAction != 0) {
      Command *pcmd1 = UpdateGUICommand::Create(&m_core,
                                                UpdateGUICommand::WN_UNDO,
                                                UpdateGUICommand::GUI_PWH_CHANGED_IN_DB);
      pmulticmds->Add(pcmd1);
      Command *pcmd = UpdatePasswordHistoryCommand::Create(&m_core,
                                                           iAction,
                                                           new_default_max);
      pmulticmds->Add(pcmd);
      ipwh_exec = pmulticmds->GetSize();

      Command *pcmd2 = UpdateGUICommand::Create(&m_core,
                                                UpdateGUICommand::WN_EXECUTE_REDO,
                                                UpdateGUICommand::GUI_PWH_CHANGED_IN_DB);
      pmulticmds->Add(pcmd2);
    }

    // If DB preferences changed and/or password history options
    if (pmulticmds != NULL) {
      if (pmulticmds->GetSize() > 0) {
        Execute(pmulticmds);
        if (ipwh_exec > 0) {
          // We did do PWHistory update
          if (pmulticmds->GetRC(ipwh_exec, num_altered)) {
            UINT uimsg_id(0);
            switch (iAction) {
              case -1:   // reset off - include protected entries
              case  1:   // reset off - exclude protected entries
                uimsg_id = IDS_ENTRIESCHANGEDSTOP;
                break;
              case -2:   // reset on - include protected entries
              case  2:   // reset on - exclude protected entries
                uimsg_id = IDS_ENTRIESCHANGEDSAVE;
                break;
              case -3:   // setmax - include protected entries
              case  3:   // setmax - exclude protected entries
                uimsg_id = IDS_ENTRIESRESETMAX;
                break;
              case -4:   // clearall - include protected entries
              case  4:   // clearall - exclude protected entries
                uimsg_id = IDS_ENTRIESCLEARALL;
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

    if (m_core.HaveDBPrefsChanged() || num_altered > 0) {
      if (m_core.HaveDBPrefsChanged())
        SetChanged(DBPrefs);
      if (num_altered > 0)
        SetChanged(Data);
      
      ChangeOkUpdate();
    }
  }

  // Restore hotkey as it was or as user changed it - if user pressed OK
  if (hotkey_enabled == TRUE) {
    WORD wVirtualKeyCode = WORD(hotkey_value & 0xffff);
    WORD mod = WORD(hotkey_value >> 16);
    WORD wModifiers = 0;
    // Translate between CHotKeyCtrl & CWnd modifiers
    if (mod & HOTKEYF_ALT) 
      wModifiers |= MOD_ALT; 
    if (mod & HOTKEYF_CONTROL) 
      wModifiers |= MOD_CONTROL; 
    if (mod & HOTKEYF_SHIFT) 
      wModifiers |= MOD_SHIFT; 
    BOOL brc = RegisterHotKey(m_hWnd, PWS_HOTKEY_ID,
                              UINT(wModifiers), UINT(wVirtualKeyCode));
    if (brc == FALSE) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_NOHOTKEY, MB_OK);
    }
  }

  // Update Minidump user streams
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly(), usPrefs);
}

void DboxMain::OnGeneratePassword()
{
  PSWDPolicyMap MapPSWDPLC = GetPasswordPolicies();

  PWSprefs *prefs = PWSprefs::GetInstance();
  st_PSWDPolicy st_default_pp;

  if (prefs->GetPref(PWSprefs::PWUseLowercase))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseLowercase;
  if (prefs->GetPref(PWSprefs::PWUseUppercase))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseUppercase;
  if (prefs->GetPref(PWSprefs::PWUseDigits))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseDigits;
  if (prefs->GetPref(PWSprefs::PWUseSymbols))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseSymbols;
  if (prefs->GetPref(PWSprefs::PWUseHexDigits))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
  if (prefs->GetPref(PWSprefs::PWUseEasyVision))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
  if (prefs->GetPref(PWSprefs::PWMakePronounceable))
    st_default_pp.pwp.flags |= PWSprefs::PWPolicyMakePronounceable;

  st_default_pp.pwp.length = prefs->GetPref(PWSprefs::PWDefaultLength);
  st_default_pp.pwp.digitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
  st_default_pp.pwp.lowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
  st_default_pp.pwp.symbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
  st_default_pp.pwp.upperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);

  st_default_pp.symbols = prefs->GetPref(PWSprefs::DefaultSymbols);

  bool bLongPPs = LongPPs();
  CPasswordPolicyDlg GenPswdPS(IDS_GENERATEPASSWORD, this, bLongPPs, 
                               IsDBReadOnly(), st_default_pp);

  // Pass default values, PolicyName map
  CString cs_poliyname(L"");
  GenPswdPS.SetPolicyData(cs_poliyname, MapPSWDPLC, this);

  GenPswdPS.DoModal();
}

void DboxMain::OnManagePasswordPolicies()
{
  bool bLongPPs = LongPPs();

  PWSprefs *prefs = PWSprefs::GetInstance();
  
  // Set up copy of preferences
  prefs->SetupCopyPrefs();
  
  CManagePSWDPolices ManagePSWDPoliciesDlg(this, bLongPPs);
  
  st_PSWDPolicy st_old_default_pp;

  // Let ManagePSWDPoliciesDlg fill out database default policy during construction
  ManagePSWDPoliciesDlg.GetDefaultPasswordPolicies(st_old_default_pp);
  
  INT_PTR rc = ManagePSWDPoliciesDlg.DoModal();
  
  if (rc == IDOK && ManagePSWDPoliciesDlg.IsChanged()) {
    // Get new DB preferences String value
    st_PSWDPolicy st_new_default_pp;
    PSWDPolicyMap MapPSWDPLC = ManagePSWDPoliciesDlg.GetPasswordPolicies(st_new_default_pp);
    
    // Maybe needed if this causes changes to database
    // (currently only DB default policy preferences + updating Named Policies)
    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);
    
    // Check for changes
    if (st_old_default_pp != st_new_default_pp) {
      // User has changed database default policy - need to update preferences
      // Update the copy only!
      PWSprefs *prefs = PWSprefs::GetInstance();

      prefs->SetPref(PWSprefs::PWUseLowercase,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyUseLowercase) != 0, true);
      prefs->SetPref(PWSprefs::PWUseUppercase,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyUseUppercase) != 0, true);
      prefs->SetPref(PWSprefs::PWUseDigits,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyUseDigits) != 0, true);
      prefs->SetPref(PWSprefs::PWUseSymbols,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyUseSymbols) != 0, true);
      prefs->SetPref(PWSprefs::PWUseHexDigits,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyUseHexDigits) != 0, true);
      prefs->SetPref(PWSprefs::PWUseEasyVision,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyUseEasyVision) != 0, true);
      prefs->SetPref(PWSprefs::PWMakePronounceable,
                 (st_new_default_pp.pwp.flags & PWSprefs::PWPolicyMakePronounceable) != 0, true);

      prefs->SetPref(PWSprefs::PWDefaultLength,
                     st_new_default_pp.pwp.length, true);
      prefs->SetPref(PWSprefs::PWDigitMinLength,
                     st_new_default_pp.pwp.digitminlength, true);
      prefs->SetPref(PWSprefs::PWLowercaseMinLength,
                     st_new_default_pp.pwp.lowerminlength, true);
      prefs->SetPref(PWSprefs::PWSymbolMinLength,
                     st_new_default_pp.pwp.symbolminlength, true);
      prefs->SetPref(PWSprefs::PWUppercaseMinLength,
                     st_new_default_pp.pwp.upperminlength, true);
    
      prefs->SetPref(PWSprefs::DefaultSymbols,
                     st_new_default_pp.symbols, true);

      // Now get new DB preferences String value
      StringX sxNewDBPrefsString(prefs->Store(true));

      // Set up Command to update string in database
      if (m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
          Command *pcmd1 = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
          pmulticmds->Add(pcmd1);
      }
    }

    // Now update named preferences
    Command *pcmd2 = DBPolicyNamesCommand::Create(&m_core, MapPSWDPLC,
                             DBPolicyNamesCommand::REPLACEALL);
    pmulticmds->Add(pcmd2);
    Execute(pmulticmds);

    // Update Minidump user streams
    app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly(), usPrefs);
    
    ChangeOkUpdate();
  }
}
