/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// OptionsPasswordPolicy.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
  #include "resource3.h"  // String resources
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
  DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
  DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
  DDX_Check(pDX, IDC_USEDIGITS, m_pwusedigits);
  DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
  DDX_Check(pDX, IDC_EASYVISION, m_pweasyvision);
  DDX_Check(pDX, IDC_USEHEXDIGITS, m_pwusehexdigits);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPasswordPolicy, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPasswordPolicy)
	ON_BN_CLICKED(IDC_USEHEXDIGITS, OnUsehexdigits)
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

  if (IsDlgButtonChecked(IDC_USEHEXDIGITS)) {
    GetDlgItem(IDC_USEHEXDIGITS)->EnableWindow(TRUE);
	   
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(FALSE);

    ((CButton*)GetDlgItem(IDC_USELOWERCASE))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_USEUPPERCASE))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_USEDIGITS))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_USESYMBOLS))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(0);
  }

  m_savepwuselowercase = m_savepwuseuppercase = m_savepwusedigits = 
	  m_savepwusesymbols = m_savepweasyvision = 0;

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsPasswordPolicy::OnUsehexdigits() 
{
  if ( IsDlgButtonChecked(IDC_USEHEXDIGITS) ) {
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(FALSE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(FALSE);

    m_savepwuselowercase = ((CButton*)GetDlgItem(IDC_USELOWERCASE))->GetCheck();
    m_savepwuseuppercase = ((CButton*)GetDlgItem(IDC_USEUPPERCASE))->GetCheck();
    m_savepwusedigits = ((CButton*)GetDlgItem(IDC_USEDIGITS))->GetCheck();
    m_savepwusesymbols = ((CButton*)GetDlgItem(IDC_USESYMBOLS))->GetCheck();
    m_savepweasyvision = ((CButton*)GetDlgItem(IDC_EASYVISION))->GetCheck();

    ((CButton*)GetDlgItem(IDC_USELOWERCASE))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_USEUPPERCASE))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_USEDIGITS))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_USESYMBOLS))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(0);
  } else {
    GetDlgItem(IDC_USELOWERCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_USEUPPERCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_USEDIGITS)->EnableWindow(TRUE);
    GetDlgItem(IDC_USESYMBOLS)->EnableWindow(TRUE);
    GetDlgItem(IDC_EASYVISION)->EnableWindow(TRUE);

    ((CButton*)GetDlgItem(IDC_USELOWERCASE))->SetCheck(m_savepwuselowercase);
    ((CButton*)GetDlgItem(IDC_USEUPPERCASE))->SetCheck(m_savepwuseuppercase);
    ((CButton*)GetDlgItem(IDC_USEDIGITS))->SetCheck(m_savepwusedigits);
    ((CButton*)GetDlgItem(IDC_USESYMBOLS))->SetCheck(m_savepwusesymbols);
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(m_savepweasyvision);
  }
}

BOOL COptionsPasswordPolicy::OnKillActive()
{
  CPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if (m_pwusehexdigits &&
      (m_pwuselowercase || m_pwuseuppercase || m_pwusedigits || m_pwusesymbols)) {
    AfxMessageBox(IDS_HEXMUTUALLYEXCL);
    return FALSE;
  }

  if (m_pwusehexdigits) {
    if (m_pwlendefault % 2 != 0) {
      AfxMessageBox(IDS_HEXMUSTBEEVEN);
      return FALSE;
    }
  } else if (!m_pwuselowercase && !m_pwuseuppercase &&
             !m_pwusedigits && !m_pwusesymbols) {
    AfxMessageBox(IDS_MUSTHAVEONEOPTION);
    return FALSE;
  }
  
  if ((m_pwlendefault < 4) || (m_pwlendefault > 1024)) {
  	AfxMessageBox(IDS_DEFAULTPWLENGTH);
  	((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
  	return FALSE;
  }
  //End check

  return TRUE;
}
