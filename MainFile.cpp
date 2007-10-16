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
#include "corelib/pwsprefs.h"
#include "corelib/util.h"
#include "corelib/PWSdirs.h"
#include "corelib/Report.h"

#include <sys/types.h>
#include <bitset>
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL
DboxMain::OpenOnInit(void)
{
  /*
    Routine to account for the differences between opening PSafe for
    the first time, and just opening a different database or
    un-minimizing the application
  */
  CMyString passkey;
  int rc = GetAndCheckPassword(m_core.GetCurFile(),
                               passkey, GCP_FIRST,
                               m_core.IsReadOnly(),
                               m_core.IsReadOnly());  // First
  int rc2 = PWScore::NOT_SUCCESS;

  switch (rc) {
  case PWScore::SUCCESS:
    rc2 = m_core.ReadCurFile(passkey);
#if !defined(POCKET_PC)
    m_titlebar = _T("Password Safe - ") + m_core.GetCurFile();
    UpdateSystemTray(UNLOCKED);
#endif
	CheckExpiredPasswords();
    break;
  case PWScore::CANT_OPEN_FILE:
    if (m_core.GetCurFile().IsEmpty()) {
      // Empty filename. Assume they are starting Password Safe
      // for the first time and don't confuse them.
      // fallthrough to New()
    } else {
      // Here if there was a filename saved from last invocation, but it couldn't
      // be opened. It was either removed or renamed, so ask the user what to do
	  CString cs_msg;
      cs_msg.Format(IDS_CANTOPENSAFE, m_core.GetCurFile());
      CGeneralMsgBox gmb;
      gmb.SetMsg(cs_msg);
      gmb.SetStandardIcon(MB_ICONQUESTION);
      gmb.AddButton(1, _T("Search"));
      gmb.AddButton(2, _T("New"));
      gmb.AddButton(3, _T("Exit"), TRUE, TRUE);
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

  BOOL retval(FALSE);
  switch (rc2) {
    case PWScore::BAD_DIGEST: {
      CString cs_msg; cs_msg.Format(IDS_FILECORRUPT, m_core.GetCurFile());
      CString cs_title(MAKEINTRESOURCE(IDS_FILEREADERROR));
      const int yn = MessageBox(cs_msg, cs_title, MB_YESNO|MB_ICONERROR);
      if (yn == IDNO) {
        CDialog::OnCancel();
        break;
      }
    }
    // DELIBERATE FALL-THRU if user chose YES
    case PWScore::SUCCESS:
      m_needsreading = false;
      startLockCheckTimer();
      UpdateSystemTray(UNLOCKED);
    	if (!m_bOpen) {
        // Previous state was closed - reset DCA in status bar
        SetDCAText();
	    }
	    m_bOpen = true;
      app.AddToMRU(m_core.GetCurFile());
      retval = TRUE;
      break;
#ifdef DEMO
    case PWScore::LIMIT_REACHED: {
      CString cs_msg, cs_msga(MAKEINTRESOURCE(IDS_LIMIT_MSGA)), cs_msgb(MAKEINTRESOURCE(IDS_LIMIT_MSGB));
      cs_msg.Format(IDS_LIMIT_MSG, MAXDEMO);
      CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE));
      if (MessageBox(cs_msg + cs_msga + cs_msgb, cs_title,
                     MB_YESNO | MB_ICONWARNING) == IDNO) {
        CDialog::OnCancel();
      }
      m_wndToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBUTTON_ADD,
                                                 FALSE);
      
      retval = TRUE;
    }
      break;
#endif
    default:
      if (!m_IsStartSilent)
        CDialog::OnCancel();
  }
  if (retval == TRUE) {
    m_core.SetDefUsername(PWSprefs::GetInstance()->
                GetPref(PWSprefs::DefaultUsername));
    m_core.SetUseDefUser(PWSprefs::GetInstance()->
                GetPref(PWSprefs::UseDefaultUser) ? true : false);
  }

  return retval;
}

void
DboxMain::OnNew()
{
  New();
}

int
DboxMain::New()
{
  int rc, rc2;

  if (m_core.IsChanged()) {
	  CString cs_temp;
    cs_temp.Format(IDS_SAVEDATABASE, m_core.GetCurFile());
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

  rc = NewFile();
  if (rc == PWScore::USER_CANCEL) {
    /*
      Everything stays as is...
      Worst case, they saved their file....
    */
    return PWScore::USER_CANCEL;
  }

  m_core.SetCurFile(_T("")); //Force a save as...
  m_core.ClearFileUUID();
#if !defined(POCKET_PC)
  m_titlebar.LoadString(IDS_UNTITLED);
  app.SetTooltipText(_T("PasswordSafe"));
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

int
DboxMain::NewFile(void)
{
  CPasskeySetup dbox_pksetup(this);
  //app.m_pMainWnd = &dbox_pksetup;
  INT_PTR rc = dbox_pksetup.DoModal();

  if (rc == IDCANCEL)
    return PWScore::USER_CANCEL;  //User cancelled password entry

  // Reset core
  m_core.ReInit();

  ClearData();
  PWSprefs::GetInstance()->SetDatabasePrefsToDefaults();
  const CMyString filename(m_core.GetCurFile());
  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.IsEmpty() && !m_core.IsReadOnly() && m_core.IsLockedFile(filename))
    m_core.UnlockFile(filename);
  m_core.SetReadOnly(false); // new file can't be read-only...
  m_core.NewFile(dbox_pksetup.m_passkey);
  m_needsreading = false;
  startLockCheckTimer();
  return PWScore::SUCCESS;
}

void
DboxMain::OnClose()
{
  Close();
}

int
DboxMain::Close()
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
	if( !m_core.GetCurFile().IsEmpty() ) {
		m_core.UnlockFile(m_core.GetCurFile());
		m_core.SetCurFile(_T(""));
	}

	// Clear all associated data
	ClearData();

  // Reset core
  m_core.ReInit();

	app.SetTooltipText(_T("PasswordSafe"));
	UpdateSystemTray(CLOSED);
	// Call UpdateMenuAndToolBar before UpdateStatusBar, as it sets m_bOpen
	UpdateMenuAndToolBar(false);
	UpdateStatusBar();
	m_titlebar = _T("Password Safe");
	return PWScore::SUCCESS;
}

void
DboxMain::OnOpen()
{
  int rc = Open();

  if (rc == PWScore::SUCCESS) {
  	if (!m_bOpen) {
  	  // Previous state was closed - reset DCA in status bar
      SetDCAText();
    }
    UpdateMenuAndToolBar(true);
  }
}

