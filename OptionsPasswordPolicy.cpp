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
  DDX_Check(pDX, IDC_USEHEXDIGITS, m_pwusehexdigits);
  DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
  DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
  DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
  DDX_Check(pDX, IDC_EASYVISION, m_pweasyvision);
  DDX_Check(pDX, IDC_SAVEPWHISTORY, m_savepwhistory);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPasswordPolicy, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPasswordPolicy)
	ON_BN_CLICKED(IDC_USELOWERCASE, OnUselowercase)
	ON_BN_CLICKED(IDC_USEUPPERCASE, OnUseuppercase)
	ON_BN_CLICKED(IDC_USEDIGITS, OnUsedigits)
	ON_BN_CLICKED(IDC_USESYMBOLS, OnUsesymbols)
	ON_BN_CLICKED(IDC_USEHEXDIGITS, OnUsehexdigits)
	ON_BN_CLICKED(IDC_EASYVISION, OnUseeasyvision)
	ON_BN_CLICKED(IDC_SAVEPWHISTORY, OnSavePWHistory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy message handlers

BOOL COptionsPasswordPolicy::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();
	
  CSpinButtonCtrl*  pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWLENSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
  pspin->SetRange(4, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwlendefault);

  if (IsDlgButtonChecked(IDC_USELOWERCASE) ||
      IsDlgButtonChecked(IDC_USEUPPERCASE) ||
      IsDlgButtonChecked(IDC_USEDIGITS) ||
      IsDlgButtonChecked(IDC_USESYMBOLS) ||
      IsDlgButtonChecked(IDC_EASYVISION) ) {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
		
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(TRUE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(TRUE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(TRUE);
  } else {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
	   
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(FALSE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsPasswordPolicy::OnUselowercase() 
{
  if (IsDlgButtonChecked(IDC_USELOWERCASE) ||
      IsDlgButtonChecked(IDC_USEUPPERCASE) ||
      IsDlgButtonChecked(IDC_USEDIGITS) ||
      IsDlgButtonChecked(IDC_USESYMBOLS) ||
      IsDlgButtonChecked(IDC_EASYVISION) ) {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
  }
}

void COptionsPasswordPolicy::OnUseuppercase() 
{
  if (IsDlgButtonChecked(IDC_USELOWERCASE) ||
      IsDlgButtonChecked(IDC_USEUPPERCASE) ||
      IsDlgButtonChecked(IDC_USEDIGITS) ||
      IsDlgButtonChecked(IDC_USESYMBOLS) ||
      IsDlgButtonChecked(IDC_EASYVISION) ) {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
  }
}

void COptionsPasswordPolicy::OnUsedigits() 
{
  if (IsDlgButtonChecked(IDC_USELOWERCASE) ||
      IsDlgButtonChecked(IDC_USEUPPERCASE) ||
      IsDlgButtonChecked(IDC_USEDIGITS) ||
      IsDlgButtonChecked(IDC_USESYMBOLS) ||
      IsDlgButtonChecked(IDC_EASYVISION) ) {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
  }
}

void COptionsPasswordPolicy::OnUsesymbols() 
{
  if (IsDlgButtonChecked(IDC_USELOWERCASE) ||
      IsDlgButtonChecked(IDC_USEUPPERCASE) ||
      IsDlgButtonChecked(IDC_USEDIGITS) ||
      IsDlgButtonChecked(IDC_USESYMBOLS) ||
      IsDlgButtonChecked(IDC_EASYVISION) ) {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
  }
}

void COptionsPasswordPolicy::OnUsehexdigits() 
{
  if ( IsDlgButtonChecked(IDC_USEHEXDIGITS) ) {
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(TRUE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(TRUE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(TRUE);
  }
}

void COptionsPasswordPolicy::OnUseeasyvision() 
{
  if (IsDlgButtonChecked(IDC_USELOWERCASE) ||
      IsDlgButtonChecked(IDC_USEUPPERCASE) ||
      IsDlgButtonChecked(IDC_USEDIGITS) ||
      IsDlgButtonChecked(IDC_USESYMBOLS) ||
      IsDlgButtonChecked(IDC_EASYVISION) ) { 
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
  }
}

BOOL COptionsPasswordPolicy::OnKillActive()
{
  CPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if (m_pwusehexdigits &&
      (m_pwuselowercase || m_pwuseuppercase || m_pwusedigits || m_pwusesymbols)) {
    AfxMessageBox(_T("Hexadecimal is mutually exclusive to all other options!"));
    return FALSE;
  }


  if (m_pwusehexdigits) {
    if (m_pwlendefault % 2 != 0) {
      AfxMessageBox(_T("Passwords generated in hexadecimal format must have even lengths\nas two hexadecimal characters make up a single ASCII character."));
      return FALSE;
    }
  } else if (!m_pwuselowercase && !m_pwuseuppercase &&
             !m_pwusedigits && !m_pwusesymbols) {
    AfxMessageBox(_T("At least one type of character (lowercase, uppercase, digits,\nsymbols, hexadecimal) must be permitted."));
    return FALSE;
  }
  //End check

  return TRUE;
}

void COptionsPasswordPolicy::OnSavePWHistory() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_SAVEPWHISTORY))->GetCheck() == 1) ? TRUE : FALSE;

  GetDlgItem(IDC_PWHSPIN)->EnableWindow(enable);
}