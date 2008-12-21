/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// file MainFile.cpp
//
// File-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

// dialog boxen
#include "DboxMain.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#include "PasskeySetup.h"
#include "TryAgainDlg.h"
#include "ExportTextDlg.h"
#include "ExportXMLDlg.h"
#include "ImportDlg.h"
#include "ImportXMLDlg.h"
#include "AdvancedDlg.h"
#include "CompareResultsDlg.h"
#include "Properties.h"
#include "GeneralMsgBox.h"
#include "MFCMessages.h"
#include "corelib/pwsprefs.h"
#include "corelib/util.h"
#include "corelib/PWSdirs.h"
#include "corelib/Report.h"
#include "corelib/ItemData.h"
#include "corelib/corelib.h"
#include "corelib/os/file.h"

#include <sys/types.h>
#include <bitset>
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL DboxMain::OpenOnInit(void)
{
  /*
    Routine to account for the differences between opening PSafe for
    the first time, and just opening a different database or
    un-minimizing the application
  */
  StringX passkey;
  bool bReadOnly = m_core.IsReadOnly();  // Can only be from -r command line parameter
  if (!bReadOnly) {
    // Command line not set - use config for first open
    bReadOnly = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO) == TRUE;
  }
  int rc = GetAndCheckPassword(m_core.GetCurFile(),
                               passkey, GCP_FIRST,
                               bReadOnly,
                               m_core.IsReadOnly());  // First
  int rc2 = PWScore::NOT_SUCCESS;

  switch (rc) {
    case PWScore::SUCCESS:
    {
      MFCAsker q;
      MFCReporter r;
      m_core.SetAsker(&q);
      m_core.SetReporter(&r);
      rc2 = m_core.ReadCurFile(passkey);
      m_core.SetAsker(NULL);
      m_core.SetReporter(NULL);
#if !defined(POCKET_PC)
      m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") +
                                         m_core.GetCurFile()).c_str();
      UpdateSystemTray(UNLOCKED);
#endif
      CheckExpiredPasswords();
    }
      break;
    case PWScore::CANT_OPEN_FILE:
      if (m_core.GetCurFile().empty()) {
        // Empty filename. Assume they are starting Password Safe
        // for the first time and don't confuse them.
        // fallthrough to New()
      } else {
        // Here if there was a filename saved from last invocation, but it couldn't
        // be opened. It was either removed or renamed, so ask the user what to do
        CString cs_msg;
        cs_msg.Format(IDS_CANTOPENSAFE, m_core.GetCurFile().c_str());
        CGeneralMsgBox gmb;
        gmb.SetMsg(cs_msg);
        gmb.SetStandardIcon(MB_ICONQUESTION);
        gmb.AddButton(1, IDS_SEARCH);
        gmb.AddButton(2, IDS_NEW);
        gmb.AddButton(3, IDS_EXIT, TRUE, TRUE);
        INT_PTR rc3 = gmb.DoModal();
        switch (rc3) {
          case 1:
            rc2 = Open();
            break;
          case 2:
            rc2 = New();
            break;
          case 3:
            rc2 = PWScore::USER_CANCEL;
            break;
        }
        break;
      }
    case TAR_NEW:
      rc2 = New();
      if (PWScore::USER_CANCEL == rc2) {
        // somehow, get DboxPasskeyEntryFirst redisplayed...
      }
      break;
    case TAR_OPEN:
      rc2 = Open();
      if (PWScore::USER_CANCEL == rc2) {
        // somehow, get DboxPasskeyEntryFirst redisplayed...
      }
      break;
    case PWScore::WRONG_PASSWORD:
    default:
      break;
  }

  bool go_ahead = false;
  /*
   * If BAD_DIGEST or LIMIT_REACHED,
   * the we prompt the user, and continue or not per user's input.
   * A bit too subtle for switch/case on rc2...
   */
  if (rc2 == PWScore::BAD_DIGEST) {
    CString cs_msg; cs_msg.Format(IDS_FILECORRUPT, m_core.GetCurFile().c_str());
    CString cs_title(MAKEINTRESOURCE(IDS_FILEREADERROR));
    const int yn = MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONERROR);
    if (yn == IDNO) {
      CDialog::OnCancel();
      return FALSE;
    }
    go_ahead = true;
  } // BAD_DIGEST
#ifdef DEMO
  if (rc2 == PWScore::LIMIT_REACHED) {
    CString cs_msg;
    cs_msg.Format(IDS_LIMIT_MSG, MAXDEMO);
    CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE));
    if (MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONWARNING) == IDNO) {
      CDialog::OnCancel();
      return FALSE;
    }
    go_ahead = true;
  } // LIMIT_REACHED
#endif /* DEMO */

  if (rc2 != PWScore::SUCCESS && !go_ahead) {
    // not a good return status, fold.
    if (!m_IsStartSilent)
      CDialog::OnCancel();
    return FALSE;
  }

  // Status OK or user chose to forge ahead...
  m_needsreading = false;
  startLockCheckTimer();
  UpdateSystemTray(UNLOCKED);
  if (!m_bOpen) {
    // Previous state was closed - reset DCA in status bar
    SetDCAText();
  }
  app.AddToMRU(m_core.GetCurFile().c_str());
  UpdateMenuAndToolBar(true); // sets m_bOpen too...
  UpdateStatusBar();

  m_core.SetDefUsername(PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DefaultUsername));
  m_core.SetUseDefUser(PWSprefs::GetInstance()->
                       GetPref(PWSprefs::UseDefaultUser) ? true : false);
#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCTSTR(m_titlebar));
  app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
  SelectFirstEntry();
  // Validation does integrity check & repair on database
  // currently invoke it iff m_bValidate set (e.g., user passed '-v' flag)
  if (m_bValidate) {
    PostMessage(WM_COMMAND, ID_MENUITEM_VALIDATE);
    m_bValidate = false;
  }
  return TRUE;
}

void DboxMain::OnNew()
{
  New();
}

int DboxMain::New()
{
  int rc, rc2;

  if (m_core.IsChanged()) {
    CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, m_core.GetCurFile().c_str());
    rc = MessageBox(cs_temp, AfxGetAppName(),
      MB_ICONQUESTION|MB_YESNOCANCEL);
    switch (rc) {
      case IDCANCEL:
        return PWScore::USER_CANCEL;
      case IDYES:
        rc2 = Save();
        /*
        Make sure that writing the file was successful
        */
        if (rc2 == PWScore::SUCCESS)
          break;
        else
          return PWScore::CANT_OPEN_FILE;
      case IDNO:
        // Reset changed flag
        SetChanged(Clear);
        break;
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
  if (rc == PWScore::CANT_OPEN_FILE) {
    CString cs_temp, cs_title(MAKEINTRESOURCE(IDS_FILEWRITEERROR));
    cs_temp.Format(IDS_CANTOPENWRITING, cs_newfile.c_str());
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::USER_CANCEL;
  }

#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") + cs_newfile).c_str();
  SetWindowText(LPCTSTR(m_titlebar));
#endif

  ChangeOkUpdate();
  UpdateSystemTray(UNLOCKED);
  m_RUEList.ClearEntries();
  if (!m_bOpen) {
    // Previous state was closed - reset DCA in status bar
    SetDCAText();
  }

  UpdateMenuAndToolBar(true);

  return PWScore::SUCCESS;
}

int DboxMain::NewFile(StringX &newfilename)
{
  CString cs_msg, cs_title, cs_temp;
  CString cs_text(MAKEINTRESOURCE(IDS_CREATENAME));

  CString cf(MAKEINTRESOURCE(IDS_DEFDBNAME)); // reasonable default for first time user
  stringT v3FileName = PWSUtil::GetNewFileName(LPCTSTR(cf), DEFAULT_SUFFIX);
  stringT dir = PWSdirs::GetSafeDir();
  INT_PTR rc;

  while (1) {
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   v3FileName.c_str(),
                   OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                   OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                   SUFFIX3_FILTERS
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = cs_text;
    fd.m_ofn.Flags &= ~OFN_READONLY;
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
      newfilename = LPCTSTR(fd.GetPathName());
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  CPasskeySetup dbox_pksetup(this);
  //app.m_pMainWnd = &dbox_pksetup;
  rc = dbox_pksetup.DoModal();

  if (rc == IDCANCEL)
    return PWScore::USER_CANCEL;  //User cancelled password entry

  // Reset core
  m_core.ReInit(true);

  ClearData();
  PWSprefs::GetInstance()->SetDatabasePrefsToDefaults();
  const StringX &oldfilename = m_core.GetCurFile();
  // The only way we're the locker is if it's locked & we're !readonly
  if (!oldfilename.empty() &&
      !m_core.IsReadOnly() &&
      m_core.IsLockedFile(oldfilename.c_str()))
    m_core.UnlockFile(oldfilename.c_str());

  m_core.SetCurFile(newfilename);

  // Now lock the new file
  stringT locker(_T("")); // null init is important here
  m_core.LockFile(newfilename.c_str(), locker);

  m_core.SetReadOnly(false); // new file can't be read-only...
  m_core.NewFile(dbox_pksetup.m_passkey);
  m_needsreading = false;
  startLockCheckTimer();
  return PWScore::SUCCESS;
}

void DboxMain::OnClose()
{
  Close();
}

int DboxMain::Close()
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Save Application related preferences
  prefs->SaveApplicationPreferences();

  if (m_bOpen) {
    // try and save it first
    int rc = SaveIfChanged();
    if (rc != PWScore::SUCCESS)
      return rc;
  }

  // Unlock the current file
  if( !m_core.GetCurFile().empty() ) {
    m_core.UnlockFile(m_core.GetCurFile().c_str());
    m_core.SetCurFile(_T(""));
  }

  // Clear all associated data
  ClearData();

  // Reset core
  m_core.ReInit();

  // Tidy up filters
  m_currentfilter.Empty();
  m_bFilterActive = false;

  // Set Dragbar images correctly
  m_DDGroup.SetStaticState(false);
  m_DDTitle.SetStaticState(false);
  m_DDPassword.SetStaticState(false);
  m_DDUser.SetStaticState(false);
  m_DDNotes.SetStaticState(false);
  m_DDURL.SetStaticState(false);

  app.SetTooltipText(_T("PasswordSafe"));
  UpdateSystemTray(CLOSED);
  // Call UpdateMenuAndToolBar before UpdateStatusBar, as it sets m_bOpen
  UpdateMenuAndToolBar(false);
  m_titlebar = _T("Password Safe");
  SetWindowText(LPCTSTR(m_titlebar));
  m_lastclipboardaction = _T("");
  UpdateStatusBar();
  return PWScore::SUCCESS;
}

void DboxMain::OnOpen()
{
  int rc = Open();

  if (rc == PWScore::SUCCESS) {
    if (!m_bOpen) {
      // Previous state was closed - reset DCA in status bar
      SetDCAText();
    }
    UpdateMenuAndToolBar(true);
    UpdateStatusBar();
  }
}

#if _MFC_VER > 1200
BOOL DboxMain::OnOpenMRU(UINT nID)
#else
void DboxMain::OnOpenMRU(UINT nID)
#endif
{
  UINT uMRUItem = nID - ID_FILE_MRU_ENTRY1;

  CString mruItem = (*app.GetMRU())[uMRUItem];

  // Save just in case need to restore if user cancels
  const bool last_ro = m_core.IsReadOnly();
  m_core.SetReadOnly(false);
  // Read-only status can be overriden by GetAndCheckPassword
  int rc = Open(LPCTSTR(mruItem), 
                PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO) == TRUE);
  if (rc == PWScore::SUCCESS) {
    UpdateSystemTray(UNLOCKED);
    m_RUEList.ClearEntries();
    if (!m_bOpen) {
      // Previous state was closed - reset DCA in status bar
      SetDCAText();
    }
    UpdateMenuAndToolBar(true);
    UpdateStatusBar();
    SelectFirstEntry();
  } else {
    // Reset Read-only status
    m_core.SetReadOnly(last_ro);
  }

#if _MFC_VER > 1200
  return TRUE;
#endif
}

