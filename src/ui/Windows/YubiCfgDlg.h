/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// \file YubiCfgDlg.h

#include "PWDialog.h"
#include "ControlExtns.h"
#include "core/PWSfileHeader.h"

class PWScore;

class CYubiCfgDlg : public CPWDialog
{
public:
  enum {YUBI_SK_LEN = PWSfileHeader::YUBI_SK_LEN};
  CYubiCfgDlg(CWnd* pParent, PWScore &core);   // standard constructor
  virtual ~CYubiCfgDlg();

  int WriteYubiSK(const unsigned char *yubi_sk_bin);

  // Dialog Data
  enum { IDD = IDD_YUBIKEY };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  
  CString m_YubiSN;
  CSecString m_YubiSK;
  CSecEditExtn m_ex_YubiSK;

  afx_msg void OnYubiGenBn();
  afx_msg void OnBnClickedOk();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnHelp();

  DECLARE_MESSAGE_MAP()

 private:
  void ReadYubiSN();
  bool IsYubiInserted() const;
  void yubiInserted(void); // called when Yubikey's inserted
  void yubiRemoved(void);  // called when Yubikey's removed
  void ShowSK();
  void HideSK();

  bool m_present; // key present?
  bool m_isSKHidden;
  mutable CMutex m_mutex; // protect against race conditions when calling Yubi API
  PWScore &m_core;
  afx_msg void OnBnClickedYubiShowHide();
};
