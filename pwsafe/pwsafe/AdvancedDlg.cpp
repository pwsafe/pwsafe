/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// ADVANCEDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "AdvancedDlg.h"
#include "corelib/ItemData.h"
#include "ThisMfcApp.h"
#include "resource.h"
#include "resource3.h"
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
             IDD_ADVANCED,      // ADV_EXPORT_XML
             IDD_ADVANCED       // ADV_FIND
};

/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg dialog

CAdvancedDlg::CAdvancedDlg(CWnd* pParent /* = NULL */, int iIndex,
                           CItemData::FieldBits bsFields, CString subgroup_name,
                           int subgroup_set, int subgroup_object, int subgroup_function)
  : CPWDialog(dialog_lookup[iIndex], pParent) , m_iIndex(iIndex), 
    m_bsFields(bsFields), m_subgroup_name(subgroup_name), 
    m_subgroup_set(subgroup_set), m_subgroup_object(subgroup_object),
    m_subgroup_function(subgroup_function), m_subgroup_case(BST_UNCHECKED),
    m_ToolTipCtrl(NULL)
{
  //{{AFX_DATA_INIT(CAdvancedDlg)
  //}}AFX_DATA_INIT

  if (m_subgroup_function < 0) {
    m_subgroup_case = BST_CHECKED;
    m_subgroup_function *= (-1);
  }

  if (m_subgroup_set == BST_UNCHECKED)
    m_subgroup_name = _T("*");

  if (m_bsFields.count() == 0)
    m_bsFields.set();
}

CAdvancedDlg::~CAdvancedDlg()
{
  if (m_iIndex != ADV_MERGE)
    delete m_ToolTipCtrl;
}

