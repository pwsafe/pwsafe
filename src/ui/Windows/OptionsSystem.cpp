/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsSystem.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"    // For Help
#include "Options_PropertySheet.h"
#include "GeneralMsgBox.h"

#include "core/PwsPlatform.h"
#include "core/PWSprefs.h"
#include "core/SysInfo.h"

#include "resource.h"
#include "resource2.h"  // Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

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

IMPLEMENT_DYNAMIC(COptionsSystem, COptions_PropertyPage)

COptionsSystem::COptionsSystem(CWnd *pParent, st_Opt_master_data *pOPTMD) 
: COptions_PropertyPage(pParent,
                        COptionsSystem::IDD, COptionsSystem::IDD_SHORT,
                        pOPTMD),
  m_DeleteRegistry(FALSE), m_saveDeleteRegistry(FALSE),
  m_Migrate2Appdata(FALSE), m_saveMigrate2Appdata(FALSE)
{
  m_UseSystemTray = M_UseSystemTray();
  m_HideSystemTray = M_HideSystemTray();
  m_Startup = M_Startup();
  m_MRUOnFileMenu = M_MRUOnFileMenu();
  m_DefaultOpenRO = M_DefaultOpenRO();
  m_MultipleInstances = M_MultipleInstances();
  m_MaxREItems = M_MaxREItems();
  m_MaxMRUItems = M_MaxMRUItems();
  m_InitialHotkeyState = M_AppHotKeyEnabled();
}

void COptionsSystem::DoDataExchange(CDataExchange *pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsSystem)
  DDX_Text(pDX, IDC_MAXREITEMS, m_MaxREItems);
  DDV_MinMaxInt(pDX, m_MaxREItems, 0, ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1);
  DDX_Check(pDX, IDC_DEFPWUSESYSTRAY, m_UseSystemTray);
  DDX_Check(pDX, IDC_DEFPWHIDESYSTRAY, m_HideSystemTray);
  DDX_Check(pDX, IDC_STARTUP, m_Startup);
  DDX_Text(pDX, IDC_MAXMRUITEMS, m_MaxMRUItems);
  DDV_MinMaxInt(pDX, m_MaxMRUItems, 0, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
  DDX_Check(pDX, IDC_MRU_ONFILEMENU, m_MRUOnFileMenu);
  DDX_Check(pDX, IDC_REGDEL, m_DeleteRegistry);
  DDX_Check(pDX, IDC_MIGRATETOAPPDATA, m_Migrate2Appdata);
  DDX_Check(pDX, IDC_DEFAULTOPENRO, m_DefaultOpenRO);
  DDX_Check(pDX, IDC_MULTIPLEINSTANCES, m_MultipleInstances);

  DDX_Control(pDX, IDC_REGDELHELP, m_Help1);
  DDX_Control(pDX, IDC_MIGRATETOAPPDATAHELP, m_Help2);
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

BOOL COptionsSystem::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  PWSprefs *prefs = PWSprefs::GetInstance();

  PWSprefs::ConfigOption configoption;
  StringX sx_CF = prefs->GetConfigFile(configoption).c_str();
  std::wstring  wsCO(L"");
  switch (configoption) {
    case PWSprefs::CF_NONE:
      LoadAString(sx_CF, IDS_NONE);
      break;
    case PWSprefs::CF_REGISTRY:
      LoadAString(sx_CF, IDS_REGISTRY);
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

  CString cs_text;

  // R/W status
  GetDlgItem(IDC_STATIC_RWSTATUS)->SetWindowText(wsCO.c_str());

  // Config file name & location
  cs_text = PWSUtil::NormalizeTTT(sx_CF, 60).c_str();
  GetDlgItem(IDC_CONFIGFILE)->SetWindowText(cs_text);

  // Effective host & user used in config file
  if (configoption == PWSprefs::CF_FILE_RO || 
      configoption == PWSprefs::CF_FILE_RW ||
      configoption == PWSprefs::CF_FILE_RW_NEW) {
    stringT hn = SysInfo::GetInstance()->GetEffectiveHost();
    PWSprefs::XMLify(charT('H'), hn);
    stringT un = SysInfo::GetInstance()->GetEffectiveUser();
    PWSprefs::XMLify(charT('u'), un);

    cs_text.Format(IDS_HOSTUSER, static_cast<LPCWSTR>(hn.c_str()), static_cast<LPCWSTR>(un.c_str()));
    GetDlgItem(IDC_STATIC_HOSTUSER)->SetWindowText(cs_text);
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
  pspin->SetRange(M_prefminREItems(), M_prefmaxREItems());
  pspin->SetBase(10);
  pspin->SetPos(m_MaxREItems);

  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_MRUSPIN);
  pspin->SetBuddy(GetDlgItem(IDC_MAXMRUITEMS));
  pspin->SetRange(M_prefminMRU(), M_prefmaxMRU());
  pspin->SetBase(10);
  pspin->SetPos(m_MaxMRUItems);

  OnUseSystemTray();

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_REGDELHELP, IDS_REGDEL);
    AddTool(IDC_MIGRATETOAPPDATAHELP, IDS_MIGRATETOAPPDATA);
    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
  }

  if (!bofferdeleteregistry) {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
  }

  if (!boffermigrate2appdata) {
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT COptionsSystem::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_UseSystemTray()     != m_UseSystemTray     ||
          M_HideSystemTray()    != m_HideSystemTray    ||
          (m_UseSystemTray      == TRUE &&
           M_MaxREItems()       != m_MaxREItems)       ||
          M_Startup()           != m_Startup           ||
          M_MaxMRUItems()       != m_MaxMRUItems       ||
          M_MRUOnFileMenu()     != m_MRUOnFileMenu     ||
          M_DefaultOpenRO()     != m_DefaultOpenRO     ||
          M_MultipleInstances() != m_MultipleInstances ||
          m_saveDeleteRegistry  != m_DeleteRegistry    ||
          m_saveMigrate2Appdata != m_Migrate2Appdata)
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

