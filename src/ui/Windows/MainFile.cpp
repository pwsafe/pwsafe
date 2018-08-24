/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include "AddEdit_DateTimes.h"
#include "PasskeyEntry.h"
#include "PWSFaultHandler.h"

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
#include "os/logit.h"

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

extern HRGN GetWorkAreaRegion();
extern const TCHAR *GROUPTITLEUSERINCHEVRONS;

static void DisplayFileWriteError(INT_PTR rc, const StringX &cs_newfile)
{
  ASSERT(rc != PWScore::SUCCESS);

  CGeneralMsgBox gmb;
  CString cs_temp, cs_title(MAKEINTRESOURCE(IDS_FILEWRITEERROR));
  switch (rc) {
  case PWScore::CANT_OPEN_FILE:
    cs_temp.Format(IDS_CANTOPENWRITING, static_cast<LPCWSTR>(cs_newfile.c_str()));
    break;
  case PWScore::FAILURE:
    cs_temp.Format(IDS_FILEWRITEFAILURE);
    break;
  case PWScore::WRONG_PASSWORD:
    cs_temp.Format(IDS_MISSINGPASSKEY);
    break;
  default:
    cs_temp.Format(IDS_UNKNOWNERROR, static_cast<LPCWSTR>(cs_newfile.c_str()));
    break;
  }
  gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
}

BOOL DboxMain::OpenOnInit()
{
  PWS_LOGIT;

  /*
    Routine to account for the differences between opening PSafe for
    the first time, and just opening a different database or
    un-minimizing the application
  */
  StringX passkey;
  BOOL retval(FALSE);
  // bReadOnly can only be from -r command line parameter unless user
  // has set the System Option to open read-only by default
  bool bReadOnly = m_core.IsReadOnly();
  if (!bReadOnly) {
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

  if (rc == PWScore::OPEN_NODB) {
    // Ensure all Toolbar buttons are correctly set
    UpdateMenuAndToolBar(false);
    return TRUE;
  }

  CString cs_title, cs_msg;
  cs_title.LoadString(IDS_FILEREADERROR);
  bool bAskerSet = m_core.IsAskerSet();
  bool bReporterSet = m_core.IsReporterSet();
  MFCAsker q;
  MFCReporter r;
  CReport Rpt;
  CRect rect;

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

  int rc2 = PWScore::NOT_SUCCESS;

  switch (rc) {
    case PWScore::SUCCESS:
      rc2 = m_core.ReadCurFile(passkey, true, MAXTEXTCHARS, &Rpt);
      m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                         m_core.GetCurFile()).c_str();
      UpdateSystemTray(UNLOCKED);
      break;
    case PWScore::CANT_OPEN_FILE:
      if (!m_core.IsDbOpen()) {
        // Empty filename. Assume they are starting Password Safe
        // for the first time and don't confuse them.
        // fall through to New()
      } else {
        // Here if there was a filename saved from last invocation, but it couldn't
        // be opened. It was either removed or renamed, so ask the user what to do
        cs_msg.Format(IDS_CANTOPENSAFE, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
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
   * If file's corrupted or read error
   * we prompt the user, and continue or not per user's input.
   * A bit too subtle for switch/case on rc2...
   */
  if (rc2 == PWScore::BAD_DIGEST ||
      rc2 == PWScore::TRUNCATED_FILE ||
      rc2 == PWScore::READ_FAIL) {
    CGeneralMsgBox gmb;
    cs_title.LoadString(IDS_FILEREADERROR);
    cs_msg.Format(IDS_FILECORRUPT, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
    if (gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONERROR) == IDNO) {
      CDialog::OnCancel();
      goto exit;
    }
    go_ahead = true;
  } // read error

  if (rc2 == PWScore::OK_WITH_VALIDATION_ERRORS) {
    rc2 = PWScore::SUCCESS;

    CGeneralMsgBox gmb;
    cs_title.LoadString(IDS_RPTVALIDATE);
    cs_msg.LoadString(IDS_VALIDATE_ISSUES);
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);

    if (gmb.DoModal() == IDS_VIEWREPORT)
      ViewReport(Rpt);
  }

  if (rc2 != PWScore::SUCCESS && !go_ahead) {
    // not a good return status, fold.
    if (m_InitMode != SilentInit)
      CDialog::OnCancel();
    goto exit;
  }

  if (!m_bOpen) {
    // Previous state was closed - reset DCA in status bar
    SetDCAText();
  }

  PostOpenProcessing();

  // Now get window sizes
  PWSprefs::GetInstance()->GetPrefRect(rect.top, rect.bottom, rect.left, rect.right);

  HRGN hrgnWork = GetWorkAreaRegion();
  // also check that window will be visible
  if ((rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) ||
    !RectInRegion(hrgnWork, rect)) {
    GetWindowRect(&rect);
    SendMessage(WM_SIZE, SIZE_RESTORED, MAKEWPARAM(rect.Width(), rect.Height()));
  } else {
    PlaceWindow(this, &rect, SW_HIDE);
  }
  ::DeleteObject(hrgnWork);

  bool bFileIsReadOnly;
  pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly);
  m_StatusBar.SetFileStatus(m_bOpen, bFileIsReadOnly);

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

  if (!m_core.IsReadOnly() && !m_bUserDeclinedSave && m_core.HasDBChanged()) {
    CGeneralMsgBox gmb;
    CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
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
        // Set flag to prevent further attempts to save
        m_bUserDeclinedSave = true;
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

  // Reset flag as new file
  m_bUserDeclinedSave = false;

  m_core.SetCurFile(cs_newfile);
  m_core.ClearFileUUID();

  rc = m_core.WriteCurFile();
  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, cs_newfile);
    return PWScore::USER_CANCEL;
  }

  m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " + cs_newfile).c_str();
  SetWindowText(LPCWSTR(m_titlebar));

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
  std::wstring newFileName = PWSUtil::GetNewFileName(LPCWSTR(cf), DEFAULT_SUFFIX);
  std::wstring dir;
  if (!m_core.IsDbOpen())
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
                     newFileName.c_str(),
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

  CSecString sPasskey;

  CPasskeySetup pksetup(this, m_core);
  rc = pksetup.DoModal();

  if (rc == IDOK)
    sPasskey = pksetup.GetPassKey();
  else
    return PWScore::USER_CANCEL;  //User cancelled password entry

  // Reset core and clear ALL associated data
  m_core.ReInit(true);

  // Clear application data
  ClearAppData();
  m_core.SafeUnlockCurFile();
  m_core.SetCurFile(newfilename);

  // Now lock the new file
  std::wstring locker(L""); // null init is important here
  m_core.LockFile(newfilename.c_str(), locker);

  m_core.SetReadOnly(false); // new file can't be read-only...
  m_core.NewFile(sPasskey);
  m_bDBNeedsReading = false;

  // Tidy up filters
  CurrentFilter().Empty();
  m_bFilterActive = false;
  m_FilterManager.SetFindFilter(false);
  m_FilterManager.SetFilterFindEntries(NULL);

  // Clear any saved group information
  m_TreeViewGroup = L"";

  return PWScore::SUCCESS;
}

void DboxMain::OnClose()
{
  PWS_LOGIT;

  // We will allow DB Close to close open dialogs IFF the DB is open in R-O and any open
  // dialogs can be closed
  bool bCloseOpenDialogs = IsDBReadOnly() && CPWDialog::GetDialogTracker()->VerifyCanCloseDialogs();

  if (bCloseOpenDialogs) {
    // Close all open dialogs - R-O mode ONLY + as above
    CPWDialog::GetDialogTracker()->CloseOpenDialogs();
  }

  Close();
}

int DboxMain::Close(const bool bTrySave)
{
  PWS_LOGIT_ARGS("bTrySave=%s", bTrySave ? L"true" : L"false");

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

      // No need to reset DB changed flag to stop being asked again here as
      // done in ClearAppData call below (which calls m_core.ClearData()
    }
  }

  // Turn off special display if on
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  m_core.SafeUnlockCurFile();
  m_core.SetCurFile(L"");

  SetDBInitiallyRO(false);

  CAddEdit_DateTimes::m_bShowUUID = false;

  // Reset core and clear ALL associated data
  m_core.ReInit();

  // Clear application data
  ClearAppData();

  // Set closed before anything else thinks there is data still here
  m_bOpen = false;

  // Tidy up filters
  m_bFilterActive = m_bUnsavedDisplayed = m_bExpireDisplayed = m_bFindFilterDisplayed = false;
  m_FilterManager.SetFindFilter(false);
  m_FilterManager.SetFilterFindEntries(NULL);
  ClearFilter();

  // Set Dragbar images correctly
  m_DDGroup.SetStaticState(false);
  m_DDTitle.SetStaticState(false);
  m_DDPassword.SetStaticState(false);
  m_DDUser.SetStaticState(false);
  m_DDNotes.SetStaticState(false);
  m_DDURL.SetStaticState(false);
  m_DDemail.SetStaticState(false);
  m_DDAutotype.SetStaticState(false);

  SetTooltipText(L"PasswordSafe");

  m_iDBIndex = 0;
  if (m_hMutexDBIndex != NULL) {
    CloseHandle(m_hMutexDBIndex);
    m_hMutexDBIndex = NULL;
  }

  UpdateSystemTray(CLOSED);

  // Call UpdateMenuAndToolBar before UpdateStatusBar
  // But we have already set m_bOpen
  UpdateMenuAndToolBar(false);
  m_titlebar = L"Password Safe";
  SetWindowText(LPCWSTR(m_titlebar));
  m_lastclipboardaction = L"";
  m_ilastaction = 0;
  UpdateStatusBar();

  m_StatusBar.SetFileStatus(false, false);

  // Delete any saved status information
  while (!m_stkSaveGUIInfo.empty()) {
    m_stkSaveGUIInfo.pop();
  }

  // Nothing to hide, don't lock on idle or logout
  // No need to check expired passwords
  KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
  KillTimer(TIMER_EXPENT);
  RegisterSessionNotification(false);

  // Update Minidump user streams
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly());

  return PWScore::SUCCESS;
}

