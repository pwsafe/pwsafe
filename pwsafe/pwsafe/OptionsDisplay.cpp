// OptionsDisplay.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif
#include "OptionsDisplay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay property page

IMPLEMENT_DYNCREATE(COptionsDisplay, CPropertyPage)

COptionsDisplay::COptionsDisplay() : CPropertyPage(COptionsDisplay::IDD)
{
	//{{AFX_DATA_INIT(COptionsDisplay)
	//}}AFX_DATA_INIT
}

COptionsDisplay::~COptionsDisplay()
{
}

void COptionsDisplay::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDisplay)
	DDX_Check(pDX, IDC_ALWAYSONTOP, m_alwaysontop);
	DDX_Check(pDX, IDC_DEFPWSHOWINLIST, m_pwshowinlist);
	DDX_Check(pDX, IDC_DEFPWSHOWINEDIT, m_pwshowinedit);
#if defined(POCKET_PC)
	DDX_Check(pDX, IDC_DCSHOWSPASSWORD, m_dcshowspassword);
#endif
	DDX_Text(pDX, IDC_MAXREITEMS, m_maxreitems);
	DDV_MinMaxInt(pDX, m_maxreitems, 1, ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1);
	DDX_Check(pDX, IDC_DEFPWUSESYSTRAY, m_usesystemtray);
	DDX_Text(pDX, IDC_MAXMRUITEMS, m_maxmruitems);
	DDV_MinMaxInt(pDX, m_maxmruitems, 1, ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1);
	DDX_Check(pDX, IDC_MRU_ONFILEMENU, m_mruonfilemenu);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDisplay, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsDisplay)
	ON_BN_CLICKED(IDC_DEFPWUSESYSTRAY, OnUseSystemTray)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay message handlers

void COptionsDisplay::OnUseSystemTray() 
{
   if (((CButton*)GetDlgItem(IDC_DEFPWUSESYSTRAY))->GetCheck() == 1)
   {
      GetDlgItem(IDC_STATIC_MAXREITEMS)->EnableWindow(TRUE);
      GetDlgItem(IDC_MAXREITEMS)->EnableWindow(TRUE);
   }
   else
   {
      GetDlgItem(IDC_STATIC_MAXREITEMS)->EnableWindow(FALSE);
      GetDlgItem(IDC_MAXREITEMS)->EnableWindow(FALSE);
   }
}

BOOL COptionsDisplay::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	OnUseSystemTray();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
