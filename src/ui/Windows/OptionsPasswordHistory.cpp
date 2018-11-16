/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsPasswordHistory.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"    // For Help

#include "DboxMain.h"  // needed for DboxMain::UpdatePasswordHistory

#include "Options_PropertySheet.h"

#include "GeneralMsgBox.h"

#include "core/PwsPlatform.h"

#include "resource.h"
#include "resource3.h"  // String resources

#include "OptionsPasswordHistory.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory property page

IMPLEMENT_DYNAMIC(COptionsPasswordHistory, COptions_PropertyPage)

COptionsPasswordHistory::COptionsPasswordHistory(CWnd *pParent, st_Opt_master_data *pOPTMD)
  : COptions_PropertyPage(pParent, COptionsPasswordHistory::IDD, pOPTMD),
  m_PWHAction(0), mApplyToProtected(BST_UNCHECKED)
{
  m_SavePWHistory = M_SavePWHistory();
  m_PWHistoryNumDefault = M_PWHistoryNumDefault();
  m_PWHAction = M_PWHAction();
  m_PWHDefExpDays = M_PWHDefExpDays();
}

void COptionsPasswordHistory::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsPasswordHistory)
  DDX_Check(pDX, IDC_SAVEPWHISTORY, m_SavePWHistory);
  DDX_Check(pDX, IDC_UPDATEPROTECTEDPWH, mApplyToProtected);
  DDX_Text(pDX, IDC_DEFPWHNUM, m_PWHistoryNumDefault);
  DDX_Text(pDX, IDC_DEFEXPIRYDAYS, m_PWHDefExpDays);
  DDX_Radio(pDX, IDC_PWHISTORYNOACTION, m_PWHAction);

  DDX_Control(pDX, IDC_SAVEPWHISTORY, m_chkbox);

  DDX_Control(pDX, IDC_RESETPWHISTORYOFFHELP, m_Help1);
  DDX_Control(pDX, IDC_RESETPWHISTORYONHELP, m_Help2);
  DDX_Control(pDX, IDC_SETMAXPWHISTORYHELP, m_Help3);
  DDX_Control(pDX, IDC_CLEARPWHISTORYHELP, m_Help4);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsPasswordHistory, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsPasswordHistory)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_SAVEPWHISTORY, OnSavePWHistory)
  ON_BN_CLICKED(IDC_PWHISTORYNOACTION, OnPWHistoryNoAction)
  ON_BN_CLICKED(IDC_RESETPWHISTORYOFF, OnPWHistoryDoAction)
  ON_BN_CLICKED(IDC_RESETPWHISTORYON, OnPWHistoryDoAction)
  ON_BN_CLICKED(IDC_SETMAXPWHISTORY, OnPWHistoryDoAction)
  ON_BN_CLICKED(IDC_CLEARPWHISTORY, OnPWHistoryDoAction)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory message handlers

BOOL COptionsPasswordHistory::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  m_chkbox.SetTextColour(CR_DATABASE_OPTIONS);
  m_chkbox.ResetBkgColour(); // Use current window's background

  // Database preferences - can't change in R/O mode of if no DB is open
  if (!GetMainDlg()->IsDBOpen() || GetMainDlg()->IsDBReadOnly()) {
    GetDlgItem(IDC_STATIC_NUMPWSDHIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_SAVEPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFPWHNUM)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(FALSE);

    GetDlgItem(IDC_STATIC_MANAGEPWH)->EnableWindow(FALSE);
    GetDlgItem(IDC_PWHISTORYNOACTION)->EnableWindow(FALSE);
    GetDlgItem(IDC_RESETPWHISTORYOFF)->EnableWindow(FALSE);
    GetDlgItem(IDC_RESETPWHISTORYON)->EnableWindow(FALSE);
    GetDlgItem(IDC_SETMAXPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEARPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(FALSE);
    GetDlgItem(IDC_UPDATEPROTECTEDPWH)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_PWHSPIN)->EnableWindow(m_SavePWHistory);
    GetDlgItem(IDC_DEFPWHNUM)->EnableWindow(m_SavePWHistory);
    GetDlgItem(IDC_STATIC_NUMPWSDHIST)->EnableWindow(m_SavePWHistory);
  }

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);
  pspin->SetBuddy(GetDlgItem(IDC_DEFPWHNUM));
  pspin->SetRange(M_prefminPWHNumber(), M_prefmaxPWHNumber());
  pspin->SetBase(10);
  pspin->SetPos(m_PWHistoryNumDefault);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_DEDSPIN);
  pspin->SetBuddy(GetDlgItem(IDC_DEFEXPIRYDAYS));
  pspin->SetRange(PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays),
                  PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays));
  pspin->SetBase(10);
  pspin->SetPos(m_PWHDefExpDays);

  // Disable text re: PWHistory changes on existing entries to start
  GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(FALSE);
  GetDlgItem(IDC_UPDATEPROTECTEDPWH)->EnableWindow(FALSE);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_QUESTIONMARK);
    m_Help4.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_RESETPWHISTORYOFFHELP, IDS_RESETPWHISTORYOFF);
    AddTool(IDC_RESETPWHISTORYONHELP, IDS_RESETPWHISTORYON);
    AddTool(IDC_SETMAXPWHISTORYHELP, IDS_SETMAXPWHISTORY);
    AddTool(IDC_CLEARPWHISTORYHELP, IDS_CLEARPWHISTORY);
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

