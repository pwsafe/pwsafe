/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// ManageFiltersDlg dialog

#include "../PWResizeDialog.h"
#include "PWHdrCtrlNoChng.h"
#include "../corelib/PWSFilters.h"

#include <vector>

// Subitem indices for list of filters
#define MFLC_FILTER_NAME    0
#define MFLC_FILTER_SOURCE  1
#define MFLC_COPYTODATABASE 2
#define MFLC_EXPORT         3
#define MFLC_INUSE          4
#define MFLC_NUM_COLUMNS    5

// Subitem indices for filter properties
#define MFPRP_FILTER_NUMBER 0
#define MFPRP_FILTER_ACTIVE 1
#define MFPRP_AND_OR        2
#define MFPRP_FIELD         3
#define MFPRP_CRITERIA_TEXT 4
#define MFPRP_NUM_COLUMNS   5

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

class DboxMain;

class CManageFiltersDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CManageFiltersDlg)

public:
  CManageFiltersDlg(CWnd* pParent,
                 bool bFilterActive,
                 PWSFilters &pmapFilters);
  virtual ~CManageFiltersDlg();
  void SetCurrentData(FilterPool activefilterpool, CString activefiltername)
  {m_activefilterpool = activefilterpool;
   m_activefiltername = activefiltername;}
  bool HasDBFiltersChanged() {return m_bDBFiltersChanged;}

// Dialog Data
  enum { IDD = IDD_MANAGEFILTERS };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  BOOL OnInitDialog();

  //{{AFX_MSG(CManageFiltersDlg)
  afx_msg void OnFilterSet();
  afx_msg void OnFilterClear();
  afx_msg void OnFilterNew();
  afx_msg void OnFilterEdit();
  afx_msg void OnFilterCopy();
  afx_msg void OnFilterDelete();
  afx_msg void OnFilterImport();
  afx_msg void OnFilterExport();
  afx_msg void OnDestroy();
  afx_msg void OnClick(NMHDR *pNotifyStruct, LRESULT *pResult);
  afx_msg void OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnItemChanging(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;

  PWSFilters &m_MapFilters;
  std::vector<st_Filterkey> m_vcs_filters;

  UINT GetFieldTypeName(const FieldType &ft);
  void DisplayFilterProperties(st_filters *pfilter);
  void UpdateFilterList();
  void ResetColumns();
  void DrawImage(CDC *pDC, CRect &rect, int nImage);
  void SortFilterView();
  static int CALLBACK FLTCompareFunc(LPARAM lParam1, LPARAM lParam2, 
                                     LPARAM pSelf);
  static CString GetFilterPoolName(FilterPool fp);

  CListCtrl m_FilterLC, m_FilterProperties;
  CStatusBar m_statusBar;
  CImageList *m_pImageList, *m_pCheckImageList;
  CPWHdrCtrlNoChng m_FLCHeader;
  CPWHdrCtrlNoChng m_FPROPHeader;

  FilterPool m_selectedfilterpool, m_activefilterpool;
  CString m_selectedfiltername, m_activefiltername;
  int m_selectedfilter, m_activefilter;
  int m_num_to_export, m_num_to_copy;
  bool m_bFilterActive, m_bDBFiltersChanged;
  int m_iSortColumn, m_bSortAscending;
};
