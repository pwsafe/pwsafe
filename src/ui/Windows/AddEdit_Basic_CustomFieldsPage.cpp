/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "StdAfx.h"
#include "PasswordSafe.h"

#include "AddEdit_Basic_CustomFieldsPage.h"
#include "AddEdit_PropertySheet.h"

#include "CustomFieldEditDlg.h"
#include "DboxMain.h"
#include "Fonts.h"
#include "resource3.h"

#include "core/CustomFields.h"

IMPLEMENT_DYNAMIC(CAddEdit_Basic_CustomFieldsPage, CAddEdit_Basic_SubPage)

CAddEdit_Basic_CustomFieldsPage::CAddEdit_Basic_CustomFieldsPage(CWnd *pParent,
                                                                 st_AE_master_data *pAEMD)
  : CAddEdit_Basic_SubPage(pParent, IDD, IDD_SHORT, pAEMD),
    m_rightClickedCustomFieldIndex(-1)
{
}

void CAddEdit_Basic_CustomFieldsPage::DoDataExchange(CDataExchange *pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_CUSTOMFIELDS_ADD, m_btnAdd);
  DDX_Control(pDX, IDC_CUSTOMFIELDS_EDIT, m_btnEdit);
  DDX_Control(pDX, IDC_CUSTOMFIELDS_DELETE, m_btnDelete);
  DDX_Control(pDX, IDC_CUSTOMFIELDS_LIST, m_customFieldsList);
}

BEGIN_MESSAGE_MAP(CAddEdit_Basic_CustomFieldsPage, CAddEdit_Basic_SubPage)
  ON_BN_CLICKED(IDC_CUSTOMFIELDS_ADD, OnCustomFieldsAdd)
  ON_BN_CLICKED(IDC_CUSTOMFIELDS_EDIT, OnCustomFieldsEdit)
  ON_BN_CLICKED(IDC_CUSTOMFIELDS_DELETE, OnCustomFieldsDelete)
  ON_COMMAND(IDC_CUSTOMFIELDS_TOGGLE_SENSITIVE, OnCustomFieldsToggleSensitive)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_CUSTOMFIELDS_LIST, OnCustomFieldsItemChanged)
  ON_NOTIFY(NM_RCLICK, IDC_CUSTOMFIELDS_LIST, OnNMRClickCustomFieldsList)
  ON_NOTIFY(NM_DBLCLK, IDC_CUSTOMFIELDS_LIST, OnNMDblclkCustomFieldsList)
  ON_NOTIFY(LVN_KEYDOWN, IDC_CUSTOMFIELDS_LIST, OnCustomFieldsKeyDown)
END_MESSAGE_MAP()

BOOL CAddEdit_Basic_CustomFieldsPage::OnInitDialog()
{
  CAddEdit_Basic_SubPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  m_customFieldsList.SetFont(Fonts::GetInstance()->GetAddEditFont());
  m_customFieldsList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

  CString cs_col;
  cs_col.LoadString(IDS_NAME);
  m_customFieldsList.InsertColumn(0, cs_col, LVCFMT_LEFT, 90);
  cs_col.LoadString(IDS_VALUE);
  m_customFieldsList.InsertColumn(1, cs_col, LVCFMT_LEFT, 160);
  LoadCustomFieldsFromList();

  const bool bReadOnly = (M_uicaller() == IDS_VIEWENTRY ||
                          (M_uicaller() == IDS_EDITENTRY && M_protected() != 0));

  m_btnAdd.EnableWindow(!bReadOnly);
  m_btnEdit.EnableWindow(FALSE);
  m_btnDelete.EnableWindow(FALSE);

  return TRUE;
}

void CAddEdit_Basic_CustomFieldsPage::LoadCustomFieldsFromList()
{
  m_customFieldsList.SetRedraw(FALSE);
  m_customFieldsList.DeleteAllItems();

  const CustomFieldList &fields = M_customfields();
  for (size_t i = 0; i < fields.size(); i++) {
    const CustomField &cf = fields[i];
    CString name(cf.GetName().c_str());
    CString value(cf.IsSensitive() ? L"********" : cf.GetValue().c_str());
    const int idx = m_customFieldsList.InsertItem(static_cast<int>(i), name);
    m_customFieldsList.SetItemText(idx, 1, value);
  }

  m_customFieldsList.SetRedraw(TRUE);
  UpdateCustomFieldButtons();
}

