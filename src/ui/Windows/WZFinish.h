/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "WZPropertyPage.h"
#include "WZExecuteThreadParms.h"

class CReport;

class CWZFinish : public CWZPropertyPage
{
public:
  DECLARE_DYNAMIC(CWZFinish)

  CWZFinish(CWnd *pParent, UINT nIDCaption, const int nType);
  ~CWZFinish();

  enum {IDD = IDD_WZFINISH};

  void DisableAbort();

protected:
  virtual BOOL OnInitDialog();

  // Generated message map functions
  //{{AFX_MSG(CWZFinish)
  afx_msg BOOL OnSetActive();
  afx_msg void OnHelp();
  afx_msg void OnAbort();
  afx_msg void OnViewReport();
  LRESULT OnExecuteThreadEnded(WPARAM wParam, LPARAM );
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  BOOL Execute();

  CReport *m_prpt;
  CWinThread *m_pExecuteThread; // Execute worker thread
  int DoExecuteThread(WZExecuteThreadParms * &pthdpms);
  int ExecuteAction();

  PWScore *m_pothercore;
  st_SaveAdvValues *m_pst_SADV;
  bool m_bInitDone, m_bInProgress, m_bComplete, m_bViewingReport;
  int m_status;
  int m_numProcessed;
  WZExecuteThreadParms m_thdpms;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
