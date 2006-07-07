/// \file EditDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "EditDlg.h"
#include "PwFont.h"
#include "OptionsPasswordPolicy.h"
#include "corelib/PWCharPool.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"
#include "ExpDTDlg.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#include ".\editdlg.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(POCKET_PC)
  #define SHOW_PASSWORD_TXT	_T("S")
  #define HIDE_PASSWORD_TXT	_T("H")
#else
  #define SHOW_PASSWORD_TXT	_T("&Show")
  #define HIDE_PASSWORD_TXT	_T("&Hide")
#endif

CEditDlg::CEditDlg(CWnd* pParent)
  : CDialog(CEditDlg::IDD, pParent),
	m_ascCTime(_T("")), m_ascPMTime(_T("")), m_ascATime(_T("")),
	m_ascLTime(_T("")), m_ascRMTime(_T("")),
	m_ClearPWHistory(false), m_iSortedColumn(-1),
    m_bSortAscending(TRUE), m_isMoreExpanded(false)
{
  m_SavePWHistory = PWSprefs::GetInstance()->
    GetPref(PWSprefs::SavePasswordHistory);
}

CEditDlg::~CEditDlg()
{
}

void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
  DDX_Text(pDX, IDC_PASSWORD2, (CString&)m_password2);
  DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_URL, (CString&)m_URL);
  DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);
  DDX_Text(pDX, IDC_CTIME, (CString&)m_ascCTime);
  DDX_Text(pDX, IDC_PMTIME, (CString&)m_ascPMTime);
  DDX_Text(pDX, IDC_ATIME, (CString&)m_ascATime);
  DDX_Text(pDX, IDC_LTIME, (CString&)m_ascLTime);
  DDX_Text(pDX, IDC_RMTIME, (CString&)m_ascRMTime);
  DDX_Control(pDX, IDC_PWHISTORY_LIST, m_PWHistListCtrl);
  DDX_Check(pDX, IDC_SAVE_PWHIST, m_SavePWHistory);
  DDX_Text(pDX, IDC_MAXPWHISTORY, m_MaxPWHistory);
  DDV_MinMaxInt(pDX, m_MaxPWHistory, 1, 25);

  if(!pDX->m_bSaveAndValidate) {
    // We are initializing the dialog.  Populate the groups combo box.
    CComboBox comboGroup;
    comboGroup.Attach(GetDlgItem(IDC_GROUP)->GetSafeHwnd());
    // For some reason, MFC calls us twice when initializing.
    // Populate the combo box only once.
    if(0 == comboGroup.GetCount()) {
      CStringArray aryGroups;
      app.m_core.GetUniqueGroups(aryGroups);
      for(int igrp = 0; igrp < aryGroups.GetSize(); igrp++) {
        comboGroup.AddString((LPCTSTR)aryGroups[igrp]);
      }
    }
    comboGroup.Detach();
  }
  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Control(pDX, IDC_MORE, m_MoreLessBtn);
  DDX_Control(pDX, IDC_SHOW_PWHIST, m_EvenMoreLessBtn);
}

BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
	ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowpassword)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	ON_BN_CLICKED(IDC_RANDOM, OnRandom)
#if defined(POCKET_PC)
	ON_WM_SHOWWINDOW()
	ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetfocus)
	ON_EN_KILLFOCUS(IDC_PASSWORD, OnPasskeyKillfocus)
#endif
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
	ON_BN_CLICKED(IDC_LTIME_CLEAR, OnBnClickedClearLTime)
	ON_BN_CLICKED(IDC_LTIME_SET, OnBnClickedSetLTime)
	ON_BN_CLICKED(IDC_SHOW_PWHIST, OnBnClickedShowPasswordHistory)
	ON_BN_CLICKED(IDC_SAVE_PWHIST, OnCheckedSavePasswordHistory)
	ON_BN_CLICKED(IDC_COPY_OLDPW_TO_CLIPBOARD, OnBnClickedCopyToClipboard)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHeaderClicked)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
	ON_BN_CLICKED(IDC_CLEAR_PWHIST, OnBnClickedClearPWHist)
END_MESSAGE_MAP()


