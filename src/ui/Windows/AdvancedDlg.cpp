/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "AdvancedDlg.h"

#include "core/ItemData.h"

#include "resource.h"
#include "resource3.h"

#include <bitset>
#include <type_traits> // for static_assert

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NORMALFIELD    0x1000
#define MANDATORYFIELD 0x0800

int CAdvancedDlg::dialog_lookup[LAST] = {
  IDD_ADVANCED,          // FIND
  IDD_ADVANCEDCOMPSYNCH  // Reduced for synchronizing only one entry
};
/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg dialog

CAdvancedDlg::CAdvancedDlg(CWnd* pParent /*=NULL*/, Type iIndex /*=INVALID*/,
                           st_SaveAdvValues *pst_SADV)
  : CPWDialog(dialog_lookup[iIndex], pParent), m_iIndex(iIndex), m_pst_SADV(pst_SADV),
  m_treatwhitespaceasempty(BST_CHECKED)
{
  if (m_pst_SADV != NULL) {
    m_bsFields = m_pst_SADV->bsFields;
    m_bsAttFields = m_pst_SADV->bsAttFields;
    m_subgroup_name = m_pst_SADV->subgroup_name.c_str();
    m_subgroup_set = m_pst_SADV->subgroup_bset ? BST_CHECKED : BST_UNCHECKED;
    m_subgroup_object = m_pst_SADV->subgroup_object;
    m_subgroup_function = m_pst_SADV->subgroup_function;
    m_subgroup_case = m_pst_SADV->subgroup_bcase ? BST_CHECKED : BST_UNCHECKED;
    m_treatwhitespaceasempty = m_pst_SADV->btreatwhitespaceasempty ? BST_CHECKED : BST_UNCHECKED;
  } else {
    m_bsFields.set();
    m_bsAttFields.reset();
    m_subgroup_name = L"";
    m_subgroup_set = BST_UNCHECKED;
    m_treatwhitespaceasempty = BST_CHECKED;
    m_subgroup_object =  0;
    m_subgroup_function = 0;
    m_subgroup_case = BST_UNCHECKED;
  }

  if (m_subgroup_function < 0) {
    m_subgroup_case = BST_CHECKED;
    m_subgroup_function *= (-1);
  }

  if (m_subgroup_set == BST_UNCHECKED)
    m_subgroup_name = L"*";

  if (m_bsFields.count() == 0 && m_bsAttFields.count() == 0) {
    m_bsFields.set();
    m_bsAttFields.reset();
  }

  m_current_version = app.GetCore()->GetReadFileVersion();
}

void CAdvancedDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAdvancedDlg)
  if (m_iIndex == FIND) {
    DDX_Check(pDX, IDC_ADVANCED_SUBGROUP_SET, m_subgroup_set);
    DDX_Check(pDX, IDC_ADVANCED_SUBGROUP_CASE, m_subgroup_case);
    DDX_Text(pDX, IDC_ADVANCED_SUBGROUP_NAME, m_subgroup_name);
  }
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdvancedDlg, CPWDialog)
  //{{AFX_MSG_MAP(CAdvancedDlg)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_ADVANCED_SUBGROUP_SET, OnSetSubGroup)
  ON_BN_CLICKED(IDC_ADVANCED_SELECTSOME, OnSelectSome)
  ON_BN_CLICKED(IDC_ADVANCED_SELECTALL, OnSelectAll)
  ON_BN_CLICKED(IDC_ADVANCED_DESELECTSOME, OnDeselectSome)
  ON_BN_CLICKED(IDC_ADVANCED_DESELECTALL, OnDeselectAll)
  ON_BN_CLICKED(IDC_ADVANCED_RESET, OnReset)

  ON_NOTIFY(LVN_ITEMCHANGING, IDC_ADVANCED_SELECTED, OnSelectedItemChanging)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAdvancedDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString cs_text, cs_tmp;
  int iItem(-1);

  m_bsAllowedFields.reset();
  m_bsDefaultSelectedFields.reset();
  m_bsMandatoryFields.reset();

  m_bsAttAllowedFields.reset();
  m_bsAttDefaultSelectedFields.reset();

  cs_text.LoadString(m_iIndex == FIND ? IDS_FINDX : IDS_SYNCHRONIZEX);

  SetWindowText(cs_text);

  if (m_iIndex != COMPARESYNCH) {
    CComboBox *cboSubgroupFunction = (CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION);
    if (cboSubgroupFunction->GetCount() == 0) {
      const PWSMatch::MatchRule mrx[] = {PWSMatch::MR_EQUALS,   PWSMatch::MR_NOTEQUAL,
                                         PWSMatch::MR_BEGINS,   PWSMatch::MR_NOTBEGIN,
                                         PWSMatch::MR_ENDS,     PWSMatch::MR_NOTEND,
                                         PWSMatch::MR_CONTAINS, PWSMatch::MR_NOTCONTAIN,
                                         PWSMatch::MR_CNTNANY,  PWSMatch::MR_NOTCNTNANY,
                                         PWSMatch::MR_CNTNALL,  PWSMatch::MR_NOTCNTNALL};

      for (size_t i = 0; i < _countof(mrx); i++) {
        UINT iumsg = PWSMatch::GetRule(mrx[i]);
        cs_text.LoadString(iumsg);
        iItem = cboSubgroupFunction->AddString(cs_text);
        cboSubgroupFunction->SetItemData(iItem, mrx[i]);
      }
    }

    for (int i = 0; i < cboSubgroupFunction->GetCount(); i++) {
      if ((int)cboSubgroupFunction->GetItemData(i) == m_subgroup_function) {
        cboSubgroupFunction->SetCurSel(i);
        break;
      }
    }

    // Note: NOT SORTED by design
    CComboBox *cboSubgroupObject = (CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT);
    if (cboSubgroupObject->GetCount () == 0) {
      const struct {int si; int ii;} subgroupInit[] = {
        {IDS_GROUP, CItemData::GROUP},
        {IDS_GROUPTITLE, CItemData::GROUPTITLE},
        {IDS_TITLE, CItemData::TITLE},
        {IDS_USERNAME, CItemData::USER},
        {IDS_URL, CItemData::URL},
        {IDS_NOTES, CItemData::NOTES},
      };

      for (auto &elem : subgroupInit) {
        cs_text.LoadString(elem.si);
        iItem = cboSubgroupObject->AddString(cs_text);
        cboSubgroupObject->SetItemData(iItem, elem.ii);
      }
    }
  
    for (int i = 0; i < cboSubgroupObject->GetCount(); i++) {
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
  }

  // m_pLC_List are those fields that aren't currently selected but could be
  // m_pLC_Selected are those fields already selected
  m_pLC_List = (CListCtrl *)GetDlgItem(IDC_ADVANCED_LIST);
  m_pLC_Selected = (CListCtrl *)GetDlgItem(IDC_ADVANCED_SELECTED);

  m_pLC_List->InsertColumn(0, L"");
  m_pLC_Selected->InsertColumn(0, L"");
  m_pLC_List->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  m_pLC_Selected->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

  // Deal with non-text fields
  if (m_iIndex == COMPARESYNCH) {
    // All these are already selected fields
    const struct {int si; int ii; bool is_temporal;} nonTextInit[] = {
      {IDS_COMPCTIME, CItemData::CTIME, true},
      {IDS_COMPPMTIME, CItemData::PMTIME, true},
      {IDS_COMPATIME, CItemData::ATIME, true},
      {IDS_COMPRMTIME, CItemData::RMTIME, true},
      {IDS_COMPXTIME, CItemData::XTIME, true},
      {IDS_PASSWORDEXPIRYDATEINT, CItemData::XTIME_INT, false},
      {IDS_PWPOLICY, CItemData::POLICY, false},
      {IDS_POLICYNAME, CItemData::POLICYNAME, false},
      {IDS_DCALONG, CItemData::DCA, false},
      {IDS_SHIFTDCALONG, CItemData::SHIFTDCA, false},
      {IDS_PROTECTED, CItemData::PROTECTED, false},
      {IDS_KBSHORTCUT, CItemData::KBSHORTCUT, false},
    };

    for (auto &elem : nonTextInit) {
      cs_text.LoadString(elem.si);
      if (elem.is_temporal) { // Time fields
        // Use Compare text and remove the quotes and leading blank
        cs_text = cs_text.Mid(2, cs_text.GetLength() - 3);
      }
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, elem.ii | NORMALFIELD);
      m_bsAllowedFields.set(elem.ii);
      m_bsDefaultSelectedFields.set(elem.ii);
    }
  } // non-text fields

  // Deal with text fields - all selected by default
  const struct {int si; int ii;} textInit[] = {
    {IDS_NOTES, CItemData::NOTES},
    {IDS_URL, CItemData::URL},
    {IDS_AUTOTYPE, CItemData::AUTOTYPE},
    {IDS_PWHISTORY, CItemData::PWHIST},
    {IDS_RUNCOMMAND, CItemData::RUNCMD},
    {IDS_EMAIL, CItemData::EMAIL},
    {IDS_SYMBOLS, CItemData::SYMBOLS},
  };

  for (auto &elem : textInit) {
    cs_text.LoadString(elem.si);
    iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
    m_pLC_Selected->SetItemData(iItem, elem.ii | NORMALFIELD);
    m_bsAllowedFields.set(elem.ii);
    m_bsDefaultSelectedFields.set(elem.ii);
  }

  // Deal with standard text fields - selected by default
  if (m_iIndex == COMPARESYNCH) {
    cs_text.LoadString(IDS_PASSWORD);
    iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
    m_pLC_Selected->SetItemData(iItem, CItemData::PASSWORD | NORMALFIELD);
    m_bsAllowedFields.set(CItemData::PASSWORD);
    m_bsDefaultSelectedFields.set(CItemData::PASSWORD);
  } else {
    const struct {int si; int ii;} stdtextInit[] = {
      {IDS_GROUP, CItemData::GROUP},
      {IDS_TITLE, CItemData::TITLE},
      {IDS_USERNAME, CItemData::USER},
      {IDS_PASSWORD, CItemData::PASSWORD},
    };

    for (auto &elem : stdtextInit) {
      cs_text.LoadString(elem.si);
      iItem = m_pLC_Selected->InsertItem(++iItem, cs_text);
      m_pLC_Selected->SetItemData(iItem, elem.ii | NORMALFIELD);
      m_bsAllowedFields.set(elem.ii);
      m_bsDefaultSelectedFields.set(elem.ii);
    }
  }

  m_pLC_List->SortItems(AdvCompareFunc, NULL);
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);

  if (m_iIndex == FIND) {
    GetDlgItem(IDC_TREATWHITESPACEASEMPTY)->EnableWindow(FALSE);
    GetDlgItem(IDC_TREATWHITESPACEASEMPTY)->ShowWindow(SW_HIDE);

    // Only add attachment fields for V4 and later
    if (m_current_version >= PWSfile::V40) {
      const CString cs_att = L" (" + CString(MAKEINTRESOURCE(IDS_ATTACHMENTS)) + L")";
      const struct {int si; int ii;} attInit[] = {
        {IDS_FILETITLE, CItemAtt::TITLE},
        {IDS_FILENAME, CItemAtt::FILENAME},
        {IDS_FILEPATH, CItemAtt::FILEPATH},
        {IDS_FILEMEDIATYPE, CItemAtt::MEDIATYPE},
      };

      for (auto &elem : attInit) {
        cs_text.LoadString(elem.si);
        cs_text += cs_att;
        iItem = m_pLC_List->InsertItem(++iItem, cs_text);
        m_pLC_List->SetItemData(iItem, elem.ii | NORMALFIELD);
        m_bsAttAllowedFields.set(elem.ii - CItemAtt::START);
      }
    } else {
      // Don't allow any
      m_bsAttAllowedFields.reset();
    }
  }

  if (m_bsFields.count() != 0 && m_bsFields.count() != m_bsFields.size() &&
      m_bsAttFields.count() != 0) {
    Set(m_bsFields);
  }

  if (m_bsAttFields.count() != 0 && m_bsAttFields.count() != m_bsAttFields.size()) {
    SetAtt(m_bsAttFields);
  }

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create Advanced Dialog ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return TRUE;  // return TRUE unless you set the focus to a control
  }

  // Tooltips
  EnableToolTips();

  // Activate the tooltip control.
  m_pToolTipCtrl->Activate(TRUE);
  m_pToolTipCtrl->SetMaxTipWidth(300);
  // Quadruple the time to allow reading by user
  int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
  m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 4 * iTime);

  // Set the tooltip
  // Note naming convention: string IDS_xxx corresponds to control IDC_xxx
  CString cs_ToolTip;
  cs_ToolTip.LoadString(IDS_ADVANCED_SELECTSOME);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_SELECTSOME), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_ADVANCED_SELECTALL);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_SELECTALL), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_ADVANCED_DESELECTSOME);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_DESELECTSOME), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_ADVANCED_DESELECTALL);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_ADVANCED_DESELECTALL), cs_ToolTip);

  return TRUE;  // return TRUE unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg message handlers

