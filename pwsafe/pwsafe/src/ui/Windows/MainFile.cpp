/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "DboxMain.h"
#include "PasskeySetup.h"
#include "TryAgainDlg.h"
#include "ImportTextDlg.h"
#include "ImportXMLDlg.h"
#include "AdvancedDlg.h"
#include "CompareResultsDlg.h"
#include "Properties.h"
#include "GeneralMsgBox.h"
#include "MFCMessages.h"
#include "PWFileDialog.h"
#include "DisplayFSBkupFiles.h"
#include "ExpPWListDlg.h"

#include "WZPropertySheet.h"

#include "core/PWSprefs.h"
#include "core/Util.h"
#include "core/PWSdirs.h"
#include "core/Report.h"
#include "core/ItemData.h"
#include "core/core.h"
#include "core/VerifyFormat.h"
#include "core/SysInfo.h"
#include "core/XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"
#include "core/ExpiredList.h"

#include "os/file.h"
#include "os/dir.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include <sys/types.h>
#include <bitset>
#include <vector>

using namespace std;
using pws_os::CUUID;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void DisplayFileWriteError(INT_PTR rc, const StringX &cs_newfile)
{
  ASSERT(rc != PWScore::SUCCESS);

  CGeneralMsgBox gmb;
  CString cs_temp, cs_title(MAKEINTRESOURCE(IDS_FILEWRITEERROR));
  switch (rc) {
  case PWScore::CANT_OPEN_FILE:
    cs_temp.Format(IDS_CANTOPENWRITING, cs_newfile.c_str());
    break;
  case PWScore::FAILURE:
    cs_temp.Format(IDS_FILEWRITEFAILURE);
    break;
  case PWScore::WRONG_PASSWORD:
    cs_temp.Format(IDS_MISSINGPASSKEY);
    break;
  default:
    cs_temp.Format(IDS_UNKNOWNERROR, cs_newfile.c_str());
    break;
  }
  gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
}

BOOL DboxMain::OpenOnInit()
{
  /*
    Routine to account for the differences between opening PSafe for
    the first time, and just opening a different database or
    un-minimizing the application
  */
  StringX passkey;
  BOOL retval(FALSE);
  bool bReadOnly = m_core.IsReadOnly();  // Can only be from -r command line parameter
  if (!bReadOnly) {
    // Command line not set - use config for first open
    bReadOnly = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO);
  }

  const StringX sxOriginalFileName = m_core.GetCurFile();
  const int flags = (bReadOnly ? GCP_READONLY : 0) | 
                    (m_core.IsReadOnly() ? GCP_FORCEREADONLY : 0);
  int rc = GetAndCheckPassword(m_core.GetCurFile(),
                               passkey, GCP_FIRST,
                               flags) ;  // First

  if (rc == PWScore::USER_CANCEL || rc == PWScore::USER_EXIT)
    return FALSE;

  CString cs_title;
  cs_title.LoadString(IDS_FILEREADERROR);
  bool bAskerSet = m_core.IsAskerSet();
  bool bReporterSet = m_core.IsReporterSet();
  MFCAsker q;
  MFCReporter r;

  if (!bAskerSet)
    m_core.SetAsker(&q);

  if (!bReporterSet)
    m_core.SetReporter(&r);

  if (rc == PWScore::SUCCESS) {
    // Verify if any recovery databases exist
    INT_PTR chkrc = CheckEmergencyBackupFiles(m_core.GetCurFile(), passkey);

    if (chkrc == IDCANCEL) {
      // Cancel "Open on Init"
      Close(false);
      CDialog::OnCancel();
      goto exit;
    }
  }

  // If the user has changed the file at OpenOnInit time but had specified
  // validation, turn off validation of the new file.  However, allow user to
  // specify just the -v command flag with no filename on the command line and
  // then validate the database selected via the initial Open dialog.
  if (m_bValidate && !sxOriginalFileName.empty() &&
      sxOriginalFileName.compare(m_core.GetCurFile()) != 0)
    m_bValidate = false;

  int rc2 = PWScore::NOT_SUCCESS;

  switch (rc) {
    case PWScore::SUCCESS:
      // Don't validate twice
      rc2 = m_core.ReadCurFile(passkey, m_bValidate ? 0 : MAXTEXTCHARS);
#if !defined(POCKET_PC)
      m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                         m_core.GetCurFile()).c_str();
      UpdateSystemTray(UNLOCKED);
#endif
      break;
    case PWScore::CANT_OPEN_FILE:
      if (m_core.GetCurFile().empty()) {
        // Empty filename. Assume they are starting Password Safe
        // for the first time and don't confuse them.
        // fall through to New()
      } else {
        // Here if there was a filename saved from last invocation, but it couldn't
        // be opened. It was either removed or renamed, so ask the user what to do
        CString cs_msg;
        cs_msg.Format(IDS_CANTOPENSAFE, m_core.GetCurFile().c_str());
        CGeneralMsgBox gmb;
        gmb.SetMsg(cs_msg);
        gmb.SetStandardIcon(MB_ICONQUESTION);
        gmb.AddButton(IDS_SEARCH, IDS_SEARCH);
        gmb.AddButton(IDS_RETRY, IDS_RETRY);
        gmb.AddButton(IDS_NEW, IDS_NEW);
        gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
        INT_PTR rc3 = gmb.DoModal();
        switch (rc3) {
          case IDS_SEARCH:
            rc2 = Open();
            break;
          case IDS_RETRY:
            return OpenOnInit();  // Recursive!
          case IDS_NEW:
            rc2 = New();
            break;
          case IDS_EXIT:
            rc2 = PWScore::USER_CANCEL;
            break;
        }
        break;
      }
    case TAR_NEW:
      rc2 = New();
      if (rc2 == PWScore::USER_CANCEL) {
        // somehow, get DboxPasskeyEntryFirst redisplayed...
      }
      break;
    case TAR_OPEN:
      rc2 = Open();
      if (rc2 == PWScore::USER_CANCEL) {
        // somehow, get DboxPasskeyEntryFirst redisplayed...
      }
      break;
    case PWScore::WRONG_PASSWORD:
    default:
      break;
  }

  bool go_ahead = false;
  /*
   * If file's corrupted, read error or LIMIT_REACHED (demo),
   * the we prompt the user, and continue or not per user's input.
   * A bit too subtle for switch/case on rc2...
   */
  if (rc2 == PWScore::BAD_DIGEST ||
      rc2 == PWScore::TRUNCATED_FILE ||
      rc2 == PWScore:: READ_FAIL) {
    CGeneralMsgBox gmb;
    CString cs_title(MAKEINTRESOURCE(IDS_FILEREADERROR)), cs_msg;
    cs_msg.Format(IDS_FILECORRUPT, m_core.GetCurFile().c_str());
    if (gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONERROR) == IDNO) {
      CDialog::OnCancel();
      goto exit;
    }
    go_ahead = true;
  } // read error
#ifdef DEMO
  if (rc2 == PWScore::LIMIT_REACHED) {
    CGeneralMsgBox gmb;
    CString cs_msg;
    cs_msg.Format(IDS_LIMIT_MSG, MAXDEMO);
    CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE));
    if (gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONWARNING) == IDNO) {
      CDialog::OnCancel();
      goto exit;
    }
    go_ahead = true;
  } // LIMIT_REACHED
#endif /* DEMO */

  if (rc2 != PWScore::SUCCESS && !go_ahead) {
    // not a good return status, fold.
    if (!m_IsStartSilent)
      CDialog::OnCancel();
    goto exit;
  }

  if (!m_bOpen) {
    // Previous state was closed - reset DCA in status bar
    SetDCAText();
  }

  PostOpenProcessing();

  // Validation does integrity check & repair on database
  // currently invoke it iff m_bValidate set (e.g., user passed '-v' flag)
  if (m_bValidate) {
    OnValidate();
    m_bValidate = false;
  }

  retval = TRUE;

exit:
  if (!bAskerSet)
    m_core.SetAsker(NULL);
  if (!bReporterSet)
    m_core.SetReporter(NULL);

  return retval;
}

void DboxMain::OnNew()
{
  New();
}

int DboxMain::New()
{
  INT_PTR rc, rc2;

  if (!m_core.IsReadOnly() && m_core.IsChanged()) {
    CGeneralMsgBox gmb;
    CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, m_core.GetCurFile().c_str());
    rc = gmb.MessageBox(cs_temp, AfxGetAppName(),
                             MB_YESNOCANCEL | MB_ICONQUESTION);
    switch (rc) {
      case IDCANCEL:
        return PWScore::USER_CANCEL;
      case IDYES:
        rc2 = Save();
        //  Make sure that writing the file was successful
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
  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, cs_newfile);
    return PWScore::USER_CANCEL;
  }
  m_core.ClearChangedNodes();

#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " + cs_newfile).c_str();
  SetWindowText(LPCWSTR(m_titlebar));
#endif

  ChangeOkUpdate();
  UpdateSystemTray(UNLOCKED);
  m_RUEList.ClearEntries();
  if (!m_bOpen) {
    // Previous state was closed - reset DCA in status bar
    SetDCAText();
  }

  // Set Dragbar images correctly
  m_DDGroup.SetStaticState(false);
  m_DDTitle.SetStaticState(false);
  m_DDPassword.SetStaticState(false);
  m_DDUser.SetStaticState(false);
  m_DDNotes.SetStaticState(false);
  m_DDURL.SetStaticState(false);
  m_DDemail.SetStaticState(false);
  m_DDAutotype.SetStaticState(false);

  UpdateMenuAndToolBar(true);
  UpdateStatusBar();

  // Set timer for user-defined idle lockout, if selected (DB preference)
  KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
    ResetIdleLockCounter();
    SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
  }
  // re-activate logout detection
  startLockCheckTimer();
  RegisterSessionNotification(true);

  return PWScore::SUCCESS;
}

int DboxMain::NewFile(StringX &newfilename)
{
  CString cs_msg, cs_temp;
  CString cs_text(MAKEINTRESOURCE(IDS_CREATENAME));

  CString cf(MAKEINTRESOURCE(IDS_DEFDBNAME)); // reasonable default for first time user
  std::wstring v3FileName = PWSUtil::GetNewFileName(LPCWSTR(cf), DEFAULT_SUFFIX);
  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  INT_PTR rc;

  while (1) {
    CPWFileDialog fd(FALSE,
                     DEFAULT_SUFFIX,
                     v3FileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(IDS_FDF_V3_ALL)),
                     this);

    fd.m_ofn.lpstrTitle = cs_text;
    fd.m_ofn.Flags &= ~OFN_READONLY;

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
      newfilename = LPCWSTR(fd.GetPathName());
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  CPasskeySetup pksetup(this);
  rc = pksetup.DoModal();

  if (rc == IDCANCEL)
    return PWScore::USER_CANCEL;  //User cancelled password entry

  // Reset core
  m_core.ReInit(true);

  ClearData();

  const StringX &oldfilename = m_core.GetCurFile();
  // The only way we're the locker is if it's locked & we're !readonly
  if (!oldfilename.empty() &&
      !m_core.IsReadOnly() &&
      m_core.IsLockedFile(oldfilename.c_str()))
    m_core.UnlockFile(oldfilename.c_str());

  m_core.SetCurFile(newfilename);

  // Now lock the new file
  std::wstring locker(L""); // null init is important here
  m_core.LockFile(newfilename.c_str(), locker);

  m_core.SetReadOnly(false); // new file can't be read-only...
  m_core.NewFile(pksetup.GetPassKey());
  m_bDBNeedsReading = false;
  
  // Tidy up filters
  m_currentfilter.Empty();
  m_bFilterActive = false;

  // Clear any saved group information
  m_TreeViewGroup = L"";

  return PWScore::SUCCESS;
}

void DboxMain::OnClose()
{
  Close();
}

