/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "PWDialog.h"
#include "ExpPswdLC.h"

#include "core/ExpiredList.h"

// CExpPWListDlg dialog

class ExpiredList;
class DboxMain;

class CExpPWListDlg : public CPWDialog
{
public:
  CExpPWListDlg(CWnd* pParent, ExpiredList &expPWList,
                const CString& a_filespec = L"");
  virtual ~CExpPWListDlg();

  // Dialog Data
  enum { IDD = IDD_DISPLAY_EXPIRED_ENTRIES };
  CExpPswdLC m_expPWListCtrl;
  CImageList *m_pImageList;
  CString m_message;
  int m_iSortedColumn; 
  BOOL m_bSortAscending; 

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  afx_msg void OnIconHelp();
  afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemDoubleClick(NMHDR* pNotifyStruct, LRESULT* result);
  virtual void OnOK();

  DECLARE_MESSAGE_MAP()

private:
  int GetEntryImage(const ExpPWEntry &ee);
  ExpiredList &m_expPWList;
  DboxMain *m_pDbx;

  int m_idays;
  static int CALLBACK ExpPWCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
