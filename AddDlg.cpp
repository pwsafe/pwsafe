/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
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
#include "ControlExtns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TCHAR PSSWDCHAR = TCHAR('*');

CString CAddDlg::CS_SHOW;
CString CAddDlg::CS_HIDE;

//-----------------------------------------------------------------------------
CAddDlg::CAddDlg(CWnd* pParent)
  : CPWDialog(CAddDlg::IDD, pParent), m_password(_T("")), m_notes(_T("")),
    m_username(_T("")), m_title(_T("")), m_group(_T("")),
    m_URL(_T("")), m_autotype(_T("")),
	m_tttLTime((time_t)0),
	m_isPwHidden(false)
{
  m_isExpanded = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DisplayExpandedAddEditDlg);
  m_SavePWHistory = PWSprefs::GetInstance()->
    GetPref(PWSprefs::SavePasswordHistory);
  m_MaxPWHistory = PWSprefs::GetInstance()->
    GetPref(PWSprefs::NumPWHistoryDefault);
  m_locLTime.LoadString(IDS_NEVER);

  if (CS_SHOW.IsEmpty()) {
#if defined(POCKET_PC)
	CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT1);
	CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT1);
#else
	CS_SHOW.LoadString(IDS_SHOWPASSWORDTXT2);
	CS_HIDE.LoadString(IDS_HIDEPASSWORDTXT2);
#endif
  }
}

BOOL CAddDlg::OnInitDialog() 
{
  CPWDialog::OnInitDialog();

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
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos(m_MaxPWHistory);  // Default suggestion of max. to keep!

  // Populate the combo box
  if(m_ex_group.GetCount() == 0) {
    CStringArray aryGroups;
    app.m_core.GetUniqueGroups(aryGroups);
    for(int igrp = 0; igrp < aryGroups.GetSize(); igrp++) {
      m_ex_group.AddString((LPCTSTR)aryGroups[igrp]);
    }
  }

  m_ex_group.ChangeColour();
  return TRUE;
}

void CAddDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
  DDX_Text(pDX, IDC_PASSWORD2, (CString&)m_password2);
  DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
  DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
  DDX_Text(pDX, IDC_TITLE, (CString&)m_title);
  DDX_Text(pDX, IDC_LTIME, (CString&)m_locLTime);
  DDX_Check(pDX, IDC_SAVE_PWHIST, m_SavePWHistory);

  DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
  DDX_Text(pDX, IDC_URL, (CString&)m_URL);
  DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);
  DDX_Control(pDX, IDC_MORE, m_moreLessBtn);
  DDX_Text(pDX, IDC_MAXPWHISTORY, m_MaxPWHistory);
  DDV_MinMaxInt(pDX, m_MaxPWHistory, 1, 255);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
  DDX_Control(pDX, IDC_PASSWORD2, m_ex_password2);
  DDX_Control(pDX, IDC_NOTES, m_ex_notes);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_URL, m_ex_URL);
  DDX_Control(pDX, IDC_AUTOTYPE, m_ex_autotype);

  GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory);
}

BEGIN_MESSAGE_MAP(CAddDlg, CPWDialog)
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
  CPWDialog::OnCancel();
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
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_HIDE);
   // Remove password character so that the password is displayed
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(0);
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
   // Don't need verification as the user can see the password entered
   GetDlgItem(IDC_PASSWORD2)->EnableWindow(FALSE);
   ((CEdit*)GetDlgItem(IDC_PASSWORD2))->Invalidate();
   m_password2.Empty();
}

void
CAddDlg::HidePassword()
{
   m_isPwHidden = true;
   GetDlgItem(IDC_SHOWPASSWORD)->SetWindowText(CS_SHOW);
   // Set password character so that the password is not displayed
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetPasswordChar(PSSWDCHAR);
   ((CEdit*)GetDlgItem(IDC_PASSWORD))->Invalidate();
   // Need verification as the user can not see the password entered
   GetDlgItem(IDC_PASSWORD2)->EnableWindow(TRUE);
   ((CEdit*)GetDlgItem(IDC_PASSWORD2))->Invalidate();
   m_password2 = m_password;
}

