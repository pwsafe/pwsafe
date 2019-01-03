/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsBackup.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"

#include "Options_PropertySheet.h"
#include "GeneralMsgBox.h"

#include "core/PwsPlatform.h"
#include "core/PWSprefs.h" // for DoubleClickAction enums
#include "core/util.h" // for datetime string

#include "os/dir.h"

#include "resource.h"
#include "resource3.h"  // String resources

#include "OptionsBackup.h" // Must be after resource.h

#include <shlwapi.h>
#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK SetSelProc(HWND hWnd, UINT uMsg, LPARAM , LPARAM lpData);

/////////////////////////////////////////////////////////////////////////////
// COptionsBackup property page

IMPLEMENT_DYNAMIC(COptionsBackup, COptions_PropertyPage)

COptionsBackup::COptionsBackup(CWnd *pParent, st_Opt_master_data *pOPTMD)
  : COptions_PropertyPage(pParent,
                          COptionsBackup::IDD, COptionsBackup::IDD_SHORT,
                          pOPTMD)
{
  m_currentFile = (CString)M_CurrentFile();
  m_UserBackupPrefix = M_UserBackupPrefix();
  m_BackupSuffix = M_BackupSuffix();
  m_BackupLocation = M_BackupLocation();
  m_UserBackupOtherLocation = M_UserBackupOtherLocation();
  m_SaveImmediately = M_SaveImmediately();
  m_BackupBeforeSave = M_BackupBeforeSave();
  m_BackupPrefix = M_BackupPrefix();
  m_MaxNumIncBackups = M_MaxNumIncBackups();

  if (m_BackupSuffix < PWSprefs::minBKSFX ||
      m_BackupSuffix > PWSprefs::maxBKSFX)
    m_BackupSuffix = M_BackupPrefix() = PWSprefs::BKSFX_None;

  // derive current db's directory and basename:
  std::wstring path(m_currentFile);
  std::wstring drive, dir, base, ext;

  pws_os::splitpath(path, drive, dir, base, ext);
  path = pws_os::makepath(drive, dir, L"", L"");
  m_currentFileDir = path.c_str();
  m_currentFileBasename = base.c_str();

  m_bKillActiveInProgress = false;
}

