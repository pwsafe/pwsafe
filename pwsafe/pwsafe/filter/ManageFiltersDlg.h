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

// Pools (LOWORD) and flags (HIWORD) (in CListCtrl ItemData)
enum {MFLT_REQUEST_EXPORT = 0x10000, 
      MFLT_REQUEST_COPY_TO_DATABASE = 0x20000};

class DboxMain;

class CManageFiltersDlg : public CPWResizeDialog
{
  DECLARE_DYNAMIC(CManageFiltersDlg)

public:
  CManageFiltersDlg(CWnd* pParent,
                 bool bFilterActive,
                 PWSFilters &pmapFilters);
  virtual ~CManageFiltersDlg();
  void SetCurrentData(int currentfilterpool, CString currentfiltername)
  {m_currentfilterpool = currentfilterpool;
   m_currentfiltername = currentfiltername;}

// Dialog Data
  enum { IDD = IDD_MANAGEFILTERS };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  BOOL OnInitDialog();

  //{{AFX_MSG(CManageFiltersDlg)
  afx_msg void OnFilterApply();
  afx_msg void OnFilterNew();
  afx_msg void OnFilterEdit();
  afx_msg void OnFilterCopy();
  afx_msg void OnFilterDelete();
  afx_msg void OnFilterImport();
  afx_msg void OnFilterExport();
  afx_msg void OnClick(NMHDR *pNotifyStruct, LRESULT *pResult);
  afx_msg void OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnItemChanging(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnHDRBeginTrack(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnHDRItemChanging(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;

  PWSFilters &m_MapFilters;
  std::vector<st_Filterkey> m_vcs_filters;

  UINT GetFieldTypeName(const FieldType &ft);
  void DisplayFilterProperties(st_filters *pfilter);
  void UpdateFilterList();
  void DrawImage(CDC *pDC, CRect &rect, int nImage);

  CListCtrl m_FilterLC, m_FilterProperties;
  CStatusBar m_statusBar;
  CImageList *m_pImageList, *m_pCheckImageList;

  int m_currentfilterpool;
  CString m_currentfiltername;
  int m_function;
  int m_selectedfilter;
  int m_num_to_export, m_num_to_copy;
  bool m_bFilterActive;
  bool m_bStopChange;
};