void CEditDlg::OnShowpassword() 
{
	UpdateData(TRUE);

	if (m_isPwHidden) {
		if (m_password.Compare(m_password2) != 0) {
			AfxMessageBox(_T("The entered passwords do not match.  Please re-enter them."));
			m_password.Empty();
			m_password2.Empty();
			UpdateData(FALSE);
			((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    } else
    	ShowPassword();
  } else {
    HidePassword();
  }

  UpdateData(FALSE);
}


void
CEditDlg::OnOK() 
{
  UpdateData(TRUE);

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(_T("This entry must have a title."));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }
  if (m_password.IsEmpty()) {
    AfxMessageBox(_T("This entry must have a password."));
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  }
  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(_T("A dot is invalid as the first character of the Group field."));
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    return;
  }
  if (m_isPwHidden && (m_password.Compare(m_password2) != 0)) {
    AfxMessageBox(_T("The entered passwords do not match.  Please re-enter them."));
    UpdateData(FALSE);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  }
  //End check

  m_realpassword = m_password;

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);

  POSITION listindex = pParent->Find(m_group, m_title, m_username);
  /*
   *  If there is a matching entry in our list, and that
   *  entry is not the same one we started editing, tell the
   *  user to try again.
   */
  if ((listindex != NULL) &&
      (m_listindex != listindex)) {
    CMyString temp =
      _T("An item with Group \"") + m_group
      + _T("\", Title \"") + m_title 
      + _T("\" and User Name \"") + m_username
      + _T("\" already exists.");
    AfxMessageBox(temp);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
  } else {
    CDialog::OnOK();
  }
}


void CEditDlg::OnCancel() 
{
   CDialog::OnCancel();
}


BOOL CEditDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
 
  SetPasswordFont(GetDlgItem(IDC_PASSWORD));
  SetPasswordFont(GetDlgItem(IDC_PASSWORD2));

  // Get password character for later
  m_passwordchar = ((CEdit*)GetDlgItem(IDC_PASSWORD))->GetPasswordChar();

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  UpdateData(FALSE);

  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory ? TRUE : FALSE);

  BOOL bpwh_count = (m_pPWHistList->GetCount() == 0) ? FALSE : TRUE;
  GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(bpwh_count);
  GetDlgItem(IDC_PWHISTORY_LIST)->EnableWindow(bpwh_count);

  if (m_IsReadOnly) {
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVE_PWHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEAR_PWHIST)->EnableWindow(FALSE);  // overrides count
  }

  m_PWHistListCtrl.InsertColumn(0, _T("Changed Date/Time"));
  m_PWHistListCtrl.InsertColumn(1, _T("Password"));

  int nPos = 0;
  POSITION itempos;

  POSITION listpos = m_pPWHistList->GetHeadPosition();
  while (listpos != NULL) {
    itempos = listpos;
    const PWHistEntry pwhentry = m_pPWHistList->GetAt(listpos);
    nPos = m_PWHistListCtrl.InsertItem(nPos, pwhentry.changedate);
    m_PWHistListCtrl.SetItemText(nPos, 1, pwhentry.password);
    m_PWHistListCtrl.SetItemData(nPos, (DWORD)itempos);
    m_pPWHistList->GetNext(listpos);
  }

  m_PWHistListCtrl.SetRedraw(FALSE);
  for (int i = 0; i < 2; i++) {
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_PWHistListCtrl.GetColumnWidth(i);
    m_PWHistListCtrl.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
  }
  m_PWHistListCtrl.SetRedraw(TRUE);

  char buffer[10];
#if _MSC_VER >= 1400
  sprintf_s(buffer, 10, "%d", m_NumPWHistory);
#else
  sprintf(buffer, "%d", m_NumPWHistory);
#endif

  m_isExpanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DisplayExpandedAddEditDlg);
  m_isMoreExpanded = false;
  ResizeDialog();
  MakeDialogWider();

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  if (m_MaxPWHistory == 0)
  	m_MaxPWHistory = 1;

  pspin->SetBuddy(GetDlgItem(IDC_MAXPWHISTORY));
  pspin->SetRange(1, 25);
  pspin->SetBase(10);
  pspin->SetPos(m_MaxPWHistory);
 
  return TRUE;
}

