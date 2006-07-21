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
#include "corelib/ItemData.h"
#include "ExpDTDlg.h"
#include "PWHistDlg.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
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

static TCHAR PSSWDCHAR = TCHAR('*');

CEditDlg::CEditDlg(CItemData *ci, CWnd* pParent)
  : CDialog(CEditDlg::IDD, pParent),
    m_ci(ci), m_bIsModified(false),
	m_ascLTime(_T("")), m_oldascLTime(_T("")),
	m_ClearPWHistory(false)
{
  ASSERT(ci != NULL);

  BOOL HasHistory = FALSE;
  ci->CreatePWHistoryList(HasHistory, m_MaxPWHistory,
                          m_NumPWHistory, 
                          &m_PWHistList, EXPORT_IMPORT);
  m_SavePWHistory = HasHistory;

  m_group = ci->GetGroup();
  m_title = ci->GetTitle();
  m_username = ci->GetUser();
  m_password = m_password2 = HIDDEN_PASSWORD;
  m_realpassword = m_oldRealPassword = ci->GetPassword();
  m_URL = ci->GetURL();
  m_autotype = ci->GetAutoType();
  m_notes = ci->GetNotes();
  m_PWHistory = ci->GetPWHistory();
  m_ascCTime = ci->GetCTime();
  m_ascPMTime = ci->GetPMTime();
  m_ascATime = ci->GetATime();
  m_ascRMTime = ci->GetRMTime();

  m_ascLTime = ci->GetLTimeN();
  if (m_ascLTime.IsEmpty())
    m_ascLTime = _T("Never");
  m_oldascLTime = m_ascLTime;
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
}

BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowpassword)
ON_BN_CLICKED(ID_HELP, OnHelp)
ON_BN_CLICKED(IDC_RANDOM, OnRandom)
#if defined(POCKET_PC)
ON_WM_SHOWWINDOW()
#endif
ON_EN_SETFOCUS(IDC_PASSWORD, OnPasskeySetfocus)
ON_EN_KILLFOCUS(IDC_PASSWORD, OnPasskeyKillfocus)
ON_BN_CLICKED(IDOK, OnBnClickedOk)
ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
ON_BN_CLICKED(IDC_LTIME_CLEAR, OnBnClickedClearLTime)
ON_BN_CLICKED(IDC_LTIME_SET, OnBnClickedSetLTime)
ON_BN_CLICKED(IDC_PWHIST, OnBnClickedPwhist)
END_MESSAGE_MAP()


void CEditDlg::OnShowpassword() 
{
  UpdateData(TRUE);

  if (m_isPwHidden) {
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
  if (m_password != HIDDEN_PASSWORD)
    m_realpassword = m_password;

  m_bIsModified |= (
                   m_group != m_ci->GetGroup() ||
                   m_title != m_ci->GetTitle() ||
                   m_username != m_ci->GetUser() ||
                   m_notes != m_ci->GetNotes() ||
                   m_URL != m_ci->GetURL() ||
                   m_autotype != m_ci->GetAutoType() ||
                   m_PWHistory != m_ci->GetPWHistory() ||
                   m_ascLTime != m_oldascLTime
                   );

  bool IsPswdModified = m_realpassword != m_oldRealPassword;
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
  } else { // Everything OK, update fields
    m_ci->SetGroup(m_group);
    m_ci->SetTitle(m_title);
    m_ci->SetUser(m_username.IsEmpty() ? m_defusername : m_username);
    m_ci->SetPassword(m_realpassword);
    m_ci->SetNotes(m_notes);
    m_ci->SetURL(m_URL);
    m_ci->SetAutoType(m_autotype);
    m_ci->SetPWHistory(m_PWHistory);

    time_t t;
    time(&t);
    if (IsPswdModified) {
      if (m_SavePWHistory)
        UpdateHistory();
      m_ci->SetPMTime(t);
    }
    if (m_bIsModified || IsPswdModified)
      m_ci->SetRMTime(t);
    if (m_oldascLTime != m_ascLTime)
      m_ci->SetLTime(m_tttLTime);

    CDialog::OnOK();
  }
}

