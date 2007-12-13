/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// OptionsBackup.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h" // for DoubleClickAction enums
#include "corelib/util.h" // for datetime string
#include "corelib/mystring.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
  #include "resource3.h"  // String resources
#endif

#include "OptionsBackup.h"

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

IMPLEMENT_DYNCREATE(COptionsBackup, CPropertyPage)

COptionsBackup::COptionsBackup(): CPWPropertyPage(COptionsBackup::IDD),
  m_ToolTipCtrl(NULL)
{
	//{{AFX_DATA_INIT(COptionsBackup)
	//}}AFX_DATA_INIT
}
 

COptionsBackup::~COptionsBackup()
{
	delete m_ToolTipCtrl;
}

void COptionsBackup::SetCurFile(const CString &currentFile)
{
  // derive current db's directory and basename:
  TCHAR path_buffer[_MAX_PATH];
  TCHAR drive[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR base[_MAX_FNAME];

#if _MSC_VER >= 1400
  _tsplitpath_s(currentFile, drive, _MAX_DRIVE, dir, _MAX_DIR, base, _MAX_FNAME, NULL, 0);
  _tmakepath_s(path_buffer, _MAX_PATH, drive, dir, NULL, NULL);
#else
  _tsplitpath(currentFile, drive, dir, base, NULL);
  _tmakepath(path_buffer, drive, dir,  NULL, NULL);
#endif
  m_currentFileDir = path_buffer;
  m_currentFileBasename = base;
}


void COptionsBackup::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(COptionsBackup)
	DDX_Check(pDX, IDC_SAVEIMMEDIATELY, m_saveimmediately);
	DDX_Check(pDX, IDC_BACKUPBEFORESAVE, m_backupbeforesave);
	DDX_Radio(pDX, IDC_DFLTBACKUPPREFIX, m_backupprefix); // only first!
	DDX_Text(pDX, IDC_USERBACKUPPREFIXVALUE, m_userbackupprefix);
	DDX_Control(pDX, IDC_BACKUPSUFFIX, m_backupsuffix_cbox);
	DDX_Radio(pDX, IDC_DFLTBACKUPLOCATION, m_backuplocation); // only first!
	DDX_Text(pDX, IDC_USERBACKUPOTHRLOCATIONVALUE, m_userbackupotherlocation);
	DDX_Text(pDX, IDC_BACKUPMAXINC, m_maxnumincbackups);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsBackup, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsBackup)
	ON_BN_CLICKED(IDC_BACKUPBEFORESAVE, OnBackupBeforeSave)
	ON_BN_CLICKED(IDC_DFLTBACKUPPREFIX, OnBackupPrefix)
	ON_BN_CLICKED(IDC_USERBACKUPPREFIX, OnBackupPrefix)
	ON_BN_CLICKED(IDC_DFLTBACKUPLOCATION, OnBackupDirectory)
	ON_BN_CLICKED(IDC_USERBACKUPOTHERLOCATION, OnBackupDirectory)
	ON_BN_CLICKED(IDC_BROWSEFORLOCATION, OnBrowseForLocation)
	ON_CBN_SELCHANGE(IDC_BACKUPSUFFIX, OnComboChanged)
	ON_EN_KILLFOCUS(IDC_USERBACKUPPREFIXVALUE, OnUserPrefixKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsBackup::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	if(m_backupsuffix_cbox.GetCount() == 0) {
		// add the strings in alphabetical order
		CString cs_text(MAKEINTRESOURCE(IDS_NONE));
		int nIndex;
		nIndex = m_backupsuffix_cbox.AddString(cs_text);
		m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_None);
		m_BKSFX_to_Index[PWSprefs::BKSFX_None] = nIndex;

		nIndex = m_backupsuffix_cbox.AddString(_T("YYYYMMDD_HHMMSS"));
		m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_DateTime);
		m_BKSFX_to_Index[PWSprefs::BKSFX_DateTime] = nIndex;

		cs_text.LoadString(IDS_INCREMENTNUM);
		nIndex = m_backupsuffix_cbox.AddString(cs_text);
		m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_IncNumber);
		m_BKSFX_to_Index[PWSprefs::BKSFX_IncNumber] = nIndex;
	}

	if (m_backupsuffix < PWSprefs::minBKSFX ||
		m_backupsuffix > PWSprefs::maxBKSFX)
		m_backupsuffix = PWSprefs::BKSFX_None;

	m_backupsuffix_cbox.SetCurSel(m_BKSFX_to_Index[m_backupsuffix]);

	GetDlgItem(IDC_BACKUPEXAMPLE)->SetWindowText(_T(""));

	CSpinButtonCtrl*  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_BKPMAXINCSPIN);

	pspin->SetBuddy(GetDlgItem(IDC_BACKUPMAXINC));
	pspin->SetRange(1, 999);
	pspin->SetBase(10);
	pspin->SetPos(m_maxnumincbackups);

	OnComboChanged();
	OnBackupBeforeSave();

	// Tooltips on Property Pages
	EnableToolTips();

	m_ToolTipCtrl = new CToolTipCtrl;
	if (!m_ToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
		TRACE("Unable To create Property Page ToolTip\n");
		return TRUE;
	}

	// Activate the tooltip control.
	m_ToolTipCtrl->Activate(TRUE);
	m_ToolTipCtrl->SetMaxTipWidth(300);
	// Quadruple the time to allow reading by user - there is a lot there!
	int iTime = m_ToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
	m_ToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

	// Set the tooltip
	// Note naming convention: string IDS_xxx corresponds to control IDC_xxx
	CString cs_ToolTip;
	cs_ToolTip.LoadString(IDS_BACKUPBEFORESAVE);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_BACKUPBEFORESAVE), cs_ToolTip);
	cs_ToolTip.LoadString(IDS_USERBACKUPOTHERLOCATION);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_USERBACKUPOTHERLOCATION), cs_ToolTip);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// COptionsBackup message handlers


