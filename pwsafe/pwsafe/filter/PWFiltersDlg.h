/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWFiltersDlg dialog

#include "../PWResizeDialog.h"
#include "../corelib/Itemdata.h"
#include "../corelib/PWSFilters.h"
#include "PWFilterLC.h"

#include "../resource.h"

class CPWFiltersDlg : public CPWResizeDialog
{
  DECLARE_DYNAMIC(CPWFiltersDlg)
public:
  CPWFiltersDlg(CWnd* pParent = NULL, const FilterType &ftype = DFTYPE_MAIN,
                const CString &filtername = _T(""));
  ~CPWFiltersDlg();

  void UpdateStatusText();
  CString GetFiltername() {return m_filtername;}
  virtual void EnableDisableApply() {}
  void UpdateDialogMaxWidth();

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
  afx_msg void OnBeginTrack(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnItemchanging(NMHDR * pNotifyStruct, LRESULT* pResult);
  afx_msg void OnProcessKey(UINT nID);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  BOOL PreTranslateMessage(MSG* pMsg);
  
 public:
  afx_msg void OnOk();
  afx_msg void OnHelp();

private:
  CPWFilterLC m_FilterLC;
  CEdit m_FNameEdit;

  HACCEL m_hAccel;

  int m_numfilters;
  int m_iType;
  bool m_bStopChange;
};
