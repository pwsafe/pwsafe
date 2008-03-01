/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsSystem.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif
#include "OptionsSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem property page

IMPLEMENT_DYNCREATE(COptionsSystem, CPropertyPage)

COptionsSystem::COptionsSystem() : CPWPropertyPage(COptionsSystem::IDD)
{
  //{{AFX_DATA_INIT(COptionsSystem)
  //}}AFX_DATA_INIT
  m_ToolTipCtrl = NULL;
  m_deleteregistry = FALSE;
}

COptionsSystem::~COptionsSystem()
{
  delete m_ToolTipCtrl;
}

void COptionsSystem::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(COptionsSystem)
  DDX_Text(pDX, IDC_MAXREITEMS, m_maxreitems);
  DDV_MinMaxInt(pDX, m_maxreitems, 0, ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1);
  DDX_Check(pDX, IDC_DEFPWUSESYSTRAY, m_usesystemtray);
  DDX_Check(pDX, IDC_STARTUP, m_startup);
  DDX_Text(pDX, IDC_MAXMRUITEMS, m_maxmruitems);
  DDV_MinMaxInt(pDX, m_maxmruitems, 0, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
  DDX_Check(pDX, IDC_MRU_ONFILEMENU, m_mruonfilemenu);
  DDX_Check(pDX, IDC_REGDEL_CB, m_deleteregistry);
  DDX_Check(pDX, IDC_NEVERSAVEDBNAMES, m_neversaveDBnames);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsSystem, CPropertyPage)
  //{{AFX_MSG_MAP(COptionsSystem)
  ON_BN_CLICKED(IDC_DEFPWUSESYSTRAY, OnUseSystemTray)
  ON_BN_CLICKED(IDC_STARTUP, OnStartup)
  ON_BN_CLICKED(IDC_REGDEL_CB, OnSetDeleteRegistry)
  ON_BN_CLICKED(IDC_REGDEL_BTN, OnApplyRegistryDeleteNow)
  ON_BN_CLICKED(IDC_NEVERSAVEDBNAMES, OnNeverSaveDBNames)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem message handlers

void COptionsSystem::OnUseSystemTray() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_DEFPWUSESYSTRAY))->GetCheck() ==
                BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_STATIC_MAXREITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_MAXREITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_RESPIN)->EnableWindow(enable);
}

void COptionsSystem::OnStartup() 
{
  // Startup implies System tray
  bool enable = ((CButton*)GetDlgItem(IDC_STARTUP))->GetCheck() == BST_CHECKED;

  if (enable) {
    ((CButton*)GetDlgItem(IDC_DEFPWUSESYSTRAY))->SetCheck(BST_CHECKED);
    GetDlgItem(IDC_STATIC_MAXREITEMS)->EnableWindow(TRUE);
    GetDlgItem(IDC_MAXREITEMS)->EnableWindow(TRUE);
    GetDlgItem(IDC_RESPIN)->EnableWindow(TRUE);
  }
}

void COptionsSystem::OnSetDeleteRegistry() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_REGDEL_CB))->GetCheck() == 1) ? TRUE : FALSE;

  GetDlgItem(IDC_REGDEL_BTN)->EnableWindow(enable);
}

BOOL COptionsSystem::OnInitDialog() 
{
  BOOL bResult = CPropertyPage::OnInitDialog();

  bool bofferdeleteregistry = 
    PWSprefs::GetInstance()->OfferDeleteRegistry();

  if (!bofferdeleteregistry) {
    GetDlgItem(IDC_REGDEL_GRP)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_REGDEL_CB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_REGDEL_BTN)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_REGDEL_CB)->EnableWindow(FALSE);
  }
  GetDlgItem(IDC_REGDEL_BTN)->EnableWindow(FALSE);

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_RESPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXREITEMS));
  pspin->SetRange(0, ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1);
  pspin->SetBase(10);
  pspin->SetPos(m_maxreitems);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_MRUSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXMRUITEMS));
  pspin->SetRange(0, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
  pspin->SetBase(10);
  pspin->SetPos(m_maxmruitems);

  BOOL enable = ((CButton*)GetDlgItem(IDC_NEVERSAVEDBNAMES))->GetCheck()== BST_CHECKED ? FALSE : TRUE;

  GetDlgItem(IDC_MRUSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_MAXMRUITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_MRU_ONFILEMENU)->EnableWindow(enable);
  GetDlgItem(IDC_STATIC_REMLAST)->EnableWindow(enable);
  GetDlgItem(IDC_STATIC_MAXMRUITEMS)->EnableWindow(enable);

  OnUseSystemTray();

  // Tooltips on Property Pages
  EnableToolTips();

  m_ToolTipCtrl = new CToolTipCtrl;
  if (!m_ToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
    TRACE("Unable To create Property Page ToolTip\n");
    return bResult;
  }

  // Activate the tooltip control.
  m_ToolTipCtrl->Activate(TRUE);
  m_ToolTipCtrl->SetMaxTipWidth(300);
  // Double time to allow reading by user - there is a lot there!
  int iTime = m_ToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_ToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 2 * iTime);

  if (m_ToolTipCtrl != NULL) {
    CString cs_ToolTip(MAKEINTRESOURCE(IDS_REGDEL_CB));
    m_ToolTipCtrl->AddTool(GetDlgItem(IDC_REGDEL_CB), cs_ToolTip);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsSystem::OnApplyRegistryDeleteNow()
{
  UpdateData(TRUE);

  if (m_deleteregistry == TRUE) {
    if (AfxMessageBox(IDS_CONFIRMDELETEREG, MB_YESNO | MB_ICONSTOP) == IDYES)
      PWSprefs::GetInstance()->DeleteRegistryEntries();
  }

  GetDlgItem(IDC_REGDEL_BTN)->EnableWindow(FALSE);
  UpdateData(FALSE);
}

void COptionsSystem::OnNeverSaveDBNames()
{
  BOOL enable = ((CButton*)GetDlgItem(IDC_NEVERSAVEDBNAMES))->GetCheck()== BST_CHECKED ? FALSE : TRUE;

  GetDlgItem(IDC_MRUSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_MAXMRUITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_MRU_ONFILEMENU)->EnableWindow(enable);
  GetDlgItem(IDC_STATIC_REMLAST)->EnableWindow(enable);
  GetDlgItem(IDC_STATIC_MAXMRUITEMS)->EnableWindow(enable);
}

BOOL COptionsSystem::OnKillActive()
{
  // Needed as we have DDV routines.

  return CPropertyPage::OnKillActive();
}

// Override PreTranslateMessage() so RelayEvent() can be 
// called to pass a mouse message to CPWSOptions's 
// tooltip control for processing.
BOOL COptionsSystem::PreTranslateMessage(MSG* pMsg) 
{
  if (m_ToolTipCtrl != NULL)
    m_ToolTipCtrl->RelayEvent(pMsg);

  return CPropertyPage::PreTranslateMessage(pMsg);
}