int DboxMain::Close(const bool bTrySave)
{
  PWSprefs *prefs = PWSprefs::GetInstance();

  if (bTrySave) {
    // Save Application related preferences
    prefs->SaveApplicationPreferences();
    prefs->SaveShortcuts();

    if (m_bOpen) {
      // try and save it first
      int rc = SaveIfChanged();
      if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
        return rc;

      // Reset changed flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
      SetChanged(Clear);
    }
  }

  // Turn off special display if on
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  // Unlock the current file
  if (!m_core.GetCurFile().empty()) {
    m_core.UnlockFile(m_core.GetCurFile().c_str());
    m_core.SetCurFile(L"");
  }

  // Clear all associated data
  ClearData();

  // Zero entry UUID selected and first visible at minimize and group text
  m_LUUIDSelectedAtMinimize = CUUID::NullUUID();
  m_TUUIDSelectedAtMinimize = CUUID::NullUUID();
  m_LUUIDVisibleAtMinimize = CUUID::NullUUID();
  m_TUUIDVisibleAtMinimize = CUUID::NullUUID();
  m_sxSelectedGroup.clear();
  m_sxVisibleGroup.clear();

  CAddEdit_DateTimes::m_bShowUUID = false;

  // Reset core
  m_core.ReInit();

  // Tidy up filters
  m_currentfilter.Empty();
  m_bFilterActive = false;
  ApplyFilters();

  // Set Dragbar images correctly
  m_DDGroup.SetStaticState(false);
  m_DDTitle.SetStaticState(false);
  m_DDPassword.SetStaticState(false);
  m_DDUser.SetStaticState(false);
  m_DDNotes.SetStaticState(false);
  m_DDURL.SetStaticState(false);
  m_DDemail.SetStaticState(false);
  m_DDAutotype.SetStaticState(false);

  app.SetTooltipText(L"PasswordSafe");
  UpdateSystemTray(CLOSED);

  // Call UpdateMenuAndToolBar before UpdateStatusBar, as it sets m_bOpen
  UpdateMenuAndToolBar(false);
  m_titlebar = L"Password Safe";
  SetWindowText(LPCWSTR(m_titlebar));
  m_lastclipboardaction = L"";
  m_ilastaction = 0;
  UpdateStatusBar();

  // Delete any saved status information
  while (!m_stkSaveGUIInfo.empty()) {
    m_stkSaveGUIInfo.pop();
  }

  // Nothing to hide, don't lock on idle or logout
  // No need to check expired passwords
  KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
  KillTimer(TIMER_EXPENT);
  RegisterSessionNotification(false);

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
  int rc = Open(LPCWSTR(mruItem), 
                PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO));
  if (rc == PWScore::SUCCESS) {
    UpdateSystemTray(UNLOCKED);
    m_RUEList.ClearEntries();
    if (!m_bOpen) {
      // Previous state was closed - reset DCA in status bar
      SetDCAText();
    }
    UpdateMenuAndToolBar(true);
    UpdateStatusBar();
  } else {
    // Reset Read-only status
    m_core.SetReadOnly(last_ro);
  }

#if _MFC_VER > 1200
  return TRUE;
#endif
}

int DboxMain::Open(const UINT uiTitle)
{
  int rc = PWScore::SUCCESS;
  StringX sx_Filename;
  CString cs_text(MAKEINTRESOURCE(uiTitle));
  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  // Open-type dialog box
  while (1) {
    CPWFileDialog fd(TRUE,
                     DEFAULT_SUFFIX,
                     NULL,
                     OFN_FILEMUSTEXIST | OFN_LONGNAMES,
                     CString(MAKEINTRESOURCE(IDS_FDF_DB_BU_ALL)),
                     this);
    fd.m_ofn.lpstrTitle = cs_text;

    if (uiTitle == IDS_CHOOSEDATABASE) {
      // Normal Open
      if (PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO))
        fd.m_ofn.Flags |= OFN_READONLY;
      else
        fd.m_ofn.Flags &= ~OFN_READONLY;
    } else {
      // Validate
      fd.m_ofn.Flags |= (OFN_HIDEREADONLY | OFN_NOREADONLYRETURN);
    }

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

    const bool last_ro = m_core.IsReadOnly(); // restore if user cancels
    m_core.SetReadOnly(fd.GetReadOnlyPref() == TRUE);
    if (rc2 == IDOK) {
      sx_Filename = LPCWSTR(fd.GetPathName());

      rc = Open(sx_Filename, fd.GetReadOnlyPref() == TRUE, uiTitle == IDS_CHOOSEDATABASEV);
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

int DboxMain::Open(const StringX &sx_Filename, const bool bReadOnly,  const bool bHideReadOnly)
{
  CGeneralMsgBox gmb;
  INT_PTR rc1;
  int rc;
  StringX passkey;
  CString cs_temp, cs_title, cs_text;

  //Check that this file isn't already open
  if (sx_Filename == m_core.GetCurFile() && !m_bDBNeedsReading) {
    //It is the same damn file
    cs_text.LoadString(IDS_ALREADYOPEN);
    cs_title.LoadString(IDS_OPENDATABASE);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::ALREADY_OPEN;
  }

  rc = SaveIfChanged();
  if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
    return rc;

  // Reset changed flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
  SetChanged(Clear);

  // If we were using a different file, unlock it do this before 
  // GetAndCheckPassword() as that routine gets a lock on the new file
  if (!m_core.GetCurFile().empty()) {
    m_core.UnlockFile(m_core.GetCurFile().c_str());
  }

  const int flags = (bReadOnly ? GCP_READONLY : 0) | (bHideReadOnly ? GCP_HIDEREADONLY :0);
  rc = GetAndCheckPassword(sx_Filename, passkey, GCP_NORMAL, flags);  // OK, CANCEL, HELP

  // Just need file extension
  std::wstring drive, dir, name, ext;
  pws_os::splitpath(sx_Filename.c_str(), drive, dir, name, ext);

  switch (rc) {
    case PWScore::SUCCESS:
      // Do not add Failsafe Backup files to the MRU
      if (ext != L".fbak")
        app.AddToMRU(sx_Filename.c_str());
      m_bAlreadyToldUserNoSave = false;
      break; // Keep going...
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_SAFENOTEXIST, sx_Filename.c_str());
      gmb.SetTitle(IDS_FILEOPENERROR);
      gmb.SetMsg(cs_temp);
      gmb.SetStandardIcon(MB_ICONQUESTION);
      gmb.AddButton(IDS_OPEN, IDS_OPEN);
      gmb.AddButton(IDS_NEW, IDS_NEW);
      gmb.AddButton(IDS_CANCEL, IDS_CANCEL, TRUE, TRUE);
      rc1 = gmb.DoModal();
      if (rc1 == IDS_OPEN)
        return Open();
      else if (rc1 == IDS_NEW)
        return New();
      else
        return PWScore::USER_CANCEL;
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

  // Zero entry UUID selected and first visible at minimize and group text
  m_LUUIDSelectedAtMinimize = CUUID::NullUUID();
  m_TUUIDSelectedAtMinimize = CUUID::NullUUID();
  m_LUUIDVisibleAtMinimize = CUUID::NullUUID();
  m_TUUIDVisibleAtMinimize = CUUID::NullUUID();
  m_sxSelectedGroup.clear();
  m_sxVisibleGroup.clear();

  cs_title.LoadString(IDS_FILEREADERROR);
  bool bAskerSet = m_core.IsAskerSet();
  bool bReporterSet = m_core.IsReporterSet();
  MFCAsker q;
  MFCReporter r;

  if (!bAskerSet)
    m_core.SetAsker(&q);

  if (!bReporterSet)
    m_core.SetReporter(&r);

  if (rc == PWScore::SUCCESS) {
    // Verify if any recovery databases exist
    INT_PTR chkrc = CheckEmergencyBackupFiles(sx_Filename, passkey);
    
    if (chkrc == IDCANCEL) {
      // Cancel Open
      rc = PWScore::USER_CANCEL;
      goto exit;
    }
  }

  // Now read the file
  rc = m_core.ReadFile(sx_Filename, passkey, MAXTEXTCHARS);

  switch (rc) {
    case PWScore::SUCCESS:
      break;
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPENREADING, sx_Filename.c_str());
      gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
      /*
        Everything stays as is... Worst case,
        they saved their file....
      */
      rc = PWScore::CANT_OPEN_FILE;
      goto exit;
    case PWScore::BAD_DIGEST:
      cs_temp.Format(IDS_FILECORRUPT, sx_Filename.c_str());
      if (gmb.MessageBox(cs_temp, cs_title, MB_YESNO | MB_ICONERROR) == IDYES) {
        rc = PWScore::SUCCESS;
        break;
      } else
        goto exit;
#ifdef DEMO
    case PWScore::LIMIT_REACHED:
    {
      CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE)), cs_msg;
      cs_msg.Format(IDS_LIMIT_MSG, MAXDEMO);
      const int yn = gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONWARNING);
      if (yn == IDNO) {
        rc = PWScore::USER_CANCEL;
        goto exit;
      }
      rc = PWScore::SUCCESS;
      m_MainToolBar.GetToolBarCtrl().EnableButton(ID_MENUITEM_ADD, FALSE);
      break;
    }
#endif
    default:
      cs_temp.Format(IDS_UNKNOWNERROR, sx_Filename.c_str());
      gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONERROR);
      goto exit;
  }

  m_core.SetCurFile(sx_Filename);
  PostOpenProcessing();

exit:
  if (!bAskerSet)
    m_core.SetAsker(NULL);
  if (!bReporterSet)
    m_core.SetReporter(NULL);

  return rc;
}

void DboxMain::PostOpenProcessing()
{
#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCWSTR(m_titlebar));
#endif
  std::wstring drive, dir, name, ext;
  pws_os::splitpath(m_core.GetCurFile().c_str(), drive, dir, name, ext);

  // Do not add recovery files to the MRU
  if (ext != L".fbak")
    app.AddToMRU(m_core.GetCurFile().c_str());

  ChangeOkUpdate();

  // Tidy up filters
  m_currentfilter.Empty();
  m_bFilterActive = false;

  // Clear any saved group information
  m_TreeViewGroup = L"";

  RefreshViews();
  SetInitialDatabaseDisplay();
  m_bDBNeedsReading = false;
  SelectFirstEntry();

  UpdateSystemTray(UNLOCKED);
  UpdateMenuAndToolBar(true); // sets m_bOpen too...
  UpdateToolBarROStatus(m_core.IsReadOnly());
  UpdateStatusBar();

  CheckExpireList(true);
  TellUserAboutExpiredPasswords();

  UUIDList RUElist;
  m_core.GetRUEList(RUElist);
  m_RUEList.SetRUEList(RUElist);

  // Set timer for user-defined idle lockout, if selected (DB preference)
  KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
    ResetIdleLockCounter();
    SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
  }

  // Set up notification of desktop state, one way or another
  startLockCheckTimer();
  RegisterSessionNotification(true);
}

