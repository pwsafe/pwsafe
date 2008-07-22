      /*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterBoolDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "FilterBoolDlg.h"
#include "../corelib/itemdata.h"
#include "../corelib/corelib.h"

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
  int iItem(-1);

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    switch (m_bt) {
      case BT_PRESENT:
        cs_text.LoadString(IDSC_ISPRESENT);
        iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, PWSMatch::MR_PRESENT);
        m_rule2selection[PWSMatch::MR_PRESENT] = iItem;

        cs_text.LoadString(IDSC_ISNOTPRESENT);
        iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTPRESENT);
        m_rule2selection[PWSMatch::MR_NOTPRESENT] = iItem;
        break;
      case BT_ACTIVE:
        cs_text.LoadString(IDSC_ISACTIVE);
        iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, PWSMatch::MR_ACTIVE);
        m_rule2selection[PWSMatch::MR_ACTIVE] = iItem;

        cs_text.LoadString(IDSC_ISINACTIVE);
        iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, PWSMatch::MR_INACTIVE);
        m_rule2selection[PWSMatch::MR_INACTIVE] = iItem;
        break;
      case BT_SET:
        cs_text.LoadString(IDSC_SET);
        iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, PWSMatch::MR_SET);
        m_rule2selection[PWSMatch::MR_SET] = iItem;

        cs_text.LoadString(IDSC_NOTSET);
        iItem = m_cbxRule.AddString(cs_text);
        m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTSET);
        m_rule2selection[PWSMatch::MR_NOTSET] = iItem;
        break;
      default:
        ASSERT(0);
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

  return TRUE;
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
    AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  CFilterBaseDlg::OnOK();
}