void COptionsBackup::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsBackup)
  DDX_Check(pDX, IDC_SAVEIMMEDIATELY, m_SaveImmediately);
  DDX_Check(pDX, IDC_BACKUPBEFORESAVE, m_BackupBeforeSave);
  DDX_Radio(pDX, IDC_DFLTBACKUPPREFIX, m_BackupPrefix); // only first!
  DDX_Text(pDX, IDC_USERBACKUPPREFIXVALUE, m_UserBackupPrefix);
  DDX_Control(pDX, IDC_BACKUPSUFFIX, m_backupsuffix_cbox);
  DDX_Radio(pDX, IDC_DFLTBACKUPLOCATION, m_BackupLocation); // only first!
  DDX_Text(pDX, IDC_USERBACKUPOTHRLOCATIONVALUE, m_UserBackupOtherLocation);
  DDX_Text(pDX, IDC_BACKUPMAXINC, m_MaxNumIncBackups);

  DDX_Control(pDX, IDC_SAVEIMMEDIATELY, m_chkbox);

  DDX_Control(pDX, IDC_BACKUPBEFORESAVEHELP, m_Help1);
  DDX_Control(pDX, IDC_USERBACKUPOTHERLOCATIONHELP, m_Help2);
  DDX_Control(pDX, IDC_USERBACKUPOTHERLOCATIONHELP2, m_Help3);
  DDX_Control(pDX, IDC_SAVEIMMEDIATELYHELP, m_Help4);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsBackup, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsBackup)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_BACKUPBEFORESAVE, OnBackupBeforeSave)
  ON_BN_CLICKED(IDC_DFLTBACKUPPREFIX, OnBackupPrefix)
  ON_BN_CLICKED(IDC_USERBACKUPPREFIX, OnBackupPrefix)
  ON_BN_CLICKED(IDC_DFLTBACKUPLOCATION, OnBackupDirectory)
  ON_BN_CLICKED(IDC_USERBACKUPOTHERLOCATION, OnBackupDirectory)
  ON_BN_CLICKED(IDC_BROWSEFORLOCATION, OnBrowseForLocation)
  ON_STN_CLICKED(IDC_STATIC_PREFERENCES, OnPreferencesHelp)
  ON_CBN_SELCHANGE(IDC_BACKUPSUFFIX, OnComboChanged)
  ON_EN_KILLFOCUS(IDC_USERBACKUPPREFIXVALUE, OnUserPrefixKillfocus)
  ON_EN_KILLFOCUS(IDC_USERBACKUPOTHRLOCATIONVALUE, OnUserBkpLocationKillfocus)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsBackup::OnInitDialog()
{
  COptions_PropertyPage::OnInitDialog();

  m_chkbox.SetTextColour(CR_DATABASE_OPTIONS);
  m_chkbox.ResetBkgColour(); //Use current window's background

  if (GetMainDlg()->IsDBOpen() && !GetMainDlg()->IsDBReadOnly()) {
    GetDlgItem(IDC_STATIC_DB_PREFS_RO_WARNING)->ShowWindow(SW_HIDE);
  }

  // Database preferences - can't change in R/O mode of if no DB is open
  if (!GetMainDlg()->IsDBOpen() || GetMainDlg()->IsDBReadOnly()) {
    CString cs_Preference_Warning;
    CString cs_temp(MAKEINTRESOURCE(GetMainDlg()->IsDBOpen() ? IDS_DB_READ_ONLY : IDS_NO_DB));

    cs_Preference_Warning.Format(IDS_STATIC_DB_PREFS_RO_WARNING, static_cast<LPCWSTR>(cs_temp));
    GetDlgItem(IDC_STATIC_DB_PREFS_RO_WARNING)->SetWindowText(cs_Preference_Warning);

    GetDlgItem(IDC_SAVEIMMEDIATELY)->EnableWindow(FALSE);
  }

  if (m_backupsuffix_cbox.GetCount() == 0) {
    // add the strings in alphabetical order
    CString cs_text(MAKEINTRESOURCE(IDS_NONE));
    int nIndex;
    nIndex = m_backupsuffix_cbox.AddString(cs_text);
    m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_None);
    m_BKSFX_to_Index[PWSprefs::BKSFX_None] = nIndex;

    cs_text.LoadString(IDS_DATETIMESTRING);
    nIndex = m_backupsuffix_cbox.AddString(cs_text);
    m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_DateTime);
    m_BKSFX_to_Index[PWSprefs::BKSFX_DateTime] = nIndex;

    cs_text.Format(IDS_INCREMENTNUM, M_prefminBackupIncrement(), M_prefmaxBackupIncrement());
    nIndex = m_backupsuffix_cbox.AddString(cs_text);
    m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_IncNumber);
    m_BKSFX_to_Index[PWSprefs::BKSFX_IncNumber] = nIndex;
  }

  m_backupsuffix_cbox.SetCurSel(m_BKSFX_to_Index[m_BackupSuffix]);

  GetDlgItem(IDC_BACKUPEXAMPLE)->SetWindowText(L"");

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_BKPMAXINCSPIN);
  pspin->SetBuddy(GetDlgItem(IDC_BACKUPMAXINC));
  pspin->SetRange(M_prefminBackupIncrement(), M_prefmaxBackupIncrement());
  pspin->SetBase(10);
  pspin->SetPos(m_MaxNumIncBackups);

  OnComboChanged();
  OnBackupBeforeSave();

  if (m_BackupLocation == 1) {
    ExpandBackupPath();
  }

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_QUESTIONMARK);
    m_Help4.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_BACKUPBEFORESAVEHELP, IDS_BACKUPBEFORESAVE);
    AddTool(IDC_USERBACKUPOTHERLOCATIONHELP, IDS_USERBACKUPOTHERLOCATION);
    AddTool(IDC_USERBACKUPOTHERLOCATIONHELP2, IDS_USERBACKUPOTHERLOCATION2);
    AddTool(IDC_SAVEIMMEDIATELYHELP, IDS_SAVEIMMEDIATELY);
    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
    m_Help4.EnableWindow(FALSE);
    m_Help4.ShowWindow(SW_HIDE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT COptionsBackup::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_UserBackupPrefix()        != m_UserBackupPrefix        ||
          M_UserBackupOtherLocation() != m_UserBackupOtherLocation ||
          M_SaveImmediately()         != m_SaveImmediately         ||
          M_BackupBeforeSave()        != m_BackupBeforeSave        ||
          M_BackupPrefix()            != m_BackupPrefix            ||
          M_BackupSuffix()            != m_BackupSuffix            ||
          M_BackupLocation()          != m_BackupLocation          ||
          M_MaxNumIncBackups()        != m_MaxNumIncBackups)
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL COptionsBackup::OnApply()
{
  UpdateData(TRUE);

  if (VerifyFields() == FALSE)
    return FALSE;

  M_UserBackupPrefix() = m_UserBackupPrefix;
  M_BackupSuffix() = m_BackupSuffix;
  M_BackupLocation() = m_BackupLocation;
  M_UserBackupOtherLocation() = m_UserBackupOtherLocation;
  M_SaveImmediately() = m_SaveImmediately;
  M_BackupBeforeSave() = m_BackupBeforeSave;
  M_BackupPrefix() = m_BackupPrefix;
  M_MaxNumIncBackups() = m_MaxNumIncBackups;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsBackup::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL COptionsBackup::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  m_bKillActiveInProgress = true;

  COptions_PropertyPage::OnKillActive();

  BOOL brc = VerifyFields();

  m_bKillActiveInProgress = false;
  return brc;
}

