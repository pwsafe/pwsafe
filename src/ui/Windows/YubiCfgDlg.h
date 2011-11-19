/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// \file YubiCfgDlg.h

#include "PWDialog.h"
#import <YubiClientAPI.dll> no_namespace, named_guids

class PWScore;

class CYubiCfgDlg : public CPWDialog
{
public:
	CYubiCfgDlg(CWnd* pParent, PWScore &core);   // standard constructor
	virtual ~CYubiCfgDlg();

// Dialog Data
	enum { IDD = IDD_YUBIKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  
	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
  CString m_YubiSN;
  CString m_YubiSK;
public:
  afx_msg void OnYubiGenBn();
  afx_msg void OnBnClickedOk();
 private:
  enum {YUBI_SK_LEN = 20};
  void Init();
  void Destroy();
  void ReadYubiSN();
  // Callbacks:
	void yubiInserted(void); // called when Yubikey's inserted
	void yubiRemoved(void);  // called when Yubikey's removed
  void yubiCompleted(ycRETCODE rc); // called when done with request
  void yubiWait(WORD seconds); // called when waiting for user activation

  IYubiClient *m_obj;
  DWORD m_eventCookie;
  ycENCODING m_encoding;
  bool m_isInit;

  unsigned char m_yubi_sk_bin[YUBI_SK_LEN];
  PWScore &m_core;
};