#if _MFC_VER > 1200
BOOL
#else
void
#endif
DboxMain::OnOpenMRU(UINT nID)
{
  UINT	uMRUItem = nID - ID_FILE_MRU_ENTRY1;

  CString mruItem = (*app.GetMRU())[uMRUItem];

  // Save just in case need to restore if user cancels
  const bool last_ro = m_core.IsReadOnly();
  m_core.SetReadOnly(false);
  // Read-only status will be set by GetAndCheckPassword
  int rc = Open( mruItem );
  if (rc == PWScore::SUCCESS) {
  	UpdateSystemTray(UNLOCKED);
    m_RUEList.ClearEntries();
  	if (!m_bOpen) {
  	  // Previous state was closed - reset DCA in status bar
      SetDCAText();
	}
    UpdateMenuAndToolBar(true);
  } else {
  	// Reset Read-only status
	m_core.SetReadOnly(last_ro);
  }

#if _MFC_VER > 1200
  return TRUE;
#endif
}

int
DboxMain::Open()
{
  int rc = PWScore::SUCCESS;
  CMyString newfile;
  CString cs_text(MAKEINTRESOURCE(IDS_CHOOSEDATABASE));
  CString dir = PWSdirs::GetSafeDir();

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
    fd.m_ofn.Flags &= ~OFN_READONLY;
    if (!dir.IsEmpty())
        fd.m_ofn.lpstrInitialDir = dir;
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
      newfile = (CMyString)fd.GetPathName();

      rc = Open( newfile );
      if ( rc == PWScore::SUCCESS ) {
        UpdateSystemTray(UNLOCKED);
        m_RUEList.ClearEntries();
        break;
      }
    } else {
      m_core.SetReadOnly(last_ro);
      return PWScore::USER_CANCEL;
    }
  }
  return rc;
}

int
DboxMain::Open( const CMyString &pszFilename )
{
    int rc;
    CMyString passkey, temp;
    CString cs_title, cs_text;

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
    if( !m_core.GetCurFile().IsEmpty() ) {
        m_core.UnlockFile(m_core.GetCurFile());
    }

    rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL);  // OK, CANCEL, HELP
    switch (rc) {
        case PWScore::SUCCESS:
            app.AddToMRU(pszFilename);
            m_bAlreadyToldUserNoSave = false;
            break; // Keep going...
        case PWScore::CANT_OPEN_FILE:
            temp.Format(IDS_SAFENOTEXIST, pszFilename);
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
    rc = m_core.ReadFile(pszFilename, passkey);
    switch (rc) {
        case PWScore::SUCCESS:
            break;
        case PWScore::CANT_OPEN_FILE:
            temp.Format(IDS_CANTOPENREADING, pszFilename);
            MessageBox(temp, cs_title, MB_OK|MB_ICONWARNING);
            /*
              Everything stays as is... Worst case,
              they saved their file....
            */
            return PWScore::CANT_OPEN_FILE;
        case PWScore::BAD_DIGEST: {
            temp.Format(IDS_FILECORRUPT, pszFilename);
            const int yn = MessageBox(temp, cs_title, MB_YESNO|MB_ICONERROR);
            if (yn == IDYES) {
                rc = PWScore::SUCCESS;
                break;
            } else
                return rc;
        }
#ifdef DEMO
        case PWScore::LIMIT_REACHED: {
            CString cs_msg; cs_msg.Format(IDS_LIMIT_MSG, MAXDEMO);
            CString cs_title(MAKEINTRESOURCE(IDS_LIMIT_TITLE));
            const int yn = MessageBox(cs_msg, cs_title,
                                      MB_YESNO|MB_ICONWARNING);
            if (yn == IDNO) {
              return PWScore::USER_CANCEL;
            }
            rc = PWScore::SUCCESS;
            m_wndToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBUTTON_ADD,
                                                       FALSE);
            break;
        }
#endif
        default:
            temp.Format(IDS_UNKNOWNERROR, pszFilename);
            MessageBox(temp, cs_title, MB_OK|MB_ICONERROR);
            return rc;
    }
    m_core.SetCurFile(pszFilename);
#if !defined(POCKET_PC)
    m_titlebar = _T("Password Safe - ") + m_core.GetCurFile();
#endif
    CheckExpiredPasswords();
    ChangeOkUpdate();
    RefreshList();
    SetInitialDatabaseDisplay();
    m_core.SetDefUsername(PWSprefs::GetInstance()->
                GetPref(PWSprefs::DefaultUsername));
    m_core.SetUseDefUser(PWSprefs::GetInstance()->
                GetPref(PWSprefs::UseDefaultUser) ? true : false);
    m_needsreading = false;
    return rc;
}

void
DboxMain::OnClearMRU()
{
	app.ClearMRU();
}

void
DboxMain::OnSave()
{
  Save();
}

int
DboxMain::Save()
{
  int rc;
  CString cs_title, cs_msg, cs_temp;
  PWSprefs *prefs = PWSprefs::GetInstance();

  // Save Application related preferences
  prefs->SaveApplicationPreferences();

  if (m_core.GetCurFile().IsEmpty())
    return SaveAs();


  if (m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
      if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
          int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
          int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
          CString userBackupPrefix = CString(prefs->GetPref(PWSprefs::BackupPrefixValue));
          CString userBackupDir = CString(prefs->GetPref(PWSprefs::BackupDir));
          if (!m_core.BackupCurFile(maxNumIncBackups, backupSuffix,
                                    userBackupPrefix, userBackupDir))
              AfxMessageBox(IDS_NOIBACKUP, MB_OK);
      }
  } else { // file version mis-match
  	CMyString NewName = PWSUtil::GetNewFileName(m_core.GetCurFile(), DEFAULT_SUFFIX );

    cs_msg.Format(IDS_NEWFORMAT, m_core.GetCurFile(), NewName);
    cs_title.LoadString(IDS_VERSIONWARNING);

    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONWARNING);
    gmb.AddButton(1, _T("Continue"));
    gmb.AddButton(2, _T("Cancel"), TRUE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      return PWScore::USER_CANCEL;
    m_core.SetCurFile(NewName);
#if !defined(POCKET_PC)
    m_titlebar = _T("Password Safe - ") + m_core.GetCurFile();
    app.SetTooltipText(m_core.GetCurFile());
#endif
  }
  rc = m_core.WriteCurFile();

  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, m_core.GetCurFile());
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
	cs_temp.Format(IDS_SAVEDATABASE, m_core.GetCurFile());
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

void
DboxMain::OnSaveAs()
{
  SaveAs();
}

