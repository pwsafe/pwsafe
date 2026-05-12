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

  afx_msg void OnCustomFieldsAdd();
  afx_msg void OnCustomFieldsDelete();
  afx_msg void OnCustomFieldsEdit();
  afx_msg void OnCustomFieldsItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomFieldsListClick(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomFieldsToggleSensitive();
  afx_msg void OnNMDblclkCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnNMRClickCustomFieldsList(NMHDR *pNMHDR, LRESULT *pResult);

  DECLARE_MESSAGE_MAP()

private:
  void LoadCustomFieldsFromList();
  void UpdateCustomFieldButtons();

  CButton m_btnAdd;
  CButton m_btnEdit;
  CButton m_btnDelete;
  CListCtrl m_customFieldsList;
  int m_rightClickedCustomFieldIndex;
};
