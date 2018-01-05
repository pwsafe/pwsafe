/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterEntryTypeDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterEntryTypeDlg.h"
#include "core/itemdata.h"
#include "core/core.h"

// CFilterEntryTypeDlg dialog

IMPLEMENT_DYNAMIC(CFilterEntryTypeDlg, CFilterBaseDlg)

CFilterEntryTypeDlg::CFilterEntryTypeDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterEntryTypeDlg::IDD, pParent),
  m_etype(CItemData::ET_INVALID)
{
  for (int i = (int)CItemData::ET_NORMAL; i < (int)CItemData::ET_LAST; i++) {
    m_etype2selection[i] = -1;
  }
}

CFilterEntryTypeDlg::~CFilterEntryTypeDlg()
{
}

void CFilterEntryTypeDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterEntryTypeDlg)
  DDX_Control(pDX, IDC_ENTRYTYPERULE, m_cbxRule);
  DDX_Control(pDX, IDC_ENTRYTYPE1, m_cbxEType);
  //{{AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterEntryTypeDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_ENTRYTYPERULE, OnCbnSelchangeEntryTypeRule)
  ON_CBN_SELCHANGE(IDC_ENTRYTYPE1, OnCbnSelchangeEntryType1)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CFilterEntryTypeDlg message handlers

BOOL CFilterEntryTypeDlg::OnInitDialog()
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

  if (m_cbxEType.GetCount() == 0) {
    cs_text.LoadString(IDSC_FNORMAL);
    iItem = m_cbxEType.AddString(cs_text);
    m_cbxEType.SetItemData(iItem, CItemData::ET_NORMAL);
    m_etype2selection[CItemData::ET_NORMAL] = iItem;

    cs_text.LoadString(IDSC_FALIAS);
    iItem = m_cbxEType.AddString(cs_text);
    m_cbxEType.SetItemData(iItem, CItemData::ET_ALIAS);
    m_etype2selection[CItemData::ET_ALIAS] = iItem;

    cs_text.LoadString(IDSC_FSHORTCUT);
    iItem = m_cbxEType.AddString(cs_text);
    m_cbxEType.SetItemData(iItem, CItemData::ET_SHORTCUT);
    m_etype2selection[CItemData::ET_SHORTCUT] = iItem;

    cs_text.LoadString(IDSC_FALIASBASE);
    iItem = m_cbxEType.AddString(cs_text);
    m_cbxEType.SetItemData(iItem, CItemData::ET_ALIASBASE);
    m_etype2selection[CItemData::ET_ALIASBASE] = iItem;

    cs_text.LoadString(IDSC_FSHORTCUTBASE);
    iItem = m_cbxEType.AddString(cs_text);
    m_cbxEType.SetItemData(iItem, CItemData::ET_SHORTCUTBASE);
    m_etype2selection[CItemData::ET_SHORTCUTBASE] = iItem;
  }

  int irsel = m_rule2selection[(int)m_rule];
  if (irsel == -1)
    m_rule = PWSMatch::MR_INVALID;

  int iesel = m_etype2selection[(int)m_etype];
  if (iesel == -1)
    m_etype = CItemData::ET_INVALID;

  if (m_rule != PWSMatch::MR_INVALID &&
      m_etype != CItemData::ET_INVALID) {
    m_cbxRule.SetCurSel(irsel);
    m_cbxEType.SetCurSel(iesel);
  } else {
    m_cbxRule.SetCurSel(-1);
    m_cbxEType.SetCurSel(-1);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterEntryTypeDlg::OnCbnSelchangeEntryTypeRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);
}

void CFilterEntryTypeDlg::OnCbnSelchangeEntryType1()
{
  int isel = m_cbxEType.GetCurSel();
  m_etype = (CItemData::EntryType)m_cbxEType.GetItemData(isel);
}

void CFilterEntryTypeDlg::OnBnClickedOk()
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
