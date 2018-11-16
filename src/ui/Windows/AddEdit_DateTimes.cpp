/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_DateTimes.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"    // For Help
#include "DboxMain.h"

#include "AddEdit_DateTimes.h"
#include "AddEdit_PropertySheet.h"
#include "GeneralMsgBox.h"

#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"

#include "resource3.h"

using pws_os::CUUID;

bool CAddEdit_DateTimes::m_bNumDaysFailed = false;
bool CAddEdit_DateTimes::m_bShowUUID = false;

static void AFXAPI DDV_CheckMaxDays(CDataExchange* pDX, const int &how,
                                    int &numDays, const int &maxDays);

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_DateTimes property page

IMPLEMENT_DYNAMIC(CAddEdit_DateTimes, CAddEdit_PropertyPage)

CAddEdit_DateTimes::CAddEdit_DateTimes(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_DateTimes::IDD, CAddEdit_DateTimes::IDD_SHORT,
                          pAEMD),
  m_how(NONE_EXP), m_numDays(PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultExpiryDays)), m_inSetX(false),
  m_bRecurringPswdExpiry(FALSE), m_bInitdone(false)
{
#ifdef _DEBUG
  m_bShowUUID = true;
#endif
}

CAddEdit_DateTimes::~CAddEdit_DateTimes()
{
}

void CAddEdit_DateTimes::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_DateTimes)
  DDX_Text(pDX, IDC_CTIME, (CString&)M_locCTime());
  DDX_Text(pDX, IDC_PMTIME, (CString&)M_locPMTime());
  DDX_Text(pDX, IDC_ATIME, (CString&)M_locATime());
  DDX_Text(pDX, IDC_RMTIME, (CString&)M_locRMTime());

  DDX_Text(pDX, IDC_EXPDAYS, m_numDays);
  DDX_Radio(pDX, IDC_SELECTBYDATETIME, m_how);
  DDX_Check(pDX, IDC_REUSE_ON_CHANGE, m_bRecurringPswdExpiry);
  DDX_Control(pDX, IDC_EXPIRYDATE, m_pDateCtl);

  // Validation
  DDV_CheckMaxDays(pDX, m_how, m_numDays, m_maxDays);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_DateTimes, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_DateTimes)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_SELECTBYNONE, OnHowChanged)
  ON_BN_CLICKED(IDC_SELECTBYDATETIME, OnHowChanged)
  ON_BN_CLICKED(IDC_SELECTBYDAYS, OnHowChanged)
  ON_BN_CLICKED(IDC_REUSE_ON_CHANGE, OnRecurringPswdExpiry)

  ON_EN_CHANGE(IDC_EXPDAYS, OnDaysChanged)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPIRYDATE, OnDateTimeChanged)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

static void AFXAPI DDV_CheckMaxDays(CDataExchange* pDX, const int &how,
                                    int &numDays, const int &maxDays)
{
  if (pDX->m_bSaveAndValidate) {
    CAddEdit_DateTimes::m_bNumDaysFailed = false;
    if (how == CAddEdit_DateTimes::RELATIVE_EXP && numDays > maxDays) {
      CGeneralMsgBox gmb;
      CString csError;
      csError.Format(IDS_MAXNUMDAYSEXCEEDED, maxDays);
      gmb.AfxMessageBox(csError);
      CAddEdit_DateTimes::m_bNumDaysFailed = true;
      pDX->Fail();
    }
  }
}

