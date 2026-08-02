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

BEGIN_MESSAGE_MAP(CCustomFieldsListCtrl, CListCtrl)
  ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

UINT CCustomFieldsListCtrl::OnGetDlgCode()
{
  return CListCtrl::OnGetDlgCode() | DLGC_WANTARROWS | DLGC_WANTCHARS;
}

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
  ON_COMMAND(IDC_CUSTOMFIELDS_COPY, OnCustomFieldsCopy)
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

  m_bInitdone = true;
  return TRUE;
}

void CAddEdit_Basic_CustomFieldsPage::LoadCustomFieldsFromList(int selectIndex)
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

  if (selectIndex >= 0 && selectIndex < static_cast<int>(fields.size()))
    SelectCustomFieldsListItem(selectIndex);

  m_customFieldsList.SetRedraw(TRUE);
  UpdateCustomFieldButtons();
}

void CAddEdit_Basic_CustomFieldsPage::SelectCustomFieldsListItem(int index)
{
  m_customFieldsList.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
  // A mouse click sets this as a side effect; programmatic SetItemState doesn't.
  // Keyboard navigation may use this anchor rather than (or in addition to) the
  // per-item LVIS_FOCUSED bit to determine "the current item".
  m_customFieldsList.SetSelectionMark(index);
  m_customFieldsList.EnsureVisible(index, FALSE);
}

void CAddEdit_Basic_CustomFieldsPage::FocusDefaultControl()
{
  if (m_customFieldsList.GetItemCount() > 0) {
    // Leave an existing selection alone; only seed one if the list has none yet.
    if (m_customFieldsList.GetNextItem(-1, LVNI_FOCUSED) < 0) {
      SelectCustomFieldsListItem(0);
      UpdateCustomFieldButtons();
    }
    m_customFieldsList.SetFocus();
  } else if (m_btnAdd.IsWindowEnabled()) {
    m_btnAdd.SetFocus();
  } else {
    // Read-only entry with no custom fields: nothing to add or select, but
    // still land keyboard focus somewhere within the tab.
    m_customFieldsList.SetFocus();
  }
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
  // Land on the newly-added row so a keyboard-only user (e.g. a screen reader
  // user) gets immediate confirmation of what was just added, and can go
  // straight into Edit/Delete without first having to find it in the list.
  LoadCustomFieldsFromList(static_cast<int>(M_customfields().size()) - 1);
  m_customFieldsList.SetFocus();
  NotifyChanged();
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
  // Keep the edited row selected so its new contents get re-announced/re-shown
  // in place, rather than losing the user's position in the list.
  LoadCustomFieldsFromList(sel);
  m_customFieldsList.SetFocus();
  NotifyChanged();
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

  // Select whatever now occupies the deleted row's position (the next item,
  // or the new last item if the deleted row was last), so a keyboard-only
  // user can keep deleting in place without re-navigating the list. If the
  // list is now empty, move on to Add instead - there's nothing left to
  // select.
  const int newCount = static_cast<int>(fields.size());
  const int nextSel = (newCount == 0) ? -1 : (sel < newCount ? sel : newCount - 1);
  LoadCustomFieldsFromList(nextSel);

  if (nextSel >= 0)
    m_customFieldsList.SetFocus();
  else if (m_btnAdd.IsWindowEnabled())
    m_btnAdd.SetFocus();

  NotifyChanged();
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
  NotifyChanged();
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

  // Row navigation: comctl32's own default handling for these keys does not
  // move the current item in this control, so drive it directly here.
  const int count = m_customFieldsList.GetItemCount();
  if (count > 0) {
    const int current = m_customFieldsList.GetNextItem(-1, LVNI_FOCUSED);
    const int page = m_customFieldsList.GetCountPerPage() > 0 ? m_customFieldsList.GetCountPerPage() : 1;
    int next = -1;

    switch (pLVKeyDown->wVKey) {
      case VK_UP:
        next = (current < 0) ? 0 : (current > 0 ? current - 1 : 0);
        break;
      case VK_DOWN:
        next = (current < 0) ? 0 : (current + 1 < count ? current + 1 : count - 1);
        break;
      case VK_HOME:
        next = 0;
        break;
      case VK_END:
        next = count - 1;
        break;
      case VK_PRIOR:
        next = (current < 0) ? 0 : (current - page > 0 ? current - page : 0);
        break;
      case VK_NEXT:
        next = (current < 0) ? 0 : (current + page < count ? current + page : count - 1);
        break;
      default:
        break;
    }

    if (next >= 0) {
      m_customFieldsList.SetItemState(-1, 0, LVIS_SELECTED | LVIS_FOCUSED);
      SelectCustomFieldsListItem(next);
    }
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

  const bool bReadOnly = (M_uicaller() == IDS_VIEWENTRY ||
                          (M_uicaller() == IDS_EDITENTRY && M_protected() != 0));

  CMenu menu;
  menu.CreatePopupMenu();

  // In r/w mode show Toggle Sensitive (Show/Hide) and Copy Value.
  // In r-o mode only show Copy Value.
  if (!bReadOnly) {
    CString menuText;
    menuText.LoadString(fields[item].IsSensitive() ? IDS_SHOW_VALUE : IDS_HIDE_VALUE);
    menu.AppendMenu(MF_STRING, IDC_CUSTOMFIELDS_TOGGLE_SENSITIVE, menuText);
  }

  CString copyText;
  copyText.LoadString(IDS_COPY_VALUE);
  menu.AppendMenu(MF_STRING, IDC_CUSTOMFIELDS_COPY, copyText);

  CPoint ptScreen;
  GetCursorPos(&ptScreen);
  menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ptScreen.x, ptScreen.y, this);
  *pResult = 0;
}

void CAddEdit_Basic_CustomFieldsPage::OnCustomFieldsCopy()
{
  if (m_rightClickedCustomFieldIndex < 0)
    return;

  const CustomFieldList &fields = M_customfields();
  if (m_rightClickedCustomFieldIndex >= static_cast<int>(fields.size()))
    return;

  GetMainDlg()->SetClipboardData(fields[m_rightClickedCustomFieldIndex].GetValue());
  GetMainDlg()->UpdateLastClipboardAction(ClipboardDataSource::CustomFieldValue);
  m_rightClickedCustomFieldIndex = -1;
}

void CAddEdit_Basic_CustomFieldsPage::OnNMDblclkCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  (void)pNMHDR;
  OnCustomFieldsEdit();
  *pResult = 0;
}
