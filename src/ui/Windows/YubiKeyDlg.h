/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once


/// \file YubiKeyDlg.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "corelib/YubiKey.h"

class CYubiKeyDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CYubiKeyDlg)

    public:
	CYubiKeyDlg(CWnd* pParent,
              const StringX &yPubID,
              unsigned int yapiID,
              const YubiApiKey_t &yapiKey);
	virtual ~CYubiKeyDlg();

  bool VerifyOTP(CString &error); // verify m_otp. If fails, error explains why
  // Dialog Data
	enum { IDD = IDD_YUBIKEY };
  
 protected:
  virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnOk();
  afx_msg void OnHelp();
  CString GetYKpubID() const {return m_YKpubID;}
  unsigned int GetApiID() const {return m_apiID;}
  const unsigned char *GetApiKey() const {return m_apiKey;}
  
 private:
  CString m_YKinfo;
  CString m_otp;
  CString m_YKstatus;
  CString m_YKpubID;
  unsigned int m_apiID;
  YubiApiKey_t m_apiKey;
  CString m_apiKeyStr;
  BOOL m_ykEnabled; // False means user doesn't want yubikey. If true,
  //                   id & api key must be filled for OK to be enabled.
   afx_msg void OnBnClickedYkEnabled();
};