int DboxMain::Open()
{
  int rc = PWScore::SUCCESS;
  StringX newfile;
  CString cs_text(MAKEINTRESOURCE(IDS_CHOOSEDATABASE));
  stringT dir = PWSdirs::GetSafeDir();

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   DEFAULT_SUFFIX,
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_LONGNAMES,
                   SUFFIX_FILTERS
                   _T("Password Safe Backups (*.bak)|*.bak|")
                   _T("Password Safe Intermediate Backups (*.ibak)|*.ibak|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = cs_text;
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO))
      fd.m_ofn.Flags |= OFN_READONLY;
    else
      fd.m_ofn.Flags &= ~OFN_READONLY;
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
    const bool last_ro = m_core.IsReadOnly(); // restore if user cancels
    m_core.SetReadOnly(fd.GetReadOnlyPref() == TRUE);
    if (rc2 == IDOK) {
      newfile = LPCTSTR(fd.GetPathName());

      rc = Open(newfile, fd.GetReadOnlyPref() == TRUE);
      if (rc == PWScore::SUCCESS) {
        UpdateSystemTray(UNLOCKED);
        m_RUEList.ClearEntries();
        break;
      } else
      if (rc == PWScore::ALREADY_OPEN) {
        m_core.SetReadOnly(last_ro);
      }
    } else {
      m_core.SetReadOnly(last_ro);
      return PWScore::USER_CANCEL;
    }
  }
  return rc;
}

int DboxMain::Open(const StringX &pszFilename, const bool bReadOnly)
{
  int rc;
  StringX passkey;
  CString temp, cs_title, cs_text;

  //Check that this file isn't already open
  if (pszFilename == m_core.GetCurFile() && !m_needsreading) {
    //It is the same damn file
    cs_text.LoadString(IDS_ALREADYOPEN);
    cs_title.LoadString(IDS_OPENDATABASE);
    MessageBox(cs_text, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::ALREADY_OPEN;
  }

  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS)
    return rc;

  // if we were using a different file, unlock it
  // do this before GetAndCheckPassword() as that
  // routine gets a lock on the new file
  if( !m_core.GetCurFile().empty() ) {
    m_core.UnlockFile(m_core.GetCurFile().c_str());
  }

  rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL, bReadOnly);  // OK, CANCEL, HELP
  switch (rc) {
    case PWScore::SUCCESS:
      app.AddToMRU(pszFilename.c_str());
      m_bAlreadyToldUserNoSave = false;
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      temp.Format(IDS_SAFENOTEXIST, pszFilename.c_str());
      cs_title.LoadString(IDS_FILEOPENERROR);
      MessageBox(temp, cs_title, MB_OK|MB_ICONWARNING);
    case TAR_OPEN:
      return Open();
    case TAR_NEW:
      return New();
    case PWScore::WRONG_PASSWORD:
    case PWScore::USER_CANCEL:
      /*
      If the user just cancelled out of the password dialog,
      assume they want to return to where they were before...
      */
      return PWScore::USER_CANCEL;
    default:
      ASSERT(0); // we should take care of all cases explicitly
      return PWScore::USER_CANCEL; // conservative behaviour for release version
  }

  // clear the data before loading the new file
  ClearData();

  cs_title.LoadString(IDS_FILEREADERROR);
  MFCAsker q;
  MFCReporter r;
  m_core.SetAsker(&q);
  m_core.SetReporter(&r);
  rc = m_core.ReadFile(pszFilename, passkey);
  m_core.SetAsker(NULL);
  m_core.SetReporter(NULL);
  switch (rc) {
    case PWScore::SUCCESS:
      break;
    case PWScore::CANT_OPEN_FILE:
      temp.Format(IDS_CANTOPENREADING, pszFilename.c_str());
      MessageBox(temp, cs_title, MB_OK|MB_ICONWARNING);
      /*
      Everything stays as is... Worst case,
      they saved their file....
      */
      return PWScore::CANT_OPEN_FILE;
    case PWScore::BAD_DIGEST:
    {
      temp.Format(IDS_FILECORRUPT, pszFilename.c_str());
      const int yn = MessageBox(temp, cs_title, MB_YESNO|MB_ICONERROR);
      if (yn == IDYES) {
        rc = PWScore::SUCCESS;
        break;
      } else
        return rc;
    }
#ifdef DEMO
    case PWScore::LIMIT_REACHED:
    {
      CString cs_msg; cs_msg.Format(IDS_LIMIT_MSG, MAXDEMO);
      CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE));
      const int yn = MessageBox(cs_msg, cs_title, MB_YESNO|MB_ICONWARNING);
      if (yn == IDNO) {
        return PWScore::USER_CANCEL;
      }
      rc = PWScore::SUCCESS;
      m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_ADD, FALSE);
      break;
    }
#endif
    default:
      temp.Format(IDS_UNKNOWNERROR, pszFilename.c_str());
      MessageBox(temp, cs_title, MB_OK|MB_ICONERROR);
      return rc;
  }
  m_core.SetCurFile(pszFilename);
#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCTSTR(m_titlebar));
#endif
  CheckExpiredPasswords();
  ChangeOkUpdate();

  // Tidy up filters
  m_currentfilter.Empty();
  m_bFilterActive = false;

  RefreshViews();
  SetInitialDatabaseDisplay();
  m_core.SetDefUsername(PWSprefs::GetInstance()->
                        GetPref(PWSprefs::DefaultUsername));
  m_core.SetUseDefUser(PWSprefs::GetInstance()->
                       GetPref(PWSprefs::UseDefaultUser) ? true : false);
  m_needsreading = false;
  SelectFirstEntry();

  return rc;
}

void DboxMain::OnClearMRU()
{
  app.ClearMRU();
}

void DboxMain::OnSave()
{
  Save();
}

int DboxMain::Save()
{
  int rc;
  CString cs_title, cs_msg, cs_temp;
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Save Application related preferences
  prefs->SaveApplicationPreferences();

  if (m_core.GetCurFile().empty())
    return SaveAs();

  int iver = (int)m_core.GetReadFileVersion();
  if (iver == PWSfile::VCURRENT) {
    if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
      int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
      int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
      stringT userBackupPrefix = prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
      stringT userBackupDir = prefs->GetPref(PWSprefs::BackupDir).c_str();
      if (!m_core.BackupCurFile(maxNumIncBackups, backupSuffix,
        userBackupPrefix, userBackupDir))
        AfxMessageBox(IDS_NOIBACKUP, MB_OK);
    }
  } 
  else if (iver != PWSfile::NEWFILE) {
    // file version mis-match
    stringT NewName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                              DEFAULT_SUFFIX);

    cs_msg.Format(IDS_NEWFORMAT,
                  m_core.GetCurFile().c_str(), NewName.c_str());
    cs_title.LoadString(IDS_VERSIONWARNING);

    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONWARNING);
    gmb.AddButton(1, IDS_CONTINUE);
    gmb.AddButton(2, IDS_CANCEL, TRUE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      return PWScore::USER_CANCEL;
    m_core.SetCurFile(NewName.c_str());
#if !defined(POCKET_PC)
    m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") +
                                       m_core.GetCurFile()).c_str();
    SetWindowText(LPCTSTR(m_titlebar));
    app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
  }
  rc = m_core.WriteCurFile();

  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, m_core.GetCurFile().c_str());
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  SetChanged(Clear);
  ChangeOkUpdate();
  return PWScore::SUCCESS;
}

int DboxMain::SaveIfChanged()
{
  // offer to save existing database if it was modified.
  // used before loading another
  // returns PWScore::SUCCESS if save succeeded or if user decided
  // not to save

  if (m_core.IsChanged()) {
    int rc, rc2;
    CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, m_core.GetCurFile().c_str());
    rc = MessageBox(cs_temp, AfxGetAppName(),
      MB_ICONQUESTION|MB_YESNOCANCEL);
    switch (rc) {
      case IDCANCEL:
        return PWScore::USER_CANCEL;
      case IDYES:
        rc2 = Save();
        // Make sure that file was successfully written
        if (rc2 == PWScore::SUCCESS)
          break;
        else
          return PWScore::CANT_OPEN_FILE;
      case IDNO:
        // Reset changed flag
        SetChanged(Clear);
        break;
    }
  }
  return PWScore::SUCCESS;
}

void DboxMain::OnSaveAs()
{
  SaveAs();
}

