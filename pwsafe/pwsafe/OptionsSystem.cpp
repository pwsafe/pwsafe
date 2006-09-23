// OptionsSystem.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"


#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
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

COptionsSystem::COptionsSystem() : CPropertyPage(COptionsSystem::IDD)
{
	//{{AFX_DATA_INIT(COptionsSystem)
	//}}AFX_DATA_INIT
	m_ToolTipCtrl = NULL;
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
	DDX_Text(pDX, IDC_MAXMRUITEMS, m_maxmruitems);
	DDV_MinMaxInt(pDX, m_maxmruitems, 0, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
	DDX_Check(pDX, IDC_MRU_ONFILEMENU, m_mruonfilemenu);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsSystem, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsSystem)
	ON_BN_CLICKED(IDC_DEFPWUSESYSTRAY, OnUseSystemTray)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSystem message handlers

void COptionsSystem::OnUseSystemTray() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_DEFPWUSESYSTRAY))->GetCheck() == 1) ? TRUE : FALSE;

  GetDlgItem(IDC_STATIC_MAXREITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_MAXREITEMS)->EnableWindow(enable);
  GetDlgItem(IDC_RESPIN)->EnableWindow(enable);
}

BOOL COptionsSystem::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

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
	
	OnUseSystemTray();

	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL COptionsSystem::OnKillActive()
{
  // Needed as we have DDV routines.

  return CPropertyPage::OnKillActive();
}