void CEditDlg::UpdateHistory()
{
  int num = m_PWHistList.GetCount();
  PWHistEntry pwh_ent;
  pwh_ent.password = m_oldRealPassword;
  time_t t;
  m_ci->GetPMTime(t);
  if ((long)t == 0L) // if never set - try creation date
    m_ci->GetCTime(t);
  pwh_ent.changetttdate = t;
  pwh_ent.changedate =
    PWSUtil::ConvertToDateTimeString(t, EXPORT_IMPORT);
  if (pwh_ent.changedate.IsEmpty()) {
    //                       1234567890123456789
    pwh_ent.changedate = _T("Unknown            ");
  }

  // Now add the latest
  m_PWHistList.AddTail(pwh_ent);

  // Increment count
  num++;

  // Too many? remove the excess
  if (num > m_MaxPWHistory) {
    for (int i = 0; i < (num - m_MaxPWHistory); i++)
      m_PWHistList.RemoveHead();
    num = m_MaxPWHistory;
  }

  // Now create string version!
  CMyString new_PWHistory;
  CString buffer;

  buffer.Format(_T("1%02x%02x"), m_MaxPWHistory, num);
  new_PWHistory = CMyString(buffer);

  POSITION listpos = m_PWHistList.GetHeadPosition();
  while (listpos != NULL) {
    const PWHistEntry pwshe = m_PWHistList.GetAt(listpos);

    buffer.Format(_T("%08x%04x%s"),
                  (long) pwshe.changetttdate, pwshe.password.GetLength(),
                  pwshe.password);
    new_PWHistory += CMyString(buffer);
    buffer.Empty();
    m_PWHistList.GetNext(listpos);
  }
  m_ci->SetPWHistory(new_PWHistory);
}

BOOL CEditDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
 
  SetPasswordFont(GetDlgItem(IDC_PASSWORD));
  SetPasswordFont(GetDlgItem(IDC_PASSWORD2));


  ((CEdit*)GetDlgItem(IDC_PASSWORD2))->SetPasswordChar(PSSWDCHAR);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowPWDefault)) {
    ShowPassword();
  } else {
    HidePassword();
  }

  UpdateData(FALSE);


  m_isExpanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DisplayExpandedAddEditDlg);
  ResizeDialog();

  return TRUE;
}

void CEditDlg::ShowPassword()
{
  m_isPwHidden = false;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(HIDE_PASSWORD_TXT);

  m_password = m_realpassword;
  // Remove password character so that the password is displayed
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(0);
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
  // Don't need verification as the user can see the password entered
  m_password2.Empty();
  GetDlgItem(IDC_PASSWORD2)->EnableWindow(FALSE);
}


void CEditDlg::HidePassword()
{
  m_isPwHidden = true;
  GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(SHOW_PASSWORD_TXT);

  m_password = m_password2 = HIDDEN_PASSWORD;
  // Set password character so that the password is not displayed
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(PSSWDCHAR);
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
  // Need verification as the user can not see the password entered
  GetDlgItem(IDC_PASSWORD2)->EnableWindow(TRUE);
}


void CEditDlg::OnRandom() 
{
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);
  UpdateData(TRUE);
  if (pParent->MakeRandomPassword(this, m_realpassword) &&
      !m_isPwHidden) {
    m_password = m_realpassword;
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


void CEditDlg::OnPasskeyKillfocus()
{
#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
  EnableWordCompletion( m_hWnd );
#endif
}


void CEditDlg::OnPasskeySetfocus()
{
  ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetSel(0, -1);
#if defined(POCKET_PC)
/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
  DisableWordCompletion( m_hWnd );
#endif
}

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
  };

  int windows_state = m_isExpanded ? SW_SHOW : SW_HIDE;
  for(int n = 0; n < sizeof(controls)/sizeof(controls[0]); n++) {
    CWnd* pWind;
    pWind = (CWnd *)GetDlgItem(controls[n]);
    pWind->ShowWindow(windows_state);
  }

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
    
    m_MoreLessBtn.SetWindowText(_T("<< &Less"));
  } else {
    // from more to less
    pLowestCtl = (CWnd *)GetDlgItem(TopHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);
    newHeight = curLowestCtlRect.top + 5 - newDialogRect.top;

    m_MoreLessBtn.SetWindowText(_T("&More >>"));
  }
  
  this->SetWindowPos(NULL, 0, 0, newDialogRect.right - newDialogRect.left,
                     newHeight, SWP_NOMOVE);
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


void CEditDlg::OnBnClickedPwhist()
{
  CPWHistDlg dlg(this, m_IsReadOnly,
                 m_PWHistory, m_PWHistList,
                 m_NumPWHistory, m_MaxPWHistory,
                 m_SavePWHistory);

  dlg.DoModal();
}