int DboxMain::SaveAs()
{
  INT_PTR rc;
  StringX newfile;
  CString cs_msg, cs_title, cs_text, cs_temp;

  if (m_core.GetReadFileVersion() != PWSfile::VCURRENT &&
      m_core.GetReadFileVersion() != PWSfile::UNKNOWN_VERSION) {
    cs_msg.Format(IDS_NEWFORMAT2, m_core.GetCurFile().c_str());
    cs_title.LoadString(IDS_VERSIONWARNING);
    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(1, IDS_CONTINUE);
    gmb.AddButton(2, IDS_CANCEL, TRUE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      return PWScore::USER_CANCEL;
  }

  //SaveAs-type dialog box
  StringX cf(m_core.GetCurFile());
  if (cf.empty()) {
    CString defname;
    defname.LoadString(IDS_DEFDBNAME); // reasonable default for first time user
    cf = LPCTSTR(defname);
  }
  stringT v3FileName = PWSUtil::GetNewFileName(cf.c_str(), DEFAULT_SUFFIX );
  while (1) {
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   v3FileName.c_str(),
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   SUFFIX_FILTERS
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    if (m_core.GetCurFile().empty())
      cs_text.LoadString(IDS_NEWNAME1);
    else
      cs_text.LoadString(IDS_NEWNAME2);
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
      newfile = fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }
  stringT locker(_T("")); // null init is important here
  // Note: We have to lock the new file before releasing the old (on success)
  if (!m_core.LockFile2(newfile.c_str(), locker)) {
    cs_temp.Format(IDS_FILEISLOCKED, newfile.c_str(), locker.c_str());
    cs_title.LoadString(IDS_FILELOCKERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  // Save file UUID, clear it to generate new one, restore if necessary
  uuid_array_t file_uuid_array;
  m_core.GetFileUUID(file_uuid_array);
  m_core.ClearFileUUID();

  rc = m_core.WriteFile(newfile);

  if (rc == PWScore::CANT_OPEN_FILE) {
    m_core.SetFileUUID(file_uuid_array);
    m_core.UnlockFile2(newfile.c_str());
    cs_temp.Format(IDS_CANTOPENWRITING, newfile.c_str());
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  if (!m_core.GetCurFile().empty())
    m_core.UnlockFile(m_core.GetCurFile().c_str());

  // Move the newfile lock to the right place
  m_core.MoveLock();

  m_core.SetCurFile(newfile);
#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(_T("Password Safe - ") +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCTSTR(m_titlebar));
  app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
  SetChanged(Clear);
  ChangeOkUpdate();

  app.AddToMRU( newfile.c_str() );

  if (m_core.IsReadOnly()) {
    // reset read-only status (new file can't be read-only!)
    // and so cause toolbar to be the correct version
    m_core.SetReadOnly(false);
  }

  return PWScore::SUCCESS;
}

void DboxMain::OnExportVx(UINT nID)
{
  INT_PTR rc;
  StringX newfile;
  CString cs_text, cs_title, cs_temp;

  //SaveAs-type dialog box
  stringT OldFormatFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                      _T("dat") );
  cs_text.LoadString(IDS_NAMEEXPORTFILE);
  while (1) {
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   OldFormatFileName.c_str(),
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   SUFFIX_FILTERS
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = cs_text;
    rc = fd.DoModal();
    if (m_inExit) {
      // If U3ExitNow called while in CFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling 
      PostQuitMessage(0);
      return;
    }
    if (rc == IDOK) {
      newfile = fd.GetPathName();
      break;
    } else
      return;
  }

  switch (nID) {
    case ID_MENUITEM_EXPORT2OLD1XFORMAT:
      rc = m_core.WriteV17File(newfile);
      break;
    case ID_MENUITEM_EXPORT2V2FORMAT:
      rc = m_core.WriteV2File(newfile);
      break;
    default:
      ASSERT(0);
      rc = PWScore::FAILURE;
      break;
  }
  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, newfile.c_str());
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
  }
}

void DboxMain::OnExportText()
{
  CExportTextDlg et;
  CString cs_text, cs_title;
  StringX cs_temp;

  cs_temp = m_core.GetCurFile();
  if (cs_temp.empty()) {
    //  Database has not been saved - prompt user to do so first!
    AfxMessageBox(IDS_SAVEBEFOREEXPORT);
    return;
  }

  INT_PTR rc = et.DoModal();
  if (rc == IDOK) {
    StringX newfile;
    StringX pw(et.m_exportTextPassword);
    if (m_core.CheckPassword(cs_temp, pw) == PWScore::SUCCESS) {
      // do the export
      //SaveAs-type dialog box
      stringT TxtFileName = PWSUtil::GetNewFileName(cs_temp.c_str(), _T("txt"));
      cs_text.LoadString(IDS_NAMETEXTFILE);
      while (1) {
        CFileDialog fd(FALSE,
                       _T("txt"),
                       TxtFileName.c_str(),
                       OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                       |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                       _T("Text files (*.txt)|*.txt|")
                       _T("CSV files (*.csv)|*.csv|")
                       _T("All files (*.*)|*.*|")
                       _T("|"),
                       this);
        fd.m_ofn.lpstrTitle = cs_text;
        rc = fd.DoModal();
        if (m_inExit) {
          // If U3ExitNow called while in CFileDialog,
          // PostQuitMessage makes us return here instead
          // of exiting the app. Try resignalling 
          PostQuitMessage(0);
          return;
        }
        if (rc == IDOK) {
          newfile = fd.GetPathName();
          break;
        } else
          return;
      } // while (1)

      const CItemData::FieldBits bsExport = et.m_bsExport;
      const stringT subgroup_name = et.m_subgroup_name;
      const int subgroup_object = et.m_subgroup_object;
      const int subgroup_function = et.m_subgroup_function;
      TCHAR delimiter = et.m_defexpdelim[0];

      OrderedItemList orderedItemList;
      MakeOrderedItemList(orderedItemList);

      rc = m_core.WritePlaintextFile(newfile, bsExport, subgroup_name,
                                     subgroup_object, subgroup_function,
                                     delimiter, &orderedItemList);
      orderedItemList.clear(); // cleanup soonest

      if (rc == PWScore::CANT_OPEN_FILE) {
        CString errmess;
        errmess.Format(IDS_CANTOPENWRITING, newfile.c_str());
        cs_title.LoadString(IDS_FILEWRITEERROR);
        MessageBox(errmess, cs_title, MB_OK|MB_ICONWARNING);
      }
    } else {
      AfxMessageBox(IDS_BADPASSKEY);
      Sleep(3000); // against automatic attacks
    }
  }
}

void DboxMain::OnExportXML()
{
  CExportXMLDlg eXML;
  CString cs_text, cs_title, cs_temp;

  INT_PTR rc = eXML.DoModal();
  if (rc == IDOK) {
    StringX newfile;
    StringX pw(eXML.m_ExportXMLPassword);
    if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
      // do the export
      //SaveAs-type dialog box
      stringT XMLFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                    _T("xml"));
      cs_text.LoadString(IDS_NAMEXMLFILE);
      while (1) {
        CFileDialog fd(FALSE,
                       _T("xml"),
                       XMLFileName.c_str(),
                       OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                       |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                       _T("XML files (*.xml)|*.xml|")
                       _T("All files (*.*)|*.*|")
                       _T("|"),
                       this);
        fd.m_ofn.lpstrTitle = cs_text;
        rc = fd.DoModal();
        if (m_inExit) {
          // If U3ExitNow called while in CFileDialog,
          // PostQuitMessage makes us return here instead
          // of exiting the app. Try resignalling 
          PostQuitMessage(0);
          return;
        }
        if (rc == IDOK) {
          newfile = fd.GetPathName();
          break;
        } else
          return;
      } // while (1)

      const CItemData::FieldBits bsExport = eXML.m_bsExport;
      const stringT subgroup_name = eXML.m_subgroup_name;
      const int subgroup_object = eXML.m_subgroup_object;
      const int subgroup_function = eXML.m_subgroup_function;
      TCHAR delimiter;
      delimiter = eXML.m_defexpdelim[0];

      OrderedItemList orderedItemList;
      MakeOrderedItemList(orderedItemList);
      rc = m_core.WriteXMLFile(newfile, bsExport, subgroup_name,
                               subgroup_object, subgroup_function,
                               delimiter, &orderedItemList);

      orderedItemList.clear(); // cleanup soonest

      if (rc == PWScore::CANT_OPEN_FILE)        {
        cs_temp.Format(IDS_CANTOPENWRITING, newfile.c_str());
        cs_title.LoadString(IDS_FILEWRITEERROR);
        MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
      }
    } else {
      AfxMessageBox(IDS_BADPASSKEY);
      Sleep(3000); // protect against automatic attacks
    }
  }
}

void DboxMain::OnImportText()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CImportDlg dlg;
  INT_PTR status = dlg.DoModal();

  if (status == IDCANCEL)
    return;

  StringX ImportedPrefix(dlg.m_groupName);
  CString cs_text, cs_title, cs_temp;
  TCHAR fieldSeparator(dlg.m_Separator[0]);
  CFileDialog fd(TRUE,
                 _T("txt"),
                 NULL,
                 OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                 _T("Text files (*.txt)|*.txt|")
                 _T("CSV files (*.csv)|*.csv|")
                 _T("All files (*.*)|*.*|")
                 _T("|"),
                 this);
  cs_text.LoadString(IDS_PICKTEXTFILE);
  fd.m_ofn.lpstrTitle = cs_text;
  INT_PTR rc = fd.DoModal();
  if (m_inExit) {
    // If U3ExitNow called while in CFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }
  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    stringT strError;
    StringX TxtFileName = fd.GetPathName();
    int numImported = 0, numSkipped = 0;
    TCHAR delimiter = dlg.m_defimpdelim[0];

    /* Create report as we go */
    CReport rpt;
    stringT cs_text;
    LoadAString(cs_text, IDS_RPTIMPORTTEXT);
    rpt.StartReport(cs_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(cs_text, IDS_TEXT);
    cs_temp.Format(IDS_IMPORTFILE, cs_text.c_str(), TxtFileName.c_str());
    rpt.WriteLine((LPCTSTR)cs_temp);
    rpt.WriteLine();

    rc = m_core.ImportPlaintextFile(ImportedPrefix, TxtFileName, strError, fieldSeparator,
      delimiter, numImported, numSkipped, rpt);

    cs_title.LoadString(IDS_FILEREADERROR);
    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
        cs_temp.Format(IDS_CANTOPENREADING, TxtFileName.c_str());
        break;
      case PWScore::INVALID_FORMAT:
        cs_temp.Format(IDS_INVALIDFORMAT, TxtFileName.c_str());
        break;
      case PWScore::FAILURE:
        cs_title.LoadString(IDS_TEXTIMPORTFAILED);
        break;
      case PWScore::SUCCESS:
      default:
      {
        rpt.WriteLine();
        CString cs_type, temp1, temp2 = _T("");
        cs_type.LoadString(numImported == 1 ? IDS_ENTRY : IDS_ENTRIES);
        temp1.Format(IDS_RECORDSIMPORTED, numImported, cs_type);
        rpt.WriteLine((LPCTSTR)temp1);
        if (numSkipped != 0) {
          cs_type.LoadString(numSkipped == 1 ? IDS_ENTRY : IDS_ENTRIES);
          temp2.Format(IDS_RECORDSNOTREAD, numSkipped, cs_type);
          rpt.WriteLine((LPCTSTR)temp2);
        }

        cs_title.LoadString(IDS_STATUS);
        cs_temp = temp1 + CString("\n") + temp2;

        ChangeOkUpdate();
        RefreshViews();
        break;
      }
    } // switch
    // Finish Report
    rpt.EndReport();

    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_temp);
    gmb.SetStandardIcon(rc == PWScore::SUCCESS ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
    gmb.AddButton(1, _T("OK"), TRUE, TRUE);
    gmb.AddButton(2, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      ViewReport(rpt);

    // May need to update menu/toolbar if original database was empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);
  }
}

void DboxMain::OnImportKeePass()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CString cs_text, cs_title, cs_temp;
  CFileDialog fd(TRUE,
                 _T("txt"),
                 NULL,
                 OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                 _T("Text files (*.txt)|*.txt|")
                 _T("CSV files (*.csv)|*.csv|")
                 _T("All files (*.*)|*.*|")
                 _T("|"),
                 this);
  cs_text.LoadString(IDS_PICKKEEPASSFILE);
  fd.m_ofn.lpstrTitle = cs_text;
  INT_PTR rc = fd.DoModal();
  if (m_inExit) {
    // If U3ExitNow called while in CFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }
  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    StringX KPsFileName = fd.GetPathName();
    rc = m_core.ImportKeePassTextFile(KPsFileName);
    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
      {
        cs_temp.Format(IDS_CANTOPENREADING, KPsFileName.c_str());
        cs_title.LoadString(IDS_FILEOPENERROR);
        MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
        break;
      }
      case PWScore::INVALID_FORMAT:
      {
        cs_temp.Format(IDS_INVALIDFORMAT, KPsFileName.c_str());
        cs_title.LoadString(IDS_FILEREADERROR);
        MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
        break;
      }
      case PWScore::SUCCESS:
      default:
        RefreshViews();
        ChangeOkUpdate();
        // May need to update menu/toolbar if original database was empty
        if (bWasEmpty)
          UpdateMenuAndToolBar(m_bOpen);
        break;
    } // switch
  }
}

