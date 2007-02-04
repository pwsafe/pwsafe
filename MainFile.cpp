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
#include "ImportXMLErrDlg.h"
#include "Properties.h"
#include "corelib/pwsprefs.h"
#include "corelib/util.h"
#include "corelib/PWSdirs.h"

#include <sys/types.h>
#include <bitset>

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
  int rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, GCP_FIRST);  // First
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
      int rc3 = MessageBox(cs_msg, AfxGetAppName(), (MB_ICONQUESTION | MB_YESNOCANCEL));
      switch (rc3) {
      case IDYES:
        rc2 = Open();
        break;
      case IDNO:
        rc2 = New();
        break;
      case IDCANCEL:
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

  switch (rc2) {
  case PWScore::BAD_DIGEST: {
    CString cs_msg; cs_msg.Format(IDS_FILECORRUPT, m_core.GetCurFile());
    CString cs_title(MAKEINTRESOURCE(IDS_FILEREADERROR));
    const int yn = MessageBox(cs_msg, cs_title, MB_YESNO|MB_ICONERROR);
    if (yn == IDNO) {
      CDialog::OnCancel();
      return FALSE;
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
    return TRUE;
  default:
    if (!m_IsStartSilent)
      CDialog::OnCancel();
    return FALSE;
  }
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
  if (rc == PWScore::USER_CANCEL)
    /*
      Everything stays as is...
      Worst case, they saved their file....
    */
    return PWScore::USER_CANCEL;

  m_core.SetCurFile(_T("")); //Force a save as...
#if !defined(POCKET_PC)
  m_titlebar.LoadString(IDS_UNTITLED);
  app.SetTooltipText(_T("PasswordSafe"));
#endif
  ChangeOkUpdate();

  return PWScore::SUCCESS;
}

int
DboxMain::NewFile(void)
{
  CPasskeySetup dbox_pksetup(this);
  //app.m_pMainWnd = &dbox_pksetup;
  int rc = dbox_pksetup.DoModal();

  if (rc == IDCANCEL)
    return PWScore::USER_CANCEL;  //User cancelled password entry

  ClearData();
  const CMyString filename(m_core.GetCurFile());
  // The only way we're the locker is if it's locked & we're !readonly
  if (!filename.IsEmpty() && !m_IsReadOnly && m_core.IsLockedFile(filename))
    m_core.UnlockFile(filename);
  SetReadOnly(false); // new file can't be read-only...
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
  const bool last_ro = m_IsReadOnly;
  m_IsReadOnly = false;
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
	SetReadOnly(last_ro);
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
    const bool last_ro = m_IsReadOnly; // restore if user cancels
    SetReadOnly(fd.GetReadOnlyPref() == TRUE);
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();

      rc = Open( newfile );

      if ( rc == PWScore::SUCCESS ) {
        UpdateSystemTray(UNLOCKED);
        m_RUEList.ClearEntries();
        break;
      }
    } else {
      SetReadOnly(last_ro);
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
  if (pszFilename == m_core.GetCurFile() && !m_needsreading)
	{
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
    if (yn == IDYES)
      break;
    else
      return rc;
  }
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
  return PWScore::SUCCESS;
}

void
DboxMain::OnClearMRU()
{
	app.ClearMRU();
}

void
DboxMain::OnSave()
{
  SaveDisplayStatus();
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
    if (MessageBox(cs_msg, cs_title, MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
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
  // offer to save existing database ifit was modified.
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
  SaveDisplayStatus();
  SaveAs();
}

int
DboxMain::SaveAs()
{
  int rc;
  CMyString newfile;
  CString cs_msg, cs_title, cs_text, cs_temp;

  if (m_core.GetReadFileVersion() != PWSfile::VCURRENT &&
      m_core.GetReadFileVersion() != PWSfile::UNKNOWN_VERSION) {
    cs_msg.Format(IDS_NEWFORMAT2, m_core.GetCurFile());
    cs_title.LoadString(IDS_VERSIONWARNING);
    if (MessageBox(cs_msg, cs_title,
                   MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
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
  rc = m_core.WriteFile(newfile);
  if (rc == PWScore::CANT_OPEN_FILE) {
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

  if (m_IsReadOnly) {
  	// reset read-only status (new file can't be read-only!)
  	// and so cause toolbar to be the correct version
  	SetReadOnly(false);
  }

  return PWScore::SUCCESS;
}

void
DboxMain::OnExportVx(UINT nID)
{
  int rc;
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
  bool bwrite_header;
  CString cs_text, cs_temp, cs_title;

  int rc = et.DoModal();
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

	  bwrite_header = (et.m_export_hdr == 1);
	  const std::bitset<16> bsExport = et.m_bsExport;
	  const CString subgroup = et.m_subgroup;
	  const int iObject = et.m_subgroup_object;
	  const int iFunction = et.m_subgroup_function;
	  TCHAR delimiter = _T('\0');
      if (et.m_querysetexpdelim == 1)
        delimiter = et.m_defexpdelim[0];

      ItemList sortedItemList;
      MakeSortedItemList(sortedItemList);
      
      rc = m_core.WritePlaintextFile(newfile, bwrite_header, 
                          bsExport, subgroup, iObject, 
                          iFunction, delimiter, &sortedItemList);
      sortedItemList.RemoveAll(); // cleanup soonest

      if (rc == PWScore::CANT_OPEN_FILE)        {
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

    int rc = eXML.DoModal();
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

            TCHAR delimiter;
            delimiter = eXML.m_defexpdelim[0];
      
            ItemList sortedItemList;
            MakeSortedItemList(sortedItemList);
            rc = m_core.WriteXMLFile(newfile, delimiter, &sortedItemList);

            sortedItemList.RemoveAll(); // cleanup soonest

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
    if (m_IsReadOnly) // disable in read-only mode
        return;

    CImportDlg dlg;
    int status = dlg.DoModal();

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
    int rc = fd.DoModal();
    if (m_inExit) {
        // If U3ExitNow called while in CFileDialog,
        // PostQuitMessage makes us return here instead
        // of exiting the app. Try resignalling 
        PostQuitMessage(0);
        return;
    }
    if (rc == IDOK) {
        CString strErrors;
        CMyString newfile = (CMyString)fd.GetPathName();
        int numImported = 0, numSkipped = 0;
        bool bimport_preV3 = (dlg.m_import_preV3 == 1);
        TCHAR delimiter;
        if (dlg.m_querysetimpdelim == 1) {
            delimiter = dlg.m_defimpdelim[0];
        } else {
            delimiter = TCHAR('\0');
        }
        rc = m_core.ImportPlaintextFile(ImportedPrefix, newfile, strErrors, fieldSeparator,
                                        delimiter, numImported, numSkipped, bimport_preV3);

        cs_title.LoadString(IDS_FILEREADERROR);
        switch (rc) {
            case PWScore::CANT_OPEN_FILE:
            {
                cs_temp.Format(IDS_CANTOPENREADING, newfile);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
            break;
            case PWScore::INVALID_FORMAT:
            {
                cs_temp.Format(IDS_INVALIDFORMAT , newfile);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
            break;
            case PWScore::SUCCESS:
            default:
            {
                CString temp1, temp2 = _T("");
                temp1.Format(IDS_RECORDSREAD, numImported, (numImported != 1) ? _T("s") : _T(""));
                if (numSkipped != 0)
                    temp2.Format(IDS_RECORDSNOTREAD, numSkipped, (numSkipped != 1) ? _T("s") : _T(""));

                cs_title.LoadString(IDS_STATUS);
                MessageBox(temp1 + temp2, cs_title, MB_ICONINFORMATION|MB_OK);
                ChangeOkUpdate();
            }
            RefreshList();
            break;
        } // switch
    }
}

void
DboxMain::OnImportKeePass()
{
    if (m_IsReadOnly) // disable in read-only mode
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
    int rc = fd.DoModal();
    if (m_inExit) {
        // If U3ExitNow called while in CFileDialog,
        // PostQuitMessage makes us return here instead
        // of exiting the app. Try resignalling 
        PostQuitMessage(0);
        return;
    }
    if (rc == IDOK) {
        CMyString newfile = (CMyString)fd.GetPathName();
        rc = m_core.ImportKeePassTextFile(newfile);
        switch (rc) {
            case PWScore::CANT_OPEN_FILE:
            {
                cs_temp.Format(IDS_CANTOPENREADING, newfile);
                cs_title.LoadString(IDS_FILEOPENERROR);
                MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
            }
            break;
            case PWScore::INVALID_FORMAT:
            {
                cs_temp.Format(IDS_INVALIDFORMAT, newfile);
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
    if (m_IsReadOnly) // disable in read-only mode
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
    int status = dlg.DoModal();

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

    int rc = fd.DoModal();
    if (m_inExit) {
        // If U3ExitNow called while in CFileDialog,
        // PostQuitMessage makes us return here instead
        // of exiting the app. Try resignalling 
        PostQuitMessage(0);
        return;
    }
    if (rc == IDOK) {
        CString strErrors;
        CString XMLFilename = (CMyString)fd.GetPathName();
        int numValidated, numImported;
        CWaitCursor waitCursor;  // This may take a while!
        rc = m_core.ImportXMLFile(ImportedPrefix, XMLFilename, XSDFilename, strErrors, numValidated, numImported);
        waitCursor.Restore();  // Restore normal cursor

        switch (rc) {
            case PWScore::XML_FAILED_VALIDATION:
            {
                CImportXMLErrDlg dlg;
                dlg.m_strActionText.Format(IDS_FAILEDXMLVALIDATE, XMLFilename);
                dlg.m_strResultText = strErrors;
                dlg.DoModal();
            }
            break;
            case PWScore::XML_FAILED_IMPORT:
            {
                CImportXMLErrDlg dlg;
                dlg.m_strActionText.Format(IDS_XMLERRORS, XMLFilename);
                dlg.m_strResultText = strErrors;
                dlg.DoModal();
            }
            break;
            case PWScore::SUCCESS:
            {
                if (!strErrors.IsEmpty()) {
                    cs_temp.Format(IDS_XMLIMPORTWITHERRORS,
                                   XMLFilename, numValidated, numImported);
                    CImportXMLErrDlg dlg;
                    dlg.m_strActionText = cs_temp;
                    dlg.m_strResultText = strErrors;
                    dlg.DoModal();
                } else {
                    cs_temp.Format(IDS_XMLIMPORTOK,
                                   numValidated, (numValidated != 1) ? _T("s") : _T(""),
                                   numImported, (numImported != 1) ? _T("s") : _T(""));
                    cs_title.LoadString(IDS_STATUS);
                    MessageBox(cs_temp, cs_title, MB_ICONINFORMATION|MB_OK);
                    ChangeOkUpdate();
                }
            }
            RefreshList();
            break;
            default:
                ASSERT(0);
        } // switch
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

            rc = Merge( newfile );

            if ( rc == PWScore::SUCCESS )
                break;
        }
        else
            return PWScore::USER_CANCEL;
    }

    return rc;
}

int
DboxMain::Merge(const CMyString &pszFilename) {
  /* open file they want to merge */
  int rc = PWScore::SUCCESS;
  CMyString passkey, temp;

  //Check that this file isn't already open
  if (pszFilename == m_core.GetCurFile()) {
      //It is the same damn file
      AfxMessageBox(IDS_ALREADYOPEN, MB_OK|MB_ICONWARNING);
      return PWScore::ALREADY_OPEN;
	}

  // Save current read-only status around opening merge fil R-O
  bool bCurrentFileIsReadOnly = m_IsReadOnly;
  // Force input database into read-only status
  rc = GetAndCheckPassword(pszFilename, passkey,
                           GCP_NORMAL, // OK, CANCEL, HELP
                           true);  // force readonly

  // Restore original database read-only status
  SetReadOnly(bCurrentFileIsReadOnly);

  CString cs_temp, cs_title;
  switch (rc) {
	case PWScore::SUCCESS:
      app.AddToMRU(pszFilename);
      break; // Keep going...
	case PWScore::CANT_OPEN_FILE:
      cs_temp.Format(IDS_CANTOPEN, m_core.GetCurFile());
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

  PWScore otherCore;
  otherCore.ReadFile(pszFilename, passkey);

  if (rc == PWScore::CANT_OPEN_FILE) {
      cs_temp.Format(IDS_CANTOPENREADING, pszFilename);
      cs_title.LoadString(IDS_FILEREADERROR);
      MessageBox(cs_temp, cs_title, MB_OK|MB_ICONWARNING);
      /*
		Everything stays as is... Worst case,
		they saved their file....
      */
      return PWScore::CANT_OPEN_FILE;
	}

  otherCore.SetCurFile(pszFilename);

  /* Put up hourglass...this might take a while */
  CWaitCursor waitCursor;

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
    add to m_core with new title suffixed with -merged-HHMMSS-DDMMYY
    else
    add to m_core directly
  */
  int numAdded = 0;
  int numConflicts = 0;

  POSITION otherPos = otherCore.GetFirstEntryPosition();
  while (otherPos) {
    CItemData otherItem = otherCore.GetEntryAt(otherPos);
    const CMyString otherGroup = otherItem.GetGroup();
    const CMyString otherTitle = otherItem.GetTitle();
    const CMyString otherUser = otherItem.GetUser();

    POSITION foundPos = m_core.Find(otherGroup, otherTitle, otherUser);
    if (foundPos) {
      /* found a match, see if other fields also match */
      CItemData curItem = m_core.GetEntryAt(foundPos);
      if (otherItem.GetPassword() != curItem.GetPassword() ||
          otherItem.GetNotes() != curItem.GetNotes() ||
          otherItem.GetURL() != curItem.GetURL() ||
          otherItem.GetAutoType() != curItem.GetAutoType()) {
        /* have a match on title/user, but not on other fields
           add an entry suffixed with -merged-HHMMSS-DDMMYY */
        CTime curTime = CTime::GetCurrentTime();
        CMyString newTitle = otherItem.GetTitle();
        newTitle += _T("-merged-");
        CMyString timeStr = curTime.Format(_T("%H%M%S-%m%d%y"));
        newTitle = newTitle + timeStr;

        /* note it as an issue for the user */
        CString warnMsg;
        warnMsg.Format(IDS_MERGECONFLICTS, otherItem.GetGroup(), otherItem.GetTitle(), otherItem.GetUser(),
          newTitle, otherItem.GetUser());

        /* tell the user the bad news */
        CString cs_title;
		cs_title.LoadString(IDS_MERGECONFLICTS2);
        MessageBox(warnMsg, cs_title, MB_OK|MB_ICONWARNING);

        /* do it */
        otherItem.SetTitle(newTitle);
        m_core.AddEntryToTail(otherItem);

        numConflicts++;
      }
    } else {
      /* didn't find any match...add it directly */
      m_core.AddEntryToTail(otherItem);
      numAdded++;
    }

    otherCore.GetNextEntry(otherPos);
  }

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

  ChangeOkUpdate();
  RefreshList();

  return rc;
}

void
DboxMain::OnProperties()
{
  CProperties dlg;

  dlg.m_database = CString(m_core.GetCurFile());

  int nmajor = m_core.GetCurrentMajorVersion();
  int nminor = m_core.GetCurrentMinorVersion();
  dlg.m_databaseformat.Format(_T("%d.%02d"), nmajor, nminor);

  CStringArray aryGroups;
  app.m_core.GetUniqueGroups(aryGroups);
  dlg.m_numgroups.Format(_T("%d"), aryGroups.GetSize());

  dlg.m_numentries.Format(_T("%d"), m_core.GetNumEntries());

  CString wls = m_core.GetWhenLastSaved();
  if (wls.GetLength() != 8) {
	  dlg.m_whenlastsaved.LoadString(IDS_UNKNOWN);
	  dlg.m_whenlastsaved.Trim();
  } else {
	  long t;
	  TCHAR *lpszWLS = wls.GetBuffer(9);
#if _MSC_VER >= 1400
	  int iread = _stscanf_s(lpszWLS, _T("%8x"), &t);
#else
	  int iread = _stscanf(lpszWLS, _T("%8x"), &t);
#endif
	  wls.ReleaseBuffer();
	  ASSERT(iread == 1);
      dlg.m_whenlastsaved =
          CString(PWSUtil::ConvertToDateTimeString((time_t) t, TMC_EXPORT_IMPORT));
  }

  wls = m_core.GetWhoLastSaved();
  if (wls.GetLength() == 0) {
	  dlg.m_wholastsaved.LoadString(IDS_UNKNOWN);
	  dlg.m_whenlastsaved.Trim();
  } else {
	  int ulen;
	  TCHAR *lpszWLS = wls.GetBuffer(wls.GetLength() + 1);
#if _MSC_VER >= 1400
	  int iread = _stscanf_s(lpszWLS, _T("%4x"), &ulen);
#else
	  int iread = _stscanf(lpszWLS, _T("%4x"), &ulen);
#endif
	wls.ReleaseBuffer();
	ASSERT(iread == 1);
	dlg.m_wholastsaved.Format(_T("%s on %s"), wls.Mid(4, ulen), wls.Mid(ulen + 4));
  }

  wls = m_core.GetWhatLastSaved();
  if (wls.GetLength() == 0) {
	dlg.m_whatlastsaved.LoadString(IDS_UNKNOWN);
	dlg.m_whenlastsaved.Trim();
  } else
	dlg.m_whatlastsaved = wls;

  dlg.DoModal();
}

void
DboxMain::OnMerge()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  Merge();
}

void
DboxMain::OnCompare()
{
	int rc = PWScore::SUCCESS;
	if (m_core.GetCurFile().IsEmpty()) {
		AfxMessageBox(IDS_NOCOMPAREFILE, MB_OK|MB_ICONWARNING);
		return;
	}

	CMyString file2;
	CString cs_text(MAKEINTRESOURCE(IDS_PICKCOMPAREFILE));

	//Open-type dialog box
	while (1) {
		CFileDialog fd(TRUE,
                       DEFAULT_SUFFIX,
                       NULL,
                       OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                       SUFFIX_FILTERS
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
			file2 = (CMyString)fd.GetPathName();
			//Check that this file isn't the current one!
			if (file2 == m_core.GetCurFile()) {
				//It is the same damn file!
				AfxMessageBox(IDS_COMPARESAME, MB_OK|MB_ICONWARNING);
			} else {
				bool bSave_RO_Status = m_IsReadOnly;  // Compare makes files R/O
				rc = Compare(file2);
				SetReadOnly(bSave_RO_Status);
				break;
			}
		} else {
			rc = PWScore::USER_CANCEL;
			break;
		}
	}

	return;
}

// The following structure needed for compare when record is in
// both databases but there are differences
struct st_Conflict {
  POSITION cPos;
  POSITION nPos;
  std::bitset<16> bsDiffs;
};

int
DboxMain::Compare(const CMyString &pszFilename)
{
	// open file they want to Compare
	int rc = PWScore::SUCCESS;
	CMyString passkey, temp;
	CString cs_title, cs_text;

	// OK, CANCEL, HELP + force READ-ONLY
	rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL, true);
	switch (rc) {
		case PWScore::SUCCESS:
			break; // Keep going...
		case PWScore::CANT_OPEN_FILE:
			temp.Format(IDS_CANTOPEN, m_core.GetCurFile());
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

	PWScore compCore;
	compCore.ReadFile(pszFilename, passkey);

	if (rc == PWScore::CANT_OPEN_FILE) {
		temp.Format(IDS_CANTOPENREADING, pszFilename);
		cs_title.LoadString(IDS_FILEREADERROR);
		MessageBox(temp, cs_title, MB_OK|MB_ICONWARNING);
		return PWScore::CANT_OPEN_FILE;
	}

	CList<POSITION, POSITION&> list_OnlyInCurrent;
	CList<POSITION, POSITION&> list_OnlyInComp;
	CList<st_Conflict, st_Conflict&> list_Conflicts;

	compCore.SetCurFile(pszFilename);

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

	std::bitset<16> bsConflicts (0);
	st_Conflict * st_diff;

	POSITION currentPos = m_core.GetFirstEntryPosition();
	while (currentPos) {
		CItemData currentItem = m_core.GetEntryAt(currentPos);
		const CMyString currentGroup = currentItem.GetGroup();
		const CMyString currentTitle = currentItem.GetTitle();
		const CMyString currentUser = currentItem.GetUser();

		POSITION foundPos = compCore.Find(currentGroup, currentTitle, currentUser);
		if (foundPos) {
			// found a match, see if all other fields also match
			// Difference flags:
			/*
				First word (values in square brackets taken from ItemData.h)
				1... ....	NAME		[0x0] - n/a - depreciated
				.1.. ....	UUID		[0x1] - n/a - unique
				..1. ....	GROUP		[0x2] - not checked - must be identical
				...1 ....	TITLE		[0x3] - not checked - must be identical
				.... 1...	USER		[0x4] - not checked - must be identical
				.... .1..	NOTES		[0x5]
				.... ..1.	PASSWORD	[0x6]
        .... ...1 CTIME    [0x7] - not checked - immaterial

				Second word
        1... .... PMTIME   [0x8] - not checked - immaterial
        .1.. .... ATIME    [0x9] - not checked - immaterial
				..1. ....	LTIME		[0xa]
				...1 ....	POLICY		[0xb] - not yet implemented
        .... 1... RMTIME   [0xc] - not checked - immaterial
				.... .1..	URL			[0xd]
				.... ..1.	AUTOTYPE	[0xe]
				.... ...1	PWHIST		[0xf]
			*/

			bsConflicts.reset();

			CItemData compItem = compCore.GetEntryAt(foundPos);
			if (currentItem.GetNotes() != compItem.GetNotes())
				bsConflicts.flip(CItemData::NOTES);
			if (currentItem.GetPassword() != compItem.GetPassword())
				bsConflicts.flip(CItemData::PASSWORD);
		/*	if (currentItem.GetCTime() != compItem.GetCTime())
				bsConflicts.flip(CItemData::CTIME);
			if (currentItem.GetPMTime() != compItem.GetPMTime())
				bsConflicts.flip(CItemData::PMTIME);
			if (currentItem.GetATime() != compItem.GetATime())
				bsConflicts.flip(CItemData::ATIME); */
			if (currentItem.GetLTime() != compItem.GetLTime())
				bsConflicts.flip(CItemData::LTIME);
        /*  if (currentItem.GetRMTime() != compItem.GetRMTime())
				bsConflicts.flip(CItemData::RMTIME); */
			if (currentItem.GetURL() != compItem.GetURL())
				bsConflicts.flip(CItemData::URL);
			if (currentItem.GetAutoType() != compItem.GetAutoType())
				bsConflicts.flip(CItemData::AUTOTYPE);
			if (currentItem.GetPWHistory() != compItem.GetPWHistory())
				bsConflicts.flip(CItemData::PWHIST);

			if (bsConflicts.any()) {
					st_diff = new st_Conflict;
					st_diff->cPos = currentPos;
					st_diff->nPos = foundPos;
					st_diff->bsDiffs = bsConflicts;
					list_Conflicts.AddTail(*st_diff);
                    delete[] st_diff;

					numConflicts++;
			}
			} else {
				/* didn't find any match... */
				list_OnlyInCurrent.AddTail(currentPos);
				numOnlyInCurrent++;
		}

		m_core.GetNextEntry(currentPos);
	}

	POSITION compPos = compCore.GetFirstEntryPosition();
	while (compPos) {
		CItemData compItem = compCore.GetEntryAt(compPos);
		const CMyString compGroup = compItem.GetGroup();
		const CMyString compTitle = compItem.GetTitle();
		const CMyString compUser = compItem.GetUser();

		if (!m_core.Find(compGroup, compTitle, compUser)) {
			/* didn't find any match... */
			list_OnlyInComp.AddTail(compPos);
			numOnlyInComp++;
		}

		compCore.GetNextEntry(compPos);
	}

	waitCursor.Restore(); // restore normal cursor

	// tell the user we're done & provide short Compare report
	CString resultStr(_T("")), buffer;
	cs_title.LoadString(IDS_COMPARECOMPLETE);
	buffer.Format(IDS_COMPARESTATISTICS, m_core.GetCurFile(), pszFilename);

	if (numOnlyInCurrent == 0 && numOnlyInComp == 0 && numConflicts == 0) {
		cs_text.LoadString(IDS_IDENTICALDATABASES);
		resultStr += buffer + cs_text;
		MessageBox(resultStr, cs_title, MB_OK);
		goto exit;
	}

	resultStr += buffer;
	buffer.Format(IDS_COMPAREONLY1, numOnlyInCurrent);
	resultStr += buffer;
	buffer.Format(IDS_COMPAREONLY2, numOnlyInComp);
	resultStr += buffer;
	buffer.Format(IDS_COMPAREBOTHDIFF, numConflicts);
	resultStr += buffer;
	buffer.LoadString(IDS_COMPARECOPYRESULT);
	resultStr += buffer;
	int mb_rc = MessageBox(resultStr, cs_title, MB_YESNO | MB_DEFBUTTON2);

	if (mb_rc == IDNO)
		goto exit;

	resultStr.Empty();
	if (numOnlyInCurrent > 0) {
		buffer.Format(IDS_COMPAREENTRIES1, m_core.GetCurFile());
		resultStr += buffer;
		POSITION currentPos = list_OnlyInCurrent.GetHeadPosition();
		while (currentPos) {
			POSITION corepos = list_OnlyInCurrent.GetAt(currentPos);
			CItemData currentItem = m_core.GetEntryAt(corepos);
			const CMyString currentGroup = currentItem.GetGroup();
			const CMyString currentTitle = currentItem.GetTitle();
			const CMyString currentUser = currentItem.GetUser();

			buffer.Format(IDS_COMPARESTATS, currentGroup, currentTitle, currentUser);
			resultStr += buffer;

			list_OnlyInCurrent.GetNext(currentPos);
		}
		resultStr += _T("\n");
	}

	if (numOnlyInComp > 0) {
		buffer.Format(IDS_COMPAREENTRIES2, pszFilename);
		resultStr += buffer;
		POSITION compPos = list_OnlyInComp.GetHeadPosition();
		while (compPos) {
			POSITION corepos = list_OnlyInComp.GetAt(compPos);
			CItemData compItem = compCore.GetEntryAt(corepos);
			const CMyString compGroup = compItem.GetGroup();
			const CMyString compTitle = compItem.GetTitle();
			const CMyString compUser = compItem.GetUser();

			buffer.Format(IDS_COMPARESTATS, compGroup, compTitle, compUser);
			resultStr += buffer;

			list_OnlyInComp.GetNext(compPos);
		}
		resultStr += _T("\n");
	}

	if (numConflicts > 0) {
		buffer.Format(IDS_COMPAREBOTHDIFF2, m_core.GetCurFile(), pszFilename);
		resultStr += buffer;
		POSITION conflictPos = list_Conflicts.GetHeadPosition();

		const CString csx_password(MAKEINTRESOURCE(IDS_COMPPASSWORD));
		const CString csx_notes(MAKEINTRESOURCE(IDS_COMPNOTES));
		const CString csx_url(MAKEINTRESOURCE(IDS_COMPURL));
		const CString csx_autotype(MAKEINTRESOURCE(IDS_COMPAUTOTYPE));
		const CString csx_ctime(MAKEINTRESOURCE(IDS_COMPCTIME));
		const CString csx_pmtime(MAKEINTRESOURCE(IDS_COMPPMTIME));
		const CString csx_atime(MAKEINTRESOURCE(IDS_COMPATIME));
		const CString csx_ltime(MAKEINTRESOURCE(IDS_COMPLTIME));
		const CString csx_rmtime(MAKEINTRESOURCE(IDS_COMPRMTIME));
		const CString csx_pwhistory(MAKEINTRESOURCE(IDS_COMPPWHISTORY));

		while (conflictPos) {
			st_Conflict st_diff = list_Conflicts.GetAt(conflictPos);
			CItemData currentItem = m_core.GetEntryAt(st_diff.cPos);
			const CMyString currentGroup = currentItem.GetGroup();
			const CMyString currentTitle = currentItem.GetTitle();
			const CMyString currentUser = currentItem.GetUser();

			buffer.Format(IDS_COMPARESTATS2, currentGroup, currentTitle, currentUser);
			resultStr += buffer;

			if (st_diff.bsDiffs.test(CItemData::PASSWORD)) resultStr += csx_password;
			if (st_diff.bsDiffs.test(CItemData::NOTES)) resultStr += csx_notes;
			if (st_diff.bsDiffs.test(CItemData::URL)) resultStr += csx_url;
			if (st_diff.bsDiffs.test(CItemData::AUTOTYPE)) resultStr += csx_autotype;
			if (st_diff.bsDiffs.test(CItemData::CTIME)) resultStr += csx_ctime;
			if (st_diff.bsDiffs.test(CItemData::PMTIME)) resultStr += csx_pmtime;
			if (st_diff.bsDiffs.test(CItemData::ATIME)) resultStr += csx_atime;
			if (st_diff.bsDiffs.test(CItemData::LTIME)) resultStr += csx_ltime;
			if (st_diff.bsDiffs.test(CItemData::RMTIME)) resultStr += csx_rmtime;
			if (st_diff.bsDiffs.test(CItemData::PWHIST)) resultStr += csx_pwhistory;

			list_Conflicts.GetNext(conflictPos);
		}
	}

	app.SetClipboardData(resultStr);

exit:
	list_OnlyInCurrent.RemoveAll();
	list_OnlyInComp.RemoveAll();
	list_Conflicts.RemoveAll();

	return rc;
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

#if !defined(POCKET_PC)
  if (!IsIconic()) {
    CRect rect;
    GetWindowRect(&rect);
    prefs->SetPrefRect(rect.top, rect.bottom, rect.left, rect.right);
  }
#endif
  prefs->SetPref(PWSprefs::SortedColumn, m_iSortedColumn);
  prefs->SetPref(PWSprefs::SortAscending, m_bSortAscending);

  // If MaintainDateTimeStamps set and not read-only,
  // save without asking user: "they get what it says on the tin"
  // Note that if database was cleared (e.g., locked), it might be
  // possible to save an empty list :-(
  // Protect against this both here and in OnSize (where we minimize
  // & possibly ClearData).
  if (!m_IsReadOnly && m_bTSUpdated && m_core.GetNumEntries() > 0)
    Save();

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
			rc2 = Save();
			if (rc2 != PWScore::SUCCESS)
				return;
		case IDNO:
			break;
	}
  } // core.IsChanged()

  //Store current filename for next time...
  if (!m_core.GetCurFile().IsEmpty())
    prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());

  // Clear clipboard on Exit?  Yes if:
  // a. the app is minimized and the systemtray is enabled
  // b. the user has set the "DontAskMinimizeClearYesNo" pref
  // c. the system is shutting down, restarting or the user is logging off
  if ((!IsWindowVisible() && prefs->GetPref(PWSprefs::UseSystemTray)) ||
      prefs->GetPref(PWSprefs::DontAskMinimizeClearYesNo) ||
      (m_iSessionEndingStatus == IDYES)) {
		app.ClearClipboardData();
  }

  ClearData();

  // Save Application related preferences
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
	const int max_displaystatus_size = m_ctlItemTree.GetCount();

	TCHAR *p_char_displaystatus = new TCHAR[max_displaystatus_size];

	memset(p_char_displaystatus, 0, max_displaystatus_size);

	int i = 0;

	GroupDisplayStatus(p_char_displaystatus, i, true);

	m_core.SetDisplayStatus(p_char_displaystatus, i);
	delete[] p_char_displaystatus;
}

void
DboxMain::RestoreDisplayStatus()
{
	CString cs_displaystatus = m_core.GetDisplayStatus();

	if (cs_displaystatus.IsEmpty())
		return;

	TCHAR *p_char_displaystatus = cs_displaystatus.GetBuffer(cs_displaystatus.GetLength());

	int i = 0;

	GroupDisplayStatus(p_char_displaystatus, i, false);
}

void
DboxMain::GroupDisplayStatus(TCHAR *p_char_displaystatus, int &i, bool bSet)
{
	const TCHAR c_one = TCHAR('1');
	HTREEITEM hItem = NULL;
	while ( NULL != (hItem = m_ctlItemTree.GetNextTreeItem(hItem)) ) {
		if (m_ctlItemTree.ItemHasChildren(hItem)) {
			const CString cs_text = m_ctlItemTree.GetItemText(hItem);
			if (bSet) {
				if (m_ctlItemTree.GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) {
					p_char_displaystatus[i] = TCHAR('1');
				} else {
					p_char_displaystatus[i] = TCHAR('0');
				}
			} else {
				if (memcmp(&p_char_displaystatus[i], &c_one, sizeof(TCHAR)) == 0) {
					m_ctlItemTree.Expand(hItem, TVE_EXPAND);
				}
			}
			i++;
		}
	}
}