int DboxMain::CheckEmergencyBackupFiles(StringX sx_Filename, StringX &passkey)
{
  StringX sx_fullfilename;
  std::wstring wsTemp, wsDrive, wsDir, wsName, wsExt;
  int rc;

  pws_os::splitpath(sx_Filename.c_str(), wsDrive, wsDir, wsName, wsExt);
  wsTemp = wsDrive + wsDir + wsName + L"_????????_??????.fbak";
  std::wstring wsDBPath = wsDrive + wsDir;
  std::wstring wsDBName = wsName + wsExt;

  // Find all associated recovery files
  std::vector<StringX> vrecoveryfiles;
  CFileFind finder;
  BOOL bWorking = finder.FindFile(wsTemp.c_str());
  const std::wstring ws_dash(L"-"), ws_colon(L":"), ws_T(L"T");
  while (bWorking) {
    bWorking = finder.FindNextFile();
    StringX sx_FoundFilename = StringX(finder.GetFileName());

    // Verify datetime portion of name before adding to vector
    // Convert dat time string so that we can use existing verification function
    // "yyyymmdd_hhmmss" -> "yyyy-mm-ddThh:mm:ss"
    time_t t;
    std::wstring ws_datetime, ws_dt;
    // Go back "yyyymmdd_hhmmss.fbak", take only the date/time
    ws_dt = sx_FoundFilename.substr(sx_FoundFilename.length() - 20, 15).c_str();
    ws_datetime = ws_dt.substr( 0, 4) + ws_dash  + ws_dt.substr( 4, 2) + ws_dash  +
                  ws_dt.substr( 6, 2) + ws_T     + ws_dt.substr( 9, 2) + ws_colon +
                  ws_dt.substr(11, 2) + ws_colon + ws_dt.substr(13, 2);

    if (!VerifyXMLDateTimeString(ws_datetime, t))
      continue;

    vrecoveryfiles.push_back(sx_FoundFilename);
  }
  finder.Close();

  if (vrecoveryfiles.empty())
    return IDOK;

  std::vector<st_recfile> vValidEBackupfiles;
  PWScore othercore;

  // Get currently selected database's information
  st_DBProperties st_dbpcore;
  othercore.ReadFile(sx_Filename, passkey);
  othercore.GetDBProperties(st_dbpcore);
  st_dbpcore.database = wsDBName.c_str();

  // Reading a new file changes the preferences!
  const StringX sxSavePrefString(PWSprefs::GetInstance()->Store());
  const bool bDBPrefsChanged = PWSprefs::GetInstance()->IsDBprefsChanged();

  for (size_t i = 0; i < vrecoveryfiles.size(); i++) {
    st_recfile st_rf;
    st_DBProperties st_dbp;
    othercore.ReInit();
    st_rf.filename = vrecoveryfiles[i];

    // First check passphrase the same.
    sx_fullfilename = StringX(wsDBPath.c_str()) + vrecoveryfiles[i];
    rc = othercore.CheckPasskey(sx_fullfilename, passkey);

    // If it is, try to open database (i.e. same passphrase) and get
    // the header record but not change anything in m_core related to
    // current open database (hence saving the database preferences for later)
    if (rc == PWScore::SUCCESS) {
      rc = othercore.ReadFile(sx_fullfilename, passkey);
      if (rc == PWScore::SUCCESS) {
        othercore.GetDBProperties(st_dbp);
        st_dbp.database = sx_fullfilename;
        st_rf.dbp = st_dbp;
      }
    }
    st_rf.rc = rc;
    vValidEBackupfiles.push_back(st_rf);
  }
  othercore.ReInit();

  // Reset database preferences - first to defaults then add saved changes!
  PWSprefs::GetInstance()->Load(sxSavePrefString);
  PWSprefs::GetInstance()->SetDBprefsChanged(bDBPrefsChanged);

  vrecoveryfiles.clear();
  if (vValidEBackupfiles.empty())
    return IDOK;

  // Now tell user we have some recovery files and ask for guidance!
  CDisplayFSBkupFiles dsprfiles(this, wsDrive, wsDBPath, st_dbpcore, vValidEBackupfiles);

  INT_PTR dsprc = dsprfiles.DoModal();

  // Check if IDIGNORE > 0 (if < 0, then select nth entry of vValidEBackupfiles)
  if (dsprc > 0)
    return (int)dsprc;

  // User specified to open a recovery file instead
  // Close original - don't save anything
  Close(false);

  // Now open the one selected by the user in R-O mode
  sx_fullfilename = vValidEBackupfiles[-dsprc].dbp.database;
  rc = m_core.ReadFile(sx_fullfilename, passkey);
  ASSERT(rc == PWScore::SUCCESS);

  m_core.SetCurFile(sx_fullfilename);
  m_core.SetReadOnly(true);

  PostOpenProcessing();

  return IDOK;
}

void DboxMain::OnClearMRU()
{
  app.ClearMRU();
}

void DboxMain::OnSave()
{
  Save();
}

int DboxMain::Save(const SaveType savetype)
{
  int rc;
  CString cs_msg, cs_temp;
  CGeneralMsgBox gmb;
  std::wstring NewName;
  stringT bu_fname; // used to undo backup if save failed

  PWSprefs *prefs = PWSprefs::GetInstance();

  // chdir to exe dir, avoid hassle with relative paths
  PWSdirs dir(PWSdirs::GetExeDir()); // changes back in d'tor

  // Save Application related preferences
  prefs->SaveApplicationPreferences();
  prefs->SaveShortcuts();

  if (m_core.GetCurFile().empty())
    return SaveAs();

  switch (m_core.GetReadFileVersion()) {
    case PWSfile::VCURRENT:
      if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
        int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
        int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
        std::wstring userBackupPrefix = prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
        std::wstring userBackupDir = prefs->GetPref(PWSprefs::BackupDir).c_str();
        if (!m_core.BackupCurFile(maxNumIncBackups, backupSuffix,
                                  userBackupPrefix, userBackupDir, bu_fname)) {
          switch (savetype) {
            case ST_NORMALEXIT:
            {
              cs_temp.LoadString(IDS_NOIBACKUP);
              cs_msg.Format(IDS_NOIBACKUP2, cs_temp);
              gmb.SetTitle(IDS_FILEWRITEERROR);
              gmb.SetMsg(cs_msg);
              gmb.SetStandardIcon(MB_ICONEXCLAMATION);
              gmb.AddButton(IDS_SAVEAS, IDS_SAVEAS);
              gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
              INT_PTR rc = gmb.DoModal();
              if (rc == IDS_EXIT)
                return PWScore::SUCCESS;
              else
                return SaveAs();
            }
            case ST_INVALID:
              // No particular end of PWS exit i.e. user clicked Save or
              // saving a changed database before opening another
              gmb.AfxMessageBox(IDS_NOIBACKUP, MB_OK);
              return PWScore::USER_CANCEL;
          }
          gmb.AfxMessageBox(IDS_NOIBACKUP, MB_OK);
          return SaveAs();
        } // BackupCurFile failed
      } // BackupBeforeEverySave
      break;
    case PWSfile::NEWFILE:
      // file version mis-match
      NewName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                        DEFAULT_SUFFIX);

      cs_msg.Format(IDS_NEWFORMAT,
                    m_core.GetCurFile().c_str(), NewName.c_str());
      gmb.SetTitle(IDS_VERSIONWARNING);
      gmb.SetMsg(cs_msg);
      gmb.SetStandardIcon(MB_ICONWARNING);
      gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
      gmb.AddButton(IDS_CANCEL, IDS_CANCEL, TRUE, TRUE);
      if (gmb.DoModal() == IDS_CANCEL)
        return PWScore::USER_CANCEL;

      m_core.SetCurFile(NewName.c_str());
#if !defined(POCKET_PC)
      m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                         m_core.GetCurFile()).c_str();
      SetWindowText(LPCWSTR(m_titlebar));
      app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
      break;
    default:
      ASSERT(0);
  } // switch on file version

  UUIDList RUElist;
  m_RUEList.GetRUEList(RUElist);
  m_core.SetRUEList(RUElist);

  rc = m_core.WriteCurFile();

  if (rc != PWScore::SUCCESS) { // Save failed!
    // Restore backup, if we have one
    if (!bu_fname.empty() && !m_core.GetCurFile().empty())
      pws_os::RenameFile(bu_fname, m_core.GetCurFile().c_str());
    // Show user that we have a problem
    DisplayFileWriteError(rc, m_core.GetCurFile());
    return rc;
  }

  m_core.ResetStateAfterSave();
  m_core.ClearChangedNodes();
  SetChanged(Clear);
  ChangeOkUpdate();

  // Added/Modified entries now saved - reverse it & refresh display
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  if (m_bFilterActive && m_bFilterForStatus) {
    m_ctlItemList.Invalidate();
    m_ctlItemTree.Invalidate();
  }

  // Only refresh views if not existing
  if (savetype != ST_NORMALEXIT)
    RefreshViews();

  return PWScore::SUCCESS;
}

int DboxMain::SaveIfChanged()
{
  /*
   * Save silently (without asking user) iff:
   * 1. NOT read-only AND
   * 2. (timestamp updates OR tree view display vector changed) AND
   * 3. Database NOT empty
   *
   * Less formally:
   *
   * If MaintainDateTimeStamps set and not read-only, save without asking
   * user: "they get what it says on the tin".
   */

  if (m_core.IsReadOnly())
    return PWScore::SUCCESS;

  // Note: RUE list saved here via time stamp being updated.
  // Otherwise it won't be saved unless something else has changed
  if ((m_bTSUpdated || m_core.WasDisplayStatusChanged()) &&
       m_core.GetNumEntries() > 0) {
    int rc = Save();
    if (rc != PWScore::SUCCESS)
      return PWScore::USER_CANCEL;
    else
      return PWScore::SUCCESS;
  }

  // offer to save existing database if it was modified.
  // used before loading another
  // returns PWScore::SUCCESS if save succeeded or if user decided
  // not to save
  if (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) {
    CGeneralMsgBox gmb;
    INT_PTR rc, rc2;
    CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, m_core.GetCurFile().c_str());
    rc = gmb.MessageBox(cs_temp, AfxGetAppName(),
                            MB_YESNOCANCEL | MB_ICONQUESTION);
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
        // It is a success but we need to know that the user said no!
        return PWScore::USER_DECLINED_SAVE;
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
  CGeneralMsgBox gmb;
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
    gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
    gmb.AddButton(IDS_CANCEL, IDS_CANCEL, TRUE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == IDS_CANCEL)
      return PWScore::USER_CANCEL;
  }

  //SaveAs-type dialog box
  StringX cf(m_core.GetCurFile());
  if (cf.empty()) {
    CString defname(MAKEINTRESOURCE(IDS_DEFDBNAME)); // reasonable default for first time user
    cf = LPCWSTR(defname);
  }

  std::wstring v3FileName = PWSUtil::GetNewFileName(cf.c_str(), DEFAULT_SUFFIX);
  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  while (1) {
    CPWFileDialog fd(FALSE,
                     DEFAULT_SUFFIX,
                     v3FileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(IDS_FDF_DB_ALL)),
                     this);
    if (m_core.GetCurFile().empty())
      cs_text.LoadString(IDS_NEWNAME1);
    else
      cs_text.LoadString(IDS_NEWNAME2);

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
      newfile = fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }

  std::wstring locker(L""); // null init is important here
  // Note: We have to lock the new file before releasing the old (on success)
  if (!m_core.LockFile2(newfile.c_str(), locker)) {
    cs_temp.Format(IDS_FILEISLOCKED, newfile.c_str(), locker.c_str());
    cs_title.LoadString(IDS_FILELOCKERROR);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  // Save file UUID, clear it to generate new one, restore if necessary
  pws_os::CUUID file_uuid = m_core.GetFileUUID();
  m_core.ClearFileUUID();

  UUIDList RUElist;
  m_RUEList.GetRUEList(RUElist);
  m_core.SetRUEList(RUElist);

  rc = m_core.WriteFile(newfile);
  m_core.ResetStateAfterSave();
  m_core.ClearChangedNodes();

  if (rc != PWScore::SUCCESS) {
    m_core.SetFileUUID(file_uuid); // restore uuid after failed save-as
    m_core.UnlockFile2(newfile.c_str());
    DisplayFileWriteError(rc, newfile);
    return PWScore::CANT_OPEN_FILE;
  }
  if (!m_core.GetCurFile().empty())
    m_core.UnlockFile(m_core.GetCurFile().c_str());

  // Move the newfile lock to the right place
  m_core.MoveLock();

  m_core.SetCurFile(newfile);
#if !defined(POCKET_PC)
  m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCWSTR(m_titlebar));
  app.SetTooltipText(m_core.GetCurFile().c_str());
#endif
  SetChanged(Clear);
  ChangeOkUpdate();

  // Added/Modified entries now saved - reverse it & refresh display
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  if (m_bFilterActive && m_bFilterForStatus) {
    m_ctlItemList.Invalidate();
    m_ctlItemTree.Invalidate();
  }
  RefreshViews();

  app.AddToMRU(newfile.c_str());

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
  CString cs_text, cs_temp;

  //SaveAs-type dialog box
  std::wstring OldFormatFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                      L"dat");
  cs_text.LoadString(IDS_NAMEEXPORTFILE);

  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  while (1) {
    CPWFileDialog fd(FALSE,
                     DEFAULT_SUFFIX,
                     OldFormatFileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(IDS_FDF_DB_ALL)),
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
  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, newfile);
  }
}

