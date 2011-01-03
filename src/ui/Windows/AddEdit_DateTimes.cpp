/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "AddEdit_DateTimes.h"
#include "AddEdit_PropertySheet.h"

#include "core/PWSprefs.h"
#include "core/PWSAuxParse.h"

#include "resource3.h"

bool CAddEdit_DateTimes::m_bNumDaysFailed = false;
bool CAddEdit_DateTimes::m_bShowUUID = false;

static void AFXAPI DDV_CheckMaxDays(CDataExchange* pDX, const int &how,
                                    int &numDays, const int &maxDays);

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_DateTimes property page

IMPLEMENT_DYNAMIC(CAddEdit_DateTimes, CAddEdit_PropertyPage)

CAddEdit_DateTimes::CAddEdit_DateTimes(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, CAddEdit_DateTimes::IDD, pAEMD),
  m_how(ABSOLUTE_EXP), m_numDays(1), m_ReuseOnPswdChange(FALSE), m_bInitdone(false)
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
  DDX_Text(pDX, IDC_XTIME, (CString&)M_locXTime());
  DDX_Text(pDX, IDC_CTIME, (CString&)M_locCTime());
  DDX_Text(pDX, IDC_PMTIME, (CString&)M_locPMTime());
  DDX_Text(pDX, IDC_ATIME, (CString&)M_locATime());
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

  ON_EN_CHANGE(IDC_EXPDAYS, OnChanged)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPIRYDATE, OnNotifyChanged)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPIRYDATE, OnNotifyChanged)

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
      return;
    }
  }
}

BOOL CAddEdit_DateTimes::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_DateTimes::OnInitDialog()
{
  CAddEdit_PropertyPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  // Set times
  UpdateTimes();

  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0) {
    // Disable Buttons
    GetDlgItem(IDC_XTIME_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
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
    GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_SET)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_XTIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_CURRENTVALUE)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_CURRENT_XTIME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(FALSE);
    GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_XTIME_RECUR)->EnableWindow(FALSE);
  }

  // Refresh dialog
  m_bInitdone = true;
  UpdateStats();
  return TRUE;
}

BOOL CAddEdit_DateTimes::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}

