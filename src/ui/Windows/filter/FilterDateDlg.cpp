      /*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterDateDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterDateDlg.h"
#include "core/itemdata.h"

// CFilterDateDlg dialog

// Allow negative numbers (not supported by style ES_NUMBER)
IMPLEMENT_DYNAMIC(CEditInt, CEdit)

BEGIN_MESSAGE_MAP(CEditInt, CEdit)
  ON_WM_CHAR()
END_MESSAGE_MAP()

void CEditInt::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  DWORD dw = GetSel();

  switch(nChar) {
    case L'+':
    case L'-':
      if (LOWORD(dw) != 0)
        return;
      break;
    case L'0':
    case L'1':
    case L'2':
    case L'3':
    case L'4':
    case L'5':
    case L'6':
    case L'7':
    case L'8':
    case L'9':
    case L'\b':
      break;
    default:
      return;
  }

  CEdit::OnChar(nChar, nRepCnt, nFlags);
}

IMPLEMENT_DYNAMIC(CFilterDateDlg, CFilterBaseDlg)

CFilterDateDlg::CFilterDateDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterDateDlg::IDD, pParent),
  m_num1(0), m_num2(0), m_datetype(0),
  m_time_t1(0), m_time_t2(0),
  m_ctime1((time_t)0), m_ctime2((time_t)0),
  m_add_present(false), m_ft(FT_INVALID)
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
  DDX_Radio(pDX, IDC_SELECTBYDATETIME, m_datetype);
  DDX_Text(pDX, IDC_INTEGER1, m_num1);
  DDX_Text(pDX, IDC_INTEGER2, m_num2);
  DDX_Control(pDX, IDC_DATERULE, m_cbxRule);
  DDX_Control(pDX, IDC_INTEGER1, (CEdit&)m_edtInteger1);
  DDX_Control(pDX, IDC_INTEGER2, (CEdit&)m_edtInteger2);
  DDX_Control(pDX, IDC_DATETIMEPICKER1, m_dtp1);
  DDX_Control(pDX, IDC_DATETIMEPICKER2, m_dtp2);
  DDX_Control(pDX, IDC_STATIC_AND, m_stcAnd);
  DDX_Control(pDX, IDC_STATIC_AND2, m_stcAnd2);
  DDX_Control(pDX, IDC_STATIC_RELDESC, m_stcRelativeDesc);
  //}}AFX_DATA_MAP

  if (m_datetype == 0) {
    /* Absolute */
    DDV_CheckDateValid(pDX, m_ctime1);
    if (m_rule == PWSMatch::MR_BETWEEN) {
      DDV_CheckDateValid(pDX, m_ctime2);
    }
    DDV_CheckDates(pDX, m_ctime1, m_ctime2);
  } else {
    /* Relative */
    DDV_CheckMinMax(pDX, m_num1);
    DDV_CheckDateValid(pDX, m_num1);
    if (m_rule == PWSMatch::MR_BETWEEN) {
      DDV_CheckMinMax(pDX, m_num2);
      DDV_CheckDateValid(pDX, m_num2);
    }
    DDV_CheckDates(pDX, m_num1, m_num2);
  }
}

BEGIN_MESSAGE_MAP(CFilterDateDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_DATERULE, OnCbnSelchangeDateRule)
  ON_BN_CLICKED(IDOK, &CFilterDateDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDC_SELECTBYDATETIME, OnAbsolute)
  ON_BN_CLICKED(IDC_SELECTBYDAYS, OnRelative)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER1, OnDtnDatetime1Change)
END_MESSAGE_MAP()

void AFXAPI CFilterDateDlg::DDV_CheckDateValid(CDataExchange* pDX,
                                               const int &num)
{
  if (m_datetype == 0 /* Absolute */)
    return;

  // Only Expiry dates can be in the past and future
  // All other dates can only be in the past (i.e. created, last accessed etc.)
  if (pDX->m_bSaveAndValidate && m_ft != FT_XTIME && num > 0) {
    CGeneralMsgBox gmb;
    CString cs_text;
    cs_text.LoadString(IDS_INVALIDFUTUREDATE);
    gmb.AfxMessageBox(cs_text);
    pDX->Fail();
  }
}

void AFXAPI CFilterDateDlg::DDV_CheckDateValid(CDataExchange* pDX,
                                               const CTime &ctime)
{
  if (m_datetype == 1 /* Relative */)
    return;

  // Only Expiry dates can be in the past and future
  // All other dates can only be in the past (i.e. created, last accessed etc.)
  if (pDX->m_bSaveAndValidate && m_ft != FT_XTIME && ctime > CTime::GetCurrentTime()) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_INVALIDFUTUREDATE);
    pDX->Fail();
  }
}