void DboxMain::OnExportText()
{
  CGeneralMsgBox gmb;
  StringX sx_temp;
  CString cs_text, cs_temp;

  sx_temp = m_core.GetCurFile();
  if (sx_temp.empty()) {
    //  Database has not been saved - prompt user to do so first!
    gmb.AfxMessageBox(IDS_SAVEBEFOREPROCESS);
    return;
  }

  CWZPropertySheet wizard(ID_MENUITEM_EXPORT2PLAINTEXT,
                          this, WZAdvanced::EXPORT_TEXT,
                          &m_SaveWZAdvValues[WZAdvanced::EXPORT_TEXT]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  wizard.DoModal();
}

void DboxMain::OnExportEntryText()
{
  if (getSelectedItem() == NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTENT2PLAINTEXT,
                          this, WZAdvanced::EXPORT_ENTRYTEXT,
                          &m_SaveWZAdvValues[WZAdvanced::EXPORT_ENTRYTEXT]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  wizard.DoModal();
}

int DboxMain::DoExportText(const StringX &sx_Filename, const bool bAll,
                           const wchar_t &delimiter, const bool bAdvanced, 
                           int &numExported, CReport *prpt)
{
  CGeneralMsgBox gmb;
  OrderedItemList orderedItemList;
  CString cs_temp;

  st_SaveAdvValues *pst_ADV = &m_SaveWZAdvValues[bAll ? WZAdvanced::EXPORT_TEXT : WZAdvanced::EXPORT_ENTRYTEXT];

  CItemData::FieldBits bsAllFields; bsAllFields.set();
  const CItemData::FieldBits bsFields = bAdvanced ? pst_ADV->bsFields : bsAllFields;
  const std::wstring subgroup_name = bAdvanced ? pst_ADV->subgroup_name : L"";
  const int subgroup_object = bAdvanced ? pst_ADV->subgroup_object : CItemData::GROUP;
  const int subgroup_function = bAdvanced ? pst_ADV->subgroup_function : 0;

  std::wstring str_text;
  LoadAString(str_text, IDS_RPTEXPORTTEXT);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  LoadAString(str_text, IDS_TEXT);
  cs_temp.Format(IDS_EXPORTFILE, str_text.c_str(), sx_Filename.c_str());
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();
 
  if (bAll) {
    // Note: MakeOrderedItemList gets its members by walking the 
    // tree therefore, if a filter is active, it will ONLY export
    // those being displayed.
    MakeOrderedItemList(orderedItemList);
  } else {
    // Note: Only selected entry
    CItemData *pci = getSelectedItem();
    orderedItemList.push_back(*pci);
  }

  ReportAdvancedOptions(prpt, bAdvanced, bAll ? WZAdvanced::EXPORT_TEXT : WZAdvanced::EXPORT_ENTRYTEXT);

  // Do the export
  int rc = m_core.WritePlaintextFile(sx_Filename, bsFields, subgroup_name,
                                     subgroup_object, subgroup_function,
                                     delimiter, numExported, &orderedItemList,
                                     prpt);

  orderedItemList.clear(); // cleanup soonest

  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, sx_Filename);
  }

  prpt->EndReport();

  orderedItemList.clear(); // cleanup soonest
  return rc;
}

void DboxMain::OnExportXML()
{
  CGeneralMsgBox gmb;
  StringX sx_temp;
  CString cs_text, cs_temp;

  sx_temp = m_core.GetCurFile();
  if (sx_temp.empty()) {
    //  Database has not been saved - prompt user to do so first!
    gmb.AfxMessageBox(IDS_SAVEBEFOREPROCESS);
    return;
  }

  CWZPropertySheet wizard(ID_MENUITEM_EXPORT2XML,
                          this, WZAdvanced::EXPORT_XML, 
                          &m_SaveWZAdvValues[WZAdvanced::EXPORT_XML]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  wizard.DoModal();
}

void DboxMain::OnExportEntryXML()
{
  if (getSelectedItem() == NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTENT2XML,
                          this, WZAdvanced::EXPORT_ENTRYXML,
                          &m_SaveWZAdvValues[WZAdvanced::EXPORT_ENTRYXML]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  wizard.DoModal();
}

int DboxMain::DoExportXML(const StringX &sx_Filename, const bool bAll,
                          const wchar_t &delimiter, const bool bAdvanced,
                          int &numExported, CReport *prpt)
{
  CGeneralMsgBox gmb;
  OrderedItemList orderedItemList;
  CString cs_temp;
 
  st_SaveAdvValues *pst_ADV = &m_SaveWZAdvValues[bAll ? WZAdvanced::EXPORT_XML : WZAdvanced::EXPORT_ENTRYXML];

  CItemData::FieldBits bsAllFields; bsAllFields.set();
  const CItemData::FieldBits bsFields = bAdvanced ? pst_ADV->bsFields : bsAllFields;
  const std::wstring subgroup_name = bAdvanced ? pst_ADV->subgroup_name : L"";
  const int subgroup_object = bAdvanced ? pst_ADV->subgroup_object : CItemData::GROUP;
  const int subgroup_function = bAdvanced ? pst_ADV->subgroup_function : 0;

  std::wstring str_text;
  LoadAString(str_text, IDS_RPTEXPORTXML);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  LoadAString(str_text, IDS_XML);
  cs_temp.Format(IDS_EXPORTFILE, str_text.c_str(), sx_Filename.c_str());
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();

  if (bAll) {
    // Note: MakeOrderedItemList gets its members by walking the 
    // tree therefore, if a filter is active, it will ONLY export
    // those being displayed.
    MakeOrderedItemList(orderedItemList);
  } else {
    // Note: Only selected entry
    CItemData *pci = getSelectedItem();
    orderedItemList.push_back(*pci);
  }

  ReportAdvancedOptions(prpt, bAdvanced, bAll ? WZAdvanced::EXPORT_XML : WZAdvanced::EXPORT_ENTRYXML);

  // Do the export
  int rc = m_core.WriteXMLFile(sx_Filename, bsFields, subgroup_name,
                               subgroup_object, subgroup_function,
                               delimiter, numExported, &orderedItemList,
                               m_bFilterActive, prpt);

  orderedItemList.clear(); // cleanup soonest

  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, sx_Filename);
  }

  orderedItemList.clear(); // cleanup soonest

  prpt->EndReport();
  return rc;
}

void DboxMain::OnImportText()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CString cs_title, cs_temp;

  CGeneralMsgBox gmb;
  // Initialize set
  GTUSet setGTU;
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_title.LoadString(IDS_TEXTIMPORTFAILED);
    cs_temp.Format(IDS_DBHASDUPLICATES, m_core.GetCurFile().c_str());
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return;
  }

  CImportTextDlg dlg;
  INT_PTR status = dlg.DoModal();

  if (status == IDCANCEL)
    return;

  StringX ImportedPrefix(dlg.m_groupName);
  CString cs_text;
  wchar_t fieldSeparator(dlg.m_Separator[0]);

  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  CPWFileDialog fd(TRUE,
                   L"txt",
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_T_C_ALL)),
                   this);

  cs_text.LoadString(IDS_PICKTEXTFILE);
  fd.m_ofn.lpstrTitle = cs_text;

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }

  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    std::wstring strError;
    StringX TxtFileName = fd.GetPathName();
    int numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numWarnings(0);
    wchar_t delimiter = dlg.m_defimpdelim[0];
    bool bImportPSWDsOnly = dlg.m_bImportPSWDsOnly == TRUE;

    // Create report as we go
    CReport rpt;
    std::wstring str_text;
    LoadAString(str_text, IDS_RPTIMPORTTEXT);
    rpt.StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(str_text, IDS_TEXT);
    cs_temp.Format(IDS_IMPORTFILE, str_text.c_str(), TxtFileName.c_str());
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.WriteLine();

    Command *pcmd = NULL;
    rc = m_core.ImportPlaintextFile(ImportedPrefix, TxtFileName, fieldSeparator,
                                    delimiter, bImportPSWDsOnly,
                                    strError,
                                    numImported, numSkipped, numPWHErrors, numRenamed,
                                    rpt, pcmd);

    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
        cs_title.LoadString(IDS_FILEREADERROR);
        cs_temp.Format(IDS_CANTOPENREADING, TxtFileName.c_str());
        delete pcmd;
        break;
      case PWScore::INVALID_FORMAT:
        cs_title.LoadString(IDS_FILEREADERROR);
        cs_temp.Format(IDS_INVALIDFORMAT, TxtFileName.c_str());
        delete pcmd;
        break;
      case PWScore::FAILURE:
        cs_title.LoadString(IDS_TEXTIMPORTFAILED);
        cs_temp = strError.c_str();
        delete pcmd;
        break;
      case PWScore::SUCCESS:
      case PWScore::OK_WITH_ERRORS:
        // deliberate fallthru
      default:
      {
        if (pcmd != NULL) {
          Execute(pcmd);
          const size_t n = ((MultiCommands *)pcmd)->GetSize();
          for (size_t i = 0; i < n; i++) {
            int iw;
            if (((MultiCommands *)pcmd)->GetRC(i, iw))
              numWarnings += iw;
          }
        }

        rpt.WriteLine();
        CString cs_type;
        cs_type.LoadString(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
        cs_temp.Format(bImportPSWDsOnly ? IDS_RECORDSUPDATED : IDS_RECORDSIMPORTED, 
                       numImported, cs_type);
        rpt.WriteLine((LPCWSTR)cs_temp);

        if (numSkipped != 0) {
          CString cs_tmp;
          cs_type.LoadString(numSkipped == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
          cs_tmp.Format(IDS_RECORDSSKIPPED, numSkipped, cs_type);
          rpt.WriteLine((LPCWSTR)cs_tmp);
          cs_temp += cs_tmp;
        }

        if (numPWHErrors != 0) {
          CString cs_tmp;
          cs_type.LoadString(numPWHErrors == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
          cs_tmp.Format(IDS_RECORDSPWHERRRORS, numPWHErrors, cs_type);
          rpt.WriteLine((LPCWSTR)cs_tmp);
          cs_temp += cs_tmp;
        }

        if (numRenamed != 0) {
          CString cs_tmp;
          cs_type.LoadString(numRenamed == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
          cs_tmp.Format(IDS_RECORDSRENAMED, numRenamed, cs_type);
          rpt.WriteLine((LPCWSTR)cs_tmp);
          cs_temp += cs_tmp;
        }

        if (numWarnings != 0) {
          CString cs_tmp(MAKEINTRESOURCE(IDS_WITHWARNINGS));
          cs_temp += cs_tmp;
        }

        cs_title.LoadString(rc == PWScore::SUCCESS ? IDS_COMPLETE : IDS_OKWITHERRORS);

        ChangeOkUpdate();
        RefreshViews();
        break;
      }
    } // switch
    // Finish Report
    rpt.EndReport();

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_temp);
    gmb.SetStandardIcon(rc == PWScore::SUCCESS ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == IDS_VIEWREPORT)
      ViewReport(rpt);

    // May need to update menu/toolbar if original database was empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);
  }
}

