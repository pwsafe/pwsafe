/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// ViewFilterDlg dialog

#include "../PWResizeDialog.h"
#include "../corelib/PWSFilters.h"

#include <vector>

// Subitem indices
#define VFLC_FILTER_NUMBER 0
#define VFLC_FILTER_ACTIVE 1
#define VFLC_AND_OR        2
#define VFLC_FIELD         3
#define VFLC_CRITERIA_TEXT 4
#define VFLC_NUM_COLUMNS   5

enum {VF_CURRENT = 0, VF_DATABASE, VF_GLOBAL};

class CViewFilterDlg : public CPWResizeDialog
{
  DECLARE_DYNAMIC(CViewFilterDlg)

public:
  CViewFilterDlg(CWnd* pParent,
                 st_filters *pfilters,
                 PWSFilters &pmapdbfilters,
                 PWSFilters &pmapglobalfilters);
  virtual ~CViewFilterDlg();

// Dialog Data
  enum { IDD = IDD_VIEWFILTER };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  BOOL OnInitDialog();

  //{{AFX_MSG(CViewFilterDlg)
  afx_msg void OnBnClickedCurrent();
  afx_msg void OnBnClickedDBStore();
  afx_msg void OnBnClickedGlobalStore();
  afx_msg void OnFilterSelected();
  afx_msg void OnBeginTrack(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnItemchanging(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  //}}AFX_MSG
 
  DECLARE_MESSAGE_MAP()

private:
  st_filters *m_pfilters;
  PWSFilters &m_DBFilters;
  PWSFilters &m_GlobalFilters;
  std::vector<CString> m_vcs_db;
  std::vector<CString> m_vcs_gbl;

  UINT GetFieldTypeName(const FieldType &ft);
  void SelectFilter(st_filters *pfilter);

  CComboBox m_combo;
  CListCtrl m_FilterLC;
  CStatusBar m_statusBar;

  int m_selectedstore;
  int m_function;
};