void COptionsBackup::OnComboChanged()
{
	int nIndex = m_backupsuffix_cbox.GetCurSel();
	m_backupsuffix = m_backupsuffix_cbox.GetItemData(nIndex);
	if (m_backupsuffix == PWSprefs::BKSFX_IncNumber) {
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
	switch (m_backupprefix) {
		case 0:
			GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(FALSE);
            m_userbackupprefix = _T("");
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
	switch (m_backuplocation) {
		case 0:
			GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(FALSE);
            m_userbackupotherlocation = _T("");
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
	if (m_backupbeforesave == FALSE) {
		GetDlgItem(IDC_DFLTBACKUPPREFIX)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPPREFIX)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BACKUPSUFFIX)->EnableWindow(FALSE);
		GetDlgItem(IDC_DFLTBACKUPLOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPOTHERLOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_DFLTBACKUPPREFIX)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPPREFIX)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BACKUPSUFFIX)->EnableWindow(TRUE);
		GetDlgItem(IDC_DFLTBACKUPLOCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPOTHERLOCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(TRUE);

		OnBackupPrefix();
		OnBackupDirectory();
		SetExample();
	}
}

void COptionsBackup::SetExample()
{
	CString cs_example;
    UpdateData(TRUE);
	switch (m_backupprefix) {
		case 0:
          cs_example = m_currentFileBasename;
			break;
		case 1:
			cs_example = m_userbackupprefix;
			break;
		default:
          ASSERT(0);
			break;
	}

	switch (m_backupsuffix) {
		case 1:
			{
				time_t now;
				time(&now);
				CString cs_datetime = (CString)PWSUtil::ConvertToDateTimeString(now, TMC_EXPORT_IMPORT);
				cs_example += _T("_");
				cs_example = cs_example + cs_datetime.Left(4) +		// YYYY
										  cs_datetime.Mid(5,2) +	// MM
										  cs_datetime.Mid(8,2) +	// DD
										  _T("_") +
										  cs_datetime.Mid(11,2) +	// HH
										  cs_datetime.Mid(14,2) +	// MM
										  cs_datetime.Mid(17,2);	// SS
			}
			break;
		case 2:
			cs_example += _T("_001");
			break;
		case 0:
		default:
			break;
	}

	cs_example += _T(".ibak");
	GetDlgItem(IDC_BACKUPEXAMPLE)->SetWindowText(cs_example);
}