void DboxMain::OnImportKeePassV1CSV()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  UINT uiReasonCode(0);
  CString cs_title, cs_msg;
  cs_title.LoadString(IDS_PICKKEEPASSFILE);
  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  CPWFileDialog fd(TRUE,
                   L"csv",
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_CSV_ALL)),
                   this);

  fd.m_ofn.lpstrTitle = cs_title;

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }

  if (rc == IDOK) {
    CGeneralMsgBox gmb;
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    Command *pcmd = NULL;
    StringX KPsFileName = fd.GetPathName();
    int numImported, numSkipped, numRenamed;

    // Create report as we go
    CReport rpt;
    std::wstring str_text;
    LoadAString(str_text, IDS_RPTIMPORTKPV1CSV);
    rpt.StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(str_text, IDS_TEXT);
    cs_msg.Format(IDS_IMPORTFILE, str_text.c_str(), KPsFileName.c_str());
    rpt.WriteLine((LPCWSTR)cs_msg);
    rpt.WriteLine();

    rc = m_core.ImportKeePassV1CSVFile(KPsFileName, numImported, numSkipped, numRenamed,
                                       uiReasonCode, rpt, pcmd);
    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
      {
        cs_msg.Format(IDS_CANTOPENREADING, KPsFileName.c_str());
        cs_title.LoadString(IDS_FILEOPENERROR);
        delete [] pcmd;
        break;
      }
      case PWScore::INVALID_FORMAT:
      case PWScore::FAILURE:
      {
        if (uiReasonCode > 0)
          cs_msg.LoadString(uiReasonCode);
        else
          cs_msg.Format(IDS_INVALIDFORMAT, KPsFileName.c_str());
        cs_title.LoadString(IDS_IMPORTFAILED);
        delete [] pcmd;
        break;
      }
      case PWScore::SUCCESS:
      default: // deliberate fallthru
        if (pcmd != NULL)
          Execute(pcmd);
        RefreshViews();
        ChangeOkUpdate();
        // May need to update menu/toolbar if original database was empty
        if (bWasEmpty)
          UpdateMenuAndToolBar(m_bOpen);

        rpt.WriteLine();
        CString cs_type;
        cs_type.LoadString(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
        cs_msg.Format(IDS_RECORDSIMPORTED, numImported, cs_type);
        rpt.WriteLine((LPCWSTR)cs_msg);

        cs_title.LoadString(rc == PWScore::SUCCESS ? IDS_COMPLETE : IDS_OKWITHERRORS);
        break;
    } // switch
    rpt.EndReport();

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(rc == PWScore::SUCCESS ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == IDS_VIEWREPORT)
      ViewReport(rpt);
  }
}

void DboxMain::OnImportKeePassV1TXT()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  UINT uiReasonCode(0);
  CString cs_title, cs_msg;
  cs_title.LoadString(IDS_PICKKEEPASSFILE);
  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  CPWFileDialog fd(TRUE,
                   L"txt",
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_TXT_ALL)),
                   this);

  fd.m_ofn.lpstrTitle = cs_title;

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }

  if (rc == IDOK) {
    CGeneralMsgBox gmb;
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    Command *pcmd = NULL;
    StringX KPsFileName = fd.GetPathName();
    int numImported, numSkipped, numRenamed;

    // Create report as we go
    CReport rpt;
    std::wstring str_text;
    LoadAString(str_text, IDS_RPTIMPORTKPV1TXT);
    rpt.StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(str_text, IDS_TEXT);
    cs_msg.Format(IDS_IMPORTFILE, str_text.c_str(), KPsFileName.c_str());
    rpt.WriteLine((LPCWSTR)cs_msg);
    rpt.WriteLine();

    rc = m_core.ImportKeePassV1TXTFile(KPsFileName, numImported, numSkipped, numRenamed,
                                       uiReasonCode, rpt, pcmd);
    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
      {
        cs_msg.Format(IDS_CANTOPENREADING, KPsFileName.c_str());
        cs_title.LoadString(IDS_FILEOPENERROR);
        delete [] pcmd;
        break;
      }
      case PWScore::INVALID_FORMAT:
      {
        if (uiReasonCode > 0)
          cs_msg.LoadString(uiReasonCode);
        else
          cs_msg.Format(IDS_INVALIDFORMAT, KPsFileName.c_str());
        cs_title.LoadString(IDS_IMPORTFAILED);
        delete [] pcmd;
        break;
      }
      case PWScore::SUCCESS:
      default: // deliberate fallthru
        if (pcmd != NULL)
          Execute(pcmd);

        RefreshViews();
        ChangeOkUpdate();
        // May need to update menu/toolbar if original database was empty
        if (bWasEmpty)
          UpdateMenuAndToolBar(m_bOpen);

        rpt.WriteLine();
        CString cs_type;
        cs_type.LoadString(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
        cs_msg.Format(IDS_RECORDSIMPORTED, numImported, cs_type);
        rpt.WriteLine((LPCWSTR)cs_msg);

        cs_title.LoadString(rc == PWScore::SUCCESS ? IDS_COMPLETE : IDS_OKWITHERRORS);

        break;
    } // switch
    // Finish Report
    rpt.EndReport();

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(rc == PWScore::SUCCESS ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == IDS_VIEWREPORT)
      ViewReport(rpt);
  }
}

void DboxMain::OnImportXML()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CString cs_title, cs_temp, cs_text;
  cs_title.LoadString(IDS_XMLIMPORTFAILED);
  cs_text.LoadString(IDS_PICKXMLFILE);

  CGeneralMsgBox gmb;
  // Initialize set
  GTUSet setGTU;
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_temp.Format(IDS_DBHASDUPLICATES, m_core.GetCurFile().c_str());
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return;
  }

  const std::wstring XSDfn(L"pwsafe.xsd");
  std::wstring XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

  if (!pws_os::FileExists(XSDFilename)) {
    CGeneralMsgBox gmb;
    cs_temp.Format(IDSC_MISSINGXSD, XSDfn.c_str());
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }

  CImportXMLDlg dlg;
  INT_PTR status = dlg.DoModal();

  if (status == IDCANCEL)
    return;

  std::wstring ImportedPrefix(dlg.m_groupName);
  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  CPWFileDialog fd(TRUE,
                   L"xml",
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_XML)),
                   this);

  fd.m_ofn.lpstrTitle = cs_text;

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling 
    PostQuitMessage(0);
    return;
  }

  if (rc == IDOK) {
    bool bWasEmpty = m_core.GetNumEntries() == 0;
    std::wstring strXMLErrors, strSkippedList, strPWHErrorList, strRenameList;
    CString XMLFilename = fd.GetPathName();
    int numValidated, numImported, numSkipped, numRenamed, numPWHErrors;
    bool bImportPSWDsOnly = dlg.m_bImportPSWDsOnly == TRUE;

    CWaitCursor waitCursor;  // This may take a while!

    // Create report as we go
    CReport rpt;
    std::wstring str_text;
    LoadAString(str_text, IDS_RPTIMPORTXML);
    rpt.StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(str_text, IDS_XML);
    cs_temp.Format(IDS_IMPORTFILE, str_text.c_str(), XMLFilename);
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.WriteLine();
    std::vector<StringX> vgroups;
    Command *pcmd = NULL;

    rc = m_core.ImportXMLFile(ImportedPrefix, std::wstring(XMLFilename),
                              XSDFilename.c_str(), bImportPSWDsOnly,
                              strXMLErrors, strSkippedList, strPWHErrorList, strRenameList,
                              numValidated, numImported, numSkipped, numPWHErrors, numRenamed,
                              rpt, pcmd);
    waitCursor.Restore();  // Restore normal cursor

    std::wstring csErrors(L"");
    switch (rc) {
      case PWScore::XML_FAILED_VALIDATION:
        rpt.WriteLine(strXMLErrors.c_str());
        cs_temp.Format(IDS_FAILEDXMLVALIDATE, fd.GetFileName(), L"");
        delete pcmd;
        break;
      case PWScore::XML_FAILED_IMPORT:
        rpt.WriteLine(strXMLErrors.c_str());
        cs_temp.Format(IDS_XMLERRORS, fd.GetFileName(), L"");
        delete pcmd;
        break;
      case PWScore::SUCCESS:
      case PWScore::OK_WITH_ERRORS:
        cs_title.LoadString(rc == PWScore::SUCCESS ? IDS_COMPLETE : IDS_OKWITHERRORS);
        if (pcmd != NULL)
          Execute(pcmd);

        if (!strXMLErrors.empty() ||
            numRenamed > 0 || numPWHErrors > 0) {
          if (!strXMLErrors.empty())
            csErrors = strXMLErrors + L"\n";

          if (!csErrors.empty()) {
            rpt.WriteLine(csErrors.c_str());
          }

          CString cs_renamed(L""), cs_PWHErrors(L""), cs_skipped(L"");
          if (numSkipped > 0) {
            cs_skipped.LoadString(IDS_TITLESKIPPED);
            rpt.WriteLine((LPCWSTR)cs_skipped);
            cs_skipped.Format(IDS_XMLIMPORTSKIPPED, numSkipped);
            rpt.WriteLine(strSkippedList.c_str());
            rpt.WriteLine();
          }
          if (numPWHErrors > 0) {
            cs_PWHErrors.LoadString(IDS_TITLEPWHERRORS);
            rpt.WriteLine((LPCWSTR)cs_PWHErrors);
            cs_PWHErrors.Format(IDS_XMLIMPORTPWHERRORS, numPWHErrors);
            rpt.WriteLine(strPWHErrorList.c_str());
            rpt.WriteLine();
          }
          if (numRenamed > 0) {
            cs_renamed.LoadString(IDS_TITLERENAMED);
            rpt.WriteLine((LPCWSTR)cs_renamed);
            cs_renamed.Format(IDS_XMLIMPORTRENAMED, numRenamed);
            rpt.WriteLine(strRenameList.c_str());
            rpt.WriteLine();
          }

          cs_temp.Format(IDS_XMLIMPORTWITHERRORS,
                         fd.GetFileName(), numValidated, numImported,
                         cs_skipped, cs_renamed, cs_PWHErrors);

          ChangeOkUpdate();
        } else {
          const CString cs_validate(MAKEINTRESOURCE(numValidated == 1 ? IDSC_ENTRY : IDSC_ENTRIES));
          const CString cs_imported(MAKEINTRESOURCE(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES));
          cs_temp.Format(IDS_XMLIMPORTOK, numValidated, cs_validate, numImported, cs_imported);
          ChangeOkUpdate();
        }

        RefreshViews();
        break;
      default:
        ASSERT(0);
    } // switch

    // Finish Report
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.EndReport();

    if (rc != PWScore::SUCCESS || !strXMLErrors.empty())
      gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    else
      gmb.SetStandardIcon(MB_ICONINFORMATION);

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_temp);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == IDS_VIEWREPORT)
      ViewReport(rpt);

    // May need to update menu/toolbar if original database was empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);
  }
}

void DboxMain::OnProperties()
{
  st_DBProperties st_dbp;
  m_core.GetDBProperties(st_dbp);

  CProperties dlg(st_dbp);

  dlg.DoModal();
}

void DboxMain::OnChangeMode()	 
{
  // From StatusBar and menu
  const bool bWasRO = IsDBReadOnly();

  if (!bWasRO) {
    // Try to save if any changes done to database
    int rc = SaveIfChanged();
    if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
      return;

    if (rc == PWScore::USER_DECLINED_SAVE) {
	     // But ask just in case	 
       CGeneralMsgBox gmb;	 
       CString cs_msg(MAKEINTRESOURCE(IDS_BACKOUT_CHANGES)), cs_title(MAKEINTRESOURCE(IDS_CHANGEMODE));	 
       INT_PTR rc = gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONQUESTION);	 
 	 
       if (rc == IDNO)	 
         return;

      // User said No to the save - so we must back-out all changes since last save
      while (m_core.IsChanged()) {
        OnUndo();
      }
    }
 
    // Reset changed flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
    SetChanged(Clear);

    // Clear the Commands
    m_core.ClearCommands();
  }

  CGeneralMsgBox gmb;
  CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_CHANGEMODE_FAILED));
  std::wstring locker = L"";
  int iErrorCode;
  bool brc = m_core.ChangeMode(locker, iErrorCode);
  if (brc) {
    UpdateStatusBar();
  } else {
    // Better give them the bad news!
    bool bInUse = false;
    UINT uiMsg = 0;
    if (bWasRO) {
      switch (iErrorCode) {
        case PWScore::DB_HAS_CHANGED:
          // We did get the lock but the DB has been changed
          // Note: PWScore has already freed the lock
          // The user must close and re-open it in R/W mode
          uiMsg = IDS_CM_FAIL_REASON3;
          break;
        
        case PWScore::CANT_GET_LOCK:
        {
          CString cs_user_and_host, cs_PID;
          cs_user_and_host = (CString)locker.c_str();
          int i_pid = cs_user_and_host.ReverseFind(L':');
          if (i_pid > -1) {
            // If PID present then it is ":%08d" = 9 chars in length
            ASSERT((cs_user_and_host.GetLength() - i_pid) == 9);
            cs_PID.Format(IDS_PROCESSID, cs_user_and_host.Right(8));
            cs_user_and_host = cs_user_and_host.Left(i_pid);
          } else {
            cs_PID = L"";
          }

          cs_msg.Format(IDS_CM_FAIL_REASON1, cs_user_and_host, cs_PID);
          bInUse = true;
          break;
        }
        case PWSfile::CANT_OPEN_FILE:
          uiMsg = IDS_CM_FAIL_REASON4;
          break;
        case PWSfile::END_OF_FILE:
          uiMsg = IDS_CM_FAIL_REASON5;
          break;
        default:
          ASSERT(0);
      }
    } else {
      // Don't need fail code when going from R/W to R-O - only one issue -
      // could not release the lock!
      uiMsg = IDS_CM_FAIL_REASON2;
    }

    if (bInUse) {
      // Big message
      gmb.SetTitle(cs_title);
      gmb.SetMsg(cs_msg);
      gmb.SetStandardIcon(MB_ICONWARNING);
      gmb.AddButton(IDS_CLOSE, IDS_CLOSE);
      gmb.DoModal();
    } else {
      cs_msg.LoadString(uiMsg);
      gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONWARNING);
    }
  }
}

