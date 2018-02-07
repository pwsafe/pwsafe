/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterMediaTypeDlg.cpp : implementation file
//

#include "../stdafx.h"

#include "../GeneralMsgBox.h"
#include "FilterMediaTypeDlg.h"

#include "core/itemdata.h"

// CFilterMediaTypeDlg dialog

// NOTE: Even though supposed to be just for strings, this dialog deals
// with the special case for Passwords - they have expired or they will
// expired in 'n' days.

IMPLEMENT_DYNAMIC(CFilterMediaTypeDlg, CFilterBaseDlg)

CFilterMediaTypeDlg::CFilterMediaTypeDlg(CWnd *pParent /*=NULL*/)
: CFilterBaseDlg(CFilterMediaTypeDlg::IDD, pParent),
m_case(BST_UNCHECKED), m_string(L""),
m_add_present(false)
{
}

CFilterMediaTypeDlg::~CFilterMediaTypeDlg()
{
}

void CFilterMediaTypeDlg::DoDataExchange(CDataExchange *pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterMediaTypeDlg)
  DDX_Check(pDX, IDC_STRINGCASE, m_case);
  DDX_Text(pDX, IDC_STRING1, m_string);
  DDX_Control(pDX, IDC_STRINGRULE, m_cbxRule);
  DDX_Control(pDX, IDC_STRING1, m_edtString);
  DDX_Control(pDX, IDC_STRINGCASE, m_btnCase);
  DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
  DDX_Control(pDX, IDC_CMB_AVALIABLEMEDIATYPES, m_cbxAvailableMTs);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterMediaTypeDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_STRINGRULE, OnCbnSelchangeStringRule)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CFilterMediaTypeDlg message handlers

BOOL CFilterMediaTypeDlg::OnInitDialog()
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
      PWSMatch::MR_BEGINS, PWSMatch::MR_NOTBEGIN,
      PWSMatch::MR_ENDS, PWSMatch::MR_NOTEND,
      PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
      PWSMatch::MR_CNTNANY, PWSMatch::MR_NOTCNTNANY,
      PWSMatch::MR_CNTNALL, PWSMatch::MR_NOTCNTNALL};

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
  } else {
    m_cbxRule.SetCurSel(-1);
    m_case = BST_UNCHECKED;
    m_string = L"";
  }

  if (m_cbxAvailableMTs.GetCount() == 0 && m_psMediaTypes != NULL) {
    size_t i = 0;
    for (auto iter = m_psMediaTypes->begin();
         iter != m_psMediaTypes->end();
         i++, iter++) {
      int iItem = m_cbxAvailableMTs.AddString(iter->c_str());
      m_cbxAvailableMTs.SetItemData(iItem, i);
    }
  }

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterMediaTypeDlg::OnCbnSelchangeStringRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);

  EnableDialogItems();
}

void CFilterMediaTypeDlg::EnableDialogItems()
{
  switch (m_rule) {
    case PWSMatch::MR_PRESENT:
    case PWSMatch::MR_NOTPRESENT:
      m_edtString.EnableWindow(FALSE);
      m_edtString.ShowWindow(SW_SHOW);
      m_btnCase.EnableWindow(FALSE);
      m_stcStatus.EnableWindow(FALSE);
      m_cbxAvailableMTs.EnableWindow(FALSE);
      m_cbxAvailableMTs.ShowWindow(SW_HIDE);
      break;
    case PWSMatch::MR_EQUALS:
    case PWSMatch::MR_NOTEQUAL:
      m_btnCase.EnableWindow(FALSE);
      m_btnCase.ShowWindow(SW_HIDE);
      GetDlgItem(IDC_STRING1)->EnableWindow(FALSE);
      GetDlgItem(IDC_STRING1)->ShowWindow(SW_HIDE);
      m_stcStatus.ShowWindow(SW_HIDE);
      m_cbxAvailableMTs.EnableWindow(TRUE);
      m_cbxAvailableMTs.ShowWindow(SW_SHOW);
      break;
    default:
      m_edtString.EnableWindow(TRUE);
      m_edtString.ShowWindow(SW_SHOW);
      m_btnCase.EnableWindow(TRUE);
      m_stcStatus.EnableWindow(TRUE);
      m_cbxAvailableMTs.EnableWindow(FALSE);
      m_cbxAvailableMTs.ShowWindow(SW_HIDE);
  }
}

void CFilterMediaTypeDlg::OnBnClickedOk()
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
      m_rule != PWSMatch::MR_EQUALS &&
      m_rule != PWSMatch::MR_NOTEQUAL &&
      m_string.IsEmpty()) {
    gmb.AfxMessageBox(IDS_NOSTRING);
    m_edtString.SetFocus();
    return;
  }

  if (m_rule == PWSMatch::MR_EQUALS || m_rule == PWSMatch::MR_NOTEQUAL) {
    const int iSelected = m_cbxAvailableMTs.GetCurSel(); 
    if (iSelected == CB_ERR) {
      gmb.AfxMessageBox(IDS_NOMEDIATYPE);
      m_cbxAvailableMTs.SetFocus();
      return;
    } else {
      const int nChars = m_cbxAvailableMTs.GetLBTextLen(iSelected);
      m_string.Empty();
      m_cbxAvailableMTs.GetLBText(iSelected, m_string.GetBuffer(nChars));
      m_string.ReleaseBuffer();

      UpdateData(FALSE);
    }
  }

  CFilterBaseDlg::OnOK();
}