int
DboxMain::SaveAs()
{
  INT_PTR rc;
  CMyString newfile;
  CString cs_msg, cs_title, cs_text, cs_temp;

  if (m_core.GetReadFileVersion() != PWSfile::VCURRENT &&
      m_core.GetReadFileVersion() != PWSfile::UNKNOWN_VERSION) {
    cs_msg.Format(IDS_NEWFORMAT2, m_core.GetCurFile());
    cs_title.LoadString(IDS_VERSIONWARNING);
    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    gmb.AddButton(1, _T("Continue"));
    gmb.AddButton(2, _T("Cancel"), TRUE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      return PWScore::USER_CANCEL;
  }
  //SaveAs-type dialog box
  CMyString cf(m_core.GetCurFile());
  if (cf.IsEmpty())
      cf.LoadString(IDS_DEFDBNAME); // reasonable default for first time user
  CMyString v3FileName = PWSUtil::GetNewFileName(cf, DEFAULT_SUFFIX );
  while (1) {
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   v3FileName,
                   OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                   |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                   SUFFIX_FILTERS
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    if (m_core.GetCurFile().IsEmpty())
      cs_text.LoadString(IDS_NEWNAME1);
    else
      cs_text.LoadString(IDS_NEWNAME2);
    fd.m_ofn.lpstrTitle = cs_text;
    CString dir = PWSdirs::GetSafeDir();
    if (!dir.IsEmpty())
        fd.m_ofn.lpstrInitialDir = dir;
    rc = fd.DoModal();
    if (m_inExit) {
        // If U3ExitNow called while in CFileDialog,
        // PostQuitMessage makes us return here instead
        // of exiting the app. Try resignalling 
        PostQuitMessage(0);
        return PWScore::USER_CANCEL;
    }
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }
  CMyString locker(_T("")); // null init is important here
  if (!m_core.LockFile(newfile, locker)) {
    cs_temp.Format(IDS_FILEISLOCKED, newfile, locker);
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
    m_core.UnlockFile(newfile);
    cs_temp.Format(IDS_CANTOPENWRITING, newfile);
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  if (!m_core.GetCurFile().IsEmpty())
    m_core.UnlockFile(m_core.GetCurFile());
  m_core.SetCurFile(newfile);
#if !defined(POCKET_PC)
  m_titlebar = _T("Password Safe - ") + m_core.GetCurFile();
  app.SetTooltipText(m_core.GetCurFile());
#endif
  SetChanged(Clear);
  ChangeOkUpdate();

  app.AddToMRU( newfile );

  if (m_core.IsReadOnly()) {
  	// reset read-only status (new file can't be read-only!)
  	// and so cause toolbar to be the correct version
  	m_core.SetReadOnly(false);
  }

  return PWScore::SUCCESS;
}

void
DboxMain::OnExportVx(UINT nID)
{
  INT_PTR rc;
  CMyString newfile;
  CString cs_text, cs_title, cs_temp;

  //SaveAs-type dialog box
  CMyString OldFormatFileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), _T("dat") );
  cs_text.LoadString(IDS_NAMEEXPORTFILE);
  while (1) {
    CFileDialog fd(FALSE,
                   DEFAULT_SUFFIX,
                   OldFormatFileName,
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
      newfile = (CMyString)fd.GetPathName();
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
    cs_temp.Format(IDS_CANTOPENWRITING, newfile);
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
  }
}

void
DboxMain::OnExportText()
{
    CExportTextDlg et;
    CString cs_text, cs_temp, cs_title;

    INT_PTR rc = et.DoModal();
    if (rc == IDOK) {
        CMyString newfile;
        CMyString pw(et.m_exportTextPassword);
        if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
            // do the export
            //SaveAs-type dialog box
            CMyString TxtFileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), _T("txt") );
            cs_text.LoadString(IDS_NAMETEXTFILE);
            while (1) {
                CFileDialog fd(FALSE,
                               _T("txt"),
                               TxtFileName,
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
                    newfile = (CMyString)fd.GetPathName();
                    break;
                } else
                    return;
            } // while (1)

            const CItemData::FieldBits bsExport = et.m_bsExport;
            const CString subgroup_name = et.m_subgroup_name;
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
                cs_temp.Format(IDS_CANTOPENWRITING, newfile);
                cs_title.LoadString(IDS_FILEWRITEERROR);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
        } else {
            AfxMessageBox(IDS_BADPASSKEY);
            Sleep(3000); // against automatic attacks
        }
    }
}

void
DboxMain::OnExportXML()
{
    CExportXMLDlg eXML;
    CString cs_text, cs_title, cs_temp;

    INT_PTR rc = eXML.DoModal();
    if (rc == IDOK) {
        CMyString newfile;
        CMyString pw(eXML.m_ExportXMLPassword);
        if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
            // do the export
            //SaveAs-type dialog box
            CMyString XMLFileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), _T("xml") );
            cs_text.LoadString(IDS_NAMEXMLFILE);
            while (1) {
                CFileDialog fd(FALSE,
                               _T("xml"),
                               XMLFileName,
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
                    newfile = (CMyString)fd.GetPathName();
                    break;
                } else
                    return;
            } // while (1)

            const CItemData::FieldBits bsExport = eXML.m_bsExport;
            const CString subgroup_name = eXML.m_subgroup_name;
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
                cs_temp.Format(IDS_CANTOPENWRITING, newfile);
                cs_title.LoadString(IDS_FILEWRITEERROR);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
        } else {
            AfxMessageBox(IDS_BADPASSKEY);
            Sleep(3000); // protect against automatic attacks
        }
    }
}

