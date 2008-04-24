/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * This class handles the password expiration date/time dialog box
 * User can specify a given expiration date, or an interval.
 * The interval can be one-time or recurring.
 */

#pragma once

#include "afxwin.h"
#include "afxdtctl.h" // only needed for date/time controls
#include "corelib\MyString.h"
#include "PWDialog.h"

class CExpDTDlg : public CPWDialog
{

public:
  CExpDTDlg(time_t baseTime, // entry creation or last modification time
            time_t expTime, // entry's current exp. time (or 0)
            int expInterval, // entry's current exp. interval (or 0)
            CWnd* pParent = NULL);  // standard constructor

  CDateTimeCtrl m_pTimeCtl;         // time picker control
  CDateTimeCtrl m_pDateCtl;         // date picker control
  CMyString m_locXTime;             // formatted time per user's Short Date/Time
  const time_t m_tttCPMTime;  // entry creation or password last changed datetime
  time_t m_tttXTime;                // Expiry date/time
  int m_XTimeInt; // interval (in days) to expiration

  // Dialog Data
  //{{AFX_DATA(CImportDlg)
  enum { IDD = IDD_PICKEXPDATETIME };
  int m_how; // is expiration absolute or relative? (int for DDX)
  int m_numDays; // interval (in days) to expiration when m_how == RELATIVE
  int m_maxDays; // limited s.t. time_t can't overflow
  BOOL m_ReuseOnPswdChange; // e.g., is interval recurring or 1-shot.
  //}}AFX_DATA
  enum {ABSOLUTE_EXP = 0, RELATIVE_EXP = 1}; // m_how's values

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
  afx_msg void OnDateTime();
  afx_msg void OnDays();
  afx_msg void OnOK();
  afx_msg void OnReuseOnPswdChange();
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