BOOL CAddEdit_DateTimes::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_DateTimes::OnInitDialog()
{
  // Last 32-bit date is 03:14:07 UTC on Tuesday, January 19, 2038
  // Find number of days from now to 2038/01/18 = max value here
  // Need to do this early, since base classe's OnInitDialog()
  // Calls UpdateData() which calls Validator, which uses m_maxDays (phew!)
  const CTime ct_Latest(2038, 1, 18, 0, 0, 0);

  CTimeSpan elapsedTime = ct_Latest - CTime::GetCurrentTime();
  m_maxDays = (int)elapsedTime.GetDays();

  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  // Set times
  UpdateTimes();

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    // Disable Buttons
    GetDlgItem(IDC_SELECTBYNONE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_CURRENTVALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_CURRENT_XTIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
  }

  if (M_uicaller() == IDS_ADDENTRY) {
    // Hide Date & Time statistics not yet set
    GetDlgItem(IDC_STATIC_DTSTATS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_CTIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_CTIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_ATIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ATIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_PMTIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PMTIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_RMTIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_RMTIME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_SIZE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ENTRYSIZE)->ShowWindow(SW_HIDE);
  }

  if (M_uicaller() == IDS_ADDENTRY || !m_bShowUUID) {
    GetDlgItem(IDC_STATIC_UUID)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_UUID)->ShowWindow(SW_HIDE);
  }

  if (M_original_entrytype() == CItemData::ET_ALIAS) {
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYNONE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_CURRENTVALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_CURRENT_XTIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
  }

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_EXPDAYSSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_EXPDAYS));
  pspin->SetBase(10);
  pspin->SetRange32(1, m_maxDays);
  pspin->SetPos(m_numDays);
  if (M_XTimeInt() == 0) {
    // if non-recurring, set num days to correspond to delta
    // between exp date & now, if delta > 0
    const CTime xt(M_tttXTime());
    const CTime now(CTime::GetCurrentTime());
    if (xt > now) {
      pspin->SetPos(int(CTimeSpan(xt - now).GetDays()));
    }
  }

  // Set the date format:
  // First get the time format picture.
  wchar_t szBuf[81];     // workspace
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBuf, 80));
  CString sDateFormat = szBuf;

  CDateTimeCtrl *pDateCtl = (CDateTimeCtrl*)GetDlgItem(IDC_EXPIRYDATE);
  pDateCtl->SetFormat(sDateFormat);

  // Refresh dialog
  m_bInitdone = true;
  UpdateStats();
  return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CAddEdit_DateTimes::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}

void CAddEdit_DateTimes::UpdateTimes()
{
  // From Item to page's controls:

  time(&M_tttCPMTime());

  // Determine m_how from M_* field
  if (M_XTimeInt() > 0) {
    m_how = RELATIVE_EXP;
  } else {
    m_how = (M_tttXTime() == (time_t)0) ? NONE_EXP : ABSOLUTE_EXP;
  }

  // enable/disable relevant controls, depending on 'how' state
  // NONE_EXP, RELATIVE_EXP (interval) or ABSOLUTE_EXP
  switch (m_how) {
  case RELATIVE_EXP:
    m_numDays = M_XTimeInt();
    m_bRecurringPswdExpiry = TRUE;
    GetDlgItem(IDC_EXPDAYS)->EnableWindow(TRUE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(TRUE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(TRUE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    break;
  case ABSOLUTE_EXP:
    GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(TRUE);
    break;
  case NONE_EXP:
    GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    break;
  default:
    ASSERT(0);
  }

  const CTime now(CTime::GetCurrentTime());

  CTime xt;
  if (M_tttXTime() != (time_t)0) {
    xt = CTime(M_tttXTime());
  } else {
    xt = now + CTimeSpan(PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultExpiryDays), 0, 1, 0);
  }

  const CTime sMinDate(xt.GetTime() < now.GetTime() ? xt : now);
  const CTime sMaxDate(CTime(2038, 1, 1, 0, 0, 0, -1));

  CDateTimeCtrl *pDateCtl = (CDateTimeCtrl*)GetDlgItem(IDC_EXPIRYDATE);

  // Set approx. limit of 32-bit times!
  pDateCtl->SetRange(&sMinDate, &sMaxDate);

  pDateCtl->SetTime(&xt);

  if (m_bRecurringPswdExpiry == FALSE) {
    if (xt > now) {
      m_numDays = int(CTimeSpan(xt - now).GetDays());
    } else
      m_numDays = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultExpiryDays);
  }

  GetDlgItem(IDC_STATIC_CURRENT_XTIME)->SetWindowText(M_locXTime());

  UpdateData(FALSE);
}

void CAddEdit_DateTimes::UpdateStats()
{
  if (!m_bInitdone)
    return;

  CString cs_text;
  cs_text.Format(L"%lu", M_entrysize());

  for (int i = cs_text.GetLength() - 3; i > 0; i -= 3) {
    cs_text.Insert(i, L",");
  }

  GetDlgItem(IDC_ENTRYSIZE)->SetWindowTextW(cs_text);
  GetDlgItem(IDC_ENTRYSIZE)->Invalidate();

  CString cs_uuid(MAKEINTRESOURCE(IDS_NA));
  if (M_entry_uuid() != pws_os::CUUID::NullUUID()) {
    ostringstreamT os;
    pws_os::CUUID huuid(*M_entry_uuid().GetARep(), true); // true for canonical format
    os << std::uppercase << huuid;
    cs_uuid = os.str().c_str();
  }
  GetDlgItem(IDC_UUID)->SetWindowText(cs_uuid);
}

LRESULT CAddEdit_DateTimes::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);
  CString cs_text(L"");

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_locXTime()     != M_oldlocXTime() ||
          M_XTimeInt()     != M_oldXTimeInt())
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
      break;
    case PP_UPDATE_TIMES:
      UpdateTimes();
      UpdateWindow();
      break;
  }
  return 0L;
}

