/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file MenuManageHandlers.cpp
 *  This file contains implementations of PasswordSafeFrame
 *  member functions corresponding to actions under the 'Manage'
 *  menubar menu.
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/filename.h>

#include "core/PWSdirs.h"

#include "os/file.h"
#include "os/env.h"

#include "ManagePasswordPoliciesDlg.h"
#include "OptionsPropertySheetDlg.h"
#include "PasswordPolicyDlg.h"
#include "PasswordSafeFrame.h"
#include "PasswordSafeSearch.h"
#include "PWSafeApp.h"
#include "SafeCombinationChangeDlg.h"
#include "SafeCombinationPromptDlg.h"
#include "SystemTray.h"
#ifndef NO_YUBI
#include "YubiCfgDlg.h"
#endif

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CHANGECOMBO
 */

void PasswordSafeFrame::OnChangePasswordClick(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::DoChangePassword);
}

void PasswordSafeFrame::DoChangePassword()
{
  DestroyWrapper<SafeCombinationChangeDlg> windowWrapper(this, m_core);
  SafeCombinationChangeDlg* window = windowWrapper.Get();
  int returnValue = window->ShowModal();
  if (returnValue == wxID_OK) {
    m_core.ChangePasskey(window->GetNewpasswd());
#ifndef NO_YUBI
  if (!window->IsYubiProtected())
    m_core.SetYubiSK(nullptr); // erase YubiSK, as it's no longer protected by Yuibkey
#endif
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_OPTIONS_M
 */

void PasswordSafeFrame::OnPreferencesClick(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::DoPreferencesClick);
}

void PasswordSafeFrame::DoPreferencesClick()
{
  PWSprefs* prefs = PWSprefs::GetInstance();
  bool showMenuSeprator = prefs->GetPref(PWSprefs::ShowMenuSeparator);
  bool autoAdjColWidth = prefs->GetPref(PWSprefs::AutoAdjColWidth);
  bool toolbarShowText = prefs->GetPref(PWSprefs::ToolbarShowText);
  const StringX sxOldDBPrefsString(prefs->Store());
  DestroyWrapper<OptionsPropertySheetDlg> windowWrapper(this, m_core);
  OptionsPropertySheetDlg* window = windowWrapper.Get();

  if (window->ShowModal() == wxID_OK) {
    if(showMenuSeprator != prefs->GetPref(PWSprefs::ShowMenuSeparator)) {
      UpdateMainToolbarSeparators(prefs->GetPref(PWSprefs::ShowMenuSeparator));
    }
    if((autoAdjColWidth != prefs->GetPref(PWSprefs::AutoAdjColWidth)) && IsGridView() && IsShown()) {
      Show(true);
    }
    if(toolbarShowText != prefs->GetPref(PWSprefs::ToolbarShowText)) {
      auto tb = GetToolBar();
      if(prefs->GetPref(PWSprefs::ToolbarShowText)) {
        tb->SetWindowStyle(tb->GetWindowStyle() | wxAUI_TB_TEXT);
        GetMainToolbarPane().MinSize(-1, 50); // workaround for issue #19241 (https://trac.wxwidgets.org/ticket/19241)
      }
      else {
        tb->SetWindowStyle(tb->GetWindowStyle() & ~wxAUI_TB_TEXT);
        GetMainToolbarPane().MinSize(-1, 25); // workaround for issue #19241 (https://trac.wxwidgets.org/ticket/19241)
      }

      tb->Realize();
      m_AuiManager.Update();
      DoLayout();
      SendSizeEvent();
    }
    
    StringX sxNewDBPrefsString(prefs->Store(true));
    // Update system tray icon if visible so changes show up immediately
    if (m_sysTray && prefs->GetPref(PWSprefs::UseSystemTray))
        m_sysTray->ShowIcon();

    if (m_core.IsDbOpen() && !m_core.IsReadOnly() &&
        m_core.GetReadFileVersion() >= PWSfile::V30) { // older versions don't have prefs
      if (sxOldDBPrefsString != sxNewDBPrefsString ||
          m_core.GetHashIters() != window->GetHashItersValue()) {
        Command *pcmd = DBPrefsCommand::Create(&m_core, sxNewDBPrefsString,
                                               window->GetHashItersValue());
        if (pcmd) {
            //I don't know why notifications should ever be suspended, but that's how
            //things were before I messed with them, so I want to limit the damage by
            //enabling notifications only as long as required and no more
            m_core.ResumeOnDBNotification();
            Execute(pcmd);  //deleted automatically
            m_core.SuspendOnDBNotification();
        }
      }
    }
  }
  if (!IsCloseInProgress()) {
    SetFocus();
  }
}

/*
 * Backup and Restore
 */
void PasswordSafeFrame::OnBackupSafe(wxCommandEvent& WXUNUSED(evt))
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  const wxFileName currbackup(towxstring(prefs->GetPref(PWSprefs::CurrentBackup)));

  const wxString title(_("Choose a Name for this Backup:"));

  wxString dir;
  if (!m_core.IsDbOpen())
    dir = towxstring(PWSdirs::GetSafeDir());
  else {
    wxFileName::SplitPath(towxstring(m_core.GetCurFile()), &dir, nullptr, nullptr);
    wxCHECK_RET(!dir.IsEmpty(), _("Could not parse current file path"));
  }

  //returns empty string if user cancels
  wxString wxbf = wxFileSelector(title,
                                 dir,
                                 currbackup.GetFullName(),
                                 wxT("bak"),
                                 _("Password Safe Backups (*.bak)|*.bak"),
                                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT,
                                 this);
  /*
  The wxFileSelector code says it appends the default extension if user
  doesn't type one, but it actually doesn't and I don't see the purported
  code in 2.8.10.  And doing it ourselves after the dialog has returned is
  risky because we might silently overwrite an existing file
  */

  //create a copy to avoid multiple conversions to StringX
  const StringX backupfile(tostringx(wxbf));

  if (!backupfile.empty()) {  //i.e. if user didn't cancel
    if (m_core.WriteFile(backupfile, m_core.GetReadFileVersion(),
                         false) == PWScore::CANT_OPEN_FILE) {
      wxMessageBox( wxbf << wxT("\n\n") << _("Could not open file for writing!"),
                    _("Write Error"), wxOK|wxICON_ERROR, this);
    }

    prefs->SetPref(PWSprefs::CurrentBackup, backupfile);
  }
}

