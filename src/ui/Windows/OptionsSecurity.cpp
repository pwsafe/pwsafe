/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsSecurity.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"

#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "OptionsSecurity.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity property page

IMPLEMENT_DYNCREATE(COptionsSecurity, COptions_PropertyPage)

COptionsSecurity::COptionsSecurity()
  : COptions_PropertyPage(COptionsSecurity::IDD)
{
  //{{AFX_DATA_INIT(COptionsSecurity)
  //}}AFX_DATA_INIT
}

COptionsSecurity::~COptionsSecurity()
{
}

void COptionsSecurity::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsSecurity)
  DDX_Check(pDX, IDC_CLEARBOARDONEXIT, m_clearclipboardonexit);
  DDX_Check(pDX, IDC_CLEARBOARDONMINIMIZE, m_clearclipboardonminimize);
  DDX_Check(pDX, IDC_LOCKONMINIMIZE, m_LockOnMinimize);
  DDX_Check(pDX, IDC_CONFIRMCOPY, m_confirmcopy);
  DDX_Check(pDX, IDC_LOCKONSCREEN, m_LockOnWindowLock);
  DDX_Check(pDX, IDC_LOCK_TIMER, m_LockOnIdleTimeout);
  DDX_Text(pDX, IDC_IDLE_TIMEOUT, m_IdleTimeOut);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsSecurity, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsSecurity)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_LOCK_TIMER, OnLockOnIdleTimeout)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity message handlers

BOOL COptionsSecurity::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsSecurity::OnLockOnIdleTimeout() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_LOCK_TIMER))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_IDLESPIN)->EnableWindow(enable);
  GetDlgItem(IDC_IDLE_TIMEOUT)->EnableWindow(enable);
}

void COptionsSecurity::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/security_tab.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

BOOL COptionsSecurity::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  OnLockOnIdleTimeout();
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_IDLESPIN);

  pspin->SetBuddy(GetDlgItem(IDC_IDLE_TIMEOUT));
  pspin->SetRange(1, 120);
  pspin->SetBase(10);
  pspin->SetPos(m_IdleTimeOut);

  m_saveclearclipboardonminimize = m_clearclipboardonminimize;
  m_saveclearclipboardonexit = m_clearclipboardonexit;
  m_saveLockOnMinimize = m_LockOnMinimize;
  m_saveconfirmcopy = m_confirmcopy;
  m_saveLockOnWindowLock = m_LockOnWindowLock;
  m_saveLockOnIdleTimeout = m_LockOnIdleTimeout;
  m_saveIdleTimeOut = m_IdleTimeOut;
 
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT COptionsSecurity::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
  UpdateData(TRUE);

  // Misc has asked for ClearClipboardOnMinimize value
  switch (wParam) {
    case PPOPT_GET_CCOM:
      {
      BOOL * pCCOM = (BOOL *)lParam;
      ASSERT(pCCOM != NULL);
      *pCCOM = (BOOL)m_clearclipboardonminimize;
      }
      return 1L;
    case PP_DATA_CHANGED:
      if (m_saveclearclipboardonminimize != m_clearclipboardonminimize ||
          m_saveclearclipboardonexit     != m_clearclipboardonexit     ||
          m_saveLockOnMinimize           != m_LockOnMinimize           ||
          m_saveconfirmcopy              != m_confirmcopy              ||
          m_saveLockOnWindowLock         != m_LockOnWindowLock         ||
          m_saveLockOnIdleTimeout        != m_LockOnIdleTimeout        ||
          (m_LockOnIdleTimeout           == TRUE &&
           m_saveIdleTimeOut              != m_IdleTimeOut))
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
    default:
      break;
  }
  return 0L;
}

BOOL COptionsSecurity::OnKillActive()
{
  CGeneralMsgBox gmb;
  // Check that options, as set, are valid.
  if ((m_IdleTimeOut < 1) || (m_IdleTimeOut > 120)) {
    gmb.AfxMessageBox(IDS_INVALIDTIMEOUT);
    ((CEdit*)GetDlgItem(IDC_IDLE_TIMEOUT))->SetFocus();
    return FALSE;
  }

  return COptions_PropertyPage::OnKillActive();
}

BOOL COptionsSecurity::OnApply() 
{
  UpdateData(TRUE);

  CGeneralMsgBox gmb;
  // Go ask Misc for DoubleClickAction value
  int iDoubleClickAction;
  if (QuerySiblings(PPOPT_GET_DCA, (LPARAM)&iDoubleClickAction) == 0L) {
    // Misc not loaded - get from Prefs
    iDoubleClickAction = 
        PWSprefs::GetInstance()->GetPref(PWSprefs::DoubleClickAction);
  }

  if (m_clearclipboardonminimize &&
      iDoubleClickAction == PWSprefs::DoubleClickCopyPasswordMinimize) {
    gmb.AfxMessageBox(IDS_MINIMIZECONFLICT);

    // Are we the current page, if not activate this page
    COptions_PropertySheet *pPS = (COptions_PropertySheet *)GetParent();
    if (pPS->GetActivePage() != (COptions_PropertyPage *)this)
      pPS->SetActivePage(this);

    GetDlgItem(IDC_CLEARBOARDONMINIMIZE)->SetFocus();
    return FALSE;
  }

  return COptions_PropertyPage::OnApply();
}
