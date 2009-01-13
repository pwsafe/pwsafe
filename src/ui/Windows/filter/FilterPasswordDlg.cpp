/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterStringDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "FilterPasswordDlg.h"
#include "corelib/itemdata.h"
#include "corelib/corelib.h"

// CFilterPasswordDlg dialog

// NOTE: Even though supposed to be just for strings, this dialog deals
// with the special case for Passwords - they have expired or they wil
// expired in 'n' days.

IMPLEMENT_DYNAMIC(CFilterPasswordDlg, CFilterBaseDlg)

CFilterPasswordDlg::CFilterPasswordDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterPasswordDlg::IDD, pParent),
  m_case(BST_UNCHECKED), m_string(_T("")), m_num1(0),
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
                                            const int num,
                                            const int min, const int max)
{
  if (m_rule != PWSMatch::MR_WILLEXPIRE)
    return;

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

// CFilterPasswordDlg message handlers

BOOL CFilterPasswordDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;
  int iItem(-1);

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    cs_text.LoadString(IDSC_EQUALS);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_EQUALS);
    m_rule2selection[PWSMatch::MR_EQUALS] = iItem;

    cs_text.LoadString(IDSC_DOESNOTEQUAL);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTEQUAL);
    m_rule2selection[PWSMatch::MR_NOTEQUAL] = iItem;

    cs_text.LoadString(IDSC_BEGINSWITH);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_BEGINS);
    m_rule2selection[PWSMatch::MR_BEGINS] = iItem;

    cs_text.LoadString(IDSC_DOESNOTBEGINSWITH);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTBEGIN);
    m_rule2selection[PWSMatch::MR_NOTBEGIN] = iItem;

    cs_text.LoadString(IDSC_ENDSWITH);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_ENDS);
    m_rule2selection[PWSMatch::MR_ENDS] = iItem;

    cs_text.LoadString(IDSC_DOESNOTENDWITH);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTEND);
    m_rule2selection[PWSMatch::MR_NOTEND] = iItem;

    cs_text.LoadString(IDSC_CONTAINS);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_CONTAINS);
    m_rule2selection[PWSMatch::MR_CONTAINS] = iItem;

    cs_text.LoadString(IDSC_DOESNOTCONTAIN);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTCONTAIN);
    m_rule2selection[PWSMatch::MR_NOTCONTAIN] = iItem;

    cs_text.LoadString(IDSC_EXPIRED);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_EXPIRED);
    m_rule2selection[PWSMatch::MR_EXPIRED] = iItem;

    cs_text.LoadString(IDSC_WILLEXPIRE);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_WILLEXPIRE);
    m_rule2selection[PWSMatch::MR_WILLEXPIRE] = iItem;
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

  return TRUE;
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

  if (m_rule == PWSMatch::MR_INVALID) {
    AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  if (m_rule != PWSMatch::MR_PRESENT &&
      m_rule != PWSMatch::MR_NOTPRESENT &&
      m_rule != PWSMatch::MR_EXPIRED &&
      m_rule != PWSMatch::MR_WILLEXPIRE &&
      m_string.IsEmpty()) {
    AfxMessageBox(IDS_NOSTRING);
    m_edtString.SetFocus();
    return;
  }

  CFilterBaseDlg::OnOK();
}