void
DboxMain::OnImportText()
{
    if (m_core.IsReadOnly()) // disable in read-only mode
        return;

    CImportDlg dlg;
    INT_PTR status = dlg.DoModal();

    if (status == IDCANCEL)
        return;

    CMyString ImportedPrefix(dlg.m_groupName);
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
        CString strError;
        CMyString TxtFileName = (CMyString)fd.GetPathName();
        int numImported = 0, numSkipped = 0;
        TCHAR delimiter = dlg.m_defimpdelim[0];

        /* Create report as we go */
        CReport rpt;
        rpt.StartReport(_T("Import_Text"), m_core.GetCurFile());
        CString cs_ReportFileName = rpt.GetReportFileName();
        cs_temp.Format(IDS_IMPORTFILE, _T("Text"), TxtFileName);
        rpt.WriteLine(cs_temp);
        rpt.WriteLine();

        rc = m_core.ImportPlaintextFile(ImportedPrefix, TxtFileName, strError, fieldSeparator,
                                        delimiter, numImported, numSkipped, rpt);

        cs_title.LoadString(IDS_FILEREADERROR);
        switch (rc) {
            case PWScore::CANT_OPEN_FILE:
                cs_temp.Format(IDS_CANTOPENREADING, TxtFileName);
            break;
            case PWScore::INVALID_FORMAT:
                cs_temp.Format(IDS_INVALIDFORMAT, TxtFileName);
            break;
            case PWScore::FAILURE:
            cs_title.LoadString(IDS_TEXTIMPORTFAILED);
            break;
            case PWScore::SUCCESS:
            default:
            {
                rpt.WriteLine();
                CString temp1, temp2 = _T("");
                temp1.Format(IDS_RECORDSIMPORTED, numImported, (numImported != 1) ? _T("s") : _T(""));
                rpt.WriteLine(temp1);
                if (numSkipped != 0) {
                    temp2.Format(IDS_RECORDSNOTREAD, numSkipped, (numSkipped != 1) ? _T("s") : _T(""));
                    rpt.WriteLine(temp2);
                }

                cs_title.LoadString(IDS_STATUS);
                MessageBox(temp1 + CString("\n") + temp2, cs_title, MB_ICONINFORMATION|MB_OK);
                ChangeOkUpdate();
            }
            RefreshList();
            break;
        } // switch
        if (rc != PWScore::SUCCESS) {
          CGeneralMsgBox gmb;
          gmb.SetTitle(cs_title);
          gmb.SetMsg(cs_temp);
          gmb.SetStandardIcon(MB_ICONEXCLAMATION);
          gmb.AddButton(1, _T("OK"), TRUE, TRUE);
          gmb.AddButton(2, _T("View Report"));
          INT_PTR rc = gmb.DoModal();
          if (rc == 2)
            ViewReport(cs_ReportFileName);
        }
    }
}

void
DboxMain::OnImportKeePass()
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
        CMyString KPsFileName = (CMyString)fd.GetPathName();
        rc = m_core.ImportKeePassTextFile(KPsFileName);
        switch (rc) {
            case PWScore::CANT_OPEN_FILE:
            {
                cs_temp.Format(IDS_CANTOPENREADING, KPsFileName);
                cs_title.LoadString(IDS_FILEOPENERROR);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
            break;
            case PWScore::INVALID_FORMAT:
            {
                cs_temp.Format(IDS_INVALIDFORMAT, KPsFileName);
                cs_title.LoadString(IDS_FILEREADERROR);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
            break;
            case PWScore::SUCCESS:
            default:
                RefreshList();
                ChangeOkUpdate();
                break;
        } // switch
    }
}

void
DboxMain::OnImportXML()
{
    if (m_core.IsReadOnly()) // disable in read-only mode
        return;

    CString cs_title, cs_temp, cs_text;
    CString XSDFilename = PWSdirs::GetXMLDir() + _T("pwsafe.xsd");

    if (XSDFilename.IsEmpty() || !PWSfile::FileExists(XSDFilename)) {
        cs_temp.LoadString(IDS_MISSINGXSD);
        cs_title.LoadString(IDS_CANTVALIDATEXML);
        MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
        return;
    }

    CImportXMLDlg dlg;
    INT_PTR status = dlg.DoModal();

    if (status == IDCANCEL)
        return;

    CString ImportedPrefix(dlg.m_groupName);

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
        CString strErrors, csErrors(_T(""));
        CString XMLFilename = (CMyString)fd.GetPathName();
        int numValidated, numImported;
        bool bBadUnknownFileFields, bBadUnknownRecordFields;
        CWaitCursor waitCursor;  // This may take a while!
        /* Create report as we go */
        CReport rpt;
        rpt.StartReport(_T("Import_XML"), m_core.GetCurFile());
        CString cs_ReportFileName = rpt.GetReportFileName();
        cs_temp.Format(IDS_IMPORTFILE, _T("XML"), XMLFilename);
        rpt.WriteLine(cs_temp);
        rpt.WriteLine();
        rc = m_core.ImportXMLFile(ImportedPrefix, XMLFilename, XSDFilename, strErrors,
                                  numValidated, numImported,
                                  bBadUnknownFileFields, bBadUnknownRecordFields, rpt);
        waitCursor.Restore();  // Restore normal cursor

        cs_title.LoadString(IDS_XMLIMPORTFAILED);
        switch (rc) {
            case PWScore::XML_FAILED_VALIDATION:
            {
                cs_temp.Format(IDS_FAILEDXMLVALIDATE, fd.GetFileName());
                csErrors = strErrors;
            }
              break;
            case PWScore::XML_FAILED_IMPORT:
            {
                cs_temp.Format(IDS_XMLERRORS, fd.GetFileName());
                csErrors = strErrors;
            }
              break;
            case PWScore::SUCCESS:
            {
                if (!strErrors.IsEmpty() ||
                    bBadUnknownFileFields || bBadUnknownRecordFields) {
                    if (!strErrors.IsEmpty())
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
                                   fd.GetFileName(), numValidated, numImported);

                    ChangeOkUpdate();
                } else {
                    cs_temp.Format(IDS_XMLIMPORTOK,
                                   numValidated, (numValidated != 1) ? _T("s") : _T(""),
                                   numImported, (numImported != 1) ? _T("s") : _T(""));
                    cs_title.LoadString(IDS_STATUS);
                    ChangeOkUpdate();
                }
            }
              RefreshList();
              break;
            default:
              ASSERT(0);
        } // switch

        // Finish Report
        rpt.WriteLine(cs_temp);
        rpt.WriteLine();
        rpt.WriteLine(csErrors);
        rpt.EndReport();

        if (rc != PWScore::SUCCESS || !strErrors.IsEmpty()) {
          CGeneralMsgBox gmb;
          gmb.SetTitle(cs_title);
          gmb.SetMsg(cs_temp);
          gmb.SetStandardIcon(MB_ICONEXCLAMATION);
          gmb.AddButton(1, _T("OK"), TRUE, TRUE);
          gmb.AddButton(2, _T("View Report"));
          INT_PTR rc = gmb.DoModal();
          if (rc == 2)
            ViewReport(cs_ReportFileName);
        } else
          MessageBox(cs_temp, cs_title, MB_ICONINFORMATION | MB_OK);
    }
}

int
DboxMain::Merge()
{
    int rc = PWScore::SUCCESS;
    CMyString newfile;
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
        CString dir = PWSdirs::GetSafeDir();
        if (!dir.IsEmpty())
            fd.m_ofn.lpstrInitialDir = dir;
        INT_PTR rc2 = fd.DoModal();
        if (m_inExit) {
            // If U3ExitNow called while in CFileDialog,
            // PostQuitMessage makes us return here instead
            // of exiting the app. Try resignalling 
            PostQuitMessage(0);
            return PWScore::USER_CANCEL;
        }
        if (rc2 == IDOK) {
            newfile = (CMyString)fd.GetPathName();

            rc = Merge(newfile);

            if (rc == PWScore::SUCCESS)
                break;
        } else
            return PWScore::USER_CANCEL;
    }

    return rc;
}

