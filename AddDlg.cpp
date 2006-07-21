/// \file AddDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "AddDlg.h"
#include "PwFont.h"
#include "corelib/PWCharPool.h"
#include "corelib/PWSprefs.h"
#include "ExpDTDlg.h"

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

const int MAX_PW_HISTORY = 25;
static TCHAR PSSWDCHAR = TCHAR('*');

//-----------------------------------------------------------------------------
CAddDlg::CAddDlg(CWnd* pParent)
  : CDialog(CAddDlg::IDD, pParent), m_password(_T("")), m_notes(_T("")),
    m_username(_T("")), m_title(_T("")), m_group(_T("")),
    m_URL(_T("")), m_autotype(_T("")),
	m_tttLTime((time_t)0), m_ascLTime(_T("Never")),
	m_MaxPWHistory(3), m_isPwHidden(false)
{
  m_isExpanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DisplayExpandedAddEditDlg);
  m_SavePWHistory = PWSprefs::GetInstance()->
    GetPref(PWSprefs::SavePasswordHistory);
}

BOOL CAddDlg::OnInitDialog() 
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

  ResizeDialog();

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXPWHISTORY));
  pspin->SetRange(1, MAX_PW_HISTORY);
  pspin->SetBase(10);
  pspin->SetPos(m_MaxPWHistory);  // Default suggestion of max. to keep!
  return TRUE;
}


void CAddDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
  DDX_Text(pDX, IDC_PASSWORD2, (CString&)m_password2);
  DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_LTIME, (CString&)m_ascLTime);
  DDX_Check(pDX, IDC_SAVE_PWHIST, m_SavePWHistory);

  if(!pDX->m_bSaveAndValidate) {
    // We are initializing the dialog.  Populate the groups combo box.
    CComboBox comboGroup;
    comboGroup.Attach(GetDlgItem(IDC_GROUP)->GetSafeHwnd());
    // For some reason, MFC calls us twice when initializing.
    // Populate the combo box only once.
    if(0 == comboGroup.GetCount()) {
      CStringArray aryGroups;
      app.m_core.GetUniqueGroups(aryGroups);
      for(int igrp=0; igrp<aryGroups.GetSize(); igrp++) {
        comboGroup.AddString((LPCTSTR)aryGroups[igrp]);
      }
    }
    comboGroup.Detach();
  }
  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Text(pDX, IDC_URL, (CString&)m_URL);
  DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);
  DDX_Control(pDX, IDC_MORE, m_moreLessBtn);
  DDX_Text(pDX, IDC_MAXPWHISTORY, m_MaxPWHistory);
  DDV_MinMaxInt(pDX, m_MaxPWHistory, 1, MAX_PW_HISTORY);

  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory);
}

BEGIN_MESSAGE_MAP(CAddDlg, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_SHOWPASSWORD, OnShowpassword)
   ON_BN_CLICKED(IDC_RANDOM, OnRandom)
   ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_BN_CLICKED(IDC_LTIME_CLEAR, OnBnClickedClearLTime)
   ON_BN_CLICKED(IDC_LTIME_SET, OnBnClickedSetLTime)
   ON_BN_CLICKED(IDC_SAVE_PWHIST, OnCheckedSavePasswordHistory)
END_MESSAGE_MAP()


void
CAddDlg::OnCancel() 
{
  CDialog::OnCancel();
}

void
CAddDlg::OnShowpassword() 
{
  UpdateData(TRUE);

  if (m_isPwHidden)
    ShowPassword();
  else
    HidePassword();

  UpdateData(FALSE);
}

void
CAddDlg::ShowPassword()
{
   m_isPwHidden = false;
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(HIDE_PASSWORD_TXT);
   // Remove password character so that the password is displayed
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(0);
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
   // Don't need verification as the user can see the password entered
   GetDlgItem(IDC_PASSWORD2)->EnableWindow(FALSE);
   m_password2.Empty();
}

