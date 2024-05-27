/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file MenuFileHandlers.cpp
 *  This file contains implementations of PasswordSafeFrame
 *  member functions corresponding to actions under the 'File'
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
#include <wx/tokenzr.h>
#include <wx/utils.h> // for wxLaunchDefaultBrowser

#include "core/core.h"
#include "core/PWSdirs.h"
#include "core/XML/XMLDefs.h"
#include "os/sleep.h"
#include "os/file.h"
#include "os/env.h"

#include "CompareDlg.h"
#include "ExportTextWarningDlg.h"
#include "ImportTextDlg.h"
#include "ImportXmlDlg.h"
#include "MergeDlg.h"
#include "PasswordSafeFrame.h"
#include "PasswordSafeSearch.h"
#include "PropertiesDlg.h"
#include "PWSafeApp.h"
#include "SafeCombinationSetupDlg.h"
#include "SelectionCriteria.h"
#include "SyncWizard.h"
#include "SystemTray.h"
#include "TreeCtrl.h"
#include "wxUtilities.h"

void PasswordSafeFrame::DisplayFileWriteError(int rc, const StringX &fname)
{
  ASSERT(rc != PWScore::SUCCESS);

  wxString cs_temp, cs_title(_("Write Error"));
  switch (rc) {
  case PWScore::CANT_OPEN_FILE:
    cs_temp = fname.c_str();
    cs_temp += wxT("\n\n");
    cs_temp += _("Could not open file for writing!");
    break;
  case PWScore::FAILURE:
    cs_temp =_("Write operation failed!\nFile may have been corrupted.\nTry saving in a different location");
    break;
  default:
    cs_temp = fname.c_str();
    cs_temp += wxT("\n\n");
    cs_temp += _("Unknown error");
    break;
  }
  wxMessageDialog dlg(nullptr, cs_temp, cs_title, wxOK | wxICON_ERROR);
  dlg.ShowModal();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_NEW
 */

void PasswordSafeFrame::OnNewClick(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter([&](){PasswordSafeFrame::New();});
}

int PasswordSafeFrame::New()
{
  int rc, rc2;

  if (!m_core.IsReadOnly() && m_core.HasDBChanged()) {
    wxString msg(_("Do you want to save changes to the password database:"));
    msg += wxT("\n");
    msg += m_core.GetCurFile().c_str();
    wxMessageDialog mbox(this, msg, "Save changes?", wxCANCEL | wxYES_NO | wxICON_QUESTION);
    mbox.SetYesNoLabels(_("Save"), _("Discard"));
    rc = mbox.ShowModal();
    switch (rc) {
    case wxID_CANCEL:
      return PWScore::USER_CANCEL;
    case wxID_YES:
      rc2 = Save();
        /*
        Make sure that writing the file was successful
        */
        if (rc2 == PWScore::SUCCESS)
          break;
        else
          return PWScore::CANT_OPEN_FILE;
    case wxID_NO:
      UpdateStatusBar();
      UpdateMenuBar();
      break;
    default:
      ASSERT(0);
    }
  }

  StringX cs_newfile;
  rc = NewFile(cs_newfile);
  if (rc == PWScore::USER_CANCEL) {
    /*
    Everything stays as is...
    Worst case, they saved their file....
    */
    return PWScore::USER_CANCEL;
  }

  m_core.SetCurFile(cs_newfile);
  m_core.ClearFileUUID();

  rc = m_core.WriteCurFile();
  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, cs_newfile);
    return PWScore::USER_CANCEL;
  }
  SetLabel(PWSUtil::NormalizeTTT(wxT("Password Safe - ") + cs_newfile).c_str());

  m_sysTray->SetTrayStatus(SystemTray::TrayStatus::UNLOCKED);
  m_RUEList.ClearEntries();
  wxGetApp().recentDatabases().AddFileToHistory(towxstring(cs_newfile));
  ResetFilters();
  GetSearchBarPane().Hide(); // There is nothing to search for in an empty database
  m_AuiManager.Update();
  // XXX TODO: Reset IdleLockTimer, as preference has reverted to default
  return PWScore::SUCCESS;
}

