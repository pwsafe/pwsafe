/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include <bitset>

struct st_SaveAdvValues {
  st_SaveAdvValues()
  : subgroup_name(L""),
    subgroup_set(BST_UNCHECKED), subgroup_case(BST_UNCHECKED), 
    treatwhitespaceasempty(BST_CHECKED),
    subgroup_object(0), subgroup_function(0)
  {
    bsFields.set();
  }

  st_SaveAdvValues(const st_SaveAdvValues &adv)
    : bsFields(adv.bsFields), subgroup_name(adv.subgroup_name),
    subgroup_set(adv.subgroup_set), subgroup_object(adv.subgroup_object),
    subgroup_function(adv.subgroup_function), subgroup_case(adv.subgroup_case),
    treatwhitespaceasempty(adv.treatwhitespaceasempty)
  {
  }

  st_SaveAdvValues &operator =(const st_SaveAdvValues &adv)
  {
    if (this != &adv) {
      bsFields = adv.bsFields;
      subgroup_name = adv.subgroup_name;
      subgroup_set = adv.subgroup_set;
      subgroup_object = adv.subgroup_object;
      subgroup_function = adv.subgroup_function;
      subgroup_case = adv.subgroup_case;
      treatwhitespaceasempty = adv.treatwhitespaceasempty;
    }
    return *this;
  }

  void Clear() {
    bsFields.set();
    subgroup_set = subgroup_case = BST_UNCHECKED;
    treatwhitespaceasempty = BST_CHECKED;
    subgroup_object = subgroup_function = 0;
    subgroup_name = L"";
  }

  CItemData::FieldBits bsFields;
  CString subgroup_name;
  int subgroup_set, subgroup_object, subgroup_function, subgroup_case;
  int treatwhitespaceasempty;
};

class CAdvancedDlg : public CPWDialog
{
public:
  enum Type {ADV_INVALID = -1,
             ADV_COMPARE = 0,
             ADV_MERGE,
             ADV_SYNCHRONIZE,
             ADV_EXPORT_TEXT,
             ADV_EXPORT_ENTRYTEXT,
             ADV_EXPORT_XML,
             ADV_EXPORT_ENTRYXML,
             ADV_FIND,
             ADV_COMPARESYNCH,
             ADV_LAST};

  CAdvancedDlg(CWnd* pParent = NULL, Type iIndex = ADV_INVALID,
               st_SaveAdvValues *pst_SADV = NULL);   // standard constructor
  virtual CAdvancedDlg::~CAdvancedDlg();

  // Dialog Data
  enum { IDD_MERGE = IDD_ADVANCEDMERGE };
  //{{AFX_DATA(CADVANCEDDlg)
  enum { IDD = IDD_ADVANCED };
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function, m_subgroup_case;
  int m_treatwhitespaceasempty;

  //}}AFX_DATA

  CItemData::FieldBits m_bsFields;

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CADVANCEDDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  Type m_iIndex;
  static int dialog_lookup[ADV_LAST];

protected:
  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CADVANCEDDlg)
  afx_msg void OnSetSubGroup();
  afx_msg void OnSelectSome();
  afx_msg void OnSelectAll();
  afx_msg void OnDeselectSome();
  afx_msg void OnDeselectAll();
  afx_msg void OnHelp();
  virtual void OnOK();
  afx_msg void OnReset();
  afx_msg void OnSelectedItemChanging(NMHDR *pNMHDR, LRESULT *pResult);

  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  BOOL PreTranslateMessage(MSG* pMsg);

private:
  static int CALLBACK AdvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  void Set(CItemData::FieldBits bsFields);

  CListCtrl *m_pLC_List, *m_pLC_Selected;
  CToolTipCtrl* m_pToolTipCtrl;
  st_SaveAdvValues *m_pst_SADV;
  CItemData::FieldBits m_bsDefaultSelectedFields, m_bsAllowedFields, m_bsMandatoryFields;
};
