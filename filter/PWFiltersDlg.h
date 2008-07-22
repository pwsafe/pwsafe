/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWFiltersDlg dialog

#include "../PWDialog.h"
#include "../corelib/Itemdata.h"
#include "../corelib/filters.h"
#include "PWFilterLC.h"

#include "../resource.h"

class CPWFiltersDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CPWFiltersDlg)
public:
  CPWFiltersDlg(CWnd* pParent = NULL, const FilterType &ftype = DFTYPE_MAIN,
                const CString &filtername = _T(""));
  ~CPWFiltersDlg();

  void UpdateStatusText();
  CString GetFiltername() {return m_filtername;}
  virtual void EnableDisableApply() {}

// Dialog Data
  enum { IDD = IDD_SETFILTERS };

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  CString m_cstitle;
  CWnd *m_pParent;
  st_filters *m_pfilters;
  CString m_filtername;
  bool VerifyFilters();

  //{{AFX_MSG(CPWFiltersDlg)
  afx_msg void OnFNameKillFocus();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBeginTrack(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnItemchanging(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnProcessKey(UINT nID);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  BOOL PreTranslateMessage(MSG* pMsg);
  
 public:
  afx_msg void OnOk();
  afx_msg void OnHelp();

private:
  void SetControls(int cx, int cy);

  CPWFilterLC m_FilterLC;
  CEdit m_FNameEdit;
  CStatusBar m_statusBar;

  HACCEL m_hAccel;

  UINT statustext[1];
  bool m_bInitDone, m_bStatusBarOK;
  int m_numfilters;
  int m_iType;
  bool m_bStopChange;
  int m_DialogMinWidth, m_DialogMinHeight, m_DialogMaxHeight;
  int m_cxBSpace, m_cyBSpace, m_cySBar;
};