int PasswordSafeFrame::NewFile(StringX &fname)
{
  wxString cs_text(_("Choose a name for the new database"));

  wxString cf(wxT("pwsafe")); // reasonable default for first time user
  wxString v3FileName = towxstring(PWSUtil::GetNewFileName(tostdstring(cf), DEFAULT_SUFFIX));
  wxString dir = towxstring(PWSdirs::GetSafeDir());
  int rc;

  while (1) { // Lock cannot be fetched or Cancel pressed
    
    wxFileDialog fd(static_cast<wxWindow *>(this), cs_text, dir, v3FileName,
                    _("psafe3 files (*.psafe3)|*.psafe3|All files(*.*; *)|*.*;*"),
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT | wxFD_CHANGE_DIR);
    rc = fd.ShowModal();

    if (rc == wxID_OK) {
      fname = fd.GetPath().c_str();
      wxFileName wxfn(fname.c_str());
      if (wxfn.GetExt().empty()) {
        wxfn.SetExt(DEFAULT_SUFFIX);
        fname = wxfn.GetFullPath().c_str();
      }
    } else
      return PWScore::USER_CANCEL;

    DestroyWrapper<SafeCombinationSetupDlg> dbox_pksetupWrapper(this);
    SafeCombinationSetupDlg* dbox_pksetup = dbox_pksetupWrapper.Get();
    rc = dbox_pksetup->ShowModal();

    if (rc == wxID_CANCEL)
      return PWScore::USER_CANCEL;  //User cancelled password entry

    // First lock the new file
    std::wstring locker(L""); // null init is important here
    if(!m_core.LockFile(fname.c_str(), locker)) {
      stringT plkUser(_T(""));
      stringT plkHost(_T(""));
      int plkPid = -1;
      bool fileLocked = false;
      if(PWSUtil::GetLockerData(locker, plkUser, plkHost, plkPid) &&
         (plkUser == pws_os::getusername()) && (plkHost == pws_os::gethostname())) {
        wxMessageDialog dialog(this, _("Lock is done by yourself"), _("Remove Lock?"), wxYES_NO | wxICON_EXCLAMATION);
        if(dialog.ShowModal() == wxID_YES) {
          HANDLE handle = 0;
          pws_os::UnlockFile(fname.c_str(), handle);
          if(m_core.LockFile(fname.c_str(), locker))
            fileLocked = true;
        }
      }
      if(!fileLocked) {
        wxString errmess;
        errmess = _("Could not lock file.\n");
        if (PWSUtil::HasValidLockerData(locker)) {
          errmess += _("Locked by ");
        }
        errmess += locker.c_str();
        wxMessageBox(wxString()<< fname.c_str() << wxT("\n\n") << errmess,
                     _("Error"), wxOK | wxICON_ERROR, this);
        continue;
      }
    }
    // Reset core and clear ALL associated data
    m_core.ReInit(true);

    // clear the application data before creating new file
    ClearAppData();

    PWSprefs::GetInstance()->SetDatabasePrefsToDefaults();
    const StringX &oldfilename = m_core.GetCurFile();
    // The only way we're the locker is if it's locked & we're !readonly
    if (!oldfilename.empty() &&
        !m_core.IsReadOnly() &&
        m_core.IsLockedFile(oldfilename.c_str())) {
      m_core.UnlockFile(oldfilename.c_str());
    }

    m_core.SetCurFile(fname);

    m_core.SetReadOnly(false); // new file can't be read-only...
    m_core.NewFile(dbox_pksetup->GetPassword());
#ifdef notyet
    startLockCheckTimer();
#endif
    return PWScore::SUCCESS;
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
 */

void PasswordSafeFrame::OnOpenClick(wxCommandEvent& WXUNUSED(evt))
{
  int rc = DoOpen(_("Open Password Database"));

  if (rc == PWScore::SUCCESS) {
    m_core.ResumeOnDBNotification();
    CreateMenubar(); // Recreate the menu with updated list of most recently used DBs
    UpdateSearchBarVisibility();
    m_AuiManager.Update();
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_CLOSE
 */

void PasswordSafeFrame::OnCloseClick(wxCommandEvent& WXUNUSED(evt))
{
  CloseDB(nullptr);
}

void PasswordSafeFrame::OnLockSafe(wxCommandEvent&)
{
  IconizeOrHideAndLock();
}

void PasswordSafeFrame::OnUnlockSafe(wxCommandEvent&)
{
  CallAfter(&PasswordSafeFrame::UnlockSafe, true, true);
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_CLEAR_MRU
 */

void PasswordSafeFrame::OnClearRecentHistory(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  wxGetApp().recentDatabases().Clear();
  CreateMenubar(); // Recreate the menu with cleared list of most recently used DBs
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ~ Open/Save
///////////////////////////////////////////////////////////////////////////////////////////////////

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_SAVE
 */

void PasswordSafeFrame::OnSaveClick(wxCommandEvent& WXUNUSED(evt))
{
  Save();
}

void PasswordSafeFrame::OnSaveAsClick(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  SaveAs();
}

int PasswordSafeFrame::SaveImmediately()
{
  // Get normal save to do this (code already there for intermediate backups)
  return Save(SaveType::IMMEDIATELY);
}

int PasswordSafeFrame::Save(SaveType savetype /* = SaveType::INVALID*/)
{
  stringT bu_fname; // used to undo backup if save failed
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Save Application related preferences
  prefs->SaveApplicationPreferences();
  prefs->SaveShortcuts();

  // Save Group Display State
  if (IsTreeView() && prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen) == PWSprefs::AsPerLastSave) {
    m_tree->SaveGroupDisplayState();
  }

  if (!m_core.IsDbOpen())
    return SaveAs();

  switch (m_core.GetReadFileVersion()) {
    case PWSfile::VCURRENT:
    case PWSfile::V40:
      if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
        unsigned int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
        int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
        std::wstring userBackupPrefix = prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
        std::wstring userBackupDir = prefs->GetPref(PWSprefs::BackupDir).c_str();
        if (!m_core.BackupCurFile(maxNumIncBackups, backupSuffix,
                                  userBackupPrefix, userBackupDir, bu_fname)) {
          switch (savetype) {
            case SaveType::NORMALEXIT:
              if (wxMessageBox(_("Unable to create intermediate backup.  Save database elsewhere or with another name?\n\nClick 'No' to exit without saving."),
                               _("Write Error"), wxYES_NO | wxICON_EXCLAMATION, this) == wxID_NO)
                return PWScore::SUCCESS;
              else
                return SaveAs();

            case SaveType::IMMEDIATELY:
              if (wxMessageBox(_("Unable to create intermediate backup.  Do you wish to save changes to your database without it?"),
                _("Write Error"), wxYES_NO | wxICON_EXCLAMATION, this) == wxID_NO)
                return PWScore::USER_CANCEL;
              break; // continue save
            case SaveType::INVALID:
              // No particular end of PWS exit i.e. user clicked Save or
              // saving a changed database before opening another
              wxMessageBox(_("Unable to create intermediate backup."), _("Write Error"), wxOK|wxICON_ERROR, this);
              return PWScore::USER_CANCEL;
            default:
              /*
               * SaveType::ENDSESSIONEXIT
               * SaveType::WTSLOGOFFEXIT
               * SaveType::FAILSAFESAVE
               */
               wxMessageBox(_("Unable to create intermediate backup."), _("Write Error"), wxOK|wxICON_ERROR, this);
               return SaveAs();
          }
        } // BackupCurFile failed
      } // BackupBeforeEverySave
      break;
    case PWSfile::NEWFILE:
    {
      // file version mis-match
      stringT NewName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(), DEFAULT_SUFFIX);

      wxString msg( wxString::Format(_("The original database, \"%ls\", is in pre-3.0 format. It will be unchanged.\nYour changes will be written as \"%ls\" in the new format, which is unusable by old versions of PasswordSafe. To save your changes in the old format, use the \"File->Export To-> Old (1.x or 2) format\" command."),
                                     m_core.GetCurFile().c_str(), NewName.c_str()));
      if (wxMessageBox(msg, _("File version warning"), wxOK|wxCANCEL|wxICON_INFORMATION, this) == wxID_CANCEL)
        return PWScore::USER_CANCEL;

      m_core.SetCurFile(NewName.c_str());
#if 0
      m_titlebar = PWSUtil::NormalizeTTT(wxT("Password Safe - ") +
                                         m_core.GetCurFile()).c_str();
      SetWindowText(LPCWSTR(m_titlebar));
      app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
      break;
    }
    default:
      ASSERT(0);
      break;
  } // switch on file version

  UUIDList RUElist;
  m_RUEList.GetRUEList(RUElist);
  m_core.SetRUEList(RUElist);

  // Note: Writing out in in V4 DB format if the DB is already V4,
  // otherwise as V3 (this include saving pre-3.0 DBs as a V3 DB!
  auto rc = m_core.WriteFile(m_core.GetCurFile(),
                             m_core.GetReadFileVersion() == PWSfile::V40 ? PWSfile::V40 : PWSfile::V30);

  if (rc != PWScore::SUCCESS) { // Save failed!
    // Restore backup, if we have one
    if (!bu_fname.empty() && m_core.IsDbOpen())
      pws_os::RenameFile(bu_fname, m_core.GetCurFile().c_str());
    // Show user that we have a problem
    DisplayFileWriteError(rc, m_core.GetCurFile());
    return rc;
  }

  UpdateStatusBar();
//  ChangeOkUpdate();

  // Added/Modified entries now saved - reverse it & refresh display
//  if (m_bUnsavedDisplayed)
//    OnShowUnsavedEntries();

//  if (m_bFilterActive && m_bFilterForStatus) {
//    m_ctlItemList.Invalidate();
//    m_ctlItemTree.Invalidate();
//  }

  // Only refresh views if not existing
  if (savetype != SaveType::NORMALEXIT)
    RefreshViews();

  return PWScore::SUCCESS;
}

int PasswordSafeFrame::SaveAs()
{
  const PWSfile::VERSION curver = m_core.GetReadFileVersion();

  if (curver != PWSfile::V30 && curver != PWSfile::V40 &&
      curver != PWSfile::UNKNOWN_VERSION) {
    if (wxMessageBox( wxString::Format(_("The original database, '%ls', is in pre-3.0 format. The data will now be written in the new format, which is unusable by old versions of PasswordSafe. To save the data in the old format, use the 'File->Export To-> Old (1.x or 2) format' command."),
                                        m_core.GetCurFile().c_str()), _("File version warning"),
                                        wxOK | wxCANCEL | wxICON_EXCLAMATION, this) == wxCANCEL) {
      return PWScore::USER_CANCEL;
    }
  }

  StringX cf(m_core.GetCurFile());
  if(cf.empty()) {
    cf = wxT("pwsafe"); // reasonable default for first time user
  }
  wxString v3FileName = towxstring(PWSUtil::GetNewFileName(cf.c_str(), DEFAULT_SUFFIX));

  wxString title = (!m_core.IsDbOpen()? _("Choose a name for the current (Untitled) database:") :
                                    _("Choose a new name for the current database:"));
  wxFileName filename(v3FileName);
  wxString dir = filename.GetPath();
  if (dir.empty())
    dir = towxstring(PWSdirs::GetSafeDir());

  //filename cannot have the path
  wxFileDialog fd(this, title, dir, filename.GetFullName(),
                  _("Password Safe Databases (*.psafe3; *.dat)|*.psafe3;*.dat|All files (*.*; *)|*.*;*"),
                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (fd.ShowModal() != wxID_OK) {
    return PWScore::USER_CANCEL;
  }

  StringX newfile = tostringx(fd.GetPath());

  std::wstring locker(L""); // null init is important here
  // Note: We have to lock the new file before releasing the old (on success)
  if (!m_core.LockFile2(newfile.c_str(), locker)) {
    wxMessageBox(wxString::Format(_("%ls\n\nFile is currently locked by %ls"), newfile.c_str(), locker.c_str()),
                    _("File lock error"), wxOK | wxICON_ERROR, this);
    return PWScore::CANT_OPEN_FILE;
  }

  // Save file UUID, clear it to generate new one, restore if necessary
  pws_os::CUUID file_uuid = m_core.GetFileUUID();
  m_core.ClearFileUUID();

  UUIDList RUElist;
  m_RUEList.GetRUEList(RUElist);
  m_core.SetRUEList(RUElist);

 // Note: Writing out in in V4 DB format if the DB is already V4,
  // otherwise as V3 (this include saving pre-3.0 DBs as a V3 DB!
  int rc = m_core.WriteFile(newfile, m_core.GetReadFileVersion() == PWSfile::V40 ? PWSfile::V40 : PWSfile::V30);

  if (rc != PWScore::SUCCESS) {
    m_core.SetFileUUID(file_uuid);
    m_core.UnlockFile2(newfile.c_str());
    DisplayFileWriteError(rc, newfile);
    return PWScore::CANT_OPEN_FILE;
  }
  m_core.SafeUnlockCurFile();

  // Move the newfile lock to the right place
  m_core.MoveLock();

  m_core.SetCurFile(newfile);
#if 0
  m_titlebar = PWSUtil::NormalizeTTT(wxT("Password Safe - ") +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCWSTR(m_titlebar));
  app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
  SetTitle(towxstring(m_core.GetCurFile()));
  UpdateStatusBar();
#if 0
  ChangeOkUpdate();

  // Added/Modified entries now saved - reverse it & refresh display
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  if (m_bFilterActive && m_bFilterForStatus) {
    m_ctlItemList.Invalidate();
    m_ctlItemTree.Invalidate();
  }
#endif
  RefreshViews();

  wxGetApp().recentDatabases().AddFileToHistory(towxstring(newfile));

  if (m_core.IsReadOnly()) {
    // reset read-only status (new file can't be read-only!)
    // and so cause toolbar to be the correct version
    m_core.SetReadOnly(false);
  }

  // In case it was an unsaved restored DB
  m_bRestoredDBUnsaved = false;

  return PWScore::SUCCESS;
}

int PasswordSafeFrame::SaveIfChanged()
{
  // Deal with unsaved but changed restored DB
  if (m_bRestoredDBUnsaved && m_core.HasDBChanged()) {
    wxMessageDialog dlg(this,
                        _("Do you wish to save this restored database as new database?"),
                        _("Unsaved restored database"),
                        (wxICON_QUESTION | wxCANCEL |
                         wxYES_NO | wxYES_DEFAULT));
    int rc = dlg.ShowModal();
    switch (rc) {
      case wxID_CANCEL:
        return PWScore::USER_CANCEL;
      case wxID_YES:
        rc = SaveAs();
        // Make sure that file was successfully written
        if (rc != PWScore::SUCCESS)
          return PWScore::CANT_OPEN_FILE;
        else {
          m_bRestoredDBUnsaved = false;
          return rc;
        }
      case wxID_NO:
        return PWScore::USER_DECLINED_SAVE;
      default:
        ASSERT(0);
    }
  }

  if (m_core.IsReadOnly())
    return PWScore::SUCCESS;

  // Offer to save existing database if it was modified.
  //
  // Note: RUE list saved here via time stamp being updated.
  // Otherwise it won't be saved unless something else has changed
  if ((m_bTSUpdated || m_core.HasDBChanged()) &&
      m_core.GetNumEntries() > 0) {
    wxString prompt(_("Do you want to save changes to the password database:"));
    if (m_core.IsDbOpen()) {
      prompt += wxT("\n");
      prompt += m_core.GetCurFile().c_str();
    }
    wxMessageDialog dlg(this, prompt, "Save changes?",
                        (wxICON_QUESTION | wxCANCEL | wxYES_NO));
    dlg.SetYesNoLabels(_("Save"), _("Discard"));
    int rc = dlg.ShowModal();
    switch (rc) {
      case wxID_CANCEL:
        return PWScore::USER_CANCEL;
      case wxID_YES:
        rc = Save();
        // Make sure that file was successfully written
        if (rc != PWScore::SUCCESS)
          return PWScore::CANT_OPEN_FILE;
        UpdateStatusBar();
        break;
      case wxID_NO:
        UpdateStatusBar();
        break;
      default:
        ASSERT(0);
    }
  }
  return PWScore::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ~ Import/Export
///////////////////////////////////////////////////////////////////////////////////////////////////

struct ExportFullText
{
  static wxString EngGetTitle() {return CReport::ReportNames.find(IDSC_RPTEXPORTTEXT)->second;}
  static wxString GetTitle() {return _("Export Text");}
  static void MakeOrderedItemList(PasswordSafeFrame* frame, OrderedItemList& olist) {
    frame->FlattenTree(olist);
  }
  static wxString GetFailureMsgTitle() {return _("Export Text failed"); }
  static stringT  FileExtension() { return wxT("txt"); }
  static wxString FileOpenPrompt() { return _("Name the plaintext file"); }
  static wxString WildCards() {return _("Text files (*.txt)|*.txt|CSV files (*.csv)|*.csv|All files (*.*; *)|*.*;*"); }
  static int Write(PWScore& core, const StringX &filename, const CItemData::FieldBits &bsFields,
                          const stringT &subgroup_name, int subgroup_object,
                          int subgroup_function, TCHAR delimiter, int &numExported,
                          const OrderedItemList *il, CReport *rpt)
  {
    return core.WritePlaintextFile(filename, bsFields, subgroup_name, subgroup_object, subgroup_function,
                          delimiter, numExported, il, rpt);
  }
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced Text Export Options");
  }

  static bool IsMandatoryField(CItemData::FieldType WXUNUSED(field)) {
    return false;
  }

  static bool IsPreselectedField(CItemData::FieldType WXUNUSED(field)) {
    return true;
  }

  static bool IsUsableField(CItemData::FieldType WXUNUSED(field)) {
    return true;
  }

  static bool ShowFieldSelection() {
    return true;
  }

  static wxString GetTaskWord() {
    return _("export");
  }
};

struct ExportFullXml {
  static wxString EngGetTitle() {return CReport::ReportNames.find(IDSC_RPTEXPORTXML)->second;}
  static wxString GetTitle() {return _("Export XML");}
  static void MakeOrderedItemList(PasswordSafeFrame* frame, OrderedItemList& olist) {
    frame->FlattenTree(olist);
  }
  static wxString GetFailureMsgTitle() {return _("Export XML failed"); }
  static stringT  FileExtension() { return wxT("xml"); }
  static wxString FileOpenPrompt() { return _("Name the XML file"); }
  static wxString WildCards() {return _("XML files (*.xml)|*.xml|All files (*.*; *)|*.*;*"); }
  static int Write(PWScore& core, const StringX &filename, const CItemData::FieldBits &bsFields,
                          const stringT &subgroup_name, int subgroup_object,
                          int subgroup_function, TCHAR delimiter, int &numExported,
                          const OrderedItemList *il, CReport *rpt)
  {
    bool bFilterActive = false;
    return core.WriteXMLFile(filename, bsFields, subgroup_name, subgroup_object, subgroup_function,
                          delimiter, wxT(""), numExported, il, bFilterActive, rpt);
  }
  static wxString GetAdvancedSelectionTitle() {
    return _("Advanced XML Export Options");
  }

  static bool IsMandatoryField(CItemData::FieldType field) {
    return field == CItemData::TITLE || field == CItemData::PASSWORD;
  }

  static bool IsPreselectedField(CItemData::FieldType WXUNUSED(field)) {
    return true;
  }

  static bool IsUsableField(CItemData::FieldType WXUNUSED(field)) {
    return true;
  }

  static bool ShowFieldSelection() {
    return true;
  }
  static wxString GetTaskWord() {
    return _("export");
  }
};

void PasswordSafeFrame::OnExportVx(wxCommandEvent& evt)
{
  int rc = PWScore::FAILURE;
  StringX newfile;
  wxString cs_fmt;
  PWSfile::VERSION ver = PWSfile::UNKNOWN_VERSION;
  stringT sfx = wxEmptyString;

  switch (evt.GetId()) {
    case ID_EXPORT2OLD1XFORMAT:
      ver =  PWSfile::V17; sfx = L"dat";
      cs_fmt = _("Password Safe Databases (*.dat)|*.dat|All files (*.*; *)|*.*;*");
      break;
    case ID_EXPORT2V2FORMAT:
      ver =  PWSfile::V20; sfx = L"dat";
      cs_fmt = _("Password Safe Databases (*.dat)|*.dat|All files (*.*; *)|*.*;*");
      break;
    case ID_EXPORT2V4FORMAT:
      ver =  PWSfile::V40; sfx = L"psafe4";
      cs_fmt =_("Password Safe Databases (*.psafe4)|*.psafe4|All files (*.*; *)|*.*;*");
      break;
    default:
      ver = PWSfile::UNKNOWN_VERSION; // internal error
      break;
  }

  if (ver != PWSfile::UNKNOWN_VERSION) {
    //SaveAs-type dialog box
    std::wstring OldFormatFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                             sfx);
    const wxString cs_text = _("Name the exported database");

    //filename cannot have the path. Need to pass it separately
    wxFileName filename(towxstring(OldFormatFileName));
    wxString dir = filename.GetPath();
    if (dir.empty())
      dir = towxstring(PWSdirs::GetSafeDir());

    wxFileDialog fd(this, cs_text, dir, filename.GetFullName(), cs_fmt,
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (fd.ShowModal() != wxID_OK)
      return;

    newfile = tostringx(fd.GetPath());
    rc = m_core.WriteFile(newfile, ver);
  } else { // internal error
    wxFAIL_MSG(_("Could not figure out why PasswordSafeFrame::OnExportVx was invoked"));
  }

  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, newfile);
  }
}

void PasswordSafeFrame::OnExportPlainText(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  CallAfter(&PasswordSafeFrame::DoExportText<ExportFullText>);
}

void PasswordSafeFrame::OnExportXml(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  CallAfter(&PasswordSafeFrame::DoExportText<ExportFullXml>);
}

IMPLEMENT_CLASS_TEMPLATE( AdvancedSelectionDlg, wxDialog, ExportFullXml )
IMPLEMENT_CLASS_TEMPLATE( AdvancedSelectionDlg, wxDialog, ExportFullText )

template <class ExportType>
void PasswordSafeFrame::DoExportText()
{
  const wxString title(ExportType::GetTitle());

  const StringX sx_temp(m_core.GetCurFile());

  //MFC code doesn't do this for XML export, but it probably should, because it tries
  //to use core.GetcurFile() later
  if (sx_temp.empty()) {
    //  Database has not been saved - prompt user to do so first!
    wxMessageBox(_("You must save this database before it can be exported."), title, wxOK|wxICON_EXCLAMATION, this);
    return;
  }

  DestroyWrapper<ExportTextWarningDlg<ExportType>> etWrapper(this);
  auto *et = etWrapper.Get(); 
  if (et->ShowModal() != wxID_OK) {
    return;
  }

  StringX newfile;
  StringX pw(et->passKey);
  if (m_core.CheckPasskey(sx_temp, pw) == PWScore::SUCCESS) {
    const CItemData::FieldBits bsExport = et->selCriteria->GetSelectedFields();
    const std::wstring subgroup_name = tostdstring(et->selCriteria->SubgroupSearchText());
    const int subgroup_object = et->selCriteria->SubgroupObject();
    const int subgroup_function = et->selCriteria->SubgroupFunctionWithCase();
    wchar_t delimiter = et->delimiter.IsEmpty()? wxT('\xbb') : et->delimiter[0];

    // Note: MakeOrderedItemList gets its members by walking the
    // tree therefore, if a filter is active, it will ONLY export
    // those being displayed.
    OrderedItemList orderedItemList;
    ExportType::MakeOrderedItemList(this, orderedItemList);

    /*
     * First parameter indicates whether or not the user has specified
     * 'Advanced' to filter the entries to be exported.
     * Effectively, subgroup_* parameters are ignored if 1st param is false.
     */
    int numExported(0);
    switch(m_core.TestSelection(false, subgroup_name, subgroup_object,
                                subgroup_function, &orderedItemList)) {
      case PWScore::SUCCESS:
      {
        // do the export
        // SaveAs-type dialog box
        wxFileName TxtFileName(towxstring(PWSUtil::GetNewFileName(sx_temp.c_str(), ExportType::FileExtension())));

        wxFileDialog fd(this, ExportType::FileOpenPrompt(), TxtFileName.GetPath(),
                        TxtFileName.GetFullName(), ExportType::WildCards(),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (fd.ShowModal() == wxID_OK) {
          newfile = fd.GetPath().c_str();
          CReport rpt;

          rpt.StartReport(IDSC_RPTEXPORTTEXT, sx_temp.c_str());
          rpt.WriteLine(tostdstring(wxString(_("Exporting database: ")) << towxstring(sx_temp) << _(" to ") << newfile<< wxT("\r\n")));

          int rc = ExportType::Write(m_core, newfile, bsExport, subgroup_name, subgroup_object,
                                      subgroup_function, delimiter, numExported, &orderedItemList, &rpt);

          rpt.EndReport();

          orderedItemList.clear(); // cleanup soonest

          if (rc != PWScore::SUCCESS) {
            DisplayFileWriteError(rc, newfile);
          }
          else {
            if ( wxMessageBox(_("Export complete.  Do you wish to see a detailed report?"), ExportType::GetTitle(),
                              wxYES_NO|wxICON_QUESTION, this) == wxYES )
              ViewReport(rpt);
          }

        }
        break;
      }

      case PWScore::NO_ENTRIES_EXPORTED:
      {
        wxMessageBox(_("No entries satisfied your selection criteria and so none were exported!"),
                      ExportType::GetFailureMsgTitle(), wxOK | wxICON_WARNING, this);
        break;
      }

      default:
        break;

    } //switch

    orderedItemList.clear(); // cleanup soonest

  } else {
    wxMessageBox(_("Passkey incorrect"), title, wxOK|wxICON_ERROR, this);
    pws_os::sleep_ms(3000); // against automatic attacks
  }
}

void PasswordSafeFrame::OnImportText(wxCommandEvent& evt)
{
  CallAfter(&PasswordSafeFrame::DoImportText, evt.GetString());
}

void PasswordSafeFrame::DoImportText(wxString filename)
{
  if (m_core.IsReadOnly()) {// disable in read-only mode
    wxMessageBox(_("The current database was opened in read-only mode.  You cannot import into it."),
                  _("Import text"), wxOK | wxICON_EXCLAMATION, this);
    return;
  }

  // Initialize set
  GTUSet setGTU;
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    wxMessageBox(wxString() << _("The database:") << wxT("\n\n") << m_core.GetCurFile() << wxT("\n\n")
                            << _("has duplicate entries with the same group/title/user combination.")
                            << _("  Fix by validating database."),
                            _("Import Text failed"), wxOK | wxICON_ERROR, this);
    return;
  }

  DestroyWrapper<ImportTextDlg> dlgWrapper(this, filename);
  ImportTextDlg *dlg = dlgWrapper.Get();
  if (dlg->ShowModal() != wxID_OK)
    return;

  StringX ImportedPrefix(dlg->groupName.c_str());
  TCHAR fieldSeparator = dlg->FieldSeparator();

  std::wstring strError;
  wxString TxtFileName = dlg->filepath;
  int numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numNoPolicyNames(0);
  wchar_t delimiter = dlg->strDelimiterLine[0];
  bool bImportPSWDsOnly = dlg->importPasswordsOnly;

  /* Create report as we go */
  CReport rpt;
  rpt.StartReport(IDSC_RPTIMPORTTEXT, m_core.GetCurFile().c_str());
  wxString header;
  header.Printf(_("%ls file being imported: %ls"), _("Text"), TxtFileName.c_str());
  rpt.WriteLine(tostdstring(header));
  rpt.WriteLine();

  Command *pcmd = nullptr;
  int rc = m_core.ImportPlaintextFile(ImportedPrefix, tostringx(TxtFileName), fieldSeparator,
                                  delimiter, bImportPSWDsOnly,
                                  strError,
                                  numImported, numSkipped, numPWHErrors, numRenamed, numNoPolicyNames,
                                  rpt, pcmd);

  wxString cs_title, cs_temp;

  switch (rc) {
    case PWScore::CANT_OPEN_FILE:
      cs_title = _("File Read Error");
      cs_temp << TxtFileName << wxT("\n\n") << _("Could not open file for reading!");
      delete pcmd;
      break;
    case PWScore::INVALID_FORMAT:
      cs_title = _("File Read Error");
      cs_temp << TxtFileName << wxT("\n\n") << _("Invalid format");
      delete pcmd;
      break;
    case PWScore::FAILURE:
      cs_title = _("Import Text failed");
      cs_temp = towxstring(strError);
      delete pcmd;
      break;
    case PWScore::SUCCESS:
    case PWScore::OK_WITH_ERRORS:
      // deliberate fallthrough
    default:
    {
      if (pcmd != nullptr)
        Execute(pcmd);

      rpt.WriteLine();
      cs_temp << (bImportPSWDsOnly ? _("Updated ") : _("Imported "))
              << numImported << (numImported == 1? _(" entry") : _(" entries"));
      rpt.WriteLine(tostdstring(cs_temp));

      if (numSkipped != 0) {
        wxString cs_tmp;
        cs_tmp << wxT("\n") << _("Skipped ") << numSkipped << (numSkipped == 1? _(" entry") : _(" entries"));
        rpt.WriteLine(tostdstring(cs_tmp));
        cs_temp += cs_tmp;
      }

      if (numPWHErrors != 0) {
        wxString cs_tmp;
        cs_tmp << wxT("\n") << _("with Password History errors ") << numPWHErrors;
        rpt.WriteLine(tostdstring(cs_tmp));
        cs_temp += cs_tmp;
      }

      if (numRenamed != 0) {
        wxString cs_tmp;
        cs_tmp << wxT("\n") << _("Renamed ") << numRenamed << (numRenamed == 1? _(" entry") : _(" entries"));
        rpt.WriteLine(tostdstring(cs_tmp));
        cs_temp += cs_tmp;
      }

      cs_title = (rc == PWScore::SUCCESS ? _("Completed successfully") : _("Completed but ...."));

      RefreshViews();

      break;
    }
  } // switch

  // Finish Report
  rpt.EndReport();

  const int iconType = (rc == PWScore::SUCCESS ? wxICON_INFORMATION : wxICON_EXCLAMATION);
  cs_temp << wxT("\n\n") << _("Do you wish to see a detailed report?");
  if (wxMessageBox(cs_temp, cs_title, wxYES_NO | iconType, this) == wxYES) {
    ViewReport(rpt);
  }
}

void PasswordSafeFrame::OnImportXML(wxCommandEvent& evt)
{
  CallAfter(&PasswordSafeFrame::DoImportXML, evt.GetString());
}

void PasswordSafeFrame::DoImportXML(wxString filename)
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  // Initialize set
  GTUSet setGTU;
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    wxMessageBox(wxString::Format( _("The database:\n\n%ls\n\nhas duplicate entries with the same group/title/user combination. Fix by validating database."),
                                    m_core.GetCurFile().c_str()), _("Import XML failed"), wxOK | wxICON_ERROR, this);
    return;
  }

  TCHAR XSDfn[] = wxT("pwsafe.xsd");
  wxFileName XSDFilename(towxstring(PWSdirs::GetXMLDir()), XSDfn);

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  if (!XSDFilename.FileExists()) {
    wxString filepath(XSDFilename.GetFullPath());
    wxMessageBox(wxString::Format(_("Can't find XML Schema Definition file (%ls) in your PasswordSafe Application Directory.\nCopy it from your installation file, or re-install PasswordSafe."), filepath.c_str()),
                          wxString(_("Missing XSD File - ")) + wxSTRINGIZE_T(USE_XML_LIBRARY) + _(" Build"), wxOK | wxICON_ERROR, this);
    return;
  }
#endif

  DestroyWrapper<ImportXmlDlg> dlgWrapper(this, filename);
  ImportXmlDlg *dlg = dlgWrapper.Get();
  if (dlg->ShowModal() != wxID_OK) {
    return;
  }

  std::wstring ImportedPrefix(tostdstring(dlg->groupName));
  std::wstring strXMLErrors, strSkippedList, strPWHErrorList, strRenameList;
  wxString XMLFilename = dlg->filepath;
  int numValidated, numImported, numSkipped, numRenamed, numPWHErrors;
  int numRenamedPolicies, numNoPolicy;
  int numShortcutsRemoved, numEmptyGroupsImported;
  bool bImportPSWDsOnly = dlg->importPasswordsOnly;

  wxBeginBusyCursor();  // This may take a while!

  /* Create report as we go */
  CReport rpt;
  rpt.StartReport(IDSC_RPTIMPORTXML, m_core.GetCurFile().c_str());
  rpt.WriteLine(tostdstring(wxString::Format(_("%ls file being imported: %ls"), _("XML"), XMLFilename.c_str())));
  rpt.WriteLine();
  std::vector<StringX> vgroups;
  Command *pcmd = nullptr;

  int rc = m_core.ImportXMLFile(ImportedPrefix, tostdstring(XMLFilename),
                            tostdstring(XSDFilename.GetFullPath()), bImportPSWDsOnly,
                            strXMLErrors, strSkippedList, strPWHErrorList, strRenameList,
                            numValidated, numImported, numSkipped, numPWHErrors, numRenamed,
                            numNoPolicy, numRenamedPolicies, numShortcutsRemoved,
                            numEmptyGroupsImported,
                            rpt, pcmd);
  wxEndBusyCursor();  // Restore normal cursor

  wxString cs_temp;
  wxString cs_title(_("Import XML failed"));

  std::wstring csErrors(wxEmptyString);
  switch (rc) {
    case PWScore::XML_FAILED_VALIDATION:
      cs_temp = wxString::Format(_("File: %ls failed validation against XML Schema:\n\n%ls"),
                                        dlg->filepath.c_str(), strXMLErrors.c_str());
      delete pcmd;
      break;
    case PWScore::XML_FAILED_IMPORT:
      cs_temp = wxString::Format(_("File: %ls passed Validation but had the following errors during import:\n\n%ls"),
                              dlg->filepath.c_str(), strXMLErrors.c_str());
      delete pcmd;
      break;
    case PWScore::SUCCESS:
    case PWScore::OK_WITH_ERRORS:
      cs_title = rc == PWScore::SUCCESS ? _("Completed successfully") :  _("Completed but ....");
      if (pcmd != nullptr)
        Execute(pcmd);

      if (!strXMLErrors.empty() ||
          numRenamed > 0 || numPWHErrors > 0) {
        if (!strXMLErrors.empty())
          csErrors = strXMLErrors + wxT("\n");

        if (!csErrors.empty()) {
          rpt.WriteLine(csErrors.c_str());
        }

        wxString cs_renamed, cs_PWHErrors, cs_skipped;
        if (numSkipped > 0) {
          cs_skipped = _("The following records were skipped:");
          rpt.WriteLine(tostdstring(cs_skipped));
          cs_skipped.Printf(_(" / skipped %d"), numSkipped);
          rpt.WriteLine(strSkippedList.c_str());
          rpt.WriteLine();
        }
        if (numPWHErrors > 0) {
          cs_PWHErrors = _("The following records had errors in their Password History:");
          rpt.WriteLine(tostdstring(cs_PWHErrors));
          cs_PWHErrors.Printf(_(" / with Password History errors %d"), numPWHErrors);
          rpt.WriteLine(strPWHErrorList.c_str());
          rpt.WriteLine();
        }
        if (numRenamed > 0) {
          cs_renamed = _("The following records were renamed as an entry already exists in your database or in the Import file:");
          rpt.WriteLine(tostdstring(cs_renamed));
          cs_renamed.Printf(_(" / renamed %d"), numRenamed);
          rpt.WriteLine(strRenameList.c_str());
          rpt.WriteLine();
        }

        cs_temp.Printf(_("File: %ls was imported (entries validated %d / imported %d%ls%ls%ls). See report for details."),
                       dlg->filepath.c_str(), numValidated, numImported,
                       cs_skipped.c_str(), cs_renamed.c_str(), cs_PWHErrors.c_str());
        // TODO -Tell user if any empty groups imported
      } else {
        const TCHAR* cs_validate = numValidated == 1 ? _("entry").c_str() : _("entries").c_str();
        const TCHAR* cs_imported = numImported == 1 ? _("entry").c_str() : _("entries").c_str();
        cs_temp.Printf(_("Validated %d %ls\n\nImported %d %ls"), numValidated, cs_validate, numImported, cs_imported);

        // TODO -Tell user if any empty groups imported
      }

      RefreshViews();
      break;
    case PWScore::UNIMPLEMENTED:
      cs_temp = _("XML import not supported in this release");
      break;
    default:
      cs_temp.Printf(_("XML import: Unexpected return code(%d)"), rc);
      break;
  } // switch

  // Finish Report
  rpt.WriteLine(tostdstring(cs_temp));
  rpt.EndReport();

  const int iconType = (rc != PWScore::SUCCESS || !strXMLErrors.empty()) ? wxICON_EXCLAMATION : wxICON_INFORMATION;

  cs_temp << wxT("\n\n") << _("Do you wish to see a detailed report?");
  if ( wxMessageBox(cs_temp, cs_title, wxYES_NO | iconType, this) == wxYES) {
    ViewReport(rpt);
  }
}

