// OptionsPasswordPolicy.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif
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
	DDX_Check(pDX, IDC_USEDIGITS, m_pwusedigits);
	DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
	DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
	DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
	DDX_Check(pDX, IDC_EASYVISION, m_pweasyvision);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPasswordPolicy, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPasswordPolicy)
	ON_BN_CLICKED(IDC_USELOWERCASE, OnUselowercase)
	ON_BN_CLICKED(IDC_USEUPPERCASE, OnUseuppercase)
	ON_BN_CLICKED(IDC_USEDIGITS, OnUsedigits)
	ON_BN_CLICKED(IDC_USESYMBOLS, OnUsesymbols)
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

void COptionsPasswordPolicy::OnUselowercase() 
{
}

void COptionsPasswordPolicy::OnUseuppercase() 
{
}

void COptionsPasswordPolicy::OnUsedigits() 
{
}

void COptionsPasswordPolicy::OnUsesymbols() 
{
}

BOOL COptionsPasswordPolicy::OnKillActive()
{
   CPropertyPage::OnKillActive();

   // Check that options, as set, are valid.
   if (!m_pwuselowercase &&
       !m_pwuseuppercase &&
       !m_pwusedigits &&
       !m_pwusesymbols)
   {
      AfxMessageBox(_T("At least one type of character (lowercase, uppercase,\ndigits, or symbols) must be permitted."));
      return FALSE;
   }

   //End check

   return TRUE;
}
