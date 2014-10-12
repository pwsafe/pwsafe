/*
* Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_Attachment.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"
#include "resource.h"

class CAddEdit_Attachment : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_Attachment)

  CAddEdit_Attachment(CWnd *pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_Attachment();

  static bool m_bNumDaysFailed;
  static bool m_bShowUUID;

  // Dialog Data
  //{{AFX_DATA(CAddEdit_Attachment)
  enum { IDD = IDD_ADDEDIT_ATT, IDD_SHORT = IDD_ADDEDIT_ATT_SHORT };

  void UpdateStats();

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_Attachment)
protected:
  BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Generated message map functions
  //{{AFX_MSG(CAddEdit_Attachment)
  afx_msg void OnHelp();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);

  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void SetXTime();
  void UpdateTimes();
  bool m_bInitdone;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
