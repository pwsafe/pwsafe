/*
 * Copyright (c) 2003-2020 Rony Shapiro <ronys@pwsafe.org>.
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
  auto window = new SafeCombinationChangeDlg(this, m_core);
  int returnValue = window->ShowModal();
  if (returnValue == wxID_OK) {
    m_core.ChangePasskey(window->GetNewpasswd());
  }
  window->Destroy();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_OPTIONS_M
 */

void PasswordSafeFrame::OnPreferencesClick(wxCommandEvent& WXUNUSED(evt))
{
  PWSprefs* prefs = PWSprefs::GetInstance();
  bool showMenuSeprator = prefs->GetPref(PWSprefs::ShowMenuSeparator);
  bool optimizedCellSize = prefs->GetPref(PWSprefs::OptimizedCellSize);
  const StringX sxOldDBPrefsString(prefs->Store());
  OptionsPropertySheetDlg *window = new OptionsPropertySheetDlg(this, m_core);
  if (window->ShowModal() == wxID_OK) {
    if(showMenuSeprator != prefs->GetPref(PWSprefs::ShowMenuSeparator))
      ReCreateMainToolbarSeparator(prefs->GetPref(PWSprefs::ShowMenuSeparator));
    if((optimizedCellSize != prefs->GetPref(PWSprefs::OptimizedCellSize)) && IsGridView() && IsShown())
      Show(true);
    
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
  window->Destroy();
}

/*
 * Backup and Restore
 */
void PasswordSafeFrame::OnBackupSafe(wxCommandEvent& WXUNUSED(evt))
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  const wxFileName currbackup(towxstring(prefs->GetPref(PWSprefs::CurrentBackup)));

  const wxString title(_("Please Choose a Name for this Backup:"));

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

#ifdef NOT_YET
  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return PWScore::USER_CANCEL;
  }
#endif

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
  wxString wxbf = wxFileSelector(_("Please Choose a Backup to Restore:"),
                                 dir,
                                 currbackup.GetFullName(),
                                 wxT("bak"),
                                 _("Password Safe Backups (*.bak)|*.bak|Password Safe Intermediate Backups (*.ibak)|*.ibak||"),
                                 wxFD_OPEN|wxFD_FILE_MUST_EXIST,
                                 this);
  if (wxbf.empty())
    return;

#ifdef NOT_YET
  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return PWScore::USER_CANCEL;
  }
#endif

  SafeCombinationPromptDlg pwdprompt(this, m_core, wxbf);
  if (pwdprompt.ShowModal() == wxID_OK) {
    const StringX passkey = pwdprompt.GetPassword();
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
  ManagePasswordPoliciesDlg ppols(this, m_core);
  ppols.ShowModal();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_GENERATE_PASSWORD
 */

void PasswordSafeFrame::OnGeneratePassword(wxCommandEvent& WXUNUSED(event))
{
  PolicyManager policyManager(m_core);
  auto customPolicies = policyManager.GetPolicies();
  auto defaultPolicy  = policyManager.GetDefaultPolicy();
  auto defaultName    = policyManager.GetDefaultPolicyName();

  customPolicies[std2stringx(defaultName)] = defaultPolicy;

  PasswordPolicyDlg ppdlg(this, m_core, customPolicies, PasswordPolicyDlg::DialogType::GENERATOR);
  ppdlg.SetPolicyData(defaultName, defaultPolicy);

  ppdlg.ShowModal();
}

#ifndef NO_YUBI
/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_YUBIKEY_MNG
 */

void PasswordSafeFrame::OnYubikeyMngClick(wxCommandEvent& WXUNUSED(event))
{
  YubiCfgDlg ykCfg(this, m_core);
  ykCfg.ShowModal();
}
#endif

/**
 * Changes the language on the fly to one of the supported languages.
 *
 * \see PasswordSafeFrame::Init() for currently supported languages.
 */
void PasswordSafeFrame::OnLanguageClick(wxCommandEvent& evt)
{
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
    ReCreateMainToolbar();

    // Recreate dragbar
    ReCreateDragToolbar();

    // Recreate search bar
    wxCHECK_RET(m_search, wxT("Search object not created so far"));
    m_search->ReCreateSearchBar();
  } else {
    GetMenuBar()->Check( m_selectedLanguage, true );
  }
}
