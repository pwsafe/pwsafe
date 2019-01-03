/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// FilterStringDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../GeneralMsgBox.h"
#include "FilterStringDlg.h"
#include "core/itemdata.h"

// CFilterStringDlg dialog

// NOTE: Even though supposed to be just for strings, this dialog deals
// with the special case for Passwords - they have expired or they will
// expired in 'n' days.

IMPLEMENT_DYNAMIC(CFilterStringDlg, CFilterBaseDlg)

CFilterStringDlg::CFilterStringDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterStringDlg::IDD, pParent),
  m_case(BST_UNCHECKED), m_string(L""),
  m_add_present(false), m_bSymbol(false)
{
}

CFilterStringDlg::~CFilterStringDlg()
{
}

void CFilterStringDlg::DoDataExchange(CDataExchange* pDX)
{
  CFilterBaseDlg::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFilterStringDlg)
  DDX_Check(pDX, IDC_STRINGCASE, m_case);
  DDX_Text(pDX, IDC_STRING1, m_string);
  DDX_Control(pDX, IDC_STRINGRULE, m_cbxRule);
  if (m_bSymbol)
    DDX_Control(pDX, IDC_STRING1, m_edtSymbolString);
  else
    DDX_Control(pDX, IDC_STRING1, m_edtString);
  DDX_Control(pDX, IDC_STRINGCASE, m_btnCase);
  DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterStringDlg, CFilterBaseDlg)
  ON_CBN_SELCHANGE(IDC_STRINGRULE, OnCbnSelchangeStringRule)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CFilterStringDlg message handlers

BOOL CFilterStringDlg::OnInitDialog()
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
    const PWSMatch::MatchRule mrx[] = {PWSMatch::MR_EQUALS,   PWSMatch::MR_NOTEQUAL,
                                       PWSMatch::MR_BEGINS,   PWSMatch::MR_NOTBEGIN,
                                       PWSMatch::MR_ENDS,     PWSMatch::MR_NOTEND,
                                       PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
                                       PWSMatch::MR_CNTNANY,  PWSMatch::MR_NOTCNTNANY,
                                       PWSMatch::MR_CNTNALL,  PWSMatch::MR_NOTCNTNALL};

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

  UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CFilterStringDlg::OnCbnSelchangeStringRule()
{
  int isel = m_cbxRule.GetCurSel();
  m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);

  EnableDialogItems();
}

void CFilterStringDlg::EnableDialogItems()
{
    switch (m_rule) {
      case PWSMatch::MR_PRESENT:
      case PWSMatch::MR_NOTPRESENT:
        if (m_bSymbol) {
          m_edtSymbolString.EnableWindow(FALSE);
          m_edtSymbolString.ShowWindow(SW_SHOW);
        } else {
          m_edtString.EnableWindow(FALSE);
          m_edtString.ShowWindow(SW_SHOW);
        }
        m_btnCase.EnableWindow(FALSE);
        m_stcStatus.EnableWindow(FALSE);
        break;
      default:
        if (m_bSymbol) {
          m_edtSymbolString.EnableWindow(TRUE);
          m_edtSymbolString.ShowWindow(SW_SHOW);
        } else {
          m_edtString.EnableWindow(TRUE);
          m_edtString.ShowWindow(SW_SHOW);
        }
        m_btnCase.EnableWindow(TRUE);
        m_stcStatus.EnableWindow(TRUE);
    }
}

void CFilterStringDlg::OnBnClickedOk()
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
      m_string.IsEmpty()) {
    gmb.AfxMessageBox(IDS_NOSTRING);
    m_edtString.SetFocus();
    return;
  }

  CFilterBaseDlg::OnOK();
}