void COptionsBackup::OnOK()
{
	UpdateData(TRUE);

	CPropertyPage::OnOK();
}

BOOL COptionsBackup::OnKillActive()
{
	CPropertyPage::OnKillActive();

	if (m_backupbeforesave != TRUE)
		return TRUE;

	// Check that correct fields are non-blank.
	if (m_backupprefix == 1  && m_userbackupprefix.IsEmpty()) {
		AfxMessageBox(IDS_OPTBACKUPPREF);
		((CEdit*)GetDlgItem(IDC_USERBACKUPPREFIXVALUE))->SetFocus();
		return FALSE;
	}


	if (m_backuplocation == 1) {
		if (m_userbackupotherlocation.IsEmpty()) {
			AfxMessageBox(IDS_OPTBACKUPLOCATION);
			((CEdit*)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
			return FALSE;
		}

		if (m_userbackupotherlocation.Right(1) != _T("\\")) {
			m_userbackupotherlocation += _T("\\");
			UpdateData(FALSE);
		}

		if (PathIsDirectory(m_userbackupotherlocation) == FALSE) {
			AfxMessageBox(IDS_OPTBACKUPNOLOC);
			((CEdit*)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
			return FALSE;
		}
	}

	if (m_backupsuffix == PWSprefs::BKSFX_IncNumber &&
		((m_maxnumincbackups < 1) || (m_maxnumincbackups > 999))) {
			AfxMessageBox(IDS_OPTBACKUPMAXNUM);
			((CEdit*)GetDlgItem(IDC_BACKUPMAXINC))->SetFocus();
			return FALSE;
	}

	//End check

	return TRUE;
}

void COptionsBackup::OnUserPrefixKillfocus()
{
	SetExample();
}

// Override PreTranslateMessage() so RelayEvent() can be
// called to pass a mouse message to CPWSOptions's
// tooltip control for processing.
BOOL COptionsBackup::PreTranslateMessage(MSG* pMsg)
{
	if (m_ToolTipCtrl != NULL)
		m_ToolTipCtrl->RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void COptionsBackup::OnBrowseForLocation()
{
  CString cs_initiallocation;
  if (m_userbackupotherlocation.IsEmpty()) {
    cs_initiallocation = m_currentFileDir;
  } else
    cs_initiallocation = m_userbackupotherlocation;

  // The BROWSEINFO struct tells the shell
  // how it should display the dialog.
  BROWSEINFO bi;
  memset(&bi, 0, sizeof(bi));

  bi.hwndOwner = this->GetSafeHwnd();
  bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
  CString cs_text(MAKEINTRESOURCE(IDS_OPTBACKUPTITLE));
  bi.lpszTitle = cs_text;
  bi.lpfn = SetSelProc;
  bi.lParam = (LPARAM)(LPCTSTR) cs_initiallocation;


  // Show the dialog and get the itemIDList for the
  // selected folder.
  LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

  if(pIDL != NULL) {
    // Create a buffer to store the path, then
    // get the path.
    TCHAR buffer[_MAX_PATH] = { 0 };
    if(::SHGetPathFromIDList(pIDL, buffer) != 0)
      m_userbackupotherlocation = CString(buffer);
    else
      m_userbackupotherlocation = _T("");

    UpdateData(FALSE);

    // free the item id list
    CoTaskMemFree(pIDL);
  }
}

//  SetSelProc
//  Callback procedure to set the initial selection of the browser.
int CALLBACK SetSelProc(HWND hWnd, UINT uMsg, LPARAM , LPARAM lpData)
{
	if (uMsg==BFFM_INITIALIZED) {
		::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData );
	}
	return 0;
}
