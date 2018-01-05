/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterEntrySizeDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterEntrySizeDlg.h"

// CFilterEntrySizeDlg dialog

IMPLEMENT_DYNAMIC(CFilterEntrySizeDlg, CFilterBaseDlg)

CFilterEntrySizeDlg::CFilterEntrySizeDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterEntrySizeDlg::IDD, pParent),
  m_size1(0), m_size2(0), m_min(-1), m_max(-1), m_unit(0)
{
}

CFilterEntrySizeDlg::~CFilterEntrySizeDlg()
{
}

void CFilterEntrySizeDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterEntrySizeDlg)
  DDX_Text(pDX, IDC_INTEGER1, m_size1);
  DDX_Text(pDX, IDC_INTEGER2, m_size2);
  DDX_Control(pDX, IDC_INTEGERRULE, m_cbxRule);
  DDX_Control(pDX, IDC_INTEGER1, m_edtSize1);
  DDX_Control(pDX, IDC_INTEGER2, m_edtSize2);
  DDX_Control(pDX, IDC_STATIC_AND, m_stcAnd);
  DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
  DDX_Radio(pDX, IDC_SIZE_B, m_unit);
  //}}AFX_DATA_MAP

  DDV_CheckMinMax(pDX, m_size1, m_min, m_max);
  if (m_rule == PWSMatch::MR_BETWEEN) {
    DDV_CheckMinMax(pDX, m_size2, m_min, m_max);
  }
  DDV_CheckNumbers(pDX, m_size1, m_size2);
}

BEGIN_MESSAGE_MAP(CFilterEntrySizeDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_INTEGERRULE, OnCbnSelchangeSizeRule)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
  ON_COMMAND_RANGE(IDC_SIZE_B, IDC_SIZE_MB, OnSizeUnit)
END_MESSAGE_MAP()

void AFXAPI CFilterEntrySizeDlg::DDV_CheckMinMax(CDataExchange* pDX,
                                            const int &size,
                                            const int &min, const int &max)
{
  CGeneralMsgBox gmb;
  if (pDX->m_bSaveAndValidate) {
    if (min != -1 && size < min) {
      CString cs_text;
      cs_text.Format(IDS_NUMTOOSMALL, min);
      gmb.AfxMessageBox(cs_text);
      pDX->Fail();
      return;
    }

    if (max != -1 && size > max) {
      CString cs_text;
      cs_text.Format(IDS_NUMTOOLARGE, max);
      gmb.AfxMessageBox(cs_text);
      pDX->Fail();
      return;
    }
  }
}

void AFXAPI CFilterEntrySizeDlg::DDV_CheckNumbers(CDataExchange* pDX,
                                             const int &size1, const int &size2)
{
  CGeneralMsgBox gmb;
  if (pDX->m_bSaveAndValidate) {
    if (size1 < 0) {
      gmb.AfxMessageBox(IDS_NUM1NEGATIVE);
      pDX->Fail();
      return;
    }

    if (m_rule == PWSMatch::MR_BETWEEN && size1 >= size2) {
      gmb.AfxMessageBox(IDS_NUM1NOTLTNUM2);
      pDX->Fail();
      return;
    }

    if (size1 == m_min && m_rule == PWSMatch::MR_LT) {
      gmb.AfxMessageBox(IDS_CANTBELESSTHANMIN);
      pDX->Fail();
      return;
    }

    if (size1 == m_max && m_rule == PWSMatch::MR_GT) {
      gmb.AfxMessageBox(IDS_CANTBEGREATERTHANMAX);
      pDX->Fail();
      return;
    }

    if (size1 == m_max && m_rule == PWSMatch::MR_BETWEEN) {
      gmb.AfxMessageBox(IDS_NUM1CANTBEMAX);
      pDX->Fail();
      return;
    }
  }
}

// CFilterEntrySizeDlg message handlers

BOOL CFilterEntrySizeDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    const PWSMatch::MatchRule mrx[] = {PWSMatch::MR_EQUALS, PWSMatch::MR_NOTEQUAL,
                                       PWSMatch::MR_LT,     PWSMatch::MR_LE,
                                       PWSMatch::MR_GT,     PWSMatch::MR_GE,
                                       PWSMatch::MR_BETWEEN};

    for (size_t i = 0; i < _countof(mrx); i++) {
      UINT iumsg = PWSMatch::GetRule(mrx[i]);
      cs_text.LoadString(iumsg);
      int iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, mrx[i]);
      m_rule2selection[mrx[i]] = iItem;
    }
  }

  int isel = m_rule2selection[(int)m_rule];
  if (isel == -1)
    m_rule = PWSMatch::MR_INVALID;

  if (m_rule != PWSMatch::MR_INVALID) {
    m_cbxRule.SetCurSel(isel);

    switch (m_rule) {
      case PWSMatch::MR_BETWEEN:
        m_edtSize1.EnableWindow(TRUE);
        m_stcAnd.EnableWindow(TRUE);
        m_edtSize2.EnableWindow(TRUE);
        if (m_size2 < m_size1)
          m_size2 = m_size1;
        break;
      default:
        m_edtSize1.EnableWindow(TRUE);
        m_stcAnd.EnableWindow(FALSE);
        m_edtSize2.EnableWindow(FALSE);
    }
  } else
    m_cbxRule.SetCurSel(-1);

  if (m_unit < 0 || m_unit > 2) {
    ASSERT(0);
    m_unit = 0;
  }

  // Set max. size values (~1GB)
  OnSizeUnit(IDC_SIZE_B + m_unit);
  m_edtSize1.SetLimitText(9);
  m_edtSize2.SetLimitText(9);

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterEntrySizeDlg::OnSizeUnit(UINT nID)
{
  // Set maximum sizes to 999,999,999 B ~= 999,999 KB ~= 999 MB
  m_unit = nID - IDC_SIZE_B;
  switch (m_unit) {
    case 0:
      m_max = 999999999;
      break;
    case 1:
      m_max = 999999;
      break;
    case 2:
      m_max = 999;
      break;
    default:
      ASSERT(0);
  }
}

void CFilterEntrySizeDlg::OnCbnSelchangeSizeRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);

  switch (m_rule) {
    case PWSMatch::MR_BETWEEN:
      m_edtSize1.EnableWindow(TRUE);
      m_stcAnd.EnableWindow(TRUE);
      m_edtSize2.EnableWindow(TRUE);
      m_stcStatus.EnableWindow(TRUE);
      if (m_size2 < m_size1)
        m_size2 = m_size1;
      break;
    default:
      m_edtSize1.EnableWindow(TRUE);
      m_stcAnd.EnableWindow(FALSE);
      m_edtSize2.EnableWindow(FALSE);
      m_stcStatus.EnableWindow(TRUE);
  }
  UpdateData(FALSE);
}

void CFilterEntrySizeDlg::OnBnClickedOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  if (m_rule == PWSMatch::MR_INVALID) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  CFilterBaseDlg::OnOK();
}