void AFXAPI CFilterDateDlg::DDV_CheckMinMax(CDataExchange* pDX,
                                            const int &num)
{
  if (m_datetype == 0 /* Absolute */)
    return;

  if (pDX->m_bSaveAndValidate) {
    CGeneralMsgBox gmb;
    CString cs_text;
    if (num < -3650) {
      cs_text.Format(IDS_NUMTOOSMALL, -3650);
      gmb.AfxMessageBox(cs_text);
      pDX->Fail();
    }

    if (num > 3650) {
      cs_text.Format(IDS_NUMTOOLARGE, 3650);
      gmb.AfxMessageBox(cs_text);
      pDX->Fail();
    }
  }
}

void AFXAPI CFilterDateDlg::DDV_CheckDates(CDataExchange* pDX,
                                           const CTime &ctime1,  const CTime &ctime2)
{
  if (m_datetype == 1 /* Relative */)
    return;

  if (pDX->m_bSaveAndValidate && m_rule == PWSMatch::MR_BETWEEN && ctime1 >= ctime2) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_DATE1NOTB4DATE2);
    pDX->Fail();
  }
}

void AFXAPI CFilterDateDlg::DDV_CheckDates(CDataExchange* pDX,
                                           const int &num1, const int &num2)
{
  if (m_datetype == 0 /* Absolute */)
    return;

  CGeneralMsgBox gmb;
  if (pDX->m_bSaveAndValidate) {
    if (m_rule == PWSMatch::MR_BETWEEN && num1 >= num2) {
      gmb.AfxMessageBox(IDS_NUM1NOTLTNUM2);
      pDX->Fail();
      return;
    }

    if (num1 == -3650 && m_rule == PWSMatch::MR_LT) {
      gmb.AfxMessageBox(IDS_CANTBELESSTHANMIN);
      pDX->Fail();
      return;
    }

    if (num1 == 3650 && m_rule == PWSMatch::MR_GT) {
      gmb.AfxMessageBox(IDS_CANTBEGREATERTHANMAX);
      pDX->Fail();
      return;
    }

    if (num1 == 3650 && m_rule == PWSMatch::MR_BETWEEN) {
      gmb.AfxMessageBox(IDS_NUM1CANTBEMAX);
      pDX->Fail();
      return;
    }
  }
}

// CFilterDateDlg message handlers

BOOL CFilterDateDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    if (m_add_present) {
      const PWSMatch::MatchRule mrx[] = {PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT};

      for (size_t i = 0; i < _countof(mrx); i++) {
        UINT iumsg = PWSMatch::GetRule(mrx[i]);
        cs_text.LoadString(iumsg);
        int iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, mrx[i]);
        m_rule2selection[mrx[i]] = iItem;
      }
    }
    const PWSMatch::MatchRule mrx[] = {PWSMatch::MR_EQUALS, PWSMatch::MR_NOTEQUAL,
                                       PWSMatch::MR_BEFORE, PWSMatch::MR_AFTER,
                                       PWSMatch::MR_BETWEEN};

    for (size_t i = 0; i < _countof(mrx); i++) {
      UINT iumsg = PWSMatch::GetRule(mrx[i]);
      cs_text.LoadString(iumsg);
      int iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, mrx[i]);
      m_rule2selection[mrx[i]] = iItem;
    }
  }

  wchar_t szBuf[81];       // workspace
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

  BOOL bAbsEnable(FALSE), bAbsEnable1(FALSE), bRelEnable(FALSE), bRelEnable1(FALSE);
  BOOL bSelect(FALSE);
  if (m_rule != PWSMatch::MR_INVALID) {
    m_cbxRule.SetCurSel(isel);

    if (m_time_t1 != 0)
      m_ctime1 = m_time_t1;

    if (m_time_t2 != 0)
      m_ctime2 = m_time_t2;

    bSelect = TRUE;
    switch (m_rule) {
      case PWSMatch::MR_BETWEEN:
        bAbsEnable = bAbsEnable1 = (m_datetype == 0) ? TRUE : FALSE;
        bRelEnable = bRelEnable1 = (m_datetype == 0) ? FALSE : TRUE;
        if (m_datetype == 0 /* Absolute */) {
          if (m_ctime2 < m_ctime1)
            m_ctime2 = m_ctime1;
        } else {
          if (m_num2 < m_num1)
            m_num2 = m_num1;
        }
        break;
      case PWSMatch::MR_PRESENT:
      case PWSMatch::MR_NOTPRESENT:
        bSelect = bAbsEnable = bAbsEnable1 = bRelEnable = bRelEnable1 = FALSE;
        break;
      default:
        if (m_datetype == 0 /* Absolute */) {
          bAbsEnable1 = TRUE;
        } else {
          bRelEnable1 = TRUE;
        }
    }
  } else
    m_cbxRule.SetCurSel(-1);

  GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(bSelect);
  GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(bSelect);
  m_dtp1.EnableWindow(bAbsEnable1);
  m_stcAnd.EnableWindow(bAbsEnable);
  m_dtp2.EnableWindow(bAbsEnable);
  m_edtInteger1.EnableWindow(bRelEnable1);
  m_stcAnd2.EnableWindow(bRelEnable);
  m_edtInteger2.EnableWindow(bRelEnable);
  m_stcRelativeDesc.EnableWindow(bSelect);

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterDateDlg::OnCbnSelchangeDateRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);

  BOOL bAbsEnable(FALSE), bAbsEnable1(FALSE), bRelEnable(FALSE), bRelEnable1(FALSE);
  BOOL bSelect(TRUE);
  switch (m_rule) {
    case PWSMatch::MR_BETWEEN:
      bAbsEnable = bAbsEnable1 = (m_datetype == 0) ? TRUE : FALSE;
      bRelEnable = bRelEnable1 = (m_datetype == 0) ? FALSE : TRUE;
      if (m_datetype == 0 /* Absolute */) {
        if (m_ctime2 < m_ctime1)
          m_ctime2 = m_ctime1;
      } else {
        if (m_num2 < m_num1)
          m_num2 = m_num1;
      }
      break;
    case PWSMatch::MR_PRESENT:
    case PWSMatch::MR_NOTPRESENT:
      bSelect = bAbsEnable = bAbsEnable1 = bRelEnable = bRelEnable1 = FALSE;
      break;
    default:
      if (m_datetype == 0 /* Absolute */) {
        bAbsEnable1 = TRUE;
      } else {
        bRelEnable1 = TRUE;
      }
  }
  GetDlgItem(IDC_SELECTBYDATETIME)->EnableWindow(bSelect);
  GetDlgItem(IDC_SELECTBYDAYS)->EnableWindow(bSelect);
  m_dtp1.EnableWindow(bAbsEnable1);
  m_stcAnd.EnableWindow(bAbsEnable);
  m_dtp2.EnableWindow(bAbsEnable);
  m_edtInteger1.EnableWindow(bRelEnable1);
  m_stcAnd2.EnableWindow(bRelEnable);
  m_edtInteger2.EnableWindow(bRelEnable);
  m_stcRelativeDesc.EnableWindow(bSelect);

  UpdateData(FALSE);
}

