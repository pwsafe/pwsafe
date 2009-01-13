      /*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterDateDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "FilterDateDlg.h"
#include "corelib/itemdata.h"
#include "corelib/corelib.h"

// CFilterDateDlg dialog

IMPLEMENT_DYNAMIC(CFilterDateDlg, CFilterBaseDlg)

CFilterDateDlg::CFilterDateDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterDateDlg::IDD, pParent),
  m_time_t1(0), m_time_t2(0),
  m_ctime1((time_t)0), m_ctime2((time_t)0),
  m_add_present(false)
{
  time_t now;
  time(&now);
  m_ctime1 = m_ctime2 = CTime(now);
}

CFilterDateDlg::~CFilterDateDlg()
{
}

void CFilterDateDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterDateDlg)
  DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_ctime1);
  DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_ctime2);
  DDX_Control(pDX, IDC_DATERULE, m_cbxRule);
  DDX_Control(pDX, IDC_DATETIMEPICKER1, m_dtp1);
  DDX_Control(pDX, IDC_DATETIMEPICKER2, m_dtp2);
  DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
  DDX_Control(pDX, IDC_STATIC_AND, m_stcAnd);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterDateDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_DATERULE, OnCbnSelchangeDateRule)
  ON_BN_CLICKED(IDOK, &CFilterDateDlg::OnBnClickedOk)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER1, OnDtnDatetime1Change)
END_MESSAGE_MAP()

// CFilterDateDlg message handlers

BOOL CFilterDateDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;
  int iItem(-1);

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    if (m_add_present) {
      cs_text.LoadString(IDSC_ISPRESENT);
      iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, PWSMatch::MR_PRESENT);
      m_rule2selection[PWSMatch::MR_PRESENT] = iItem;

      cs_text.LoadString(IDSC_ISNOTPRESENT);
      iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTPRESENT);
      m_rule2selection[PWSMatch::MR_NOTPRESENT] = iItem;
    }
    cs_text.LoadString(IDSC_EQUALS);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_EQUALS);
    m_rule2selection[PWSMatch::MR_EQUALS] = iItem;

    cs_text.LoadString(IDSC_DOESNOTEQUAL);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTEQUAL);
    m_rule2selection[PWSMatch::MR_NOTEQUAL] = iItem;

    cs_text.LoadString(IDSC_BEFORE);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_BEFORE);
    m_rule2selection[PWSMatch::MR_BEFORE] = iItem;

    cs_text.LoadString(IDSC_AFTER);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_AFTER);
    m_rule2selection[PWSMatch::MR_AFTER] = iItem;

    cs_text.LoadString(IDSC_BETWEEN);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_BETWEEN);
    m_rule2selection[PWSMatch::MR_BETWEEN] = iItem;
  }

  TCHAR szBuf[81];       // workspace
  VERIFY(::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBuf, 80));

  m_dtp1.SetFormat(szBuf);
  m_dtp2.SetFormat(szBuf);

  const CTime dtMin((time_t)0);
  const CTime dtMax(2038, 1, 1, 0, 0, 0, -1);  // time32_t limit
  m_dtp1.SetRange(&dtMin, &dtMax);
  m_dtp2.SetRange(&dtMin, &dtMax);

  int isel = m_rule2selection[(int)m_rule];
  if (isel == -1)
    m_rule = PWSMatch::MR_INVALID;

  if (m_rule != PWSMatch::MR_INVALID) {
    m_cbxRule.SetCurSel(isel);

    if (m_time_t1 != 0)
      m_ctime1 = m_time_t1;

    if (m_time_t2 != 0)
      m_ctime2 = m_time_t2;

    switch (m_rule) {
      case PWSMatch::MR_BETWEEN:
        m_dtp1.EnableWindow(TRUE);
        m_stcAnd.EnableWindow(TRUE);
        m_dtp2.EnableWindow(TRUE);
        if (m_ctime2 < m_ctime1)
          m_ctime2 = m_ctime1;
        break;
      case PWSMatch::MR_PRESENT:
      case PWSMatch::MR_NOTPRESENT:
        m_dtp1.EnableWindow(FALSE);
        m_stcAnd.EnableWindow(FALSE);
        m_dtp2.EnableWindow(FALSE);
        break;
      default:
        m_dtp1.EnableWindow(TRUE);
        m_stcAnd.EnableWindow(FALSE);
        m_dtp2.EnableWindow(FALSE);
    }
  } else
    m_cbxRule.SetCurSel(-1);

  UpdateData(FALSE);

  return TRUE;
}

void CFilterDateDlg::OnCbnSelchangeDateRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);

  switch (m_rule) {
    case PWSMatch::MR_BETWEEN:
      m_dtp1.EnableWindow(TRUE);
      m_stcAnd.EnableWindow(TRUE);
      m_dtp2.EnableWindow(TRUE);
      m_stcStatus.EnableWindow(TRUE);
      if (m_ctime2 < m_ctime1)
        m_ctime2 = m_ctime1;
      break;
    case PWSMatch::MR_PRESENT:
    case PWSMatch::MR_NOTPRESENT:
      m_dtp1.EnableWindow(FALSE);
      m_stcAnd.EnableWindow(FALSE);
      m_dtp2.EnableWindow(FALSE);
      m_stcStatus.EnableWindow(FALSE);
      break;
    default:
      m_dtp1.EnableWindow(TRUE);
      m_stcAnd.EnableWindow(FALSE);
      m_dtp2.EnableWindow(FALSE);
      m_stcStatus.EnableWindow(TRUE);
  }
  UpdateData(FALSE);
}

void CFilterDateDlg::OnBnClickedOk()
{
  m_time_t1 = m_time_t2 = (time_t)0;
  UpdateData(TRUE);

  if (m_rule == PWSMatch::MR_INVALID) {
    AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  if (m_rule != PWSMatch::MR_PRESENT &&
      m_rule != PWSMatch::MR_NOTPRESENT) {
    m_time_t1 = (time_t)(CTime(m_ctime1.GetYear(), m_ctime1.GetMonth(), m_ctime1.GetDay(),
                             0, 0, 0).GetTime());

    if (m_rule == PWSMatch::MR_BETWEEN) {
      m_time_t2 = (time_t)(CTime(m_ctime2.GetYear(), m_ctime2.GetMonth(), m_ctime2.GetDay(),
                               0, 0, 0).GetTime());
      if (m_time_t1 >= m_time_t2) {
        AfxMessageBox(IDS_DATE1NOTB4DATE2);
        return;
      }
    }
  }

  CFilterBaseDlg::OnOK();
}

void CFilterDateDlg::OnDtnDatetime1Change(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
  *pResult = 0;

  if ((pDTChange->dwFlags & GDT_VALID) != GDT_VALID)
    return;

  CTime ct(pDTChange->st);
  if (m_rule == PWSMatch::MR_BETWEEN && m_ctime2 <= ct) {
    m_ctime2 = ct;
    UpdateData(FALSE);
  }
}