void PasswordSafeFrame::OnRestoreSafe(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::DoRestoreSafe);
}

void PasswordSafeFrame::DoRestoreSafe()
{
  if (SaveIfChanged() != PWScore::SUCCESS)
    return;

  const wxFileName currbackup(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::CurrentBackup)));

  wxString dir;
  if (!m_core.IsDbOpen())
    dir = towxstring(PWSdirs::GetSafeDir());
  else {
    wxFileName::SplitPath(towxstring(m_core.GetCurFile()), &dir, nullptr, nullptr);
    wxCHECK_RET(!dir.IsEmpty(), _("Could not parse current file path"));
  }

  //returns empty string if user cancels
  wxString wxbf = wxFileSelector(_("Backup File"),
                                 dir,
                                 currbackup.GetFullName(),
                                 wxT("bak"),
                                 _("Password Safe Backups (*.bak)|*.bak|Password Safe Intermediate Backups (*.ibak)|*.ibak||"),
                                 wxFD_OPEN|wxFD_FILE_MUST_EXIST,
                                 this);
  if (wxbf.empty())
    return;

  DestroyWrapper<SafeCombinationPromptDlg> pwdprompt(this, m_core, wxbf);
  if (pwdprompt.Get()->ShowModal() == wxID_OK) {
    const StringX passkey = pwdprompt.Get()->GetPassword();
    // unlock the file we're leaving
    m_core.SafeUnlockCurFile();

    // Reset core and clear ALL associated data
    m_core.ReInit();

    // clear the application data before restoring
    ClearAppData();

    if (m_core.ReadFile(tostringx(wxbf), passkey, true, MAXTEXTCHARS) == PWScore::CANT_OPEN_FILE) {
      wxMessageBox(wxbf << wxT("\n\n") << _("Could not open file for reading!"),
                      _("File Read Error"), wxOK | wxICON_ERROR, this);
      return /*PWScore::CANT_OPEN_FILE*/;
    }

    m_core.SetCurFile(wxEmptyString);    // Force a Save As...
    m_bRestoredDBUnsaved = true; // So that the restored file will be saved

    SetTitle(_("Password Safe - <Untitled Restored Backup>"));

#ifdef NOT_YET
    app.SetTooltipText(L"PasswordSafe");
#endif

#ifdef NOT_YET
    ChangeOkUpdate();
#endif

    RefreshViews();
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_PWDPOLSM
 */

void PasswordSafeFrame::OnPwdPolsMClick( wxCommandEvent&  )
{
  CallAfter(&PasswordSafeFrame::DoPwdPolsMClick);
}

void PasswordSafeFrame::DoPwdPolsMClick()
{
  ShowModalAndGetResult<ManagePasswordPoliciesDlg>(this, m_core);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_GENERATE_PASSWORD
 */

void PasswordSafeFrame::OnGeneratePassword(wxCommandEvent& WXUNUSED(event))
{
  CallAfter(&PasswordSafeFrame::DoGeneratePassword);
}

void PasswordSafeFrame::DoGeneratePassword()
{
  PolicyManager policyManager(m_core);
  auto customPolicies = policyManager.GetPolicies();
  auto defaultPolicy  = policyManager.GetDefaultPolicy();
  auto defaultName    = policyManager.GetDefaultPolicyName();

  customPolicies[std2stringx(defaultName)] = defaultPolicy;

  DestroyWrapper<PasswordPolicyDlg> ppdlg(this, m_core, customPolicies, PasswordPolicyDlg::DialogType::GENERATOR);
  ppdlg.Get()->SetPolicyData(defaultName, defaultPolicy);

  ppdlg.Get()->ShowModal();
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_YUBIKEY_MNG
 */

void PasswordSafeFrame::OnYubikeyMngClick(wxCommandEvent& WXUNUSED(event))
{
  CallAfter(&PasswordSafeFrame::DoYubikeyMngClick);
}

void PasswordSafeFrame::DoYubikeyMngClick()
{
  ShowModalAndGetResult<YubiCfgDlg>(this, m_core);
}
#endif

/**
 * Changes the language on the fly to one of the supported languages.
 *
 * \see PasswordSafeFrame::Init() for currently supported languages.
 */
void PasswordSafeFrame::OnLanguageClick(wxCommandEvent& evt)
{
  Freeze();

  auto id = evt.GetId();
  // First, uncheck all language menu items, hence the previously selected but also the new one
  for (int menu_id = ID_LANGUAGE_BEGIN+1; menu_id<ID_LANGUAGE_END; menu_id++)
    GetMenuBar()->Check( menu_id, false );

  // If a new language has been selected successfully we have to
  // recreate the UI so that the language change takes effect
  wxLanguage userLang=std::get<0>(m_languages[id]);
  if (wxGetApp().ActivateLanguage(userLang, false)) {
    m_selectedLanguage = id;
    wxString userLangName=wxLocale::GetLanguageCanonicalName(userLang);
    if (!userLangName.IsEmpty()){
      PWSprefs::GetInstance()->SetPref(PWSprefs::LanguageFile, tostringx(userLangName));
      pws_os::Trace(L"Saved user-preferred language: name= %ls\n", ToStr(userLangName));
    }

    // Recreate menubar
    CreateMenubar();
    UpdateMenuBar();

    // Recreate toolbar
    RefreshToolbarButtons();

    // Update dragbar tooltips
    UpdateDragbarTooltips();

    // Recreate search bar
    wxCHECK_RET(m_search, wxT("Search object not created so far"));
    m_search->ReCreateSearchBar();
  } else {
    GetMenuBar()->Check( m_selectedLanguage, true );
  }

  Thaw();
  DoLayout();
  SendSizeEvent();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CHANGEMODE
 */

void PasswordSafeFrame::OnChangeMode(wxCommandEvent&)
{
  CallAfter(&PasswordSafeFrame::DoChangeMode);
}

void PasswordSafeFrame::DoChangeMode()
{
  // Don't bother doing anything if DB is read-only on disk
  bool bFileIsReadOnly;
  if (pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly) && bFileIsReadOnly)
    return;

  // Don't allow prior format versions to become R/W
  // Do allow current (and possible future 'experimental') formats
  if (m_core.GetReadFileVersion() >= PWSfile::VCURRENT)
    ChangeMode(true); // true means "prompt use for password".

  // Update Statusbar
  UpdateStatusBar();
  Refresh();
}

bool PasswordSafeFrame::ChangeMode(bool promptUser)
{
  // We need to prompt the user for password from r-o to r/w
  // when this is called with main window open. Arguably more
  // secure, s.t. an untrusted user can't change things.
  // When called as part of unlock, user just provided it.
  // From StatusBar and menu

  // Return value says change was successful
  const bool bWasRO = m_core.IsReadOnly();

  if (!bWasRO) { // R/W -> R-O
    // Try to save if any changes done to database
    int rc = SaveIfChanged();
    if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
      return false;

    if (rc == PWScore::USER_DECLINED_SAVE) {
       // But ask just in case
      wxMessageDialog mbox(this, _("You have chosen not to save your changes, which will be lost. Do you wish to continue?"), _("Change database mode?"), wxYES_NO | wxICON_QUESTION);
      if(mbox.ShowModal() == wxID_NO)
        return false;

      // User said No to the save - so we must back-out all changes since last save
      while (m_core.HasDBChanged()) {
        m_core.Undo();
      }
    } // USER_DECLINED_SAVE

    // Clear the Commands & DB pre-command states
    m_core.ClearCommands();
  } else if (promptUser) { // R-O -> R/W
    // Taken from GetAndCheckPassword.
    // We don't want all the other processing that GetAndCheckPassword does
    int rc = ShowModalAndGetResult<SafeCombinationPromptDlg>(this, m_core, towxstring(m_core.GetCurFile()));

    if(rc != wxID_OK)
      return false;
  } // R-O -> R/W

  bool doAgain = true;
  bool rc(true);
  
  while (doAgain) {
    doAgain = false;
    rc = true;
    stringT locker = L"";
    int iErrorCode(0);
    bool brc = m_core.ChangeMode(locker, iErrorCode);
    if (brc) {
      UpdateStatusBar();
    } else {
      rc = false;
      // Better give them the bad news!
    
      wxString cs_msg, cs_title(_("Requested mode change failed"));
      if (bWasRO) {
        switch (iErrorCode) {
          case PWScore::DB_HAS_CHANGED:
            // We did get the lock but the DB has been changed
            // Note: PWScore has already freed the lock
            // The user must close and re-open it in R/W mode
            cs_msg = _("The database has been changed since you opened it in R-O mode, so it is not possible to switch to R/W mode.\n\nClose the database and re-open it in R/W mode.");
            break;

          case PWScore::CANT_GET_LOCK:
          {
            stringT plkUser(_T(""));
            stringT plkHost(_T(""));
            int plkPid = -1;
          
            if(PWSUtil::GetLockerData(locker, plkUser, plkHost, plkPid) &&
               (plkUser == pws_os::getusername()) && (plkHost == pws_os::gethostname())) {
              wxMessageDialog dialog(this, _("Lock is done by yourself"), _("Remove Lock?"), wxYES_NO | wxICON_EXCLAMATION);
              if(dialog.ShowModal() == wxID_YES) {
                HANDLE handle = 0;
                pws_os::UnlockFile(m_core.GetCurFile().c_str(), handle);
                doAgain = true;
                continue;
              }
            }

            stringT cs_user_and_host, cs_PID, tmp_msg;
            cs_user_and_host = locker;
            size_t i_pid = locker.rfind(L':');
            if (i_pid != stringT::npos) {
              Format(cs_PID, _(" [Process ID=%ls]").c_str(), locker.substr(i_pid + 1).c_str());
              cs_user_and_host = locker.substr(0, i_pid);
            } else {
              cs_PID = L"";
            }

            Format(tmp_msg, _("Failed to switch from R-O to R/W because database is already open in R/W mode by:\n%ls %ls\n(Only one R/W access at a time is allowed)").c_str(),
                   cs_user_and_host.c_str(), cs_PID.c_str());
            cs_msg = towxstring(tmp_msg);
            break;
          }
          case PWSfile::CANT_OPEN_FILE:
            cs_msg = _("Failed to switch from R-O to R/W: Could not open the database. It may have been deleted or renamed or there may be a problem with its drive.");
            break;
          case PWSfile::END_OF_FILE:
            cs_msg = _("Failed to switch from R-O to R/W: The file appears to be too short to be a valid database.");
            break;
          case PWSfile::READ_FAIL:
            // Temporary use of this value to indicate DB is R-O at the file level
            // and so cannot change to R/W
            cs_msg = _("Given path is a directory or file is read-only");
            break;
          default:
            ASSERT(0);
        }
      } else {
        // Don't need fail code when going from R/W to R-O - only one issue -
        // could not release the lock!
        cs_msg = _("Failed to switch from R/W to R-O: Could not release database lock.\nTry exiting and restarting program.");
      }

      wxMessageDialog gmb(this, cs_msg, cs_title, wxOK | wxICON_ERROR);
      gmb.ShowModal();
    }
    
  }  // While dogAgain
  return rc;
}
