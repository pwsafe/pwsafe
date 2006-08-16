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
#include "corelib\pwsprefs.h"

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
	DDX_Radio(pDX, IDC_TREE_DISPLAY_COLLAPSED, m_treedisplaystatusatopen); // only first!	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDisplay, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsDisplay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay message handlers

BOOL COptionsDisplay::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	CButton *pBCollapsed = (CButton *)GetDlgItem(IDC_TREE_DISPLAY_COLLAPSED);
	CButton *pBExpanded = (CButton *)GetDlgItem(IDC_TREE_DISPLAY_EXPANDED);
	CButton *pBAsPerLastSave = (CButton *)GetDlgItem(IDC_TREE_DISPLAY_LASTSAVE);

	switch (m_treedisplaystatusatopen) {
		case PWSprefs::AllCollapsed:
			pBCollapsed->SetCheck(1);
			pBExpanded->SetCheck(0);
			pBAsPerLastSave->SetCheck(0);
			break;
		case PWSprefs::AllExpanded:
			pBCollapsed->SetCheck(0);
			pBExpanded->SetCheck(1);
			pBAsPerLastSave->SetCheck(0);
			break;
	case PWSprefs::AsPerLastSave:
			pBCollapsed->SetCheck(0);
			pBExpanded->SetCheck(0);
			pBAsPerLastSave->SetCheck(1);
			break;
	default:
		ASSERT(0);
	}

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}
