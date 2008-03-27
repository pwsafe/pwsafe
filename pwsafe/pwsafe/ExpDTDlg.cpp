/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// expDT.cpp : implementation file
//

#include "stdafx.h"
#include "ExpDTDlg.h"
#include "corelib/util.h"

// CExpDTDlg dialog

CExpDTDlg::CExpDTDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CExpDTDlg::IDD, pParent), m_how(DATETIME),
  m_numDays(1), m_ReuseOnPswdChange(FALSE), m_tttLTime(time_t(0)),
  m_tttXTime(time_t(0))
{
  //{{AFX_DATA_INIT(CImportDlg)
  //}}AFX_DATA_INIT
}

void CExpDTDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExpDTDlg)
  DDX_Control(pDX, IDC_EXPIRYDATE, m_pDateCtl);
  DDX_Control(pDX, IDC_EXPIRYTIME, m_pTimeCtl);
  DDX_Radio(pDX, IDC_SELECTBYDATETIME, m_how);
  DDX_Text(pDX, IDC_EXPDAYS, m_numDays);
  DDX_Check(pDX, IDC_REUSE_ON_CHANGE, m_ReuseOnPswdChange);
  //{{AFX_DATA_MAP
  DDV_CheckMaxDays(pDX, m_how, m_numDays, m_maxDays);
}

BEGIN_MESSAGE_MAP(CExpDTDlg, CPWDialog)
  ON_BN_CLICKED(IDOK, &CExpDTDlg::OnOK)
  ON_BN_CLICKED(IDC_SELECTBYDATETIME, OnDateTime)
  ON_BN_CLICKED(IDC_SELECTBYDAYS, OnDays)
  ON_BN_CLICKED(IDC_REUSE_ON_CHANGE, OnReuseOnPswdChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Public functions

void AFXAPI DDV_CheckMaxDays(CDataExchange* pDX, const int &how, 
                             int &numDays, const int &maxDays)
{
  if (pDX->m_bSaveAndValidate) {
    if (how == CExpDTDlg::DAYS && numDays > maxDays) {
      CString csError;
      csError.Format(IDS_MAXNUMDAYSEXCEEDED, maxDays);
      AfxMessageBox(csError);
      pDX->Fail();
      return;
    }
  }
}

BOOL CExpDTDlg::OnInitDialog()
{
  TCHAR szBuf[81];       // workspace
  CString sTimeFormat;   // the time format being worked on
  CString sDateFormat;
  CString sSearch;       // the string to search for
  int nIndex;            // index of the string, if found

  CPWDialog::OnInitDialog();

  GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);

  // Last 32-bit date is 03:14:07 UTC on Tuesday, January 19, 2038
  // Find number of days from now to 2038/01/18 = max value here
  const CTime ct_Latest(2038, 1, 18, 0, 0, 0);
  const CTime ct_Now(CTime::GetCurrentTime());
  CTimeSpan elapsedTime = ct_Latest - ct_Now;
  m_maxDays = (int)elapsedTime.GetDays();

  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_EXPDAYSSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_EXPDAYS));
  pspin->SetBase(10);

  if ((long)m_tttLTime > 0L && (long)m_tttLTime <= 3650L && (long)m_tttXTime != 0L) {
    m_ReuseOnPswdChange = TRUE;
    pspin->SetRange32(1, 3650);  // 10 years!
    pspin->SetPos((long)m_tttLTime);
    m_numDays = (int)m_tttLTime;
    m_how = DAYS;
  } else {
    pspin->SetRange32(1, m_maxDays);
    pspin->SetPos(1);
  }

  GetDlgItem(IDC_EXPDAYS)->EnableWindow(m_how == DAYS ? TRUE : FALSE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(m_how == DAYS ? TRUE : FALSE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(m_how == DAYS ? FALSE : TRUE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(m_how == DAYS ? FALSE : TRUE);

  GetDlgItem(IDC_STATIC_LTINTERVAL_ALWAYS)->ShowWindow(m_ReuseOnPswdChange == TRUE ? 
                                            SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->ShowWindow(m_ReuseOnPswdChange == TRUE ? 
                                            SW_HIDE : SW_SHOW);

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
  VERIFY(::GetLocaleInfo ( LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBuf, 80));
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

  if (m_tttLTime == 0) {
    m_locLTime.LoadString(IDS_NEVER);
  } else {
    if ((long)m_tttLTime > 0L && (long)m_tttLTime <= 3650L) {
      ct =CTime(m_tttXTime) + CTimeSpan((long)m_tttLTime, 0, 0, 0);
    } else {
      xt = CTime(m_tttLTime);
      ct = CTime(xt.GetYear(), xt.GetMonth(), xt.GetDay(),
                 xt.GetHour(), xt.GetMinute(), 0, -1);
    }
    m_locLTime = CMyString(ct.Format(_T("%#c")));
  }

  pDateCtl->SetTime(&ct);
  pTimeCtl->SetTime(&ct);

  GetDlgItem(IDC_STATIC_CURRENT_LTIME)->SetWindowText(m_locLTime);
  UpdateData(FALSE);

  return TRUE;
}

void CExpDTDlg::OnDays() 
{
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(TRUE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
  m_how = DAYS;
}

void CExpDTDlg::OnDateTime() 
{
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(TRUE);
  m_how = DATETIME;
}

void CExpDTDlg::OnReuseOnPswdChange()
{
  UpdateData(TRUE);

  const bool bReuse(m_ReuseOnPswdChange == TRUE);
  const int new_max =bReuse ? 3650 : m_maxDays;
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_EXPDAYSSPIN);
  pspin->SetRange32(1, new_max);
  if (m_numDays > new_max)
    m_numDays = 1;

  GetDlgItem(IDC_STATIC_LTINTERVAL_ALWAYS)->ShowWindow(bReuse ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->ShowWindow(bReuse ? SW_HIDE : SW_SHOW);

  UpdateData(FALSE);
}

void CExpDTDlg::OnOK()
{
  if (UpdateData(TRUE) != TRUE) {
    // Only reason is numDays!  Set to max.
    m_numDays = m_maxDays;
    UpdateData(FALSE);
    return;
  }

  CTime LTime, LDate, LDateTime;
  DWORD dwResult;

  if (m_how == DATETIME) {
    dwResult = m_pTimeCtl.GetTime(LTime);
    ASSERT(dwResult == GDT_VALID);

    dwResult = m_pDateCtl.GetTime(LDate);
    ASSERT(dwResult == GDT_VALID);

    LDateTime = CTime(LDate.GetYear(), LDate.GetMonth(), LDate.GetDay(), 
      LTime.GetHour(), LTime.GetMinute(), 0, -1);
    m_tttLTime = (time_t)LDateTime.GetTime();
  } else {
    if (m_ReuseOnPswdChange == FALSE) {
      LDateTime = CTime::GetCurrentTime() + CTimeSpan(m_numDays, 0, 0, 0);
      m_tttLTime = (time_t)LDateTime.GetTime();
    } else {
      LDateTime = CTime(m_tttXTime) + CTimeSpan(m_numDays, 0, 0, 0);
      m_tttLTime = m_numDays;
    }
  }
  m_locLTime = PWSUtil::ConvertToDateTimeString((time_t)LDateTime.GetTime(), TMC_LOCALE);

  CPWDialog::OnOK();
}
