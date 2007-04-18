/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ADVANCEDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "AdvancedDlg.h"
#include "corelib/ItemData.h"
#include "ThisMfcApp.h"
#include <bitset>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CAdvancedDlg::dialog_lookup[ADV_LAST] = {
             IDD_ADVANCED,      // ADV_COMPARE
             IDD_ADVANCEDMERGE, // ADV_MERGE (significantly reduced dialog)
             IDD_ADVANCED,      // ADV_EXPORT_TEXT
             IDD_ADVANCED       // ADV_EXPORT_XML
};

/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg dialog

CAdvancedDlg::CAdvancedDlg(CWnd* pParent /*=NULL*/, int iIndex)
  : CDialog(dialog_lookup[iIndex], pParent) , m_iIndex(iIndex)
{
  //{{AFX_DATA_INIT(CAdvancedDlg)
  if (m_iIndex != ADV_MERGE) {
    if (m_iIndex == ADV_COMPARE) {
      m_ctime = BST_UNCHECKED;
      m_pmtime = BST_UNCHECKED;
      m_atime = BST_UNCHECKED;
      m_rmtime = BST_UNCHECKED;
    } else {
      m_ctime = BST_CHECKED;
      m_pmtime = BST_CHECKED;
      m_atime = BST_CHECKED;
      m_rmtime = BST_CHECKED;
    }
    m_group = BST_CHECKED;
    m_title = BST_CHECKED;
    m_user = BST_CHECKED;
    m_password = BST_CHECKED;
    m_notes = BST_CHECKED;
    m_url = BST_CHECKED;
    m_autotype = BST_CHECKED;
    m_ltime = BST_CHECKED;
    m_pwhist = BST_CHECKED;
  }

  m_subgroup_set = BST_UNCHECKED;
  m_subgroup_name = _T("*");
  m_subgroup_case = BST_UNCHECKED;
  m_subgroup_object = -1;
  m_subgroup_function = -1;
  //}}AFX_DATA_INIT
}

BOOL CAdvancedDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  CString cs_dialogwindowtext;

  switch (m_iIndex) {
    case ADV_COMPARE:
      cs_dialogwindowtext.LoadString(IDS_COMPAREX);
      break;
    case ADV_MERGE:
      cs_dialogwindowtext.LoadString(IDS_MERGEX);
      break;
    case ADV_EXPORT_TEXT:
      cs_dialogwindowtext.LoadString(IDS_EXPORT_TEXTX);
      break;
    case ADV_EXPORT_XML:
      cs_dialogwindowtext.LoadString(IDS_EXPORT_XMLX);
      break;
    default:
      ASSERT(FALSE);
  }
  SetWindowText(cs_dialogwindowtext);
  if (m_iIndex == ADV_EXPORT_XML) {
    m_title = BST_INDETERMINATE;
    m_password = BST_INDETERMINATE;

    CString cs_titlebuttontext, cs_passwordbuttontext;
    cs_titlebuttontext.LoadString(IDS_ADVANCED_TITLETEXT);
    cs_passwordbuttontext.LoadString(IDS_ADVANCED_PASSWORDTEXT);

    CButton *pBtn;
    pBtn = (CButton*)GetDlgItem(IDC_ADVANCED_TITLE);
    pBtn->SetButtonStyle(BS_AUTO3STATE);
    pBtn->SetWindowText(cs_titlebuttontext);
    pBtn->SetCheck(BST_INDETERMINATE);

    pBtn = (CButton*)GetDlgItem(IDC_ADVANCED_PASSWORD);
    pBtn->SetButtonStyle(BS_AUTO3STATE);
    pBtn->SetWindowText(cs_passwordbuttontext);
    pBtn->SetCheck(BST_INDETERMINATE);
  }

  m_bsFields.set();  // note: impossible to set them all even via the advanced dialog

  int index;
  CString cs_text;

  CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION);
  if (cboSubgroupFunction->GetCount() == 0) {
    cs_text.LoadString(IDS_EQUALS);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_EQUALS);
    cs_text.LoadString(IDS_DOESNOTEQUAL);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTEQUAL);
    cs_text.LoadString(IDS_BEGINSWITH);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_BEGINS);
    cs_text.LoadString(IDS_DOESNOTBEGINSWITH);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTBEGIN);
    cs_text.LoadString(IDS_ENDSWITH);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_ENDS);
    cs_text.LoadString(IDS_DOESNOTENDWITH);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTEND);
    cs_text.LoadString(IDS_CONTAINS);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_CONTAINS);
    cs_text.LoadString(IDS_DOESNOTCONTAIN);
    index = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(index, CItemData::SGF_NOTCONTAIN);
  }
  cboSubgroupFunction->SetCurSel(0);

  CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT);
  if (cboSubgroupObject->GetCount() == 0) {
    cs_text.LoadString(IDS_GROUP);
    index = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(index, CItemData::SGO_GROUP);
    cs_text.LoadString(IDS_TITLE);
    index = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(index, CItemData::SGO_TITLE);
    cs_text.LoadString(IDS_USERNAME);
    index = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(index, CItemData::SGO_USER);
    cs_text.LoadString(IDS_GROUPTITLE);
    index = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(index, CItemData::SGO_GROUPTITLE);
    cs_text.LoadString(IDS_URL);
    index = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(index, CItemData::SGO_URL);
    cs_text.LoadString(IDS_NOTES);
    index = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(index, CItemData::SGO_NOTES);
  }
  cboSubgroupObject->SetCurSel(0);

  return TRUE;
}

void CAdvancedDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAdvancedDlg)
  if (m_iIndex != ADV_MERGE) {
    DDX_Check(pDX, IDC_ADVANCED_GROUP, m_group);
    DDX_Check(pDX, IDC_ADVANCED_TITLE, m_title);
    DDX_Check(pDX, IDC_ADVANCED_USER, m_user);
    DDX_Check(pDX, IDC_ADVANCED_NOTES, m_notes);
    DDX_Check(pDX, IDC_ADVANCED_PASSWORD, m_password);
    DDX_Check(pDX, IDC_ADVANCED_CTIME, m_ctime);
    DDX_Check(pDX, IDC_ADVANCED_PMTIME, m_pmtime);
    DDX_Check(pDX, IDC_ADVANCED_ATIME, m_atime);
    DDX_Check(pDX, IDC_ADVANCED_LTIME, m_ltime);
    DDX_Check(pDX, IDC_ADVANCED_RMTIME, m_rmtime);
    DDX_Check(pDX, IDC_ADVANCED_URL, m_url);
    DDX_Check(pDX, IDC_ADVANCED_AUTOTYPE, m_autotype);
    DDX_Check(pDX, IDC_ADVANCED_PWHIST, m_pwhist);
  }

  DDX_Check(pDX, IDC_ADVANCED_SUBGROUP_SET, m_subgroup_set);
  DDX_Check(pDX, IDC_ADVANCED_SUBGROUP_CASE, m_subgroup_case);
  DDX_Text(pDX, IDC_ADVANCED_SUBGROUP_NAME, m_subgroup_name);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdvancedDlg, CDialog)
  //{{AFX_MSG_MAP(CAdvancedDlg)
  ON_BN_CLICKED(IDC_ADVANCED_SETTIMES, OnSetTimes)
  ON_BN_CLICKED(IDC_ADVANCED_CLEARTIMES, OnClearTimes)
  ON_BN_CLICKED(IDC_ADVANCED_SUBGROUP_SET, OnSetSubGroup)
  ON_BN_CLICKED(IDC_ADVANCED_SETALL, OnSetAll)
  ON_BN_CLICKED(IDC_ADVANCED_CLEARALL, OnClearAll)
  ON_BN_CLICKED(IDC_ADVANCED_TITLE, OnSetTitle)
  ON_BN_CLICKED(IDC_ADVANCED_PASSWORD, OnSetPassword)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg message handlers