void DboxMain::OnImportXML()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CString cs_title, cs_temp, cs_text;
  stringT csErrors(_T(""));
  const stringT XSDfn(_T("pwsafe.xsd"));
  stringT XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  // Expat is a non-validating parser - no use for Schema!
  if (!pws_os::FileExists(XSDFilename)) {
    cs_temp.Format(IDSC_MISSINGXSD, XSDfn.c_str());
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }
#endif

  CImportXMLDlg dlg;
  INT_PTR status = dlg.DoModal();

  if (status == IDCANCEL)
    return;

  stringT ImportedPrefix(dlg.m_groupName);

  CFileDialog fd(TRUE,
                 _T("xml"),
                 NULL,
                 OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                 _T("XML files (*.xml)|*.xml||"),
                 this);
  cs_text.LoadString(IDS_PICKXMLFILE);
  fd.m_ofn.lpstrTitle = cs_text;

  INT_PTR rc = fd.DoModal();
  if (m_inExit) {
    // If U3ExitNow called while in CFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }
  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    stringT strErrors;
    CString XMLFilename = fd.GetPathName();
    int numValidated, numImported;
    bool bBadUnknownFileFields, bBadUnknownRecordFields;
    CWaitCursor waitCursor;  // This may take a while!
    /* Create report as we go */
    CReport rpt;
    CString cs_text;
    cs_text.LoadString(IDS_RPTIMPORTXML);
    rpt.StartReport(cs_text, m_core.GetCurFile().c_str());
    cs_text.LoadString(IDS_XML);
    cs_temp.Format(IDS_IMPORTFILE, cs_text, XMLFilename);
    rpt.WriteLine((LPCTSTR)cs_temp);
    rpt.WriteLine();
    rc = m_core.ImportXMLFile(ImportedPrefix, stringT(XMLFilename),
                              XSDFilename.c_str(), strErrors,
                              numValidated, numImported,
                              bBadUnknownFileFields, bBadUnknownRecordFields,
                              rpt);
    waitCursor.Restore();  // Restore normal cursor

    cs_title.LoadString(IDS_XMLIMPORTFAILED);
    switch (rc) {
      case PWScore::XML_FAILED_VALIDATION:
        cs_temp.Format(IDS_FAILEDXMLVALIDATE, fd.GetFileName(),
                       strErrors.c_str());
        break;
      case PWScore::XML_FAILED_IMPORT:
        cs_temp.Format(IDS_XMLERRORS, fd.GetFileName(), strErrors.c_str());
        break;
      case PWScore::SUCCESS:
        if (!strErrors.empty() ||
            bBadUnknownFileFields || bBadUnknownRecordFields) {
          if (!strErrors.empty())
            csErrors = strErrors + _T("\n");
          if (bBadUnknownFileFields) {
            cs_temp.Format(IDS_XMLUNKNFLDIGNORED, _T("header"));
            csErrors += cs_temp + _T("\n");
          }
          if (bBadUnknownRecordFields) {
            cs_temp.Format(IDS_XMLUNKNFLDIGNORED, _T("record"));
            csErrors += cs_temp;
          }

          cs_temp.Format(IDS_XMLIMPORTWITHERRORS,
                         fd.GetFileName(), numValidated,
                         numImported, csErrors.c_str());

          ChangeOkUpdate();
        } else {
          const CString cs_validate(MAKEINTRESOURCE(numValidated == 1 ? IDS_ENTRY : IDS_ENTRIES));
          const CString cs_imported(MAKEINTRESOURCE(numValidated == 1 ? IDS_ENTRY : IDS_ENTRIES));
          cs_temp.Format(IDS_XMLIMPORTOK,
                         numValidated, cs_validate, numImported, cs_imported);
          cs_title.LoadString(IDS_STATUS);
          ChangeOkUpdate();
        }
        RefreshViews();
        break;
      default:
        ASSERT(0);
    } // switch

    // Finish Report
    rpt.WriteLine((LPCTSTR)cs_temp);
    rpt.WriteLine();
    rpt.WriteLine(csErrors.c_str());
    rpt.EndReport();

    CGeneralMsgBox gmb;
    if (rc != PWScore::SUCCESS || !strErrors.empty())
      gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    else
      gmb.SetStandardIcon(MB_ICONINFORMATION);

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_temp);
    gmb.AddButton(1, _T("OK"), TRUE, TRUE);
    gmb.AddButton(2, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      ViewReport(rpt);

    // May need to update menu/toolbar if original database was empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);
  }
}

int DboxMain::Merge()
{
  int rc = PWScore::SUCCESS;
  StringX newfile;
  CString cs_temp;

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   DEFAULT_SUFFIX,
                   NULL,
                   OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_READONLY|OFN_LONGNAMES,
                   SUFFIX_FILTERS
                   _T("Password Safe Backups (*.bak)|*.bak|")
                   _T("Password Safe Intermediate Backups (*.ibak)|*.ibak|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    cs_temp.LoadString(IDS_PICKMERGEFILE);
    fd.m_ofn.lpstrTitle = cs_temp;
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
      newfile = fd.GetPathName();

      rc = Merge(newfile);

      if (rc == PWScore::SUCCESS)
        break;
    } else
      return PWScore::USER_CANCEL;
  }

  return rc;
}

void DboxMain::OnMerge()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  Merge();
}

// Merge flags indicating differing fields if group, title and user are identical
#define MRG_PASSWORD   0x8000
#define MRG_NOTES      0x4000
#define MRG_URL        0x2000
#define MRG_AUTOTYPE   0x1000
#define MRG_HISTORY    0x0800
#define MRG_POLICY     0x0400
#define MRG_XTIME      0x0200
#define MRG_XTIME_INT  0x0100
#define MRG_UNUSED     0x00ff

int DboxMain::Merge(const StringX &pszFilename) {
  /* open file they want to merge */
  StringX passkey, temp;

  //Check that this file isn't already open
  if (pszFilename == m_core.GetCurFile()) {
    //It is the same damn file
    AfxMessageBox(IDS_ALREADYOPEN, MB_OK|MB_ICONWARNING);
    return PWScore::ALREADY_OPEN;
  }

  // Force input database into read-only status
  PWScore othercore;
  int rc = GetAndCheckPassword(pszFilename, passkey,
                               GCP_ADVANCED, // OK, CANCEL, HELP
                               true,         // readonly
                               true,         // user cannot change readonly status
                               &othercore,   // Use other core
                               ADV_MERGE);   // Advanced type

  CString cs_temp, cs_title;
  switch (rc) {
    case PWScore::SUCCESS:
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPEN, othercore.GetCurFile().c_str());
      cs_title.LoadString(IDS_FILEOPENERROR);
      MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    case TAR_OPEN:
    case TAR_NEW:
    case PWScore::WRONG_PASSWORD:
    case PWScore::USER_CANCEL:
      /*
      If the user just cancelled out of the password dialog,
      assume they want to return to where they were before...
      */
      othercore.ClearData();
      return PWScore::USER_CANCEL;
  }

  othercore.ReadFile(pszFilename, passkey);

  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENREADING, pszFilename.c_str());
    cs_title.LoadString(IDS_FILEREADERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    /*
    Everything stays as is... Worst case,
    they saved their file....
    */
    othercore.ClearData();
    return PWScore::CANT_OPEN_FILE;
  }

  othercore.SetCurFile(pszFilename);

  /* Put up hourglass...this might take a while */
  CWaitCursor waitCursor;

  bool bWasEmpty = m_core.GetNumEntries() == 0;

  /* Create report as we go */
  CReport rpt;
  CString cs_text;
  cs_text.LoadString(IDS_RPTMERGE);
  rpt.StartReport(cs_text, m_core.GetCurFile().c_str());
  cs_temp.Format(IDS_MERGINGDATABASE, pszFilename.c_str());
  rpt.WriteLine((LPCTSTR)cs_temp);

  /*
  Purpose:
  Merge entries from otherCore to m_core

  Algorithm:
  Foreach entry in otherCore
    Find in m_core
    if find a match
      if pw, notes, & group also matches
        no merge
      else
        add to m_core with new title suffixed with -merged-YYYYMMDD-HHMMSS
    else
      add to m_core directly
  */
  int numAdded = 0;
  int numConflicts = 0;
  int numAliasesAdded = 0;
  int numShortcutsAdded = 0;
  uuid_array_t base_uuid, new_base_uuid;
  bool bTitleRenamed(false);

  ItemListConstIter otherPos;
  for (otherPos = othercore.GetEntryIter();
       otherPos != othercore.GetEntryEndIter();
       otherPos++) {
    CItemData otherItem = othercore.GetEntry(otherPos);
    CItemData::EntryType et = otherItem.GetEntryType();

    // Handle Aliases and Shortcuts when processing their base entries
    if (et == CItemData::ET_ALIAS || et == CItemData::ET_SHORTCUT)
      continue;

    if (m_subgroup_set == BST_CHECKED &&
        !otherItem.Matches(stringT(m_subgroup_name),
                           m_subgroup_object, m_subgroup_function))
      continue;

    const StringX otherGroup = otherItem.GetGroup();
    const StringX otherTitle = otherItem.GetTitle();
    const StringX otherUser = otherItem.GetUser();

    CString timeStr(_T(""));
    ItemListConstIter foundPos = m_core.Find(otherGroup, otherTitle, otherUser);

    otherItem.GetUUID(base_uuid);
    memcpy(new_base_uuid, base_uuid, sizeof(uuid_array_t));
    bTitleRenamed = false;
    if (foundPos != m_core.GetEntryEndIter()) {
      /* found a match, see if other fields also match */
      CItemData curItem = m_core.GetEntry(foundPos);

      CString csDiffs(_T("")), cs_temp;
      int diff_flags = 0;
      int cxtint, oxtint;
      time_t cxt, oxt;
      if (otherItem.GetPassword() != curItem.GetPassword()) {
        diff_flags |= MRG_PASSWORD;
        cs_temp.LoadString(IDS_PASSWORD);
        csDiffs += cs_temp + _T(", ");
      }
      if (otherItem.GetNotes() != curItem.GetNotes()) {
        diff_flags |= MRG_NOTES;
        cs_temp.LoadString(IDS_NOTES);
        csDiffs += cs_temp + _T(", ");
      }
      if (otherItem.GetURL() != curItem.GetURL()) {
        diff_flags |= MRG_URL;
        cs_temp.LoadString(IDS_URL);
        csDiffs += cs_temp + _T(", ");
      }
      if (otherItem.GetAutoType() != curItem.GetAutoType()) {
        diff_flags |= MRG_AUTOTYPE;
        cs_temp.LoadString(IDS_AUTOTYPE);
        csDiffs += cs_temp + _T(", ");
      }
      if (otherItem.GetPWHistory() != curItem.GetPWHistory()) {
        diff_flags |= MRG_HISTORY;
        cs_temp.LoadString(IDS_PWHISTORY);
        csDiffs += cs_temp + _T(", ");
      }
      if (otherItem.GetPWPolicy() != curItem.GetPWPolicy()) {
        diff_flags |= MRG_POLICY;
        cs_temp.LoadString(IDS_PWPOLICY);
        csDiffs += cs_temp + _T(", ");
      }
      otherItem.GetXTime(oxt);
      curItem.GetXTime(cxt);
      if (oxt != cxt) {
        diff_flags |= MRG_XTIME;
        cs_temp.LoadString(IDS_PASSWORDEXPIRYDATE);
        csDiffs += cs_temp + _T(", ");
      }
      otherItem.GetXTimeInt(oxtint);
      curItem.GetXTimeInt(cxtint);
      if (oxtint != cxtint) {
        diff_flags |= MRG_XTIME_INT;
        cs_temp.LoadString(IDS_PASSWORDEXPIRYDATEINT);
        csDiffs += cs_temp + _T(", ");
      }
      if (diff_flags |= 0) {
        /* have a match on title/user, but not on other fields
        add an entry suffixed with -merged-YYYYMMDD-HHMMSS */
        StringX newTitle = otherTitle;
        CTime curTime = CTime::GetCurrentTime();
        newTitle += _T("-merged-");
        timeStr = curTime.Format(_T("%Y%m%d-%H%M%S"));
        newTitle += timeStr;

        /* note it as an issue for the user */
        CString warnMsg;
        warnMsg.Format(IDS_MERGECONFLICTS, 
                       otherItem.GetGroup().c_str(), 
                       otherItem.GetTitle().c_str(),
                       otherItem.GetUser().c_str(),
                       otherItem.GetGroup().c_str(), 
                       newTitle.c_str(), 
                       otherItem.GetUser().c_str(),
                       csDiffs);

        /* log it */
        rpt.WriteLine((LPCTSTR)warnMsg);

        /* Check no conflict of unique uuid */
        if (m_core.Find(base_uuid) != m_core.GetEntryEndIter()) {
          otherItem.CreateUUID();
          otherItem.GetUUID(new_base_uuid);
        }

        /* do it */
        bTitleRenamed = true;
        otherItem.SetTitle(newTitle);
        m_core.AddEntry(otherItem);
        numConflicts++;
      }
    } else {
      /* didn't find any match...add it directly */
      /* Check no conflict of unique uuid */
      if (m_core.Find(base_uuid) != m_core.GetEntryEndIter()) {
        otherItem.CreateUUID();
        otherItem.GetUUID(new_base_uuid);
      }

      m_core.AddEntry(otherItem);
      numAdded++;
    }
    if (et == CItemData::ET_ALIASBASE)
      numAliasesAdded += MergeDependents(&othercore, 
                      base_uuid, new_base_uuid,
                      bTitleRenamed, timeStr, CItemData::ET_ALIAS);
    if (et == CItemData::ET_SHORTCUTBASE)
      numShortcutsAdded += MergeDependents(&othercore,
                      base_uuid, new_base_uuid, 
                      bTitleRenamed, timeStr, CItemData::ET_SHORTCUT); 
  } // iteration over other core's entries

  othercore.ClearData();

  waitCursor.Restore(); /* restore normal cursor */

  /* tell the user we're done & provide short merge report */
  int totalAdded = numAdded + numConflicts + numAliasesAdded + numShortcutsAdded;
  CString resultStr;
  const CString cs_entries(MAKEINTRESOURCE(totalAdded == 1 ? IDS_ENTRY : IDS_ENTRIES));
  const CString cs_conflicts(MAKEINTRESOURCE(numConflicts == 1 ? IDS_CONFLICT : IDS_CONFLICTS));
  const CString cs_aliases(MAKEINTRESOURCE(numAliasesAdded == 1 ? IDS_ALIAS : IDS_ALIASES));
  const CString cs_shortcuts(MAKEINTRESOURCE(numShortcutsAdded == 1 ? IDS_SHORTCUT : IDS_SHORTCUTS));
  resultStr.Format(IDS_MERGECOMPLETED,
                   totalAdded, cs_entries, numConflicts, cs_conflicts,
                   numAliasesAdded, cs_aliases,
                   numShortcutsAdded, cs_shortcuts);
  cs_title.LoadString(IDS_MERGECOMPLETED2);
  //MessageBox(resultStr, cs_title, MB_OK);
  rpt.WriteLine((LPCTSTR)resultStr);
  rpt.EndReport();

  CGeneralMsgBox gmb;
  gmb.SetTitle(cs_title);
  gmb.SetMsg(resultStr);
  gmb.SetStandardIcon(MB_ICONINFORMATION);
  gmb.AddButton(1, _T("OK"), TRUE, TRUE);
  gmb.AddButton(2, IDS_VIEWREPORT);
  INT_PTR msg_rc = gmb.DoModal();
  if (msg_rc == 2)
    ViewReport(rpt);

  ChangeOkUpdate();
  RefreshViews();
  // May need to update menu/toolbar if original database was empty
  if (bWasEmpty)
    UpdateMenuAndToolBar(m_bOpen);

  return rc;
}