void CAddEdit_Basic_CustomFieldsPage::UpdateCustomFieldButtons()
{
  const BOOL bHasSelection = m_customFieldsList.GetNextItem(-1, LVNI_SELECTED) >= 0 ? TRUE : FALSE;
  const bool bReadOnly = (M_uicaller() == IDS_VIEWENTRY ||
                          (M_uicaller() == IDS_EDITENTRY && M_protected() != 0));

  // Edit/Delete only enabled when there is a selection and not in read-only mode.
  m_btnEdit.EnableWindow(bHasSelection && !bReadOnly);
  m_btnDelete.EnableWindow(bHasSelection && !bReadOnly);
  // Add is controlled by InitDialog based on read-only state but ensure it's
  // disabled when read-only as well.
  m_btnAdd.EnableWindow(!bReadOnly);
}

void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
  (void)pNMHDR;
  UpdateCustomFieldButtons();
  *pResult = 0;
}

void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsAdd()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  CCustomFieldEditDlg dlg(this, M_customfields());
  if (dlg.DoModal() != IDOK)
    return;

  CustomField cf;
  cf.SetName(StringX(dlg.m_name));
  cf.SetValue(StringX(dlg.m_value));
  cf.SetSensitive(dlg.m_sensitive == TRUE);

  M_customfields().push_back(cf);
  LoadCustomFieldsFromList();
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsEdit()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  const int sel = m_customFieldsList.GetNextItem(-1, LVNI_SELECTED);
  if (sel < 0)
    return;

  CustomFieldList &fields = M_customfields();
  if (sel >= static_cast<int>(fields.size()))
    return;

  CustomField &cf = fields[sel];
  CCustomFieldEditDlg dlg(this, fields, cf);
  if (dlg.DoModal() != IDOK)
    return;

  cf.SetName(StringX(dlg.m_name));
  cf.SetValue(StringX(dlg.m_value));
  cf.SetSensitive(dlg.m_sensitive == TRUE);
  LoadCustomFieldsFromList();
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsDelete()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  const int sel = m_customFieldsList.GetNextItem(-1, LVNI_SELECTED);
  if (sel < 0)
    return;

  CustomFieldList &fields = M_customfields();
  if (sel >= static_cast<int>(fields.size()))
    return;

  fields.erase(fields.begin() + sel);
  LoadCustomFieldsFromList();
  m_ae_psh->SetChanged(true);
}

void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsToggleSensitive()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;
  if (m_rightClickedCustomFieldIndex < 0)
    return;

  CustomFieldList &fields = M_customfields();
  if (m_rightClickedCustomFieldIndex >= static_cast<int>(fields.size()))
    return;

  CustomField &cf = fields[m_rightClickedCustomFieldIndex];
  cf.SetSensitive(!cf.IsSensitive());
  LoadCustomFieldsFromList();
  m_ae_psh->SetChanged(true);
  m_rightClickedCustomFieldIndex = -1;
}



void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

  // Handle Ctrl-C to copy the selected custom field value to clipboard.
  if (pLVKeyDown->wVKey == 'C' && (GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) {
    const int sel = m_customFieldsList.GetNextItem(-1, LVNI_SELECTED);
    if (sel >= 0) {
      const CustomFieldList &fields = M_customfields();
      if (sel < static_cast<int>(fields.size())) {
        GetMainDlg()->SetClipboardData(fields[sel].GetValue());
        GetMainDlg()->UpdateLastClipboardAction(ClipboardDataSource::CustomFieldValue);
      }
    }

    *pResult = 0; // handled
    return;
  }

  *pResult = 1; // not handled, allow default processing
}

void CAddEdit_Basic_CustomFieldsPage::OnNMRClickCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  (void)pNMHDR;

  CPoint pt;
  GetCursorPos(&pt);
  m_customFieldsList.ScreenToClient(&pt);

  LVHITTESTINFO hit = {};
  hit.pt = pt;
  const int item = m_customFieldsList.SubItemHitTest(&hit);
  if (item < 0 || hit.iSubItem != 1) {
    *pResult = 0;
    return;
  }

  m_rightClickedCustomFieldIndex = item;

  const CustomFieldList &fields = M_customfields();
  if (item >= static_cast<int>(fields.size())) {
    *pResult = 0;
    return;
  }

  CString menuText;
  menuText.LoadString(fields[item].IsSensitive() ? IDS_SHOW_VALUE : IDS_HIDE_VALUE);
  CMenu menu;
  menu.CreatePopupMenu();
  menu.AppendMenu(MF_STRING, IDC_CUSTOMFIELDS_TOGGLE_SENSITIVE, menuText);

  CPoint ptScreen;
  GetCursorPos(&ptScreen);
  menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ptScreen.x, ptScreen.y, this);
  *pResult = 0;
}

void CAddEdit_Basic_CustomFieldsPage::OnNMDblclkCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  (void)pNMHDR;
  OnCustomFieldsEdit();
  *pResult = 0;
}