void
CAddDlg::HidePassword()
{
   m_isPwHidden = true;
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(SHOW_PASSWORD_TXT);
   // Set password character so that the password is not displayed
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(PSSWDCHAR);
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
   // Need verification as the user can not see the password entered
   GetDlgItem(IDC_PASSWORD2)->EnableWindow(TRUE);
   m_password2 = m_password;
}

void
CAddDlg::OnOK() 
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

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);

  if (pParent->Find(m_group, m_title, m_username) != NULL) {
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


void CAddDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  //WinHelp(0x2008E, HELP_CONTEXT);
  ::HtmlHelp(NULL,
             "pwsafe.chm::/html/entering_pwd.html",
             HH_DISPLAY_TOPIC, 0);
#endif
}


void CAddDlg::OnRandom() 
{
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);

  UpdateData(TRUE);
  if (pParent->MakeRandomPassword(this, m_password)) {
    if (m_isPwHidden) {
    	m_password2 = m_password;
    }
    UpdateData(FALSE);
  }
}
//-----------------------------------------------------------------------------

void CAddDlg::OnBnClickedMore()
{
  m_isExpanded = !m_isExpanded;
  PWSprefs::GetInstance()->
    SetPref(PWSprefs::DisplayExpandedAddEditDlg, m_isExpanded);
  ResizeDialog();
}


void CAddDlg::OnBnClickedOk()
{
	OnOK();
}

void CAddDlg::ResizeDialog()
{
  int TopHideableControl = IDC_TOP_HIDEABLE;
  int BottomHideableControl = IDC_BOTTOM_HIDEABLE;
  int controls[]={
    IDC_STATIC_URL,
    IDC_URL,
    IDC_AUTOTYPE,
    IDC_SAVE_PWHIST,
    IDC_STATIC_AUTO,
    IDC_LTIME,
    IDC_STATIC_LTIME,
    IDC_LTIME_CLEAR,
    IDC_LTIME_SET,
    IDC_STATIC_DTEXPGROUP,
    IDC_MAXPWHISTORY,
    IDC_STATIC_OLDPW1,
    IDC_PWHSPIN,
  };	
	
  for(int n = 0; n < sizeof(controls)/sizeof(controls[0]); n++) {
    CWnd* pWind = (CWnd *)GetDlgItem(controls[n]);
    pWind->ShowWindow(m_isExpanded);
  }
	
  RECT curDialogRect;
	
  this->GetWindowRect(&curDialogRect);

  RECT newDialogRect=curDialogRect;


  RECT curLowestCtlRect;
  CWnd* pLowestCtl;
  int newHeight;
  if (m_isExpanded) {
    // from less to more
    pLowestCtl = (CWnd *)GetDlgItem(BottomHideableControl);
	  
    pLowestCtl->GetWindowRect(&curLowestCtlRect);

    newHeight =  curLowestCtlRect.bottom + 15  - newDialogRect.top;
    m_moreLessBtn.SetWindowText(_T("<< &Less"));
  } else {
    // from more to less
    pLowestCtl = (CWnd *)GetDlgItem(TopHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);

    newHeight =  curLowestCtlRect.top + 5  - newDialogRect.top;

    m_moreLessBtn.SetWindowText(_T("&More >>"));
  }
  

  this->SetWindowPos(NULL,0,0,
                     newDialogRect.right - newDialogRect.left ,
                     newHeight , 
                     SWP_NOMOVE );

}

void CAddDlg::OnBnClickedClearLTime()
{
	GetDlgItem(IDC_LTIME)->SetWindowText(_T("Never"));
	m_ascLTime = _T("Never");
	m_tttLTime = (time_t)0;
}

void CAddDlg::OnBnClickedSetLTime()
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
CAddDlg::OnCheckedSavePasswordHistory()
{
	m_SavePWHistory = ((CButton*)GetDlgItem(IDC_SAVE_PWHIST))->GetCheck();

	GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory);
}
