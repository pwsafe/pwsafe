/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// CryptKeyEntry.h
//-----------------------------------------------------------------------------

#include "SecString.h"
#include "core/PwsPlatform.h"
#include <afxwin.h>

/**
 * This dialog box is used for the "undocumented" file encryption/decryption mode.
 * This means that it's invoked instead of DboxMain, and specifically before our
 * framework is fully initialized. This is why this MUST be a CDialog, and NOT
 * CPWDialog derived class.
 */

class CCryptKeyEntry : public CDialog
{
  // Construction
public:
  CCryptKeyEntry(bool isEncrypt, CWnd* pParent = NULL);

  // Dialog Data
  //{{AFX_DATA(CCryptKeyEntry)
  enum { IDD = IDD_CRYPTKEYENTRY };
  CSecString m_cryptkey1;
  CSecString m_cryptkey2;
  //}}AFX_DATA

protected:
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CCryptKeyEntry)
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  //}}AFX_VIRTUAL

  // Implementation
  // Generated message map functions
  //{{AFX_MSG(CCryptKeyEntry)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  //}}AFX_MSG
  
  DECLARE_MESSAGE_MAP()
  
private:
  bool m_encrypt; // from c'tor. False == decrypt, don't confirm password
  
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