void DboxMain::OnCompare()
{
  if (m_core.GetCurFile().empty() || m_core.GetNumEntries() == 0) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_NOCOMPAREFILE, MB_OK | MB_ICONWARNING);
    return;
  }

  CWZPropertySheet wizard(ID_MENUITEM_COMPARE,
                          this, WZAdvanced::COMPARE,
                          &m_SaveWZAdvValues[WZAdvanced::COMPARE]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  wizard.DoModal();
}

void DboxMain::OnMerge()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CWZPropertySheet wizard(ID_MENUITEM_MERGE,
                          this, WZAdvanced::MERGE, 
                          &m_SaveWZAdvValues[WZAdvanced::MERGE]);

  INT_PTR rc = wizard.DoModal();

  if (rc == ID_WIZFINISH)
    UpdateToolBarDoUndo();
}

void DboxMain::OnSynchronize()
{
  // disable in read-only mode or empty
  if (m_core.IsReadOnly() || m_core.GetCurFile().empty() || m_core.GetNumEntries() == 0)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_SYNCHRONIZE,
                          this, WZAdvanced::SYNCH,
                          &m_SaveWZAdvValues[WZAdvanced::SYNCH]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  INT_PTR rc = wizard.DoModal();

  if (rc == ID_WIZFINISH && wizard.GetNumProcessed() > 0)
    SetChanged(Data);
}

stringT DboxMain::DoMerge(PWScore *pothercore,
                          const bool bAdvanced, CReport *prpt)
{
  CGeneralMsgBox gmb;
  CString cs_title, cs_temp,cs_text;
  // Initialize set
  GTUSet setGTU;

  // First check other database
  if (!pothercore->GetUniqueGTUValidated() && !pothercore->InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_title.LoadString(IDS_MERGEFAILED);
    cs_temp.Format(IDS_DBHASDUPLICATES, pothercore->GetCurFile().c_str());
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return L"";
  }

  // Next check us - we need the setGTU later
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_title.LoadString(IDS_MERGEFAILED);
    cs_temp.Format(IDS_DBHASDUPLICATES, m_core.GetCurFile().c_str());
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return L"";
  }

  // Create report as we go
  std::wstring str_text;
  LoadAString(str_text, IDS_RPTMERGE);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  cs_temp.Format(IDS_MERGINGDATABASE, pothercore->GetCurFile().c_str());
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();
 
  std::vector<StringX> vs_added;
  std::vector<StringX> vs_AliasesAdded;
  std::vector<StringX> vs_ShortcutsAdded;

  CItemData::FieldBits bsFields;
  std::wstring subgroup_name;
  int subgroup_object, subgroup_function;
  bool subgroup_bset;

   if (bAdvanced == TRUE) {
    // Use saved or latest values
    subgroup_name = m_SaveWZAdvValues[WZAdvanced::MERGE].subgroup_name;
    subgroup_bset = m_SaveWZAdvValues[WZAdvanced::MERGE].subgroup_bset;
    subgroup_object = m_SaveWZAdvValues[WZAdvanced::MERGE].subgroup_object;
    subgroup_function = m_SaveWZAdvValues[WZAdvanced::MERGE].subgroup_function;
  } else {
    // Turn off advanced settings
    subgroup_name = L"";
    subgroup_bset = false;
    subgroup_object = CItemData::GROUP;
    subgroup_function = 0;
  }

  ReportAdvancedOptions(prpt, bAdvanced, WZAdvanced::MERGE);
 
  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  // Do the Merge
  std::wstring str_result = m_core.Merge(pothercore, subgroup_bset, 
             subgroup_name, subgroup_object, subgroup_function, prpt);

  // restore normal cursor
  waitCursor.Restore();

  prpt->EndReport();

  return str_result;
}

bool DboxMain::DoCompare(PWScore *pothercore,
                         const bool bAdvanced, CReport *prpt)
{
  CString cs_temp, cs_text, cs_buffer;

  m_list_OnlyInCurrent.clear();
  m_list_OnlyInComp.clear();
  m_list_Conflicts.clear();
  m_list_Identical.clear();

  // Create report as we go
  std::wstring str_text;
  LoadAString(str_text, IDS_RPTCOMPARE);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  cs_temp.Format(IDS_COMPARINGDATABASE, pothercore->GetCurFile().c_str());
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();

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

  CItemData::FieldBits bsFields;
  std::wstring subgroup_name;
  int subgroup_object, subgroup_function;
  bool subgroup_bset, bTreatWhiteSpaceasEmpty;

  if (bAdvanced) {
    // Use saved or latest values
    bsFields = m_SaveWZAdvValues[WZAdvanced::COMPARE].bsFields;
    subgroup_name = m_SaveWZAdvValues[WZAdvanced::COMPARE].subgroup_name;
    subgroup_bset = m_SaveWZAdvValues[WZAdvanced::COMPARE].subgroup_bset;
    subgroup_object = m_SaveWZAdvValues[WZAdvanced::COMPARE].subgroup_object;
    subgroup_function = m_SaveWZAdvValues[WZAdvanced::COMPARE].subgroup_function;
    bTreatWhiteSpaceasEmpty = m_SaveWZAdvValues[WZAdvanced::COMPARE].btreatwhitespaceasempty;
  } else {
    // Turn off advanced settings
    subgroup_name = L"";
    subgroup_bset = false;
    subgroup_object = CItemData::GROUP;
    subgroup_function = 0;
    bTreatWhiteSpaceasEmpty = false;

    // Set on all fields
    bsFields.set();
    // Turn off time fields if not explicitly turned on by user via Advanced dialog
    bsFields.reset(CItemData::CTIME);
    bsFields.reset(CItemData::PMTIME);
    bsFields.reset(CItemData::ATIME);
    bsFields.reset(CItemData::XTIME);
    bsFields.reset(CItemData::RMTIME);
  }

  ReportAdvancedOptions(prpt, bAdvanced, WZAdvanced::COMPARE);

  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  m_core.Compare(pothercore,
                 bsFields, subgroup_bset, bTreatWhiteSpaceasEmpty,
                 subgroup_name, subgroup_object, 
                 subgroup_function,
                 m_list_OnlyInCurrent, m_list_OnlyInComp,
                 m_list_Conflicts, m_list_Identical);

  // restore normal cursor
  waitCursor.Restore();

  cs_buffer.Format(IDS_COMPARESTATISTICS,
                m_core.GetCurFile().c_str(), pothercore->GetCurFile().c_str());

  bool brc(true);  // True == databases are identical
  if (m_list_OnlyInCurrent.size() == 0 &&
      m_list_OnlyInComp.size() == 0 &&
      m_list_Conflicts.size() == 0) {
    m_list_Identical.clear();
    cs_text.LoadString(IDS_IDENTICALDATABASES);
    cs_buffer += cs_text;
    prpt->WriteLine((LPCWSTR)cs_buffer);
    prpt->EndReport();
  } else {
    prpt->WriteLine((LPCWSTR)cs_buffer);
    brc = false;
  }

  return brc;
}

CString DboxMain::ShowCompareResults(const StringX sx_Filename1, const StringX sx_Filename2,
                                     PWScore *pothercore, CReport *prpt)
{
  // Can't do UI from a worker thread!
  CCompareResultsDlg CmpRes(this, m_list_OnlyInCurrent, m_list_OnlyInComp, 
                            m_list_Conflicts, m_list_Identical, 
                            m_bsFields, &m_core, pothercore, prpt);

  CmpRes.m_scFilename1 = sx_Filename1;
  CmpRes.m_scFilename2 = sx_Filename2;
  CmpRes.m_bOriginalDBReadOnly = m_core.IsReadOnly();
  CmpRes.m_bComparisonDBReadOnly = pothercore->IsReadOnly();

  CmpRes.DoModal();

  if (CmpRes.m_OriginalDBChanged) {
    FixListIndexes();
    RefreshViews();
  }

  m_list_OnlyInCurrent.clear();
  m_list_OnlyInComp.clear();
  m_list_Conflicts.clear();
  m_list_Identical.clear();

  CString cs_results = CmpRes.GetResults();

  return cs_results;
}

void DboxMain::DoSynchronize(PWScore *pothercore,
                             const bool bAdvanced, int &numUpdated, CReport *prpt)
{
  numUpdated = 0;

  CGeneralMsgBox gmb;
  CString str_temp, str_title, str_buffer;
  // Initialize set
  GTUSet setGTU;

  // First check other database
  if (!pothercore->GetUniqueGTUValidated() && !pothercore->InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    str_title.LoadString(IDS_SYNCHFAILED);
    str_temp.Format(IDS_DBHASDUPLICATES, pothercore->GetCurFile().c_str());
    gmb.MessageBox(str_temp, str_title, MB_ICONEXCLAMATION);
    return;
  }

  // Next check us
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    str_title.LoadString(IDS_SYNCHFAILED);
    str_temp.Format(IDS_DBHASDUPLICATES, m_core.GetCurFile().c_str());
    gmb.MessageBox(str_temp, str_title, MB_ICONEXCLAMATION);
    return;
  }

  setGTU.clear();  // Don't need it anymore - so clear it now

  CItemData::FieldBits bsFields;
  std::wstring subgroup_name;
  int subgroup_object, subgroup_function;
  bool subgroup_bset;

  if (bAdvanced) {
    // Use saved or latest values
    bsFields = m_SaveWZAdvValues[WZAdvanced::SYNCH].bsFields;
    subgroup_name = m_SaveWZAdvValues[WZAdvanced::SYNCH].subgroup_name;
    subgroup_bset = m_SaveWZAdvValues[WZAdvanced::SYNCH].subgroup_bset;
    subgroup_object = m_SaveWZAdvValues[WZAdvanced::SYNCH].subgroup_object;
    subgroup_function = m_SaveWZAdvValues[WZAdvanced::SYNCH].subgroup_function;
  } else {
    // Turn off advanced settings
    subgroup_name = L"";
    subgroup_bset = false;
    subgroup_object = CItemData::GROUP;
    subgroup_function = 0;

    // Set on all fields
    bsFields.set();
    // Turn off
    bsFields.reset(CItemData::NAME);
    bsFields.reset(CItemData::UUID);
    bsFields.reset(CItemData::GROUP);
    bsFields.reset(CItemData::TITLE);
    bsFields.reset(CItemData::USER);
    bsFields.reset(CItemData::RESERVED);
  }

  // Create report as we go
  std::wstring str_text;
  LoadAString(str_text, IDS_RPTSYNCH);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  str_temp.Format(IDS_SYNCHINGDATABASE, pothercore->GetCurFile().c_str());
  prpt->WriteLine((LPCWSTR)str_temp);
  prpt->WriteLine();
  std::vector<StringX> vs_updated;

  ReportAdvancedOptions(prpt, bAdvanced, WZAdvanced::SYNCH);

  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  // Do the Synchronize
  m_core.Synchronize(pothercore, bsFields, subgroup_bset,
                     subgroup_name, subgroup_object, subgroup_function,
                     numUpdated, prpt);

  // Restore normal cursor
  waitCursor.Restore();

  prpt->EndReport();
}