void CEditDlg::ShowPassword()
{
	m_password = m_password2 = m_realpassword;
	m_isPwHidden = false;
	GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(HIDE_PASSWORD_TXT);
	// Remove password character so that the password is displayed
	((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(0);
	((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
	// Don't need verification as the user can see the password entered
	GetDlgItem(IDC_PASSWORD2)->EnableWindow(FALSE);
	m_password2.Empty();
}


void CEditDlg::HidePassword()
{
	m_password = m_password2 = m_realpassword;
	m_isPwHidden = true;
	GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(SHOW_PASSWORD_TXT);
	// Set password character so that the password is not displayed
	((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(m_passwordchar);
	((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
	// Need verification as the user can not see the password entered
	GetDlgItem(IDC_PASSWORD2)->EnableWindow(TRUE);
}


void CEditDlg::OnRandom() 
{
	DboxMain* pParent = (DboxMain*)GetParent();
	ASSERT(pParent != NULL);
	UpdateData(TRUE);
	if (pParent->MakeRandomPassword(this, m_realpassword)) {
		m_password = m_password2 = m_realpassword;
		UpdateData(FALSE);
	}
}


void CEditDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#editview"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  ::HtmlHelp(
             NULL,
             "pwsafe.chm::/html/entering_pwd.html",
             HH_DISPLAY_TOPIC, 0);

#endif
}


#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CEditDlg::OnPasskeyKillfocus()
{
  EnableWordCompletion( m_hWnd );
}


/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CEditDlg::OnPasskeySetfocus()
{
  DisableWordCompletion( m_hWnd );
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void CEditDlg::OnBnClickedOk()
{
  OnOK();
}

void CEditDlg::OnBnClickedMore()
{
  m_isExpanded = !m_isExpanded;
  PWSprefs::GetInstance()->
    SetPref(PWSprefs::DisplayExpandedAddEditDlg, m_isExpanded);
  ResizeDialog();
}

void
CEditDlg::OnBnClickedShowPasswordHistory()
{
  m_isMoreExpanded = !m_isMoreExpanded;
  MakeDialogWider();
}
void CEditDlg::ResizeDialog()
{
  const int TopHideableControl = IDC_TOP_HIDEABLE;
  const int BottomHideableControl = IDC_BOTTOM_HIDEABLE;
  const int controls[]={
    IDC_URL,
    IDC_STATIC_URL,
    IDC_AUTOTYPE,
    IDC_STATIC_AUTO,
	IDC_CTIME,
	IDC_STATIC_CTIME,
	IDC_PMTIME,
	IDC_STATIC_PMTIME,
	IDC_ATIME,
	IDC_STATIC_ATIME,
	IDC_LTIME,
	IDC_STATIC_LTIME,
	IDC_RMTIME,
	IDC_STATIC_RMTIME,
	IDC_LTIME_CLEAR,
	IDC_LTIME_SET,
	IDC_STATIC_DTGROUP,
	IDC_STATIC_DTEXPGROUP,
	IDC_SHOW_PWHIST
  };

  int windows_state = m_isExpanded ? SW_SHOW : SW_HIDE;
  for(int n = 0; n < sizeof(controls)/sizeof(IDC_URL); n++) {
    CWnd* pWind;
    pWind = (CWnd *)GetDlgItem(controls[n]);
    pWind->ShowWindow(windows_state);
  }

  //  Make sure that the extra expansion is always off.
  if (m_isMoreExpanded)
   	OnBnClickedShowPasswordHistory();

  RECT curDialogRect;
	
  this->GetWindowRect(&curDialogRect);

  RECT newDialogRect = curDialogRect;

  RECT curLowestCtlRect;
  CWnd* pLowestCtl;
  int newHeight;

  if (m_isExpanded) {
    // from less to more
    pLowestCtl = (CWnd *)GetDlgItem(BottomHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);
    newHeight = curLowestCtlRect.bottom + 15 - newDialogRect.top;
    
    m_MoreLessBtn.SetWindowText(_T("<< Less"));
  } else {
    // from more to less
    pLowestCtl = (CWnd *)GetDlgItem(TopHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);
    newHeight = curLowestCtlRect.top + 5 - newDialogRect.top;

    m_MoreLessBtn.SetWindowText(_T("More >>"));
  }
  
  this->SetWindowPos(NULL, 0, 0, newDialogRect.right - newDialogRect.left,
                     newHeight, SWP_NOMOVE);
}

void
CEditDlg::MakeDialogWider()
{
  const int LeftHideableControl = IDC_LEFT_HIDEABLE;
  const int RightHideableControl = IDC_RIGHT_HIDEABLE;
  const int controls[]={
	IDC_PWHSPIN,
	IDC_STATIC_PWHGROUP,
	IDC_PWHISTORY_LIST,
	IDC_MAXPWHISTORY,
	IDC_COPY_OLDPW_TO_CLIPBOARD,
	IDC_CLEAR_PWHIST,
	IDC_SAVE_PWHIST,
	IDC_STATIC_OLDPW1,
  };

  int windows_state = m_isMoreExpanded ? SW_SHOW : SW_HIDE;
  for(int n = 0; n < sizeof(controls)/sizeof(IDC_PWHSPIN); n++) {
    CWnd* pWind;
    pWind = (CWnd *)GetDlgItem(controls[n]);
    pWind->ShowWindow(windows_state);
  }

  RECT curDialogRect;
	
  this->GetWindowRect(&curDialogRect);

  RECT newDialogRect = curDialogRect;

  RECT curRightMostCtlRect;
  CWnd* pRightMostCtl;
  int newWidth;

  if (m_isMoreExpanded) {
    // from less to more
    pRightMostCtl = (CWnd *)GetDlgItem(RightHideableControl);
    pRightMostCtl->GetWindowRect(&curRightMostCtlRect);
    newWidth = curRightMostCtlRect.right + 15 - newDialogRect.left;  
    
    m_EvenMoreLessBtn.SetWindowText(_T("Hide Password History"));
  } else {
    // from more to less
    pRightMostCtl = (CWnd *)GetDlgItem(LeftHideableControl);
    pRightMostCtl->GetWindowRect(&curRightMostCtlRect);
    newWidth = curRightMostCtlRect.right + 5 - newDialogRect.left; 

    m_EvenMoreLessBtn.SetWindowText(_T("Show Password History"));
  }
  
  this->SetWindowPos(NULL, 0, 0, newWidth,
                     newDialogRect.bottom - newDialogRect.top, SWP_NOMOVE);
}

void CEditDlg::OnBnClickedClearLTime()
{
	GetDlgItem(IDC_LTIME)->SetWindowText(_T("Never"));
	m_ascLTime = "Never";
	m_tttLTime = (time_t)0;
}


void CEditDlg::OnBnClickedSetLTime()
{
  CExpDTDlg dlg_expDT(this);

  dlg_expDT.m_ascLTime = m_ascLTime;

  app.DisableAccelerator();
  int rc = dlg_expDT.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    m_tttLTime = dlg_expDT.m_tttLTime;
    m_ascLTime = dlg_expDT.m_ascLTime;
    GetDlgItem(IDC_LTIME)->SetWindowText(m_ascLTime);
  }
}

void
CEditDlg::OnCheckedSavePasswordHistory()
{
  m_SavePWHistory = ((CButton*)GetDlgItem(IDC_SAVE_PWHIST))->GetCheck();
  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory ? TRUE : FALSE);
}

void
CEditDlg::OnBnClickedCopyToClipboard()
{
  CString data = _T("");
  const CString CRLF = _T("\r\n");
  const CString TAB = _T('\t');

  POSITION listpos = m_pPWHistList->GetHeadPosition();
  while (listpos != NULL) {
    const PWHistEntry pwhentry = m_pPWHistList->GetAt(listpos);
    data = data +
      (CString)pwhentry.changedate + TAB +
      (CString)pwhentry.password + CRLF;
    m_pPWHistList->GetNext(listpos);
  }

  app.SetClipboardData(data);
}

void
CEditDlg::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

  if(phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if(phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = TRUE;

    m_iSortedColumn = phdn->iItem;
    m_PWHistListCtrl.SortItems(CompareFunc, (LPARAM)this);

    // Note: WINVER defines the minimum system level for which this is program compiled and
    // NOT the level of system it is running on!
    // In this case, these values are defined in Windows XP and later and supported
    // by V6 of comctl32.dll (supplied with Windows XP) and later.
    // They should be ignored by earlier levels of this dll or .....
    //     we can check the dll version (code available on request)!

#if (WINVER < 0x0501)	// These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_PWHistListCtrl.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
    m_PWHistListCtrl.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pResult = 0;
}

int CALLBACK CEditDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                   LPARAM closure)
{
  CEditDlg *self = (CEditDlg*)closure;
  int nSortColumn = self->m_iSortedColumn;
  POSITION Lpos = (POSITION)lParam1;
  POSITION Rpos = (POSITION)lParam2;
  const PWHistEntry pLHS = self->m_pPWHistList->GetAt(Lpos);
  const PWHistEntry pRHS = self->m_pPWHistList->GetAt(Rpos);
  CMyString password1, changedate1;
  CMyString password2, changedate2;
  time_t t1, t2;

  int iResult;
  switch(nSortColumn) {
  case 0:
    t1 = pLHS.changetttdate;
    t2 = pRHS.changetttdate;
    iResult = ((long) t1 < (long) t2) ? -1 : 1;
    break;
  case 1:
    password1 = pLHS.password;
    password2 = pRHS.password;
    iResult = ((CString)password1).Compare(password2);
    break;
  default:
    iResult = 0; // should never happen - just keep compiler happy
    ASSERT(FALSE);
  }

  if (!self->m_bSortAscending)
    iResult *= -1;

  return iResult;
}

void CEditDlg::OnBnClickedClearPWHist()
{
  m_ClearPWHistory = true;
  m_PWHistListCtrl.DeleteAllItems();
}
