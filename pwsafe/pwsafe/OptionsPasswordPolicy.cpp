// OptionsPasswordPolicy.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "resource.h"
#include "OptionsPasswordPolicy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy property page

IMPLEMENT_DYNCREATE(COptionsPasswordPolicy, CPropertyPage)

COptionsPasswordPolicy::COptionsPasswordPolicy() : CPropertyPage(COptionsPasswordPolicy::IDD)
{
	//{{AFX_DATA_INIT(COptionsPasswordPolicy)
	//}}AFX_DATA_INIT
}

COptionsPasswordPolicy::~COptionsPasswordPolicy()
{
}

void COptionsPasswordPolicy::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPasswordPolicy)
	DDX_Text(pDX, IDC_DEFPWLENGTH, m_pwlendefault);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPasswordPolicy, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPasswordPolicy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy message handlers

BOOL COptionsPasswordPolicy::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
   CSpinButtonCtrl*  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1);

   pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
   pspin->SetRange(4, 1024);
   pspin->SetBase(10);
   pspin->SetPos(m_pwlendefault);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
