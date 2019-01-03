/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// ManageFiltersDlg dialog

#include "../PWDialog.h"
#include "../PWHdrCtrlNoChng.h"
#include "core/PWSFilters.h"

#include <vector>

// Subitem indices for list of filters
enum {
  MFLC_FILTER_NAME = 0,
  MFLC_FILTER_SOURCE,
  MFLC_INUSE,
  MFLC_COPYTODATABASE,
  MFLC_EXPORT,
  MFLC_NUM_COLUMNS
};

// Subitem indices for filter properties
enum {
  MFPRP_FILTER_NUMBER = 0,
  MFPRP_FILTER_ACTIVE,
  MFPRP_AND_OR,
  MFPRP_FIELD,
  MFPRP_CRITERIA_TEXT,
  MFPRP_NUM_COLUMNS
};

// Filter Flags
enum {MFLT_SELECTED           = 0x8000,
      MFLT_REQUEST_COPY_TO_DB = 0x4000,
      MFLT_REQUEST_EXPORT     = 0x2000,
      MFLT_INUSE              = 0x1000,
      MFLT_UNUSED             = 0x0fff};

// Structure pointed to be CListCtrl item data field
struct st_FilterItemData {
  st_Filterkey flt_key;
  UINT flt_flags;
};

class CManageFiltersDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CManageFiltersDlg)

public:
  CManageFiltersDlg(CWnd* pParent,
                 bool bFilterActive,
                 PWSFilters &pmapFilters,
                 bool bCanHaveAttachments);
  virtual ~CManageFiltersDlg();
  void SetCurrentData(FilterPool activefilterpool, CString activefiltername)
  {m_activefilterpool = activefilterpool;
   m_activefiltername = activefiltername;}
  bool HasDBFiltersChanged() {return m_bDBFiltersChanged;}

// Dialog Data
  enum { IDD = IDD_MANAGEFILTERS };

  enum CheckImage { CHECKED = 0, CHECKED_DISABLED, EMPTY, EMPTY_DISABLED };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  //{{AFX_MSG(CManageFiltersDlg)
  afx_msg void OnFilterNew();
  afx_msg void OnFilterEdit();
  afx_msg void OnFilterCopy();
  afx_msg void OnFilterDelete();
  afx_msg void OnFilterImport();
  afx_msg void OnFilterExport();
  afx_msg void OnDestroy();
  afx_msg void OnClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHelp();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  // This dialog's filter map - so as not to confuse with DboxMain and PWScore
  PWSFilters &m_MapMFDFilters;
  std::vector<st_Filterkey> m_vcs_filters;

  CString GetFieldTypeName(FieldType ft);
  void SetFilter();
  void ClearFilter();
  void DisplayFilterProperties(st_filters *pfilter);
  void UpdateFilterList();
  void ResetColumns();
  void DrawImage(CDC *pDC, CRect &rect, CheckImage nImage);
  void SortFilterView();
  static int CALLBACK FLTCompareFunc(LPARAM lParam1, LPARAM lParam2, 
                                     LPARAM pSelf);
  static CString GetFilterPoolName(FilterPool fp);

  CListCtrl m_FilterLC, m_FilterProperties;
  CImageList *m_pImageList, *m_pCheckImageList;
  CPWHdrCtrlNoChng m_FLCHeader;
  CPWHdrCtrlNoChng m_FPROPHeader;

  FilterPool m_selectedfilterpool, m_activefilterpool;
  CString m_selectedfiltername, m_activefiltername;
  int m_selectedfilter, m_activefilter;
  int m_num_to_export, m_num_to_copy;
  bool m_bMFFilterActive, m_bDBFiltersChanged;
  int m_iSortColumn, m_bSortAscending;

  bool m_bCanHaveAttachments;
  bool m_bDBReadOnly;
  std::set<StringX> m_sMediaTypes;
};
