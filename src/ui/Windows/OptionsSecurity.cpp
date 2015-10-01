/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsSecurity.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"
#include "GeneralMsgBox.h"

#include "core/PwsPlatform.h"
#include "core/PWSprefs.h"
#include "os/env.h"

#include "resource.h"
#include "resource3.h"  // String resources

#include "OptionsSecurity.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity property page

IMPLEMENT_DYNAMIC(COptionsSecurity, COptions_PropertyPage)

COptionsSecurity::COptionsSecurity(CWnd *pParent, st_Opt_master_data *pOPTMD)
: COptions_PropertyPage(pParent, COptionsSecurity::IDD, pOPTMD),
  m_HashIterSliderValue(0), m_HashIter(0)
{
  m_ClearClipboardOnMinimize = M_ClearClipboardOnMinimize();
  m_ClearClipboardOnExit = M_ClearClipboardOnExit();
  m_LockOnMinimize = M_LockOnMinimize();
  m_ConfirmCopy = M_ConfirmCopy();
  m_LockOnWindowLock = M_LockOnWindowLock();
  m_LockOnIdleTimeout = M_LockOnIdleTimeout();
  m_CopyPswdBrowseURL = M_CopyPswdBrowseURL();
  m_IdleTimeOut = M_IdleTimeOut();
  SetHashIter(M_HashIters());
}

COptionsSecurity::~COptionsSecurity()
{
}

void COptionsSecurity::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsSecurity)
  DDX_Check(pDX, IDC_LOCK_TIMER, m_LockOnIdleTimeout);
  DDX_Text(pDX, IDC_IDLE_TIMEOUT, m_IdleTimeOut);
  DDX_Check(pDX, IDC_COPYPSWDURL, m_CopyPswdBrowseURL);
  DDX_Check(pDX, IDC_CLEARBOARDONEXIT, m_ClearClipboardOnExit);
  DDX_Check(pDX, IDC_CLEARBOARDONMINIMIZE, m_ClearClipboardOnMinimize);
  DDX_Check(pDX, IDC_LOCKONMINIMIZE, m_LockOnMinimize);
  DDX_Check(pDX, IDC_CONFIRMCOPY, m_ConfirmCopy);
  DDX_Check(pDX, IDC_LOCKONSCREEN, m_LockOnWindowLock);

  DDX_Control(pDX, IDC_COPYPSWDURL, m_chkbox[0]);
  DDX_Control(pDX, IDC_LOCK_TIMER, m_chkbox[1]);
  //}}AFX_DATA_MAP
  DDX_Slider(pDX, IDC_HASHITERSLIDER, m_HashIterSliderValue);
}

BEGIN_MESSAGE_MAP(COptionsSecurity, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsSecurity)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_LOCK_TIMER, OnLockOnIdleTimeout)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSecurity message handlers

BOOL COptionsSecurity::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  for (int i = 0; i < 2; i++) {
    m_chkbox[i].SetTextColour(CR_DATABASE_OPTIONS);
    m_chkbox[i].ResetBkgColour();//Use current window's background
  }

  OnLockOnIdleTimeout();
  CSpinButtonCtrl *pspin;

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_IDLESPIN);
  pspin->SetBuddy(GetDlgItem(IDC_IDLE_TIMEOUT));
  pspin->SetRange(1, 120);
  pspin->SetBase(10);
  pspin->SetPos(m_IdleTimeOut);

  CSliderCtrl *pslider = (CSliderCtrl *)GetDlgItem(IDC_HASHITERSLIDER);
  pslider->SetRange(MinHIslider, MaxHIslider);
  pslider->SetTicFreq(1);
  pslider->SetPos(m_HashIterSliderValue);

  return TRUE;
}

void COptionsSecurity::UpdateHashIter()
{
  if (m_HashIterSliderValue == 0) {
    m_HashIter = MIN_HASH_ITERATIONS;
  } else {
    const int step = MAX_USABLE_HASH_ITERS/(MaxHIslider - MinHIslider);
    m_HashIter = uint32(m_HashIterSliderValue * step);
  }
}

