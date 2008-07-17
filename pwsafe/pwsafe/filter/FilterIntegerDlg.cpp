/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterIntegerDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "FilterIntegerDlg.h"
#include "../corelib/itemdata.h"
#include "../corelib/corelib.h"

// CFilterIntegerDlg dialog

IMPLEMENT_DYNAMIC(CFilterIntegerDlg, CPWDialog)

CFilterIntegerDlg::CFilterIntegerDlg(CWnd* pParent /*=NULL*/)
  : CPWDialog(CFilterIntegerDlg::IDD, pParent),
  m_rule(PWSMatch::MR_INVALID), m_bFirst(true),
  m_num1(0), m_num2(0), m_min(-1), m_max(-1)
{
  for (int i = (int)PWSMatch::MR_INVALID; i < (int)PWSMatch::MR_LAST; i++) {
    m_rule2selection[i] = -1;
  }
}

CFilterIntegerDlg::~CFilterIntegerDlg()
{
}

void CFilterIntegerDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterIntegerDlg)
  DDX_Text(pDX, IDC_INTEGER1, m_num1);
  DDX_Text(pDX, IDC_INTEGER2, m_num2);
  DDX_Control(pDX, IDC_INTEGERRULE, m_cbxRule);
  DDX_Control(pDX, IDC_INTEGER1, m_edtInteger1);
  DDX_Control(pDX, IDC_INTEGER2, m_edtInteger2);
  DDX_Control(pDX, IDC_STATIC_AND, m_stcAnd);
  DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
  //}}AFX_DATA_MAP

  DDV_CheckMinMax(pDX, m_num1, m_min, m_max);
  if (m_rule == PWSMatch::MR_BETWEEN) {
    DDV_CheckMinMax(pDX, m_num2, m_min, m_max);
  }
  DDV_CheckNumbers(pDX, m_num1, m_num2);
}

BEGIN_MESSAGE_MAP(CFilterIntegerDlg, CPWDialog)
  ON_CBN_SELCHANGE(IDC_INTEGERRULE, OnCbnSelchangeIntegerRule)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

void AFXAPI CFilterIntegerDlg::DDV_CheckMinMax(CDataExchange* pDX,
                                            const int num,
                                            const int min, const int max)
{
  if (pDX->m_bSaveAndValidate) {
    if (min != -1 && num < min) {
      CString cs_text;
      cs_text.Format(IDS_NUMTOOSMALL, min);
      AfxMessageBox(cs_text);
      pDX->Fail();
      return;
    }

    if (max != -1 && num > max) {
      CString cs_text;
      cs_text.Format(IDS_NUMTOOLARGE, max);
      AfxMessageBox(cs_text);
      pDX->Fail();
      return;
    }
  }
}

void AFXAPI CFilterIntegerDlg::DDV_CheckNumbers(CDataExchange* pDX,
                                             const int num1, const int num2)
{
  if (pDX->m_bSaveAndValidate) {
    if (num1 < 0) {
      AfxMessageBox(IDS_NUM1NEGATIVE);
      pDX->Fail();
      return;
    }

    if (m_rule == PWSMatch::MR_BETWEEN && num1 >= num2) {
      AfxMessageBox(IDS_NUM1NOTLTNUM2);
      pDX->Fail();
      return;
    }

    if (num1 == m_min && m_rule == PWSMatch::MR_LT) {
      AfxMessageBox(IDS_CANTBELESSTHANMIN);
      pDX->Fail();
      return;
    }

    if (num1 == m_max && m_rule == PWSMatch::MR_GT) {
      AfxMessageBox(IDS_CANTBEGREATERTHANMAX);
      pDX->Fail();
      return;
    }

    if (num1 == m_max && m_rule == PWSMatch::MR_BETWEEN) {
      AfxMessageBox(IDS_NUM1CANTBEMAX);
      pDX->Fail();
      return;
    }
  }
}

// CFilterIntegerDlg message handlers

BOOL CFilterIntegerDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

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

    cs_text.LoadString(IDSC_BETWEEN);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_BETWEEN);
    m_rule2selection[PWSMatch::MR_BETWEEN] = iItem;

    cs_text.LoadString(IDSC_LESSTHAN);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_LT);
    m_rule2selection[PWSMatch::MR_LT] = iItem;

    cs_text.LoadString(IDSC_LESSTHANEQUAL);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_LE);
    m_rule2selection[PWSMatch::MR_LE] = iItem;

    cs_text.LoadString(IDSC_GREATERTHAN);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_GT);
    m_rule2selection[PWSMatch::MR_GT] = iItem;

    cs_text.LoadString(IDSC_GREATERTHANEQUAL);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_GE);
    m_rule2selection[PWSMatch::MR_GE] = iItem;
  }

  if (m_bFirst) {
    GetWindowText(m_oldtitle);
    m_bFirst = false;
  }

  SetWindowText(m_oldtitle + m_title);

  int isel = m_rule2selection[(int)m_rule];
  if (isel == -1)
    m_rule = PWSMatch::MR_INVALID;

  if (m_rule != PWSMatch::MR_INVALID) {
    m_cbxRule.SetCurSel(isel);

    switch (m_rule) {
      case PWSMatch::MR_BETWEEN:
        m_edtInteger1.EnableWindow(TRUE);
        m_stcAnd.EnableWindow(TRUE);
        m_edtInteger2.EnableWindow(TRUE);
        if (m_num2 < m_num1)
          m_num2 = m_num1;
        break;
      case PWSMatch::MR_PRESENT:
      case PWSMatch::MR_NOTPRESENT:
        m_edtInteger1.EnableWindow(FALSE);
        m_stcAnd.EnableWindow(FALSE);
        m_edtInteger2.EnableWindow(FALSE);
        break;
      default:
        m_edtInteger1.EnableWindow(TRUE);
        m_stcAnd.EnableWindow(FALSE);
        m_edtInteger2.EnableWindow(FALSE);
    }
  } else
    m_cbxRule.SetCurSel(-1);

  UpdateData(FALSE);

  return TRUE;
}

void CFilterIntegerDlg::OnCbnSelchangeIntegerRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);

  switch (m_rule) {
    case PWSMatch::MR_BETWEEN:
      m_edtInteger1.EnableWindow(TRUE);
      m_stcAnd.EnableWindow(TRUE);
      m_edtInteger2.EnableWindow(TRUE);
      m_stcStatus.EnableWindow(TRUE);
      if (m_num2 < m_num1)
        m_num2 = m_num1;
      break;
    case PWSMatch::MR_PRESENT:
    case PWSMatch::MR_NOTPRESENT:
      m_edtInteger1.EnableWindow(FALSE);
      m_stcAnd.EnableWindow(FALSE);
      m_edtInteger2.EnableWindow(FALSE);
      m_stcStatus.EnableWindow(FALSE);
      break;
    default:
      m_edtInteger1.EnableWindow(TRUE);
      m_stcAnd.EnableWindow(FALSE);
      m_edtInteger2.EnableWindow(FALSE);
      m_stcStatus.EnableWindow(TRUE);
  }
  UpdateData(FALSE);
}

void CFilterIntegerDlg::OnBnClickedOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  if (m_rule == PWSMatch::MR_INVALID) {
    AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  CPWDialog::OnOK();
}
