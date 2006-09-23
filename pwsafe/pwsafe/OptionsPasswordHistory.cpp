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
#include "DboxMain.h"  // needed for DboxMain::UpdatePasswordHistory

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
  m_resetpwhistoryoff = m_resetpwhistoryon = m_setmaxpwhistory = FALSE;
  //}}AFX_DATA_INIT
  m_ToolTipCtrl = NULL;
}

COptionsPasswordHistory::~COptionsPasswordHistory()
{
	delete m_ToolTipCtrl;
}

void COptionsPasswordHistory::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(COptionsPasswordHistory)
  DDX_Check(pDX, IDC_SAVEPWHISTORY, m_savepwhistory);
  DDX_Text(pDX, IDC_DEFPWHNUM, m_pwhistorynumdefault);
  DDX_Check(pDX, IDC_RESETPWHISTORYOFF, m_resetpwhistoryoff);
  DDX_Check(pDX, IDC_RESETPWHISTORYON, m_resetpwhistoryon);
  DDX_Check(pDX, IDC_SETMAXPWHISTORY, m_setmaxpwhistory);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsPasswordHistory, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPasswordHistory)
	ON_BN_CLICKED(IDC_SAVEPWHISTORY, OnSavePWHistory)
	ON_BN_CLICKED(IDC_RESETPWHISTORYOFF, OnResetPWHistoryOff)
	ON_BN_CLICKED(IDC_RESETPWHISTORYON, OnResetPWHistoryOn)
	ON_BN_CLICKED(IDC_SETMAXPWHISTORY, OnSetMaxPWHistory)
	ON_BN_CLICKED(IDC_APPLYPWHCHANGESNOW, &COptionsPasswordHistory::OnBnClickedApplyPWHChanges)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPasswordHistory message handlers

BOOL COptionsPasswordHistory::OnInitDialog() 
{
  BOOL bResult = CPropertyPage::OnInitDialog();
	
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PWHSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_DEFPWHNUM));
  pspin->SetRange(1, 255);
  pspin->SetBase(10);
  pspin->SetPos(m_pwhistorynumdefault);

  // Tooltips on Property Pages
  EnableToolTips();

  m_ToolTipCtrl = new CToolTipCtrl;
  if (!m_ToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
    TRACE("Unable To create Property Page ToolTip\n");
    return bResult;
  }

  // Activate the tooltip control.
  m_ToolTipCtrl->Activate(TRUE);
  m_ToolTipCtrl->SetMaxTipWidth(300);
  // Quadruple the time to allow reading by user - there is a lot there!
  int iTime = m_ToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_ToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

  // Set the tooltip
  // Note naming convention: string IDS_xxx corresponds to control IDC_xxx
  CString cs_ToolTip;
  cs_ToolTip.LoadString(IDS_RESETPWHISTORYOFF);
  m_ToolTipCtrl->AddTool(GetDlgItem(IDC_RESETPWHISTORYOFF), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_RESETPWHISTORYON);
  m_ToolTipCtrl->AddTool(GetDlgItem(IDC_RESETPWHISTORYON), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_SETMAXPWHISTORY);
  m_ToolTipCtrl->AddTool(GetDlgItem(IDC_SETMAXPWHISTORY), cs_ToolTip);

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
}

void COptionsPasswordHistory::OnResetPWHistoryOff() 
{
  if (((CButton*)GetDlgItem(IDC_RESETPWHISTORYOFF))->GetCheck() == 1) {
  	((CButton*)GetDlgItem(IDC_RESETPWHISTORYON))->SetCheck(0);
  	((CButton*)GetDlgItem(IDC_SETMAXPWHISTORY))->SetCheck(0);
	GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(TRUE);
  } else
    GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(FALSE);
}

void COptionsPasswordHistory::OnResetPWHistoryOn() 
{
  if (((CButton*)GetDlgItem(IDC_RESETPWHISTORYON))->GetCheck() == 1) {
  	((CButton*)GetDlgItem(IDC_RESETPWHISTORYOFF))->SetCheck(0);
  	((CButton*)GetDlgItem(IDC_SETMAXPWHISTORY))->SetCheck(0);
	GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(TRUE);
  } else
    GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(FALSE);
}

void COptionsPasswordHistory::OnSetMaxPWHistory() 
{
  if (((CButton*)GetDlgItem(IDC_SETMAXPWHISTORY))->GetCheck() == 1) {
  	((CButton*)GetDlgItem(IDC_RESETPWHISTORYOFF))->SetCheck(0);
  	((CButton*)GetDlgItem(IDC_RESETPWHISTORYON))->SetCheck(0);
	GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(TRUE);
  } else
    GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(FALSE);
}

void COptionsPasswordHistory::OnBnClickedApplyPWHChanges()
{
  ASSERT(m_pDboxMain != NULL);

  UpdateData(TRUE);
  m_pDboxMain->UpdatePasswordHistory(m_resetpwhistoryoff +
							2 * m_resetpwhistoryon +
							4 * m_setmaxpwhistory,
								m_pwhistorynumdefault);

  ((CButton*)GetDlgItem(IDC_RESETPWHISTORYOFF))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RESETPWHISTORYON))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_SETMAXPWHISTORY))->SetCheck(0);
  GetDlgItem(IDC_APPLYPWHCHANGESNOW)->EnableWindow(FALSE);
}

// Override PreTranslateMessage() so RelayEvent() can be 
// called to pass a mouse message to CPWSOptions's 
// tooltip control for processing.
BOOL COptionsPasswordHistory::PreTranslateMessage(MSG* pMsg) 
{
	if (m_ToolTipCtrl != NULL)
		m_ToolTipCtrl->RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}
