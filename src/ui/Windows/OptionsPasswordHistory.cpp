/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsPasswordHistory.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"  // needed for DboxMain::UpdatePasswordHistory
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"

#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "OptionsPasswordHistory.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory property page

IMPLEMENT_DYNCREATE(COptionsPasswordHistory, COptions_PropertyPage)

COptionsPasswordHistory::COptionsPasswordHistory()
  : COptions_PropertyPage(COptionsPasswordHistory::IDD),
  m_pToolTipCtrl(NULL), m_pDboxMain(NULL), m_pwhaction(0)
{
  //{{AFX_DATA_INIT(COptionsPasswordHistory)
  //}}AFX_DATA_INIT
}

COptionsPasswordHistory::~COptionsPasswordHistory()
{
  delete m_pToolTipCtrl;
}

void COptionsPasswordHistory::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsPasswordHistory)
  DDX_Check(pDX, IDC_SAVEPWHISTORY, m_savepwhistory);
  DDX_Text(pDX, IDC_DEFPWHNUM, m_pwhistorynumdefault);
  //}}AFX_DATA_MAP
  DDX_Radio(pDX, IDC_PWHISTORYNOACTION, m_pwhaction);
}

BEGIN_MESSAGE_MAP(COptionsPasswordHistory, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsPasswordHistory)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_SAVEPWHISTORY, OnSavePWHistory)
  ON_BN_CLICKED(IDC_PWHISTORYNOACTION, OnPWHistoryNoAction)
  ON_BN_CLICKED(IDC_RESETPWHISTORYOFF, OnPWHistoryDoAction)
  ON_BN_CLICKED(IDC_RESETPWHISTORYON, OnPWHistoryDoAction)
  ON_BN_CLICKED(IDC_SETMAXPWHISTORY, OnPWHistoryDoAction)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory message handlers

BOOL COptionsPasswordHistory::PreTranslateMessage(MSG* pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsPasswordHistory::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/password_history_tab.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

BOOL COptionsPasswordHistory::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWHNUM));
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos(m_pwhistorynumdefault);

  GetDlgItem(IDC_PWHSPIN)->EnableWindow(m_savepwhistory);
  GetDlgItem(IDC_DEFPWHNUM)->EnableWindow(m_savepwhistory);

  m_savesavepwhistory = m_savepwhistory;
  m_savepwhistorynumdefault = m_pwhistorynumdefault;

  // Disable text re: PWHistory changes on existing entries to start
  GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(FALSE);

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
  cs_ToolTip.LoadString(IDS_RESETPWHISTORYOFF);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_RESETPWHISTORYOFF), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_RESETPWHISTORYON);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_RESETPWHISTORYON), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_SETMAXPWHISTORY);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_SETMAXPWHISTORY), cs_ToolTip);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL COptionsPasswordHistory::OnKillActive()
{
  CGeneralMsgBox gmb;
  // Check that options, as set, are valid.
  if (m_savepwhistory && ((m_pwhistorynumdefault < 1) || (m_pwhistorynumdefault > 255))) {
    gmb.AfxMessageBox(IDS_DEFAULTNUMPWH);
    ((CEdit*)GetDlgItem(IDC_DEFPWHNUM))->SetFocus();
    return FALSE;
  }
  //End check

  return COptions_PropertyPage::OnKillActive();;
}

void COptionsPasswordHistory::OnSavePWHistory() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_SAVEPWHISTORY))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_PWHSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_DEFPWHNUM)->EnableWindow(enable);
}

LRESULT COptionsPasswordHistory::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (m_savesavepwhistory        != m_savepwhistory        ||
          (m_savepwhistory           == TRUE &&
           m_savepwhistorynumdefault != m_pwhistorynumdefault))
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

void COptionsPasswordHistory::OnPWHistoryNoAction()
{
  GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(FALSE);
}

void COptionsPasswordHistory::OnPWHistoryDoAction() 
{
  GetDlgItem(IDC_STATIC_UPDATEPWHISTORY)->EnableWindow(TRUE);
}