LRESULT DboxMain::OnEditExpiredPasswordEntry(WPARAM wParam, LPARAM )
{
  st_ExpLocalListEntry *pELLE = (st_ExpLocalListEntry *)wParam;

  ItemListIter iter = Find(pELLE->uuid);
  ASSERT(iter != End());
  if (iter == End())
    return FALSE;

  CItemData *pci = &iter->second;
  ASSERT(pci != NULL);

  // Edit the correct entry
  if (EditItem(pci)) {
    // pci is now invalid after EditItem - find the new one!
    iter = Find(pELLE->uuid);
    ASSERT(iter != End());
    pci = &iter->second;
    ASSERT(pci != NULL);

    // User may have changed these!
    pELLE->sx_group = pci->GetGroup();
    pELLE->sx_title = pci->GetTitle();
    pELLE->sx_user  = pci->GetUser();
    if (pci->IsProtected())
      pELLE->sx_title += L" #";

    // Update time fields
    time_t tttXTime;
    pci->GetXTime(tttXTime);
    // If value is >0 & <=3650, this corresponds to expiry interval in days (< 10 years)
    if (tttXTime > time_t(0) && tttXTime <= time_t(3650)) {
      time_t tttCPMTime;
      pci->GetPMTime(tttCPMTime);
      if ((long)tttCPMTime == 0L)
        pci->GetCTime(tttCPMTime);
      tttXTime = (time_t)((long)tttCPMTime + (long)tttXTime * 86400);
    }
    pELLE->expirytttXTime = tttXTime;
    pELLE->sx_expirylocdate = PWSUtil::ConvertToDateTimeString(tttXTime, TMC_LOCALE);

    return TRUE;
  }

  return FALSE;
}

LRESULT DboxMain::OnProcessCompareResultFunction(WPARAM wParam, LPARAM lFunction)
{
  PWScore *pcore;
  st_CompareInfo *st_info;
  LRESULT lres(FALSE);
  CUUID entryUUID;

  st_info = (st_CompareInfo *)wParam;

  if (st_info->clicked_column == CCompareResultsDlg::CURRENT) {
    pcore = st_info->pcore0;
    entryUUID = st_info->uuid0;
  } else {
    pcore = st_info->pcore1;
    entryUUID = st_info->uuid1;
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
    case CCompareResultsDlg::SYNCH:
      lres = SynchCompareResult(st_info->pcore1, st_info->pcore0,
                                st_info->uuid1, st_info->uuid0);
      break;
    default:
      ASSERT(0);
  }
  return lres;
}

LRESULT DboxMain::ViewCompareResult(PWScore *pcore, const CUUID &entryUUID)
{  
  ItemListIter pos = pcore->Find(entryUUID);
  ASSERT(pos != pcore->GetEntryEndIter());
  CItemData *pci = &pos->second;

  // View the correct entry and make sure R-O
  bool bSaveRO = pcore->IsReadOnly();
  pcore->SetReadOnly(true);

  EditItem(pci, pcore);

  pcore->SetReadOnly(bSaveRO);

  return FALSE;
}

LRESULT DboxMain::EditCompareResult(PWScore *pcore, const CUUID &entryUUID)
{
  ItemListIter pos = pcore->Find(entryUUID);
  ASSERT(pos != pcore->GetEntryEndIter());
  CItemData *pci = &pos->second;

  // Edit the correct entry
  return EditItem(pci, pcore) ? TRUE : FALSE;
}

LRESULT DboxMain::CopyCompareResult(PWScore *pfromcore, PWScore *ptocore,
                                    const CUUID &fromUUID, const CUUID &toUUID)
{
  bool bWasEmpty = ptocore->GetNumEntries() == 0;

  // Copy *pfromcore -> *ptocore entry
  ItemListIter fromPos = pfromcore->Find(fromUUID);
  ASSERT(fromPos != pfromcore->GetEntryEndIter());
  const CItemData *pfromEntry = &fromPos->second;
  CItemData ci_temp(*pfromEntry);  // Set up copy

  DisplayInfo *pdi = new DisplayInfo;
  ci_temp.SetDisplayInfo(pdi); // DisplayInfo values will be set later

  // If the UUID is not in use, copy it too, otherwise reuse current
  if (ptocore->Find(fromUUID) == ptocore->GetEntryEndIter())
    ci_temp.SetUUID(fromUUID);
  else
    ci_temp.SetUUID(toUUID);

  Command *pcmd(NULL);
  
  // Is it already there:?
  const StringX sxgroup(ci_temp.GetGroup()), sxtitle(ci_temp.GetTitle()),
    sxuser(ci_temp.GetUser());
  ItemListIter toPos = ptocore->Find(sxgroup, sxtitle, sxuser);

  if (toPos != ptocore->GetEntryEndIter()) {
    // Already there - change it
    CItemData *ptoEntry = &toPos->second;
    ci_temp.SetStatus(CItemData::ES_MODIFIED);
    pcmd = EditEntryCommand::Create(ptocore, *ptoEntry, ci_temp);
  } else {
    // Not there - add it
    ci_temp.SetStatus(CItemData::ES_ADDED);
    pcmd = AddEntryCommand::Create(ptocore, ci_temp);
  }
  Execute(pcmd, ptocore);

  if (ptocore == &m_core) {
    SetChanged(Data);
    ChangeOkUpdate();
    // May need to update menu/toolbar if database was previously empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);

    CItemData *pci = GetLastSelected();
    UpdateToolBarForSelectedItem(pci);
  }

  return TRUE;
}

LRESULT DboxMain::SynchCompareResult(PWScore *pfromcore, PWScore *ptocore,
                                     const CUUID &fromUUID, const CUUID &toUUID)
{
  // Synch 1 entry *pfromcore -> *ptocore
  CItemData::FieldBits bsFields;

  // Use a cut down Advanced dialog (only fields to synchronize)
  CAdvancedDlg Adv(this, CAdvancedDlg::COMPARESYNCH,
                   &m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH]);

  INT_PTR rc = Adv.DoModal();

  if (rc != IDOK)
    return FALSE;

  ItemListIter fromPos = pfromcore->Find(fromUUID);
  ASSERT(fromPos != pfromcore->GetEntryEndIter());
  const CItemData *pfromEntry = &fromPos->second;

  ItemListIter toPos = ptocore->Find(toUUID);
  ASSERT(toPos != ptocore->GetEntryEndIter());
  CItemData *ptoEntry = &toPos->second;
  CItemData updtEntry(*ptoEntry);

  bool bUpdated(false);
  for (size_t i = 0; i < bsFields.size(); i++) {
    if (m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH].bsFields.test(i)) {
      const StringX sxValue = pfromEntry->GetFieldValue((CItemData::FieldType)i);
      if (sxValue != updtEntry.GetFieldValue((CItemData::FieldType)i)) {
        bUpdated = true;
        updtEntry.SetFieldValue((CItemData::FieldType)i, sxValue);
      }
    }
  }

  if (bUpdated) {
    updtEntry.SetStatus(CItemData::ES_MODIFIED);
    Command *pcmd = EditEntryCommand::Create(ptocore, *ptoEntry, updtEntry);
    Execute(pcmd, ptocore);
    return TRUE;
  }

  return FALSE;
}

void DboxMain::OnOK() 
{
  SavePreferencesOnExit();

  int rc = SaveDatabaseOnExit(ST_NORMALEXIT);
  if (rc == PWScore::SUCCESS) {
    CleanUpAndExit();
  }
}

void RelativizePath(stringT &curfile)
{
  // If  IsUnderPw2go() && exec's drive == curfile's drive, remove
  // from latter's path. This supports DoK usage
  if (SysInfo::IsUnderPw2go()) {
    const stringT execDir = pws_os::getexecdir();
    stringT execDrive, dontCare;
    pws_os::splitpath(execDir, execDrive, dontCare, dontCare, dontCare);
    stringT fileDrive, fileDir, fileFile, fileExt;
    pws_os::splitpath(curfile, fileDrive, fileDir, fileFile, fileExt);
    ToUpper(fileDrive); ToUpper(execDrive);
    if (fileDrive == execDrive) {
      curfile = pws_os::makepath(L"", fileDir, fileFile, fileExt);
    }
  }
}

void DboxMain::SavePreferencesOnExit()
{
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

  CString cs_columns(L"");
  CString cs_columnswidths(L"");
  wchar_t wc_buffer[8], widths[8];

  for (int iOrder = 0; iOrder < m_nColumns; iOrder++) {
    int iIndex = m_nColumnIndexByOrder[iOrder];
#if (_MSC_VER >= 1400)
    _itow_s(m_nColumnTypeByIndex[iIndex], wc_buffer, 8, 10);
    _itow_s(m_nColumnWidthByIndex[iIndex], widths, 8, 10);
#else
    _itow(m_nColumnTypeByIndex[iIndex], wc_buffer, 10);
    _itow(m_nColumnWidthByIndex[iIndex], widths, 10);
#endif
    cs_columns += wc_buffer;
    cs_columnswidths += widths;
    cs_columns += L",";
    cs_columnswidths += L",";
  }

  prefs->SetPref(PWSprefs::SortedColumn, m_iTypeSortColumn);
  prefs->SetPref(PWSprefs::SortAscending, m_bSortAscending);
  prefs->SetPref(PWSprefs::ListColumns, LPCWSTR(cs_columns));
  prefs->SetPref(PWSprefs::ColumnWidths, LPCWSTR(cs_columnswidths));

  SaveGroupDisplayState(); // since it's not always up to date
  // (CPWTreeCtrl::OnExpandCollapse not always called!)

  //Store current filename for next time...
  if (prefs->GetPref(PWSprefs::MaxMRUItems) == 0) {
    // Ensure Application preferences have been changed for a rewrite
    prefs->SetPref(PWSprefs::CurrentFile, L"");
    prefs->SetPref(PWSprefs::CurrentBackup, L"");
    prefs->ForceWriteApplicationPreferences();

    // Naughty Windows saves information in the registry for every Open and Save!
    RegistryAnonymity();
  } else
    if (!m_core.GetCurFile().empty()) {
      stringT curFile = m_core.GetCurFile().c_str();
      RelativizePath(curFile);
      prefs->SetPref(PWSprefs::CurrentFile, curFile.c_str());
    }
  // Now save the Find Toolbar display status
  prefs->SetPref(PWSprefs::ShowFindToolBarOnOpen, m_FindToolBar.IsVisible() == TRUE);

  prefs->SaveApplicationPreferences();
  prefs->SaveShortcuts();
}