void CAdvancedDlg::OnHelp()
{
  CString cs_HelpTopic(app.GetHelpFileName());
  switch (m_iIndex) {
    case ADV_COMPARE:
      cs_HelpTopic += _T("::/html/comparex.html");
      break;
    case ADV_MERGE:
      cs_HelpTopic += _T("::/html/mergex.html");
      break;
    case ADV_EXPORT_TEXT:
      cs_HelpTopic += _T("::/html/exporttextx.html");
      break;
    case ADV_EXPORT_XML:
      cs_HelpTopic += _T("::/html/exportxmlx.html");
      break;
    default:
      ASSERT(FALSE);
  }
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CAdvancedDlg::OnOK()
{
  UpdateData();
  m_bsFields.reset();

  if (m_iIndex != ADV_MERGE) {
    if (m_iIndex == ADV_EXPORT_XML) {
      // Mandatory
      m_bsFields.set(CItemData::TITLE, true);
      m_bsFields.set(CItemData::PASSWORD, true);
    } else {
      // Optional
      m_bsFields.set(CItemData::TITLE, m_title == BST_CHECKED ? true : false);
      m_bsFields.set(CItemData::PASSWORD, m_password == BST_CHECKED ? true : false);
    }

    m_bsFields.set(CItemData::GROUP, m_group == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::USER, m_user == BST_CHECKED? true : false);
    m_bsFields.set(CItemData::NOTES, m_notes == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::PASSWORD, m_password == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::CTIME, m_ctime == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::PMTIME, m_pmtime == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::ATIME, m_atime == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::LTIME, m_ltime == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::RMTIME, m_rmtime == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::URL, m_url == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::AUTOTYPE, m_autotype == BST_CHECKED ? true : false);
    m_bsFields.set(CItemData::PWHIST, m_pwhist == BST_CHECKED ? true : false);

    if (m_bsFields.count() == 0) {
      AfxMessageBox(IDS_NOFIELDSFOREXPORT);
      m_bsFields.set();  // note: impossible to set them all even via the advanced dialog
      return;
    }
  }

  if (m_subgroup_set == BST_CHECKED) {
    GetDlgItemText(IDC_ADVANCED_SUBGROUP_NAME, m_subgroup_name);
    int nObject = ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT))->GetCurSel();
    if (nObject == CB_ERR) {
      AfxMessageBox(IDS_NOOBJECT);
      m_bsFields.set();  // note: impossible to set them all even via the advanced dialog
      ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT))->SetFocus();
      return;
    }

    int nFunction = ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION))->GetCurSel();
    if (nFunction == CB_ERR) {
      AfxMessageBox(IDS_NOFUNCTION);
      m_bsFields.set();  // note: impossible to set them all even via the advanced dialog
      ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION))->SetFocus();
      return;
    }

    m_subgroup_object = ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT))->GetItemData(nObject);
    m_subgroup_function = ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION))->GetItemData(nFunction);
    if (m_subgroup_case == BST_CHECKED)
      m_subgroup_function *= (-1);
  }

  if (m_subgroup_name == _T("*"))
    m_subgroup_name.Empty();

  CDialog::OnOK();
}

void CAdvancedDlg::OnSetSubGroup()
{
  m_subgroup_set = ((CButton*)GetDlgItem(IDC_ADVANCED_SUBGROUP_SET))->GetCheck();
  GetDlgItem(IDC_ADVANCED_SUBGROUP_NAME)->
    EnableWindow(m_subgroup_set == BST_CHECKED ? TRUE : FALSE);
}

