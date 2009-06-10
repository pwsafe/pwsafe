/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsSecurity.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif
#include "OptionsSecurity.h"
#include "Options_PropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity property page

IMPLEMENT_DYNCREATE(COptionsSecurity, CPWPropertyPage)

COptionsSecurity::COptionsSecurity() : CPWPropertyPage(COptionsSecurity::IDD)
{
  //{{AFX_DATA_INIT(COptionsSecurity)
  //}}AFX_DATA_INIT
}

COptionsSecurity::~COptionsSecurity()
{
}

void COptionsSecurity::DoDataExchange(CDataExchange* pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsSecurity)
  DDX_Check(pDX, IDC_CLEARBOARDONEXIT, m_clearclipboardonexit);
  DDX_Check(pDX, IDC_CLEARBOARDONMINIMIZE, m_clearclipboardonminimize);
  DDX_Check(pDX, IDC_LOCKBASE, m_lockdatabase);
  DDX_Check(pDX, IDC_CONFIRMCOPY, m_confirmcopy);
  DDX_Check(pDX, IDC_LOCKONSCREEN, m_LockOnWindowLock);
  DDX_Check(pDX, IDC_LOCK_TIMER, m_LockOnIdleTimeout);
  DDX_Text(pDX, IDC_IDLE_TIMEOUT, m_IdleTimeOut);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsSecurity, CPWPropertyPage)
  //{{AFX_MSG_MAP(COptionsSecurity)
  ON_BN_CLICKED(IDC_LOCKBASE, OnLockbase)
  ON_BN_CLICKED(IDC_LOCK_TIMER, OnLockbase)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity message handlers

void COptionsSecurity::OnLockbase() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_LOCK_TIMER))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_IDLESPIN)->EnableWindow(enable);
  GetDlgItem(IDC_IDLE_TIMEOUT)->EnableWindow(enable);
}

BOOL COptionsSecurity::OnInitDialog() 
{
  CPWPropertyPage::OnInitDialog();

  OnLockbase();
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_IDLESPIN);

  pspin->SetBuddy(GetDlgItem(IDC_IDLE_TIMEOUT));
  pspin->SetRange(1, 120);
  pspin->SetBase(10);
  pspin->SetPos(m_IdleTimeOut);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT COptionsSecurity::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
  UpdateData(TRUE);

  // Misc has asked for ClearClipboardOnMinimize value
  switch (wParam) {
    case COptions_PropertySheet::PP_GET_CCOM:
      {
      BOOL * pCCOM = (BOOL *)lParam;
      ASSERT(pCCOM != NULL);
      *pCCOM = (BOOL)m_clearclipboardonminimize;
      }
      return 1L;
    default:
      break;
  }
  return 0L;
}

BOOL COptionsSecurity::OnKillActive()
{
  CPWPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if ((m_IdleTimeOut < 1) || (m_IdleTimeOut > 120)) {
    AfxMessageBox(IDS_INVALIDTIMEOUT);
    ((CEdit*)GetDlgItem(IDC_IDLE_TIMEOUT))->SetFocus();
    return FALSE;
  }

  return TRUE;
}

BOOL COptionsSecurity::OnApply() 
{
  UpdateData(TRUE);

  // Go ask Misc for DoubleClickAction value
  int iDoubleClickAction;
  if (QuerySiblings(COptions_PropertySheet::PP_GET_DCA,
                    (LPARAM)&iDoubleClickAction) == 0L) {
    // Misc not loaded - get from Prefs
    iDoubleClickAction = 
        PWSprefs::GetInstance()->GetPref(PWSprefs::DoubleClickAction);
  }

  if (m_clearclipboardonminimize &&
      iDoubleClickAction == PWSprefs::DoubleClickCopyPasswordMinimize) {
    AfxMessageBox(IDS_MINIMIZECONFLICT);

    // Are we the current page, if not activate this page
    COptions_PropertySheet *pPS = (COptions_PropertySheet *)GetParent();
    if (pPS->GetActivePage() != (CPWPropertyPage *)this)
      pPS->SetActivePage(this);

    GetDlgItem(IDC_CLEARBOARDONMINIMIZE)->SetFocus();
    return FALSE;
  }

  return CPWPropertyPage::OnApply();
}
