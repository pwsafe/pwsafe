/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
  CDisplayAuthCodeDlg(CWnd* pParent, PWScore& core, const pws_os::CUUID& uuidEntry); // standard constructor
  ~CDisplayAuthCodeDlg();

  enum { IDD = IDD_DISPLAY_AUTH_CODE };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  CItemData* GetItem() {
    auto it = m_core.Find(m_uuidItem);
    ASSERT(it != m_core.GetEntryEndIter());
    return (it != m_core.GetEntryEndIter()) ? &it->second : nullptr;
  }

  CItemData* GetCredentialItem() {
    CItemData* pci = GetItem();
    return pci ? m_core.GetCredentialEntry(pci) : nullptr;
  }

  CTBMStatic m_Help1, m_Help2;

  //{{AFX_MSG(CDisplayAuthCodeDlg)
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnCopyTwoFactorCode();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool UpdateAuthCode(CItemData* pciCred);
  void CopyAuthCodeToClipboard();
  void SetupAuthenticationCodeUiElements();
  void StopAuthenticationCodeUi();

private:
  PWScore& m_core;
  pws_os::CUUID m_uuidItem;
  CStaticExtn m_stcEntryName;
  CProgressPieCtrl m_btnCopyTwoFactorCode;
  CStaticExtn m_stcTwoFactorCode;
  CButton m_btnClose;
  CFont m_fontTwoFactorCode;
  bool m_bCopyToClipboard;
  StringX m_sxLastAuthCode;
  CRect m_rcInitial;
  CRect m_rwInitial;
  CRect m_rectInitialEntryName;
  CRect m_rectInitialAuthCodeButton;
  CRect m_rectInitialAuthCode;
  CRect m_rectInitialCloseButton;
  int m_cyAuthCodeButtonMarginBottom;
  int m_cxMinWidth;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