int
DboxMain::Merge(const CMyString &pszFilename) {
  /* open file they want to merge */
  CMyString passkey, temp;

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
    cs_temp.Format(IDS_CANTOPEN, othercore.GetCurFile());
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
    cs_temp.Format(IDS_CANTOPENREADING, pszFilename);
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

  /* Create report as we go */
  CReport rpt;
  rpt.StartReport(_T("Merge"), m_core.GetCurFile());
  cs_temp.Format(IDS_MERGINGDATABASE, pszFilename);
  rpt.WriteLine(cs_temp);

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
  uuid_array_t uuid;

  ItemListConstIter otherPos;
  for (otherPos = othercore.GetEntryIter();
       otherPos != othercore.GetEntryEndIter();
       otherPos++) {
    CItemData otherItem = othercore.GetEntry(otherPos);

    if (m_subgroup_set == BST_CHECKED &&
        !otherItem.Matches(m_subgroup_name, m_subgroup_object,
                           m_subgroup_function))
      continue;

    const CMyString otherGroup = otherItem.GetGroup();
    const CMyString otherTitle = otherItem.GetTitle();
    const CMyString otherUser = otherItem.GetUser();

    ItemListConstIter foundPos = m_core.Find(otherGroup, otherTitle, otherUser);
    if (foundPos != m_core.GetEntryEndIter()) {
      /* found a match, see if other fields also match */
      CItemData curItem = m_core.GetEntry(foundPos);
      if (otherItem.GetPassword() != curItem.GetPassword() ||
          otherItem.GetNotes() != curItem.GetNotes() ||
          otherItem.GetURL() != curItem.GetURL() ||
          otherItem.GetAutoType() != curItem.GetAutoType()) {

        /* have a match on title/user, but not on other fields
           add an entry suffixed with -merged-YYYYMMDD-HHMMSS */
        CTime curTime = CTime::GetCurrentTime();
        CMyString newTitle = otherItem.GetTitle();
        newTitle += _T("-merged-");
        CMyString timeStr = curTime.Format(_T("%Y%m%d-%H%M%S"));
        newTitle = newTitle + timeStr;

        /* note it as an issue for the user */
        CString warnMsg;
        warnMsg.Format(IDS_MERGECONFLICTS, 
                       otherItem.GetGroup(), 
                       otherItem.GetTitle(),
                       otherItem.GetUser(),
                       otherItem.GetGroup(), 
                       newTitle, 
                       otherItem.GetUser());

        /* log it */
        rpt.WriteLine(warnMsg);

        /* Check no conflict of unique uuid */
        otherItem.GetUUID(uuid);
        if (m_core.Find(uuid) != m_core.GetEntryEndIter())
          otherItem.CreateUUID();

        /* do it */
        otherItem.SetTitle(newTitle);
        m_core.AddEntry(otherItem);
        numConflicts++;
      }
    } else {
      /* didn't find any match...add it directly */
      /* Check no conflict of unique uuid */
      otherItem.GetUUID(uuid);
      if (m_core.Find(uuid) != m_core.GetEntryEndIter())
        otherItem.CreateUUID();

      m_core.AddEntry(otherItem);
      numAdded++;
    }
  } // iteration over other core's entries

  othercore.ClearData();

  waitCursor.Restore(); /* restore normal cursor */

  /* tell the user we're done & provide short merge report */
  int totalAdded = numAdded+numConflicts;
  CString resultStr;
  resultStr.Format(IDS_MERGECOMPLETED,
                   totalAdded,
                   totalAdded == 1 ? _T("y") : _T("ies"),
                   numConflicts,
                   numConflicts == 1 ? _T("") : _T("s"));
  cs_title.LoadString(IDS_MERGECOMPLETED2);
  MessageBox(resultStr, cs_title, MB_OK);
  rpt.WriteLine(resultStr);
  rpt.EndReport();

  ChangeOkUpdate();
  RefreshList();

  return rc;
}

void
DboxMain::OnProperties()
{
  CProperties dlg;

  dlg.m_database = CString(m_core.GetCurFile());

  dlg.m_databaseformat.Format(_T("%d.%02d"),
                              m_core.GetHeader().m_nCurrentMajorVersion,
                              m_core.GetHeader().m_nCurrentMinorVersion);

  CStringArray aryGroups;
  app.m_core.GetUniqueGroups(aryGroups);
  dlg.m_numgroups.Format(_T("%d"), aryGroups.GetSize());

  dlg.m_numentries.Format(_T("%d"), m_core.GetNumEntries());

  time_t twls = m_core.GetHeader().m_whenlastsaved;
  if (twls == 0) {
	  dlg.m_whenlastsaved.LoadString(IDS_UNKNOWN);
	  dlg.m_whenlastsaved.Trim();
  } else {
    dlg.m_whenlastsaved =
      CString(PWSUtil::ConvertToDateTimeString(twls, TMC_EXPORT_IMPORT));
  }

  if (m_core.GetHeader().m_lastsavedby.IsEmpty() &&
      m_core.GetHeader().m_lastsavedon.IsEmpty()) {
	  dlg.m_wholastsaved.LoadString(IDS_UNKNOWN);
	  dlg.m_whenlastsaved.Trim();
  } else {
    CString user = m_core.GetHeader().m_lastsavedby.IsEmpty() ?
      _T("?") : m_core.GetHeader().m_lastsavedby;
    CString host = m_core.GetHeader().m_lastsavedon.IsEmpty() ?
      _T("?") : m_core.GetHeader().m_lastsavedon;
    dlg.m_wholastsaved.Format(_T("%s on %s"), user, host);
  }

  CString wls = m_core.GetHeader().m_whatlastsaved;
  if (wls.IsEmpty()) {
    dlg.m_whatlastsaved.LoadString(IDS_UNKNOWN);
    dlg.m_whenlastsaved.Trim();
  } else
    dlg.m_whatlastsaved = wls;

  uuid_array_t file_uuid_array, ref_uuid_array;
  memset(ref_uuid_array, 0x00, sizeof(ref_uuid_array));
  m_core.GetFileUUID(file_uuid_array);

  if (memcmp(file_uuid_array, ref_uuid_array, sizeof(file_uuid_array)) == 0)
    wls = _T("N/A");
  else
    wls.Format(_T("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
               file_uuid_array[0],  file_uuid_array[1],  file_uuid_array[2],  file_uuid_array[3],
               file_uuid_array[4],  file_uuid_array[5],  file_uuid_array[6],  file_uuid_array[7],
               file_uuid_array[8],  file_uuid_array[9],  file_uuid_array[10], file_uuid_array[11],
               file_uuid_array[12], file_uuid_array[13], file_uuid_array[14], file_uuid_array[15]);
  dlg.m_file_uuid = wls;

  int num = m_core.GetNumRecordsWithUnknownFields();
  if (num != 0 || m_core.HasHeaderUnknownFields()) {
    const CString cs_Yes(MAKEINTRESOURCE(IDS_YES));
    const CString cs_No(MAKEINTRESOURCE(IDS_NO));
    const CString cs_HdrYesNo = m_core.HasHeaderUnknownFields() ?
      cs_Yes : cs_No;

    dlg.m_unknownfields.Format(IDS_UNKNOWNFIELDS, cs_HdrYesNo);
    if (num == 0)
      dlg.m_unknownfields += cs_No + _T(")");
    else {
      wls.Format(_T("%d"), num);
      dlg.m_unknownfields += wls + _T(")");
    }
  } else {
    dlg.m_unknownfields.LoadString(IDS_NONE);
  }

  dlg.DoModal();
}

void
DboxMain::OnMerge()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  Merge();
}

