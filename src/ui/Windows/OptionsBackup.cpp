/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsBackup.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"

#include "core/PwsPlatform.h"
#include "core/PWSprefs.h" // for DoubleClickAction enums
#include "core/util.h" // for datetime string

#include "os/dir.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

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
                          pOPTMD),
  m_pToolTipCtrl(NULL)
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
}

COptionsBackup::~COptionsBackup()
{
  delete m_pToolTipCtrl;
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
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsBackup::OnInitDialog()
{
  COptions_PropertyPage::OnInitDialog();

  m_chkbox.SetTextColour(CR_DATABASE_OPTIONS);

  if (!M_pDbx()->IsDBReadOnly())
    GetDlgItem(IDC_STATIC_DB_PREFS_RO_WARNING)->ShowWindow(SW_HIDE);

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

    cs_text.LoadString(IDS_INCREMENTNUM);
    nIndex = m_backupsuffix_cbox.AddString(cs_text);
    m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_IncNumber);
    m_BKSFX_to_Index[PWSprefs::BKSFX_IncNumber] = nIndex;
  }

  m_backupsuffix_cbox.SetCurSel(m_BKSFX_to_Index[m_BackupSuffix]);

  GetDlgItem(IDC_BACKUPEXAMPLE)->SetWindowText(L"");

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_BKPMAXINCSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_BACKUPMAXINC));
  pspin->SetRange(1, 999);
  pspin->SetBase(10);
  pspin->SetPos(m_MaxNumIncBackups);

  OnComboChanged();
  OnBackupBeforeSave();

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create Property Page ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return TRUE;
  }

  // Tooltips on Property Pages
  EnableToolTips();

  // Activate the tooltip control.
  m_pToolTipCtrl->Activate(TRUE);
  m_pToolTipCtrl->SetMaxTipWidth(300);
  // Quadruple the time to allow reading by user - there is a lot there!
  int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

  // Set the tooltip
  // Note naming convention: string IDS_xxx corresponds to control IDC_xxx
  CString cs_ToolTip;
  cs_ToolTip.LoadString(IDS_BACKUPBEFORESAVE);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_BACKUPBEFORESAVE), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_USERBACKUPOTHERLOCATION);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_USERBACKUPOTHERLOCATION), cs_ToolTip);

  return TRUE;
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
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL COptionsBackup::OnApply()
{
  UpdateData(TRUE);

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

BOOL COptionsBackup::PreTranslateMessage(MSG* pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}


BOOL COptionsBackup::OnKillActive()
{
  COptions_PropertyPage::OnKillActive();

  if (m_BackupBeforeSave != TRUE)
    return TRUE;

  CGeneralMsgBox gmb;
  // Check that correct fields are non-blank.
  if (m_BackupPrefix == 1  && m_UserBackupPrefix.IsEmpty()) {
    gmb.AfxMessageBox(IDS_OPTBACKUPPREF);
    ((CEdit*)GetDlgItem(IDC_USERBACKUPPREFIXVALUE))->SetFocus();
    return FALSE;
  }

  if (m_BackupLocation == 1) {
    if (m_UserBackupOtherLocation.IsEmpty()) {
      gmb.AfxMessageBox(IDS_OPTBACKUPLOCATION);
      ((CEdit*)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
      return FALSE;
    }

    if (m_UserBackupOtherLocation.Right(1) != L"\\") {
      m_UserBackupOtherLocation += L"\\";
      UpdateData(FALSE);
    }

    if (PathIsDirectory(m_UserBackupOtherLocation) == FALSE) {
      gmb.AfxMessageBox(IDS_OPTBACKUPNOLOC);
      ((CEdit*)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
      return FALSE;
    }
  }

  if (m_BackupSuffix == PWSprefs::BKSFX_IncNumber &&
    ((m_MaxNumIncBackups < 1) || (m_MaxNumIncBackups > 999))) {
      gmb.AfxMessageBox(IDS_OPTBACKUPMAXNUM);
      ((CEdit*)GetDlgItem(IDC_BACKUPMAXINC))->SetFocus();
      return FALSE;
  }

  //End check

  return TRUE;
}

void COptionsBackup::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/backups_tab.html";
  ::HtmlHelp(this->GetSafeHwnd(), (LPCWSTR)cs_HelpTopic, HH_DISPLAY_TOPIC, 0);
}

void COptionsBackup::OnPreferencesHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/preferences.html";
  ::HtmlHelp(this->GetSafeHwnd(), (LPCWSTR)cs_HelpTopic, HH_DISPLAY_TOPIC, 0);
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
      m_UserBackupOtherLocation = L"";
      break;
    case 1:
      GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(TRUE);
      GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(TRUE);
      break;
    default:
      ASSERT(0);
      break;
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
                                                             TMC_EXPORT_IMPORT).c_str();
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

void COptionsBackup::OnUserPrefixKillfocus()
{
  SetExample();
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
      //pDC->SetTextColor(CR_DATABASE_OPTIONS);
      //pDC->SetBkMode(TRANSPARENT);
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
