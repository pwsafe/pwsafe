/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CViewAttachments dialog

#include "PWDialog.h"
#include "PWAttLC.h"
#include "PWHdrCtrlNoChng.h"

#include "corelib/attachments.h"

class DboxMain;

class CViewAttachments : public CPWDialog
{
  DECLARE_DYNAMIC(CViewAttachments)

public:
  CViewAttachments(CWnd* pParent);   // standard constructor
  virtual ~CViewAttachments();

// Dialog Data
  enum { IDD = IDD_VIEW_ATTACHMENTS };
  void SetExAttachments(const ATRExVector &vATRecordsEx)
  {m_vATRecordsEx = vATRecordsEx;}

protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHelp();

  DECLARE_MESSAGE_MAP()

private:
  static int CALLBACK AttCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  CPWAttLC m_AttLC;
  CToolTipCtrl *m_pToolTipCtrl;

  ATRExVector m_vATRecordsEx;
  BOOL m_bInitdone;
  int m_iSortedColumn;
  bool m_bSortAscending;
  DboxMain *m_pDbx;
};
