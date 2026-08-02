/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "resource.h"

#include "AddEdit_Basic_SubPage.h"

// A plain CListCtrl doesn't claim WM_GETDLGCODE's DLGC_WANTARROWS, so a dialog
// manager hosting it (IsDialogMessage) treats Up/Down as dialog-navigation keys
// instead of passing them through for the list's own row navigation.
class CCustomFieldsListCtrl : public CListCtrl
{
protected:
  afx_msg UINT OnGetDlgCode();

  DECLARE_MESSAGE_MAP()
};

class CAddEdit_Basic_CustomFieldsPage : public CAddEdit_Basic_SubPage
{
public:
  DECLARE_DYNAMIC(CAddEdit_Basic_CustomFieldsPage)

  CAddEdit_Basic_CustomFieldsPage(CWnd *pParent, st_AE_master_data *pAEMD);

  enum {
    IDD = IDD_ADDEDIT_BASIC_CUSTOMFIELDS,
    IDD_SHORT = IDD_ADDEDIT_BASIC_CUSTOMFIELDS_SHORT
  };

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);

  // Alt+C (the tab's mnemonic) lands here: focus the list on its current/first
  // row when there's data to act on, or the Add button when the list is empty
  // (Edit/Delete would have nothing to do), so keyboard-only users always land
  // somewhere useful.
  virtual void FocusDefaultControl() override;

  afx_msg void OnCustomFieldsAdd();
  afx_msg void OnCustomFieldsDelete();
  afx_msg void OnCustomFieldsEdit();
  afx_msg void OnCustomFieldsItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomFieldsToggleSensitive();
  afx_msg void OnNMDblclkCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnNMRClickCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomFieldsKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomFieldsCopy();

  DECLARE_MESSAGE_MAP()

private:
  // selectIndex, when >= 0, is selected and focused (visually and for the
  // list's own keyboard-focus rect) after the list is repopulated, so an
  // Add/Edit/Delete doesn't strand a keyboard-only user without feedback
  // about which row they just acted on.
  void LoadCustomFieldsFromList(int selectIndex = -1);
  void UpdateCustomFieldButtons();
  void SelectCustomFieldsListItem(int index);

  CButton m_btnAdd;
  CButton m_btnEdit;
  CButton m_btnDelete;
  CCustomFieldsListCtrl m_customFieldsList;
  int m_rightClickedCustomFieldIndex;
};
