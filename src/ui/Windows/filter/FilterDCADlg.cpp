      /*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterDCADlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterDCADlg.h"
#include "core/itemdata.h"
#include "core/core.h"

// CFilterDCADlg dialog

IMPLEMENT_DYNAMIC(CFilterDCADlg, CFilterBaseDlg)

CFilterDCADlg::CFilterDCADlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterDCADlg::IDD, pParent),
  m_DCA(0)
{
}

CFilterDCADlg::~CFilterDCADlg()
{
}

void CFilterDCADlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterDCADlg)
  DDX_Control(pDX, IDC_DCARULE, m_cbxRule);
  DDX_Control(pDX, IDC_DCA1, m_cbxDCA);
  //{{AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterDCADlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_DCARULE, OnCbnSelchangeDCARule)
  ON_CBN_SELCHANGE(IDC_DCA1, OnCbnSelchangeDCA1)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CFilterDCADlg message handlers

BOOL CFilterDCADlg::OnInitDialog()
{
  CFilterBaseDlg::OnInitDialog();

  CString cs_text;
  int iItem(-1);

  // NOTE: This ComboBox is NOT sorted by design !
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

  if (m_cbxDCA.GetCount() == 0) {
    // This is sorted
    CString cs_dca;
    UINT ui_dca;

    const short si_DCA = (short)PWSprefs::GetInstance()->GetPref(m_type == PWSMatch::MT_DCA ?
        PWSprefs::DoubleClickAction : PWSprefs::ShiftDoubleClickAction);
    switch (si_DCA) {
      case PWSprefs::DoubleClickAutoType:             ui_dca = IDSC_DCAAUTOTYPE;        break;
      case PWSprefs::DoubleClickBrowse:               ui_dca = IDSC_DCABROWSE;          break;
      case PWSprefs::DoubleClickCopyNotes:            ui_dca = IDSC_DCACOPYNOTES;       break;
      case PWSprefs::DoubleClickCopyPassword:         ui_dca = IDSC_DCACOPYPASSWORD;    break;
      case PWSprefs::DoubleClickCopyUsername:         ui_dca = IDSC_DCACOPYUSERNAME;    break;
      case PWSprefs::DoubleClickViewEdit:             ui_dca = IDSC_DCAVIEWEDIT;        break;
      case PWSprefs::DoubleClickCopyPasswordMinimize: ui_dca = IDSC_DCACOPYPASSWORDMIN; break;
      case PWSprefs::DoubleClickBrowsePlus:           ui_dca = IDSC_DCABROWSEPLUS;      break;
      case PWSprefs::DoubleClickRun:                  ui_dca = IDSC_DCARUN;             break;
      case PWSprefs::DoubleClickSendEmail:            ui_dca = IDSC_DCASENDEMAIL;       break;
      default:                                        ui_dca = IDSC_STATCOMPANY;
    }

    cs_dca.LoadString(ui_dca);
    cs_text.Format(IDSC_CURRENTDEFAULTDCA, static_cast<LPCWSTR>(cs_dca));
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, (DWORD)-1);  // Default!

    cs_text.LoadString(IDSC_DCACOPYPASSWORD);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickCopyPassword);

    cs_text.LoadString(IDSC_DCAVIEWEDIT);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickViewEdit);

    cs_text.LoadString(IDSC_DCAAUTOTYPE);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickAutoType);

    cs_text.LoadString(IDSC_DCABROWSE);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickBrowse);

    cs_text.LoadString(IDSC_DCACOPYNOTES);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickCopyNotes);

    cs_text.LoadString(IDSC_DCACOPYUSERNAME);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickCopyUsername);

    cs_text.LoadString(IDSC_DCACOPYPASSWORDMIN);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickCopyPasswordMinimize);

    cs_text.LoadString(IDSC_DCABROWSEPLUS);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickBrowsePlus);

    cs_text.LoadString(IDSC_DCARUN);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickRun);

    cs_text.LoadString(IDSC_DCASENDEMAIL);
    iItem = m_cbxDCA.AddString(cs_text);
    m_cbxDCA.SetItemData(iItem, PWSprefs::DoubleClickSendEmail);

    for (int i = 0; i < m_cbxDCA.GetCount(); i++) {
      DWORD_PTR ival = m_cbxDCA.GetItemData(i);
      m_DCA2selection[ival + 1] = i;  // Note: special case of '+ 1'
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

void CFilterDCADlg::OnCbnSelchangeDCARule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);
}

void CFilterDCADlg::OnCbnSelchangeDCA1()
{
  int isel = m_cbxDCA.GetCurSel();
  m_DCA = (short)m_cbxDCA.GetItemData(isel);
}

void CFilterDCADlg::OnBnClickedOk()
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