BOOL CAdvancedDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString cs_text;
  int iItem(-1), i;

  switch (m_iIndex) {
    case ADV_COMPARE:
      cs_text.LoadString(IDS_COMPAREX);
      break;
    case ADV_MERGE:
      cs_text.LoadString(IDS_MERGEX);
      break;
    case ADV_EXPORT_TEXT:
      cs_text.LoadString(IDS_EXPORT_TEXTX);
      break;
    case ADV_EXPORT_XML:
      cs_text.LoadString(IDS_EXPORT_XMLX);
      break;
    case ADV_FIND:
      cs_text.LoadString(IDS_FINDX);
      break;
    default:
      ASSERT(FALSE);
  }
  SetWindowText(cs_text);

  CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION);
  if (cboSubgroupFunction->GetCount () == 0) {
    cs_text.LoadString(IDS_EQUALS);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_EQUALS);
    cs_text.LoadString(IDS_DOESNOTEQUAL);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_NOTEQUAL);
    cs_text.LoadString(IDS_BEGINSWITH);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_BEGINS);
    cs_text.LoadString(IDS_DOESNOTBEGINSWITH);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_NOTBEGIN);
    cs_text.LoadString(IDS_ENDSWITH);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_ENDS);
    cs_text.LoadString(IDS_DOESNOTENDWITH);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_NOTEND);
    cs_text.LoadString(IDS_CONTAINS);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_CONTAINS);
    cs_text.LoadString(IDS_DOESNOTCONTAIN);
    iItem = cboSubgroupFunction->AddString(cs_text);
    cboSubgroupFunction->SetItemData(iItem, CItemData::SGF_NOTCONTAIN);
  }

  for (i = 0; i < cboSubgroupFunction->GetCount(); i++) {
    if ((int)cboSubgroupFunction->GetItemData(i) == m_subgroup_function) {
      cboSubgroupFunction->SetCurSel(i);
      break;
    }
  }

  CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT);
  if (cboSubgroupObject->GetCount () == 0) {
    cs_text.LoadString(IDS_GROUP);
    iItem = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(iItem, CItemData::SGO_GROUP);
    cs_text.LoadString(IDS_TITLE);
    iItem = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(iItem, CItemData::SGO_TITLE);
    cs_text.LoadString(IDS_USERNAME);
    iItem = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(iItem, CItemData::SGO_USER);
    cs_text.LoadString(IDS_GROUPTITLE);
    iItem = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(iItem, CItemData::SGO_GROUPTITLE);
    cs_text.LoadString(IDS_URL);
    iItem = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(iItem, CItemData::SGO_URL);
    cs_text.LoadString(IDS_NOTES);
    iItem = cboSubgroupObject->AddString(cs_text);
    cboSubgroupObject->SetItemData(iItem, CItemData::SGO_NOTES);
  }

  for (i = 0; i < cboSubgroupObject->GetCount(); i++) {
    if ((int)cboSubgroupObject->GetItemData(i) == m_subgroup_object) {
      cboSubgroupObject->SetCurSel(i);
      break;
    }
  }

  BOOL bEnable = (m_subgroup_set == BST_CHECKED) ? TRUE : FALSE;
  GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION)->EnableWindow(bEnable);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT)->EnableWindow(bEnable);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_NAME)->EnableWindow(bEnable);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_CASE)->EnableWindow(bEnable);

  if (m_iIndex == ADV_MERGE)
    return TRUE;

  m_pLC_List = (CListCtrl*)GetDlgItem(IDC_ADVANCED_LIST);
  m_pLC_Selected = (CListCtrl*)GetDlgItem(IDC_ADVANCED_SELECTED);

  m_pLC_List->InsertColumn(0, _T(""));
  m_pLC_Selected->InsertColumn(0, _T(""));
  m_pLC_List->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  m_pLC_Selected->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

  if (m_index == ADV_COMPARE) {
    cs_text.LoadString(IDS_CREATED);
    iItem = m_pLC_List->InsertItem(++iItem, cs_text);
    m_pLC_List->SetItemData(iItem, CItemData::CTIME);
    cs_text.LoadString(IDS_PASSWORDMODIFIED);
    iItem = m_pLC_List->InsertItem(++iItem, cs_text);
    m_pLC_List->SetItemData(iItem, CItemData::PMTIME);
    cs_text.LoadString(IDS_LASTACCESSED);
    iItem = m_pLC_List->InsertItem(++iItem, cs_text);
    m_pLC_List->SetItemData(iItem, CItemData::ATIME);
    cs_text.LoadString(IDS_LASTMODIFIED);
    iItem = m_pLC_List->InsertItem(++iItem, cs_text);
    m_pLC_List->SetItemData(iItem, CItemData::RMTIME);
  } else {
    if (m_index != ADV_FIND) {
      cs_text.LoadString(IDS_CREATED);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::CTIME);
      cs_text.LoadString(IDS_PASSWORDMODIFIED);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::PMTIME);
      cs_text.LoadString(IDS_LASTACCESSED);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::ATIME);
      cs_text.LoadString(IDS_LASTMODIFIED);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::RMTIME);
      cs_text.LoadString(IDS_PWPOLICY);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::POLICY);
    }
  }

  cs_text.LoadString(IDS_NOTES);
  iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
  m_pLC_Selected->SetItemData(iItem, CItemData::NOTES);
  cs_text.LoadString(IDS_URL);
  iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
  m_pLC_Selected->SetItemData(iItem, CItemData::URL);
  cs_text.LoadString(IDS_AUTOTYPE);
  iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
  m_pLC_Selected->SetItemData(iItem, CItemData::AUTOTYPE);
  cs_text.LoadString(IDS_EXPIRYDATETIME);
  if (m_index != ADV_FIND) {
    iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
    m_pLC_Selected->SetItemData(iItem, CItemData::LTIME);
  }
  cs_text.LoadString(IDS_PWHIST);
  iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
  m_pLC_Selected->SetItemData(iItem, CItemData::PWHIST);

  switch (m_iIndex) {
    case ADV_EXPORT_XML:
      cs_text.LoadString(IDS_GROUP);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::GROUP);
      cs_text.LoadString(IDS_ADVANCED_TITLETEXT); // <-- Special
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::TITLE);
      cs_text.LoadString(IDS_USERNAME);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::USER);
      cs_text.LoadString(IDS_ADVANCED_PASSWORDTEXT); // <-- Special
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::PASSWORD);
      break;
    case ADV_COMPARE:
      cs_text.LoadString(IDS_ADVANCED_GROUPTEXT); // <-- Special
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::GROUP);
      cs_text.LoadString(IDS_ADVANCED_TITLETEXT); // <-- Special
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::TITLE);
      cs_text.LoadString(IDS_ADVANCED_USERTEXT); // <-- Special
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::USER);
      cs_text.LoadString(IDS_PASSWORD);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::PASSWORD);
      break;
    default:
      cs_text.LoadString(IDS_GROUP);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::GROUP);
      cs_text.LoadString(IDS_TITLE);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::TITLE);
      cs_text.LoadString(IDS_USERNAME);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::USER);
      cs_text.LoadString(IDS_PASSWORD);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, CItemData::PASSWORD);
  }

  if (m_bsFields.count() != 0) {
    LVFINDINFO findinfo;
    CString cs_text;
    int iItem;
    DWORD_PTR dw_data;

    memset(&findinfo, 0, sizeof(findinfo));

    findinfo.flags = LVFI_PARAM;

    for (int i = 0; i < CItemData::LAST; i++) {
      // If selected, leave in the list of selected fields
      if (m_bsFields.test(i))
        continue;

      // Not selected - find entry in list of selected fields and move it
      findinfo.lParam = i;
      iItem = m_pLC_Selected->FindItem(&findinfo);
      if (iItem == -1)
        continue;

      cs_text = m_pLC_Selected->GetItemText(iItem, 0);
      dw_data = m_pLC_Selected->GetItemData(iItem);
      m_pLC_Selected->DeleteItem(iItem);
      iItem = m_pLC_List->InsertItem(0, cs_text);
      m_pLC_List->SetItemData(iItem, dw_data);
    }
  }

  m_pLC_List->SortItems(AdvCompareFunc, NULL);
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);

	// Tooltips
	EnableToolTips();

	m_ToolTipCtrl = new CToolTipCtrl;
	if (!m_ToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
		TRACE("Unable To create Advanced Dialog ToolTip\n");
		return TRUE;
	}

	// Activate the tooltip control.
	m_ToolTipCtrl->Activate(TRUE);
	m_ToolTipCtrl->SetMaxTipWidth(300);
	// Quadruple the time to allow reading by user
	int iTime = m_ToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
	m_ToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

	// Set the tooltip
	// Note naming convention: string IDS_xxx corresponds to control IDC_xxx
	CString cs_ToolTip;
	cs_ToolTip.LoadString(IDS_ADVANCED_SELECTSOME);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_SELECTSOME), cs_ToolTip);
	cs_ToolTip.LoadString(IDS_ADVANCED_SELECTALL);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_SELECTALL), cs_ToolTip);
	cs_ToolTip.LoadString(IDS_ADVANCED_DESELECTSOME);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_DESELECTSOME), cs_ToolTip);
	cs_ToolTip.LoadString(IDS_ADVANCED_DESELECTALL);
	m_ToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_DESELECTALL), cs_ToolTip);

  return TRUE;
}

void CAdvancedDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAdvancedDlg)
  DDX_Check(pDX, IDC_ADVANCED_SUBGROUP_SET, m_subgroup_set);
  DDX_Check(pDX, IDC_ADVANCED_SUBGROUP_CASE, m_subgroup_case);
  DDX_Text(pDX, IDC_ADVANCED_SUBGROUP_NAME, m_subgroup_name);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdvancedDlg, CPWDialog)
  //{{AFX_MSG_MAP(CAdvancedDlg)
  ON_BN_CLICKED(IDC_ADVANCED_SUBGROUP_SET, OnSetSubGroup)
  ON_BN_CLICKED(IDC_ADVANCED_SELECTSOME, OnSelectSome)
  ON_BN_CLICKED(IDC_ADVANCED_SELECTALL, OnSelectAll)
  ON_BN_CLICKED(IDC_ADVANCED_DESELECTSOME, OnDeselectSome)
  ON_BN_CLICKED(IDC_ADVANCED_DESELECTALL, OnDeselectAll)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_ADVANCED_SELECTED, OnSelectedItemchanging) 
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
    case ADV_FIND:
      cs_HelpTopic += _T("::/html/findx.html");
      break;
    default:
      ASSERT(FALSE);
  }
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CAdvancedDlg::OnOK()
{
  CString cs_text;
  DWORD_PTR dw_data;

  UpdateData();
  m_bsFields.reset();

  if (m_iIndex != ADV_MERGE) {
    int num_selected = m_pLC_Selected->GetItemCount();
    int nItem(-1);

    for (int i = 0; i < num_selected; i++) {
      nItem = m_pLC_Selected->GetNextItem(nItem, LVNI_ALL);
      dw_data = m_pLC_Selected->GetItemData(nItem);
      m_bsFields.set(dw_data, true);
    }

    if (m_bsFields.count() == 0) {
      CString cs_error_msg;
      switch (m_iIndex) {
        case ADV_COMPARE:
          cs_error_msg.LoadString(IDS_NOFIELDSFORCOMPARE);
          break;
        case ADV_EXPORT_TEXT:
          cs_error_msg.LoadString(IDS_NOFIELDSFOREXPORT);
          break;
        case ADV_FIND:
          cs_error_msg.LoadString(IDS_NOFIELDSFORSEARCH);
          break;
        default:
          ASSERT(FALSE);
      }
      AfxMessageBox(cs_error_msg);
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

    m_subgroup_object = int(((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT))->GetItemData(nObject));
    m_subgroup_function = int(((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION))->GetItemData(nFunction));
    if (m_subgroup_case == BST_CHECKED)
      m_subgroup_function *= (-1);
  }

  if (m_subgroup_name == _T("*"))
    m_subgroup_name.Empty();

  CPWDialog::OnOK();
}

void CAdvancedDlg::OnSetSubGroup()
{
  m_subgroup_set = ((CButton*)GetDlgItem(IDC_ADVANCED_SUBGROUP_SET))->GetCheck();

  BOOL bEnable = (m_subgroup_set == BST_CHECKED) ? TRUE : FALSE;
  GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION)->EnableWindow(bEnable);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT)->EnableWindow(bEnable);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_NAME)->EnableWindow(bEnable);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_CASE)->EnableWindow(bEnable);
}

