/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// CWZAdvanced.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWZAdvanced PropertyPage

#include "WZPropertySheet.h"
#include "WZPropertyPage.h"

#include "AdvancedValues.h"

#include "core/ItemData.h"

#include "resource.h"

class CWZAdvanced : public CWZPropertyPage
{
public:
  DECLARE_DYNAMIC(CWZAdvanced)

  CWZAdvanced(CWnd *pParent, UINT nIDCaption, const int nType = -1, WZAdvanced::AdvType iIndex = WZAdvanced::INVALID,
    st_SaveAdvValues *pst_SADV = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CWZAdvanced)
  enum { IDD = IDD_WZADVANCED };

  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function, m_subgroup_case;
  int m_treatwhitespaceasempty;

  //}}AFX_DATA

  CItemData::FieldBits m_bsFields;

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CWZAdvanced)
protected:
  void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  LRESULT OnWizardNext();
  //}}AFX_VIRTUAL

  // Implementation
  WZAdvanced::AdvType m_iIndex;
  static int dialog_lookup[WZAdvanced::LAST];

  // Generated message map functions
  //{{AFX_MSG(CWZAdvanced)
  afx_msg void OnHelp();
  afx_msg void OnSetSubGroup();
  afx_msg void OnSelectSome();
  afx_msg void OnSelectAll();
  afx_msg void OnDeselectSome();
  afx_msg void OnDeselectAll();
  afx_msg void OnReset();
  afx_msg void OnSelectedItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK AdvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  void Set(CItemData::FieldBits bsFields);

  CListCtrl *m_pLC_List, *m_pLC_Selected;
  st_SaveAdvValues *m_pst_SADV;
  CItemData::FieldBits m_bsDefaultSelectedFields, m_bsAllowedFields, m_bsMandatoryFields;
  StringX m_sx_group;
};