void PasswordSafeFrame::OnImportKeePass(wxCommandEvent& evt)
{
  CallAfter(&PasswordSafeFrame::DoImportKeePass, evt.GetString());
}

void PasswordSafeFrame::DoImportKeePass(wxString filename)
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  wxString KPsFileName;
  
  if(filename.IsEmpty()) {
    wxFileDialog fd(this, _("Import a KeePass Text File"),
                  wxEmptyString, filename,
                  _("Text files (*.txt)|*.txt|CSV files (*.csv)|*.csv|All files (*.*; *)|*.*;*"),
                  (wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_PREVIEW));

    if (fd.ShowModal() != wxID_OK )
      return;
    
    KPsFileName = fd.GetPath();
  }
  else {
    KPsFileName = filename;
  }
  CReport rpt;

  enum { KeePassCSV, KeePassTXT } ImportType = wxFileName(KPsFileName).GetExt() == wxT("csv")? KeePassCSV: KeePassTXT;

  if (ImportType == KeePassCSV)
    rpt.StartReport(IDSC_RPTIMPORTKPV1CSV, m_core.GetCurFile().c_str());
  else
    rpt.StartReport(IDSC_RPTIMPORTKPV1TXT, m_core.GetCurFile().c_str());

  rpt.WriteLine(wxString::Format(_("Text file being imported: %ls"), KPsFileName.c_str()));
  rpt.WriteLine();

  int numImported, numSkipped, numRenamed;
  unsigned int uiReasonCode = 0;
  int rc;
  Command *pcmd = nullptr;

  if (ImportType == KeePassCSV)
    rc = m_core.ImportKeePassV1CSVFile(tostringx(KPsFileName), numImported, numSkipped, numRenamed,
                                       uiReasonCode, rpt, pcmd);
  else
    rc = m_core.ImportKeePassV1TXTFile(tostringx(KPsFileName), numImported, numSkipped, numRenamed,
                                       uiReasonCode, rpt, pcmd);
  switch (rc) {
    case PWScore::CANT_OPEN_FILE:
    {
      wxMessageBox( wxString::Format(_("%ls\n\nCould not open file for reading!"), KPsFileName.GetData()),
                    _("File open error"), wxOK | wxICON_ERROR, this);
      delete [] pcmd;
      break;
    }
    case PWScore::INVALID_FORMAT:
    case PWScore::FAILURE:
    {
      wxString msg;
      if (uiReasonCode > 0) {
        stringT s;
        LoadAString(s, uiReasonCode);
        msg = towxstring(s);
      }
      else {
        msg = wxString::Format(_("%ls\n\nInvalid format"), KPsFileName.GetData());
      }
      msg << wxT("\n\n") << _("Do you wish to see a detailed report?");
      if (wxMessageBox(msg, _("Import failed"), wxYES_NO | wxICON_ERROR, this) == wxYES)
        ViewReport(rpt);
      delete [] pcmd;
      break;
    }
    case PWScore::SUCCESS:
    default: // deliberate fallthrough
      if (pcmd != nullptr)
        Execute(pcmd);
      RefreshViews();
#ifdef NOT_YET
      ChangeOkUpdate();
#endif
      rpt.WriteLine();
      wxString cs_type(numImported == 1 ? _("entry") : _("entries"));
      wxString cs_msg = wxString::Format(_("Imported %d %ls"), numImported, cs_type.GetData());
      rpt.WriteLine(static_cast<const TCHAR*>(cs_msg.c_str()));
      rpt.EndReport();
      wxString title(rc == PWScore::SUCCESS ? _("Completed successfully") : _("Completed but ...."));
      int icon = (rc == PWScore::SUCCESS ? wxICON_INFORMATION : wxICON_EXCLAMATION);
      cs_msg << wxT("\n\n") << _("Do you wish to see a detailed report?");
      if (wxMessageBox(cs_msg, title, icon|wxYES_NO, this) == wxYES)
        ViewReport(rpt);
      break;
  } // switch
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ~ Merge, Synchronize and Compare
///////////////////////////////////////////////////////////////////////////////////////////////////

