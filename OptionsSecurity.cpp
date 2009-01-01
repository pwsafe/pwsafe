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

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif
#include "OptionsSecurity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity property page

IMPLEMENT_DYNCREATE(COptionsSecurity, CPropertyPage)

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
  CPropertyPage::DoDataExchange(pDX);
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

BEGIN_MESSAGE_MAP(COptionsSecurity, CPropertyPage)
  //{{AFX_MSG_MAP(COptionsSecurity)
  ON_BN_CLICKED(IDC_LOCKBASE, OnLockbase)
  ON_BN_CLICKED(IDC_LOCK_TIMER, OnLockbase)
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
  CPropertyPage::OnInitDialog();

  OnLockbase();
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_IDLESPIN);

  pspin->SetBuddy(GetDlgItem(IDC_IDLE_TIMEOUT));
  pspin->SetRange(1, 120);
  pspin->SetBase(10);
  pspin->SetPos(m_IdleTimeOut);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL COptionsSecurity::OnKillActive()
{
  CPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if ((m_IdleTimeOut < 1) || (m_IdleTimeOut > 120)) {
    AfxMessageBox(IDS_INVALIDTIMEOUT);
    ((CEdit*)GetDlgItem(IDC_IDLE_TIMEOUT))->SetFocus();
    return FALSE;
  }

  return TRUE;
}
