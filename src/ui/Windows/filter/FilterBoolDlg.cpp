/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterBoolDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterBoolDlg.h"
#include "core/itemdata.h"

// CFilterBoolDlg dialog

IMPLEMENT_DYNAMIC(CFilterBoolDlg, CFilterBaseDlg)

CFilterBoolDlg::CFilterBoolDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterBoolDlg::IDD, pParent),
  m_bt(CFilterBoolDlg::BT_PRESENT)
{
}

CFilterBoolDlg::~CFilterBoolDlg()
{
}

void CFilterBoolDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterBoolDlg)
  DDX_Control(pDX, IDC_BOOLRULE, m_cbxRule);
  //{{AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterBoolDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_BOOLRULE, OnCbnSelchangeBoolRule)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CFilterBoolDlg message handlers

BOOL CFilterBoolDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    const PWSMatch::MatchRule mrxp[2] = {PWSMatch::MR_PRESENT, PWSMatch::MR_NOTPRESENT};
    const PWSMatch::MatchRule mrxa[2] = {PWSMatch::MR_ACTIVE,  PWSMatch::MR_INACTIVE};
    const PWSMatch::MatchRule mrxs[2] = {PWSMatch::MR_SET,     PWSMatch::MR_NOTSET};
    const PWSMatch::MatchRule mrxi[2] = {PWSMatch::MR_IS,      PWSMatch::MR_ISNOT};

    const PWSMatch::MatchRule *pmrx(NULL);
    switch (m_bt) {
      case BT_PRESENT:
        pmrx = mrxp;
        break;
      case BT_ACTIVE:
        pmrx = mrxa;
        break;
      case BT_SET:
        pmrx = mrxs;
        break;
      case BT_IS:
        pmrx = mrxi;
        break;
      default:
        ASSERT(0);
    }
    for (size_t i = 0; i < 2; i++) {
      UINT iumsg = PWSMatch::GetRule(pmrx[i]);
      cs_text.LoadString(iumsg);
      int iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, pmrx[i]);
      m_rule2selection[pmrx[i]] = iItem;
    }
  }

  int isel = m_rule2selection[(int)m_rule];
  if (isel == -1)
    m_rule = PWSMatch::MR_INVALID;

  if (m_rule != PWSMatch::MR_INVALID) {
    m_cbxRule.SetCurSel(isel);
  } else
    m_cbxRule.SetCurSel(-1);

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterBoolDlg::OnCbnSelchangeBoolRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);
}

void CFilterBoolDlg::OnBnClickedOk()
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
