/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWDialog.h"
#include "ColumnChooserLC.h"
#include "LVHdrCtrl.h"

// CColumnChooserDlg dialog

class CColumnChooserDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CColumnChooserDlg)

public:
  CColumnChooserDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CColumnChooserDlg();

  BOOL Create(UINT nID, CWnd *parent);
  void SetLVHdrCtrlPtr(CLVHdrCtrl *pLVHdrCtrl) {m_pLVHdrCtrl = pLVHdrCtrl;}

  // Dialog Data
  //{{AFX_DATA(CColumnChooserDlg)
  enum { IDD = IDD_COLUMNCHOOSER };
  CColumnChooserLC m_ccListCtrl;
  //}}AFX_DATA

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual void PostNcDestroy();
  virtual BOOL OnInitDialog();

  //{{AFX_DATA(CColumnChooserDlg)
  afx_msg void OnDestroy();
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
  //}}AFX_DATA

  DECLARE_MESSAGE_MAP()

public:
  CLVHdrCtrl *m_pLVHdrCtrl;
};
