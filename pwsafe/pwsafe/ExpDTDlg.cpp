/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
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

void CExpDTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPIRYDATE, m_pDateCtl);
	DDX_Control(pDX, IDC_EXPIRYTIME, m_pTimeCtl);
}

BEGIN_MESSAGE_MAP(CExpDTDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CExpDTDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CExpDTDlg::OnInitDialog()
{
	TCHAR          szBuf[64];               // workspace
	CString        sTimeFormat;             // the time format being worked on
	CString        sSearch;                 // the string to search for
	int            nIndex;                  // index of the string, if found

	CDialog::OnInitDialog();

    // First get the time format picture.
    VERIFY(::GetLocaleInfo ( LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, szBuf, 64));
    sTimeFormat = szBuf;

    // Next get the separator character.
    VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, szBuf, 64));
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

	CDateTimeCtrl *pTimeCtl = (CDateTimeCtrl*)GetDlgItem(IDC_EXPIRYTIME);
	CDateTimeCtrl *pDateCtl = (CDateTimeCtrl*)GetDlgItem(IDC_EXPIRYDATE);
    pTimeCtl->SetFormat(sTimeFormat);
	pDateCtl->SetFormat(_T("ddd dd'/'MM'/'yyyy"));

	time_t tt;
	CTime ct, xt;
	CTime now(CTime::GetCurrentTime());
	ct = CTime(now.GetYear(), now.GetMonth(), now.GetDay(), 0, 0, 0, -1);

	const CTime sMinDate(ct);
	const CTime sMaxDate(CTime(2038, 1, 1, 0, 0, 0, -1));
	// Set approx. limit of 32-bit times!
	pDateCtl->SetRange(&sMinDate, &sMaxDate);

	if (!PWSUtil::VerifyASCDateTimeString(m_ascLTime, tt)) {
		m_ascLTime.LoadString(IDS_NEVER);
	} else {
		xt = CTime(tt);
		ct = CTime(xt.GetYear(), xt.GetMonth(), xt.GetDay(),
					xt.GetHour(), xt.GetMinute(), 0, -1);
	}

	pDateCtl->SetTime(&ct);
	pTimeCtl->SetTime(&ct);

	GetDlgItem(IDC_STATIC_CURRENT_LTIME)->SetWindowText(m_ascLTime);

	return TRUE;
}

void CExpDTDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	CTime LTime, LDate, LDateTime;
	DWORD dwResult;

	dwResult = m_pTimeCtl.GetTime(LTime);
	ASSERT(dwResult == GDT_VALID);

	dwResult = m_pDateCtl.GetTime(LDate);
	ASSERT(dwResult == GDT_VALID);
	
	LDateTime = CTime(LDate.GetYear(), LDate.GetMonth(), LDate.GetDay(), 
		LTime.GetHour(), LTime.GetMinute(), 0, -1);
	m_tttLTime = (time_t)LDateTime.GetTime();
	m_ascLTime = (CMyString)LDateTime.Format("%a %b %d %H:%M:00 %Y");

	OnOK();
}
