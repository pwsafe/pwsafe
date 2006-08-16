// OptionsPasswordHistory.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif
#include "OptionsPasswordHistory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory property page

IMPLEMENT_DYNCREATE(COptionsPasswordHistory, CPropertyPage)

COptionsPasswordHistory::COptionsPasswordHistory() : CPropertyPage(COptionsPasswordHistory::IDD)
{
  //{{AFX_DATA_INIT(COptionsPasswordHistory)
  //}}AFX_DATA_INIT
}

COptionsPasswordHistory::~COptionsPasswordHistory()
{
}

void COptionsPasswordHistory::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(COptionsPasswordHistory)
  DDX_Check(pDX, IDC_SAVEPWHISTORY, m_savepwhistory);
  DDX_Text(pDX, IDC_DEFPWHNUM, m_pwhistorynumdefault);
  DDX_Check(pDX, IDC_APPLYPWHISTORY, m_applypwhistory);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPasswordHistory, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPasswordHistory)
	ON_BN_CLICKED(IDC_SAVEPWHISTORY, OnSavePWHistory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory message handlers

BOOL COptionsPasswordHistory::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();
	
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWHNUM));
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos(m_pwhistorynumdefault);

  m_applypwhistory = FALSE;

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL COptionsPasswordHistory::OnKillActive()
{
  CPropertyPage::OnKillActive();

  // Check that options, as set, are valid.
  if (m_savepwhistory && ((m_pwhistorynumdefault < 1) || (m_pwhistorynumdefault > 255))) {
  	AfxMessageBox(_T("Default number of saved password history entries must be between 1 and 255."));
  	((CEdit*)GetDlgItem(IDC_DEFPWHNUM))->SetFocus();
  	return FALSE;
  }

  //End check

  return TRUE;
}

void COptionsPasswordHistory::OnSavePWHistory() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_SAVEPWHISTORY))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_PWHSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_DEFPWHNUM)->EnableWindow(enable);
  GetDlgItem(IDC_APPLYPWHISTORY)->EnableWindow(enable);
}
