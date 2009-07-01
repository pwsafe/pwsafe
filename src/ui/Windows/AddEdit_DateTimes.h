/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "corelib/ItemData.h"
#include "corelib/PWSprefs.h"

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

  const wchar_t *GetHelpName() const {return L"TO_DO!";}

  static bool m_bNumDaysFailed;

  // Dialog Data
  //{{AFX_DATA(CAddEdit_DateTimes)
  enum { IDD = IDD_ADDEDIT_DATETIMES };

  enum {ABSOLUTE_EXP = 0, RELATIVE_EXP = 1}; // m_how's values

  CDateTimeCtrl m_pTimeCtl;    // time picker control
  CDateTimeCtrl m_pDateCtl;    // date picker control

  BOOL m_ReuseOnPswdChange;     // e.g., is interval recurring or 1-shot.
  int m_how;                    // is expiration absolute or relative? (int for DDX)
  int m_numDays;                // interval (in days) to expiration when m_how == RELATIVE
  int m_maxDays;                // limited s.t. time_t can't overflow

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CAddEdit_DateTimes)
protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnApply();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CAddEdit_DateTimes)
  afx_msg void OnHelp();
  afx_msg BOOL OnKillActive();
  afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM);
  afx_msg void OnClearXTime();
  afx_msg void OnSetXTime();
  afx_msg void OnDateTime();
  afx_msg void OnDays();
  afx_msg void OnReuseOnPswdChange();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