void CFilterDateDlg::OnAbsolute()
{
  m_dtp1.EnableWindow(TRUE);
  m_dtp2.EnableWindow(m_rule == PWSMatch::MR_BETWEEN ? TRUE :FALSE);
  m_stcAnd.EnableWindow(m_rule == PWSMatch::MR_BETWEEN ? TRUE :FALSE);
  m_edtInteger1.EnableWindow(FALSE);
  m_edtInteger2.EnableWindow(FALSE);
  m_stcAnd2.EnableWindow(FALSE);
  m_datetype = 0;  // Absolute
}

void CFilterDateDlg::OnRelative()
{
  m_dtp1.EnableWindow(FALSE);
  m_dtp2.EnableWindow(FALSE);
  m_stcAnd.EnableWindow(FALSE);
  m_edtInteger1.EnableWindow(TRUE);
  m_edtInteger2.EnableWindow(m_rule == PWSMatch::MR_BETWEEN ? TRUE : FALSE);
  m_stcAnd2.EnableWindow(m_rule == PWSMatch::MR_BETWEEN ? TRUE : FALSE);
  m_datetype = 1;  // Relative
}

void CFilterDateDlg::OnCancel()
{
  m_datetype = 0;
  m_num1 = m_num2 = 0;
  m_time_t1 = m_time_t2 = (time_t)0;

  CFilterBaseDlg::OnCancel();
}

void CFilterDateDlg::OnBnClickedOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  m_time_t1 = m_time_t2 = (time_t)0;

  if (m_rule == PWSMatch::MR_INVALID) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  if (m_rule != PWSMatch::MR_PRESENT &&
      m_rule != PWSMatch::MR_NOTPRESENT) {
    if (m_datetype == 0 /* Absolute */) {
      m_time_t1 = (time_t)(CTime(m_ctime1.GetYear(), m_ctime1.GetMonth(), m_ctime1.GetDay(),
                             0, 0, 0).GetTime());

      if (m_rule == PWSMatch::MR_BETWEEN) {
        m_time_t2 = (time_t)(CTime(m_ctime2.GetYear(), m_ctime2.GetMonth(), m_ctime2.GetDay(),
                               0, 0, 0).GetTime());
        if (m_time_t1 >= m_time_t2) {
          CGeneralMsgBox gmb;
          gmb.AfxMessageBox(IDS_DATE1NOTB4DATE2);
          return;
        }
      }
    } else {
      /* Relative */
      if (m_rule != PWSMatch::MR_BETWEEN)
        m_num2 = 0;
    }
  }

  CFilterBaseDlg::OnOK();
}

void CFilterDateDlg::OnDtnDatetime1Change(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNotifyStruct);
  *pLResult = 0;

  if ((pDTChange->dwFlags & GDT_VALID) != GDT_VALID)
    return;

  CTime ct(pDTChange->st);
  if (m_rule == PWSMatch::MR_BETWEEN && m_ctime2 <= ct) {
    m_ctime2 = ct;
    UpdateData(FALSE);
  }
}