BOOL CAddEdit_DateTimes::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE; //CAddEdit_PropertyPage::OnApply();

  if (UpdateData(TRUE) == FALSE) {
    if (m_bNumDaysFailed) {
      // Set to max.
      m_numDays = m_maxDays;
      UpdateData(FALSE);
    } else
      return FALSE;  // Something else
  }

  return CAddEdit_PropertyPage::OnApply();
}

void CAddEdit_DateTimes::OnDateTimeChanged(NMHDR *, LRESULT *pLResult)
{
  *pLResult = 0;
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0 || m_inSetX)
    return;

  SetXTime();
}

void CAddEdit_DateTimes::OnDaysChanged()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0 || m_inSetX)
    return;

  SetXTime();
}

void CAddEdit_DateTimes::OnHelp()
{
  ShowHelp(L"::/html/entering_pwd_date.html");
}

void CAddEdit_DateTimes::OnHowChanged()
{
  UpdateData(TRUE); // Gets new m_how
  switch (m_how) {
  case NONE_EXP:
    M_locXTime().LoadString(IDS_NEVER);
    if (M_tttXTime() != (time_t)0 || M_XTimeInt() != 0)
      m_ae_psh->SetChanged(true);

    M_tttXTime() = (time_t)0;
    M_XTimeInt() = 0;

    GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPDAYSSPIN)->EnableWindow(FALSE);
    break;
  case ABSOLUTE_EXP:
    m_ae_psh->SetChanged(true);
    SetXTime();
    GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(TRUE);
    GetDlgItem(IDC_EXPDAYSSPIN)->EnableWindow(FALSE);
    break;
  case RELATIVE_EXP:
    m_ae_psh->SetChanged(true);
    SetXTime();
    GetDlgItem(IDC_EXPDAYS)->EnableWindow(TRUE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(TRUE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(TRUE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPDAYSSPIN)->EnableWindow(TRUE);
    break;
  default:
    ASSERT(0);
  }
  Invalidate();
}

void CAddEdit_DateTimes::SetXTime()
{
  m_inSetX = true;
  UpdateData(TRUE);
  CTime LDate, LDateTime;
  const CTime now(CTime::GetCurrentTime());

  if (m_how == ABSOLUTE_EXP) {
    VERIFY(m_pDateCtl.GetTime(LDate) == GDT_VALID);

    LDateTime = CTime(LDate.GetYear(), LDate.GetMonth(), LDate.GetDay(), 0, 1, 0);
    m_numDays = static_cast<int>((LDate.GetTime() - now.GetTime()) / (24*60*60)) + 1;
    CString nds;
    nds.Format(L"%d", m_numDays);
    GetDlgItem(IDC_EXPDAYS)->SetWindowText(nds);
    M_XTimeInt() = 0;
  } else { // m_how == RELATIVE_EXP
    const CTime today(now.GetYear(), now.GetMonth(), now.GetDay(), 0, 1, 0);
    LDateTime = today + CTimeSpan(m_numDays, 0, 0, 0);
    VERIFY(m_pDateCtl.SetTime(&LDateTime));
    M_XTimeInt() = m_bRecurringPswdExpiry == FALSE ? 0 : m_numDays;
  }

  M_tttXTime() = (time_t)LDateTime.GetTime();
  M_locXTime() = PWSUtil::ConvertToDateTimeString(M_tttXTime(), PWSUtil::TMC_LOCALE_DATE_ONLY);

  m_ae_psh->SetChanged(true);
  m_inSetX = false;
}

void CAddEdit_DateTimes::OnRecurringPswdExpiry()
{
  ASSERT(m_how == RELATIVE_EXP); // meaningless when absolute date given
  UpdateData(TRUE);

  m_ae_psh->SetChanged(true);

  // If user chose "recurring", then set the max interval to pref max (~10 years)
  // (should suffice for most purposes). For non-recurring, limit is
  // the max that won't overflow time_t
  const int new_max = (m_bRecurringPswdExpiry == TRUE) ?
    PWSprefs::GetInstance()->GetPrefMaxVal(PWSprefs::DefaultExpiryDays) : m_maxDays;
  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_EXPDAYSSPIN);
  pspin->SetRange32(PWSprefs::GetInstance()->GetPrefMinVal(PWSprefs::DefaultExpiryDays), new_max);
  if (m_numDays > new_max)
    m_numDays = 1;

  SetXTime();
  UpdateData(FALSE);
}