int DboxMain::SaveDatabaseOnExit(const SaveType saveType)
{
  INT_PTR rc;

  if (saveType == ST_FAILSAFESAVE &&
      (m_core.IsChanged() || m_core.HaveDBPrefsChanged())) {
    // Save database as "<dbname>_YYYYMMDD_HHMMSS.fbak"
    std::wstring cs_newfile, cs_temp;
    std::wstring drv, dir, name, ext;
    const std::wstring path = m_core.GetCurFile().c_str();
    pws_os::splitpath(path.c_str(), drv, dir, name, ext);
    cs_temp = drv + dir + name;
    cs_temp += L"_";
    time_t now;
    time(&now);
    StringX cs_datetime = PWSUtil::ConvertToDateTimeString(now, TMC_EXPORT_IMPORT);
    StringX nf = cs_temp.c_str() + 
                     cs_datetime.substr( 0, 4) +  // YYYY
                     cs_datetime.substr( 5, 2) +  // MM
                     cs_datetime.substr( 8, 2) +  // DD
                     StringX(_T("_")) +
                     cs_datetime.substr(11, 2) +  // HH
                     cs_datetime.substr(14, 2) +  // MM
                     cs_datetime.substr(17, 2);   // SS
    cs_newfile = nf.c_str();
    cs_newfile += L".fbak";
    rc = m_core.WriteFile(cs_newfile.c_str());
    return (int)rc;
  }

  if (saveType == ST_NORMALEXIT) {
    bool bAutoSave = true; // false if user saved or decided not to 
    if (m_core.IsChanged() || m_core.HaveDBPrefsChanged()) {
      CGeneralMsgBox gmb;
      CString cs_msg(MAKEINTRESOURCE(IDS_SAVEFIRST));
      rc = gmb.MessageBox(cs_msg, AfxGetAppName(), 
                        MB_YESNOCANCEL | MB_ICONQUESTION);

      switch (rc) {
        case IDCANCEL:
          return PWScore::USER_CANCEL;
        case IDYES:
          rc = Save(saveType);
          if (rc != PWScore::SUCCESS)
            return PWScore::USER_CANCEL;
          // Drop through to reset bAutoSave to prevent multiple saves
        case IDNO:
          bAutoSave = false;
          break;
      }
    } // core.IsChanged()

    /*
    * Save silently (without asking user) iff:
    * 0. User didn't explicitly save OR say that she doesn't want to AND
    * 1. NOT read-only AND
    * 2. (timestamp updates OR tree view display vector changed) AND
    * 3. Database NOT empty
    *
    * Less formally:
    *
    * If MaintainDateTimeStamps set and not read-only, save without asking
    * user: "they get what it says on the tin".
    * Note: that if database was cleared (e.g., locked), it might be possible
    * to save an empty list :-(
    * Protect against this both here and in OnSize (where we minimize & possibly
    * ClearData).
    */

    if (bAutoSave && !m_core.IsReadOnly() &&
        (m_bTSUpdated || m_core.WasDisplayStatusChanged()) &&
        m_core.GetNumEntries() > 0) {
      rc = Save(saveType);
      if (rc != PWScore::SUCCESS)
        return PWScore::USER_CANCEL;
    }
    return PWScore::SUCCESS;
  } // ST_NORMALEXIT
  
  if (saveType == ST_ENDSESSIONEXIT || saveType == ST_WTSLOGOFFEXIT) {
    // ST_ENDSESSIONEXIT: Windows XP or earlier
    // ST_WTSLOGOFFEXIT:  Windows XP or later (if OnQueryEndSession not called)
    if (!m_core.IsReadOnly() && m_core.GetNumEntries() > 0) {
      rc = Save(saveType);
      if (rc != PWScore::SUCCESS)
        return PWScore::USER_CANCEL;
    }
  } // ST_ENDSESSIONEXIT || ST_WTSLOGOFFEXIT

  return PWScore::SUCCESS;
}

void DboxMain::CleanUpAndExit(const bool bNormalExit)
{
  // Clear clipboard on Exit?  Yes if:
  // a. the app is minimized and the systemtray is enabled
  // b. the user has set the "ClearClipboardOnExit" pref
  // c. the system is shutting down, restarting or the user is logging off
  PWSprefs *prefs = PWSprefs::GetInstance();
  if ((!IsWindowVisible() && prefs->GetPref(PWSprefs::UseSystemTray)) ||
      prefs->GetPref(PWSprefs::ClearClipboardOnExit)) {
    ClearClipboardData();
  }

  // wipe data, save prefs, go home.
  ClearData();

  // Cleanup here - doesn't work in ~DboxMain or ~CCoolMenuManager
  m_menuManager.Cleanup();

  // Clear out filters
  m_MapFilters.clear();

  // If we are called normally, then exit gracefully. If not, force the issue
  // after the caller has processed the current message by posting another message
  // for later (PostQuitMessage).
  if (bNormalExit)
    CDialog::OnOK();
  else
    PostQuitMessage(0);
}

void DboxMain::OnCancel()
{
  // If system tray is enabled, cancel (escape) 
  // minimizes to the system tray, else exit application
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray)) {
    ShowWindow(SW_MINIMIZE);
  } else {
    SavePreferencesOnExit();
    int rc = SaveDatabaseOnExit(ST_NORMALEXIT);
    if (rc == PWScore::SUCCESS) {
      CleanUpAndExit();
    }
  }
}

void DboxMain::RegistryAnonymity()
{
  // For the paranoid - definitely remove information from Registry of previous
  // directory containing PWS databases!
  // Certainly for WinXP but should do no harm on other versions.
  const CString csSubkey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32";

  HKEY hSubkey;
  LONG dw;

  // First deal with information saved by Windows Common Dialog for Open/Save of
  // the file types used by PWS in its CFileDialog
  dw = RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey + L"\\OpenSaveMRU", NULL,
                    KEY_ALL_ACCESS, &hSubkey);

  if (dw == ERROR_SUCCESS) {
    // Delete entries relating to PWS
    app.DelRegTree(hSubkey, L"psafe3");
    app.DelRegTree(hSubkey, L"ibak");
    app.DelRegTree(hSubkey, L"bak");
    app.DelRegTree(hSubkey, L"*");

    dw = RegCloseKey(hSubkey);
    ASSERT(dw == ERROR_SUCCESS);
  }

  // Now deal with Windows remembering the last directory visited by PWS
  dw = RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey + L"\\LastVisitedMRU", NULL,
                      KEY_ALL_ACCESS, &hSubkey);

  if (dw == ERROR_SUCCESS) {
    CString cs_AppName;
    wchar_t szMRUList[_MAX_PATH], szAppNameAndDir[_MAX_PATH * 2];
    wchar_t szMRUListMember[2];
    DWORD dwMRUListLength, dwAppNameAndDirLength, dwType(0);
    int iNumberOfMRU, iIndex;
    dwMRUListLength = sizeof(szMRUList);

    // Get the MRU List
    dw = RegQueryValueEx(hSubkey, L"MRUList", NULL,
                         &dwType, (LPBYTE)szMRUList, &dwMRUListLength);
    if (dw == ERROR_SUCCESS) {
      iNumberOfMRU = dwMRUListLength / sizeof(wchar_t);

      // Search the MRU List
      szMRUListMember[1] = L'\0';
      for (iIndex = 0; iIndex < iNumberOfMRU; iIndex++) {
        szMRUListMember[0] = szMRUList[iIndex];

        dwAppNameAndDirLength = sizeof(szAppNameAndDir);
        // Note: these Registry entries are stored in RG_BINARY format as 2 concatenated
        // Unicode null terminated strings: L"application" L"Last fully qualified Directory"
        dw = RegQueryValueEx(hSubkey, szMRUListMember, 0, &dwType,
                             (LPBYTE)szAppNameAndDir, &dwAppNameAndDirLength);
        if (dw == ERROR_SUCCESS) {
          cs_AppName = szAppNameAndDir;
          if (cs_AppName.MakeLower() == L"pwsafe.exe") {
            dw = RegDeleteValue(hSubkey, szMRUListMember);
            if (dw == ERROR_SUCCESS) {
              // Remove deleted entry from MRU List and rewrite it
              CString cs_NewMRUList(szMRUList);
              iNumberOfMRU = cs_NewMRUList.Delete(iIndex, 1);
              LPWSTR pszNewMRUList = cs_NewMRUList.GetBuffer(iNumberOfMRU);
              dw = RegSetValueEx(hSubkey, L"MRUList", 0, REG_SZ, (LPBYTE)pszNewMRUList,
                            (iNumberOfMRU + 1) * sizeof(wchar_t));
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

void DboxMain::ReportAdvancedOptions(CReport *pRpt, const bool bAdvanced, const WZAdvanced::AdvType type)
{
  CString cs_buffer, cs_temp, cs_text;
  UINT uimsgftn(0);
  switch (type) {
    case WZAdvanced::COMPARE:
      uimsgftn = IDS_RPTCOMPARE;
      break;
    case WZAdvanced::MERGE:
      uimsgftn = IDS_RPTMERGE;
      break;
    case WZAdvanced::SYNCH:
      uimsgftn = IDS_RPTSYNCH;
      break;
    case WZAdvanced::EXPORT_TEXT:
      uimsgftn = IDS_RPTEXPORTTEXT;
      break;
    case WZAdvanced::EXPORT_ENTRYTEXT:
      uimsgftn = IDS_RPTEXPORTTEXT;
      break;
    case WZAdvanced::EXPORT_XML:
      uimsgftn = IDS_RPTEXPORTXML;
      break;
    case WZAdvanced::EXPORT_ENTRYXML:
      uimsgftn = IDS_RPTEXPORTXML;
      break;
    default:
      ASSERT(0);
      return;
  }

  // Get appropriate saved search values
  st_SaveAdvValues &st_SADV = m_SaveWZAdvValues[type];

  // tell the user we're done & provide short Compare report
  if (bAdvanced == FALSE) {
    cs_temp.LoadString(IDS_NONE);
    cs_buffer.Format(IDS_ADVANCEDOPTIONS, cs_temp);
    pRpt->WriteLine((LPCWSTR)cs_buffer);
    pRpt->WriteLine();
  } else {
    if (!st_SADV.subgroup_bset) {
      cs_temp.LoadString(IDS_NONE);
    } else {
      CString cs_Object, cs_case;
      UINT uistring(IDS_NONE);

      switch(st_SADV.subgroup_object) {
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

      cs_case.LoadString(st_SADV.subgroup_function > 0 ? IDS_ADVCASE_INSENSITIVE : IDS_ADVCASE_SENSITIVE);

      uistring = PWSMatch::GetRule(PWSMatch::MatchRule(abs(st_SADV.subgroup_function)));

      cs_text.LoadString(uistring);
      cs_temp.Format(IDS_ADVANCEDSUBSET, cs_Object, cs_text, st_SADV.subgroup_name.c_str(),
                     cs_case);
    }

    if (type == WZAdvanced::MERGE)
      return;

    cs_buffer.Format(IDS_ADVANCEDOPTIONS, cs_temp);
    pRpt->WriteLine((LPCWSTR)cs_buffer);
    pRpt->WriteLine();

    cs_temp.LoadString(uimsgftn);
    cs_buffer.Format(IDS_ADVANCEDFIELDS, cs_temp);
    pRpt->WriteLine((LPCWSTR)cs_buffer);

    cs_buffer = L"\t";
    // Non-time fields
    int ifields[] = {CItemData::PASSWORD, CItemData::NOTES, CItemData::URL,
                     CItemData::AUTOTYPE, CItemData::PWHIST, CItemData::POLICY,
                     CItemData::RUNCMD, CItemData::DCA, CItemData::EMAIL,
                     CItemData::PROTECTED, CItemData::SYMBOLS};
    UINT uimsgids[] = {IDS_COMPPASSWORD, IDS_COMPNOTES, IDS_COMPURL,
                       IDS_COMPAUTOTYPE, IDS_COMPPWHISTORY, IDS_COMPPWPOLICY,
                       IDS_COMPRUNCOMMAND, IDS_COMPDCA, IDS_COMPEMAIL,
                       IDS_COMPPROTECTED, IDS_COMPSYMBOLS};
    ASSERT(_countof(ifields) == _countof(uimsgids));

    // Time fields
    int itfields[] = {CItemData::CTIME, CItemData::PMTIME, CItemData::ATIME,
                      CItemData::XTIME, CItemData::RMTIME, CItemData::XTIME_INT};
    UINT uitmsgids[] = {IDS_COMPCTIME, IDS_COMPPMTIME, IDS_COMPATIME,
                        IDS_COMPXTIME, IDS_COMPRMTIME, IDS_COMPXTIME_INT};
    ASSERT(_countof(itfields) == _countof(uitmsgids));

    int n(0);
    for (int i = 0; i < _countof(ifields); i++) {
      if (st_SADV.bsFields.test(ifields[i])) {
        cs_buffer += L"\t" + CString(MAKEINTRESOURCE(uimsgids[i]));
        n++;
        if ((n % 5) == 0)
          cs_buffer += L"\r\n\t";
      }
    }
    cs_buffer += L"\r\n\t";
    n = 0;
    for (int i = 0; i < _countof(itfields); i++) {
      if (st_SADV.bsFields.test(itfields[i])) {
        cs_buffer += L"\t" + CString(MAKEINTRESOURCE(uitmsgids[i]));
        n++;
        if ((n % 3) == 0)
          cs_buffer += L"\r\n\t";
      }
    }

    pRpt->WriteLine((LPCWSTR)cs_buffer);
    pRpt->WriteLine();
  }
}
