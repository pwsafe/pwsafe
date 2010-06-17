/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsSystem.cpp : implementation file
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
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#include "OptionsSystem.h" // Must be after resource.h

extern bool OfferConfigMigration();
extern bool PerformConfigMigration();

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem property page

bool COptionsSystem::m_bShowConfigFile = false;

IMPLEMENT_DYNCREATE(COptionsSystem, COptions_PropertyPage)

COptionsSystem::COptionsSystem() 
  : COptions_PropertyPage(COptionsSystem::IDD), m_pToolTipCtrl(NULL),
  m_deleteregistry(FALSE), m_migrate2appdata(FALSE)
{
#ifdef _DEBUG
  m_bShowConfigFile = true;
#endif
}

COptionsSystem::~COptionsSystem()
{
  delete m_pToolTipCtrl;
}

void COptionsSystem::DoDataExchange(CDataExchange* pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsSystem)
  DDX_Text(pDX, IDC_MAXREITEMS, m_maxreitems);
  DDV_MinMaxInt(pDX, m_maxreitems, 0, ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1);
  DDX_Check(pDX, IDC_DEFPWUSESYSTRAY, m_usesystemtray);
  DDX_Check(pDX, IDC_STARTUP, m_startup);
  DDX_Text(pDX, IDC_MAXMRUITEMS, m_maxmruitems);
  DDV_MinMaxInt(pDX, m_maxmruitems, 0, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
  DDX_Check(pDX, IDC_MRU_ONFILEMENU, m_mruonfilemenu);
  DDX_Check(pDX, IDC_REGDEL, m_deleteregistry);
  DDX_Check(pDX, IDC_MIGRATETOAPPDATA, m_migrate2appdata);
  DDX_Check(pDX, IDC_DEFAULTOPENRO, m_defaultopenro);
  DDX_Check(pDX, IDC_MULTIPLEINSTANCES, m_multipleinstances);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsSystem, CPWPropertyPage)
  //{{AFX_MSG_MAP(COptionsSystem)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_DEFPWUSESYSTRAY, OnUseSystemTray)
  ON_BN_CLICKED(IDC_STARTUP, OnStartup)
  ON_BN_CLICKED(IDC_REGDEL, OnSetDeleteRegistry)
  ON_BN_CLICKED(IDC_MIGRATETOAPPDATA, OnSetMigrate2Appdata)
  ON_BN_CLICKED(IDC_APPLYCONFIGCHANGES, OnApplyConfigChanges)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem message handlers

BOOL COptionsSystem::PreTranslateMessage(MSG* pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsSystem::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/system_tab.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

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
  BOOL enable = (((CButton*)GetDlgItem(IDC_REGDEL))->GetCheck() == 1) ? TRUE : FALSE;

  GetDlgItem(IDC_APPLYCONFIGCHANGES)->EnableWindow(enable);
}

void COptionsSystem::OnSetMigrate2Appdata()
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_MIGRATETOAPPDATA))->GetCheck() == 1) ? TRUE : FALSE;

  GetDlgItem(IDC_APPLYCONFIGCHANGES)->EnableWindow(enable);
}

