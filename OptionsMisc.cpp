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
	DDX_Check(pDX, IDC_ESC_EXITS, m_escexits);
	DDX_Radio(pDX, IDC_DOUBLE_CLICK_COPIES, m_doubleclickaction); // only first!
	DDX_Check(pDX, IDC_HOTKEY_ENABLE, m_hotkey_enabled);
// JHF class CHotKeyCtrl not defined under WinCE
#if !defined(POCKET_PC)
	DDX_Control(pDX, IDC_HOTKEY_CTRL, m_hotkey);
#endif
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsMisc, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsMisc)
	ON_BN_CLICKED(IDC_HOTKEY_ENABLE, OnEnableHotKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsMisc::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  CButton *pBCopyRB = (CButton *)GetDlgItem(IDC_DOUBLE_CLICK_COPIES);
  CButton *pBEditRB = (CButton *)GetDlgItem(IDC_DOUBLE_CLICK_EDITS);
  CButton *pAutoTypeRB = (CButton *)GetDlgItem(IDC_DOUBLE_CLICK_AUTOTYPES);
  ASSERT(pBCopyRB != NULL && pBEditRB != NULL);

  switch (m_doubleclickaction) {
  case PWSprefs::DoubleClickCopy:
    pBCopyRB->SetCheck(1); pBEditRB->SetCheck(0); pAutoTypeRB->SetCheck(0);
    break;
  case PWSprefs::DoubleClickEdit:
    pBCopyRB->SetCheck(0); pBEditRB->SetCheck(1); pAutoTypeRB->SetCheck(0);
    break;
  case PWSprefs::DoubleClickAutoType:
    pBCopyRB->SetCheck(0); pBEditRB->SetCheck(0); pAutoTypeRB->SetCheck(1);
    break;
  default:
    ASSERT(0);
  }

  // JHF ditto here
#if !defined(POCKET_PC)
  m_hotkey.SetHotKey(LOWORD(m_hotkey_value),HIWORD(m_hotkey_value));
  if (m_hotkey_enabled == FALSE)
    m_hotkey.EnableWindow(FALSE);
#endif
  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// COptionsMisc message handlers

void COptionsMisc::OnEnableHotKey() 
{
	// JHF : no hotkeys on WinCE
#if !defined(POCKET_PC)
  if (((CButton*)GetDlgItem(IDC_HOTKEY_ENABLE))->GetCheck() == 1)
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(TRUE);
  else
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(FALSE);
#endif
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