void
DboxMain::OnCompare()
{
  INT_PTR rc = PWScore::SUCCESS;
  if (m_core.GetCurFile().IsEmpty()) {
    AfxMessageBox(IDS_NOCOMPAREFILE, MB_OK|MB_ICONWARNING);
    return;
  }

  CMyString cs_file2;
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
    CString dir = PWSdirs::GetSafeDir();
    if (!dir.IsEmpty())
      fd.m_ofn.lpstrInitialDir = dir;
      rc = fd.DoModal();
      if (m_inExit) {
        // If U3ExitNow called while in CFileDialog,
        // PostQuitMessage makes us return here instead
        // of exiting the app. Try resignalling 
        PostQuitMessage(0);
        return;
      }
    if (rc == IDOK) {
      cs_file2 = (CMyString)fd.GetPathName();
      //Check that this file isn't the current one!
      if (cs_file2 == m_core.GetCurFile()) {
        //It is the same damn file!
        AfxMessageBox(IDS_COMPARESAME, MB_OK|MB_ICONWARNING);
      } else {
        const CMyString cs_file1(m_core.GetCurFile());
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

int
DboxMain::Compare(const CMyString &cs_Filename1, const CMyString &cs_Filename2)
{
	// open file they want to Compare
	int rc = PWScore::SUCCESS;

	CMyString passkey, temp;
	CString cs_title, cs_text;
  PWScore othercore;

  // Reading a new file changes the preferences!
  const CMyString cs_SavePrefString(PWSprefs::GetInstance()->Store());

	// OK, CANCEL, HELP, ADVANCED + (nolonger force R/O) + use othercore
  rc = GetAndCheckPassword(cs_Filename2, passkey,
                               GCP_ADVANCED, // OK, CANCEL, HELP
                               true,         // readonly
                               false,        // user can change readonly status
                               &othercore,   // Use other core
                               ADV_COMPARE);   // Advanced type
	switch (rc) {
  case PWScore::SUCCESS:
    break; // Keep going...
  case PWScore::CANT_OPEN_FILE:
    temp.Format(IDS_CANTOPEN, cs_Filename2);
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
	}

  // Not really needed but...
  othercore.ClearData();

	rc = othercore.ReadFile(cs_Filename2, passkey);

  switch (rc) {
  case PWScore::SUCCESS:
    break;
  case PWScore::CANT_OPEN_FILE:
    temp.Format(IDS_CANTOPENREADING, cs_Filename2);
    MessageBox(temp, cs_title, MB_OK | MB_ICONWARNING);
    break;
  case PWScore::BAD_DIGEST:
    {
      temp.Format(IDS_FILECORRUPT, cs_Filename2);
      const int yn = MessageBox(temp, cs_title, MB_YESNO|MB_ICONERROR);
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
    temp.Format(IDS_UNKNOWNERROR, cs_Filename2);
    MessageBox(temp, cs_title, MB_OK|MB_ICONERROR);
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


	// Put up hourglass...this might take a while
	CWaitCursor waitCursor;

	/*
    Purpose:
    Compare entries from comparison database (compCore) with current database (m_core)

    Algorithm:
    Foreach entry in current database {
    Find in comparison database
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
    Find in current database
    if not found
    save & increment numOnlyInComp
    }
	*/

	int numOnlyInCurrent = 0;
	int numOnlyInComp = 0;
	int numConflicts = 0;
  int numIdentical = 0;

	CItemData::FieldBits bsConflicts(0);
	st_CompareData st_data;

	ItemListIter currentPos;
  for (currentPos = m_core.GetEntryIter();
       currentPos != m_core.GetEntryEndIter();
       currentPos++) {
		CItemData currentItem = m_core.GetEntry(currentPos);

    if (m_subgroup_set == BST_UNCHECKED ||
        currentItem.Matches(m_subgroup_name, m_subgroup_object,
                            m_subgroup_function)) {
      const CMyString currentGroup = currentItem.GetGroup();
      const CMyString currentTitle = currentItem.GetTitle();
      const CMyString currentUser = currentItem.GetUser();

      ItemListIter foundPos = othercore.Find(currentGroup,
                                             currentTitle, currentUser);
      if (foundPos != othercore.GetEntryEndIter()) {
        // found a match, see if all other fields also match
        // Difference flags:
        /*
                First word (values in square brackets taken from ItemData.h)
                1... ....  NAME     [0x0] - n/a - depreciated
                .1.. ....  UUID     [0x1] - n/a - unique
                ..1. ....  GROUP    [0x2] - not checked - must be identical
                ...1 ....  TITLE    [0x3] - not checked - must be identical
                .... 1...  USER     [0x4] - not checked - must be identical
                .... .1..  NOTES    [0x5]
                .... ..1.  PASSWORD [0x6]
                .... ...1  CTIME    [0x7] - not checked - immaterial

                Second word
                1... ....  PMTIME   [0x8] - not checked - immaterial
                .1.. ....  ATIME    [0x9] - not checked - immaterial
                ..1. ....  LTIME    [0xa]
                ...1 ....  POLICY   [0xb] - not yet implemented
                .... 1...  RMTIME   [0xc] - not checked - immaterial
                .... .1..  URL      [0xd]
                .... ..1.  AUTOTYPE [0xe]
                .... ...1  PWHIST   [0xf]
        */

        bsConflicts.reset();

        CItemData compItem = othercore.GetEntry(foundPos);
        if (m_bsFields.test(CItemData::NOTES) &&
            currentItem.GetNotes() != compItem.GetNotes())
          bsConflicts.flip(CItemData::NOTES);
        if (m_bsFields.test(CItemData::PASSWORD) &&
            currentItem.GetPassword() != compItem.GetPassword())
          bsConflicts.flip(CItemData::PASSWORD);
        if (m_bAdvanced && m_bsFields.test(CItemData::CTIME) &&
            currentItem.GetCTime() != compItem.GetCTime())
          bsConflicts.flip(CItemData::CTIME);
        if (m_bAdvanced && m_bsFields.test(CItemData::PMTIME) &&
            currentItem.GetPMTime() != compItem.GetPMTime())
          bsConflicts.flip(CItemData::PMTIME);
        if (m_bAdvanced && m_bsFields.test(CItemData::ATIME) &&
            currentItem.GetATime() != compItem.GetATime())
          bsConflicts.flip(CItemData::ATIME);
        if (m_bsFields.test(CItemData::LTIME) &&
            currentItem.GetLTime() != compItem.GetLTime())
          bsConflicts.flip(CItemData::LTIME);
        if (m_bAdvanced && m_bsFields.test(CItemData::RMTIME) &&
            currentItem.GetRMTime() != compItem.GetRMTime())
          bsConflicts.flip(CItemData::RMTIME);
        if (m_bsFields.test(CItemData::URL) &&
            currentItem.GetURL() != compItem.GetURL())
          bsConflicts.flip(CItemData::URL);
        if (m_bsFields.test(CItemData::AUTOTYPE) &&
            currentItem.GetAutoType() != compItem.GetAutoType())
          bsConflicts.flip(CItemData::AUTOTYPE);
        if (m_bsFields.test(CItemData::PWHIST) &&
            currentItem.GetPWHistory() != compItem.GetPWHistory())
          bsConflicts.flip(CItemData::PWHIST);

        if (bsConflicts.any()) {
          numConflicts++;
          st_data.pos0 = currentPos;
          st_data.pos1 = foundPos;
          st_data.bsDiffs = bsConflicts;
          st_data.group = currentGroup;
          st_data.title = currentTitle;
          st_data.user = currentUser;
          st_data.column = -1;
          st_data.unknflds0 = currentItem.NumberUnknownFields() > 0;
          st_data.unknflds1 = compItem.NumberUnknownFields() > 0;
          st_data.id = numConflicts;
          list_Conflicts.push_back(st_data);
        } else {
          numIdentical++;
          st_data.pos0 = currentPos;
          st_data.pos1 = foundPos;
          st_data.bsDiffs = bsConflicts;
          st_data.group = currentGroup;
          st_data.title = currentTitle;
          st_data.user = currentUser;
          st_data.column = -1;
          st_data.unknflds0 = currentItem.NumberUnknownFields() > 0;
          st_data.unknflds1 = compItem.NumberUnknownFields() > 0;
          st_data.id = numIdentical;
          list_Identical.push_back(st_data);
        }
      } else {
        /* didn't find any match... */
        numOnlyInCurrent++;
        st_data.pos0 = currentPos;
        st_data.pos1 = othercore.GetEntryEndIter();
        st_data.bsDiffs.reset();
        st_data.group = currentGroup;
        st_data.title = currentTitle;
        st_data.user = currentUser;
        st_data.column = 0;
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
        !compItem.Matches(m_subgroup_name, m_subgroup_object,
                          m_subgroup_function)) {
      const CMyString compGroup = compItem.GetGroup();
      const CMyString compTitle = compItem.GetTitle();
      const CMyString compUser = compItem.GetUser();
  
      if (m_core.Find(compGroup, compTitle, compUser) ==
          m_core.GetEntryEndIter()) {
        /* didn't find any match... */
        numOnlyInComp++;
        st_data.pos0 = m_core.GetEntryEndIter();
        st_data.pos1 = compPos;
        st_data.bsDiffs.reset();
        st_data.group = compGroup;
        st_data.title = compTitle;
        st_data.user = compUser;
        st_data.column = 1;
        st_data.unknflds0 = false;
        st_data.unknflds1 = compItem.NumberUnknownFields() > 0;
        st_data.id = numOnlyInComp;
        list_OnlyInComp.push_back(st_data);
      }
    }
	} // iteration over other core's element

	waitCursor.Restore(); // restore normal cursor

	// tell the user we're done & provide short Compare report
  CString resultStr(_T("")), buffer;
	cs_title.LoadString(IDS_COMPARECOMPLETE);
	buffer.Format(IDS_COMPARESTATISTICS, cs_Filename1, cs_Filename2);

	if (numOnlyInCurrent == 0 && numOnlyInComp == 0 && numConflicts == 0) {
		cs_text.LoadString(IDS_IDENTICALDATABASES);
		resultStr += buffer + cs_text;
		MessageBox(resultStr, cs_title, MB_OK);
  } else {
    CCompareResultsDlg CmpRes(this, list_OnlyInCurrent, list_OnlyInComp, 
                              list_Conflicts, list_Identical, 
                              m_bsFields, &m_core, &othercore);

    CmpRes.m_cs_Filename1 = cs_Filename1;
    CmpRes.m_cs_Filename2 = cs_Filename2;
    CmpRes.m_bOriginalDBReadOnly = m_core.IsReadOnly();
    CmpRes.m_bComparisonDBReadOnly = othercore.IsReadOnly();

    CmpRes.DoModal();
    if (CmpRes.m_OriginalDBChanged) {
      FixListIndexes();
      RefreshList();
    }

    if (CmpRes.m_ComparisonDBChanged) {
      SaveCore(&othercore);
    }
  }

  if (othercore.IsLockedFile(othercore.GetCurFile()))
    othercore.UnlockFile(othercore.GetCurFile());

  othercore.ClearData();
  othercore.SetCurFile(_T(""));
  
  // Reset database preferences - first to defaults then add saved changes!
  PWSprefs::GetInstance()->Load(cs_SavePrefString);

	return rc;
}

int
DboxMain::SaveCore(PWScore *pcore)
{
  // Stolen from Save!
  int rc;
  CString cs_title, cs_msg, cs_temp;
  PWSprefs *prefs = PWSprefs::GetInstance();

  if (pcore->GetReadFileVersion() == PWSfile::VCURRENT) {
    if (prefs->GetPref(PWSprefs::BackupBeforeEverySave)) {
      int maxNumIncBackups = prefs->GetPref(PWSprefs::BackupMaxIncremented);
      int backupSuffix = prefs->GetPref(PWSprefs::BackupSuffix);
      CString userBackupPrefix = CString(prefs->GetPref(PWSprefs::BackupPrefixValue));
      CString userBackupDir = CString(prefs->GetPref(PWSprefs::BackupDir));
      if (!pcore->BackupCurFile(maxNumIncBackups, backupSuffix,
                                userBackupPrefix, userBackupDir))
        AfxMessageBox(IDS_NOIBACKUP, MB_OK);
    }
  } else { // file version mis-match
    CMyString NewName = PWSUtil::GetNewFileName(pcore->GetCurFile(), DEFAULT_SUFFIX );
    cs_msg.Format(IDS_NEWFORMAT, pcore->GetCurFile(), NewName);
    cs_title.LoadString(IDS_VERSIONWARNING);

    CGeneralMsgBox gmb;
    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_msg);
    gmb.SetStandardIcon(MB_ICONWARNING);
    gmb.AddButton(1, _T("Continue"));
    gmb.AddButton(2, _T("Cancel"), FALSE, TRUE);
    INT_PTR rc = gmb.DoModal();
    if (rc == 2)
      return PWScore::USER_CANCEL;
    pcore->SetCurFile(NewName);
  }
  rc = pcore->WriteCurFile();

  if (rc == PWScore::CANT_OPEN_FILE) {
    cs_temp.Format(IDS_CANTOPENWRITING, pcore->GetCurFile());
    cs_title.LoadString(IDS_FILEWRITEERROR);
    MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  pcore->SetChanged(false);
  return PWScore::SUCCESS;
}

LRESULT
DboxMain::OnProcessCompareResultFunction(WPARAM wParam, LPARAM lParam)
{
  st_CompareInfo *st_info;
  LRESULT lres(FALSE);

  st_info = (st_CompareInfo *)wParam;

  PWScore *pcore = (st_info->column == 0) ? st_info->pcore0 : st_info->pcore1;
  ItemListIter pos = (st_info->column == 0) ? st_info->pos0 : st_info->pos1;
  switch ((int)lParam) {
    case CCompareResultsDlg::EDIT:
      lres = EditCompareResult(pcore, pos);
      break;      
    case CCompareResultsDlg::VIEW:
      lres = ViewCompareResult(pcore, pos);
      break;
    case CCompareResultsDlg::COPY_TO_ORIGINALDB:
      lres = CopyCompareResult(st_info->pcore1, st_info->pcore0, st_info->pos1);
      break;
    case CCompareResultsDlg::COPY_TO_COMPARISONDB:
      lres = CopyCompareResult(st_info->pcore0, st_info->pcore1, st_info->pos0);
      break;
    default:
      ASSERT(0);
  }
  return lres;
}

LRESULT
DboxMain::ViewCompareResult(PWScore *pcore, ItemListIter pos)
{  
  CItemData *ci = &pcore->GetEntry(pos);

  // View the correct entry and make sure R/O
  bool bSaveRO = m_core.IsReadOnly();
  m_core.SetReadOnly(true);

  EditItem(ci);

  m_core.SetReadOnly(bSaveRO);

  return FALSE;
}

LRESULT
DboxMain::EditCompareResult(PWScore *pcore, ItemListIter pos)
{
  CItemData *ci = &pcore->GetEntry(pos);

  // Edit the correct entry
  return EditItem(ci) ? TRUE : FALSE;
}

LRESULT
DboxMain::CopyCompareResult(PWScore *pfromcore, PWScore *ptocore,
                            ItemListIter fromPos)
{
  // Copy *pfromcore -> *ptocore entry at fromPos

  ItemListIter toPos, touuidPos;
  CMyString group, title, user, notes, password, url, autotype, pwhistory;
  uuid_array_t fromUUID;
  time_t ct, at, lt, pmt, rmt;
  int nfromUnknownRecordFields;

  const CItemData *fromEntry = &pfromcore->GetEntry(fromPos);

  fromEntry->GetUUID(fromUUID);
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
  fromEntry->GetLTime(lt);
  fromEntry->GetPMTime(pmt);
  fromEntry->GetRMTime(rmt);
  nfromUnknownRecordFields = fromEntry->NumberUnknownFields();

  touuidPos = ptocore->Find(fromUUID);

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
    toEntry->SetLTime(lt);
    toEntry->SetPMTime(pmt);
    toEntry->SetRMTime(rmt);

    // If the UUID is not in use, copy it too, otherwise reuse current
    if (touuidPos == ptocore->GetEntryEndIter())
      toEntry->SetUUID(fromUUID);

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
    if (touuidPos == ptocore->GetEntryEndIter())
      temp.SetUUID(fromUUID);
    else
      temp.CreateUUID();

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
    temp.SetLTime(lt);
    temp.SetPMTime(pmt);
    temp.SetRMTime(rmt);
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

void
DboxMain::OnOK() 
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
  prefs->SetPref(PWSprefs::ListColumns, cs_columns);
  prefs->SetPref(PWSprefs::ColumnWidths, cs_columnswidths);

  //Store current filename for next time...
  if (!m_core.GetCurFile().IsEmpty())
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
  // b. the user has set the "DontAskMinimizeClearYesNo" pref
  // c. the system is shutting down, restarting or the user is logging off
  if ((!IsWindowVisible() && prefs->GetPref(PWSprefs::UseSystemTray)) ||
      prefs->GetPref(PWSprefs::DontAskMinimizeClearYesNo) ||
      (m_iSessionEndingStatus == IDYES)) {
		ClearClipboardData();
  }

  // wipe data, save prefs, go home.
  ClearData();
  prefs->SaveApplicationPreferences();
  CDialog::OnOK();
}

void
DboxMain::OnCancel()
{
  // If system tray is enabled, cancel (X on title bar) closes
  // window, else exit application
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray))
    ShowWindow(SW_MINIMIZE);
  else
    OnOK();
}

void
DboxMain::SaveDisplayStatus()
{
	vector <bool> v = GetGroupDisplayStatus(); // update it
	m_core.SetDisplayStatus(v); // store it
}

void
DboxMain::RestoreDisplayStatus()
{
	const vector<bool> &displaystatus = m_core.GetDisplayStatus();    

	if (!displaystatus.empty())
    SetGroupDisplayStatus(displaystatus);
}

vector<bool>
DboxMain::GetGroupDisplayStatus()
{
	HTREEITEM hItem = NULL;
  vector<bool> v;

  while ( NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem)) ) {
    if (m_ctlItemTree.ItemHasChildren(hItem)) {
      bool state = (m_ctlItemTree.GetItemState(hItem, TVIS_EXPANDED)
                    & TVIS_EXPANDED) != 0;
      v.push_back(state);
    }
  }
  return v;
}

void
DboxMain::SetGroupDisplayStatus(const vector<bool> &displaystatus)
{
	HTREEITEM hItem = NULL;
  unsigned i = 0;
	while ( NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem)) ) {
		if (m_ctlItemTree.ItemHasChildren(hItem)) {
      if (i < displaystatus.size())
        m_ctlItemTree.Expand(hItem,
                             displaystatus[i] ? TVE_EXPAND : TVE_COLLAPSE);
      i++;
    }
	}
}
