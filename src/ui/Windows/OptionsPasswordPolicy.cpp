/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

IMPLEMENT_DYNCREATE(COptionsPasswordPolicy, CPWPropertyPage)

const UINT COptionsPasswordPolicy::nonHex[COptionsPasswordPolicy::N_NOHEX] = {
  IDC_USELOWERCASE, IDC_USEUPPERCASE, IDC_USEDIGITS,
  IDC_USESYMBOLS, IDC_EASYVISION, IDC_PRONOUNCEABLE};

const UINT COptionsPasswordPolicy::LenTxts[COptionsPasswordPolicy::N_HEX_LENGTHS * 2] = {
  IDC_STATIC_LC1, IDC_STATIC_LC2, IDC_STATIC_UC1, IDC_STATIC_UC2,
  IDC_STATIC_DG1, IDC_STATIC_DG2, IDC_STATIC_SY1, IDC_STATIC_SY2};

const UINT COptionsPasswordPolicy::nonHexLengths[COptionsPasswordPolicy::N_HEX_LENGTHS] = {
 IDC_MINLOWERLENGTH, IDC_MINUPPERLENGTH, IDC_MINDIGITLENGTH, IDC_MINSYMBOLLENGTH};

const UINT COptionsPasswordPolicy::nonHexLengthSpins[COptionsPasswordPolicy::N_HEX_LENGTHS] = {
  IDC_SPINLOWERCASE, IDC_SPINUPPERCASE, IDC_SPINDIGITS, IDC_SPINSYMBOLS};

COptionsPasswordPolicy::COptionsPasswordPolicy()
  : CPWPropertyPage(COptionsPasswordPolicy::IDD)
{
  //{{AFX_DATA_INIT(COptionsPasswordPolicy)
  //}}AFX_DATA_INIT
}

COptionsPasswordPolicy::~COptionsPasswordPolicy()
{
}

void COptionsPasswordPolicy::DoDataExchange(CDataExchange* pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsPasswordPolicy)
  DDX_Text(pDX, IDC_DEFPWLENGTH, m_pwdefaultlength);
  DDX_Text(pDX, IDC_MINDIGITLENGTH, m_pwdigitminlength);
  DDX_Text(pDX, IDC_MINLOWERLENGTH, m_pwlowerminlength);
  DDX_Text(pDX, IDC_MINSYMBOLLENGTH, m_pwsymbolminlength);
  DDX_Text(pDX, IDC_MINUPPERLENGTH, m_pwupperminlength);
  DDX_Check(pDX, IDC_USELOWERCASE, m_pwuselowercase);
  DDX_Check(pDX, IDC_USEUPPERCASE, m_pwuseuppercase);
  DDX_Check(pDX, IDC_USEDIGITS, m_pwusedigits);
  DDX_Check(pDX, IDC_USESYMBOLS, m_pwusesymbols);
  DDX_Check(pDX, IDC_EASYVISION, m_pweasyvision);
  DDX_Check(pDX, IDC_USEHEXDIGITS, m_pwusehexdigits);
  DDX_Check(pDX, IDC_PRONOUNCEABLE, m_pwmakepronounceable);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsPasswordPolicy, CPWPropertyPage)
  //{{AFX_MSG_MAP(COptionsPasswordPolicy)
  ON_BN_CLICKED(IDC_USEHEXDIGITS, OnUsehexdigits)
  ON_BN_CLICKED(IDC_USELOWERCASE, OnUselowercase)
  ON_BN_CLICKED(IDC_USEUPPERCASE, OnUseuppercase)
  ON_BN_CLICKED(IDC_USEDIGITS, OnUsedigits)
  ON_BN_CLICKED(IDC_USESYMBOLS, OnUsesymbols)
  ON_BN_CLICKED(IDC_EASYVISION, OnEasyVision)
  ON_BN_CLICKED(IDC_PRONOUNCEABLE, OnMakePronounceable)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordPolicy message handlers

