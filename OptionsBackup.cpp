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

COptionsBackup::COptionsBackup() : CPropertyPage(COptionsBackup::IDD)
{
	//{{AFX_DATA_INIT(COptionsBackup)
	//}}AFX_DATA_INIT
	m_ToolTipCtrl = NULL;
}

COptionsBackup::~COptionsBackup()
{
	delete m_ToolTipCtrl;
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
	DDX_Text(pDX, IDC_USERBACKUPSUBDIRECTORYVALUE, m_userbackupsubdirectory);
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
	ON_BN_CLICKED(IDC_USERBACKUPSUBDIRECTORY, OnBackupDirectory)
	ON_BN_CLICKED(IDC_USERBACKUPOTHERLOCATION, OnBackupDirectory)
	ON_BN_CLICKED(IDC_BROWSEFORLOCATION, OnBrowseForLocation)
	ON_CBN_SELCHANGE(IDC_BACKUPSUFFIX, OnComboChanged)
	ON_EN_KILLFOCUS(IDC_USERBACKUPPREFIX, OnUserPrefixKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsBackup::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	if(m_backupsuffix_cbox.GetCount() == 0) {
		// add the strings in alphabetical order
		int nIndex;
		nIndex = m_backupsuffix_cbox.AddString(_T("None"));
		m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_None);
		m_BKSFX_to_Index[PWSprefs::BKSFX_None] = nIndex;

		nIndex = m_backupsuffix_cbox.AddString(_T("YYYYMMDD_HHMMSS"));
		m_backupsuffix_cbox.SetItemData(nIndex, PWSprefs::BKSFX_DateTime);
		m_BKSFX_to_Index[PWSprefs::BKSFX_DateTime] = nIndex;

		nIndex = m_backupsuffix_cbox.AddString(_T("Incremented Number [001-999]"));
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
	cs_ToolTip.LoadString(IDS_USERBACKUPSUBDIRECTORY);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_USERBACKUPSUBDIRECTORY), cs_ToolTip);
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
			break;
		case 1:
			GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(TRUE);
			break;
		default:
			break;
	}
	SetExample();
}

void COptionsBackup::OnBackupDirectory()
{
	UpdateData(TRUE);
	switch (m_backuplocation) {
		case 0:
			GetDlgItem(IDC_USERBACKUPSUBDIRECTORYVALUE)->EnableWindow(FALSE);
			GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(FALSE);
			break;
		case 1:
			GetDlgItem(IDC_USERBACKUPSUBDIRECTORYVALUE)->EnableWindow(TRUE);
			GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(FALSE);
			break;
		case 2:
			GetDlgItem(IDC_USERBACKUPSUBDIRECTORYVALUE)->EnableWindow(FALSE);
			GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(TRUE);
			GetDlgItem(IDC_BROWSEFORLOCATION)->EnableWindow(TRUE);
			break;
		default:
			break;
	}
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
		GetDlgItem(IDC_USERBACKUPSUBDIRECTORY)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPOTHERLOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPSUBDIRECTORYVALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_DFLTBACKUPPREFIX)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPPREFIX)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPPREFIXVALUE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BACKUPSUFFIX)->EnableWindow(TRUE);
		GetDlgItem(IDC_DFLTBACKUPLOCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPSUBDIRECTORY)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPOTHERLOCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPSUBDIRECTORYVALUE)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE)->EnableWindow(TRUE);

		OnBackupPrefix();
		OnBackupDirectory();
		SetExample();
	}
}

void COptionsBackup::SetExample()
{
	CString cs_example;

	switch (m_backupprefix) {
		case 0:
			cs_example = _T("<database name>");
			break;
		case 1:
			cs_example = m_userbackupprefix;
			break;
		default:
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
		AfxMessageBox(_T("Please enter your backup database prefix!"));
		((CEdit*)GetDlgItem(IDC_USERBACKUPPREFIXVALUE))->SetFocus();
		return FALSE;
	}

	if (m_backuplocation == 1) {
		if(m_userbackupsubdirectory.IsEmpty()) {
			AfxMessageBox(_T("Please specify the sub-directory to contain all backups."));
			((CEdit*)GetDlgItem(IDC_USERBACKUPSUBDIRECTORYVALUE))->SetFocus();
			return FALSE;
		}

		if (m_userbackupsubdirectory.Right(1) == _T("\\")) {
			m_userbackupsubdirectory.Left(m_userbackupsubdirectory.GetLength()-1);
			UpdateData(FALSE);
		}
	}

	if (m_backuplocation == 2) {
		if (m_userbackupotherlocation.IsEmpty()) {
			AfxMessageBox(_T("Please specify the destination location to contain all backups."));
			((CEdit*)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
			return FALSE;
		}

		if (m_userbackupotherlocation.Right(1) != _T("\\")) {
			m_userbackupotherlocation += _T("\\");
			UpdateData(FALSE);
		}

		if (PathIsDirectory(m_userbackupotherlocation) == FALSE) {
			AfxMessageBox(_T("Destination location does not exist or is not accessible."));
			((CEdit*)GetDlgItem(IDC_USERBACKUPOTHRLOCATIONVALUE))->SetFocus();
			return FALSE;
		}
	}

	if (m_backupsuffix == PWSprefs::BKSFX_IncNumber &&
		((m_maxnumincbackups < 1) || (m_maxnumincbackups > 999))) {
			AfxMessageBox(_T("Maximum number of backups kept using incremented number suffix\n"
							 "must be between 1 and 999."));
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
	if (m_userbackupotherlocation.IsEmpty())
		cs_initiallocation = _T("C:\\");
	else
		cs_initiallocation = m_userbackupotherlocation;

	// The BROWSEINFO struct tells the shell
	// how it should display the dialog.
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));

	bi.hwndOwner = this->GetSafeHwnd();
	bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
	bi.lpszTitle = _T("Select Location for Intermediate Backups");
	bi.lpfn = SetSelProc;
	bi.lParam = (LPARAM)(LPCSTR) cs_initiallocation;


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