void
CAddDlg::OnOK() 
{
  if (UpdateData(TRUE) != TRUE)
	  return;

  m_group.EmptyIfOnlyWhiteSpace();
  m_title.EmptyIfOnlyWhiteSpace();
  m_username.EmptyIfOnlyWhiteSpace();
  if (m_password.IsOnlyWhiteSpace()) {
    m_password.Empty();
    if (m_isPwHidden)
      m_password2.Empty();
  }
  m_notes.EmptyIfOnlyWhiteSpace();
  m_URL.EmptyIfOnlyWhiteSpace();
  m_autotype.EmptyIfOnlyWhiteSpace();

  UpdateData(FALSE);

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVETITLE);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }
  if (m_password.IsEmpty()) {
    AfxMessageBox(IDS_MUSTHAVEPASSWORD);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  }
  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(IDS_DOTINVALID);
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    return;
  }
  if (m_isPwHidden && (m_password.Compare(m_password2) != 0)) {
    AfxMessageBox(IDS_PASSWORDSNOTMATCH);
    UpdateData(FALSE);
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  }

  DboxMain* dbx = static_cast<DboxMain *>(GetParent());
  ASSERT(dbx != NULL);

  CMyString csPwdGroup, csPwdTitle, csPwdUser;
  bool bBase_was_Alias(false);
  m_ibasedata = dbx->GetBaseEntry(m_password, m_base_uuid, bBase_was_Alias,
                         csPwdGroup, csPwdTitle, csPwdUser);

  // m_ibasedata:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but base entry NOT found (n = 1, 2 or 3)
  if (m_ibasedata < 0) {
    CString cs_msg;
    const CString cs_msgA(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDA));
    const CString cs_msgZ(MAKEINTRESOURCE(IDS_ALIASNOTFOUNDZ));
    int rc(IDNO);
    switch (m_ibasedata) {
      case -1:
        cs_msg.Format(IDS_ALIASNOTFOUND0, csPwdTitle);
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -2:
        // In this case the 2 fields from the password are in Title & User
        cs_msg.Format(IDS_ALIASNOTFOUND1, csPwdTitle, csPwdUser, csPwdTitle, csPwdUser);
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        break;
      case -3:
        cs_msg.Format(IDS_ALIASNOTFOUND2, csPwdGroup, csPwdTitle, csPwdUser);
        rc = AfxMessageBox(cs_msgA + cs_msg + cs_msgZ, MB_YESNO | MB_DEFBUTTON2);
        break;
      default:
        ASSERT(0);
    }
    if (rc == IDNO) {
      UpdateData(FALSE);
      ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
      return;
    }
  }
  if (m_ibasedata > 0 && bBase_was_Alias) {
    CString cs_msg;
    cs_msg.Format(IDS_BASEISALIAS, csPwdGroup, csPwdTitle, csPwdUser);
    if (AfxMessageBox(cs_msg, MB_YESNO | MB_DEFBUTTON2) == IDNO) {
      UpdateData(FALSE);
      ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
      return;
    }
  }
  //End check

  if (dbx->Find(m_group, m_title, m_username) != dbx->End()) {
    CMyString temp;
    if (m_group.IsEmpty())
      temp.Format(IDS_ENTRYEXISTS2, m_title, m_username);
    else
      temp.Format(IDS_ENTRYEXISTS, m_group, m_title, m_username);
    AfxMessageBox(temp);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
  } else {
    CPWDialog::OnOK();
  }
}


void CAddDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/entering_pwd.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
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

  for(unsigned n = 0; n < sizeof(controls)/sizeof(controls[0]); n++) {
    CWnd* pWind = (CWnd *)GetDlgItem(controls[n]);
    pWind->ShowWindow(m_isExpanded);
  }

  RECT curDialogRect;

  this->GetWindowRect(&curDialogRect);

  RECT newDialogRect=curDialogRect;

  RECT curLowestCtlRect;
  CWnd* pLowestCtl;
  int newHeight;
  CString cs_text;
  if (m_isExpanded) {
    // from less to more
    pLowestCtl = (CWnd *)GetDlgItem(BottomHideableControl);
	  
    pLowestCtl->GetWindowRect(&curLowestCtlRect);

    newHeight =  curLowestCtlRect.bottom + 15  - newDialogRect.top;
    cs_text.LoadString(IDS_LESS);
    m_moreLessBtn.SetWindowText(cs_text);
  } else {
    // from more to less
    pLowestCtl = (CWnd *)GetDlgItem(TopHideableControl);
    pLowestCtl->GetWindowRect(&curLowestCtlRect);

    newHeight =  curLowestCtlRect.top + 5  - newDialogRect.top;

	cs_text.LoadString(IDS_MORE);
    m_moreLessBtn.SetWindowText(cs_text);
  }
  

  this->SetWindowPos(NULL,0,0,
                     newDialogRect.right - newDialogRect.left ,
                     newHeight , 
                     SWP_NOMOVE );

}

void CAddDlg::OnBnClickedClearLTime()
{
	m_locLTime.LoadString(IDS_NEVER);
	GetDlgItem(IDC_LTIME)->SetWindowText((CString)m_locLTime);
	m_tttLTime = (time_t)0;
}

void CAddDlg::OnBnClickedSetLTime()
{
	CExpDTDlg dlg_expDT(this);

	dlg_expDT.m_locLTime = m_locLTime;

	app.DisableAccelerator();
	INT_PTR rc = dlg_expDT.DoModal();
	app.EnableAccelerator();

	if (rc == IDOK) {
		m_tttLTime = dlg_expDT.m_tttLTime;
		m_locLTime = dlg_expDT.m_locLTime;
		GetDlgItem(IDC_LTIME)->SetWindowText(m_locLTime);
	}
}

void
CAddDlg::OnCheckedSavePasswordHistory()
{
	m_SavePWHistory = ((CButton*)GetDlgItem(IDC_SAVE_PWHIST))->GetCheck();

	GetDlgItem(IDC_MAXPWHISTORY)->EnableWindow(m_SavePWHistory);
}