BOOL COptionsPasswordPolicy::OnInitDialog() 
{
  CPWPropertyPage::OnInitDialog();

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWLENSPIN);
  CSpinButtonCtrl* pspinD = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINDIGITS);
  CSpinButtonCtrl* pspinL = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINLOWERCASE);
  CSpinButtonCtrl* pspinS = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINSYMBOLS);
  CSpinButtonCtrl* pspinU = (CSpinButtonCtrl *)GetDlgItem(IDC_SPINUPPERCASE);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWLENGTH));
  pspin->SetRange(4, 1024);
  pspin->SetBase(10);
  pspin->SetPos(m_pwdefaultlength);

  pspinD->SetBuddy(GetDlgItem(IDC_MINDIGITLENGTH));
  pspinD->SetRange(0, 1024);
  pspinD->SetBase(10);
  pspinD->SetPos(m_pwdigitminlength);

  pspinL->SetBuddy(GetDlgItem(IDC_MINLOWERLENGTH));
  pspinL->SetRange(0, 1024);
  pspinL->SetBase(10);
  pspinL->SetPos(m_pwlowerminlength);

  pspinS->SetBuddy(GetDlgItem(IDC_MINSYMBOLLENGTH));
  pspinS->SetRange(0, 1024);
  pspinS->SetBase(10);
  pspinS->SetPos(m_pwsymbolminlength);

  pspinU->SetBuddy(GetDlgItem(IDC_MINUPPERLENGTH));
  pspinU->SetRange(0, 1024);
  pspinU->SetBase(10);
  pspinU->SetPos(m_pwupperminlength);

  m_save[0] = m_pwuselowercase; m_save[1] = m_pwuseuppercase;
  m_save[2] = m_pwusedigits;    m_save[3] = m_pwusesymbols;
  m_save[4] = m_pweasyvision;   m_save[5] = m_pwmakepronounceable;

  if (m_pwuselowercase == TRUE && m_pwlowerminlength == 0)
    m_pwlowerminlength = 1;
  if (m_pwuseuppercase == TRUE && m_pwupperminlength == 0)
    m_pwupperminlength = 1;
  if (m_pwusedigits == TRUE && m_pwdigitminlength == 0)
    m_pwdigitminlength = 1;
  if (m_pwusesymbols == TRUE && m_pwsymbolminlength == 0)
    m_pwsymbolminlength = 1;

  m_savelen[0] = m_pwlowerminlength; m_savelen[1] = m_pwupperminlength;
  m_savelen[2] = m_pwdigitminlength; m_savelen[3] = m_pwsymbolminlength;

  do_nohex(m_pwusehexdigits == FALSE);
  do_easyorpronounceable(m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsPasswordPolicy::do_nohex(const bool bNonHex)
{
  CString cs_value;
  int i;
  if (bNonHex) { // true means enable non-hex, restore state
    for (i = 0; i < N_NOHEX; i++) {
      UINT id = nonHex[i];
      GetDlgItem(id)->EnableWindow(TRUE);
      ((CButton*)GetDlgItem(id))->SetCheck(m_save[i]);
    }
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      cs_value.Format(L"%d", m_savelen[i]);
      GetDlgItem(id)->SetWindowText(cs_value);
      GetDlgItem(id)->EnableWindow(m_save[i]);
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i*2])->EnableWindow(m_save[i]);
      GetDlgItem(LenTxts[i*2 + 1])->EnableWindow(m_save[i]);
    }
    m_pwlowerminlength = m_savelen[0]; m_pwupperminlength = m_savelen[1];
    m_pwdigitminlength = m_savelen[2]; m_pwsymbolminlength = m_savelen[3];

  } else { // hex, save state for possible re-enable
    for (i = 0; i < N_NOHEX; i++) {
      UINT id = nonHex[i];
      m_save[i] = ((CButton*)GetDlgItem(id))->GetCheck();
      ((CButton*)GetDlgItem(id))->SetCheck(BST_UNCHECKED);
      GetDlgItem(id)->EnableWindow(FALSE);
    }
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      UINT id = nonHexLengths[i];
      GetDlgItem(id)->EnableWindow(FALSE);
      GetDlgItem(id)->SetWindowText(L"0");
      GetDlgItem(nonHexLengthSpins[i])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i*2])->EnableWindow(FALSE);
      GetDlgItem(LenTxts[i*2 + 1])->EnableWindow(FALSE);
    }
    m_savelen[0] = m_pwlowerminlength; m_savelen[1] = m_pwupperminlength;
    m_savelen[2] = m_pwdigitminlength; m_savelen[3] = m_pwsymbolminlength;
  }
}

