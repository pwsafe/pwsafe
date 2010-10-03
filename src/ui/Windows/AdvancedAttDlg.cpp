/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ADVANCEDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "AdvancedAttDlg.h"

#include "corelib/corelib.h"

#include "resource.h"
#include "resource3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT CAdvancedAttDlg::ui_tests[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_ADVANCED_TEST1, IDC_ADVANCED_TEST2, IDC_ADVANCED_TEST3};
const UINT CAdvancedAttDlg::ui_fncts[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_ADVANCED_FUNCTION1, IDC_ADVANCED_FUNCTION2, IDC_ADVANCED_FUNCTION3};
const UINT CAdvancedAttDlg::ui_names[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_ADVANCED_NAME1, IDC_ADVANCED_NAME2, IDC_ADVANCED_NAME3};
const UINT CAdvancedAttDlg::ui_objs[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_ADVANCED_OBJECT1, IDC_ADVANCED_OBJECT2, IDC_ADVANCED_OBJECT3};
const UINT CAdvancedAttDlg::ui_cases[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_ADVANCED_CASE1, IDC_ADVANCED_CASE2, IDC_ADVANCED_CASE3};
const UINT CAdvancedAttDlg::ui_texts[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_STATIC_TEXT1, IDC_STATIC_TEXT2, IDC_STATIC_TEXT3};
const UINT CAdvancedAttDlg::ui_where[CAdvancedAttDlg::NUM_TESTS] = {
             IDC_STATIC_WHERE1, IDC_STATIC_WHERE2, IDC_STATIC_WHERE3};

/////////////////////////////////////////////////////////////////////////////
// CAdvancedAttDlg dialog

CAdvancedAttDlg::CAdvancedAttDlg(CWnd* pParent, ATFVector &vatf)
  : CPWDialog(IDD, pParent), m_vatf(vatf)
{
  //{{AFX_DATA_INIT(CAdvancedAttDlg)
  //}}AFX_DATA_INIT
  ASSERT(NUM_TESTS == vatf.size());

  for (int i = 0; i < NUM_TESTS; i++) {
    m_test_value[i] = vatf[i].value.c_str();
    m_test_set[i] = vatf[i].set;
    m_test_object[i] = vatf[i].object; 
    m_test_function[i] = vatf[i].function;

    if (m_test_function[i] < 0) {
      m_test_case[i] = BST_CHECKED;
      m_test_function[i] *= (-1);
    } else
      m_test_case[i] = BST_UNCHECKED;

    if (m_test_set[i] == BST_UNCHECKED)
      m_test_value[i] = L"*";
  }
}

BOOL CAdvancedAttDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString cs_text, cs_tmp;
  int iItem(-1), i, j;

  cs_text.Format(IDS_EXPORT_XMLX, cs_tmp);
  SetWindowText(cs_text);

  for (j = 0; j < NUM_TESTS; j++) {
    CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(ui_fncts[j]);
    if (cboSubgroupFunction->GetCount() == 0) {
      cs_text.LoadString(IDSC_EQUALS);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_EQUALS);
      cs_text.LoadString(IDSC_DOESNOTEQUAL);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_NOTEQUAL);
      cs_text.LoadString(IDSC_BEGINSWITH);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_BEGINS);
      cs_text.LoadString(IDSC_DOESNOTBEGINSWITH);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_NOTBEGIN);
      cs_text.LoadString(IDSC_ENDSWITH);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_ENDS);
      cs_text.LoadString(IDSC_DOESNOTENDWITH);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_NOTEND);
      cs_text.LoadString(IDSC_CONTAINS);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_CONTAINS);
      cs_text.LoadString(IDSC_DOESNOTCONTAIN);
      iItem = cboSubgroupFunction->AddString(cs_text);
      cboSubgroupFunction->SetItemData(iItem, PWSMatch::MR_NOTCONTAIN);
  
      for (i = 0; i < cboSubgroupFunction->GetCount(); i++) {
        if ((int)cboSubgroupFunction->GetItemData(i) == m_test_function[j]) {
          cboSubgroupFunction->SetCurSel(i);
          break;
        }
      }
    }

    // Note: NOT SORTED by design
    CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(ui_objs[j]);
    if (cboSubgroupObject->GetCount () == 0) {
      cs_text.LoadString(IDS_GROUP);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTGROUP);
      cs_text.LoadString(IDS_GROUPTITLE);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTGROUPTITLE);
      cs_text.LoadString(IDS_TITLE);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTTITLE);
      cs_text.LoadString(IDS_USERNAME);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTUSER);
      cs_text.LoadString(IDS_PATH);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTPATH);
      cs_text.LoadString(IDS_FILENAME_EXTN);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTFILENAME);
      cs_text.LoadString(IDS_DESCRIPTION);
      iItem = cboSubgroupObject->AddString(cs_text);
      cboSubgroupObject->SetItemData(iItem, ATTDESCRIPTION);
    }
  
    for (i = 0; i < cboSubgroupObject->GetCount(); i++) {
      if ((int)cboSubgroupObject->GetItemData(i) == m_test_object[j]) {
        cboSubgroupObject->SetCurSel(i);
        break;
      }
    }

    BOOL bEnable = (m_test_set[j] == BST_CHECKED) ? TRUE : FALSE;
    GetDlgItem(ui_fncts[j])->EnableWindow(bEnable);
    GetDlgItem(ui_objs[j])->EnableWindow(bEnable);
    GetDlgItem(ui_names[j])->EnableWindow(bEnable);
    GetDlgItem(ui_cases[j])->EnableWindow(bEnable);
    GetDlgItem(ui_where[j])->EnableWindow(bEnable);
    GetDlgItem(ui_texts[j])->EnableWindow(bEnable);
    
    GetDlgItem(ui_tests[j])->EnableWindow(TRUE);
  }

  return TRUE;
}

void CAdvancedAttDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAdvancedAttDlg)
    DDX_Check(pDX, IDC_ADVANCED_TEST1, m_test_set[0]);
    DDX_Check(pDX, IDC_ADVANCED_CASE1, m_test_case[0]);
    DDX_Text(pDX, IDC_ADVANCED_NAME1, m_test_value[0]);
    DDX_Check(pDX, IDC_ADVANCED_TEST2, m_test_set[1]);
    DDX_Check(pDX, IDC_ADVANCED_CASE2, m_test_case[1]);
    DDX_Text(pDX, IDC_ADVANCED_NAME2, m_test_value[1]);
    DDX_Check(pDX, IDC_ADVANCED_TEST3, m_test_set[2]);
    DDX_Check(pDX, IDC_ADVANCED_CASE3, m_test_case[2]);
    DDX_Text(pDX, IDC_ADVANCED_NAME3, m_test_value[2]);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdvancedAttDlg, CPWDialog)
  //{{AFX_MSG_MAP(CAdvancedAttDlg)
  // NOTE: IDC_ADVANCED_TEST1 ... IDC_ADVANCED_TEST3 must be consecutive in resource.h
  ON_COMMAND_RANGE(IDC_ADVANCED_TEST1, IDC_ADVANCED_TEST3, OnSetTest)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedAttDlg message handlers

void CAdvancedAttDlg::OnHelp()
{
  CString cs_HelpTopic(app.GetHelpFileName());
  cs_HelpTopic += L"::/html/export_att.html";

  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CAdvancedAttDlg::OnOK()
{
  CGeneralMsgBox gmb;
  CString cs_text;

  UpdateData();

  for (int i = 0; i < NUM_TESTS; i++) {
    if (m_test_set[i] == BST_CHECKED) {
      GetDlgItemText(ui_names[i], m_test_value[i]);
      int nObject = ((CComboBox *)GetDlgItem(ui_objs[i]))->GetCurSel();
      if (nObject == CB_ERR) {
        gmb.AfxMessageBox(IDS_NOOBJECT);
        ((CComboBox *)GetDlgItem(ui_objs[i]))->SetFocus();
        return;
      }
  
      int nFunction = ((CComboBox *)GetDlgItem(ui_fncts[i]))->GetCurSel();
      if (nFunction == CB_ERR) {
        gmb.AfxMessageBox(IDS_NOFUNCTION);
        ((CComboBox *)GetDlgItem(ui_fncts[i]))->SetFocus();
        return;
      }
 
      m_test_object[i] = int(((CComboBox *)GetDlgItem(ui_objs[i]))->GetItemData(nObject));
      m_test_function[i] = int(((CComboBox *)GetDlgItem(ui_fncts[i]))->GetItemData(nFunction));
      if (m_test_case[i] == BST_CHECKED)
        m_test_function[i] *= (-1);
    }

    if (m_test_value[i] == L"*")
      m_test_value[i].Empty();

    m_vatf[i].value = m_test_value[i];
    m_vatf[i].set = m_test_set[i];
    m_vatf[i].object = m_test_object[i]; 
    m_vatf[i].function = m_test_function[i];
  }

  CPWDialog::OnOK();
}

void CAdvancedAttDlg::OnSetTest(UINT id)
{
  int i(0);
  for (int j = 0; j < NUM_TESTS; j++) {
    if (ui_tests[j] == id) {
      i = j;
      break;
    }
  }

  m_test_set[i] = ((CButton*)GetDlgItem(id))->GetCheck();

  BOOL bEnable = (m_test_set[i] == BST_CHECKED) ? TRUE : FALSE;
  GetDlgItem(ui_fncts[i])->EnableWindow(bEnable);
  GetDlgItem(ui_objs[i])->EnableWindow(bEnable);
  GetDlgItem(ui_names[i])->EnableWindow(bEnable);
  GetDlgItem(ui_cases[i])->EnableWindow(bEnable);
  GetDlgItem(ui_texts[i])->EnableWindow(bEnable);
  GetDlgItem(ui_where[i])->EnableWindow(bEnable);
}