void CAdvancedDlg::OnSelectSome()
{
  CString cs_text;
  int iItem;
  DWORD_PTR dw_data;

  int num_selected = m_pLC_List->GetItemCount();
  if (num_selected == 0)
    return;

  int nItem(-1);

  for (int i = 0; i < num_selected; i++) {
    nItem = m_pLC_List->GetNextItem(nItem, LVNI_SELECTED);
    if (nItem == -1)
      continue;
    cs_text = m_pLC_List->GetItemText(nItem, 0);
    dw_data = m_pLC_List->GetItemData(nItem);
    m_pLC_List->DeleteItem(nItem);
    iItem = m_pLC_Selected->InsertItem(0, cs_text);
    m_pLC_Selected->SetItemData(iItem, dw_data);
  }
  m_pLC_List->SortItems(AdvCompareFunc, NULL);
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);
}

void CAdvancedDlg::OnSelectAll()
{
  CString cs_text;
  int iItem;
  DWORD_PTR dw_data;

  int num_in_list = m_pLC_List->GetItemCount();
  if (num_in_list == 0)
    return;

  for (int i = 0; i < num_in_list; i++) {
    cs_text = m_pLC_List->GetItemText(i, 0);
    dw_data = m_pLC_List->GetItemData(i);
    iItem = m_pLC_Selected->InsertItem(0, cs_text);
    m_pLC_Selected->SetItemData(iItem, dw_data);
  }
  m_pLC_List->DeleteAllItems();
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);
}