void DboxMain::OnOpen()
{
  PWS_LOGIT;

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

BOOL DboxMain::OnOpenMRU(UINT nID)
{
  UINT uMRUItem = nID - ID_FILE_MRU_ENTRY1;

  CString mruItem = (*app.GetMRU())[uMRUItem];

  // Save just in case need to restore if user cancels
  const bool last_ro = m_core.IsReadOnly();
  m_core.SetReadOnly(false);

  // Read-only status can be overridden by GetAndCheckPassword
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

  return TRUE;
}

int DboxMain::Open(const UINT uiTitle)
{
  PWS_LOGIT_ARGS("uiTitle=%d", uiTitle);

  int rc = PWScore::SUCCESS;
  StringX sx_Filename;
  CString cs_text(MAKEINTRESOURCE(uiTitle));
  std::wstring DBpath, cdrive, cdir, dontCare;
  if (!m_core.IsDbOpen()) {
    // Can't use same directory as currently open DB as there isn't one.
    // Attempt to get path from last opened database from MRU
    // If valid and accessible, use it, if not valid or not accessible use
    // value returned from PWSdirs::GetSafeDir() - "My Safes"
    bool bUseGetSafeDir(true);
    if (app.GetMRU() != NULL && !app.GetMRU()->IsMRUEmpty()) {
      CString mruItem = (*app.GetMRU())[0];
      pws_os::splitpath((LPCWSTR)mruItem, cdrive, cdir, dontCare, dontCare);
      DBpath = cdrive + cdir;

      // _stat functions will fail for a directory if it ends with a slash
      // and splitpath always adds a slash!
      if (cdir.length() > 0)
        DBpath.pop_back();

      // Check it exists and accessible but don't are for information retrieved
      struct _stat stat_buf;
      int istat = _wstat(DBpath.c_str(), &stat_buf);
      // Now put trailing slash back!
      DBpath += L"\\";

      // If _stat worked and user has at least read access to the directory
      // use this value for DBpath, otherwise use result from PWSdirs::GetSafeDir()
      if (istat == 0 && _waccess_s(DBpath.c_str(), R_OK) == 0)
        bUseGetSafeDir = false;
    }
    if (bUseGetSafeDir)
      DBpath = PWSdirs::GetSafeDir();
  } else {
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    DBpath = cdrive + cdir;
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

    if (!DBpath.empty())
      fd.m_ofn.lpstrInitialDir = DBpath.c_str();

    INT_PTR rc2 = fd.DoModal();

    if (m_inExit) {
      // If U3ExitNow called while in CPWFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling
      PostQuitMessage(0);
      return PWScore::USER_CANCEL;
    }

    const bool last_ro = m_core.IsReadOnly(); // restore if user cancels
    m_core.SetReadOnly(m_bDBInitiallyRO || fd.GetReadOnlyPref() == TRUE);
    if (rc2 == IDOK) {
      sx_Filename = LPCWSTR(fd.GetPathName());

      rc = Open(sx_Filename, m_core.IsReadOnly(), uiTitle == IDS_CHOOSEDATABASEV);

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

  bool bFileIsReadOnly;
  pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly);
  m_StatusBar.SetFileStatus(m_bOpen, bFileIsReadOnly);

  return rc;
}

int DboxMain::Open(const StringX &sx_Filename, const bool bReadOnly,  const bool bHideReadOnly)
{
  CGeneralMsgBox gmb;
  INT_PTR rc1;
  int rc;
  StringX passkey;
  CString cs_temp, cs_title, cs_text, cs_msg;

  // Check that this file isn't already open
  if (sx_Filename == m_core.GetCurFile() && !m_bDBNeedsReading) {
    // It is the same damn file
    cs_text.LoadString(IDS_ALREADYOPEN);
    cs_title.LoadString(IDS_OPENDATABASE);
    gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::ALREADY_OPEN;
  }

  if (m_bOpen) {
    rc = SaveIfChanged();
    if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
      return rc;
  }

  // Set flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
  m_bUserDeclinedSave = true;

  // If we were using a different file, unlock it do this before
  // GetAndCheckPassword() as that routine gets a lock on the new file
  m_core.SafeUnlockCurFile();

  const int flags = ((bReadOnly ? GCP_READONLY : 0) | (bHideReadOnly ? GCP_HIDEREADONLY : 0) |
                     (m_bDBInitiallyRO ? GCP_FORCEREADONLY : 0));
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
      cs_temp.Format(IDS_SAFENOTEXIST, static_cast<LPCWSTR>(sx_Filename.c_str()));
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

  // Reset core and clear ALL associated data
  m_core.ReInit();

  // Clear application data
  ClearAppData();

  // Reset saved DB preferences
  m_savedDBprefs = EMPTYSAVEDDBPREFS;

  // Reset flag as new file
  m_bUserDeclinedSave = false;

  cs_title.LoadString(IDS_FILEREADERROR);
  bool bAskerSet = m_core.IsAskerSet();
  bool bReporterSet = m_core.IsReporterSet();
  MFCAsker q;
  MFCReporter r;
  CReport Rpt;

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

  // Now read the file - as initial open, file will be validated
  rc = m_core.ReadFile(sx_Filename, passkey, !m_bNoValidation, MAXTEXTCHARS, &Rpt);

  switch (rc) {
    case PWScore::OK_WITH_VALIDATION_ERRORS:
      break;
    case PWScore::SUCCESS:
      break;
    case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPENREADING, static_cast<LPCWSTR>(sx_Filename.c_str()));
      gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
      /*
        Everything stays as is... Worst case,
        they saved their file....
      */
      rc = PWScore::CANT_OPEN_FILE;
      goto exit;
    case PWScore::BAD_DIGEST:
      cs_temp.Format(IDS_FILECORRUPT, static_cast<LPCWSTR>(sx_Filename.c_str()));
      if (gmb.MessageBox(cs_temp, cs_title, MB_YESNO | MB_ICONERROR) == IDYES) {
        rc = PWScore::SUCCESS;
        break;
      } else
        goto exit;
    default:
      cs_temp.Format(IDS_UNKNOWNERROR, static_cast<LPCWSTR>(sx_Filename.c_str()));
      gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONERROR);
      goto exit;
  }

  m_core.SetCurFile(sx_Filename);
  PostOpenProcessing();

  if (rc == PWScore::OK_WITH_VALIDATION_ERRORS) {
    rc = PWScore::SUCCESS;

    cs_title.LoadString(IDS_RPTVALIDATE);
    cs_msg.LoadString(IDS_VALIDATE_ISSUES);
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);

    if (gmb.DoModal() == IDS_VIEWREPORT)
      ViewReport(Rpt);
  }

  bool bFileIsReadOnly;
  pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly);
  m_StatusBar.SetFileStatus(m_bOpen, bFileIsReadOnly);

exit:
  if (!bAskerSet)
    m_core.SetAsker(NULL);
  if (!bReporterSet)
    m_core.SetReporter(NULL);

  return rc;
}

void DboxMain::PostOpenProcessing()
{
  PWS_LOGIT;

  // Force prior format versions to be read-only
  if (m_core.GetReadFileVersion() < PWSfile::VCURRENT) {
    m_core.SetReadOnly(true);
  }

  m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCWSTR(m_titlebar));
  std::wstring drive, dir, name, ext;
  pws_os::splitpath(m_core.GetCurFile().c_str(), drive, dir, name, ext);

  // Do not add recovery files to the MRU
  if (ext != L".fbak")
    app.AddToMRU(m_core.GetCurFile().c_str());

  ChangeOkUpdate();

  // Clear any saved group information
  m_TreeViewGroup = L"";

  // Make row height update
  m_ctlItemList.UpdateRowHeight(true);

  // Set highlighting - need to do it here as SaveImmediately is a DB preference
  m_ctlItemTree.SetHighlightChanges(PWSprefs::GetInstance()->GetPref(PWSprefs::HighlightChanges) &&
                                    !PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately));

  RefreshViews();
  SetInitialDatabaseDisplay();
  m_bDBNeedsReading = false;
  SelectFirstEntry();

  UpdateSystemTray(UNLOCKED);
  UpdateMenuAndToolBar(true); // sets m_bOpen too...
  UpdateToolBarROStatus(m_core.IsReadOnly());
  UpdateToolBarDoUndo(); // BR1466
  UpdateStatusBar();

  CheckExpireList(true);
  TellUserAboutExpiredPasswords();

  m_RUEList.SetRUEList(m_core.GetRUEList());

  // Set timer for user-defined idle lockout, if selected (DB preference)
  KillTimer(TIMER_LOCKDBONIDLETIMEOUT);
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::LockDBOnIdleTimeout)) {
    ResetIdleLockCounter();
    SetTimer(TIMER_LOCKDBONIDLETIMEOUT, IDLE_CHECK_INTERVAL, NULL);
  }

  // Set up notification of desktop state, one way or another
  startLockCheckTimer();
  RegisterSessionNotification(true);

  // Save initial R-O or R/W state for when locking
  m_bDBInitiallyRO = m_core.IsReadOnly();

  // Update Minidump user streams
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly());

  // Now enable notification of DB changes
  ResumeOnDBNotification();

  // Set initial horizontal scroll bar position
  m_iListHBarPos = m_iTreeHBarPos = 0;
  m_ctlItemList.Scroll(CSize(SB_HORZ, 0));
  m_ctlItemTree.SetScrollPos(SB_HORZ, 0);
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
  PWSAuxCore othercore; // Leaves DB prefs untouched!

  // Get currently selected database's information
  // No Report or MAXCHARS value, implies no validation of the file
  // except the mandatory UUID uniqueness
  st_DBProperties st_dbpcore;
  othercore.ReadFile(sx_Filename, passkey);
  othercore.GetDBProperties(st_dbpcore);
  st_dbpcore.database = wsDBName.c_str();

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
  CGeneralMsgBox gmb;
  gmb.SetTitle(IDS_CLEARMRU);
  gmb.SetMsg(IDS_CONFIRMCLEARMRU);
  gmb.SetStandardIcon(MB_ICONEXCLAMATION);
  gmb.AddButton(IDS_YES, IDS_YES);
  gmb.AddButton(IDS_NO, IDS_NO, TRUE, TRUE);
  if (gmb.DoModal() == IDS_YES) {
    app.ClearMRU();
  }
}

void DboxMain::OnSave()
{
  Save();
}

int DboxMain::Save(const SaveType savetype)
{
  /*
   * We're treating both V3 and V4 as 'current'
   * versions, doing incremental backups for both.
   * For older versions, we offer to convert.
   * This means explicit V3 -> V4 conversion
   * Is done via SaveAs()
   */
  PWS_LOGIT_ARGS("savetype=%d", savetype);

  int rc;
  CString cs_msg, cs_temp;
  CGeneralMsgBox gmb;
  std::wstring NewName;
  std::wstring bu_fname; // used to undo backup if save failed

  const StringX sxCurrFile = m_core.GetCurFile();
  const PWSfile::VERSION current_version = m_core.GetReadFileVersion();

  PWSprefs *prefs = PWSprefs::GetInstance();

  // chdir to exe dir, avoid hassle with relative paths
  PWSdirs dir(PWSdirs::GetExeDir()); // changes back in d'tor

  // Save Application related preferences
  prefs->SaveApplicationPreferences();
  prefs->SaveShortcuts();

  if (sxCurrFile.empty())
    return SaveAs();

  switch (current_version) {
    case PWSfile::V30:
    case PWSfile::V40:
      if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
        int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
        int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
        std::wstring userBackupPrefix = prefs->GetPref(PWSprefs::BackupPrefixValue).c_str();
        std::wstring userBackupDir = prefs->GetPref(PWSprefs::BackupDir).c_str();

        // Max path can actually be 32K (UNC path), although we generally limit it to MAX_PATH (260)
        wchar_t wsExpandedPath[MAX_PATH + 1];
        DWORD dwResult = ExpandEnvironmentStrings(userBackupDir.c_str(),
                                                  wsExpandedPath, MAX_PATH + 1);
        if (dwResult == 0 || dwResult > (MAX_PATH + 1)) {
          CGeneralMsgBox gmbx;
          CString cs_msgx, cs_titlex(MAKEINTRESOURCE(IDS_EXPANDPATH));
          cs_msg.Format(IDS_CANT_EXPANDPATH, static_cast<LPCWSTR>(userBackupDir.c_str()));
          gmbx.MessageBox(cs_msgx, cs_titlex, MB_OK | MB_ICONEXCLAMATION);

          gmb.AfxMessageBox(IDS_NOIBACKUP, MB_OK);
          return PWScore::USER_CANCEL;
        } else {
          userBackupDir = wsExpandedPath;
        }

        if (!m_core.BackupCurFile(maxNumIncBackups, backupSuffix,
                                  userBackupPrefix, userBackupDir, bu_fname)) {
          switch (savetype) {
            case ST_NORMALEXIT:
            {
              cs_temp.LoadString(IDS_NOIBACKUP);
              cs_msg.Format(IDS_NOIBACKUP2, static_cast<LPCWSTR>(cs_temp));
              gmb.SetTitle(IDS_FILEWRITEERROR);
              gmb.SetMsg(cs_msg);
              gmb.SetStandardIcon(MB_ICONEXCLAMATION);
              gmb.AddButton(IDS_SAVEAS, IDS_SAVEAS);
              gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);

              if (gmb.DoModal() == IDS_EXIT)
                return PWScore::USER_CANCEL;
              else
                return SaveAs();
            }

            case ST_SAVEIMMEDIATELY:
            {
              cs_temp.LoadString(IDS_NOIBACKUP);
              cs_msg.Format(IDS_NOIBACKUP3, static_cast<LPCWSTR>(cs_temp));
              gmb.SetTitle(IDS_FILEWRITEERROR);
              gmb.SetMsg(cs_msg);
              gmb.SetStandardIcon(MB_ICONEXCLAMATION);
              gmb.AddButton(IDS_YES, IDS_YES);
              gmb.AddButton(IDS_NO, IDS_NO, TRUE, TRUE);

              if (gmb.DoModal() == IDS_NO)
                return PWScore::USER_CANCEL;
            }

            case ST_INVALID:
              // No particular end of PWS exit i.e. user clicked Save or
              // saving a changed database before opening another
              gmb.AfxMessageBox(IDS_NOIBACKUP, MB_OK);
              return PWScore::USER_CANCEL;

            default:
              break;
          }
          gmb.AfxMessageBox(IDS_NOIBACKUP, MB_OK);
          return SaveAs();
        } // BackupCurFile failed
      } // BackupBeforeEverySave
      break;

    // Do NOT code the default case statement - each version value must be specified
    // Prior versions are always Read-Only and so Save is not appropriate - although
    // they can export to prior versions (no point if not changed) or SaveAs in the
    // current version format
    case PWSfile::V17:
    case PWSfile::V20:
    case PWSfile::NEWFILE:
    case PWSfile::UNKNOWN_VERSION:
      ASSERT(0);
      return PWScore::FAILURE;
  } // switch on file version

  // Set DB header information not set via a Command i.e.
  // GroupDisplay and RUEList
  SaveGroupDisplayState();

  UUIDList RUEList;
  m_RUEList.GetRUEList(RUEList);
  m_core.SetRUEList(RUEList);

  // We are saving the current DB. Retain current version
  rc = m_core.WriteFile(sxCurrFile, current_version);

  if (rc != PWScore::SUCCESS) { // Save failed!
    // Restore backup, if we have one
    if (!bu_fname.empty() && !sxCurrFile.empty())
      pws_os::RenameFile(bu_fname, sxCurrFile.c_str());

    // Show user that we have a problem
    DisplayFileWriteError(rc, sxCurrFile);

    BlockLogoffShutdown(true);
    return rc;
  }

  BlockLogoffShutdown(false);

  // Reset all indications entry times changed
  m_bEntryTimestampsChanged = false;

  ChangeOkUpdate();

  // Added/Modified entries now saved - reverse it & refresh display
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  if (m_bFilterActive) { // we no longer limit this to status-changed filter
    // although strictly speaking, we should (overhead doesn't seem worth it)
    m_ctlItemList.Invalidate();
    m_ctlItemTree.Invalidate();
  }

  // Only refresh views if not existing
  if (savetype != ST_NORMALEXIT)
    RefreshViews();

  UpdateStatusBar();

  return PWScore::SUCCESS;
}

