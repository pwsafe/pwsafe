/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// expDT.cpp : implementation file
//

#include "stdafx.h"
#include "ExpDTDlg.h"
#include "corelib/util.h"

// CExpDTDlg dialog

CExpDTDlg::CExpDTDlg(CWnd* pParent /*=NULL*/)
	: CPWDialog(CExpDTDlg::IDD, pParent), m_tttLTime(0)
{
	//{{AFX_DATA_INIT(CImportDlg)
	m_how = 0;
  m_numDays = 1;
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
  //{{AFX_DATA_MAP
  DDV_CheckMaxDays(pDX, m_how, m_numDays, m_maxDays);
}

BEGIN_MESSAGE_MAP(CExpDTDlg, CPWDialog)
	ON_BN_CLICKED(IDOK, &CExpDTDlg::OnOK)
    ON_BN_CLICKED(IDC_SELECTBYDATETIME, OnDateTime)
    ON_BN_CLICKED(IDC_SELECTBYDAYS, OnDays)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Public functions

void AFXAPI DDV_CheckMaxDays(CDataExchange* pDX, const int &how, 
                             int &numDays, const int &maxDays)
{
  if (pDX->m_bSaveAndValidate) {
    if (how == 1 && numDays > maxDays) {
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
	TCHAR          szBuf[81];               // workspace
	CString        sTimeFormat;             // the time format being worked on
  CString        sDateFormat;
	CString        sSearch;                 // the string to search for
	int            nIndex;                  // index of the string, if found

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
  pspin->SetRange32(1, m_maxDays);
  pspin->SetBase(10);
  pspin->SetPos(1);

  // First get the time format picture.
  VERIFY(::GetLocaleInfo ( LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, szBuf, 80));
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
		xt = CTime(m_tttLTime);
		ct = CTime(xt.GetYear(), xt.GetMonth(), xt.GetDay(),
               xt.GetHour(), xt.GetMinute(), 0, -1);
		m_locLTime = CMyString(ct.Format(_T("%#c")));
	}

	pDateCtl->SetTime(&ct);
	pTimeCtl->SetTime(&ct);

	GetDlgItem(IDC_STATIC_CURRENT_LTIME)->SetWindowText(m_locLTime);

	return TRUE;
}

void CExpDTDlg::OnDays() 
{
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
  m_how = 1;
}

void CExpDTDlg::OnDateTime() 
{
  GetDlgItem(IDC_EXPDAYS)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(TRUE);
  m_how = 0;
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

    if (m_how == 0) {
	  dwResult = m_pTimeCtl.GetTime(LTime);
	  ASSERT(dwResult == GDT_VALID);

	  dwResult = m_pDateCtl.GetTime(LDate);
	  ASSERT(dwResult == GDT_VALID);

	  LDateTime = CTime(LDate.GetYear(), LDate.GetMonth(), LDate.GetDay(), 
          LTime.GetHour(), LTime.GetMinute(), 0, -1);
    } else {
      LDateTime = CTime::GetCurrentTime() + CTimeSpan(m_numDays, 0, 0, 0);
    }

	m_tttLTime = (time_t)LDateTime.GetTime();

  SYSTEMTIME systime;
  TCHAR time_str[80], datetime_str[80];
  systime.wYear = (WORD)LDateTime.GetYear();
  systime.wMonth = (WORD)LDateTime.GetMonth();
  systime.wDay = (WORD)LDateTime.GetDay();
  systime.wDayOfWeek = (WORD) LDateTime.GetDayOfWeek();
  systime.wHour = (WORD)LDateTime.GetHour();
  systime.wMinute = (WORD)LDateTime.GetMinute();
  systime.wSecond = (WORD)0;
  systime.wMilliseconds = (WORD)0;
  TCHAR szBuf[80];
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBuf, 80));
  GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, szBuf, datetime_str, 80);
  szBuf[0] = _T(' ');  // Put a blank between date and time
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, &szBuf[1], 79));
  GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, szBuf, time_str, 80);
  _tcscat_s(datetime_str, 80, time_str);
  m_locLTime = CMyString(datetime_str);

	CPWDialog::OnOK();
}