void PasswordSafeFrame::OnMergeAnotherSafe(wxCommandEvent& evt)
{
  CallAfter(&PasswordSafeFrame::DoMergeAnotherSafe, evt.GetString());
}

void PasswordSafeFrame::DoMergeAnotherSafe(wxString filename)
{
  DestroyWrapper<MergeDlg> dlgWrapper(this, &m_core, filename);
  MergeDlg* dlg = dlgWrapper.Get();

  if (dlg->ShowModal() == wxID_OK) {
    PWScore othercore; // NOT PWSAuxCore, as we handle db prefs explicitly
    // Reading a new file changes the preferences as they are instance dependent
    // not core dependent
    PWSprefs *prefs =  PWSprefs::GetInstance();

    const StringX sxSavePrefString(prefs->Store());

    // Save all the 'other core' preferences in the copy - to use for
    // 'other' default Password Policy when needed in Compare, Merge & Sync
    prefs->SetupCopyPrefs();

    int rc = ReadCore(othercore, dlg->GetOtherSafePath(),
                      dlg->GetOtherSafeCombination(), true, this);

    // Reset database preferences - first to defaults then add saved changes!
    prefs->Load(sxSavePrefString);

    if (rc == PWScore::SUCCESS) {
        Merge(tostringx(dlg->GetOtherSafePath()), &othercore, dlg->GetSelectionCriteria());
    }
  }
}

