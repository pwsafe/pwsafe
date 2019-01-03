/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddEdit_DateTimes.h
//-----------------------------------------------------------------------------

#pragma once

#include "AddEdit_PropertyPage.h"
#include "ControlExtns.h"
#include "core/ItemData.h"
#include "core/PWSprefs.h"

#include "resource.h"

#include "afxdtctl.h" // only needed for date/time controls

class DboxMain;

class CAddEdit_DateTimes : public CAddEdit_PropertyPage
{
  // Construction
public:
  DECLARE_DYNAMIC(CAddEdit_DateTimes)

  CAddEdit_DateTimes(CWnd *pParent, st_AE_master_data *pAEMD);
  ~CAddEdit_DateTimes();

  static bool m_bNumDaysFailed;
  static bool m_bShowUUID;

  // Dialog Data
  //{{AFX_DATA(CAddEdit_DateTimes)
  enum { IDD = IDD_ADDEDIT_DATETIMES, IDD_SHORT = IDD_ADDEDIT_DATETIMES_SHORT };

  // m_how's values DEFAULT_SYMBOLS - in Radio button order
  enum {ABSOLUTE_EXP = 0, RELATIVE_EXP = 1, NONE_EXP = 2};

  CDateTimeCtrl m_pDateCtl;    // date picker control

  BOOL m_bRecurringPswdExpiry;   // e.g., is interval recurring or 1-shot.
  int m_how;                    // is expiration absolute or relative? (int for DDX)
  int m_numDays;                // interval (in days) to expiration when m_how == RELATIVE
  int m_maxDays;                // limited s.t. time_t can't overflow
  bool m_inSetX;                // avoid nasty recursion when updating stuff directly

  void UpdateStats();

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_DateTimes)
protected:
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  virtual BOOL OnKillActive();
  //}}AFX_VIRTUAL

  // Generated message map functions
  //{{AFX_MSG(CAddEdit_DateTimes)
  afx_msg void OnHelp();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);

  afx_msg void OnDaysChanged();
  afx_msg void OnDateTimeChanged(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnRecurringPswdExpiry();
  //}}AFX_MSG
  afx_msg void OnHowChanged(); // When a RB's clicked

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