int DboxMain::MergeDependents(PWScore *pothercore,
                              uuid_array_t &base_uuid, uuid_array_t &new_base_uuid, 
                              const bool bTitleRenamed, CString &timeStr, 
                              const CItemData::EntryType et)
{
  UUIDList dependentslist;
  UUIDListIter paiter;
  ItemListIter iter;
  uuid_array_t entry_uuid, new_entry_uuid;
  ItemListConstIter foundPos;
  CItemData tempitem;
  int numadded(0);

  // Get all the dependents
  pothercore->GetAllDependentEntries(base_uuid, dependentslist, et);
  for (paiter = dependentslist.begin();
       paiter != dependentslist.end(); paiter++) {
    paiter->GetUUID(entry_uuid);
    iter = pothercore->Find(entry_uuid);

    if (iter == pothercore->GetEntryEndIter())
      continue;

    CItemData *curitem = &iter->second;
    tempitem = (*curitem);

    memcpy(new_entry_uuid, entry_uuid, sizeof(uuid_array_t));
    if (m_core.Find(entry_uuid) != m_core.GetEntryEndIter()) {
      tempitem.CreateUUID();
      tempitem.GetUUID(new_entry_uuid);
    }

    // If the base title was renamed - we should automatically rename any dependent.
    // If we didn't, we still need to check uniqueness!
    StringX newTitle = tempitem.GetTitle();
    if (bTitleRenamed) {
      newTitle += _T("-merged-");
      newTitle += timeStr;
      tempitem.SetTitle(newTitle);
    }
    // Check this is unique - if not - don't add this one! - its only an alias/shortcut!
    // We can't keep trying for uniqueness after adding a timestanp!
    foundPos = m_core.Find(tempitem.GetGroup(), newTitle, tempitem.GetUser());
    if (foundPos != m_core.GetEntryEndIter()) 
      continue;

    m_core.AddEntry(tempitem);
    m_core.AddDependentEntry(new_base_uuid, new_entry_uuid, et);

    if (et == CItemData::ET_ALIAS) {
      tempitem.SetPassword(_T("[Alias]"));
      tempitem.SetAlias();
    } else
    if (et == CItemData::ET_SHORTCUT) {
      tempitem.SetPassword(_T("[Shortcut]"));
      tempitem.SetShortcut();
    } else
      ASSERT(0);

    numadded++;
  }
  return numadded;
}

void DboxMain::OnProperties()
{
  CProperties dlg(m_core);

  dlg.DoModal();
}

void DboxMain::OnCompare()
{
  INT_PTR rc = PWScore::SUCCESS;
  if (m_core.GetCurFile().empty()) {
    AfxMessageBox(IDS_NOCOMPAREFILE, MB_OK|MB_ICONWARNING);
    return;
  }

  StringX cs_file2;
  CString cs_text(MAKEINTRESOURCE(IDS_PICKCOMPAREFILE));

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   DEFAULT_SUFFIX,
                   NULL,
                   OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                   SUFFIX_FILTERS
                   _T("Password Safe Backups (*.bak)|*.bak|")
                   _T("Password Safe Intermediate Backups (*.ibak)|*.ibak|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
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
      return;
    }
    if (rc == IDOK) {
      cs_file2 = fd.GetPathName();
      //Check that this file isn't the current one!
      if (cs_file2 == m_core.GetCurFile()) {
        //It is the same damn file!
        AfxMessageBox(IDS_COMPARESAME, MB_OK|MB_ICONWARNING);
      } else {
        const StringX cs_file1(m_core.GetCurFile());
        rc = Compare(cs_file1, cs_file2);
        break;
      }
    } else {
      rc = PWScore::USER_CANCEL;
      break;
    }
  }

  return;
}

int DboxMain::Compare(const StringX &cs_Filename1, const StringX &cs_Filename2)
{
  // open file they want to Compare
  int rc = PWScore::SUCCESS;

  StringX passkey;
  CString cs_temp, cs_title, cs_text;
  PWScore othercore;

  // Reading a new file changes the preferences!
  const StringX cs_SavePrefString(PWSprefs::GetInstance()->Store());

  // OK, CANCEL, HELP, ADVANCED + (nolonger force R/O) + use othercore
  rc = GetAndCheckPassword(cs_Filename2, passkey,
                           GCP_ADVANCED, // OK, CANCEL, HELP
                           true,         // readonly
                           false,        // user can change readonly status
                           &othercore,   // Use other core
                           ADV_COMPARE); // Advanced type
  switch (rc) {
    case PWScore::SUCCESS:
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPEN, cs_Filename2.c_str());
      cs_title.LoadString(IDS_FILEOPENERROR);
      MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    case TAR_OPEN:
      return Open();
    case TAR_NEW:
      return New();
    case PWScore::WRONG_PASSWORD:
    case PWScore::USER_CANCEL:
      /*
      If the user just cancelled out of the password dialog,
      assume they want to return to where they were before...
      */
      return PWScore::USER_CANCEL;
  }

  // Not really needed but...
  othercore.ClearData();

  rc = othercore.ReadFile(cs_Filename2, passkey);

  switch (rc) {
    case PWScore::SUCCESS:
      break;
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPENREADING, cs_Filename2.c_str());
      MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
      break;
    case PWScore::BAD_DIGEST:
    {
      cs_temp.Format(IDS_FILECORRUPT, cs_Filename2.c_str());
      const int yn = MessageBox(cs_temp, cs_title, MB_YESNO|MB_ICONERROR);
      if (yn == IDYES)
        rc = PWScore::SUCCESS;
      break;
    }
#ifdef DEMO
    case PWScore::LIMIT_REACHED:
    {
      CString cs_msg; cs_msg.Format(IDS_LIMIT_MSG2, MAXDEMO);
      CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE));
      MessageBox(cs_msg, cs_title, MB_ICONWARNING);
      break;
    }
