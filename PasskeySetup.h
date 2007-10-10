/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once

// PasskeySetup.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"
#include "PWDialog.h"

class CPasskeySetup : public CPWDialog
{
// Construction
public:
   CPasskeySetup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CPasskeySetup)
   enum { IDD = IDD_PASSKEYSETUP };
   CMyString	m_passkey;
   CMyString	m_verify;
   //}}AFX_DATA

protected:
   virtual BOOL OnInitDialog();
   //{{AFX_VIRTUAL(CPasskeySetup)
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   // Generated message map functions
   //{{AFX_MSG(CPasskeySetup)
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