LRESULT COptionsPasswordHistory::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Save current value - make negative if to update protected entries too
  M_PWHAction() = m_PWHAction * (mApplyToProtected == 0 ? 1 : -1);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_SavePWHistory()        != m_SavePWHistory        ||
          (m_SavePWHistory         == TRUE &&
           M_PWHistoryNumDefault() != m_PWHistoryNumDefault) ||
          M_PWHDefExpDays()        != m_PWHDefExpDays)
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

BOOL COptionsPasswordHistory::OnApply()
{
  UpdateData(TRUE);

  M_SavePWHistory() = m_SavePWHistory;
  M_PWHistoryNumDefault() = m_PWHistoryNumDefault;
  M_PWHAction() = m_PWHAction * (mApplyToProtected == 0 ? 1 : -1);
  M_PWHDefExpDays() = m_PWHDefExpDays;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsPasswordHistory::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL COptionsPasswordHistory::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  // Update variable from text box
  CString csText;
  ((CEdit *)GetDlgItem(IDC_DEFPWHNUM))->GetWindowText(csText);
  m_PWHistoryNumDefault = _wtoi(csText);

  // Check that options, as set, are valid.
  if (m_SavePWHistory &&
      ((m_PWHistoryNumDefault < M_prefminPWHNumber()) ||
       (m_PWHistoryNumDefault > M_prefmaxPWHNumber()))) {
    CGeneralMsgBox gmb;
    csText.Format(IDS_DEFAULTNUMPWH, M_prefminPWHNumber(), M_prefmaxPWHNumber());
    gmb.AfxMessageBox(csText);
    ((CEdit *)GetDlgItem(IDC_DEFPWHNUM))->SetFocus();
    return FALSE;
  }
  //End check

  return COptions_PropertyPage::OnKillActive();
}

void COptionsPasswordHistory::OnHelp()
{
  ShowHelp(L"::/html/password_history_tab.html");
}

void COptionsPasswordHistory::OnSavePWHistory() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_SAVEPWHISTORY))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_PWHSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_DEFPWHNUM)->EnableWindow(enable);
  GetDlgItem(IDC_STATIC_NUMPWSDHIST)->EnableWindow(enable);
}

void COptionsPasswordHistory::OnPWHistoryNoAction()
{
  GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(FALSE);
  GetDlgItem(IDC_UPDATEPROTECTEDPWH)->EnableWindow(FALSE);
  ((CButton *)GetDlgItem(IDC_UPDATEPROTECTEDPWH))->SetCheck(BST_UNCHECKED);
  mApplyToProtected = BST_UNCHECKED;
}

void COptionsPasswordHistory::OnPWHistoryDoAction() 
{
  GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(TRUE);
  GetDlgItem(IDC_UPDATEPROTECTEDPWH)->EnableWindow(TRUE);
}

HBRUSH COptionsPasswordHistory::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Database preferences - associated static text
  switch (pWnd->GetDlgCtrlID()) {
    case IDC_STATIC_NUMPWSDHIST:
    case IDC_SAVEPWHISTORY:
    case IDC_STATIC_MANAGEPWH:
    case IDC_PWHISTORYNOACTION:
    case IDC_RESETPWHISTORYOFF:
    case IDC_RESETPWHISTORYON:
    case IDC_SETMAXPWHISTORY:
    case IDC_CLEARPWHISTORY:
    case IDC_STATIC_UPDATEPWHISTORY:
    case IDC_UPDATEPROTECTEDPWH:
      pDC->SetTextColor(CR_DATABASE_OPTIONS);
      pDC->SetBkMode(TRANSPARENT);
      break;
  }

  return hbr;
}
