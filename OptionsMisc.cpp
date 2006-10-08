// OptionsMisc.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"
#include "corelib/Pwsprefs.h" // for DoubleClickAction enums

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif
#include "OptionsMisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc property page

IMPLEMENT_DYNCREATE(COptionsMisc, CPropertyPage)

COptionsMisc::COptionsMisc() : CPropertyPage(COptionsMisc::IDD)
{
	//{{AFX_DATA_INIT(COptionsMisc)
	//}}AFX_DATA_INIT
}

COptionsMisc::~COptionsMisc()
{
}

void COptionsMisc::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	
	//{{AFX_DATA_MAP(COptionsMisc)
	DDX_Check(pDX, IDC_CONFIRMDELETE, m_confirmdelete);
	DDX_Check(pDX, IDC_SAVEIMMEDIATELY, m_saveimmediately);
	DDX_Check(pDX, IDC_MAINTAINDATETIMESTAMPS, m_maintaindatetimestamps);
	DDX_Check(pDX, IDC_ESC_EXITS, m_escexits);
	DDX_Control(pDX, IDC_DOUBLE_CLICK_ACTION, m_dblclk_cbox);
	DDX_Check(pDX, IDC_HOTKEY_ENABLE, m_hotkey_enabled);
// JHF class CHotKeyCtrl not defined under WinCE
#if !defined(POCKET_PC)
	DDX_Control(pDX, IDC_HOTKEY_CTRL, m_hotkey);
#endif
	DDX_Check(pDX, IDC_USEDEFUSER, m_usedefuser);
	DDX_Check(pDX, IDC_QUERYSETDEF, m_querysetdef);
	DDX_Text(pDX, IDC_DEFUSERNAME, m_defusername);
	//}}AFX_DATA_MAP

}

BEGIN_MESSAGE_MAP(COptionsMisc, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsMisc)
	ON_BN_CLICKED(IDC_HOTKEY_ENABLE, OnEnableHotKey)
	ON_BN_CLICKED(IDC_USEDEFUSER, OnUsedefuser)
	ON_CBN_SELCHANGE(IDC_DOUBLE_CLICK_ACTION, OnComboChanged) 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsMisc::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  // For some reason, MFC calls us twice when initializing.
  // Populate the combo box only once.
  if(m_dblclk_cbox.GetCount() == 0) {
	m_dblclk_cbox.AddString(_T("Copies password to clipboard"));
	m_dblclk_cbox.AddString(_T("View/Edit selected entry"));
	m_dblclk_cbox.AddString(_T("Autotype"));
	m_dblclk_cbox.AddString(_T("Browse to URL"));
  }

  if (m_doubleclickaction < PWSprefs::minDCA ||
	  m_doubleclickaction > PWSprefs::maxDCA)
  	m_doubleclickaction = PWSprefs::DoubleClickCopy;
  m_dblclk_cbox.SetCurSel(m_doubleclickaction);

  // JHF ditto here
#if !defined(POCKET_PC)
  m_hotkey.SetHotKey(LOWORD(m_hotkey_value),HIWORD(m_hotkey_value));
  if (m_hotkey_enabled == FALSE)
    m_hotkey.EnableWindow(FALSE);
#endif

  OnUsedefuser();

  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// COptionsMisc message handlers

void COptionsMisc::OnEnableHotKey() 
{
	// JHF : no hotkeys on WinCE
#if !defined(POCKET_PC)
  if (((CButton*)GetDlgItem(IDC_HOTKEY_ENABLE))->GetCheck() == 1) {
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(TRUE);
	GetDlgItem(IDC_HOTKEY_CTRL)->SetFocus();
  } else
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(FALSE);
#endif
}

void COptionsMisc::OnComboChanged() 
{
	m_doubleclickaction = m_dblclk_cbox.GetCurSel();
}

void COptionsMisc::OnUsedefuser() 
{
   if (((CButton*)GetDlgItem(IDC_USEDEFUSER))->GetCheck() == 1)
   {
      GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(TRUE);
      GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(TRUE);
      GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(FALSE);
   }
   else
   {
      GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(FALSE);
      GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(TRUE);
   }
}

void COptionsMisc::OnOK() 
{
  UpdateData(TRUE);
  // JHF ditto
#if !defined(POCKET_PC)
  WORD wVirtualKeyCode, wModifiers;
  m_hotkey.GetHotKey(wVirtualKeyCode, wModifiers);
  DWORD v = wVirtualKeyCode | (wModifiers << 16);
  m_hotkey_value = v;
#endif
  CPropertyPage::OnOK();  
}
