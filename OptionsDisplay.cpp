// OptionsDisplay.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"

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
	DDX_Check(pDX, IDC_DEFPWUSESYSTRAY, m_usesystemtray);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDisplay, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsDisplay)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay message handlers