void CAddEdit_DateTimes::UpdateTimes()
{
  // Time fields
  time(&M_tttCPMTime());

  wchar_t szBuf[81];     // workspace
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
  CString cs_text(L"");
  if (M_XTimeInt() > 0) {
    m_how = RELATIVE_EXP;
    m_numDays = M_XTimeInt();
    m_ReuseOnPswdChange = TRUE;
    cs_text.Format(IDS_IN_N_DAYS, M_XTimeInt());
  }
  GetDlgItem(IDC_XTIME_RECUR)->SetWindowText(cs_text);

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
  sSearch += L"ss";
  nIndex = sTimeFormat.Find(sSearch);

  if (nIndex != -1) {
    // Found it!  Remove it from the format picture.
    sTimeFormat.Delete(nIndex, sSearch.GetLength());
  } else {
    // No ":ss", so try ":s".
    sSearch = szBuf;
    sSearch += L"s";
    nIndex = sTimeFormat.Find(sSearch);

    if (nIndex != -1) {
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

  CTime xt;
  CTime now(CTime::GetCurrentTime());

  if (M_tttXTime() != (time_t)0) {
    xt = CTime(M_tttXTime());
  } else {
    xt = now;
  }

  const CTime sMinDate(now);
  const CTime sMaxDate(CTime(2038, 1, 1, 0, 0, 0, -1));

  // Set approx. limit of 32-bit times!
  pDateCtl->SetRange(&sMinDate, &sMaxDate);

  pDateCtl->SetTime(&xt);
  pTimeCtl->SetTime(&xt);

  GetDlgItem(IDC_STATIC_CURRENT_XTIME)->SetWindowText(M_locXTime());

  // Need to update dialog from OnInitDialog and when called during OnApply
  UpdateData(FALSE);
}

void CAddEdit_DateTimes::UpdateStats()
{
  if (!m_bInitdone)
    return;

  CString cs_text;
  cs_text.Format(L"%u", M_entrysize());

  for (int i = cs_text.GetLength() - 3; i > 0; i -= 3) {
    cs_text.Insert(i, L",");
  }

  GetDlgItem(IDC_ENTRYSIZE)->SetWindowTextW(cs_text);
  GetDlgItem(IDC_ENTRYSIZE)->Invalidate();

  CString cs_uuid(_T("N/A"));
  if (memcmp(M_entry_uuid(), PWScore::NULL_UUID, sizeof(uuid_array_t)) != 0) {
    ostringstreamT os;
    CUUIDGen huuid(M_entry_uuid(), true); // true for canonical format
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
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
      break;
    case PP_UPDATE_TIMES:
      UpdateTimes();
      UpdateWindow();
      break;
    case PP_PROTECT_CHANGED:
    {
      const BOOL bProtect = M_protected() != 0 ? FALSE : TRUE;
      // Enable/Disable Buttons
      GetDlgItem(IDC_XTIME_CLEAR)->EnableWindow(bProtect);
      GetDlgItem(IDC_XTIME_SET)->EnableWindow(bProtect);
      GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(bProtect);
      GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(bProtect);
      GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(bProtect);
      GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(bProtect);
      GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(bProtect);
      GetDlgItem(IDC_STATIC_LTINTERVAL_NOW)->EnableWindow(bProtect);
      GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(bProtect);
      GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(bProtect);
      break;
    }
  }
  return 0L;
}

BOOL CAddEdit_DateTimes::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return CAddEdit_PropertyPage::OnApply();

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

void CAddEdit_DateTimes::OnNotifyChanged(NMHDR *, LRESULT *)
{
  OnChanged();
}

void CAddEdit_DateTimes::OnChanged()
{
  if (!m_bInitdone || m_AEMD.uicaller != IDS_EDITENTRY)
    return;

  UpdateData(TRUE);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_DateTimes::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/entering_pwd_date.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CAddEdit_DateTimes::OnClearXTime()
{
  M_locXTime().LoadString(IDS_NEVER);
  GetDlgItem(IDC_XTIME)->SetWindowText((CString)M_locXTime());
  GetDlgItem(IDC_XTIME_RECUR)->SetWindowText(L"");
  if (M_tttXTime() != (time_t)0 || M_XTimeInt() != 0)
    m_ae_psh->SetChanged(true);

  M_tttXTime() = (time_t)0;
  M_XTimeInt() = 0;
}

void CAddEdit_DateTimes::OnSetXTime()
{
  UpdateData(TRUE);
  CTime XTime, LDate, LDateTime;
  CTime now(CTime::GetCurrentTime());

  if (m_how == ABSOLUTE_EXP) {
    VERIFY(m_pTimeCtl.GetTime(XTime) == GDT_VALID);
    VERIFY(m_pDateCtl.GetTime(LDate) == GDT_VALID);

    LDateTime = CTime(LDate.GetYear(), LDate.GetMonth(), LDate.GetDay(),
                      XTime.GetHour(), XTime.GetMinute(), 0, -1);
    if (now > LDateTime) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_INVALIDEXPIRYDATE, MB_OK | MB_ICONEXCLAMATION);
      return;
    }
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

  CString cs_text(L"");
  if (M_XTimeInt() != 0) // recurring expiration
    cs_text.Format(IDS_IN_N_DAYS, M_XTimeInt());

  GetDlgItem(IDC_XTIME)->SetWindowText(M_locXTime());
  GetDlgItem(IDC_XTIME_RECUR)->SetWindowText(cs_text);
  m_ae_psh->SetChanged(true);
}

void CAddEdit_DateTimes::OnDays()
{
  m_ae_psh->SetChanged(true);

  GetDlgItem(IDC_EXPDAYS)->EnableWindow(TRUE);
  GetDlgItem(IDC_REUSE_ON_CHANGE)->EnableWindow(TRUE);
  GetDlgItem(IDC_EXPIRYDATE)->EnableWindow(FALSE);
  GetDlgItem(IDC_EXPIRYTIME)->EnableWindow(FALSE);
  m_how = RELATIVE_EXP;
}

void CAddEdit_DateTimes::OnDateTime()
{
  m_ae_psh->SetChanged(true);

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

  m_ae_psh->SetChanged(true);

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