void COptionsSecurity::SetHashIter(uint32 value)
{
  if (value <= MIN_HASH_ITERATIONS) {
    m_HashIterSliderValue = MinHIslider;
  } else {
    const int step = MAX_USABLE_HASH_ITERS/(MaxHIslider - MinHIslider);
    m_HashIterSliderValue = int(value) / step;
  }
}

LRESULT COptionsSecurity::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
  UpdateData(TRUE);

  // Misc has asked for ClearClipboardOnMinimize value
  switch (wParam) {
    case PPOPT_GET_CCOM:
    {
      BOOL *pCCOM = (BOOL *)lParam;
      ASSERT(pCCOM != NULL);
      *pCCOM = (BOOL)m_ClearClipboardOnMinimize;
      return 1L;
    }
    case PP_DATA_CHANGED:
      if (M_ClearClipboardOnMinimize() != m_ClearClipboardOnMinimize ||
          M_ClearClipboardOnExit()     != m_ClearClipboardOnExit     ||
          M_LockOnMinimize()           != m_LockOnMinimize           ||
          M_ConfirmCopy()              != m_ConfirmCopy              ||
          M_LockOnWindowLock()         != m_LockOnWindowLock         ||
          M_LockOnIdleTimeout()        != m_LockOnIdleTimeout        ||
          M_CopyPswdBrowseURL()        != m_CopyPswdBrowseURL        ||
          (m_LockOnIdleTimeout         == TRUE &&
           M_IdleTimeOut()             != m_IdleTimeOut))
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

  if (m_ClearClipboardOnMinimize &&
      iDoubleClickAction == PWSprefs::DoubleClickCopyPasswordMinimize) {
    gmb.AfxMessageBox(IDS_MINIMIZECONFLICT);

    // Are we the current page, if not activate this page
    COptions_PropertySheet *pPS = (COptions_PropertySheet *)GetParent();
    if (pPS->GetActivePage() != (COptions_PropertyPage *)this)
      pPS->SetActivePage(this);

    GetDlgItem(IDC_CLEARBOARDONMINIMIZE)->SetFocus();
    return FALSE;
  }

  M_ClearClipboardOnMinimize() = m_ClearClipboardOnMinimize;
  M_ClearClipboardOnExit() = m_ClearClipboardOnExit;
  M_LockOnMinimize() = m_LockOnMinimize;
  M_ConfirmCopy() = m_ConfirmCopy;
  M_LockOnWindowLock() = m_LockOnWindowLock;
  M_LockOnIdleTimeout() = m_LockOnIdleTimeout;
  M_CopyPswdBrowseURL() = m_CopyPswdBrowseURL;
  M_IdleTimeOut() = m_IdleTimeOut;
  UpdateHashIter();
  M_HashIters() = m_HashIter;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsSecurity::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
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

void COptionsSecurity::OnHelp()
{
  ShowHelp(L"::/html/security_tab.html");
}

void COptionsSecurity::OnLockOnIdleTimeout() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_LOCK_TIMER))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_IDLESPIN)->EnableWindow(enable);
  GetDlgItem(IDC_IDLE_TIMEOUT)->EnableWindow(enable);
  GetDlgItem(IDC_STATIC_IDLEMINS)->EnableWindow(enable);
}

HBRUSH COptionsSecurity::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Database preferences - associated static text
  switch (pWnd->GetDlgCtrlID()) {
    case IDC_STATIC_IDLEMINS:
    case IDC_COPYPSWDURL:
    case IDC_LOCK_TIMER:
    case IDC_STATIC_HASHITER:
    case IDC_STATIC_HASHITER_MIN:
    case IDC_STATIC_HASHITER_MAX:
      pDC->SetTextColor(CR_DATABASE_OPTIONS);
      pDC->SetBkMode(TRANSPARENT);
      break;
  }

  return hbr;
}