void COptionsPasswordPolicy::do_easyorpronounceable(const bool bSet)
{
  // Can't have minimum lengths!
  if ((m_pweasyvision == TRUE || m_pwmakepronounceable == TRUE) &&
    (m_pwdigitminlength > 1  || m_pwlowerminlength > 1 || 
    m_pwsymbolminlength > 1 || m_pwupperminlength > 1))
    AfxMessageBox(IDS_CANTSPECIFYMINNUMBER);

  CString cs_value;
  int i;
  if (bSet) {
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->ShowWindow(SW_HIDE);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[2*i])->ShowWindow(SW_HIDE);
      GetDlgItem(LenTxts[2*i + 1])->ShowWindow(SW_HIDE);
    }
    m_savelen[0] = m_pwlowerminlength; m_savelen[1] = m_pwupperminlength;
    m_savelen[2] = m_pwdigitminlength; m_savelen[3] = m_pwsymbolminlength;
  } else {
    for (i = 0; i < N_HEX_LENGTHS; i++) {
      GetDlgItem(nonHexLengths[i])->ShowWindow(SW_SHOW);
      GetDlgItem(nonHexLengthSpins[i])->ShowWindow(SW_SHOW);
      GetDlgItem(LenTxts[2*i])->ShowWindow(SW_SHOW);
      GetDlgItem(LenTxts[2*i + 1])->ShowWindow(SW_SHOW);
    }
    m_pwlowerminlength = m_savelen[0]; m_pwupperminlength = m_savelen[1];
    m_pwdigitminlength = m_savelen[2]; m_pwsymbolminlength = m_savelen[3];
  }
}

void COptionsPasswordPolicy::OnUselowercase() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USELOWERCASE) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINLOWERLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINLOWERCASE)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_LC1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_LC2)->EnableWindow(bChecked);
  m_pwlowerminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUseuppercase() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USEUPPERCASE) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINUPPERLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINUPPERCASE)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_UC1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_UC2)->EnableWindow(bChecked);
  m_pwupperminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUsedigits() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USEDIGITS) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINDIGITLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINDIGITS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DG1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_DG2)->EnableWindow(bChecked);
  m_pwdigitminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUsesymbols() 
{
  UpdateData(TRUE);
  BOOL bChecked = (IsDlgButtonChecked(IDC_USESYMBOLS) == BST_CHECKED) ? TRUE : FALSE;

  GetDlgItem(IDC_MINSYMBOLLENGTH)->EnableWindow(bChecked);
  GetDlgItem(IDC_SPINSYMBOLS)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_SY1)->EnableWindow(bChecked);
  GetDlgItem(IDC_STATIC_SY2)->EnableWindow(bChecked);
  m_pwsymbolminlength = bChecked;  // Based on FALSE=0 & TRUE=1
  UpdateData(FALSE);
}

void COptionsPasswordPolicy::OnUsehexdigits() 
{
  UpdateData(TRUE);
  do_nohex(IsDlgButtonChecked(IDC_USEHEXDIGITS) == BST_UNCHECKED);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_hex" will be undone
}

void COptionsPasswordPolicy::OnEasyVision() 
{
  UpdateData(TRUE);
  if (m_pweasyvision && m_pwmakepronounceable) {
    ((CButton*)GetDlgItem(IDC_EASYVISION))->SetCheck(FALSE);
    AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_pweasyvision = FALSE;
  }

  const bool bChecked = (IsDlgButtonChecked(IDC_EASYVISION) == BST_CHECKED);

  do_easyorpronounceable(bChecked);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

void COptionsPasswordPolicy::OnMakePronounceable() 
{
  UpdateData(TRUE);
  if (m_pweasyvision && m_pwmakepronounceable) {
    ((CButton*)GetDlgItem(IDC_PRONOUNCEABLE))->SetCheck(FALSE);
    AfxMessageBox(IDS_PROVISMUTUALLYEXCL);
    m_pwmakepronounceable = FALSE;
  }

  const bool bChecked = (IsDlgButtonChecked(IDC_PRONOUNCEABLE) == BST_CHECKED);

  do_easyorpronounceable(bChecked);
  // Do not use UpdateData(FALSE) here or
  // all the good work in "do_easyorpronounceable" will be undone
}

BOOL COptionsPasswordPolicy::OnKillActive()
{
  CPWPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if (m_pwusehexdigits &&
     (m_pwuselowercase || m_pwuseuppercase || m_pwusedigits ||
      m_pwusesymbols || m_pweasyvision || m_pwmakepronounceable)) {
    AfxMessageBox(IDS_HEXMUTUALLYEXCL);
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

  if (!(m_pwusehexdigits || m_pweasyvision || m_pwmakepronounceable) &&
      (m_pwdigitminlength + m_pwlowerminlength + 
       m_pwsymbolminlength + m_pwupperminlength) > m_pwdefaultlength) {
    AfxMessageBox(IDS_DEFAULTPWLENGTHTOOSMALL);
    ((CEdit*)GetDlgItem(IDC_DEFPWLENGTH))->SetFocus();
    return FALSE;
  }

  if ((m_pwusehexdigits || m_pweasyvision || m_pwmakepronounceable))
    m_pwdigitminlength = m_pwlowerminlength = 
       m_pwsymbolminlength = m_pwupperminlength = 1;
  //End check

  return TRUE;
}