void CAdvancedDlg::OnHelp()
{
  ShowHelp(L"::/html/advanced.html");
}

void CAdvancedDlg::OnReset()
{
  Set(m_bsDefaultSelectedFields);
  SetAtt(m_bsAttDefaultSelectedFields);

  m_pLC_List->SortItems(AdvCompareFunc, NULL);
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);
  m_pLC_List->Invalidate();
  m_pLC_Selected->Invalidate();

  m_subgroup_name = L"";
  m_subgroup_set = m_subgroup_case = BST_UNCHECKED;
  m_treatwhitespaceasempty = BST_CHECKED;
  m_subgroup_object =  m_subgroup_function = 0;

  ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT))->SetCurSel(1);  // Group/Title
  ((CComboBox *)GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION))->SetCurSel(-1);

  GetDlgItem(IDC_ADVANCED_SUBGROUP_FUNCTION)->EnableWindow(FALSE);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_OBJECT)->EnableWindow(FALSE);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_NAME)->EnableWindow(FALSE);
  GetDlgItem(IDC_ADVANCED_SUBGROUP_CASE)->EnableWindow(FALSE);

  UpdateData(FALSE);
}

void CAdvancedDlg::Set(CItemData::FieldBits bsFields)
{
  LVFINDINFO findinfo;
  CString cs_text;
  int iItem;
  DWORD_PTR dw_data;

  SecureZeroMemory(&findinfo, sizeof(LVFINDINFO));

  findinfo.flags = LVFI_PARAM;
  // Note: Mandatory fields have a ItemData value + 0x800 rather than 0x1000
  // and so will not be found and so not moved anywhere.
  for (int i = 0; i < CItem::LAST_DATA; i++) {
    // Don't move or allow non-allowed fields
    if (!m_bsAllowedFields.test(i))
      continue;

    if (bsFields.test(i)) {
      // Selected - find entry in list of available fields and move it
      findinfo.lParam = i | NORMALFIELD;
      iItem = m_pLC_List->FindItem(&findinfo);
      if (iItem == -1)
        continue;

      cs_text = m_pLC_List->GetItemText(iItem, 0);
      dw_data = m_pLC_List->GetItemData(iItem);
      m_pLC_List->DeleteItem(iItem);
      iItem = m_pLC_Selected->InsertItem(0, cs_text);
      m_pLC_Selected->SetItemData(iItem, dw_data);
    } else {
      // Not selected - find entry in list of selected fields and move it
      findinfo.lParam = i | NORMALFIELD;
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
}

void CAdvancedDlg::SetAtt(CItemAtt::AttFieldBits bsAttFields)
{
  LVFINDINFO findinfo;
  CString cs_text;
  int iItem;
  DWORD_PTR dw_data;

  SecureZeroMemory(&findinfo, sizeof(LVFINDINFO));

  findinfo.flags = LVFI_PARAM;
  // Note: Mandatory fields have a ItemData value + 0x800 rather than 0x1000
  // and so will not be found and so not moved anywhere.

  // Following check should compile, but silly MSVC thinks bitset<>::size() is runtime
  // static_assert((m_bsAttAllowedFields.size() == bsAttFields.size()), "bitset size mismatch");

  for (size_t i = 0; i < m_bsAttAllowedFields.size(); i++) {
    // Don't move or allow non-allowed fields
    if (!m_bsAttAllowedFields.test(i))
      continue;

    if (bsAttFields.test(i)) {
      // Selected - find entry in list of available fields and move it
      findinfo.lParam = (CItemAtt::START + i) | NORMALFIELD;
      iItem = m_pLC_List->FindItem(&findinfo);
      if (iItem == -1)
        continue;

      cs_text = m_pLC_List->GetItemText(iItem, 0);
      dw_data = m_pLC_List->GetItemData(iItem);
      m_pLC_List->DeleteItem(iItem);
      iItem = m_pLC_Selected->InsertItem(0, cs_text);
      m_pLC_Selected->SetItemData(iItem, dw_data);
    } else {
      // Not selected - find entry in list of selected fields and move it
      findinfo.lParam = (CItemAtt::START + i) | NORMALFIELD;
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
}

void CAdvancedDlg::OnOK()
{
  CGeneralMsgBox gmb;
  CString cs_text;

  UpdateData();
  m_bsFields.reset();
  m_bsAttFields.reset();

  int num_selected = m_pLC_Selected->GetItemCount();
  int nItem(-1);

  for (int i = 0; i < num_selected; i++) {
    nItem = m_pLC_Selected->GetNextItem(nItem, LVNI_ALL);
    DWORD_PTR dw_data = LOWORD(m_pLC_Selected->GetItemData(nItem));

    const short index = dw_data & 0xff;

    if (index < CItem::LAST_DATA) {
      m_bsFields.set(index, true);
    }
    else if (index < CItem::LAST_ATT) {
      m_bsAttFields.set(index - CItemAtt::START, true);
    }
  }

  if (m_bsFields.count() == 0 && m_bsAttFields.count() == 0) {
    CString cs_error_msg;
    cs_error_msg.LoadString(m_iIndex == FIND ? 
                    IDS_NOFIELDSFORSEARCH : IDS_NOFIELDSFORSYNCH);

    gmb.AfxMessageBox(cs_error_msg);
    m_bsFields.set();  // note: impossible to set them all even via the advanced dialog
    return;
  }

  if (m_subgroup_name == L"*")
    m_subgroup_name.Empty();

  m_pst_SADV->bsFields = m_bsFields;
  m_pst_SADV->bsAttFields = m_bsAttFields;
  m_pst_SADV->subgroup_name = m_subgroup_name;
  m_pst_SADV->subgroup_bset = m_subgroup_set == BST_CHECKED;
  m_pst_SADV->subgroup_object = m_subgroup_object;
  m_pst_SADV->subgroup_function = m_subgroup_function;
  m_pst_SADV->subgroup_bcase = m_subgroup_case == BST_CHECKED;
  m_pst_SADV->btreatwhitespaceasempty = m_treatwhitespaceasempty == BST_CHECKED;

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
  int num_in_list = m_pLC_List->GetItemCount();
  if (num_in_list == 0)
    return;

  for (int i = 0; i < num_in_list; i++) {
    CString cs_text = m_pLC_List->GetItemText(i, 0);
    DWORD_PTR dw_data = m_pLC_List->GetItemData(i);
    int iItem = m_pLC_Selected->InsertItem(0, cs_text);
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

    const short index = dw_data & 0xff;

    if (index < CItem::LAST_DATA) {
      // Ignore mandatory fields - can't be deselected
      if (m_bsMandatoryFields.test(index))
        continue;
    }

    m_pLC_Selected->DeleteItem(nItem);
    iItem = m_pLC_List->InsertItem(0, cs_text);
    m_pLC_List->SetItemData(iItem, dw_data);
  }

  m_pLC_List->SortItems(AdvCompareFunc, NULL);
  m_pLC_Selected->SortItems(AdvCompareFunc, NULL);
}

void CAdvancedDlg::OnDeselectAll()
{
  int num_selected = m_pLC_Selected->GetItemCount();
  if (num_selected == 0)
    return;

  for (int i = num_selected - 1; i >= 0; i--) {
    CString cs_text = m_pLC_Selected->GetItemText(i, 0);
    DWORD_PTR dw_data = m_pLC_Selected->GetItemData(i);

    const short index = dw_data & 0xff;

    if (index < CItem::LAST_DATA) {
      // Ignore mandatory fields - can't be deselected
      if (m_bsMandatoryFields.test(dw_data & 0xff))
        continue;
    }

    int iItem = m_pLC_List->InsertItem(0, cs_text);
    m_pLC_List->SetItemData(iItem, dw_data);
    m_pLC_Selected->DeleteItem(i);
  }

  m_pLC_List->SortItems(AdvCompareFunc, NULL);
}

void CAdvancedDlg::OnSelectedItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // Prevent mandatory fields being deselected
  NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNotifyStruct;

  *pLResult = FALSE;

  const short index = pNMListView->lParam & 0xff;

  if (index < CItem::LAST_DATA) {
    if (m_bsMandatoryFields.test(index) &&
      (pNMListView->uNewState & LVIS_SELECTED)) {
      pNMListView->uNewState &= ~LVIS_SELECTED;
      *pLResult = TRUE;
    }
  }
  else if (index < CItem::LAST_ATT) {
    if (m_bsMandatoryFields.test(index - CItem::START_ATT) &&
      (pNMListView->uNewState & LVIS_SELECTED)) {
      pNMListView->uNewState &= ~LVIS_SELECTED;
      *pLResult = TRUE;
    }
  }
}

// Override PreTranslateMessage() so RelayEvent() can be
// called to pass a mouse message to CPWSOptions's
// tooltip control for processing.
BOOL CAdvancedDlg::PreTranslateMessage(MSG *pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  return CPWDialog::PreTranslateMessage(pMsg);
}

int CALLBACK CAdvancedDlg::AdvCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                          LPARAM /* closure */)
{
  return (int)(lParam1 - lParam2);
}