void PasswordSafeFrame::Merge(const StringX &sx_Filename2, PWScore *pothercore, const SelectionCriteria& selection)
{
  /* Put up hourglass...this might take a while */
  ::wxBeginBusyCursor();

  /* Create report as we go */
  CReport rpt;

  rpt.StartReport(IDSC_RPTMERGE, m_core.GetCurFile().c_str());
  rpt.WriteLine(tostdstring(wxString(_("Merging database: ")) << towxstring(sx_Filename2) << wxT("\r\n")));

  stringT result = m_core.Merge(pothercore,
                                selection.HasSubgroupRestriction(),
                                tostdstring(selection.SubgroupSearchText()),
                                selection.SubgroupObject(),
                                selection.SubgroupFunction(),
                                &rpt);

  ::wxEndBusyCursor();

  rpt.EndReport();

  if (!result.empty() && wxMessageBox(towxstring(result) + wxT("\n\n") +
                                      _("Do you wish to see a detailed report?"),
                                      _("Merge Complete"), wxYES_NO|wxICON_QUESTION, this) == wxYES) {
    ViewReport(rpt);
  }
}

void PasswordSafeFrame::OnCompare(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::DoCompare);
}

void PasswordSafeFrame::DoCompare()
{
  ShowModalAndGetResult<CompareDlg>(this, &m_core);
}

