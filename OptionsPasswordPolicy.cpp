/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
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

const int COptionsPasswordPolicy::nonHex[COptionsPasswordPolicy::N_NOHEX] = {
  IDC_USELOWERCASE, IDC_USEUPPERCASE, IDC_USEDIGITS,
  IDC_USESYMBOLS, IDC_EASYVISION, IDC_PRONOUNCEABLE};

COptionsPasswordPolicy::COptionsPasswordPolicy() :
  CPWPropertyPage(COptionsPasswordPolicy::IDD)
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
   DDX_Text(pDX, IDC_DEFPWLENGTH, m_pwdefaultlength);
   DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
   DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
   DDX_Check(pDX, IDC_USEDIGITS, m_pwusedigits);
   DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
   DDX_Check(pDX, IDC_EASYVISION, m_pweasyvision);
   DDX_Check(pDX, IDC_USEHEXDIGITS, m_pwusehexdigits);
   DDX_Check(pDX, IDC_PRONOUNCEABLE, m_pwmakepronounceable);
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

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWLENSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
  pspin->SetRange(4, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwdefaultlength);

  m_save[0] = m_pwuselowercase; m_save[1] = m_pwuseuppercase;
  m_save[2] = m_pwusedigits; m_save[3] = m_pwusesymbols;
  m_save[4] = m_pweasyvision; m_save[5] = m_pwmakepronounceable;

  do_nohex(m_pwusehexdigits == FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsPasswordPolicy::do_nohex(bool mode)
{
  int i;
  if (mode) { // true means enable non-hex, restore state
    for (i = 0; i < N_NOHEX; i++) {
      int id = nonHex[i];
      GetDlgItem(id)->EnableWindow(TRUE);
      ((CButton*)GetDlgItem(id))->SetCheck(m_save[i]);
    }
  } else { // disable, save state for possible re-enable
    for (i = 0; i < N_NOHEX; i++) {
      int id = nonHex[i];
      GetDlgItem(id)->EnableWindow(FALSE);
      m_save[i] = ((CButton*)GetDlgItem(id))->GetCheck();
      ((CButton*)GetDlgItem(id))->SetCheck(0);
    }
  }
}

void COptionsPasswordPolicy::OnUsehexdigits() 
{
  do_nohex(IsDlgButtonChecked(IDC_USEHEXDIGITS) == 0);
}

BOOL COptionsPasswordPolicy::OnKillActive()
{
  CPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if (m_pwusehexdigits &&
      (m_pwuselowercase || m_pwuseuppercase || m_pwusedigits ||
       m_pwusesymbols || m_pwmakepronounceable)) {
    AfxMessageBox(IDS_HEXMUTUALLYEXCL);
    return FALSE;
  }

  if (m_pwmakepronounceable && m_pweasyvision) {
    AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    return FALSE;
  }

  if (m_pwusehexdigits) {
    if (m_pwdefaultlength % 2 != 0) {
      AfxMessageBox(IDS_HEXMUSTBEEVEN);
      return FALSE;
    }
  } else if (!m_pwuselowercase && !m_pwuseuppercase &&
             !m_pwusedigits && !m_pwusesymbols) {
    AfxMessageBox(IDS_MUSTHAVEONEOPTION);
    return FALSE;
  }
  
  if ((m_pwdefaultlength < 4) || (m_pwdefaultlength > 1024)) {
  	AfxMessageBox(IDS_DEFAULTPWLENGTH);
  	((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
  	return FALSE;
  }
  //End check

  return TRUE;
}
