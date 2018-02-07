/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterEntryStatusDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterEntryStatusDlg.h"
#include "core/itemdata.h"
#include "core/core.h"

// CFilterEntryStatusDlg dialog

IMPLEMENT_DYNAMIC(CFilterEntryStatusDlg, CFilterBaseDlg)

CFilterEntryStatusDlg::CFilterEntryStatusDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterEntryStatusDlg::IDD, pParent),
  m_estatus(CItemData::ES_INVALID)
{
  for (int i = (int)CItemData::ES_CLEAN; i < (int)CItemData::ES_LAST; i++) {
    m_estatus2selection[i] = -1;
  }
}

CFilterEntryStatusDlg::~CFilterEntryStatusDlg()
{
}

void CFilterEntryStatusDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterEntryStatusDlg)
  DDX_Control(pDX, IDC_ENTRYSTATUSRULE, m_cbxRule);
  DDX_Control(pDX, IDC_ENTRYSTATUS1, m_cbxEStatus);
  //{{AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterEntryStatusDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_ENTRYSTATUSRULE, OnCbnSelchangeEntryStatusRule)
  ON_CBN_SELCHANGE(IDC_ENTRYSTATUS1, OnCbnSelchangeEntryStatus1)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CFilterEntryStatusDlg message handlers

BOOL CFilterEntryStatusDlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;
  int iItem;

  // NOTE: These ComboBox are NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    cs_text.LoadString(IDSC_IS);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_IS);
    m_rule2selection[PWSMatch::MR_IS] = iItem;

    cs_text.LoadString(IDSC_ISNOT);
    iItem = m_cbxRule.AddString(cs_text);
    m_cbxRule.SetItemData(iItem, PWSMatch::MR_ISNOT);
    m_rule2selection[PWSMatch::MR_ISNOT] = iItem;
  }

  if (m_cbxEStatus.GetCount() == 0) {
    cs_text.LoadString(IDSC_FSCLEAN);
    iItem = m_cbxEStatus.AddString(cs_text);
    m_cbxEStatus.SetItemData(iItem, CItemData::ES_CLEAN);
    m_estatus2selection[CItemData::ES_CLEAN] = iItem;

    cs_text.LoadString(IDSC_FSADDED);
    iItem = m_cbxEStatus.AddString(cs_text);
    m_cbxEStatus.SetItemData(iItem, CItemData::ES_ADDED);
    m_estatus2selection[CItemData::ES_ADDED] = iItem;

    cs_text.LoadString(IDSC_FSMODIFIED);
    iItem = m_cbxEStatus.AddString(cs_text);
    m_cbxEStatus.SetItemData(iItem, CItemData::ES_MODIFIED);
    m_estatus2selection[CItemData::ES_MODIFIED] = iItem;
  }

  int irsel = m_rule2selection[(int)m_rule];
  if (irsel == -1)
    m_rule = PWSMatch::MR_INVALID;

  int iesel = m_estatus2selection[(int)m_estatus];
  if (iesel == -1)
    m_estatus = CItemData::ES_INVALID;

  if (m_rule != PWSMatch::MR_INVALID &&
      m_estatus != CItemData::ES_INVALID) {
    m_cbxRule.SetCurSel(irsel);
    m_cbxEStatus.SetCurSel(iesel);
  } else {
    m_cbxRule.SetCurSel(-1);
    m_cbxEStatus.SetCurSel(-1);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterEntryStatusDlg::OnCbnSelchangeEntryStatusRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);
}

void CFilterEntryStatusDlg::OnCbnSelchangeEntryStatus1()
{
  int isel = m_cbxEStatus.GetCurSel();
  m_estatus = (CItemData::EntryStatus)m_cbxEStatus.GetItemData(isel);
}

void CFilterEntryStatusDlg::OnBnClickedOk()
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