BOOL COptionsSystem::OnApply()
{
  UpdateData(TRUE);

  M_UseSystemTray() = m_UseSystemTray;
  M_HideSystemTray() = m_HideSystemTray;
  M_Startup() = m_Startup;
  M_MRUOnFileMenu() = m_MRUOnFileMenu;
  M_DefaultOpenRO() = m_DefaultOpenRO;
  M_MultipleInstances() = m_MultipleInstances;
  M_MaxREItems() = m_MaxREItems;
  M_MaxMRUItems() = m_MaxMRUItems;
  M_AppHotKeyEnabled() = m_InitialHotkeyState;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsSystem::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL COptionsSystem::OnKillActive()
{
  // Needed as we have DDV routines.
  return CPWPropertyPage::OnKillActive();
}

void COptionsSystem::OnHelp()
{
  ShowHelp(L"::/html/system_tab.html");
}

void COptionsSystem::OnUseSystemTray() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_DEFPWUSESYSTRAY))->GetCheck() ==
                BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_STATIC_MAXREITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_MAXREITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_RESPIN)->EnableWindow(enable);

  if (enable == TRUE) {
    // Check if user has the Misc PP open and hot key set
    if (QuerySiblings(PPOPT_HOTKEY_SET, 0L) == 1L) {
      // Yes - open and hot key is set
      GetDlgItem(IDC_DEFPWHIDESYSTRAY)->EnableWindow(TRUE);
    } else {
      // No - Not open - then take initial value as the answer
      GetDlgItem(IDC_DEFPWHIDESYSTRAY)->EnableWindow(m_InitialHotkeyState);
    }
  }
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

void COptionsSystem::OnApplyConfigChanges()
{
  UpdateData(TRUE);

  CGeneralMsgBox gmb;
  if (m_DeleteRegistry == TRUE) {
    if (gmb.AfxMessageBox(IDS_CONFIRMDELETEREG, MB_YESNO | MB_ICONSTOP) == IDYES) {
      PWSprefs::GetInstance()->DeleteRegistryEntries();
      GetDlgItem(IDC_REGDEL)->EnableWindow(FALSE);
    }
  }

  if (m_Migrate2Appdata == TRUE) {
    GetDlgItem(IDC_MIGRATETOAPPDATA)->EnableWindow(FALSE);
    PerformConfigMigration();
  }

  if (!GetDlgItem(IDC_REGDEL)->IsWindowEnabled() && 
      !GetDlgItem(IDC_MIGRATETOAPPDATA)->IsWindowEnabled())
    GetDlgItem(IDC_APPLYCONFIGCHANGES)->EnableWindow(FALSE);

  UpdateData(FALSE);
}

BOOL COptionsSystem::OnSetActive()
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_DEFPWUSESYSTRAY))->GetCheck() ==
                BST_CHECKED) ? TRUE : FALSE;

  if (enable == TRUE) {
    // Check if user has the Misc PP open and hot key set
    if (QuerySiblings(PPOPT_HOTKEY_SET, 0L) == 1L) {
      // Yes - open and hot key is set
      GetDlgItem(IDC_DEFPWHIDESYSTRAY)->EnableWindow(TRUE);
    } else {
      // No - Not open - then take initial value as the answer
      GetDlgItem(IDC_DEFPWHIDESYSTRAY)->EnableWindow(m_InitialHotkeyState);
    }
  }

  return CPWPropertyPage::OnSetActive();
}