BOOL COptionsBackup::VerifyFields()
{
  if (m_BackupBeforeSave != TRUE)
    return TRUE;

  CGeneralMsgBox gmb;
  // Check that correct fields are non-blank.
  if (m_BackupPrefix == 1 && m_UserBackupPrefix.IsEmpty()) {
    gmb.AfxMessageBox(IDS_OPTBACKUPPREF);
    ((CEdit *)GetDlgItem(IDC_USERBACKUPPREFIXVALUE))->SetFocus();
    return FALSE;
  }

  if (m_BackupLocation == 1) {
    ExpandBackupPath();

    if (m_UserBackupOtherLocation.IsEmpty()) {
      gmb.AfxMessageBox(IDS_OPTBACKUPLOCATION);
      ((CEdit *)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
      return FALSE;
    }

    if (m_csExpandedPath.GetLength() > 0) {
      if (m_csExpandedPath.Right(1) != L"\\") {
        m_csExpandedPath += L"\\";
        m_UserBackupOtherLocation += L"\\";
        UpdateData(FALSE);
      }
    } else {
      if (m_UserBackupOtherLocation.Right(1) != L"\\") {
        m_UserBackupOtherLocation += L"\\";
        UpdateData(FALSE);
      }
    }

    // PathIsDirectory will return OK even if no drive specified i.e.
    // User specified %homepath% rather than, say, D:%homepath% or %homedrive%%homepath%
    // This may work but we should enforce a proper expanded form.
    CString csBackupPath = m_csExpandedPath.GetLength() > 0 ?
      m_csExpandedPath : m_UserBackupOtherLocation;

    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(std::wstring(csBackupPath),
                        cdrive, cdir, dontCare, dontCare);

    if (cdrive.length() == 0) {
      gmb.AfxMessageBox(IDS_OPTBACKUPNODRIVE);
      ((CEdit *)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
      return FALSE;
    }

    if (PathIsDirectory(csBackupPath) == FALSE) {
      gmb.AfxMessageBox(IDS_OPTBACKUPNOLOC);
      ((CEdit *)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
      return FALSE;
    }
  }

  // Update variable from text box
  CString csText;
  ((CEdit *)GetDlgItem(IDC_BACKUPMAXINC))->GetWindowText(csText);
  m_MaxNumIncBackups = _wtoi(csText);

  if (m_BackupSuffix == PWSprefs::BKSFX_IncNumber &&
      ((m_MaxNumIncBackups < M_prefminBackupIncrement()) ||
       (m_MaxNumIncBackups > M_prefmaxBackupIncrement()))) {
    csText.Format(IDS_OPTBACKUPMAXNUM, M_prefminBackupIncrement(), M_prefmaxBackupIncrement());
    gmb.AfxMessageBox(csText);
    ((CEdit *)GetDlgItem(IDC_BACKUPMAXINC))->SetFocus();
    return FALSE;
  }
  //End check

  return TRUE;
}

void COptionsBackup::OnHelp()
{
  ShowHelp(L"::/html/backups_tab.html");
}

void COptionsBackup::OnPreferencesHelp()
{
  ShowHelp(L"::/html/preferences.html");
}

/////////////////////////////////////////////////////////////////////////////
// COptionsBackup message handlers

void COptionsBackup::OnComboChanged()
{
  int nIndex = m_backupsuffix_cbox.GetCurSel();
  m_BackupSuffix = (int)m_backupsuffix_cbox.GetItemData(nIndex);
  if (m_BackupSuffix == PWSprefs::BKSFX_IncNumber) {
    GetDlgItem(IDC_BACKUPMAXINC)->EnableWindow(TRUE);
    GetDlgItem(IDC_BKPMAXINCSPIN)->EnableWindow(TRUE);
    GetDlgItem(IDC_BACKUPMAX)->EnableWindow(TRUE);
  } else {
    GetDlgItem(IDC_BACKUPMAXINC)->EnableWindow(FALSE);
    GetDlgItem(IDC_BKPMAXINCSPIN)->EnableWindow(FALSE);
    GetDlgItem(IDC_BACKUPMAX)->EnableWindow(FALSE);
  }
  SetExample();
}

void COptionsBackup::OnBackupPrefix()
{
  UpdateData(TRUE);
  switch (m_BackupPrefix) {
    case 0:
      GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(FALSE);
      m_UserBackupPrefix = L"";
      break;
    case 1:
      GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(TRUE);
      break;
    default:
      ASSERT(0);
      break;
  }
  UpdateData(FALSE);
  SetExample();
}

void COptionsBackup::OnBackupDirectory()
{
  UpdateData(TRUE);
  switch (m_BackupLocation) {
    case 0:
      GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(FALSE);
      GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(FALSE);

      GetDlgItem(IDC_EXPANDEDUSERBACKUPOTHRLOC)->EnableWindow(FALSE);
      GetDlgItem(IDC_EXPANDEDUSERBACKUPOTHRLOC)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_EXPANDEDUSERBACKUPOTHRLOC)->SetWindowText(L"");

      m_Help3.EnableWindow(FALSE);
      m_Help3.ShowWindow(SW_HIDE);

      m_UserBackupOtherLocation = L"";
      break;
    case 1:
      GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(TRUE);
      GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(TRUE);

      m_Help3.EnableWindow(TRUE);
      m_Help3.ShowWindow(SW_SHOW);
      break;
    default:
      ASSERT(0);
      break;
  }

  if (m_BackupLocation == 1) {
    ExpandBackupPath();
  }

  UpdateData(FALSE);
}

void COptionsBackup::OnBackupBeforeSave()
{
  UpdateData(TRUE);

  GetDlgItem(IDC_DFLTBACKUPPREFIX)->EnableWindow(m_BackupBeforeSave);
  GetDlgItem(IDC_USERBACKUPPREFIX)->EnableWindow(m_BackupBeforeSave);
  GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(m_BackupBeforeSave);
  GetDlgItem(IDC_BACKUPSUFFIX)->EnableWindow(m_BackupBeforeSave);
  GetDlgItem(IDC_DFLTBACKUPLOCATION)->EnableWindow(m_BackupBeforeSave);
  GetDlgItem(IDC_USERBACKUPOTHERLOCATION)->EnableWindow(m_BackupBeforeSave);
  GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(m_BackupBeforeSave);

  if (m_BackupBeforeSave == TRUE) {
    OnBackupPrefix();
    OnBackupDirectory();
    SetExample();
  }
}

void COptionsBackup::SetExample()
{
  CString cs_example;
  UpdateData(TRUE);
  switch (m_BackupPrefix) {
    case 0:
      cs_example = m_currentFileBasename;
      break;
    case 1:
      cs_example = m_UserBackupPrefix;
      break;
    default:
      ASSERT(0);
      break;
  }

  switch (m_BackupSuffix) {
    case 1:
    {
      time_t now;
      time(&now);
      CString cs_datetime = PWSUtil::ConvertToDateTimeString(now,
                                                             PWSUtil::TMC_EXPORT_IMPORT).c_str();
      cs_example += L"_";
      cs_example = cs_example + cs_datetime.Left(4) +  // YYYY
        cs_datetime.Mid(5,2) +  // MM
        cs_datetime.Mid(8,2) +  // DD
        L"_" +
        cs_datetime.Mid(11,2) +  // HH
        cs_datetime.Mid(14,2) +  // MM
        cs_datetime.Mid(17,2);   // SS
      break;
    }
    case 2:
      cs_example += L"_001";
      break;
    case 0:
    default:
      break;
  }

  cs_example += L".ibak";
  GetDlgItem(IDC_BACKUPEXAMPLE)->SetWindowText(cs_example);
}

void COptionsBackup::ExpandBackupPath()
{
  bool bSetRealPath(false);
  if (m_BackupLocation == 1) {
    wchar_t wsExpandedPath[MAX_PATH + 1];
    DWORD dwResult = ExpandEnvironmentStrings(m_UserBackupOtherLocation,
                                              wsExpandedPath, MAX_PATH + 1);
    if (dwResult == 0 || dwResult > (MAX_PATH + 1)) {
      CGeneralMsgBox gmb;
      CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_EXPANDPATH));
      cs_msg.Format(IDS_CANT_EXPANDPATH, static_cast<LPCWSTR>(m_UserBackupOtherLocation));
      gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONEXCLAMATION);
    } else {
      m_csExpandedPath = wsExpandedPath;
      if (m_UserBackupOtherLocation != m_csExpandedPath) {
        bSetRealPath = true;
      }
    }
  }

  if (!bSetRealPath)
    m_csExpandedPath.Empty();

  GetDlgItem(IDC_EXPANDEDUSERBACKUPOTHRLOC)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPANDEDUSERBACKUPOTHRLOC)->ShowWindow(SW_SHOW);
  GetDlgItem(IDC_EXPANDEDUSERBACKUPOTHRLOC)->SetWindowText(bSetRealPath ? m_csExpandedPath : L"");

  m_Help3.EnableWindow(TRUE);
  m_Help3.ShowWindow(SW_SHOW);
}

