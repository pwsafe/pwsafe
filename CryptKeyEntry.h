/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// CryptKeyEntry.h
//-----------------------------------------------------------------------------

#include "SecString.h"
#include "corelib/PwsPlatform.h"
#include "PWDialog.h"

class CCryptKeyEntry : public CPWDialog
{
  // Construction
public:
  CCryptKeyEntry(CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CCryptKeyEntry)
  enum { IDD = IDD_CRYPTKEYENTRY };
  CSecString m_cryptkey1;
  CSecString m_cryptkey2;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CCryptKeyEntry)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CCryptKeyEntry)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
#if defined(POCKET_PC)
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();
#endif
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
