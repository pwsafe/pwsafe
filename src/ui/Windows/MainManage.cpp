/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "PasskeyEntry.h"
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
#include "ManagePSWDPols.h"
#include "HKModifiers.h"
#include "YubiCfgDlg.h"

#include "core/core.h"
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
  if (!m_core.IsDbOpen())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  // SaveAs-type dialog box
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

  rc = m_core.WriteFile(tempname, m_core.GetReadFileVersion(), false);
  if (rc == PWScore::CANT_OPEN_FILE) {
    CGeneralMsgBox gmb;
    cs_temp.Format(IDS_CANTOPENWRITING, static_cast<LPCWSTR>(tempname.c_str()));
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
  StringX sx_backup, sx_passkey;
  StringX sx_currbackup =
    PWSprefs::GetInstance()->GetPref(PWSprefs::CurrentBackup);

  if (m_bOpen) {
    rc = SaveIfChanged();
    if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
      return rc;

    // Reset changed flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
    if (rc == PWScore::USER_DECLINED_SAVE)
      m_bUserDeclinedSave = true;
  }

  CString cs_text, cs_temp, cs_title;
  cs_text.LoadString(IDS_PICKRESTORE);

  std::wstring dir;
  if (!m_core.IsDbOpen())
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
                     sx_currbackup.c_str(),
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
      sx_backup = fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  rc = GetAndCheckPassword(sx_backup, sx_passkey, GCP_NORMAL);  // OK, CANCEL, HELP

  CGeneralMsgBox gmb;
  switch (rc) {
    case PWScore::SUCCESS:
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPEN, static_cast<LPCWSTR>(sx_backup.c_str()));
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
  m_core.SafeUnlockCurFile();

  // Reset core and clear ALL associated data
  m_core.ReInit();

  // Clear application data
  ClearAppData();

  // Validate it unless user says NO
  CReport Rpt;
  rc = m_core.ReadFile(sx_backup, sx_passkey, !m_bNoValidation, MAXTEXTCHARS, &Rpt);
  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENREADING, static_cast<LPCWSTR>(sx_backup.c_str()));
    cs_title.LoadString(IDS_FILEREADERROR);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  m_core.SetCurFile(L"");    // Force a Save As...
  // Rather than set the DB as having been changed to force it to
  // be saved, use new variable
  m_bRestoredDBUnsaved = true;

  m_titlebar.LoadString(IDS_UNTITLEDRESTORE);
  SetTooltipText(L"PasswordSafe");
  
  ChangeOkUpdate();
  RefreshViews();

  return PWScore::SUCCESS;
}