void CAdvancedDlg::OnDeselectSome()
{
  CString cs_text;
  int iItem;
  DWORD_PTR dw_data;

  int num_selected = m_pLC_Selected->GetItemCount();
  if (num_selected == 0)
    return;

  int nItem(-1);

  for (int i = 0; i < num_selected; i++) {
    nItem = m_pLC_Selected->GetNextItem(nItem, LVNI_SELECTED);
    if (nItem == -1)
      continue;
    cs_text = m_pLC_Selected->GetItemText(nItem, 0);
    dw_data = m_pLC_Selected->GetItemData(nItem);
    m_pLC_Selected->DeleteItem(nItem);
    iItem = m_pLC_List->InsertItem(0, cs_text);
    m_pLC_List->SetItemData(iItem, dw_data);
  }
  m_pLC_List->SortItems(AdvCompareFunc, NULL);
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);
}

void CAdvancedDlg::OnDeselectAll()
{
  CString cs_text;
  int iItem;
  DWORD_PTR dw_data;

  int num_selected = m_pLC_Selected->GetItemCount();
  if (num_selected == 0)
    return;

  for (int i = num_selected - 1; i >= 0; i--) {
    cs_text = m_pLC_Selected->GetItemText(i, 0);
    dw_data = m_pLC_Selected->GetItemData(i);
    if (m_iIndex == ADV_EXPORT_XML &&
       (dw_data == CItemData::TITLE ||
        dw_data == CItemData::PASSWORD))
       continue;
    if (m_iIndex == ADV_COMPARE &&
       (dw_data == CItemData::GROUP ||
        dw_data == CItemData::TITLE ||
        dw_data == CItemData::USER ))
       continue;
    iItem = m_pLC_List->InsertItem(0, cs_text);
    m_pLC_List->SetItemData(iItem, dw_data);
    m_pLC_Selected->DeleteItem(i);
  }
  m_pLC_List->SortItems(AdvCompareFunc, NULL);
}

void
CAdvancedDlg::OnSelectedItemchanging(NMHDR * pNMHDR, LRESULT * pResult)
{
  // Prevent TITLE and PASSWORD being deselected in EXPORT_XML function
  // as they are mandatory fields
  // Prevent group, TITLE and USER being deselected in COMPARE function
  // as they are always checked
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR; 
  *pResult = FALSE;

  switch (m_iIndex) {
    case ADV_COMPARE:
      if ((pNMListView->lParam == CItemData::GROUP ||
           pNMListView->lParam == CItemData::TITLE ||
           pNMListView->lParam == CItemData::USER) &&
          (pNMListView->uNewState & LVIS_SELECTED)) {
        pNMListView->uNewState &= ~LVIS_SELECTED;
        *pResult = TRUE;
      }
      break;
    case ADV_EXPORT_XML:
      if ((pNMListView->lParam == CItemData::TITLE ||
           pNMListView->lParam == CItemData::PASSWORD) &&
          (pNMListView->uNewState & LVIS_SELECTED)) {
        pNMListView->uNewState &= ~LVIS_SELECTED;
        *pResult = TRUE;
      }
      break;
    case ADV_FIND:
    case ADV_MERGE:
    case ADV_EXPORT_TEXT:
      break;
    default:
      ASSERT(0);

  }
}

// Override PreTranslateMessage() so RelayEvent() can be
// called to pass a mouse message to CPWSOptions's
// tooltip control for processing.
BOOL CAdvancedDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_ToolTipCtrl != NULL)
		m_ToolTipCtrl->RelayEvent(pMsg);

	return CPWDialog::PreTranslateMessage(pMsg);
}

int CALLBACK CAdvancedDlg::AdvCompareFunc(LPARAM lParam1, LPARAM lParam2,
										LPARAM /* closure */)
{
  return (int)(lParam1 - lParam2);
}