#endif
    default:
      cs_temp.Format(IDS_UNKNOWNERROR, cs_Filename2.c_str());
      MessageBox(cs_temp, cs_title, MB_OK|MB_ICONERROR);
      break;
  }

  if (rc != PWScore::SUCCESS) {
    othercore.ClearData();
    othercore.SetCurFile(_T(""));
    return rc;
  }

  othercore.SetCurFile(cs_Filename2);

  CompareData list_OnlyInCurrent;
  CompareData list_OnlyInComp;
  CompareData list_Conflicts;
  CompareData list_Identical;

  /* Create report as we go */
  CReport rpt;
  cs_text.LoadString(IDS_RPTCOMPARE);
  rpt.StartReport(cs_text, m_core.GetCurFile().c_str());
  cs_temp.Format(IDS_COMPARINGDATABASE, cs_Filename2.c_str());
  rpt.WriteLine((LPCTSTR)cs_temp);
  rpt.WriteLine();

  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  /*
  Purpose:
  Compare entries from comparison database (compCore) with current database (m_core)

  Algorithm:
  Foreach entry in current database {
    Find in comparison database - subject to subgroup checking
    if found {
      Compare
      if match
        OK
      else
        There are conflicts; note them & increment numConflicts
    } else {
      save & increment numOnlyInCurrent
    }
  }

  Foreach entry in comparison database {
    Find in current database - subject to subgroup checking
    if not found
      save & increment numOnlyInComp
  }
  */

  if (!m_bAdvanced) {
    // turn off time fields if not explicitly turned on by user via Advanced dialog
    m_bsFields.reset(CItemData::CTIME);
    m_bsFields.reset(CItemData::PMTIME);
    m_bsFields.reset(CItemData::ATIME);
    m_bsFields.reset(CItemData::XTIME);
    m_bsFields.reset(CItemData::RMTIME);
  }

  int numOnlyInCurrent = 0;
  int numOnlyInComp = 0;
  int numConflicts = 0;
  int numIdentical = 0;

  CItemData::FieldBits bsConflicts(0);
  st_CompareData st_data;
  uuid_array_t xuuid;

  ItemListIter currentPos;
  for (currentPos = m_core.GetEntryIter();
       currentPos != m_core.GetEntryEndIter();
       currentPos++) {
    CItemData currentItem = m_core.GetEntry(currentPos);

    if (m_subgroup_set == BST_UNCHECKED ||
        currentItem.Matches(stringT(m_subgroup_name), m_subgroup_object,
                            m_subgroup_function)) {
      st_data.group = currentItem.GetGroup();
      st_data.title = currentItem.GetTitle();
      st_data.user = currentItem.GetUser();

      ItemListIter foundPos = othercore.Find(st_data.group,
                                             st_data.title, st_data.user);
      if (foundPos != othercore.GetEntryEndIter()) {
        // found a match, see if all other fields also match
        // Difference flags:
        /*
         First byte (values in square brackets taken from ItemData.h)
         1... ....  NAME     [0x00] - n/a - depreciated
         .1.. ....  UUID     [0x01] - n/a - unique
         ..1. ....  GROUP    [0x02] - not checked - must be identical
         ...1 ....  TITLE    [0x03] - not checked - must be identical
         .... 1...  USER     [0x04] - not checked - must be identical
         .... .1..  NOTES    [0x05]
         .... ..1.  PASSWORD [0x06]
         .... ...1  CTIME    [0x07] - not checked by default

         Second byte
         1... ....  PMTIME   [0x08] - not checked by default
         .1.. ....  ATIME    [0x09] - not checked by default
         ..1. ....  XTIME    [0x0a] - not checked by default
         ...1 ....  RESERVED [0x0b] - not used
         .... 1...  RMTIME   [0x0c] - not checked by default
         .... .1..  URL      [0x0d]
         .... ..1.  AUTOTYPE [0x0e]
         .... ...1  PWHIST   [0x0f]

         Third byte
         1... ....  POLICY   [0x10]
        */
        bsConflicts.reset();

        CItemData compItem = othercore.GetEntry(foundPos);
        if (m_bsFields.test(CItemData::NOTES) &&
            FieldsNotEqual(currentItem.GetNotes(), compItem.GetNotes()))
          bsConflicts.flip(CItemData::NOTES);
        if (m_bsFields.test(CItemData::PASSWORD) &&
            currentItem.GetPassword() != compItem.GetPassword())
          bsConflicts.flip(CItemData::PASSWORD);
        if (m_bAdvanced) {
          // Only checked if specified by the user in via the Advanced dialog
          if (m_bsFields.test(CItemData::CTIME) &&
              currentItem.GetCTime() != compItem.GetCTime())
            bsConflicts.flip(CItemData::CTIME);
          if (m_bsFields.test(CItemData::PMTIME) &&
              currentItem.GetPMTime() != compItem.GetPMTime())
            bsConflicts.flip(CItemData::PMTIME);
          if (m_bsFields.test(CItemData::ATIME) &&
              currentItem.GetATime() != compItem.GetATime())
            bsConflicts.flip(CItemData::ATIME);
          if (m_bsFields.test(CItemData::XTIME) &&
              currentItem.GetXTime() != compItem.GetXTime())
            bsConflicts.flip(CItemData::XTIME);
          if (m_bsFields.test(CItemData::RMTIME) &&
              currentItem.GetRMTime() != compItem.GetRMTime())
            bsConflicts.flip(CItemData::RMTIME);
        }
        if (m_bsFields.test(CItemData::URL) &&
            FieldsNotEqual(currentItem.GetURL(), compItem.GetURL()))
          bsConflicts.flip(CItemData::URL);
        if (m_bsFields.test(CItemData::AUTOTYPE) &&
            FieldsNotEqual(currentItem.GetAutoType(), compItem.GetAutoType()))
          bsConflicts.flip(CItemData::AUTOTYPE);
        if (m_bsFields.test(CItemData::PWHIST) &&
            currentItem.GetPWHistory() != compItem.GetPWHistory())
          bsConflicts.flip(CItemData::PWHIST);
        if (m_bsFields.test(CItemData::POLICY) &&
            currentItem.GetPWPolicy() != compItem.GetPWPolicy())
          bsConflicts.flip(CItemData::POLICY);

        currentPos->first.GetUUID(xuuid);
        memcpy(st_data.uuid0, xuuid, sizeof(uuid_array_t));
        foundPos->first.GetUUID(xuuid);
        memcpy(st_data.uuid1, xuuid, sizeof(uuid_array_t));
        st_data.bsDiffs = bsConflicts;
        st_data.indatabase = CCompareResultsDlg::BOTH;
        st_data.unknflds0 = currentItem.NumberUnknownFields() > 0;
        st_data.unknflds1 = compItem.NumberUnknownFields() > 0;

        if (bsConflicts.any()) {
          numConflicts++;
          st_data.id = numConflicts;
          list_Conflicts.push_back(st_data);
        } else {
          numIdentical++;
          st_data.id = numIdentical;
          list_Identical.push_back(st_data);
        }
      } else {
        /* didn't find any match... */
        numOnlyInCurrent++;
        currentPos->first.GetUUID(xuuid);
        memcpy(st_data.uuid0, xuuid, sizeof(uuid_array_t));
        memset(st_data.uuid1, 0x00, sizeof(uuid_array_t));
        st_data.bsDiffs.reset();
        st_data.indatabase = CCompareResultsDlg::CURRENT;
        st_data.unknflds0 = currentItem.NumberUnknownFields() > 0;
        st_data.unknflds1 = false;
        st_data.id = numOnlyInCurrent;
        list_OnlyInCurrent.push_back(st_data);
      }
    }
  } // iteration over our entries

  ItemListIter compPos;
  for (compPos = othercore.GetEntryIter();
       compPos != othercore.GetEntryEndIter();
       compPos++) {
    CItemData compItem = othercore.GetEntry(compPos);

    if (m_subgroup_set == BST_UNCHECKED ||
        compItem.Matches(stringT(m_subgroup_name), m_subgroup_object,
                         m_subgroup_function)) {
      st_data.group = compItem.GetGroup();
      st_data.title = compItem.GetTitle();
      st_data.user = compItem.GetUser();

      if (m_core.Find(st_data.group, st_data.title, st_data.user) ==
          m_core.GetEntryEndIter()) {
        /* didn't find any match... */
        numOnlyInComp++;
        memset(st_data.uuid0, 0x00, sizeof(uuid_array_t));
        compPos->first.GetUUID(xuuid);
        memcpy(st_data.uuid1, xuuid, sizeof(uuid_array_t));
        st_data.bsDiffs.reset();
        st_data.indatabase = CCompareResultsDlg::COMPARE;
        st_data.unknflds0 = false;
        st_data.unknflds1 = compItem.NumberUnknownFields() > 0;
        st_data.id = numOnlyInComp;
        list_OnlyInComp.push_back(st_data);
      }
    }
  } // iteration over other core's element

  waitCursor.Restore(); // restore normal cursor

  CString buffer;
  // tell the user we're done & provide short Compare report
  if (!m_bAdvanced) {
    cs_temp.LoadString(IDS_NONE);
    buffer.Format(IDS_ADVANCEDOPTIONS, cs_temp);
    rpt.WriteLine((LPCTSTR)buffer);
    rpt.WriteLine();
  } else {
    if (m_subgroup_set == BST_UNCHECKED) {
      cs_temp.LoadString(IDS_NONE);
    } else {
      CString cs_Object, cs_case;
      UINT uistring;

      switch(m_subgroup_object) {
        case CItemData::GROUP:
          uistring = IDS_GROUP;
          break;
        case CItemData::TITLE:
          uistring = IDS_TITLE;
          break;
        case CItemData::USER:
          uistring = IDS_USERNAME;
          break;
        case CItemData::GROUPTITLE:
          uistring = IDS_GROUPTITLE;
          break;
        case CItemData::URL:
          uistring = IDS_URL;
          break;
        case CItemData::NOTES:
          uistring = IDS_NOTES;
          break;
        default:
          ASSERT(0);
      }
      cs_Object.LoadString(uistring);

      cs_case.LoadString(m_subgroup_function > 0 ? IDS_ADVCASE_INSENSITIVE : IDS_ADVCASE_SENSITIVE);

      switch (m_subgroup_function) {
        case -PWSMatch::MR_EQUALS:
        case  PWSMatch::MR_EQUALS:
          uistring = IDSC_EQUALS;
          break;
        case -PWSMatch::MR_NOTEQUAL:
        case  PWSMatch::MR_NOTEQUAL:
          uistring = IDSC_DOESNOTEQUAL;
          break;
        case -PWSMatch::MR_BEGINS:
        case  PWSMatch::MR_BEGINS:
          uistring = IDSC_BEGINSWITH;
          break;
        case -PWSMatch::MR_NOTBEGIN:
        case  PWSMatch::MR_NOTBEGIN:
          uistring = IDSC_DOESNOTBEGINSWITH;
          break;
        case -PWSMatch::MR_ENDS:
        case  PWSMatch::MR_ENDS:
          uistring = IDSC_ENDSWITH;
          break;
        case -PWSMatch::MR_NOTEND:
        case  PWSMatch::MR_NOTEND:
          uistring = IDSC_DOESNOTENDWITH;
          break;
        case -PWSMatch::MR_CONTAINS:
        case  PWSMatch::MR_CONTAINS:
          uistring = IDSC_CONTAINS;
          break;
        case -PWSMatch::MR_NOTCONTAIN:
        case  PWSMatch::MR_NOTCONTAIN:
          uistring = IDSC_DOESNOTCONTAIN;
          break;
        default:
          ASSERT(0);
      }
      cs_text.LoadString(uistring);
      cs_temp.Format(IDS_ADVANCEDSUBSET, cs_Object, cs_text, m_subgroup_name,
                     cs_case);
    }
    buffer.Format(IDS_ADVANCEDOPTIONS, cs_temp);
    rpt.WriteLine((LPCTSTR)buffer);
    rpt.WriteLine();

    cs_temp.LoadString(IDS_RPTCOMPARE);
    buffer.Format(IDS_ADVANCEDFIELDS, cs_temp);
    rpt.WriteLine((LPCTSTR)buffer);

    buffer = _T("\t");
    if (m_bsFields.test(CItemData::PASSWORD))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPPASSWORD));
    if (m_bsFields.test(CItemData::NOTES))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPNOTES));
    if (m_bsFields.test(CItemData::URL))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPURL));
    if (m_bsFields.test(CItemData::AUTOTYPE))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPAUTOTYPE));
    if (m_bsFields.test(CItemData::CTIME))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPCTIME));
    if (m_bsFields.test(CItemData::PMTIME))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPPMTIME));
    if (m_bsFields.test(CItemData::ATIME))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPATIME));
    if (m_bsFields.test(CItemData::XTIME))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPXTIME));
    if (m_bsFields.test(CItemData::RMTIME))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPRMTIME));
    if (m_bsFields.test(CItemData::PWHIST))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPPWHISTORY));
    if (m_bsFields.test(CItemData::POLICY))
      buffer += _T("\t") + CString(MAKEINTRESOURCE(IDS_COMPPWPOLICY));
    rpt.WriteLine((LPCTSTR)buffer);
    rpt.WriteLine();
  }

  cs_title.LoadString(IDS_COMPARECOMPLETE);
  buffer.Format(IDS_COMPARESTATISTICS,
                cs_Filename1.c_str(), cs_Filename2.c_str());

  if (numOnlyInCurrent == 0 && numOnlyInComp == 0 && numConflicts == 0) {
    cs_text.LoadString(IDS_IDENTICALDATABASES);
    buffer += cs_text;
    MessageBox(buffer, cs_title, MB_OK);
    rpt.WriteLine((LPCTSTR)buffer);
    rpt.EndReport();
  } else {
    CCompareResultsDlg CmpRes(this, list_OnlyInCurrent, list_OnlyInComp, 
                              list_Conflicts, list_Identical, 
                              m_bsFields, &m_core, &othercore, &rpt);

    CmpRes.m_cs_Filename1 = cs_Filename1;
    CmpRes.m_cs_Filename2 = cs_Filename2;
    CmpRes.m_bOriginalDBReadOnly = m_core.IsReadOnly();
    CmpRes.m_bComparisonDBReadOnly = othercore.IsReadOnly();

    INT_PTR rc = CmpRes.DoModal();
    if (CmpRes.m_OriginalDBChanged) {
      FixListIndexes();
      RefreshViews();
    }

    if (CmpRes.m_ComparisonDBChanged) {
      SaveCore(&othercore);
    }

    rpt.EndReport();

    if (rc == 2)
      ViewReport(rpt);
  }

  if (othercore.IsLockedFile(othercore.GetCurFile().c_str()))
    othercore.UnlockFile(othercore.GetCurFile().c_str());

  othercore.ClearData();
  othercore.SetCurFile(_T(""));

  // Reset database preferences - first to defaults then add saved changes!
  PWSprefs::GetInstance()->Load(cs_SavePrefString);

  return rc;
}