void CAdvancedDlg::OnSetAll()
{
  if (m_iIndex == ADV_EXPORT_XML) {
    m_title = m_password = BST_INDETERMINATE;
    ((CButton*)GetDlgItem(IDC_ADVANCED_TITLE))->SetCheck(BST_INDETERMINATE);
    ((CButton*)GetDlgItem(IDC_ADVANCED_PASSWORD))->SetCheck(BST_INDETERMINATE);
  } else {
    m_title = m_password = BST_CHECKED;
    ((CButton*)GetDlgItem(IDC_ADVANCED_TITLE))->SetCheck(BST_CHECKED);
    ((CButton*)GetDlgItem(IDC_ADVANCED_PASSWORD))->SetCheck(BST_CHECKED);
  }

  ((CButton*)GetDlgItem(IDC_ADVANCED_GROUP))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_USER))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_NOTES))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_CTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_PMTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_ATIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_LTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_RMTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_URL))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_AUTOTYPE))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_PWHIST))->SetCheck(BST_CHECKED);

  m_group = m_title = m_user = m_notes = m_password =
    m_ctime = m_pmtime = m_atime = m_ltime = m_rmtime =
    m_url = m_autotype = m_pwhist = BST_CHECKED;
}

void CAdvancedDlg::OnClearAll()
{
  if (m_iIndex == ADV_EXPORT_XML) {
    m_title = m_password = BST_INDETERMINATE;
    ((CButton*)GetDlgItem(IDC_ADVANCED_TITLE))->SetCheck(BST_INDETERMINATE);
    ((CButton*)GetDlgItem(IDC_ADVANCED_PASSWORD))->SetCheck(BST_INDETERMINATE);
  } else {
    m_title = m_password = BST_UNCHECKED;
    ((CButton*)GetDlgItem(IDC_ADVANCED_TITLE))->SetCheck(BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_ADVANCED_PASSWORD))->SetCheck(BST_UNCHECKED);
  }

  ((CButton*)GetDlgItem(IDC_ADVANCED_GROUP))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_USER))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_NOTES))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_CTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_PMTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_ATIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_LTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_RMTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_URL))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_AUTOTYPE))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_PWHIST))->SetCheck(BST_UNCHECKED);

  m_group = m_user = m_notes = 
    m_ctime = m_pmtime = m_atime = m_ltime = m_rmtime =
    m_url = m_autotype = m_pwhist = BST_UNCHECKED;

}

void CAdvancedDlg::OnSetTimes()
{
  ((CButton*)GetDlgItem(IDC_ADVANCED_CTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_PMTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_ATIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_LTIME))->SetCheck(BST_CHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_RMTIME))->SetCheck(BST_CHECKED);
  m_ctime = m_pmtime = m_atime = m_ltime = m_rmtime = BST_CHECKED;
}

void CAdvancedDlg::OnClearTimes()
{
  ((CButton*)GetDlgItem(IDC_ADVANCED_CTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_PMTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_ATIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_LTIME))->SetCheck(BST_UNCHECKED);
  ((CButton*)GetDlgItem(IDC_ADVANCED_RMTIME))->SetCheck(BST_UNCHECKED);
  m_ctime = m_pmtime = m_atime = m_ltime = m_rmtime = BST_UNCHECKED;
}

void CAdvancedDlg::OnSetTitle()
{
  if (m_iIndex == ADV_EXPORT_XML) {
    // Disabled control looks bad, just grey out tick using INDETERMINATE trick
    ((CButton*)GetDlgItem(IDC_ADVANCED_TITLE))->SetCheck(BST_INDETERMINATE);
    UpdateData(FALSE);
  }
}

void CAdvancedDlg::OnSetPassword()
{
  if (m_iIndex == ADV_EXPORT_XML) {
    // Disabled control looks bad, just grey out tick using INDETERMINATE trick
    ((CButton*)GetDlgItem(IDC_ADVANCED_PASSWORD))->SetCheck(BST_INDETERMINATE);
    UpdateData(FALSE);
  }
}
