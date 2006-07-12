/// file MainFile.cpp
//
// File-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

// dialog boxen
#include "DboxMain.h"

#include "PasskeySetup.h"
#include "TryAgainDlg.h"
#include "ExportText.h"
#include "ExportXML.h"
#include "ImportDlg.h"
#include "ImportXMLDlg.h"
#include "ImportXMLErrDlg.h"
#include "corelib/pwsprefs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_SUFFIX _T("psafe3")
#define SUFFIX_FILTERS _T("Password Safe Databases (*.psafe3; *.dat)|*.psafe3; *.dat|")

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
    m_title = "Password Safe - " + m_core.GetCurFile();
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
      CMyString msg = _T("The database ") + m_core.GetCurFile();
      msg += _T(" couldn't be opened.\nDo you wish to look for it elsewhere (Yes), ");
      msg += _T("create a new database (No), or exit (Cancel)?");
      int rc3 = MessageBox(msg, AfxGetAppName(), (MB_ICONQUESTION | MB_YESNOCANCEL));
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
    CMyString msg(m_core.GetCurFile());
    msg += _T("\n\nFile corrupt or truncated!\n");
    msg += _T("Data may have been lost or modified.\nContinue anyway?");
    const int yn = MessageBox(msg, _T("File Read Error"),
                              MB_YESNO|MB_ICONERROR);
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
	m_saveMRU = true;
    return TRUE;
  default:
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
    CMyString temp =
      _T("Do you want to save changes to the password database: ")
      + m_core.GetCurFile()
      + _T("?");

    rc = MessageBox(temp,
                    AfxGetAppName(),
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
  m_title = _T("Password Safe - <Untitled>");
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
DboxMain::OnOpen()
{
  Open();
}

int
DboxMain::Open()
{
  int rc = PWScore::SUCCESS;
  CMyString newfile;

  //Open-type dialog box
  while (1) {
    CFileDialog fd(TRUE,
                   DEFAULT_SUFFIX,
                   NULL,
                   OFN_FILEMUSTEXIST|OFN_LONGNAMES,
                   SUFFIX_FILTERS
                   _T("Password Safe Backups (*.bak)|*.bak|")
                   _T("All files (*.*)|*.*|")
                   _T("|"),
                   this);
    fd.m_ofn.lpstrTitle = _T("Please Choose a Database to Open:");
    rc = fd.DoModal();
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
  m_saveMRU = true;
  return rc;
}

int
DboxMain::Open( const CMyString &pszFilename )
{
  int rc;
  CMyString passkey, temp;

  //Check that this file isn't already open
  if (pszFilename == m_core.GetCurFile() && !m_needsreading)
	{
      //It is the same damn file
      MessageBox(_T("That file is already open."),
                 _T("Oops!"),
                 MB_OK|MB_ICONWARNING);
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

  // clear the data before loading the new file
  ClearData();

  rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL);  // OK, CANCEL, HELP
  switch (rc) {
  case PWScore::SUCCESS:
    app.AddToMRU(pszFilename);
    break; // Keep going...
  case PWScore::CANT_OPEN_FILE:
    temp = m_core.GetCurFile()
      + _T("\n\nCan't open file. Please choose another.");
    MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
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

  temp = pszFilename;
  rc = m_core.ReadFile(pszFilename, passkey);
  switch (rc) {
  case PWScore::SUCCESS:
    break;
  case PWScore::CANT_OPEN_FILE:
    temp += _T("\n\nCould not open file for reading!");
    MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
    /*
      Everything stays as is... Worst case,
      they saved their file....
    */
    return PWScore::CANT_OPEN_FILE;
  case PWScore::BAD_DIGEST: {
    temp += _T("\n\nFile corrupt or truncated!\n");
    temp += _T("Data may have been lost or modified.\nContinue anyway?");
    const int yn = MessageBox(temp, _T("File Read Error"),
                              MB_YESNO|MB_ICONERROR);
    if (yn == IDYES)
      break;
    else
      return rc;
  }
  default:
    temp += _T("Unknown error");
    MessageBox(temp, _T("File read error."), MB_OK|MB_ICONERROR);
	return rc;
  }
  m_core.SetCurFile(pszFilename);
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - ") + m_core.GetCurFile();
#endif
  CheckExpiredPasswords();
  ChangeOkUpdate();
  RefreshList();

  return PWScore::SUCCESS;
}

void
DboxMain::OnClearMRU()
{
	app.ClearMRU();
	m_saveMRU = false;
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

  if (m_core.GetCurFile().IsEmpty())
    return SaveAs();

  if (m_core.GetReadFileVersion() == PWSfile::VCURRENT) {
    m_core.BackupCurFile(); // to save previous reversion
  } else { // file version mis-match
  	CMyString NewName = PWSUtil::GetNewFileName(m_core.GetCurFile(), DEFAULT_SUFFIX );

    CString msg = _T("The original database, \"");
    msg += CString(m_core.GetCurFile());
    msg += _T("\", is in pre-3.0 format."
              " It will be unchanged.\n");
    msg += _T("Your changes will be written as \"");
    msg += NewName;
    msg += _T("\" in the new"
              " format, which is unusable by old versions of PasswordSafe."
              " To save your changes in the old format, use the \"File->Export To"
              "-> Old (1.x or 2) format\" command.\n\n"
              "Press OK to continue saving, or Cancel to stop.");
    if (MessageBox(msg, _T("File version warning"),
                   MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
      return PWScore::USER_CANCEL;
    m_core.SetCurFile(NewName);
#if !defined(POCKET_PC)
    m_title = _T("Password Safe - ") + m_core.GetCurFile();
    app.SetTooltipText(m_core.GetCurFile());
#endif
  }
  rc = m_core.WriteCurFile();

  if (rc == PWScore::CANT_OPEN_FILE) {
    CMyString temp = m_core.GetCurFile() +
      _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error"), MB_OK|MB_ICONWARNING);
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
    CMyString temp =
      _T("Do you want to save changes to the password database: ")
      + m_core.GetCurFile()
      + _T("?");
    rc = MessageBox(temp,
                    AfxGetAppName(),
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
  int rc;
  CMyString newfile;

  if (m_core.GetReadFileVersion() != PWSfile::VCURRENT &&
      m_core.GetReadFileVersion() != PWSfile::UNKNOWN_VERSION) {
    CMyString msg = _T("The original database, \"");
    msg += m_core.GetCurFile();
    msg += _T("\", is in pre-3.0 format. The data will now be written in the new"
              " format, which is unusable by old versions of PasswordSafe."
              " To save the data in the old format, use the \"File->Export To"
              "-> Old (1.x or 2) format\" command.\n\n"
              "Press OK to continue saving, or Cancel to stop.");
    if (MessageBox(msg, _T("File version warning"),
                   MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
      return PWScore::USER_CANCEL;
  }
  //SaveAs-type dialog box
  CMyString v3FileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), DEFAULT_SUFFIX );
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
      fd.m_ofn.lpstrTitle =
        _T("Please Choose a Name for the Current (Untitled) Database:");
    else
      fd.m_ofn.lpstrTitle =
        _T("Please Choose a New Name for the Current Database:");
    rc = fd.DoModal();
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();
      break;
    } else
      return PWScore::USER_CANCEL;
  }
  CMyString locker(_T("")); // null init is important here
  if (!m_core.LockFile(newfile, locker)) {
    CMyString temp = newfile + _T("\n\nFile is currently locked by ") + locker;
    MessageBox(temp, _T("File lock error"), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  rc = m_core.WriteFile(newfile);
  if (rc == PWScore::CANT_OPEN_FILE) {
    m_core.UnlockFile(newfile);
    CMyString temp = newfile + _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error"), MB_OK|MB_ICONWARNING);
    return PWScore::CANT_OPEN_FILE;
  }
  if (!m_core.GetCurFile().IsEmpty())
    m_core.UnlockFile(m_core.GetCurFile());
  m_core.SetCurFile(newfile);
#if !defined(POCKET_PC)
  m_title = _T("Password Safe - ") + m_core.GetCurFile();
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

  //SaveAs-type dialog box
  CMyString OldFormatFileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), _T("dat") );
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
    fd.m_ofn.lpstrTitle =
      _T("Please name the exported database");
    rc = fd.DoModal();
    if (rc == IDOK) {
      newfile = (CMyString)fd.GetPathName();
      break;
    } else
      return;
  }

  switch (nID) {
  case ID_FILE_EXPORTTO_OLD1XFORMAT:
    rc = m_core.WriteV17File(newfile);
    break;
  case ID_FILE_EXPORTTO_V2FORMAT:
    rc = m_core.WriteV2File(newfile);
    break;
  default:
    ASSERT(0);
    rc = PWScore::FAILURE;
    break;
  }
  if (rc == PWScore::CANT_OPEN_FILE) {
    CMyString temp = newfile + _T("\n\nCould not open file for writing!");
    MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
  }
}

void
DboxMain::OnExportText()
{
  CExportTextDlg et;
  bool bwrite_header;
  int rc = et.DoModal();
  if (rc == IDOK) {
    CMyString newfile;
    CMyString pw(et.m_exportTextPassword);
    if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
      // do the export
      //SaveAs-type dialog box
	  CMyString TxtFileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), _T("txt") );
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
        fd.m_ofn.lpstrTitle =
          _T("Please name the plaintext file");
        rc = fd.DoModal();
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

      rc = m_core.WritePlaintextFile(newfile, bwrite_header, bsExport, subgroup, iObject, iFunction, delimiter);

      if (rc == PWScore::CANT_OPEN_FILE)        {
        CMyString temp = newfile + _T("\n\nCould not open file for writing!");
        MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      }
    } else {
      MessageBox(_T("Passkey incorrect"), _T("Error"));
      Sleep(3000); // protect against automatic attacks
    }
  }
}

void
DboxMain::OnExportXML()
{
  CExportXMLDlg eXML;
  int rc = eXML.DoModal();
  if (rc == IDOK) {
    CMyString newfile;
    CMyString pw(eXML.m_ExportXMLPassword);
    if (m_core.CheckPassword(m_core.GetCurFile(), pw) == PWScore::SUCCESS) {
      // do the export
      //SaveAs-type dialog box
      CMyString XMLFileName = PWSUtil::GetNewFileName(m_core.GetCurFile(), _T("xml") );
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
        fd.m_ofn.lpstrTitle =
          _T("Please name the XML file");
        rc = fd.DoModal();
        if (rc == IDOK) {
          newfile = (CMyString)fd.GetPathName();
          break;
        } else
          return;
      } // while (1)

      char delimiter;
      delimiter = eXML.m_defexpdelim[0];
      rc = m_core.WriteXMLFile(newfile, delimiter);

      if (rc == PWScore::CANT_OPEN_FILE)        {
        CMyString temp = newfile + _T("\n\nCould not open file for writing!");
        MessageBox(temp, _T("File write error."), MB_OK|MB_ICONWARNING);
      }
    } else {
      MessageBox(_T("Passkey incorrect"), _T("Error"));
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
  fd.m_ofn.lpstrTitle = _T("Please Choose a Text File to Import");
  int rc = fd.DoModal();
  if (rc == IDOK) {
  	CString strErrors;
    CMyString newfile = (CMyString)fd.GetPathName();
    int numImported = 0, numSkipped = 0;
    bool bimport_preV3 = (dlg.m_import_preV3 == 1) ? true : false;
    char delimiter;
    if (dlg.m_querysetimpdelim == 1) {
      delimiter = dlg.m_defimpdelim[0];
    } else {
      delimiter = '\0';
    }
    rc = m_core.ImportPlaintextFile(ImportedPrefix, newfile, strErrors, fieldSeparator,
                                    delimiter, numImported, numSkipped, bimport_preV3);

    switch (rc) {
    case PWScore::CANT_OPEN_FILE:
      {
        CMyString temp = newfile + _T("\n\nCould not open file for reading!");
        MessageBox(temp, _T("File open error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::INVALID_FORMAT:
      {
        CMyString temp = newfile + _T("\n\nInvalid format");
        MessageBox(temp, _T("File read error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::SUCCESS:
    default:
      {
      	CString temp1, temp2 = _T("");
      	temp1.Format(_T("Read %d record%s"), numImported, (numImported != 1) ? _T("s") : _T(""));
        if (numSkipped != 0)
          temp2.Format(_T("\nCouldn't read %d record%s"), numSkipped, (numSkipped != 1) ? _T("s") : _T(""));

        MessageBox(temp1 + temp2, _T("Status"), MB_ICONINFORMATION|MB_OK);
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

  CFileDialog fd(TRUE,
                 _T("txt"),
                 NULL,
                 OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                 _T("Text files (*.txt)|*.txt|")
                 _T("CSV files (*.csv)|*.csv|")
                 _T("All files (*.*)|*.*|")
                 _T("|"),
                 this);
  fd.m_ofn.lpstrTitle = _T("Please Choose a KeePass Text File to Import");
  int rc = fd.DoModal();
  if (rc == IDOK) {
    CMyString newfile = (CMyString)fd.GetPathName();
    rc = m_core.ImportKeePassTextFile(newfile);
    switch (rc) {
    case PWScore::CANT_OPEN_FILE:
      {
        CMyString temp = newfile + _T("\n\nCould not open file for reading!");
        MessageBox(temp, _T("File open error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::INVALID_FORMAT:
      {
        CMyString temp = newfile + _T("\n\nInvalid format");
        MessageBox(temp, _T("File read error"), MB_OK|MB_ICONWARNING);
      }
      break;
    case PWScore::SUCCESS:
    default:
      RefreshList();
      break;
    } // switch
  }
}

void
DboxMain::OnImportXML()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  CString XSDFilename = _T("");
  TCHAR acPath[MAX_PATH + 1];
  struct _stat statbuf;

  if ( GetModuleFileName( NULL, acPath, MAX_PATH + 1 ) != 0) {
    // guaranteed file name of at least one character after path '\'
    *(_tcsrchr(acPath, _T('\\')) + 1) = _T('\0');
    // Add on xsd filename
    XSDFilename = CString(acPath) + _T("pwsafe.xsd");
  }

  if (XSDFilename.IsEmpty() || _tstat(XSDFilename, &statbuf) != 0) {
    CMyString temp = _T("Unable to find required XML Schema Definition file (pwsafe.xsd) in your PasswordSafe Application Directory.  Please copy it from your installation file.");
    MessageBox(temp, _T("Missing XSD File - Unable to validate XML files"), MB_OK | MB_ICONSTOP);
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
  fd.m_ofn.lpstrTitle = _T("Please Choose a XML File to Import");

  int rc = fd.DoModal();
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
        dlg.m_strActionText = XMLFilename + _T(" failed validation against XML Schema Definition pwsafe.xsd");
        dlg.m_strResultText = strErrors;
        dlg.DoModal();
      }
      break;
    case PWScore::XML_FAILED_IMPORT:
      {
        CImportXMLErrDlg dlg;
        dlg.m_strActionText = XMLFilename + _T(" passed Validation but had the following errors during import:");
        dlg.m_strResultText = strErrors;
        dlg.DoModal();
      }
      break;
    case PWScore::SUCCESS:
      {
        if (!strErrors.IsEmpty()) {
          CString temp;
          temp.Format(_T("%s was imported (entries validated %d / imported %d) but had the following errors:"),
                      XMLFilename, numValidated, numImported);
          CImportXMLErrDlg dlg;
          dlg.m_strActionText = temp;
          dlg.m_strResultText = strErrors;
          dlg.DoModal();
        } else {
          CString temp;
          temp.Format(_T("Validated %d record%s\r\n\r\nImported %d record%s"),
                      numValidated, (numValidated != 1) ? _T("s") : _T(""),
                      numImported, (numImported != 1) ? _T("s") : _T(""));
          MessageBox(temp, _T("Status"), MB_ICONINFORMATION|MB_OK);
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

   //Open-type dialog box
   while (1)
   {
      CFileDialog fd(TRUE,
                     DEFAULT_SUFFIX,
                     NULL,
                     OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                     SUFFIX_FILTERS
                     _T("Password Safe Backups (*.bak)|*.bak|")
                     _T("All files (*.*)|*.*|")
                     _T("|"),
                     this);
      fd.m_ofn.lpstrTitle = _T("Please Choose a Database to Merge");
      rc = fd.DoModal();
      if (rc == IDOK)
      {
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
  if (pszFilename == m_core.GetCurFile())
	{
      //It is the same damn file
      MessageBox(_T("That file is already open."),
                 _T("Oops!"),
                 MB_OK|MB_ICONWARNING);
      return PWScore::ALREADY_OPEN;
	}

  rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL);  // OK, CANCEL, HELP
  switch (rc)
	{
	case PWScore::SUCCESS:
      app.AddToMRU(pszFilename);
      break; // Keep going...
	case PWScore::CANT_OPEN_FILE:
      temp = m_core.GetCurFile()
        + _T("\n\nCan't open file. Please choose another.");
      MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
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

  if (rc == PWScore::CANT_OPEN_FILE)
	{
      temp = pszFilename;
      temp += _T("\n\nCould not open file for reading!");
      MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
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
        CMyString warnMsg;
        warnMsg = _T("Conflicting entries for ") +
          otherItem.GetGroup() + _T(",") + 
          otherItem.GetTitle() + _T(",") +
          otherItem.GetUser() + _T("\n");
        warnMsg += _T("Adding new entry as ") +
          newTitle + _T(",") + 
          otherItem.GetUser() + _T("\n");

        /* tell the user the bad news */
        MessageBox(warnMsg,
                   _T("Merge conflict"),
                   MB_OK|MB_ICONWARNING);				

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
  resultStr.Format(_T("Merge complete:\n%d entr%s added (%d conflict%s)"),
                   totalAdded,
                   totalAdded == 1 ? _T("y") : _T("ies"),
                   numConflicts,
                   numConflicts == 1 ? _T("") : _T("s"));
  MessageBox(resultStr, _T("Merge Complete"), MB_OK);

  ChangeOkUpdate();
  RefreshList();

  return rc;
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
		MessageBox(_T("No database open with which to compare against another database!"),
					_T("Oops!"), MB_OK|MB_ICONWARNING);
		return;
	}

	CMyString file2;

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
		fd.m_ofn.lpstrTitle = _T("Please Choose a Database to Compare with current open database");
		rc = fd.DoModal();
		if (rc == IDOK) {
			file2 = (CMyString)fd.GetPathName();
			//Check that this file isn't the current one!
			if (file2 == m_core.GetCurFile()) {
				//It is the same damn file!
				MessageBox(_T("This database is the same as the current database!"),
							_T("Oops!"), MB_OK|MB_ICONWARNING);
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

	// OK, CANCEL, HELP + force READ-ONLY
	rc = GetAndCheckPassword(pszFilename, passkey, GCP_NORMAL, true);
	switch (rc) {
		case PWScore::SUCCESS:
			break; // Keep going...
		case PWScore::CANT_OPEN_FILE:
			temp = m_core.GetCurFile()
					+ _T("\n\nCan't open file. Please choose another.");
			MessageBox(temp, _T("File open error."), MB_OK|MB_ICONWARNING);
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
		temp = pszFilename;
		temp += _T("\n\nCould not open file for reading!");
		MessageBox(temp, _T("File read error."), MB_OK|MB_ICONWARNING);
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
				.... ...1	CTIME		[0x7]

				Second word
				1... ....	PMTIME		[0x8]
				.1.. ....	ATIME		[0x9]
				..1. ....	LTIME		[0xa]
				...1 ....	POLICY		[0xb] - not yet implemented
				.... 1...	RMTIME		[0xc]
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
			if (currentItem.GetCTime() != compItem.GetCTime())
				bsConflicts.flip(CItemData::CTIME);
			if (currentItem.GetPMTime() != compItem.GetPMTime())
				bsConflicts.flip(CItemData::PMTIME);
			if (currentItem.GetATime() != compItem.GetATime())
				bsConflicts.flip(CItemData::ATIME);
			if (currentItem.GetLTime() != compItem.GetLTime())
				bsConflicts.flip(CItemData::LTIME);
			if (currentItem.GetRMTime() != compItem.GetRMTime())
				bsConflicts.flip(CItemData::RMTIME);
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
	buffer.Format(_T("Compare complete of current database:\n\t %s\n and:\n\t %s"),
					m_core.GetCurFile(), pszFilename);

	if (numOnlyInCurrent == 0 && numOnlyInComp == 0 && numConflicts == 0) {
		resultStr += buffer + _T("\n\nDatabases identical!");
		MessageBox(resultStr, _T("Compare Complete"), MB_OK);
		goto exit;
	}

	resultStr += buffer;
	buffer.Empty();
	buffer.Format(_T("\n\nNumber of entr%s only in the current database is %d"),
						numOnlyInCurrent == 1 ? _T("y") : _T("ies"), numOnlyInCurrent);
	resultStr += buffer;
	buffer.Empty();
	buffer.Format(_T("\nNumber of entr%s only in the comparison database is %d"),
						numOnlyInComp == 1 ? _T("y") : _T("ies"), numOnlyInComp);
	resultStr += buffer;
	buffer.Empty();
	buffer.Format(_T("\nNumber of entr%s in both but with differences is %d"),
						numConflicts == 1 ? _T("y") : _T("ies"), numConflicts);
	resultStr += buffer;
	buffer.Empty();
	resultStr += _T("\n\nTo copy details to the clipboard, press Yes - otherwise press No (default).");
	int mb_rc = MessageBox(resultStr, _T("Compare Complete"), MB_YESNO | MB_DEFBUTTON2);

	if (mb_rc == IDNO)
		goto exit;

	resultStr.Empty();
	if (numOnlyInCurrent > 0) {
		buffer.Format(_T("Entries only in current database (%s):"), m_core.GetCurFile());
		resultStr += buffer;
		POSITION currentPos = list_OnlyInCurrent.GetHeadPosition();
		while (currentPos) {
			POSITION corepos = list_OnlyInCurrent.GetAt(currentPos);
			CItemData currentItem = m_core.GetEntryAt(corepos);
			const CMyString currentGroup = currentItem.GetGroup();
			const CMyString currentTitle = currentItem.GetTitle();
			const CMyString currentUser = currentItem.GetUser();

			resultStr += _T("\n\tGroup:\"") + currentGroup +
						 _T("\"; Title:\"") + currentTitle +
						 _T("\"; User:\"") + currentUser +
						 _T("\"");

			list_OnlyInCurrent.GetNext(currentPos);
		}
		resultStr += _T("\n");
	}

	if (numOnlyInComp > 0) {
		buffer.Format(_T("Entries only in comparison database (%s):"), pszFilename);
		resultStr += buffer;
		POSITION compPos = list_OnlyInComp.GetHeadPosition();
		while (compPos) {
			POSITION corepos = list_OnlyInComp.GetAt(compPos);
			CItemData compItem = compCore.GetEntryAt(corepos);
			const CMyString compGroup = compItem.GetGroup();
			const CMyString compTitle = compItem.GetTitle();
			const CMyString compUser = compItem.GetUser();

			resultStr += _T("\n\tGroup:\"") + compGroup +
						 _T("\"; Title:\"") + compTitle +
						 _T("\"; User:\"") + compUser +
						 _T("\"");

			list_OnlyInComp.GetNext(compPos);
		}
		resultStr += _T("\n");
	}

	if (numConflicts > 0) {
		buffer.Format(_T("Entries in both %s and %s but with differences:"),
					m_core.GetCurFile(), pszFilename);
		resultStr += buffer;
		POSITION conflictPos = list_Conflicts.GetHeadPosition();
		while (conflictPos) {
			st_Conflict st_diff = list_Conflicts.GetAt(conflictPos);
			CItemData currentItem = m_core.GetEntryAt(st_diff.cPos);
			const CMyString currentGroup = currentItem.GetGroup();
			const CMyString currentTitle = currentItem.GetTitle();
			const CMyString currentUser = currentItem.GetUser();

			resultStr += _T("\n\tIn entry - Group:\"") + currentGroup +
						 _T("\"; Title:\"") + currentTitle +
						 _T("\"; User:\"") + currentUser +
						 _T("\"\n\t\tthe following fields have differences:");

			if (st_diff.bsDiffs.test(CItemData::PASSWORD)) resultStr += _T(" 'Password'");
			if (st_diff.bsDiffs.test(CItemData::NOTES)) resultStr += _T(" 'Notes'");
			if (st_diff.bsDiffs.test(CItemData::URL)) resultStr += _T(" 'URL'");
			if (st_diff.bsDiffs.test(CItemData::AUTOTYPE)) resultStr += _T(" 'Autotype'");
			if (st_diff.bsDiffs.test(CItemData::CTIME)) resultStr += _T(" 'Creation Time'");
			if (st_diff.bsDiffs.test(CItemData::PMTIME)) resultStr += _T(" 'Password Modification Time'");
			if (st_diff.bsDiffs.test(CItemData::ATIME)) resultStr += _T(" 'Last Access Time'");
			if (st_diff.bsDiffs.test(CItemData::LTIME)) resultStr += _T(" 'Password Expiry Time'");
			if (st_diff.bsDiffs.test(CItemData::RMTIME)) resultStr += _T(" 'Record Modification Time'");
			if (st_diff.bsDiffs.test(CItemData::PWHIST)) resultStr += _T(" 'Password History'");

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
    rc = MessageBox(_T("Do you want to save changes to the password list?"),
                    AfxGetAppName(),
                    MB_ICONQUESTION|MB_YESNOCANCEL);
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
  if (m_saveMRU)
    prefs->SetPref(PWSprefs::CurrentFile, m_core.GetCurFile());
  else
    prefs->SetPref(PWSprefs::CurrentFile, _T(""));

  // Clear clipboard on Exit?  Yes if:
  // a. the app is minimized and the systemtray is enabled
  // b. the user has set the "DontAskMinimizeClearYesNo" pref
  // c. the system is shutting down, restarting or the user is logging off
  if ((!IsWindowVisible() && prefs->GetPref(PWSprefs::UseSystemTray)) ||
      prefs->GetPref(PWSprefs::DontAskMinimizeClearYesNo) ||
      m_bSessionEnding) {
		app.ClearClipboardData();
  }

  ClearData();
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