int DboxMain::SaveCore(PWScore *pcore)
{
  // Stolen from Save!
  int rc;
  CString cs_title, cs_msg, cs_temp;
  PWSprefs *prefs = PWSprefs::GetInstance();

  if (pcore->GetReadFileVersion() == PWSfile::VCURRENT) {
    if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
      int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
      int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
      StringX userBackupPrefix = prefs->GetPref(PWSprefs::BackupPrefixValue);
      StringX userBackupDir = prefs->GetPref(PWSprefs::BackupDir);
      if (!pcore->BackupCurFile(maxNumIncBackups, backupSuffix,
                                userBackupPrefix.c_str(),
                                userBackupDir.c_str()))
        AfxMessageBox(IDS_NOIBACKUP, MB_OK);
    }
  } else { // file version mis-match
    stringT NewName = PWSUtil::GetNewFileName(pcore->GetCurFile().c_str(),
                                              DEFAULT_SUFFIX );
    cs_msg.Format(IDS_NEWFORMAT,
                  pcore->GetCurFile().c_str(), NewName.c_str());
    cs_title.LoadString(IDS_VERSIONWARNING);

    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONWARNING);
    gmb.AddButton(1, IDS_CONTINUE);
    gmb.AddButton(2, IDS_CANCEL, FALSE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      return PWScore::USER_CANCEL;
    pcore->SetCurFile(NewName.c_str());
  }
  rc = pcore->WriteCurFile();

  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, pcore->GetCurFile().c_str());
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  pcore->SetChanged(false);
  return PWScore::SUCCESS;
}

LRESULT DboxMain::OnProcessCompareResultFunction(WPARAM wParam, LPARAM lFunction)
{
  PWScore *pcore;
  st_CompareInfo *st_info;
  LRESULT lres(FALSE);
  uuid_array_t entryUUID;

  st_info = (st_CompareInfo *)wParam;

  if (st_info->clicked_column == CCompareResultsDlg::CURRENT) {
    pcore = st_info->pcore0;
    memcpy(entryUUID, st_info->uuid0, sizeof(uuid_array_t));
  } else {
    pcore = st_info->pcore1;
    memcpy(entryUUID, st_info->uuid1, sizeof(uuid_array_t));
  }

  switch ((int)lFunction) {
    case CCompareResultsDlg::EDIT:
      lres = EditCompareResult(pcore, entryUUID);
      break;      
    case CCompareResultsDlg::VIEW:
      lres = ViewCompareResult(pcore, entryUUID);
      break;
    case CCompareResultsDlg::COPY_TO_ORIGINALDB:
      lres = CopyCompareResult(st_info->pcore1, st_info->pcore0,
                               st_info->uuid1, st_info->uuid0);
      break;
    case CCompareResultsDlg::COPY_TO_COMPARISONDB:
      lres = CopyCompareResult(st_info->pcore0, st_info->pcore1,
                               st_info->uuid0, st_info->uuid1);
      break;
    default:
      ASSERT(0);
  }
  return lres;
}

LRESULT DboxMain::ViewCompareResult(PWScore *pcore, uuid_array_t &entryUUID)
{  
  ItemListIter pos = pcore->Find(entryUUID);
  ASSERT(pos != pcore->GetEntryEndIter());
  CItemData *ci = &pos->second;

  // View the correct entry and make sure R/O
  bool bSaveRO = pcore->IsReadOnly();
  pcore->SetReadOnly(true);

  EditItem(ci, pcore);

  pcore->SetReadOnly(bSaveRO);

  return FALSE;
}

LRESULT DboxMain::EditCompareResult(PWScore *pcore, uuid_array_t &entryUUID)
{
  ItemListIter pos = pcore->Find(entryUUID);
  ASSERT(pos != pcore->GetEntryEndIter());
  CItemData *ci = &pos->second;

  // Edit the correct entry
  return EditItem(ci, pcore) ? TRUE : FALSE;
}

LRESULT DboxMain::CopyCompareResult(PWScore *pfromcore, PWScore *ptocore,
                                    uuid_array_t &fromUUID, uuid_array_t &toUUID)
{
  // Copy *pfromcore -> *ptocore entry at fromPos

  ItemListIter toPos;
  StringX group, title, user, notes, password, url, autotype, pwhistory;
  time_t ct, at, xt, pmt, rmt;
  PWPolicy pwp;
  int nfromUnknownRecordFields;
  bool bFromUUIDIsNotInTo;

  ItemListIter fromPos = pfromcore->Find(fromUUID);
  ASSERT(fromPos != pfromcore->GetEntryEndIter());
  const CItemData *fromEntry = &fromPos->second;

  group = fromEntry->GetGroup();
  title = fromEntry->GetTitle();
  user = fromEntry->GetUser();
  notes = fromEntry->GetNotes();
  password = fromEntry->GetPassword();
  url = fromEntry->GetURL();
  autotype = fromEntry->GetAutoType();
  pwhistory = fromEntry->GetPWHistory();
  fromEntry->GetCTime(ct);
  fromEntry->GetATime(at);
  fromEntry->GetXTime(xt);
  fromEntry->GetPMTime(pmt);
  fromEntry->GetRMTime(rmt);
  fromEntry->GetPWPolicy(pwp);
  nfromUnknownRecordFields = fromEntry->NumberUnknownFields();

  bFromUUIDIsNotInTo = (ptocore->Find(fromUUID) == ptocore->GetEntryEndIter());

  // Is it already there:?
  toPos = ptocore->Find(group, title, user);
  if (toPos != ptocore->GetEntryEndIter()) {
    // Yes - just overwrite everything!
    CItemData *toEntry = &ptocore->GetEntry(toPos);

    toEntry->SetNotes(notes);
    toEntry->SetPassword(password);
    toEntry->SetURL(url);
    toEntry->SetAutoType(autotype);
    toEntry->SetPWHistory(pwhistory);
    toEntry->SetCTime(ct);
    toEntry->SetATime(at);
    toEntry->SetXTime(xt);
    toEntry->SetPMTime(pmt);
    toEntry->SetRMTime(rmt);
    toEntry->SetPWPolicy(pwp);

    // If the UUID is not in use, copy it too, otherwise reuse current
    if (bFromUUIDIsNotInTo)
      toEntry->SetUUID(fromUUID);

    toEntry->GetUUID(toUUID);

    // Delete any old unknown records and copy these if present
    int ntoUnknownRecordFields = toEntry->NumberUnknownFields();

    if (ntoUnknownRecordFields == 0 && nfromUnknownRecordFields > 0)
      ptocore->IncrementNumRecordsWithUnknownFields();
    if (ntoUnknownRecordFields > 0 && nfromUnknownRecordFields == 0)
      ptocore->DecrementNumRecordsWithUnknownFields();

    toEntry->ClearUnknownFields();
    if (nfromUnknownRecordFields != 0) {
      unsigned int length = 0;
      unsigned char type;
      unsigned char *pdata = NULL;

      for (int i = 0; i < nfromUnknownRecordFields; i++) {
        fromEntry->GetUnknownField(type, length, pdata, i);
        if (length == 0)
          continue;
        toEntry->SetUnknownField(type, length, pdata);
        trashMemory(pdata, length);
        delete[] pdata;
      }
    }
  } else {
    CItemData temp;

    // If the UUID is not in use, copy it too otherwise create it
    if (bFromUUIDIsNotInTo)
      temp.SetUUID(fromUUID);
    else
      temp.CreateUUID();

    temp.GetUUID(toUUID);
    temp.SetGroup(group);
    temp.SetTitle(title);
    temp.SetUser(user);
    temp.SetPassword(password);
    temp.SetNotes(notes);
    temp.SetURL(url);
    temp.SetAutoType(autotype);
    temp.SetPWHistory(pwhistory);
    temp.SetCTime(ct);
    temp.SetATime(at);
    temp.SetXTime(xt);
    temp.SetPMTime(pmt);
    temp.SetRMTime(rmt);
    temp.SetPWPolicy(pwp);
    if (nfromUnknownRecordFields != 0) {
      ptocore->IncrementNumRecordsWithUnknownFields();

      for (int i = 0; i < nfromUnknownRecordFields; i++) {
        unsigned int length = 0;
        unsigned char type;
        unsigned char *pdata = NULL;
        fromEntry->GetUnknownField(type, length, pdata, i);
        if (length == 0)
          continue;
        temp.SetUnknownField(type, length, pdata);
        trashMemory(pdata, length);
        delete[] pdata;
      }
    }
    ptocore->AddEntry(temp);
  }

  ptocore->SetChanged(true);
  return TRUE;
}

