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
#include "FilterStringDlg.h"
#include "corelib/itemdata.h"
#include "corelib/corelib.h"

// CFilterStringDlg dialog

// NOTE: Even though supposed to be just for strings, this dialog deals
// with the special case for Passwords - they have expired or they wil
// expired in 'n' days.

IMPLEMENT_DYNAMIC(CFilterStringDlg, CFilterBaseDlg)

CFilterStringDlg::CFilterStringDlg(CWnd* pParent /*=NULL*/)
  : CFilterBaseDlg(CFilterStringDlg::IDD, pParent),
  m_case(BST_UNCHECKED), m_string(L""),
  m_add_present(false)
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
  int iItem(-1);

  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    if (m_add_present) {
      cs_text.LoadString(IDSC_ISPRESENT);
      iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, PWSMatch::MR_PRESENT);
      m_rule2selection[PWSMatch::MR_PRESENT] = iItem;

      cs_text.LoadString(IDSC_ISNOTPRESENT);
      iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, PWSMatch::MR_NOTPRESENT);
      m_rule2selection[PWSMatch::MR_NOTPRESENT] = iItem;
    }
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

  return TRUE;
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
        m_edtString.EnableWindow(FALSE);
        m_edtString.ShowWindow(SW_SHOW);
        m_btnCase.EnableWindow(FALSE);
        m_stcStatus.EnableWindow(FALSE);
        break;
      default:
        m_edtString.EnableWindow(TRUE);
        m_edtString.ShowWindow(SW_SHOW);
        m_btnCase.EnableWindow(TRUE);
        m_stcStatus.EnableWindow(TRUE);
    }
}

void CFilterStringDlg::OnBnClickedOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  if (m_rule == PWSMatch::MR_INVALID) {
    AfxMessageBox(IDS_NORULESELECTED);
    return;
  }

  if (m_rule != PWSMatch::MR_PRESENT &&
      m_rule != PWSMatch::MR_NOTPRESENT &&
      m_string.IsEmpty()) {
    AfxMessageBox(IDS_NOSTRING);
    m_edtString.SetFocus();
    return;
  }

  CFilterBaseDlg::OnOK();
}
