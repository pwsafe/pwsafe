/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "corelib/PWSprefs.h"
#include "corelib/PWSAuxParse.h"

#include "resource3.h"

bool CAddEdit_DateTimes::m_bNumDaysFailed = false;

static void AFXAPI DDV_CheckMaxDays(CDataExchange* pDX, const int &how,
                                    int &numDays, const int &maxDays);

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_DateTimes property page

IMPLEMENT_DYNAMIC(CAddEdit_DateTimes, CAddEdit_PropertyPage)

CAddEdit_DateTimes::CAddEdit_DateTimes(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, CAddEdit_DateTimes::IDD, pAEMD),
  m_how(ABSOLUTE_EXP), m_numDays(1), m_ReuseOnPswdChange(FALSE)
{
}

CAddEdit_DateTimes::~CAddEdit_DateTimes()
{
}

void CAddEdit_DateTimes::DoDataExchange(CDataExchange* pDX)
{
  CAddEdit_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CAddEdit_DateTimes)
  DDX_Text(pDX, IDC_XTIME, (CString&)M_locXTime());
  DDX_Text(pDX, IDC_CTIME, (CString&)M_locCTime());
  DDX_Text(pDX, IDC_PMTIME, (CString&)M_locPMTime());
  DDX_Text(pDX, IDC_ATIME, (CString&)M_locATime());
  DDX_Text(pDX, IDC_XTIME, (CString&)M_locXTime());
  DDX_Text(pDX, IDC_RMTIME, (CString&)M_locRMTime());

  DDX_Text(pDX, IDC_EXPDAYS, m_numDays);
  DDX_Radio(pDX, IDC_SELECTBYDATETIME, m_how);
  DDX_Check(pDX, IDC_REUSE_ON_CHANGE, m_ReuseOnPswdChange);

  DDX_Control(pDX, IDC_EXPIRYDATE, m_pDateCtl);
  DDX_Control(pDX, IDC_EXPIRYTIME, m_pTimeCtl);

  // Validation
  DDV_CheckMaxDays(pDX, m_how, m_numDays, m_maxDays);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_DateTimes, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_DateTimes)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_XTIME_CLEAR, OnClearXTime)
  ON_BN_CLICKED(IDC_XTIME_SET, OnSetXTime)
  ON_BN_CLICKED(IDC_SELECTBYDATETIME, OnDateTime)
  ON_BN_CLICKED(IDC_SELECTBYDAYS, OnDays)
  ON_BN_CLICKED(IDC_REUSE_ON_CHANGE, OnReuseOnPswdChange)
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
      CString csError;
      csError.Format(IDS_MAXNUMDAYSEXCEEDED, maxDays);
      AfxMessageBox(csError);
      CAddEdit_DateTimes::m_bNumDaysFailed = true;
      pDX->Fail();
      return;
    }
  }
}

BOOL CAddEdit_DateTimes::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx (0, WS_EX_CONTROLPARENT);

  // Time fields
  time(&M_tttCPMTime());

  TCHAR szBuf[81];       // workspace
  CString sTimeFormat;   // the time format being worked on
  CString sDateFormat;
  CString sSearch;       // the string to search for
  int nIndex;            // index of the string, if found

  GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);

  // Last 32-bit date is 03:14:07 UTC on Tuesday, January 19, 2038
  // Find number of days from now to 2038/01/18 = max value here
  const CTime ct_Latest(2038, 1, 18, 0, 0, 0);
  const CTime ct_Now(CTime::GetCurrentTime());

  CTimeSpan elapsedTime = ct_Latest - ct_Now;
  m_maxDays = (int)elapsedTime.GetDays();

  CSpinButtonCtrl * pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_EXPDAYSSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_EXPDAYS));
  pspin->SetBase(10);
  pspin->SetRange32(1, m_maxDays);
  pspin->SetPos(1);

  // enable/disable relevant controls, depending on 'how' state
  // RELATIVE_EXP (interval) or ABSOLUTE_EXP
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(m_how == RELATIVE_EXP ? TRUE : FALSE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(m_how == RELATIVE_EXP ? TRUE : FALSE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(m_how == RELATIVE_EXP ? FALSE : TRUE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(m_how == RELATIVE_EXP ? FALSE : TRUE);

  // First get the time format picture.
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, szBuf, 80));
  sTimeFormat = szBuf;

  // Next get the separator character.
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, szBuf, 80));
  // Search for ":ss".
  sSearch = szBuf;
  sSearch += _T("ss");
  nIndex = sTimeFormat.Find(sSearch);

  if (nIndex != -1) {
    // Found it!  Remove it from the format picture.
    sTimeFormat.Delete(nIndex, sSearch.GetLength());
  } else {
    // No ":ss", so try ":s".
    sSearch = szBuf;
    sSearch += _T("s");
    nIndex = sTimeFormat.Find(sSearch);

    if (nIndex != -1 ) {
      // Found it!  Remove it from the format picture.
      sTimeFormat.Delete(nIndex, sSearch.GetLength());
    }
  }
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBuf, 80));
  sDateFormat = szBuf;

  CDateTimeCtrl *pTimeCtl = (CDateTimeCtrl*)GetDlgItem(IDC_EXPIRYTIME);
  CDateTimeCtrl *pDateCtl = (CDateTimeCtrl*)GetDlgItem(IDC_EXPIRYDATE);
  pTimeCtl->SetFormat(sTimeFormat);
  pDateCtl->SetFormat(sDateFormat);

  CTime ct, xt;
  CTime now(CTime::GetCurrentTime());
  ct = CTime(now.GetYear(), now.GetMonth(), now.GetDay(), 0, 0, 0, -1);

  const CTime sMinDate(ct);
  const CTime sMaxDate(CTime(2038, 1, 1, 0, 0, 0, -1));

  // Set approx. limit of 32-bit times!
  pDateCtl->SetRange(&sMinDate, &sMaxDate);

  pDateCtl->SetTime(&ct);
  pTimeCtl->SetTime(&ct);

  GetDlgItem(IDC_STATIC_CURRENT_XTIME)->SetWindowText(M_locXTime());

  if (M_uicaller() == IDS_VIEWENTRY) {
    // Disable Buttons
    GetDlgItem(IDC_XTIME_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(FALSE);
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
  }

  UpdateData(FALSE);

  return TRUE;
}