void DboxMain::OnOK() 
{
  int rc, rc2;

  PWSprefs::IntPrefs WidthPrefs[] = {
    PWSprefs::Column1Width,
    PWSprefs::Column2Width,
    PWSprefs::Column3Width,
    PWSprefs::Column4Width,
  };
  PWSprefs *prefs = PWSprefs::GetInstance();

  LVCOLUMN lvColumn;
  lvColumn.mask = LVCF_WIDTH;

  for (int i = 0; i < 4; i++) {
    if (m_ctlItemList.GetColumn(i, &lvColumn)) {
      prefs->SetPref(WidthPrefs[i], lvColumn.cx);
    }
  }

  CString cs_columns(_T(""));
  CString cs_columnswidths(_T(""));
  TCHAR buffer[8], widths[8];

  for (int iOrder = 0; iOrder < m_nColumns; iOrder++) {
    int iIndex = m_nColumnIndexByOrder[iOrder];
#if _MSC_VER >= 1400
    _itot_s(m_nColumnTypeByIndex[iIndex], buffer, 8, 10);
    _itot_s(m_nColumnWidthByIndex[iIndex], widths, 8, 10);
#else
    _itot(m_nColumnTypeByIndex[iIndex], buffer, 10);
    _itot(m_nColumnWidthByIndex[iIndex], widths, 10);
#endif
    cs_columns += buffer;
    cs_columnswidths += widths;
    cs_columns += _T(",");
    cs_columnswidths += _T(",");
  }

  prefs->SetPref(PWSprefs::SortedColumn, m_iTypeSortColumn);
  prefs->SetPref(PWSprefs::SortAscending, m_bSortAscending);
  prefs->SetPref(PWSprefs::ListColumns, LPCTSTR(cs_columns));
  prefs->SetPref(PWSprefs::ColumnWidths, LPCTSTR(cs_columnswidths));

  SaveDisplayStatus(); // since it's not always up to date
  // (CPWTreeCtrl::OnExpandCollapse not always called!)

  //Store current filename for next time...
  if (prefs->GetPref(PWSprefs::MaxMRUItems) == 0) {
    // Ensure Application preferences have been changed for a rewrite
    prefs->SetPref(PWSprefs::CurrentFile, _T(""));
    prefs->SetPref(PWSprefs::CurrentBackup, _T(""));
    prefs->ForceWriteApplicationPreferences();

    // Naughty Windows saves information in the registry for every Open and Save!
    RegistryAnonymity();
  } else
  if (!m_core.GetCurFile().empty())
    prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());

  bool autoSave = true; // false if user saved or decided not to 
  if (m_core.IsChanged()) {
    CString cs_msg(MAKEINTRESOURCE(IDS_SAVEFIRST));
    switch (m_iSessionEndingStatus) {
      case IDIGNORE:
        // Session is not ending - user has an option to cancel
        rc = MessageBox(cs_msg, AfxGetAppName(), MB_ICONQUESTION | MB_YESNOCANCEL);
        break;
      case IDOK:
        // Session is ending - user does not have an option to cancel
        rc = MessageBox(cs_msg, AfxGetAppName(), MB_ICONQUESTION | MB_YESNO);
        break;
      case IDNO:
      case IDYES:
        // IDYES: Don't ask - user already said YES during OnQueryEndSession
        // IDNO:  Don't ask - user already said NO during OnQueryEndSession
        rc = m_iSessionEndingStatus;
        break; 
      default: 
        ASSERT(0); // should never happen... 
        rc = IDCANCEL; // ...but if it does, behave conservatively. 
    }
    switch (rc) {
      case IDCANCEL:
        return;
      case IDYES:
        autoSave = false;
        rc2 = Save();
        if (rc2 != PWScore::SUCCESS)
          return;
      case IDNO:
        autoSave = false;
        break;
    }
  } // core.IsChanged()

  // Save silently (without asking user) iff:
  // 0. User didn't explicitly save OR say that she doesn't want to AND
  // 1. NOT read-only AND
  // 2. (timestamp updates OR tree view display vector changed) AND
  // 3. database NOT empty
  // Less formally:
  //
  // If MaintainDateTimeStamps set and not read-only,
  // save without asking user: "they get what it says on the tin"
  // Note that if database was cleared (e.g., locked), it might be
  // possible to save an empty list :-(
  // Protect against this both here and in OnSize (where we minimize
  // & possibly ClearData).

  if (autoSave && !m_core.IsReadOnly() &&
      (m_bTSUpdated || m_core.WasDisplayStatusChanged()) &&
      m_core.GetNumEntries() > 0)
    Save();

  // Clear clipboard on Exit?  Yes if:
  // a. the app is minimized and the systemtray is enabled
  // b. the user has set the "ClearClipboardOnExit" pref
  // c. the system is shutting down, restarting or the user is logging off
  if ((!IsWindowVisible() && prefs->GetPref(PWSprefs::UseSystemTray)) ||
      prefs->GetPref(PWSprefs::ClearClipboardOnExit) ||
      (m_iSessionEndingStatus == IDYES)) {
    ClearClipboardData();
  }

  // Now save the Find Toolbar display status
  prefs->SetPref(PWSprefs::ShowFindToolBarOnOpen, m_FindToolBar.IsVisible() == TRUE);

  // wipe data, save prefs, go home.
  ClearData();
  prefs->SaveApplicationPreferences();
  // Cleanup here - doesn't work in ~DboxMain or ~CCoolMenuManager
  m_menuManager.Cleanup();

  // Clear out filters
  m_MapFilters.clear();

  CDialog::OnOK();
}

void DboxMain::OnCancel()
{
  // If system tray is enabled, cancel (X on title bar) closes
  // window, else exit application
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray))
    ShowWindow(SW_MINIMIZE);
  else
    OnOK();
}

void DboxMain::SaveDisplayStatus()
{
  vector <bool> v = GetGroupDisplayStatus(); // update it
  m_core.SetDisplayStatus(v); // store it
}

void DboxMain::RestoreDisplayStatus()
{
  const vector<bool> &displaystatus = m_core.GetDisplayStatus();    

  if (!displaystatus.empty())
    SetGroupDisplayStatus(displaystatus);
}

vector<bool> DboxMain::GetGroupDisplayStatus()
{
  HTREEITEM hItem = NULL;
  vector<bool> v;

  if (m_ctlItemTree.GetSafeHwnd() == NULL)
    return v;

  while ( NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem)) ) {
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      bool state = (m_ctlItemTree.GetItemState(hItem, TVIS_EXPANDED)
                    & TVIS_EXPANDED) != 0;
      v.push_back(state);
    }
  }
  return v;
}

void DboxMain::SetGroupDisplayStatus(const vector<bool> &displaystatus)
{
  // We need to copy displaystatus since Expand may cause
  // SaveDisplayStatus to be called, updating it

  // Could be called from OnSize before anything set up!
  // Check Tree is valid first
  if (m_ctlItemTree.GetSafeHwnd() == NULL || displaystatus.empty())
    return;

  const vector<bool> dstatus(displaystatus);
  const size_t num = dstatus.size();
  if (num == 0)
    return;

  HTREEITEM hItem = NULL;
  size_t i(0);
  while (NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem))) {
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      m_ctlItemTree.Expand(hItem, dstatus[i] ? TVE_EXPAND : TVE_COLLAPSE);
      i++;
      if (i == num)
        break;
    }
  }
}

void DboxMain::RegistryAnonymity()
{
  // For the paranoid - definitely remove information from Registry of previous
  // directory containing PWS databases!
  // Certainly for WinXP but should do no harm on other versions.
  const CString csSubkey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32");

  HKEY hSubkey;
  LONG dw;

  // First deal with information saved by Windows Common Dialog for Open/Save of
  // the file types used by PWS in its CFileDialog
  dw = RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey + _T("\\OpenSaveMRU"), NULL,
                    KEY_ALL_ACCESS, &hSubkey);

  if (dw == ERROR_SUCCESS) {
    // Delete entries relating to PWS
    app.DelRegTree(hSubkey, _T("psafe3"));
    app.DelRegTree(hSubkey, _T("ibak"));
    app.DelRegTree(hSubkey, _T("bak"));
    app.DelRegTree(hSubkey, _T("*"));

    dw = RegCloseKey(hSubkey);
    ASSERT(dw == ERROR_SUCCESS);
  }

  // Now deal with Windows remembering the last directory visited by PWS
  dw = RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey + _T("\\LastVisitedMRU"), NULL,
                      KEY_ALL_ACCESS, &hSubkey);

  if (dw == ERROR_SUCCESS) {
    CString cs_AppName;
    TCHAR szMRUList[_MAX_PATH], szAppNameAndDir[_MAX_PATH * 2];
    TCHAR szMRUListMember[2];
    DWORD dwMRUListLength, dwAppNameAndDirLength, dwType(0);
    int iNumberOfMRU, iIndex;
    dwMRUListLength = sizeof(szMRUList);

    // Get the MRU List
    dw = RegQueryValueEx(hSubkey, _T("MRUList"), NULL,
                         &dwType, (LPBYTE)szMRUList, &dwMRUListLength);
    if (dw == ERROR_SUCCESS) {
      iNumberOfMRU = dwMRUListLength / sizeof(TCHAR);

      // Search the MRU List
      szMRUListMember[1] = _T('\0');
      for (iIndex = 0; iIndex < iNumberOfMRU; iIndex++) {
        szMRUListMember[0] = szMRUList[iIndex];

        dwAppNameAndDirLength = sizeof(szAppNameAndDir);
        // Note: these Registry entries are stored in RG_BINARY format as 2 concatenated
        // Unicode null terminated strings: L"application" L"Last fully qualified Directory"
        dw = RegQueryValueEx(hSubkey, szMRUListMember, 0, &dwType,
                             (LPBYTE)szAppNameAndDir, &dwAppNameAndDirLength);
        if (dw == ERROR_SUCCESS) {
          cs_AppName = szAppNameAndDir;
          if (cs_AppName.MakeLower() == _T("pwsafe.exe")) {
            dw = RegDeleteValue(hSubkey, szMRUListMember);
            if (dw == ERROR_SUCCESS) {
              // Remove deleted entry from MRU List and rewrite it
              CString cs_NewMRUList(szMRUList);
              iNumberOfMRU = cs_NewMRUList.Delete(iIndex, 1);
              LPTSTR pszNewMRUList = cs_NewMRUList.GetBuffer(iNumberOfMRU);
              dw = RegSetValueEx(hSubkey, _T("MRUList"), 0, REG_SZ, (LPBYTE)pszNewMRUList,
                            (iNumberOfMRU + 1) * sizeof(TCHAR));
              ASSERT(dw == ERROR_SUCCESS);
              cs_NewMRUList.ReleaseBuffer();
            }
            break;
          }
        }
      }
    }
    dw = RegCloseKey(hSubkey);
    ASSERT(dw == ERROR_SUCCESS);
  }
  return;
}