BOOL COptionsSystem::OnInitDialog() 
{
  BOOL bResult = COptions_PropertyPage::OnInitDialog();

  PWSprefs *prefs = PWSprefs::GetInstance();
  if (!m_bShowConfigFile) {
    GetDlgItem(IDC_STATIC_CONFIGFILE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_RWSTATUS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_CONFIGFILE)->ShowWindow(SW_HIDE);
  } else {
    PWSprefs::ConfigOption configoption;
    std::wstring wsCF = prefs->GetConfigFile(configoption);
    std::wstring wsCO(L"");
    switch (configoption) {
      case PWSprefs::CF_NONE:
        LoadAString(wsCF, IDS_NONE);
        break;
      case PWSprefs::CF_REGISTRY:
        LoadAString(wsCF, IDS_REGISTRY);
        break;
      case PWSprefs::CF_FILE_RO:
        LoadAString(wsCO, IDS_READ_ONLY);
        break;
      case PWSprefs::CF_FILE_RW:
      case PWSprefs::CF_FILE_RW_NEW:
        LoadAString(wsCO, IDS_READ_WRITE);
        break;
      default:
        ASSERT(0);
    }
    GetDlgItem(IDC_CONFIGFILE)->SetWindowText(wsCF.c_str());
    GetDlgItem(IDC_STATIC_RWSTATUS)->SetWindowText(wsCO.c_str());
  }

  bool bofferdeleteregistry = prefs->OfferDeleteRegistry();

  bool boffermigrate2appdata = OfferConfigMigration();

  if (!bofferdeleteregistry) {
    GetDlgItem(IDC_REGDEL)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_REGDEL)->EnableWindow(FALSE);
  }

  if (!boffermigrate2appdata) {
    GetDlgItem(IDC_MIGRATETOAPPDATA)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_MIGRATETOAPPDATA)->EnableWindow(FALSE);
  }

  if (!bofferdeleteregistry && !boffermigrate2appdata) {
    GetDlgItem(IDC_CONFIG_GRP)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_APPLYCONFIGCHANGES)->ShowWindow(SW_HIDE);
  } else {
    GetDlgItem(IDC_APPLYCONFIGCHANGES)->ShowWindow(SW_SHOW);
  }

  GetDlgItem(IDC_APPLYCONFIGCHANGES)->EnableWindow(FALSE);

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_RESPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXREITEMS));
  pspin->SetRange(0, ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1);
  pspin->SetBase(10);
  pspin->SetPos(m_maxreitems);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_MRUSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_MAXMRUITEMS));
  pspin->SetRange(0, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
  pspin->SetBase(10);
  pspin->SetPos(m_maxmruitems);

  OnUseSystemTray();

  m_savemaxreitems = m_maxreitems;
  m_saveusesystemtray = m_usesystemtray;
  m_savestartup = m_startup;
  m_savemaxmruitems = m_maxmruitems;
  m_savemruonfilemenu = m_mruonfilemenu;
  m_savedeleteregistry = m_deleteregistry;
  m_savemigrate2appdata = m_migrate2appdata;
  m_savedefaultopenro = m_defaultopenro;
  m_savemultipleinstances = m_multipleinstances;

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create Property Page ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return bResult;
  }

  // Tooltips on Property Pages
  EnableToolTips();

  // Activate the tooltip control.
  m_pToolTipCtrl->Activate(TRUE);
  m_pToolTipCtrl->SetMaxTipWidth(300);
  // Double time to allow reading by user - there is a lot there!
  int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 2 * iTime);

  if (m_pToolTipCtrl != NULL) {
    CString cs_ToolTip(MAKEINTRESOURCE(IDS_REGDEL));
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_REGDEL), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_MIGRATETOAPPDATA);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_MIGRATETOAPPDATA), cs_ToolTip);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsSystem::OnApplyConfigChanges()
{
  UpdateData(TRUE);

  CGeneralMsgBox gmb;
  if (m_deleteregistry == TRUE) {
    if (gmb.AfxMessageBox(IDS_CONFIRMDELETEREG, MB_YESNO | MB_ICONSTOP) == IDYES) {
      PWSprefs::GetInstance()->DeleteRegistryEntries();
      GetDlgItem(IDC_REGDEL)->EnableWindow(FALSE);
    }
  }

  if (m_migrate2appdata == TRUE) {
    GetDlgItem(IDC_MIGRATETOAPPDATA)->EnableWindow(FALSE);
    PerformConfigMigration();
  }

  if (!GetDlgItem(IDC_REGDEL)->IsWindowEnabled() && 
      !GetDlgItem(IDC_MIGRATETOAPPDATA)->IsWindowEnabled())
    GetDlgItem(IDC_APPLYCONFIGCHANGES)->EnableWindow(FALSE);

  UpdateData(FALSE);
}

BOOL COptionsSystem::OnKillActive()
{
  // Needed as we have DDV routines.
  return CPWPropertyPage::OnKillActive();
}

LRESULT COptionsSystem::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (m_saveusesystemtray     != m_usesystemtray   ||
          (m_usesystemtray        == TRUE &&
           m_savemaxreitems       != m_maxreitems)     ||
          m_savestartup           != m_startup         ||
          m_savemaxmruitems       != m_maxmruitems     ||
          m_savemruonfilemenu     != m_mruonfilemenu   ||
          m_savedeleteregistry    != m_deleteregistry  ||
          m_savemigrate2appdata   != m_migrate2appdata ||
          m_savedefaultopenro     != m_defaultopenro   ||
          m_savemultipleinstances != m_multipleinstances)
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
