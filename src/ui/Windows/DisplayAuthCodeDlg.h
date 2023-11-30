/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 29-Nov-2023
*/
#pragma once

// DisplayAuthCodeDlg.h : header file
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "ControlExtns.h"
#include "TBMStatic.h"
#include "ProgressPieCtrl.h"

#include "core/PWScore.h"
#include "core/ItemData.h"

class CDisplayAuthCodeDlg : public CPWDialog
{
public:
  CDisplayAuthCodeDlg(CWnd* pParent, PWScore& core, CItemData& ci); // standard constructor
  ~CDisplayAuthCodeDlg();

  enum { IDD = IDD_DISPLAY_AUTH_CODE };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  CTBMStatic m_Help1, m_Help2;

  //{{AFX_MSG(CDisplayAuthCodeDlg)
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnCopyTwoFactorCode();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool UpdateAuthCode();
  void CopyAuthCodeToClipboard();
  void SetupAuthenticationCodeUiElements();
  void StopAuthenticationCodeUi();

private:
  PWScore& m_core;
  CItemData& m_ci;
  CItemData& m_ciCredential;
  CProgressPieCtrl m_btnCopyTwoFactorCode;
  CStaticExtn m_stcTwoFactorCode;
  CFont m_fontTwoFactorCode;
  bool m_bCopyToClipboard;
  StringX m_sxLastAuthCode;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
