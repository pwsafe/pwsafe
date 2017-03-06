/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file HotKeyConflictDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"

// CHotKeyConflictDlg dialog

class CHotKeyConflictDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CHotKeyConflictDlg)

public:
	CHotKeyConflictDlg(CWnd *pParent, int iRC,
    CString csAPPVALUE, CString csATVALUE,
    CString csAPPMENU, CString csAPPENTRY, CString csATMENU, CString csATENTRY);
	virtual ~CHotKeyConflictDlg();

  enum HotKeyError {
    HKE_NONE = 0,
    HKE_APPMENU = 1, HKE_APPENTRY = 2, HKE_APP = 3  /* HKE_APPMENU + HKE_APPENTRY */,
    HKE_ATMENU  = 4, HKE_ATENTRY  = 8, HKE_AT  = 12 /* HKE_ATMENU  + HKE_ATENTRY  */
  };

// Dialog Data
	enum { IDD = IDD_HOTKEYCONFLICTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

private:
  CString m_csAPPVALUE, m_csATVALUE, m_csAPPMENU, m_csAPPENTRY, m_csATMENU, m_csATENTRY;
  int m_iRC;
  CFont m_BoldFont;
};