BOOL CAddEdit_DateTimes::OnKillActive()
{
  CAddEdit_PropertyPage::OnKillActive();

  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return TRUE;
}

LRESULT CAddEdit_DateTimes::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_locXTime()     != M_oldlocXTime() ||
          M_XTimeInt()     != M_oldXTimeInt())
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL CAddEdit_DateTimes::OnApply()
{
  if (UpdateData(TRUE) == FALSE) {
    if (m_bNumDaysFailed) {
      // Set to max.
      m_numDays = m_maxDays;
      UpdateData(FALSE);
    } else
      return FALSE;  // Something else - probably max. saved passwords
  }

  return CAddEdit_PropertyPage::OnApply();
}

void CAddEdit_DateTimes::OnHelp()
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL,
                FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/entering_pwd.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

void CAddEdit_DateTimes::OnClearXTime()
{
  M_locXTime().LoadString(IDS_NEVER);
  GetDlgItem(IDC_XTIME)->SetWindowText((CString)M_locXTime());
  GetDlgItem(IDC_XTIME_RECUR)->SetWindowText(_T(""));
  M_tttXTime() = (time_t)0;
  M_XTimeInt() = 0;
}

void CAddEdit_DateTimes::OnSetXTime()
{
  CTime XTime, LDate, LDateTime;

  if (m_how == ABSOLUTE_EXP) {
    VERIFY(m_pTimeCtl.GetTime(XTime) == GDT_VALID);
    VERIFY(m_pDateCtl.GetTime(LDate) == GDT_VALID);

    LDateTime = CTime(LDate.GetYear(), LDate.GetMonth(), LDate.GetDay(),
                      XTime.GetHour(), XTime.GetMinute(), 0, -1);
    M_XTimeInt() = 0;
  } else { // m_how == RELATIVE_EXP
    if (m_ReuseOnPswdChange == FALSE) { // non-recurring
      LDateTime = CTime::GetCurrentTime() + CTimeSpan(m_numDays, 0, 0, 0);
      M_XTimeInt() = 0;
    } else { // recurring interval
      LDateTime = CTime(M_tttCPMTime()) + CTimeSpan(m_numDays, 0, 0, 0);
      M_XTimeInt() = m_numDays;
    }
  }

  // m_XTimeInt is non-zero iff user specified a relative & recurring exp. date
  M_tttXTime() = (time_t)LDateTime.GetTime();
  M_locXTime() = PWSUtil::ConvertToDateTimeString(M_tttXTime(), TMC_LOCALE);

  CString cs_text(_T(""));
  if (M_XTimeInt() != 0) // recurring expiration
    cs_text.Format(IDS_IN_N_DAYS, M_XTimeInt());

  GetDlgItem(IDC_XTIME)->SetWindowText(M_locXTime());
  GetDlgItem(IDC_XTIME_RECUR)->SetWindowText(cs_text);
}

void CAddEdit_DateTimes::OnDays()
{
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(TRUE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
  m_how = RELATIVE_EXP;
}

void CAddEdit_DateTimes::OnDateTime()
{
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(TRUE);
  m_how = ABSOLUTE_EXP;
}

void CAddEdit_DateTimes::OnReuseOnPswdChange()
{
  ASSERT(m_how == RELATIVE_EXP); // meaningless when absolute date given
  UpdateData(TRUE);

  // If user chose "recurring", then set the max interval to ~10 years
  // (should suffice for most purposes). For non-recurring, limit is
  // the max that won't overflow time_t
  const int new_max = (m_ReuseOnPswdChange == TRUE) ? 3650 : m_maxDays;
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_EXPDAYSSPIN);
  pspin->SetRange32(1, new_max);
  if (m_numDays > new_max)
    m_numDays = 1;

  UpdateData(FALSE);
}
