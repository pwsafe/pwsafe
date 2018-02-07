/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterStringDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterPasswordDlg.h"
#include "core/itemdata.h"

// CFilterPasswordDlg dialog

// NOTE: Even though supposed to be just for strings, this dialog deals
// with the special case for Passwords - they have expired or they wil
// expired in 'n' days.

IMPLEMENT_DYNAMIC(CFilterPasswordDlg, CFilterBaseDlg)

CFilterPasswordDlg::CFilterPasswordDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterPasswordDlg::IDD, pParent),
  m_case(BST_UNCHECKED), m_string(L""), m_num1(0),
  m_maxDays(2)
{
}

CFilterPasswordDlg::~CFilterPasswordDlg()
{
}

void CFilterPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterPasswordDlg)
  DDX_Check(pDX, IDC_STRINGCASE, m_case);
  DDX_Text(pDX, IDC_STRING1, m_string);
  DDX_Text(pDX, IDC_INTEGER1, m_num1);
  DDX_Control(pDX, IDC_STRINGRULE, m_cbxRule);
  DDX_Control(pDX, IDC_STRING1, m_edtString);
  DDX_Control(pDX, IDC_STRINGCASE, m_btnCase);
  DDX_Control(pDX, IDC_INTEGER1, m_edtInteger1);
  DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
  DDX_Control(pDX, IDC_STATIC_IN, m_stcIn);
  DDX_Control(pDX, IDC_STATIC_DAYS, m_stcDays);
  //}}AFX_DATA_MAP

  DDV_CheckMinMax(pDX, m_num1, 1, m_maxDays);
}

BEGIN_MESSAGE_MAP(CFilterPasswordDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_STRINGRULE, OnCbnSelchangePasswordRule)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

void AFXAPI CFilterPasswordDlg::DDV_CheckMinMax(CDataExchange* pDX,
                                            const int &num,
                                            const int &min, const int &max)
{
  if (m_rule != PWSMatch::MR_WILLEXPIRE)
    return;

  CGeneralMsgBox gmb;
  if (pDX->m_bSaveAndValidate) {
    if (min != -1 && num < min) {
      CString cs_text;
      cs_text.Format(IDS_NUMTOOSMALL, min);
      gmb.AfxMessageBox(cs_text);
      pDX->Fail();
      return;
    }

    if (max != -1 && num > max) {
      CString cs_text;
      cs_text.Format(IDS_NUMTOOLARGE, max);
      gmb.AfxMessageBox(cs_text);
      pDX->Fail();
      return;
    }
  }
}

// CFilterPasswordDlg message handlers

BOOL CFilterPasswordDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    const PWSMatch::MatchRule mrx[] = {PWSMatch::MR_EQUALS,   PWSMatch::MR_NOTEQUAL,
                                       PWSMatch::MR_BEGINS,   PWSMatch::MR_NOTBEGIN,
                                       PWSMatch::MR_ENDS,     PWSMatch::MR_NOTEND,
                                       PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
                                       PWSMatch::MR_CNTNANY,  PWSMatch::MR_NOTCNTNANY,
                                       PWSMatch::MR_CNTNALL,  PWSMatch::MR_NOTCNTNALL,
                                       PWSMatch::MR_EXPIRED,  PWSMatch::MR_WILLEXPIRE};

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

    EnableDialogItems();
  } else
    m_cbxRule.SetCurSel(-1);

  // Last 32-bit date is 03:14:07 UTC on Tuesday, January 19, 2038
  // Find number of days from now to 2038/01/18 = max value here
  const CTime ct_Latest(2038, 1, 18, 0, 0, 0);
  const CTime ct_Now(CTime::GetCurrentTime());
  CTimeSpan elapsedTime = ct_Latest - ct_Now;
  m_maxDays = (int)elapsedTime.GetDays();

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterPasswordDlg::OnCbnSelchangePasswordRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);
  if (m_rule == PWSMatch::MR_WILLEXPIRE) {
    if (m_num1 < 1)
      m_num1 = 1;
  } else
    m_num1 = 0;

  EnableDialogItems();
}

void CFilterPasswordDlg::EnableDialogItems()
{
    switch (m_rule) {
      case PWSMatch::MR_WILLEXPIRE:
        m_edtString.EnableWindow(FALSE);
        m_edtString.ShowWindow(SW_HIDE);
        m_btnCase.EnableWindow(FALSE);
        m_btnCase.ShowWindow(SW_HIDE);
        m_stcStatus.ShowWindow(SW_HIDE);

        m_edtInteger1.EnableWindow(TRUE);
        m_edtInteger1.ShowWindow(SW_SHOW);
        m_stcIn.ShowWindow(SW_SHOW);
        m_stcDays.ShowWindow(SW_SHOW);
        break;

      case PWSMatch::MR_EXPIRED:
      case PWSMatch::MR_PRESENT:
      case PWSMatch::MR_NOTPRESENT:
        m_edtString.EnableWindow(FALSE);
        m_edtString.ShowWindow(SW_SHOW);
        m_btnCase.EnableWindow(FALSE);
        m_btnCase.ShowWindow(SW_SHOW);
        m_stcStatus.ShowWindow(SW_SHOW);
        m_stcStatus.EnableWindow(FALSE);

        m_edtInteger1.EnableWindow(FALSE);
        m_edtInteger1.ShowWindow(SW_HIDE);
        m_stcIn.ShowWindow(SW_HIDE);
        m_stcDays.ShowWindow(SW_HIDE);
        break;
      default:
        m_edtString.EnableWindow(TRUE);
        m_edtString.ShowWindow(SW_SHOW);
        m_btnCase.EnableWindow(TRUE);
        m_btnCase.ShowWindow(SW_SHOW);
        m_stcStatus.ShowWindow(SW_SHOW);
        m_stcStatus.EnableWindow(TRUE);

        m_edtInteger1.EnableWindow(FALSE);
        m_edtInteger1.ShowWindow(SW_HIDE);
        m_stcIn.ShowWindow(SW_HIDE);
        m_stcDays.ShowWindow(SW_HIDE);
    }
}

void CFilterPasswordDlg::OnBnClickedOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  CGeneralMsgBox gmb;
  if (m_rule == PWSMatch::MR_INVALID) {
    gmb.AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  if (m_rule != PWSMatch::MR_PRESENT &&
      m_rule != PWSMatch::MR_NOTPRESENT &&
      m_rule != PWSMatch::MR_EXPIRED &&
      m_rule != PWSMatch::MR_WILLEXPIRE &&
      m_string.IsEmpty()) {
    gmb.AfxMessageBox(IDS_NOSTRING);
    m_edtString.SetFocus();
    return;
  }

  CFilterBaseDlg::OnOK();
}