int DboxMain::SaveIfChanged()
{
  PWS_LOGIT;

  /*
    Save silently (without asking user) iff:
    1. NOT read-only AND
    2. (timestamp updates OR tree view display vector changed) AND
    3. Database NOT empty

    Less formally:
     If MaintainDateTimeStamps set and not read-only, save without asking
     user: "they get what it says on the tin".
   */

  // Deal with unsaved but changed restored DB
  if (m_bRestoredDBUnsaved && m_core.HasDBChanged()) {
    CGeneralMsgBox gmb;

    gmb.SetTitle(IDS_UNSAVEDRESTOREDDB);
    gmb.SetMsg(IDS_SAVEDRESTOREDDB);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_YES, IDS_YES, TRUE, TRUE);
    gmb.AddButton(IDS_NO, IDS_NO);

    if (gmb.DoModal() == IDS_NO)
      return PWScore::USER_DECLINED_SAVE;

    int rc = SaveAs();
    if (rc == PWScore::SUCCESS) {
      m_bRestoredDBUnsaved = false;
    }

    return rc;
  }

  if (m_core.IsReadOnly())
    return PWScore::SUCCESS;

  // Here we save the DB if the DB has at least one entry or empty group AND:
  //  Entry Access Times have been changed OR
  //  The Group Display has changed and the User specified to use it at open time OR
  //  RUE list has changed and the user wants them saved
  PWSprefs *prefs = PWSprefs::GetInstance();
  if (!m_bUserDeclinedSave &&
      (m_bEntryTimestampsChanged || 
       (prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen) == PWSprefs::AsPerLastSave && 
            m_core.HasGroupDisplayChanged()) ||
       (prefs->GetPref(PWSprefs::MaxREItems) > 0 &&
            m_core.HasRUEListChanged())) &&
      (m_core.GetNumEntries() > 0 || m_core.GetNumberEmptyGroups() > 0)) {
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
  if (m_core.HasDBChanged()) {
    CGeneralMsgBox gmb;
    INT_PTR rc, rc2;
    CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
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

int DboxMain::SaveImmediately()
{
  // Get normal save to do this (code already there for intermediate backups)
  return Save(ST_SAVEIMMEDIATELY);
}

void DboxMain::OnSaveAs()
{
  SaveAs();
}

int DboxMain::SaveAs()
{
  // SaveAs can only save in the current format
  // To save as a lower or higher format, the user should use Export

  // HOWEVER, in this "Experimental" version, V1.7, V2 & V3 DBs will be saved
  // as V3 and only if the current DB is V4 will it be saved in V4 format.

  PWS_LOGIT;

  INT_PTR rc;
  StringX newfile;
  CString cs_msg, cs_title, cs_text, cs_temp;

  const PWSfile::VERSION current_version = m_core.GetReadFileVersion();

  // Only need to warn user if current DB is prior to V3 - no implications
  // if saving V4 as V4 or V3 as V3
  if (current_version < PWSfile::V30 && 
      current_version != PWSfile::UNKNOWN_VERSION) {
    CGeneralMsgBox gmb;

    // Note: string IDS_NEWFORMAT2 will need to be updated when DB V4 is the default
    cs_msg.Format(IDS_NEWFORMAT2, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
    cs_title.LoadString(IDS_VERSIONWARNING);

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
    gmb.AddButton(IDS_CANCEL, IDS_CANCEL, TRUE, TRUE);

    if (gmb.DoModal() == IDS_CANCEL)
      return PWScore::USER_CANCEL;
  }

  // SaveAs-type dialog box
  StringX cf(m_core.GetCurFile());
  if (cf.empty()) {
    CString defname(MAKEINTRESOURCE(IDS_DEFDBNAME)); // reasonable default for first time user
    cf = LPCWSTR(defname);
  }

  // Note: The default export DB will be V3 unless the current DB is already in V4 format
  // This ensures that a user won't create an "Experimental" V4 DB by mistake
  std::wstring newFileName = PWSUtil::GetNewFileName(cf.c_str(),
                current_version == PWSfile::V40 ? V4_SUFFIX : V3_SUFFIX);

  std::wstring dir;
  if (!m_core.IsDbOpen())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  while (1) {
    CPWFileDialog fd(FALSE,
                     current_version == PWSfile::V40 ? V4_SUFFIX : V3_SUFFIX,
                     newFileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                        CString(MAKEINTRESOURCE(current_version == PWSfile::V40 ? IDS_FDF_V4_ALL : IDS_FDF_V3_ALL)),
                     this);
    if (!m_core.IsDbOpen())
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
    CGeneralMsgBox gmb;
    cs_temp.Format(IDS_FILEISLOCKED, static_cast<LPCWSTR>(newfile.c_str()),
                   static_cast<LPCWSTR>(locker.c_str()));
    cs_title.LoadString(IDS_FILELOCKERROR);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }

  // Save file UUID, clear it to generate new one, restore if necessary
  pws_os::CUUID file_uuid = m_core.GetFileUUID();
  m_core.ClearFileUUID();

  // Set DB header information not set via a Command i.e.
  // GroupDisplay and RUEList
  PWSprefs *prefs = PWSprefs::GetInstance();
  SaveGroupDisplayState(prefs->GetPref(PWSprefs::TreeDisplayStatusAtOpen) != 
    PWSprefs::AsPerLastSave);

  UUIDList RUElist;
  if (prefs->GetPref(PWSprefs::MaxREItems) > 0)
    m_RUEList.GetRUEList(RUElist);
  m_core.SetRUEList(RUElist);

  // Note: Writing out in in V4 DB format if the DB is already V4,
  // otherwise as V3 (this include saving pre-3.0 DBs as a V3 DB!
  rc = m_core.WriteFile(newfile, current_version == PWSfile::V40 ? PWSfile::V40 : PWSfile::V30);

  if (rc != PWScore::SUCCESS) {
    m_core.SetFileUUID(file_uuid); // restore uuid after failed save-as
    m_core.UnlockFile2(newfile.c_str());
    DisplayFileWriteError(rc, newfile);

    BlockLogoffShutdown(true);

    return PWScore::CANT_OPEN_FILE;
  }

  BlockLogoffShutdown(false);

  m_core.SafeUnlockCurFile();

  // Move the newfile lock to the right place
  m_core.MoveLock();

  m_core.SetCurFile(newfile);
  m_titlebar = PWSUtil::NormalizeTTT(L"Password Safe - " +
                                     m_core.GetCurFile()).c_str();
  SetWindowText(LPCWSTR(m_titlebar));

  std::wstring cdrive, cdir, cFilename, cExtn;
  pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, cFilename, cExtn);
  
  CString csTooltip;
  if (m_iDBIndex == 0) {
    csTooltip.Format(L"%s\n%s", (cdrive + cdir).c_str(), (cFilename + cExtn).c_str());
  } else {
    csTooltip.Format(L"%2d: %s\n    %s", m_iDBIndex, (cdrive + cdir).c_str(),
      (cFilename + cExtn).c_str());
  }

  SetTooltipText(csTooltip);

  // Reset all indications entry times changed
  m_bEntryTimestampsChanged = false;

  ChangeOkUpdate();

  // Added/Modified entries now saved - reverse it & refresh display
  if (m_bUnsavedDisplayed)
    OnShowUnsavedEntries();

  if (m_bFilterActive) { // we no longer limit this to status-changed filter
    // although strictly speaking, we should (overhead doesn't seem worth it)
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

  // In case it was an unsaved restored DB
  m_bRestoredDBUnsaved = false;

  UpdateStatusBar();

  return PWScore::SUCCESS;
}

void DboxMain::OnExportVx(UINT nID)
{
  INT_PTR rc;
  StringX newfile;
  CString cs_title, cs_text;

  const PWSfile::VERSION current_version = m_core.GetReadFileVersion();
  PWSfile::VERSION export_version = PWSfile::UNKNOWN_VERSION;
  std::wstring sfx = L"";
  int fdf = IDS_FDF_DB_ALL;

  switch (nID) {
    case ID_MENUITEM_EXPORT2OLD1XFORMAT:
      export_version = PWSfile::V17; sfx = L"dat"; fdf = IDS_FDF_V12_ALL;
      break;
    case ID_MENUITEM_EXPORT2V2FORMAT:
      export_version = PWSfile::V20; sfx = L"dat"; fdf = IDS_FDF_V12_ALL;
      break;
    case ID_MENUITEM_EXPORT2V3FORMAT:
      export_version = PWSfile::V30; sfx = L"psafe3"; fdf = IDS_FDF_V3_ALL;
      break;
    case ID_MENUITEM_EXPORT2V4FORMAT:
      export_version = PWSfile::V40; sfx = L"psafe4"; fdf = IDS_FDF_V4_ALL;
      break;
    default:
      ASSERT(0);
      return;
  }

  if (export_version < current_version) {
    // Only need warning if exporting to a lower DB version
    CGeneralMsgBox gmb;

    cs_text.Format(IDS_EXPORTWARNING, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
    cs_title.LoadString(IDS_VERSIONWARNING);

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_text);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
    gmb.AddButton(IDS_CANCEL, IDS_CANCEL, TRUE, TRUE);

    if (gmb.DoModal() == IDS_CANCEL)
      return;
  }

  //SaveAs-type dialog box
  std::wstring exportFileName = PWSUtil::GetNewFileName(m_core.GetCurFile().c_str(),
                                                        sfx);
  cs_text.LoadString(IDS_NAMEEXPORTFILE);

  std::wstring dir;
  if (!m_core.IsDbOpen())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  while (1) {
    CPWFileDialog fd(FALSE,
                     DEFAULT_SUFFIX,
                     exportFileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(fdf)),
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

  // Can't take a straight copy of current core because of the use
  // of pointers and because the entries have pointers to their
  // display info, which would be tried to be freed twice.
  // Do bare minimum - save header information only

  // Save & Restore of header whilst exporting now perfomed in
  // PWScore::WriteFile

  // Now export it in the requested version
  rc = m_core.WriteFile(newfile, export_version, false);

  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, newfile);
  }
}

void DboxMain::OnExportEntryDB()
{
  if (getSelectedItem() == NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTENT2DB,
    this, WZAdvanced::INVALID, NULL);

  wizard.SetDBVersion(m_core.GetReadFileVersion());

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

void DboxMain::OnExportGroupDB()
{
  if (getSelectedItem() != NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTGRP2DB,
    this, WZAdvanced::INVALID, NULL);

  wizard.SetDBVersion(m_core.GetReadFileVersion());

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

int DboxMain::DoExportDB(const StringX &sx_Filename, const UINT nID,
                         const bool bExportDBFilters, const StringX &sx_ExportKey,
                         int &numExported, CReport *prpt)
{
  INT_PTR rc;
  PWScore export_core;

  OrderedItemList OIL;
  CString cs_temp;
  std::vector<StringX> vEmptyGroups;
  std::vector<pws_os::CUUID> vuuidAddedBases;

  std::wstring str_text;
  LoadAString(str_text, IDS_RPTEXPORTDB);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  LoadAString(str_text, IDS_EXDB);
  cs_temp.Format(IDS_EXPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                 static_cast<LPCWSTR>(sx_Filename.c_str()));
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();

  if (nID == ID_MENUITEM_EXPORTGRP2DB || nID == ID_MENUITEM_EXPORTFILTERED2DB) {
    // Note: MakeOrderedItemList gets its members by walking the
    // tree therefore, if a filter is active, it will only export
    // those being displayed - except it will include bases of eligible entries
    // that not in the group or displayed because of the filter.
    vuuidAddedBases = MakeOrderedItemList(OIL,
                     (nID == ID_MENUITEM_EXPORTGRP2DB) ? m_ctlItemTree.GetSelectedItem() : NULL);

    // Get empty groups being exported
    std::vector<StringX> vAllEmptyGroups;
    vAllEmptyGroups = m_core.GetEmptyGroups();

    // Now prune empty groups vestor
    StringX sxPath = GetGroupName(true) + L".";
    const size_t len = sxPath.length();
    for (size_t i = 0; i < vAllEmptyGroups.size(); i++) {
      if (vAllEmptyGroups[i].substr(0, len) == sxPath)
        vEmptyGroups.push_back(vAllEmptyGroups[i]);
    }
  } else {
    // Note: Only selected entry but...
    // if Alias - use entry with base's password
    // If Shortcut - use complete base entry
    CItemData *pci = getSelectedItem();
    CItemData::EntryType et = pci->GetEntryType();
    switch (et) {
      case CItemData::ET_ALIAS:
      {
        CItemData ci = *pci;
        ci.SetPassword(pci->GetPassword());
        OIL.push_back(ci);
        break;
      }
      case CItemData::ET_SHORTCUT:
        pci = GetBaseEntry(pci);
        // Drop through intentionally
      default:
        OIL.push_back(*pci);
    }
  }

  numExported = (int)OIL.size();

  export_core.SetCurFile(sx_Filename);
  export_core.SetReadOnly(false);
  export_core.NewFile(sx_ExportKey);
  export_core.SetApplicationNameAndVersion(AfxGetAppName(), app.GetOSMajorMinor());
  rc = export_core.WriteExportFile(sx_Filename, &OIL, &m_core, m_core.GetReadFileVersion(), 
                                   vEmptyGroups, bExportDBFilters, vuuidAddedBases, prpt);

  OIL.clear();
  export_core.ClearDBData();

  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, sx_Filename);
  }

  if (!vuuidAddedBases.empty()) {
    prpt->WriteLine();
    CString csMessage(MAKEINTRESOURCE(IDS_INCLUDEDBASES));
    prpt->WriteLine(csMessage, true);

    for (size_t i = 0; i < vuuidAddedBases.size(); i++) {
      ItemListIter iter = Find(vuuidAddedBases[i]);
      StringX sx_exported;
      Format(sx_exported, GROUPTITLEUSERINCHEVRONS,
        iter->second.GetGroup().c_str(), iter->second.GetTitle().c_str(), iter->second.GetUser().c_str());
      prpt->WriteLine(sx_exported.c_str(), true);
    }
  }

  prpt->EndReport();
    
  return PWScore::SUCCESS;
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
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

void DboxMain::OnExportEntryText()
{
  if (getSelectedItem() == NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTENT2PLAINTEXT,
                          this, WZAdvanced::EXPORT_ENTRYTEXT,
                          &m_SaveWZAdvValues[WZAdvanced::EXPORT_ENTRYTEXT]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

void DboxMain::OnExportGroupText()
{
  if (getSelectedItem() != NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTGRP2PLAINTEXT,
    this, WZAdvanced::EXPORT_GROUPTEXT,
    &m_SaveWZAdvValues[WZAdvanced::EXPORT_GROUPTEXT]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

int DboxMain::DoExportText(const StringX &sx_Filename, const UINT nID,
                           const wchar_t &delimiter, const bool bAdvanced,
                           int &numExported, CReport *prpt)
{
  CGeneralMsgBox gmb;
  OrderedItemList OIL;
  CString cs_temp;
  WZAdvanced::AdvType nAdvType(WZAdvanced::INVALID);

  switch (nID) {
    case ID_MENUITEM_EXPORT2PLAINTEXT:
      nAdvType = WZAdvanced::EXPORT_TEXT;
      break;
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
      nAdvType = WZAdvanced::EXPORT_ENTRYTEXT;
      break;
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      nAdvType = WZAdvanced::EXPORT_GROUPTEXT;
      break;
    default:
      ASSERT(0);
      return PWScore::FAILURE;
  }

  st_SaveAdvValues *pst_ADV = &m_SaveWZAdvValues[nAdvType];

  CItemData::FieldBits bsAllFields; bsAllFields.set();
  const CItemData::FieldBits bsFields = bAdvanced ? pst_ADV->bsFields : bsAllFields;
  const std::wstring subgroup_name = bAdvanced ? pst_ADV->subgroup_name : L"";
  const int subgroup_object = bAdvanced ? pst_ADV->subgroup_object : CItemData::GROUP;
  const int subgroup_function = bAdvanced ? pst_ADV->subgroup_function : 0;

  std::wstring str_text;
  LoadAString(str_text, IDS_RPTEXPORTTEXT);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  LoadAString(str_text, IDS_TEXT);
  cs_temp.Format(IDS_EXPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                 static_cast<LPCWSTR>(sx_Filename.c_str()));
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();

  if (nID != ID_MENUITEM_EXPORTENT2PLAINTEXT) {
    // Note: MakeOrderedItemList gets its members by walking the
    // tree therefore, if a filter is active, it will ONLY export
    // those being displayed.
    MakeOrderedItemList(OIL, (nID == ID_MENUITEM_EXPORTGRP2PLAINTEXT) ? m_ctlItemTree.GetSelectedItem() : NULL);
  } else {
    // Note: Only selected entry but...
    // if Alias - use entry with base's password
    // If Shortcut - use complete base entry
    CItemData *pci = getSelectedItem();
    CItemData::EntryType et = pci->GetEntryType();
    switch (et) {
      case CItemData::ET_ALIAS:
      {
        CItemData ci = *pci;
        ci.SetPassword(pci->GetPassword());
        OIL.push_back(ci);
        break;
      }
      case CItemData::ET_SHORTCUT:
        pci = GetBaseEntry(pci);
        // Drop through intentionally
      default:
        OIL.push_back(*pci);
    }
  }

  ReportAdvancedOptions(prpt, bAdvanced, nAdvType);

  // Do the export
  int rc = m_core.WritePlaintextFile(sx_Filename, bsFields, subgroup_name,
                                     subgroup_object, subgroup_function,
                                     delimiter, numExported, &OIL,
                                     prpt);

  OIL.clear(); // cleanup soonest

  if (rc != PWScore::SUCCESS) {
    DisplayFileWriteError(rc, sx_Filename);
  }

  prpt->EndReport();
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
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

void DboxMain::OnExportEntryXML()
{
  if (getSelectedItem() == NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTENT2XML,
                          this, WZAdvanced::EXPORT_ENTRYXML,
                          &m_SaveWZAdvValues[WZAdvanced::EXPORT_ENTRYXML]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

void DboxMain::OnExportGroupXML()
{
  if (getSelectedItem() != NULL)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_EXPORTGRP2XML,
    this, WZAdvanced::EXPORT_GROUPXML,
    &m_SaveWZAdvValues[WZAdvanced::EXPORT_GROUPXML]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
}

int DboxMain::DoExportXML(const StringX &sx_Filename, const UINT nID,
                          const wchar_t &delimiter, const bool bAdvanced,
                          int &numExported, CReport *prpt)
{
  CGeneralMsgBox gmb;
  OrderedItemList OIL;
  CString cs_temp;

  WZAdvanced::AdvType nAdvType(WZAdvanced::INVALID);

  switch (nID) {
    case ID_MENUITEM_EXPORT2XML:
      nAdvType = WZAdvanced::EXPORT_XML;
      break;
    case ID_MENUITEM_EXPORTENT2XML:
      nAdvType = WZAdvanced::EXPORT_ENTRYXML;
      break;
    case ID_MENUITEM_EXPORTGRP2XML:
      nAdvType = WZAdvanced::EXPORT_GROUPXML;
      break;
    default:
      ASSERT(0);
      return PWScore::FAILURE;
  }

  st_SaveAdvValues *pst_ADV = &m_SaveWZAdvValues[nAdvType];

  CItemData::FieldBits bsAllFields; bsAllFields.set();
  const CItemData::FieldBits bsFields = bAdvanced ? pst_ADV->bsFields : bsAllFields;
  const std::wstring subgroup_name = bAdvanced ? pst_ADV->subgroup_name : L"";
  const int subgroup_object = bAdvanced ? pst_ADV->subgroup_object : CItemData::GROUP;
  const int subgroup_function = bAdvanced ? pst_ADV->subgroup_function : 0;
  std::wstring exportgroup(L"");

  std::wstring str_text;
  LoadAString(str_text, IDS_RPTEXPORTXML);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  LoadAString(str_text, IDS_XML);
  cs_temp.Format(IDS_EXPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                 static_cast<LPCWSTR>(sx_Filename.c_str()));
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();

  if (nID != ID_MENUITEM_EXPORTENT2XML) {
    // Note: MakeOrderedItemList gets its members by walking the
    // tree therefore, if a filter is active, it will ONLY export
    // those being displayed.
    HTREEITEM hi(NULL);
    if (nID == ID_MENUITEM_EXPORTGRP2XML) {
      // As exporting a group, set subgroup to be this group
      // so that only empty groups within it are exported rather than all.
      hi = m_ctlItemTree.GetSelectedItem();
      exportgroup = std::wstring(m_mapTreeItemToGroup[hi].c_str()) + std::wstring(L".");
    }
    MakeOrderedItemList(OIL, hi);
  } else {
    // Note: Only selected entry but...
    // if Alias - use entry with base's password
    // If Shortcut - use complete base entry
    CItemData *pci = getSelectedItem();
    CItemData::EntryType et = pci->GetEntryType();
    switch (et) {
      case CItemData::ET_ALIAS:
      {
        CItemData ci = *pci;
        ci.SetPassword(pci->GetPassword());
        OIL.push_back(ci);
        break;
      }
      case CItemData::ET_SHORTCUT:
        pci = GetBaseEntry(pci);
        // Drop through intentionally
      default:
        OIL.push_back(*pci);
    }
  }

  ReportAdvancedOptions(prpt, bAdvanced, nAdvType);

  // Do the export
  int rc = m_core.WriteXMLFile(sx_Filename, bsFields, subgroup_name,
                               subgroup_object, subgroup_function,
                               delimiter, exportgroup, 
                               numExported, &OIL,
                               m_bFilterActive, prpt);

  OIL.clear(); // cleanup soonest

  if (rc != PWScore::SUCCESS && rc != PWScore::OK_WITH_ERRORS) {
    DisplayFileWriteError(rc, sx_Filename);
  }

  prpt->EndReport();
  return rc;
}


void DboxMain::OnExportAttachment()
{
  /*
    We save the attachment, in the same initial directory as the current DB, as follows:

    * If the user retains the original file extension - just write it as-is BUT do not
      use CImage, even if it is an image file, as it might "optimise" the file and
      could become to be smaller causing confusion!
    * If it ia an image file (from media type) but with a different extension, 
      use CImage to convert it
    * If not an image file, just write it.
  */

  CItemData *pci = getSelectedItem();
  ASSERT(pci != NULL && pci->HasAttRef());

  CItemAtt &att = m_core.GetAtt(pci->GetAttUUID());

  CString filter, csMediaType;
  CSimpleArray<GUID> aguidFileTypes;
  HRESULT hResult;
  StringX sxAttFileName;
  std::wstring soutputfile;
  CImage AttImage;
  int iAttType(0);  // -1 not an image, 0 not yet tested, +1 an image

  // Get file name and extension
  sxAttFileName = att.GetFileName();

  wchar_t fullfilename[_MAX_PATH];
  wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT], new_ext[_MAX_EXT];
  _wsplitpath_s(sxAttFileName.c_str(), NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

  // Get current DB drive and directory
  _wsplitpath_s(m_core.GetCurFile().c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

  // Create new full file name
  _wmakepath_s(fullfilename, _MAX_FNAME, drive, dir, fname, ext);
  soutputfile = fullfilename;

  // Default suffix should be the same as the original file (skip over leading ".")
  CString cs_ext = ext[0] == '.' ? ext + 1 : ext;

  // Get media type
  csMediaType = att.GetMediaType().c_str();

  if (csMediaType.Left(5) == L"image") {
    // Should be an image file - but may not be supported by CImage - try..
    // Allocate attachment buffer
    UINT imagesize = (UINT)att.GetContentSize();
    HGLOBAL gMemory = GlobalAlloc(GMEM_MOVEABLE, imagesize);
    ASSERT(gMemory);

    BYTE *pBuffer = (BYTE *)GlobalLock(gMemory);
    ASSERT(pBuffer);

    // Load image into buffer
    att.GetContent(pBuffer, imagesize);

    // Put it into a IStream
    IStream *pStream = NULL;
    hResult = CreateStreamOnHGlobal(gMemory, FALSE, &pStream);
    if (hResult == S_OK) {
      // Load it
      hResult = AttImage.Load(pStream);
    } else {
      // Reset result so that user gets error dialog below
      // as we couldn't create the IStream object
      hResult = E_FAIL;
    }

    // Clean up - no real need to trash the buffer
    pStream->Release();
    GlobalUnlock(gMemory);
    GlobalFree(gMemory);

    // Was it an image?  (No : Yes)
    iAttType = FAILED(hResult) ? -1 : 1;
  } else {
    // Definitely not an image
    iAttType = -1;
  }

  switch (iAttType) {
    case 1:
    {
      // Should be an image and loaded into AttImage
      if (AttImage.IsNull()) {
        ASSERT(0);
        return;
      }

      hResult = AttImage.GetExporterFilterString(filter, aguidFileTypes);
      if (FAILED(hResult))
        return;

      // Extensions look much nicer in lower case
      filter.MakeLower();

      // Get index of current extension in filter string - note in pairs so need to skip every other one
      int cPos = 0;
      int iIndex = 1;  // Unusually, the filter index starts at 1 not 0
      CString cs_ext_nocase(ext); cs_ext_nocase.MakeLower();
      CString cs_filter_nocase(filter);
      CString cs_token;
      cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Descriptions
      cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Extensions
      if (cs_token.Find(cs_ext_nocase) == -1) {
        while (!cs_token.IsEmpty()) {
          cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Descriptions
          cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Extensions
          if (cs_token.Find(cs_ext_nocase) != -1)
            break;
          iIndex++;  // Documentation says index is per pair of file types
        };
      }

      // Do not use lpstrInitialDir as it has no effect after the first CFileDIalog call
      // Instead set the file name to be the full path
      CPWFileDialog fileDlg(FALSE, cs_ext, soutputfile.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
      fileDlg.m_pOFN->nFilterIndex = iIndex + 1;  // Not sure why need to add 1 but it seems to work!

      if (fileDlg.DoModal() == IDOK) {
        soutputfile = fileDlg.GetPathName();

        // Get new extension
        _wsplitpath_s(soutputfile.c_str(), NULL, 0, NULL, 0, NULL, 0, new_ext, _MAX_EXT);

        // If new extension is the same as old, export the file rather than use
        // CImage to save it (which may well change its size)
        if (_wcsicmp(ext, new_ext) == 0) {
          int rc = att.Export(soutputfile);
          hResult = (rc == PWScore::SUCCESS) ? S_OK : E_FAIL;
        } else {
          hResult = AttImage.Save(soutputfile.c_str());
        }

        // Now delete it
        AttImage.Destroy();

        if (FAILED(hResult)) {
          CGeneralMsgBox gmb;
          const CString cs_errmsg = L"Failed to save image";
          gmb.AfxMessageBox(cs_errmsg);
          return;
        }
      } else {
        // User cancelled save
        return;
      }
      break;
    }
    case -1:
    {
      // Not an image file
      // Set filter "??? files (*.???)|*.???||"
      SHFILEINFO sfi = {0};
      DWORD_PTR dw = SHGetFileInfo(soutputfile.c_str(), 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);

      if (dw != 0) {
        filter.Format(IDS_FDF_FILES, static_cast<LPCWSTR>(sfi.szTypeName), ext, ext);
      } else {
        // Use All files!
        filter.LoadString(IDS_FDF_ALL);
      }

      CPWFileDialog fileDlg(FALSE, cs_ext, fname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
      if (fileDlg.DoModal() == IDOK) {
        soutputfile = fileDlg.GetPathName();
        att.Export(soutputfile);
      } else {
        // User cancelled save
        return;
      }
      break;
    }
  }

  // We have saved/exported the file - now reset its file times to the original
  // when it was first imported/added
  if (soutputfile.empty()) {
    // Shouldn't get here with an empty export file name!
    ASSERT(0);
    return;
  }

  // Get old file times
  time_t ctime, mtime, atime;
  att.GetFileCTime(ctime);
  att.GetFileMTime(mtime);
  att.GetFileATime(atime);

  bool bUpdateFileTimes = pws_os::SetFileTimes(soutputfile, ctime, mtime, atime);
  if (!bUpdateFileTimes) {
    CGeneralMsgBox gmb;
    const CString cs_errmsg = L"Unable to open newly exported file to set file times.";
    gmb.AfxMessageBox(cs_errmsg);
  }
}

void DboxMain::OnExportFilteredDB()
{
  CWZPropertySheet wizard(ID_MENUITEM_EXPORTFILTERED2DB,
    this, WZAdvanced::INVALID, NULL);

  wizard.SetDBVersion(m_core.GetReadFileVersion());

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  wizard.DoModal();
  m_bWizardActive = false;
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
    cs_temp.Format(IDS_DBHASDUPLICATES, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return;
  }

  CImportTextDlg dlg;
  INT_PTR status = dlg.DoModal();

  if (status == IDCANCEL)
    return;

  StringX ImportedPrefix(dlg.m_groupName);
  CString cs_text;

  std::wstring dir;
  if (!m_core.IsDbOpen())
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
    int numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numWarnings(0),
      numNoPolicy(0);
    wchar_t delimiter = dlg.m_defimpdelim[0];
    bool bImportPSWDsOnly = dlg.m_bImportPSWDsOnly == TRUE;

    // Create report as we go
    CReport rpt;
    std::wstring str_text;
    LoadAString(str_text, IDS_RPTIMPORTTEXT);
    rpt.StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(str_text, IDS_TEXT);
    cs_temp.Format(IDS_IMPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                   static_cast<LPCWSTR>(TxtFileName.c_str()));
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.WriteLine();

    Command *pcmd = NULL;
    wchar_t fieldSeparator(dlg.m_Separator[0]);
    rc = m_core.ImportPlaintextFile(ImportedPrefix, TxtFileName, fieldSeparator,
                                    delimiter, bImportPSWDsOnly,
                                    strError,
                                    numImported, numSkipped, numPWHErrors, numRenamed,
                                    numNoPolicy,
                                    rpt, pcmd);

    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
        cs_title.LoadString(IDS_FILEREADERROR);
        cs_temp.Format(IDS_CANTOPENREADING, static_cast<LPCWSTR>(TxtFileName.c_str()));
        delete pcmd;
        break;
      case PWScore::INVALID_FORMAT:
        cs_title.LoadString(IDS_FILEREADERROR);
        cs_temp.Format(IDS_INVALIDFORMAT, static_cast<LPCWSTR>(TxtFileName.c_str()));
        delete pcmd;
        break;
      case PWScore::FAILURE:
        cs_title.LoadString(IDS_TEXTIMPORTFAILED);
        cs_temp = strError.c_str();
        delete pcmd;
        break;
      case PWScore::SUCCESS:
      case PWScore::OK_WITH_ERRORS:
        // deliberate fallthrough
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
                       numImported, static_cast<LPCWSTR>(cs_type));
        rpt.WriteLine((LPCWSTR)cs_temp);

        if (numSkipped != 0) {
          CString cs_tmp;
          cs_type.LoadString(numSkipped == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
          cs_tmp.Format(IDS_RECORDSSKIPPED, numSkipped, static_cast<LPCWSTR>(cs_type));
          rpt.WriteLine((LPCWSTR)cs_tmp);
          cs_temp += cs_tmp;
        }

        if (numPWHErrors != 0) {
          CString cs_tmp;
          cs_type.LoadString(numPWHErrors == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
          cs_tmp.Format(IDS_RECORDSPWHERRRORS, numPWHErrors, static_cast<LPCWSTR>(cs_type));
          rpt.WriteLine((LPCWSTR)cs_tmp);
          cs_temp += cs_tmp;
        }

        if (numRenamed != 0) {
          CString cs_tmp;
          cs_type.LoadString(numRenamed == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
          cs_tmp.Format(IDS_RECORDSRENAMED, numRenamed, static_cast<LPCWSTR>(cs_type));
          rpt.WriteLine((LPCWSTR)cs_tmp);
          cs_temp += cs_tmp;
        }

        if (numWarnings != 0 || numNoPolicy != 0) {
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

    if (gmb.DoModal() == IDS_VIEWREPORT)
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
  if (!m_core.IsDbOpen())
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
    cs_msg.Format(IDS_IMPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                  static_cast<LPCWSTR>(KPsFileName.c_str()));
    rpt.WriteLine((LPCWSTR)cs_msg);
    rpt.WriteLine();

    rc = m_core.ImportKeePassV1CSVFile(KPsFileName, numImported, numSkipped, numRenamed,
                                       uiReasonCode, rpt, pcmd);
    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
      {
        cs_msg.Format(IDS_CANTOPENREADING, static_cast<LPCWSTR>(KPsFileName.c_str()));
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
          cs_msg.Format(IDS_INVALIDFORMAT, static_cast<LPCWSTR>(KPsFileName.c_str()));
        cs_title.LoadString(IDS_IMPORTFAILED);
        delete [] pcmd;
        break;
      }
      case PWScore::SUCCESS:
      default: // deliberate fallthrough
        if (pcmd != NULL) {
          // Do it
          Execute(pcmd);

          ChangeOkUpdate();
        }

        RefreshViews();

        // May need to update menu/toolbar if original database was empty
        if (bWasEmpty)
          UpdateMenuAndToolBar(m_bOpen);

        rpt.WriteLine();
        CString cs_type;
        cs_type.LoadString(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
        cs_msg.Format(IDS_RECORDSIMPORTED, numImported, static_cast<LPCWSTR>(cs_type));
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

    if (gmb.DoModal() == IDS_VIEWREPORT)
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
  if (!m_core.IsDbOpen())
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
    cs_msg.Format(IDS_IMPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                  static_cast<LPCWSTR>(KPsFileName.c_str()));
    rpt.WriteLine((LPCWSTR)cs_msg);
    rpt.WriteLine();

    rc = m_core.ImportKeePassV1TXTFile(KPsFileName, numImported, numSkipped, numRenamed,
                                       uiReasonCode, rpt, pcmd);
    switch (rc) {
      case PWScore::CANT_OPEN_FILE:
      {
        cs_msg.Format(IDS_CANTOPENREADING, static_cast<LPCWSTR>(KPsFileName.c_str()));
        cs_title.LoadString(IDS_FILEOPENERROR);
        delete [] pcmd;
        break;
      }
      case PWScore::INVALID_FORMAT:
      {
        if (uiReasonCode > 0)
          cs_msg.LoadString(uiReasonCode);
        else
          cs_msg.Format(IDS_INVALIDFORMAT, static_cast<LPCWSTR>(KPsFileName.c_str()));
        cs_title.LoadString(IDS_IMPORTFAILED);
        delete [] pcmd;
        break;
      }
      case PWScore::SUCCESS:
      default: // deliberate fallthrough
        if (pcmd != NULL) {
          // Do it
          Execute(pcmd);

          ChangeOkUpdate();
        }

        RefreshViews();
        // May need to update menu/toolbar if original database was empty
        if (bWasEmpty)
          UpdateMenuAndToolBar(m_bOpen);

        rpt.WriteLine();
        CString cs_type;
        cs_type.LoadString(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
        cs_msg.Format(IDS_RECORDSIMPORTED, numImported, static_cast<LPCWSTR>(cs_type));
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

    if (gmb.DoModal() == IDS_VIEWREPORT)
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
    cs_temp.Format(IDS_DBHASDUPLICATES, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return;
  }

  const std::wstring XSDfn(L"pwsafe.xsd");
  std::wstring XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

  if (!pws_os::FileExists(XSDFilename)) {
    cs_temp.Format(IDSC_MISSINGXSD, static_cast<LPCWSTR>(XSDfn.c_str()));
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
  if (!m_core.IsDbOpen())
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
    //num* must be initialised because ImportXMLFile doesn't set them in case of validation errors
    int numValidated(0), numImported(0), numSkipped(0), numRenamed(0), numPWHErrors(0);
    int numNoPolicy(0), numRenamedPolicies(0), numShortcutsRemoved(0), numEmptyGroupsImported(0);
    bool bImportPSWDsOnly = dlg.m_bImportPSWDsOnly == TRUE;

    CWaitCursor waitCursor;  // This may take a while!

    // Create report as we go
    CReport rpt;
    std::wstring str_text;
    LoadAString(str_text, IDS_RPTIMPORTXML);
    rpt.StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
    LoadAString(str_text, IDS_XML);
    cs_temp.Format(IDS_IMPORTFILE, static_cast<LPCWSTR>(str_text.c_str()),
                   static_cast<LPCWSTR>(XMLFilename));
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.WriteLine();

    Command *pcmd = NULL;

    rc = m_core.ImportXMLFile(ImportedPrefix, std::wstring(XMLFilename),
                              XSDFilename.c_str(), bImportPSWDsOnly,
                              strXMLErrors, strSkippedList, strPWHErrorList, strRenameList,
                              numValidated, numImported, numSkipped, numPWHErrors, numRenamed,
                              numNoPolicy, numRenamedPolicies, numShortcutsRemoved,
                              numEmptyGroupsImported,
                              rpt, pcmd);
    waitCursor.Restore();  // Restore normal cursor

    std::wstring csErrors(L"");
    switch (rc) {
      case PWScore::XML_FAILED_VALIDATION:
        rpt.WriteLine(strXMLErrors.c_str());
        cs_temp.Format(IDS_FAILEDXMLVALIDATE, static_cast<LPCWSTR>(fd.GetFileName()), L"");
        delete pcmd;
        break;
      case PWScore::XML_FAILED_IMPORT:
        rpt.WriteLine(strXMLErrors.c_str());
        cs_temp.Format(IDS_XMLERRORS, static_cast<LPCWSTR>(fd.GetFileName()), L"");
        delete pcmd;
        break;
      case PWScore::SUCCESS:
      case PWScore::OK_WITH_ERRORS:
        cs_title.LoadString(rc == PWScore::SUCCESS ? IDS_COMPLETE : IDS_OKWITHERRORS);
        if (pcmd != NULL) {
          Execute(pcmd);

          ChangeOkUpdate();
        }

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
                         static_cast<LPCWSTR>(fd.GetFileName()),
                         numValidated, numImported,
                         static_cast<LPCWSTR>(cs_skipped),
                         static_cast<LPCWSTR>(cs_renamed),
                         static_cast<LPCWSTR>(cs_PWHErrors));
        } else {
          const CString cs_validate(MAKEINTRESOURCE(numValidated == 1 ? IDSC_ENTRY : IDSC_ENTRIES));
          const CString cs_imported(MAKEINTRESOURCE(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES));
          cs_temp.Format(IDS_XMLIMPORTOK, numValidated, static_cast<LPCWSTR>(cs_validate),
                         numImported, static_cast<LPCWSTR>(cs_imported));
        }

        RefreshViews();
        break;
      default:
        ASSERT(0);
    } // switch

    // Finish Report
    rpt.WriteLine((LPCWSTR)cs_temp);

    if (numNoPolicy != 0 || numRenamedPolicies != 0 || numShortcutsRemoved != 0) {
      CString cs_tmp(MAKEINTRESOURCE(IDS_WITHWARNINGS));
      cs_temp += cs_tmp;

      if (numNoPolicy != 0) {
        rpt.WriteLine();
        cs_tmp.LoadString(IDSC_MISSINGPOLICYNAMES);
        rpt.WriteLine((LPCWSTR)cs_tmp);
      }
      if (numRenamedPolicies != 0) {
        rpt.WriteLine();
        cs_tmp.LoadString(IDSC_RENAMEDPOLICYNAMES);
        rpt.WriteLine((LPCWSTR)cs_tmp);
      }
      if (numShortcutsRemoved != 0) {
        rpt.WriteLine();
        CString cs_imported(MAKEINTRESOURCE(IDSC_IMPORTED));
        cs_tmp.Format(IDSC_REMOVEDKBSHORTCUTS, static_cast<LPCWSTR>(cs_imported));
        rpt.WriteLine((LPCWSTR)cs_tmp);
      }
    }

    if ((rc == PWScore::SUCCESS || rc == PWScore::OK_WITH_ERRORS) && numEmptyGroupsImported != 0) {
      CString cs_tmp;
      cs_tmp.Format(IDSC_IMPORTEDEMPTYGROUPS, numEmptyGroupsImported);
      rpt.WriteLine((LPCWSTR)cs_tmp);
    }

    rpt.EndReport();

    if (rc != PWScore::SUCCESS || !strXMLErrors.empty())
      gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    else
      gmb.SetStandardIcon(MB_ICONINFORMATION);

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_temp);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);

    if (gmb.DoModal() == IDS_VIEWREPORT)
      ViewReport(rpt);

    // May need to update menu/toolbar if original database was empty
    if (bWasEmpty)
      UpdateMenuAndToolBar(m_bOpen);
  }
}

void DboxMain::OnProperties()
{
  st_DBProperties st_initialdbp, st_dbp;
  m_core.GetDBProperties(st_initialdbp);
  st_dbp = st_initialdbp;

  CProperties dlg(&st_dbp, IsDBReadOnly(), this);

  INT_PTR rc = dlg.DoModal();

  if (rc == IDOK && dlg.HasDataChanged()) {
    // Update user fields in header
    MultiCommands *pmulticmds = MultiCommands::Create(&m_core);
    if (st_dbp.db_name != st_initialdbp.db_name) {
      Command *pcmd_name = ChangeDBHeaderCommand::Create(&m_core,
        st_dbp.db_name, PWSfile::HDR_DBNAME);
      pmulticmds->Add(pcmd_name);
    }

    if (st_dbp.db_description != st_initialdbp.db_description) {
      Command *pcmd_desc = ChangeDBHeaderCommand::Create(&m_core,
        st_dbp.db_description, PWSfile::HDR_DBDESC);
      pmulticmds->Add(pcmd_desc);
    }

    if (!pmulticmds->IsEmpty()) {
      // Do it
      Execute(pmulticmds);
      ChangeOkUpdate();
    } else {
      delete pmulticmds;
    }
  }
}

bool DboxMain::ChangeMode(bool promptUser)
{
  // We need to prompt the user for password from r-o to r/w
  // when this is called with main window open. Arguably more
  // secure, s.t. an untrusted user can't change things.
  // When called as part of unlock, user just provided it.
  // From StatusBar and menu

  // Return value says change was successful
  const bool bWasRO = IsDBReadOnly();

  if (!bWasRO) { // R/W -> R-O
    // Try to save if any changes done to database
    int rc = SaveIfChanged();
    if (rc != PWScore::SUCCESS && rc != PWScore::USER_DECLINED_SAVE)
      return false;

    if (rc == PWScore::USER_DECLINED_SAVE) {
       // But ask just in case
       CGeneralMsgBox gmb;
       CString cs_msg(MAKEINTRESOURCE(IDS_BACKOUT_CHANGES)), cs_title(MAKEINTRESOURCE(IDS_CHANGEMODE));
       if (gmb.MessageBox(cs_msg, cs_title, MB_YESNO | MB_ICONQUESTION) == IDNO) {
         // Reset changed flag to stop being asked again (only if rc == PWScore::USER_DECLINED_SAVE)
         m_bUserDeclinedSave = true;
         return false;
       }

      // User said No to the save - so we must back-out all changes since last save
      while (m_core.HasDBChanged()) {
        OnUndo();
      }
    } // USER_DECLINED_SAVE

    // Clear the Commands & DB pre-command states
    m_core.ClearCommands();
  } else if (promptUser) { // R-O -> R/W
    // Taken from GetAndCheckPassword.
    // We don't want all the other processing that GetAndCheckPassword does
    CPasskeyEntry PasskeyEntryDlg(this, m_core.GetCurFile().c_str(),
      GCP_CHANGEMODE, true, false, false, true);

    INT_PTR rc = PasskeyEntryDlg.DoModal();
    if (rc != IDOK)
      return false;
  } // R-O -> R/W

  bool rc(true);
  std::wstring locker = L"";
  int iErrorCode(0);
  bool brc = m_core.ChangeMode(locker, iErrorCode);
  if (brc) {
    UpdateStatusBar();
    UpdateToolBarROStatus(!bWasRO);
  } else {
    rc = false;
    // Better give them the bad news!
    CGeneralMsgBox gmb;
    CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_CHANGEMODE_FAILED));
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
            cs_PID.Format(IDS_PROCESSID, static_cast<LPCWSTR>(cs_user_and_host.Right(8)));
            cs_user_and_host = cs_user_and_host.Left(i_pid);
          } else {
            cs_PID = L"";
          }

          cs_msg.Format(IDS_CM_FAIL_REASON1, static_cast<LPCWSTR>(cs_user_and_host),
                        static_cast<LPCWSTR>(cs_PID));
          bInUse = true;
          break;
        }
        case PWSfile::CANT_OPEN_FILE:
          uiMsg = IDS_CM_FAIL_REASON4;
          break;
        case PWSfile::END_OF_FILE:
          uiMsg = IDS_CM_FAIL_REASON5;
          break;
        case PWSfile::READ_FAIL:
          // Temporary use of this value to indicate DB is R-O at the file level
          // and so cannot change to R/W
          uiMsg = IDS_FILEREADONLY;
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

  if (bWasRO != IsDBReadOnly())
    UpdateEditViewAccelerator(IsDBReadOnly());

  // Update Minidump user streams - mode is in user stream 0
  app.SetMinidumpUserStreams(m_bOpen, !IsDBReadOnly(), us0);

  return rc;
}

void DboxMain::OnChangeMode()
{
  PWS_LOGIT;

  // Don't bother doing anything if DB is read-only on disk
  bool bFileIsReadOnly;
  if (pws_os::FileExists(m_core.GetCurFile().c_str(), bFileIsReadOnly) && bFileIsReadOnly)
    return;

  // Don't allow prior format versions to become R/W
  // Do allow current (and possible future 'experimental') formats
  if (m_core.GetReadFileVersion() >= PWSfile::VCURRENT)
    ChangeMode(true); // true means "prompt use for password".

  // Update Toolbar
  UpdateToolBarROStatus(m_core.IsReadOnly());

  CItemData *pci = GetLastSelected();
  UpdateToolBarForSelectedItem(pci);
}

void DboxMain::OnCompare()
{
  if (!m_core.IsDbOpen() || m_core.GetNumEntries() == 0) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_NOCOMPAREFILE, MB_OK | MB_ICONWARNING);
    return;
  }

  CWZPropertySheet wizard(ID_MENUITEM_COMPARE,
                          this, WZAdvanced::COMPARE,
                          &m_SaveWZAdvValues[WZAdvanced::COMPARE]);

  // Don't care about the return code: ID_WIZFINISH or IDCANCEL
  m_bWizardActive = true;
  INT_PTR rc = wizard.DoModal();
  m_bWizardActive = false;

  if (rc == ID_WIZFINISH && wizard.GetNumProcessed() > 0) {
    ChangeOkUpdate();

    UpdateToolBarDoUndo();
  }
}

void DboxMain::OnMerge()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  CWZPropertySheet wizard(ID_MENUITEM_MERGE,
                          this, WZAdvanced::MERGE,
                          &m_SaveWZAdvValues[WZAdvanced::MERGE]);

  m_bWizardActive = true;
  INT_PTR rc = wizard.DoModal();
  m_bWizardActive = false;

  if (rc == ID_WIZFINISH && wizard.GetNumProcessed() > 0) {
    ChangeOkUpdate();

    UpdateToolBarDoUndo();
  }

  // Couldn't do this whilst wizard open
  SetToolBarPositions();
}

void DboxMain::OnSynchronize()
{
  // disable in read-only mode or empty
  if (m_core.IsReadOnly() || !m_core.IsDbOpen() || m_core.GetNumEntries() == 0)
    return;

  CWZPropertySheet wizard(ID_MENUITEM_SYNCHRONIZE,
                          this, WZAdvanced::SYNCH,
                          &m_SaveWZAdvValues[WZAdvanced::SYNCH]);

  m_bWizardActive = true;
  INT_PTR rc = wizard.DoModal();
  m_bWizardActive = false;

  if (rc == ID_WIZFINISH && wizard.GetNumProcessed() > 0) {
    ChangeOkUpdate();

    UpdateToolBarDoUndo();
  }

  // Couldn't do this whilst wizard open
  SetToolBarPositions();
}

std::wstring DboxMain::DoMerge(PWScore *pothercore,
                               const bool bAdvanced, CReport *prpt, bool *pbCancel)
{
  CGeneralMsgBox gmb;
  CString cs_title, cs_temp,cs_text;
  // Initialize set
  GTUSet setGTU;

  // First check other database
  if (!pothercore->GetUniqueGTUValidated() && !pothercore->InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_title.LoadString(IDS_MERGEFAILED);
    cs_temp.Format(IDS_DBHASDUPLICATES, static_cast<LPCWSTR>(pothercore->GetCurFile().c_str()));
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return L"";
  }

  // Next check us - we need the setGTU later
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_title.LoadString(IDS_MERGEFAILED);
    cs_temp.Format(IDS_DBHASDUPLICATES, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return L"";
  }

  // Create report as we go
  std::wstring str_text;
  LoadAString(str_text, IDS_RPTMERGE);
  prpt->StartReport(str_text.c_str(), m_core.GetCurFile().c_str());
  cs_temp.Format(IDS_MERGINGDATABASE, static_cast<LPCWSTR>(pothercore->GetCurFile().c_str()));
  prpt->WriteLine((LPCWSTR)cs_temp);
  prpt->WriteLine();

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
             subgroup_name, subgroup_object, subgroup_function, prpt, pbCancel);

  // restore normal cursor
  waitCursor.Restore();

  if (pbCancel != NULL && *pbCancel) {
    CString cs_buffer(MAKEINTRESOURCE(IDS_OPERATION_ABORTED));
    prpt->WriteLine((LPCWSTR)cs_buffer);
  }

  prpt->EndReport();

  return str_result;
}

bool DboxMain::DoCompare(PWScore *pothercore,
                         const bool bAdvanced, CReport *prpt, bool *pbCancel)
{
  CString cs_temp, cs_text, cs_buffer;

  m_list_OnlyInCurrent.clear();
  m_list_OnlyInComp.clear();
  m_list_Conflicts.clear();
  m_list_Identical.clear();

  // Create report as we go
  std::wstring str_text;
  LoadAString(str_text, IDS_RPTCOMPARE);
  prpt->StartReport(str_text.c_str(), static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
  cs_temp.Format(IDS_COMPARINGDATABASE, static_cast<LPCWSTR>(pothercore->GetCurFile().c_str()));
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

  m_bsFields = bsFields;

  ReportAdvancedOptions(prpt, bAdvanced, WZAdvanced::COMPARE);

  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  m_core.Compare(pothercore,
                 bsFields, subgroup_bset, bTreatWhiteSpaceasEmpty,
                 subgroup_name, subgroup_object,
                 subgroup_function,
                 m_list_OnlyInCurrent, m_list_OnlyInComp,
                 m_list_Conflicts, m_list_Identical, pbCancel);

  // restore normal cursor
  waitCursor.Restore();

  if (pbCancel != NULL && *pbCancel) {
    cs_buffer.LoadString(IDS_OPERATION_ABORTED);
    prpt->WriteLine((LPCWSTR)cs_buffer);
    prpt->EndReport();
    return false;
  }

  cs_buffer.Format(IDS_COMPARESTATISTICS,
                   static_cast<LPCWSTR>(m_core.GetCurFile().c_str()),
                   static_cast<LPCWSTR>(pothercore->GetCurFile().c_str()));

  bool brc(true);  // True == databases are identical
  if (m_list_OnlyInCurrent.empty() &&
      m_list_OnlyInComp.empty() &&
      m_list_Conflicts.empty()) {
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

CString DboxMain::ShowCompareResults(const StringX sx_Filename1,
                                     const StringX sx_Filename2,
                                     PWScore *pothercore, CReport *prpt)
{
  CString csProtect = Fonts::GetInstance()->GetProtectedSymbol().c_str();
  CString csAttachment = Fonts::GetInstance()->GetAttachmentSymbol().c_str();


  // Can't do UI from a worker thread!
  CCompareResultsDlg CmpRes(this, m_list_OnlyInCurrent, m_list_OnlyInComp,
                            m_list_Conflicts, m_list_Identical,
                            m_bsFields, &m_core, pothercore,
                            csProtect, csAttachment, prpt);

  CmpRes.m_scFilename1 = sx_Filename1;
  CmpRes.m_scFilename2 = sx_Filename2;
  CmpRes.m_bOriginalDBReadOnly = m_core.IsReadOnly();
  CmpRes.m_bComparisonDBReadOnly = pothercore->IsReadOnly();

  CmpRes.DoModal();

  if (CmpRes.m_OriginalDBChanged) {
    // We didn't save after each change within the ComapreResults dialog
    // So potentially do it now
    if (PWSprefs::GetInstance()->GetPref(PWSprefs::SaveImmediately)) {
      SaveImmediately();
    }

    // Have to update views as user may have changed/added entries
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
                             const bool bAdvanced, int &numUpdated, CReport *prpt,
                             bool *pbCancel)
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
    str_temp.Format(IDS_DBHASDUPLICATES, static_cast<LPCWSTR>(pothercore->GetCurFile().c_str()));
    gmb.MessageBox(str_temp, str_title, MB_ICONEXCLAMATION);
    return;
  }

  // Next check us
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    str_title.LoadString(IDS_SYNCHFAILED);
    str_temp.Format(IDS_DBHASDUPLICATES, static_cast<LPCWSTR>(m_core.GetCurFile().c_str()));
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
  str_temp.Format(IDS_SYNCHINGDATABASE, static_cast<LPCWSTR>(pothercore->GetCurFile().c_str()));
  prpt->WriteLine((LPCWSTR)str_temp);
  prpt->WriteLine();

  ReportAdvancedOptions(prpt, bAdvanced, WZAdvanced::SYNCH);

  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  // Do the Synchronize
  m_core.Synchronize(pothercore, bsFields, subgroup_bset,
                     subgroup_name, subgroup_object, subgroup_function,
                     numUpdated, prpt, pbCancel);

  // Restore normal cursor
  waitCursor.Restore();

  if (pbCancel != NULL && *pbCancel) {
    CString cs_buffer(MAKEINTRESOURCE(IDS_OPERATION_ABORTED));
    prpt->WriteLine((LPCWSTR)cs_buffer);
  }

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
    pELLE->bIsProtected = pci->IsProtected();
    pELLE->bHasAttachment = pci->HasAttRef();

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
    pELLE->sx_expirylocdate = PWSUtil::ConvertToDateTimeString(tttXTime, PWSUtil::TMC_LOCALE);

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
    case CCompareResultsDlg::SYNCH:
      lres = SynchCompareResult(st_info->pcore1, st_info->pcore0,
                                st_info->uuid1, st_info->uuid0);
      break;
    default:
      ASSERT(0);
  }
  return lres;
}

LRESULT DboxMain::OnProcessCompareResultAllFunction(WPARAM wParam, LPARAM lFunction)
{
  LRESULT lres(FALSE);

  switch ((int)lFunction) {
    case CCompareResultsDlg::COPYALL_TO_ORIGINALDB:
      lres = CopyAllCompareResult(wParam);
      break;
    case CCompareResultsDlg::SYNCHALL:
      lres = SynchAllCompareResult(wParam);
      break;
    default:
      ASSERT(0);
  }
  return lres;
}

LRESULT DboxMain::OnDroppedFile(WPARAM /* wParam */, LPARAM /* lParam */)
{
  bool bReadOnly = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO);
  Open(m_ctlItemTree.GetDroppedFile(), bReadOnly);
  return 0;
}

LRESULT DboxMain::ViewCompareResult(PWScore *pcore, const CUUID &entryUUID)
{
  ItemListIter pos = pcore->Find(entryUUID);
  ASSERT(pos != pcore->GetEntryEndIter());
  CItemData *pci = &pos->second;

  // View the correct entry and make sure R-O
  bool bSaveRO = pcore->IsReadOnly();
  pcore->SetReadOnly(true);

  // Edit the correct entry
  if (pci->GetEntryType() != CItemData::ET_SHORTCUT)
    EditItem(pci, pcore);
  else
    EditShortcut(pci, pcore);

  pcore->SetReadOnly(bSaveRO);

  return FALSE;
}

LRESULT DboxMain::EditCompareResult(PWScore *pcore, const CUUID &entryUUID)
{
  ItemListIter pos = pcore->Find(entryUUID);
  ASSERT(pos != pcore->GetEntryEndIter());
  CItemData *pci = &pos->second;

  // Edit the correct entry
  if (pci->GetEntryType() != CItemData::ET_SHORTCUT)
    return EditItem(pci, pcore) ? TRUE : FALSE;
  else
    return EditShortcut(pci, pcore) ? TRUE : FALSE;
}

LRESULT DboxMain::CopyCompareResult(PWScore *pfromcore, PWScore *ptocore,
                                    const CUUID &fromUUID, const CUUID &toUUID)
{
  // This is always from Comparison DB to Current DB
  bool bWasEmpty = ptocore->GetNumEntries() == 0;

  // Copy *pfromcore entry -> *ptocore entry
  ItemListIter fromPos = pfromcore->Find(fromUUID);
  ASSERT(fromPos != pfromcore->GetEntryEndIter());
  const CItemData *pfromEntry = &fromPos->second;
  CItemData ci_temp(*pfromEntry);  // Set up copy

  // If the UUID is not in use in the "to" core, copy it too, otherwise reuse current
  if (ptocore->Find(fromUUID) == ptocore->GetEntryEndIter()) {
    ci_temp.SetUUID(fromUUID);
  } else {
    if (toUUID == CUUID::NullUUID())
      ci_temp.CreateUUID();
    else
      ci_temp.SetUUID(toUUID);
  }

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

  // Check policy names
  // Don't really need the map and vector as only copying 1 entry
  std::map<StringX, StringX> mapRenamedPolicies;
  std::vector<StringX> vs_PoliciesAdded;
  bool bUpdated;  // Not need for Copy

  const StringX sxCopy_DateTime = PWSUtil::GetTimeStamp(true).c_str();
  StringX sxPolicyName = pfromEntry->GetPolicyName();

  // Special processing for password policies (default & named)
  Command *pPolicyCmd = ptocore->ProcessPolicyName(pfromcore, ci_temp,
      mapRenamedPolicies, vs_PoliciesAdded, sxPolicyName, bUpdated,
      sxCopy_DateTime, IDSC_COPYPOLICY);

  if (pPolicyCmd != NULL)
    pmulticmds->Add(pPolicyCmd);

  // Is it already there?
  const StringX sxgroup(ci_temp.GetGroup()), sxtitle(ci_temp.GetTitle()),
    sxuser(ci_temp.GetUser());
  ItemListIter toPos = ptocore->Find(sxgroup, sxtitle, sxuser);

  if (toPos != ptocore->GetEntryEndIter()) {
    // Already there - change it
    CItemData *ptoEntry = &toPos->second;
    ci_temp.SetStatus(CItemData::ES_MODIFIED);
    pmulticmds->Add(EditEntryCommand::Create(ptocore, *ptoEntry, ci_temp));
  } else {
    // Not there - add it
    // Need to check that entry keyboard shortcut not already in use!
    int32 iKBShortcut;
    ci_temp.GetKBShortcut(iKBShortcut);
    if (iKBShortcut != 0 &&
      m_core.GetKBShortcut(iKBShortcut) != CUUID::NullUUID()) {
      // Remove it but no mechanism to tell user!
      ci_temp.SetKBShortcut(0);
    }
    ci_temp.SetStatus(CItemData::ES_ADDED);
    pmulticmds->Add(AddEntryCommand::Create(ptocore, ci_temp));
  }

  // Do it
  Execute(pmulticmds);

  ChangeOkUpdate();

  // May need to update menu/toolbar if database was previously empty
  if (bWasEmpty)
    UpdateMenuAndToolBar(m_bOpen);

  CItemData *pci = GetLastSelected();
  UpdateToolBarForSelectedItem(pci);

  return TRUE;
}

LRESULT DboxMain::SynchCompareResult(PWScore *pfromcore, PWScore *ptocore,
                                     const CUUID &fromUUID, const CUUID &toUUID)
{
  // Synch 1 entry *pfromcore -> *ptocore

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

  MultiCommands *pmulticmds = MultiCommands::Create(ptocore);

  bool bUpdated(false);
  for (size_t i = 0; i < m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH].bsFields.size(); i++) {
    if (m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH].bsFields.test(i)) {
      const StringX sxValue = pfromEntry->GetFieldValue((CItemData::FieldType)i);

      // Special processing for password policies (default & named)
      if ((CItemData::FieldType)i == CItemData::POLICYNAME) {
        // Don't really need the map and vector as only sync'ing 1 entry
        std::map<StringX, StringX> mapRenamedPolicies;
        std::vector<StringX> vs_PoliciesAdded;

        const StringX sxSync_DateTime = PWSUtil::GetTimeStamp(true).c_str();
        StringX sxPolicyName = pfromEntry->GetPolicyName();

        Command *pPolicyCmd = ptocore->ProcessPolicyName(pfromcore, updtEntry,
             mapRenamedPolicies, vs_PoliciesAdded, sxPolicyName, bUpdated,
             sxSync_DateTime, IDSC_SYNCPOLICY);
        if (pPolicyCmd != NULL)
          pmulticmds->Add(pPolicyCmd);
      } else {
        if (sxValue != updtEntry.GetFieldValue((CItemData::FieldType)i)) {
          bUpdated = true;
          updtEntry.SetFieldValue((CItemData::FieldType)i, sxValue);
        }
      }
    }
  }

  if (bUpdated) {
    updtEntry.SetStatus(CItemData::ES_MODIFIED);
    pmulticmds->Add(EditEntryCommand::Create(ptocore, *ptoEntry, updtEntry));

    // Do it
    Execute(pmulticmds, ptocore);

    ChangeOkUpdate();
    return TRUE;
  }

  return FALSE;
}

LRESULT DboxMain::CopyAllCompareResult(WPARAM wParam)
{
  // This is always from Comparison DB to Current DB
  bool bWasEmpty = GetNumEntries() == 0;
  std::vector<st_CompareInfo *> *vpst_info = (std::vector<st_CompareInfo *> *)wParam;

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

  // Make sure we don't add or rename named password policies multiple times
  std::map<StringX, StringX> mapRenamedPolicies;
  std::vector<StringX> vs_PoliciesAdded;

  const StringX sxCopy_DateTime = PWSUtil::GetTimeStamp(true).c_str();

  for (size_t index = 0; index < vpst_info->size(); index++) {
    st_CompareInfo *pst_info = (*vpst_info)[index];
    // Copy *pfromcore entry -> *ptocore entry
    PWScore *ptocore = pst_info->pcore0;
    PWScore *pfromcore = pst_info->pcore1;
    CUUID toUUID = pst_info->uuid0;
    CUUID fromUUID = pst_info->uuid1;

    ItemListIter fromPos = pfromcore->Find(fromUUID);
    ASSERT(fromPos != pfromcore->GetEntryEndIter());
    const CItemData *pfromEntry = &fromPos->second;
    CItemData ci_temp(*pfromEntry);  // Set up copy

    // Special processing for password policies (default & named)
    StringX sxPolicyName = pfromEntry->GetPolicyName();
    bool bUpdated;  // Not needed for Copy

    Command *pcmd = ptocore->ProcessPolicyName(pfromcore, ci_temp,
      mapRenamedPolicies, vs_PoliciesAdded, sxPolicyName, bUpdated,
      sxCopy_DateTime, IDSC_COPYPOLICY);

    if (pcmd != NULL)
      pmulticmds->Add(pcmd);

    // If the UUID is not in use in the "to" core, copy it too, otherwise reuse current
    if (ptocore->Find(fromUUID) == ptocore->GetEntryEndIter()) {
      ci_temp.SetUUID(fromUUID);
    } else {
      if (toUUID == CUUID::NullUUID())
        ci_temp.CreateUUID();
      else
        ci_temp.SetUUID(toUUID);
    }

    Command *pcmdCopy(NULL);

    // Is it already there:?
    const StringX sxgroup(ci_temp.GetGroup()), sxtitle(ci_temp.GetTitle()),
         sxuser(ci_temp.GetUser());
    ItemListIter toPos = ptocore->Find(sxgroup, sxtitle, sxuser);

    if (toPos != ptocore->GetEntryEndIter()) {
      // Already there - change it
      CItemData *ptoEntry = &toPos->second;
      ci_temp.SetStatus(CItemData::ES_MODIFIED);
      pcmdCopy = EditEntryCommand::Create(ptocore, *ptoEntry, ci_temp);
      pcmdCopy->SetNoGUINotify();
    } else {
      // Not there - add it
      // Need to check that entry keyboard shortcut not already in use!
      int32 iKBShortcut;
      ci_temp.GetKBShortcut(iKBShortcut);
      if (iKBShortcut != 0 &&
        m_core.GetKBShortcut(iKBShortcut) != CUUID::NullUUID()) {
        // Remove it but no mechanism to tell user!
        ci_temp.SetKBShortcut(0);
      }
      ci_temp.SetStatus(CItemData::ES_ADDED);
      pcmdCopy = AddEntryCommand::Create(ptocore, ci_temp);
      pcmdCopy->SetNoGUINotify();
    }
    pmulticmds->Add(pcmdCopy);
  }

  if (pmulticmds->IsEmpty())
    return FALSE;

  CWaitCursor waitCursor;

  // Do it
  Execute(pmulticmds);

  waitCursor.Restore();

  RefreshViews();

  ChangeOkUpdate();

  // May need to update menu/toolbar if database was previously empty
  if (bWasEmpty)
    UpdateMenuAndToolBar(m_bOpen);

  return TRUE;
}

LRESULT DboxMain::SynchAllCompareResult(WPARAM wParam)
{
  // Synch multiple entries *pfromcore -> *ptocore

  // Use a cut down Advanced dialog (only fields to synchronize)
  // This will apply to all entries that are synchronised
  CAdvancedDlg Adv(this, CAdvancedDlg::COMPARESYNCH,
                   &m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH]);

  INT_PTR rc = Adv.DoModal();

  if (rc != IDOK)
    return FALSE;

  // This is always from Comparison DB to Current DB
  std::vector<st_CompareInfo *> *vpst_info = (std::vector<st_CompareInfo *> *)wParam;

  MultiCommands *pmulticmds = MultiCommands::Create(&m_core);

  // Make sure we don't add/rename password policies multiple times
  std::map<StringX, StringX> mapRenamedPolicies;
  std::vector<StringX> vs_PoliciesAdded;

  const StringX sxSync_DateTime = PWSUtil::GetTimeStamp(true).c_str();

  for (size_t index = 0; index < vpst_info->size(); index++) {
    st_CompareInfo *pst_info = (*vpst_info)[index];
    // Synchronise *pfromcore entry -> *ptocore entry
    PWScore *ptocore = pst_info->pcore0;
    PWScore *pfromcore = pst_info->pcore1;
    CUUID toUUID = pst_info->uuid0;
    CUUID fromUUID = pst_info->uuid1;

    ItemListIter fromPos = pfromcore->Find(fromUUID);
    ASSERT(fromPos != pfromcore->GetEntryEndIter());
    const CItemData *pfromEntry = &fromPos->second;

    ItemListIter toPos = ptocore->Find(toUUID);
    ASSERT(toPos != ptocore->GetEntryEndIter());
    CItemData *ptoEntry = &toPos->second;
    CItemData updtEntry(*ptoEntry);

    bool bUpdated(false);
    for (size_t i = 0; i < m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH].bsFields.size(); i++) {
      if (m_SaveAdvValues[CAdvancedDlg::COMPARESYNCH].bsFields.test(i)) {
        StringX sxValue = pfromEntry->GetFieldValue((CItemData::FieldType)i);

        // Special processing for password policies (default & named)
        if ((CItemData::FieldType)i == CItemData::POLICYNAME) {
          Command *pPolicyCmd = ptocore->ProcessPolicyName(pfromcore, updtEntry,
               mapRenamedPolicies, vs_PoliciesAdded, sxValue, bUpdated, sxSync_DateTime,
               IDSC_SYNCPOLICY);
          if (pPolicyCmd != NULL)
            pmulticmds->Add(pPolicyCmd);
        } else {
          if (sxValue != updtEntry.GetFieldValue((CItemData::FieldType)i)) {
            bUpdated = true;
            updtEntry.SetFieldValue((CItemData::FieldType)i, sxValue);
          }
        }
      }
    }

    if (bUpdated) {
      updtEntry.SetStatus(CItemData::ES_MODIFIED);
      Command *pcmd = EditEntryCommand::Create(ptocore, *ptoEntry, updtEntry);
      pcmd->SetNoGUINotify();
      pmulticmds->Add(pcmd);
    }
  }

  if (!pmulticmds->IsEmpty()) {
    CWaitCursor waitCursor;
    Execute(pmulticmds);
    waitCursor.Restore();

    RefreshViews();

    ChangeOkUpdate();
    return TRUE;
  }

  return FALSE;
}

void DboxMain::OnOK()
{
  PWS_LOGIT;

  SavePreferencesOnExit();

  // We will allow pgm Exit to close open dialogs IFF the DB is open in R-O and any open
  // dialogs can be closed
  bool bCloseOpenDialogs = IsDBReadOnly() && CPWDialog::GetDialogTracker()->VerifyCanCloseDialogs();

  if (bCloseOpenDialogs) {
    // Close all open dialogs - R-O mode ONLY + as above
    CPWDialog::GetDialogTracker()->CloseOpenDialogs();
  }

  int rc = SaveDatabaseOnExit(ST_NORMALEXIT);
  if (rc == PWScore::SUCCESS || rc == PWScore::USER_EXIT) {
    CleanUpAndExit();
  }
}

void RelativizePath(std::wstring &curfile)
{
  // If  IsUnderPw2go() && exec's drive == curfile's drive, remove
  // from latter's path. This supports DoK usage
  if (SysInfo::IsUnderPw2go()) {
    const std::wstring execDir = pws_os::getexecdir();
    std::wstring execDrive, dontCare;
    pws_os::splitpath(execDir, execDrive, dontCare, dontCare, dontCare);
    std::wstring fileDrive, fileDir, fileFile, fileExt;
    pws_os::splitpath(curfile, fileDrive, fileDir, fileFile, fileExt);
    ToUpper(fileDrive); ToUpper(execDrive);
    if (fileDrive == execDrive) {
      curfile = pws_os::makepath(L"", fileDir, fileFile, fileExt);
    }
  }
}

void DboxMain::SavePreferencesOnExit()
{
  PWS_LOGIT;

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
    _itow_s(m_nColumnTypeByIndex[iIndex], wc_buffer, 8, 10);
    _itow_s(m_nColumnWidthByIndex[iIndex], widths, 8, 10);
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
    if (m_core.IsDbOpen()) {
      std::wstring curFile = m_core.GetCurFile().c_str();
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
  PWS_LOGIT_ARGS("saveType=%d", saveType);

  if (!m_bOpen)
    return PWScore::SUCCESS;

  INT_PTR rc;

  if (saveType == ST_FAILSAFESAVE && m_core.HasDBChanged()) {
    // Save database as "<dbname>_YYYYMMDD_HHMMSS.fbak"
    std::wstring cs_newfile, cs_temp;
    std::wstring drv, dir, name, ext;
    const std::wstring path = m_core.GetCurFile().c_str();
    pws_os::splitpath(path.c_str(), drv, dir, name, ext);
    cs_temp = drv + dir + name;
    cs_temp += L"_";
    time_t now;
    time(&now);
    StringX cs_datetime = PWSUtil::ConvertToDateTimeString(now, PWSUtil::TMC_EXPORT_IMPORT);
    StringX nf = cs_temp.c_str() +
                     cs_datetime.substr( 0, 4) +  // YYYY
                     cs_datetime.substr( 5, 2) +  // MM
                     cs_datetime.substr( 8, 2) +  // DD
                     StringX(L"_") +
                     cs_datetime.substr(11, 2) +  // HH
                     cs_datetime.substr(14, 2) +  // MM
                     cs_datetime.substr(17, 2);   // SS
    cs_newfile = nf.c_str();
    cs_newfile += L".fbak";
    rc = m_core.WriteFile(cs_newfile.c_str(), m_core.GetReadFileVersion());
    BlockLogoffShutdown(rc == PWScore::SUCCESS ? false : true);

    return (int)rc;
  }

  if (saveType == ST_NORMALEXIT) {
    bool bAutoSave = true; // false if user saved or decided not to
    if (m_core.HasDBChanged()) {
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
    } // core.HasAnythingChanged()

    /*
    * Save silently (without asking user) iff:
    * 0. User didn't explicitly save OR say that he/she doesn't want to AND
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
    * ClearAppData).
    */

    if (bAutoSave && !m_core.IsReadOnly() &&
        (m_bEntryTimestampsChanged || m_core.HasGroupDisplayChanged()) &&
        m_core.GetNumEntries() > 0) {
      rc = Save(saveType);
      switch (rc) {
        case PWScore::SUCCESS:
          break;
        case PWScore::USER_EXIT:
          return PWScore::USER_EXIT;
        default:
          return PWScore::USER_CANCEL;
      }
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
  PWS_LOGIT_ARGS("bNormalExit=%s", bNormalExit ? L"true" : L"false");

  // Clear clipboard on Exit?  Yes if:
  // a. the app is minimized and the systemtray is enabled
  // b. the user has set the "ClearClipboardOnExit" pref
  // c. the system is shutting down, restarting or the user is logging off
  PWSprefs *prefs = PWSprefs::GetInstance();
  if ((!IsWindowVisible() && prefs->GetPref(PWSprefs::UseSystemTray)) ||
      prefs->GetPref(PWSprefs::ClearClipboardOnExit)) {
    ClearClipboardData();
  }

  // Reset core and clear ALL associated data
  m_core.ReInit();

  // Clear application data
  ClearAppData();

  // Cleanup here - doesn't work in ~DboxMain or ~CCoolMenuManager
  m_menuManager.Cleanup();

  // Clear out filters
  m_MapAllFilters.clear();

  if (m_pTrayIcon != NULL) {
    m_pTrayIcon->DestroyWindow();
    delete m_pTrayIcon;
  }

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
    OnMinimize();
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
    app.DelRegTree(hSubkey, V3_SUFFIX);
    app.DelRegTree(hSubkey, V4_SUFFIX);
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
    case WZAdvanced::EXPORT_ENTRYTEXT:
    case WZAdvanced::EXPORT_GROUPTEXT:
      uimsgftn = IDS_RPTEXPORTTEXT;
      break;
    case WZAdvanced::EXPORT_XML:
    case WZAdvanced::EXPORT_ENTRYXML:
    case WZAdvanced::EXPORT_GROUPXML:
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
    cs_buffer.Format(IDS_ADVANCEDOPTIONS, static_cast<LPCWSTR>(cs_temp));
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
      cs_temp.Format(IDS_ADVANCEDSUBSET, static_cast<LPCWSTR>(cs_Object),
                     static_cast<LPCWSTR>(cs_text),
                     static_cast<LPCWSTR>(st_SADV.subgroup_name.c_str()),
                     static_cast<LPCWSTR>(cs_case));
    }

    if (type == WZAdvanced::MERGE)
      return;

    cs_buffer.Format(IDS_ADVANCEDOPTIONS, static_cast<LPCWSTR>(cs_temp));
    pRpt->WriteLine((LPCWSTR)cs_buffer);
    pRpt->WriteLine();

    cs_temp.LoadString(uimsgftn);
    cs_buffer.Format(IDS_ADVANCEDFIELDS, static_cast<LPCWSTR>(cs_temp));
    pRpt->WriteLine((LPCWSTR)cs_buffer);

    cs_buffer = L"\t";
    // Non-time fields
    int ifields[] = {CItemData::PASSWORD, CItemData::NOTES, CItemData::URL,
                     CItemData::AUTOTYPE, CItemData::PWHIST, CItemData::POLICY,
                     CItemData::RUNCMD, CItemData::DCA, CItemData::SHIFTDCA, CItemData::EMAIL,
                     CItemData::PROTECTED, CItemData::SYMBOLS, CItemData::POLICYNAME,
                     CItemData::KBSHORTCUT, CItemData::ATTREF};
    UINT uimsgids[] = {IDS_COMPPASSWORD, IDS_COMPNOTES, IDS_COMPURL,
                       IDS_COMPAUTOTYPE, IDS_COMPPWHISTORY, IDS_COMPPWPOLICY,
                       IDS_COMPRUNCOMMAND, IDS_COMPDCA, IDS_COMPSHIFTDCA, IDS_COMPEMAIL,
                       IDS_COMPPROTECTED, IDS_COMPSYMBOLS, IDS_COMPPOLICYNAME,
                       IDS_COMPKBSHORTCUT, IDS_ATTREF};
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