void COptionsBackup::OnUserPrefixKillfocus()
{
  SetExample();
}

void COptionsBackup::OnUserBkpLocationKillfocus()
{
  // Windows getting the focus
  CWnd *pWnd = GetFocus();

  // Don't bother verifying data if user is clicking on the other option
  // or browsing for a directory
  if (pWnd == GetDlgItem(IDC_DFLTBACKUPLOCATION) ||
      pWnd == GetDlgItem(IDC_BROWSEFORLOCATION)) {
    return;
  }

  // Don't bother verifying data if user is cancelling the whole thing
  // Rather complicated!!!!
  if (pWnd == m_options_psh->GetDlgItem(IDCANCEL)) {
    // Reset value to last good one
    m_UserBackupOtherLocation = M_UserBackupOtherLocation();

    // Now simulate the pressing of the cancel button
    WPARAM WParam = MAKEWPARAM(IDCANCEL, BN_CLICKED);
    m_options_psh->PostMessage(WM_COMMAND, WParam, NULL);
    return;
  }

  UpdateData(TRUE);
  ExpandBackupPath();

  // If OnKillActive active - skip it again
  if (!m_bKillActiveInProgress) {
    VerifyFields();
  }
}

void COptionsBackup::OnBrowseForLocation()
{
  CString cs_initiallocation;
  if (m_UserBackupOtherLocation.IsEmpty()) {
    cs_initiallocation = m_currentFileDir;
  } else
    cs_initiallocation = m_UserBackupOtherLocation;

  // The BROWSEINFO struct tells the shell
  // how it should display the dialog.
  BROWSEINFO bi;
  SecureZeroMemory(&bi, sizeof(bi));

  bi.hwndOwner = this->GetSafeHwnd();
  bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
  CString cs_text(MAKEINTRESOURCE(IDS_OPTBACKUPTITLE));
  bi.lpszTitle = cs_text;
  bi.lpfn = SetSelProc;
  bi.lParam = (LPARAM)(LPCWSTR) cs_initiallocation;

  // Show the dialog and get the itemIDList for the
  // selected folder.
  LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

  if (pIDL != NULL) {
    // Create a buffer to store the path, then
    // get the path.
    wchar_t buffer[_MAX_PATH] = { 0 };
    if (::SHGetPathFromIDList(pIDL, buffer) != 0)
      m_UserBackupOtherLocation = CString(buffer);
    else
      m_UserBackupOtherLocation = L"";

    UpdateData(FALSE);

    // free the item id list
    CoTaskMemFree(pIDL);
  }

  ExpandBackupPath();
}

//  SetSelProc
//  Callback procedure to set the initial selection of the browser.
int CALLBACK SetSelProc(HWND hWnd, UINT uMsg, LPARAM , LPARAM lpData)
{
  if (uMsg == BFFM_INITIALIZED) {
    ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
  }
  return 0;
}

HBRUSH COptionsBackup::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Database preferences - controls + associated static text
  switch (pWnd->GetDlgCtrlID()) {
    case IDC_SAVEIMMEDIATELY:
      pDC->SetTextColor(CR_DATABASE_OPTIONS);
      pDC->SetBkMode(TRANSPARENT);
      break;
    case IDC_STATIC_PREFERENCES:
      pDC->SetTextColor(RGB(0, 0, 255));
      pDC->SetBkMode(TRANSPARENT);
      break;
    case IDC_STATIC_DB_PREFS_RO_WARNING:
      pDC->SetTextColor(RGB(255, 0, 0));
      pDC->SetBkMode(TRANSPARENT);
      break;
  }

  return hbr;
}
