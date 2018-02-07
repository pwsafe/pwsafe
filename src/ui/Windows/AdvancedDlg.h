/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once
// AdvancedDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvancedDlg dialog

#include "PWDialog.h"
#include "core/ItemData.h"
#include "core/PWSfile.h"
#include "AdvancedValues.h"

class CAdvancedDlg : public CPWDialog
{
public:
  enum Type {INVALID = -1,
             FIND,
             COMPARESYNCH,
             LAST};

  CAdvancedDlg(CWnd* pParent = NULL, Type iIndex = INVALID,
               st_SaveAdvValues *pst_SADV = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CAdvancedDlg)
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function, m_subgroup_case;
  int m_treatwhitespaceasempty;
  //}}AFX_DATA

  CItemData::FieldBits m_bsFields;
  CItemAtt::AttFieldBits m_bsAttFields;

protected:
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAdvancedDlg)
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  //}}AFX_VIRTUAL

  Type m_iIndex;
  static int dialog_lookup[LAST];

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CAdvancedDlg)
  afx_msg void OnSetSubGroup();
  afx_msg void OnSelectSome();
  afx_msg void OnSelectAll();
  afx_msg void OnDeselectSome();
  afx_msg void OnDeselectAll();
  afx_msg void OnHelp();
  virtual void OnOK();
  afx_msg void OnReset();
  afx_msg void OnSelectedItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK AdvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  void Set(CItemData::FieldBits bsFields);
  void SetAtt(CItemAtt::AttFieldBits bsAttFields);

  CListCtrl *m_pLC_List, *m_pLC_Selected;
  st_SaveAdvValues *m_pst_SADV;
  CItemData::FieldBits m_bsDefaultSelectedFields, m_bsAllowedFields, m_bsMandatoryFields;
  CItemAtt::AttFieldBits m_bsAttDefaultSelectedFields, m_bsAttAllowedFields;

  PWSfile::VERSION m_current_version;
};