void PasswordSafeFrame::OnSynchronize(wxCommandEvent& evt)
{
  CallAfter(&PasswordSafeFrame::DoSynchronize, evt.GetString());
}

void PasswordSafeFrame::DoSynchronize(wxString filename)
{
  // disable in read-only mode or empty
  wxCHECK_RET(!m_core.IsReadOnly() && m_core.IsDbOpen() && m_core.GetNumEntries() != 0,
                wxT("Synchronize menu enabled for empty or read-only database!"));

  SyncWizard wiz(this, &m_core, filename);
  if (wiz.RunWizard(wiz.GetFirstPage())) {
    if (wiz.GetNumUpdated() > 0 && wiz.GetSyncCommands()) {
      m_core.Execute(wiz.GetSyncCommands());
      UpdateStatusBar();
    }
#ifdef NOT_YET
    ChangeOkUpdate();
#endif

    RefreshViews();

    if (wiz.ShowReport()) {
      ViewReport(*wiz.GetReport());
    }
  }
}

void PasswordSafeFrame::OnPropertiesClick(wxCommandEvent& WXUNUSED(evt))
{
  CallAfter(&PasswordSafeFrame::DoPropertiesClick);
}

void PasswordSafeFrame::DoPropertiesClick()
{
  DestroyWrapper<PropertiesDlg> dlgWrapper(this, m_core);
  PropertiesDlg* propsDialog = dlgWrapper.Get();
  propsDialog->ShowModal();

  if (propsDialog->HasDbNameChanged() || propsDialog->HasDbDescriptionChanged()) {

    auto multiCommands = MultiCommands::Create(&m_core);

    if (propsDialog->HasDbNameChanged()) {

      multiCommands->Add(
        ChangeDBHeaderCommand::Create(
          &m_core, propsDialog->GetNewDbName(), PWSfile::HDR_DBNAME
        )
      );
    }

    if (propsDialog->HasDbDescriptionChanged()) {

      multiCommands->Add(
        ChangeDBHeaderCommand::Create(
          &m_core, propsDialog->GetNewDbDescription(), PWSfile::HDR_DBDESC
        )
      );
    }

    if (!multiCommands->IsEmpty()) {
      Execute(multiCommands);
    }
    else {
      delete multiCommands;
    }
  }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
 */

void PasswordSafeFrame::OnExitClick(wxCommandEvent& WXUNUSED(evt))
{
  CloseAllWindows(&TimedTaskChain::CreateTaskChain([](){}), CloseFlags::CLOSE_NORMAL, [this](bool success) {
    if (!success) {
      // `this` should be valid here, because we haven't closed DB
      wxMessageBox(_("Can't close database. There are unsaved changes in opened dialogs."), wxTheApp->GetAppName(), wxOK | wxICON_WARNING, this);
    }
  });
}
