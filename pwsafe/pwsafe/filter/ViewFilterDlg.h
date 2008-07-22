/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// ViewFilterDlg dialog

#include "../PWDialog.h"
#include "../corelib/filters.h"

#include <vector>

enum {VF_CURRENT = 0, VF_DATABASE, VF_GLOBAL};

class CViewFilterDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CViewFilterDlg)

public:
  CViewFilterDlg(CWnd* pParent,
                 st_filters *pfilters,
                 MapFilters &pmapdbfilters,
                 MapFilters &pmapglobalfilters);
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
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBeginTrack(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnItemchanging(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  //}}AFX_MSG
 
  DECLARE_MESSAGE_MAP()

private:
  void SetControls(int cx, int cy);

  st_filters *m_pfilters;
  MapFilters &m_pMapDBFilters;
  MapFilters &m_pMapGlobalFilters;
  std::vector<CString> m_vcs_db;
  std::vector<CString> m_vcs_gbl;

  UINT GetFieldTypeName(const FieldType &ft);
  void SelectFilter(st_filters *pfilter);

  CComboBox m_combo;
  CListCtrl m_FilterLC;
  CStatusBar m_statusBar;

  int m_selectedstore;
  int m_function;

  bool m_bInitDone, m_bStatusBarOK;
  int m_DialogMinWidth, m_DialogMinHeight, m_DialogMaxHeight;
  int m_cxBSpace, m_cyBSpace, m_cySBar;
};