void DboxMain::OnOptions()
{
  PWS_LOGIT;

  PWSprefs *prefs = PWSprefs::GetInstance();

  // Get old DB preferences String value for use later
  const StringX sxOldDBPrefsString(prefs->Store());

  // Save current Window transparency in case we have to change it
  const BYTE byteOldPercentTransparency = (BYTE)prefs->GetPref(PWSprefs::WindowTransparency);
  const bool bOldTransparancyEnabled = prefs->GetPref(PWSprefs::EnableWindowTransparency);

  // Save Hotkey info
  BOOL bAppHotKeyEnabled;
  int32 iAppHotKeyValue = int32(prefs->GetPref(PWSprefs::HotKey));
  // Can't be enabled if not set!
  if (iAppHotKeyValue == 0)
    bAppHotKeyEnabled = FALSE;
  else
    bAppHotKeyEnabled = prefs->GetPref(PWSprefs::HotKeyEnabled) ? TRUE : FALSE;

  // Get current status of how the user wants to the initial display
  bool bTreeOpenStatus = prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen) != PWSprefs::AsPerLastSave;

  // Disable Hotkey around this as the user may press the current key when 
  // selecting the new key!
  UnregisterHotKey(m_hWnd, PWS_HOTKEY_ID); // clear last - never hurts

  COptions_PropertySheet *pOptionsPS(NULL);
  
  // Try Tall version
  pOptionsPS = new COptions_PropertySheet(IDS_OPTIONS, this, true);

  // Remove the "Apply Now" button.
  pOptionsPS->m_psh.dwFlags |= PSH_NOAPPLYNOW;

  INT_PTR rc = pOptionsPS->DoModal();

  if (rc < 0) {
    // Try again with Wide version
    delete pOptionsPS;
    pOptionsPS = new COptions_PropertySheet(IDS_OPTIONS, this, false);

    // Remove the "Apply Now" button.
    pOptionsPS->m_psh.dwFlags |= PSH_NOAPPLYNOW;
  
    rc = pOptionsPS->DoModal(); 
  }

  if (rc == IDOK) {
    // Now update the application look and feel as appropriate

    // Get updated Hotkey information as we will either re-instate the original or
    // set these new values
    bAppHotKeyEnabled = pOptionsPS->GetHotKeyState();
    iAppHotKeyValue = pOptionsPS->GetHotKeyValue();

    // Update status bar
    UINT uiMessage(IDSC_STATCOMPANY);
    switch (pOptionsPS->GetDCA()) {
      case PWSprefs::DoubleClickAutoType:
        uiMessage = IDSC_STATAUTOTYPE; break;
      case PWSprefs::DoubleClickBrowse:
        uiMessage = IDSC_STATBROWSE; break;
      case PWSprefs::DoubleClickCopyNotes:
        uiMessage = IDSC_STATCOPYNOTES; break;
      case PWSprefs::DoubleClickCopyPassword:
        uiMessage = IDSC_STATCOPYPASSWORD; break;
      case PWSprefs::DoubleClickCopyUsername:
        uiMessage = IDSC_STATCOPYUSERNAME; break;
      case PWSprefs::DoubleClickViewEdit:
        uiMessage = IDSC_STATVIEWEDIT; break;
      case PWSprefs::DoubleClickCopyPasswordMinimize:
        uiMessage = IDSC_STATCOPYPASSWORDMIN; break;
      case PWSprefs::DoubleClickBrowsePlus:
        uiMessage = IDSC_STATBROWSEPLUS; break;
      case PWSprefs::DoubleClickRun:
        uiMessage = IDSC_STATRUN; break;
      case PWSprefs::DoubleClickSendEmail:
        uiMessage = IDSC_STATSENDEMAIL; break;
      default:
        uiMessage = IDSC_STATCOMPANY;
    }
    statustext[CPWStatusBar::SB_DBLCLICK] = uiMessage;
    m_StatusBar.SetIndicators(statustext, CPWStatusBar::SB_TOTAL);
    UpdateStatusBar();

    // Make a sunken or recessed border around the first pane
    m_StatusBar.SetPaneInfo(CPWStatusBar::SB_DBLCLICK,
                            m_StatusBar.GetItemID(CPWStatusBar::SB_DBLCLICK),
                            SBPS_STRETCH, NULL);

    if (pOptionsPS->GetMaxMRUItems() == 0)
      app.ClearMRU();  // Clear any currently saved
    
    UpdateAlwaysOnTop();
    
    DWORD dwExtendedStyle = m_ctlItemList.GetExtendedStyle();
    bool bGridLines = ((dwExtendedStyle & LVS_EX_GRIDLINES) == LVS_EX_GRIDLINES);

    if (pOptionsPS->GetEnableGrid() != bGridLines) {
      if (pOptionsPS->GetEnableGrid()) {
        dwExtendedStyle |= LVS_EX_GRIDLINES;
      } else {
        dwExtendedStyle &= ~LVS_EX_GRIDLINES;
      }
      m_ctlItemList.SetExtendedStyle(dwExtendedStyle);
      m_ctlItemList.UpdateRowHeight(true);
    }

    m_ctlItemTree.ActivateND(pOptionsPS->GetNotesAsTips());
    m_ctlItemList.ActivateND(pOptionsPS->GetNotesAsTips());

    m_RUEList.SetMax(pOptionsPS->GetMaxREItems());
    
    if (pOptionsPS->StartupShortcutChanged()) {
      CShortcut pws_shortcut;
      const CString PWSLnkName(L"Password Safe"); // for startup shortcut
      if (pOptionsPS->StartupShortcut()) {
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
    if (pOptionsPS->LockOnWindowLockChanged()) {
      if (pOptionsPS->LockOnWindowLock()) {
        startLockCheckTimer();
      } else {
        KillTimer(TIMER_LOCKONWTSLOCK);
      }
    }

    m_ctlItemList.SetHighlightChanges(pOptionsPS->HighlightChanges() &&
      !pOptionsPS->SaveImmediately());
    m_ctlItemTree.SetHighlightChanges(pOptionsPS->HighlightChanges() &&
      !pOptionsPS->SaveImmediately());

    if (pOptionsPS->RefreshViews()) {
      RefreshViews();
    }

    if (pOptionsPS->UpdateShortcuts()) {
      // Create vector of shortcuts for user's config file
      std::vector<st_prefShortcut> vShortcuts;
      MapMenuShortcutsIter iter, iter_entry, iter_parent;
      m_MapMenuShortcuts = pOptionsPS->GetMaps();

      for (iter = m_MapMenuShortcuts.begin(); iter != m_MapMenuShortcuts.end();
           iter++) {
        // User should not have these sub-entries in their config file
        if (iter->first == ID_MENUITEM_GROUPENTER  ||
            iter->first == ID_MENUITEM_VIEWENTRY   || 
            iter->first == ID_MENUITEM_DELETEENTRY ||
            iter->first == ID_MENUITEM_DELETEGROUP ||
            iter->first == ID_MENUITEM_RENAMEENTRY ||
            iter->first == ID_MENUITEM_RENAMEGROUP) {
          continue;
        }

        // Now only those different from default
        if (iter->second.siVirtKey  != iter->second.siDefVirtKey  ||
            iter->second.cPWSModifier != iter->second.cDefPWSModifier) {
          iter_parent = m_MapMenuShortcuts.find(iter->second.uiParentID);
          std::wstring name(L"");
          do {
            name = iter_parent->second.name + std::wstring(L"->") + name;
            iter_parent = m_MapMenuShortcuts.find(iter_parent->second.uiParentID);
          } while (iter_parent != m_MapMenuShortcuts.end());

          name += iter->second.name;
          Remove(name, L'&');

          st_prefShortcut stxst;
          stxst.id = iter->first;
          stxst.siVirtKey = iter->second.siVirtKey;
          stxst.cPWSModifier = iter->second.cPWSModifier;
          stxst.Menu_Name = name;
          vShortcuts.push_back(stxst);
        }
      }

      prefs->SetPrefShortcuts(vShortcuts);
      prefs->SaveShortcuts();

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
        HideIcon();
      else if (IsIconVisible() == FALSE)
        ShowIcon();
    } else {
      HideIcon();
    }

    if (pOptionsPS->CheckExpired()) {
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
    Command *pcmd;

    // Save current horizontal scroll bar position in case view is refreshed
    if (m_ctlItemList.GetItemCount() == 0) {
      m_iListHBarPos = m_iTreeHBarPos = 0;
    } else {
      m_iListHBarPos = m_ctlItemList.GetScrollPos(SB_HORZ);
      m_iTreeHBarPos = m_ctlItemTree.GetScrollPos(SB_HORZ);
    }

    if (m_core.GetReadFileVersion() >= PWSfile::V30) { // older versions don't have prefs
      if (sxOldDBPrefsString != sxNewDBPrefsString ||
          m_core.GetHashIters() != pOptionsPS->GetHashIters()) {
        // Determine whether Tree needs redisplaying due to change
        // in what is shown (e.g. usernames/passwords)
        bool bUserDisplayChanged = pOptionsPS->UserDisplayChanged();
        // Note: passwords are only shown in the Tree if usernames are also shown
        bool bPswdDisplayChanged = pOptionsPS->PswdDisplayChanged();
        bool bNeedGUITreeUpdate = bUserDisplayChanged || 
                 (pOptionsPS->ShowUsernameInTree() && bPswdDisplayChanged);
        if (bNeedGUITreeUpdate) {
          pcmd = UpdateGUICommand::Create(&m_core,
                                                  UpdateGUICommand::WN_UNDO,
                                                  UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED);
          pmulticmds->Add(pcmd);
        }
        pcmd = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString, pOptionsPS->GetHashIters());
        pmulticmds->Add(pcmd);

        if (bNeedGUITreeUpdate) {
          pcmd = UpdateGUICommand::Create(&m_core,
                                                  UpdateGUICommand::WN_EXECUTE_REDO,
                                                  UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED);
          pmulticmds->Add(pcmd);
        }
      }

      // Save group display if the user has switched to Explorer mode or
      // has changed the TreeDisplayStatusAtOpen to AsPerLastSave
      if (pOptionsPS->SaveGroupDisplayState() ||
        (!bTreeOpenStatus &&
          prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen, true) == PWSprefs::AsPerLastSave)) {
        SaveGroupDisplayState();
      }
    } // file version check

    const int iAction = pOptionsPS->GetPWHAction();
    const int new_default_max = pOptionsPS->GetPWHistoryMax();
    size_t ipwh_exec(0);

    if (iAction != 0) {
      pcmd = UpdateGUICommand::Create(&m_core,
                                              UpdateGUICommand::WN_UNDO,
                                              UpdateGUICommand::GUI_PWH_CHANGED_IN_DB);
      pmulticmds->Add(pcmd);
      pcmd = UpdatePasswordHistoryCommand::Create(&m_core,
                                                          iAction,
                                                          new_default_max);
      pmulticmds->Add(pcmd);
      ipwh_exec = pmulticmds->GetSize();

      pcmd = UpdateGUICommand::Create(&m_core,
                                              UpdateGUICommand::WN_EXECUTE_REDO,
                                              UpdateGUICommand::GUI_PWH_CHANGED_IN_DB);
      pmulticmds->Add(pcmd);
    }

    // If DB preferences changed and/or password history options
    int num_altered(0);
    if (!pmulticmds->IsEmpty()) {
      // Do it
      Execute(pmulticmds);

      if (ipwh_exec > 0) {
        // We did PWHistory update
        if (pmulticmds->GetRC(ipwh_exec, num_altered)) {
          UINT uimsg_id(0);
          switch (iAction) {
          case PWHist::STOP_INCL_PROT:   // reset off - include protected entries
          case PWHist::STOP_EXCL_PROT:   // reset off - exclude protected entries
            uimsg_id = IDS_ENTRIESCHANGEDSTOP;
            break;
          case PWHist::START_INCL_PROT:   // reset on - include protected entries
          case PWHist::START_EXCL_PROT:   // reset on - exclude protected entries
            uimsg_id = IDS_ENTRIESCHANGEDSAVE;
            break;
          case PWHist::SETMAX_INCL_PROT:   // setmax - include protected entries
          case PWHist::SETMAX_EXCL_PROT:   // setmax - exclude protected entries
            uimsg_id = IDS_ENTRIESRESETMAX;
            break;
          case PWHist::CLEAR_INCL_PROT:   // clearall - include protected entries
          case PWHist::CLEAR_EXCL_PROT:   // clearall - exclude protected entries
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

        if (num_altered > 0) {
          ChangeOkUpdate();
        }
      } // ipwh_exec > 0

      // Restore current horizontal scroll bar position
      m_ctlItemList.Scroll(CSize(m_iListHBarPos, 0));
      m_ctlItemTree.SetScrollPos(SB_HORZ, m_iTreeHBarPos);
    } else { // multicmds was created but no commands added
      delete pmulticmds;
    }
  } // rc == IDOK
  
  if (m_bOnStartupTransparancyEnabled) {
    if (!prefs->GetPref(PWSprefs::EnableWindowTransparency)) {
      // User turned off transparency
      SetLayered(this, 0);
    } else {
      if (byteOldPercentTransparency != (BYTE)prefs->GetPref(PWSprefs::WindowTransparency)) {
        // User changed transparency
        SetLayered(this);
      }
    }
  }

  // Restore hotkey as it was or as user changed it - if user pressed OK
  m_core.SetAppHotKey(iAppHotKeyValue);

  if (bAppHotKeyEnabled == TRUE) {
    WORD wVirtualKeyCode = iAppHotKeyValue & 0xff;
    WORD wPWSModifiers = iAppHotKeyValue >> 16;

    // Translate from PWS modifer to Windows modifiers
    WORD wWinModifiers = ConvertModifersPWS2Windows(wPWSModifiers);

    // Only set if valid i.e. a character plus at least Alt or Ctrl
    if (wVirtualKeyCode != 0 && (wWinModifiers & (MOD_ALT | MOD_CONTROL)) != 0) {
      BOOL brc = RegisterHotKey(m_hWnd, PWS_HOTKEY_ID,
                                UINT(wWinModifiers), UINT(wVirtualKeyCode));
      if (brc == FALSE) {
        CGeneralMsgBox gmb;
        gmb.AfxMessageBox(IDS_NOHOTKEY, MB_OK);
        bAppHotKeyEnabled = FALSE;
      }
    } else {
      bAppHotKeyEnabled = FALSE;
    }
  }
  // Just in case we reset this being enabled
  prefs->SetPref(PWSprefs::HotKeyEnabled, bAppHotKeyEnabled == TRUE);

  // Update Minidump user streams
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly(), usPrefs);
  
  // Delete Options Property page
  delete pOptionsPS;
}

void DboxMain::OnGeneratePassword()
{
  PSWDPolicyMap MapPSWDPLC = GetPasswordPolicies();
  PWPolicy st_default_pp(PWSprefs::GetInstance()->GetDefaultPolicy());
  CPasswordPolicyDlg *pDlg(NULL);
  
  // Try Tall version
  pDlg = new CPasswordPolicyDlg(IDS_GENERATEPASSWORD, this, true, IsDBReadOnly(), st_default_pp);

  // Pass default values, PolicyName map
  CString cs_poliyname(L"");
  pDlg->SetPolicyData(cs_poliyname, MapPSWDPLC);

  INT_PTR rc = pDlg->DoModal();
  
  if (rc < 0) {
    // Try again with Wide version
    delete pDlg;
    pDlg = new CPasswordPolicyDlg(IDS_GENERATEPASSWORD, this, false, IsDBReadOnly(), st_default_pp);

    // Pass default values, PolicyName map
    pDlg->SetPolicyData(cs_poliyname, MapPSWDPLC);
  
    pDlg->DoModal(); 
  }

  // Delete generate password dialog
  delete pDlg;
}

void DboxMain::OnYubikey()
{
#ifndef NO_YUBI
  CYubiCfgDlg dlg(this, m_core);
  dlg.DoModal();
#endif /* NO_YUBI */
}

void DboxMain::OnManagePasswordPolicies()
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  
  // Set up copy of preferences
  prefs->SetupCopyPrefs();
  
  CManagePSWDPols *pDlg(NULL);
  
  // Try Tall version
  pDlg = new CManagePSWDPols(this, true);
  
  PWPolicy st_old_default_pp;

  // Let ManagePSWDPoliciesDlg fill out database default policy during construction
  pDlg->GetDefaultPasswordPolicies(st_old_default_pp);
  
  INT_PTR rc = pDlg->DoModal();
  
  if (rc < 0) {
    // Try again with Wide version
    delete pDlg;
    pDlg = new CManagePSWDPols(this, false);

    pDlg->DoModal(); 
  }
  
  if (rc == IDOK && pDlg->IsChanged()) {
    // Get new DB preferences String value
    PWPolicy st_new_default_pp;
    PSWDPolicyMap MapPSWDPLC = pDlg->GetPasswordPolicies(st_new_default_pp);
    
    // Maybe needed if this causes changes to database
    // (currently only DB default policy preferences + updating Named Policies)
    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);
    
    // Check for changes
    if (st_old_default_pp != st_new_default_pp) {
      // User has changed database default policy - need to update preferences
      // Update the copy only!
      PWSprefs::GetInstance()->SetDefaultPolicy(st_new_default_pp, true);

      // Now get new DB preferences String value
      StringX sxNewDBPrefsString(PWSprefs::GetInstance()->Store(true));

      // Set up Command to update string in database
      if (m_core.GetReadFileVersion() >= PWSfile::V30) { // older versions don't have prefs
        Command *pcmd_undo = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString);
        pmulticmds->Add(pcmd_undo);
      }
    }

    // Now update named preferences
    Command *pcmd = DBPolicyNamesCommand::Create(&m_core, MapPSWDPLC,
                             DBPolicyNamesCommand::NP_REPLACEALL);
    pmulticmds->Add(pcmd);

    // Do it
    Execute(pmulticmds);

    // Update Minidump user streams
    app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly(), usPrefs);
    
    ChangeOkUpdate();
  }
  
  // Delete Manage Password Policies dialog
  delete pDlg;
}
