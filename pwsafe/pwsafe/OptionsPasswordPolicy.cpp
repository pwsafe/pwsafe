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
	DDX_Check(pDX, IDC_USEDIGITS, m_pwusedigits);
	DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
	DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
	DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
	DDX_Text(pDX, IDC_MINDIGITS, m_pwmindigits);
	DDX_Text(pDX, IDC_MINSYMBOLS, m_pwminsymbols);
	DDX_Text(pDX, IDC_MINUPPERCASE, m_pwminuppercase);
	DDX_Text(pDX, IDC_MINLOWERCASE, m_pwminlowercase);
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

   pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINLOWERCASE);
   pspin->SetBuddy(GetDlgItem(IDC_MINLOWERCASE));
   pspin->SetRange(0, 1024);
   pspin->SetBase(10);
   pspin->SetPos(m_pwminlowercase);

   pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINUPPERCASE);
   pspin->SetBuddy(GetDlgItem(IDC_MINUPPERCASE));
   pspin->SetRange(0, 1024);
   pspin->SetBase(10);
   pspin->SetPos(m_pwminuppercase);

   pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINDIGITS);
   pspin->SetBuddy(GetDlgItem(IDC_MINDIGITS));
   pspin->SetRange(0, 1024);
   pspin->SetBase(10);
   pspin->SetPos(m_pwmindigits);

   pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINSYMBOLS);
   pspin->SetBuddy(GetDlgItem(IDC_MINSYMBOLS));
   pspin->SetRange(0, 1024);
   pspin->SetBase(10);
   pspin->SetPos(m_pwminsymbols);

   OnUselowercase();
	OnUseuppercase();
   OnUsedigits();
   OnUsesymbols();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsPasswordPolicy::OnUselowercase() 
{
   if (((CButton*)GetDlgItem(IDC_USELOWERCASE))->GetCheck() == 1)
   {
      GetDlgItem(IDC_STATIC_MINLOWERCASE)->EnableWindow(TRUE);
      GetDlgItem(IDC_MINLOWERCASE)->EnableWindow(TRUE);
      GetDlgItem(IDC_SPINLOWERCASE)->EnableWindow(TRUE);
   }
   else
   {
      GetDlgItem(IDC_STATIC_MINLOWERCASE)->EnableWindow(FALSE);
      GetDlgItem(IDC_MINLOWERCASE)->EnableWindow(FALSE);
      GetDlgItem(IDC_SPINLOWERCASE)->EnableWindow(FALSE);
   }
}

void COptionsPasswordPolicy::OnUseuppercase() 
{
   if (((CButton*)GetDlgItem(IDC_USEUPPERCASE))->GetCheck() == 1)
   {
      GetDlgItem(IDC_STATIC_MINUPPERCASE)->EnableWindow(TRUE);
      GetDlgItem(IDC_MINUPPERCASE)->EnableWindow(TRUE);
      GetDlgItem(IDC_SPINUPPERCASE)->EnableWindow(TRUE);
   }
   else
   {
      GetDlgItem(IDC_STATIC_MINUPPERCASE)->EnableWindow(FALSE);
      GetDlgItem(IDC_MINUPPERCASE)->EnableWindow(FALSE);
      GetDlgItem(IDC_SPINUPPERCASE)->EnableWindow(FALSE);
   }
}

void COptionsPasswordPolicy::OnUsedigits() 
{
   if (((CButton*)GetDlgItem(IDC_USEDIGITS))->GetCheck() == 1)
   {
      GetDlgItem(IDC_STATIC_MINDIGITS)->EnableWindow(TRUE);
      GetDlgItem(IDC_MINDIGITS)->EnableWindow(TRUE);
      GetDlgItem(IDC_SPINDIGITS)->EnableWindow(TRUE);
   }
   else
   {
      GetDlgItem(IDC_STATIC_MINDIGITS)->EnableWindow(FALSE);
      GetDlgItem(IDC_MINDIGITS)->EnableWindow(FALSE);
      GetDlgItem(IDC_SPINDIGITS)->EnableWindow(FALSE);
   }
}

void COptionsPasswordPolicy::OnUsesymbols() 
{
   if (((CButton*)GetDlgItem(IDC_USESYMBOLS))->GetCheck() == 1)
   {
      GetDlgItem(IDC_STATIC_MINSYMBOLS)->EnableWindow(TRUE);
      GetDlgItem(IDC_MINSYMBOLS)->EnableWindow(TRUE);
      GetDlgItem(IDC_SPINSYMBOLS)->EnableWindow(TRUE);
   }
   else
   {
      GetDlgItem(IDC_STATIC_MINSYMBOLS)->EnableWindow(FALSE);
      GetDlgItem(IDC_MINSYMBOLS)->EnableWindow(FALSE);
      GetDlgItem(IDC_SPINSYMBOLS)->EnableWindow(FALSE);
   }
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
      AfxMessageBox("At least one type of character (lowercase, uppercase,\ndigits, or symbols) must be permitted.");
      return FALSE;
   }

   UINT totalmins =
      (m_pwuselowercase?m_pwminlowercase:0) +
      (m_pwuseuppercase?m_pwminuppercase:0) +
      (m_pwusedigits?m_pwmindigits:0) +
      (m_pwusesymbols?m_pwminsymbols:0);

   if (totalmins > m_pwlendefault)
   {
      char buf[256];
      sprintf(buf, "Total minimum number of characters (%d)\ncannot exceed default password length (%d).", 
              totalmins, m_pwlendefault);
      AfxMessageBox(buf);
      return FALSE;
   }
   //End check

   return TRUE;
}
