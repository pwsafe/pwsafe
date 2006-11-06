/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// PasskeyChangeDlg.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #define SUPERCLASS	CDialog
#endif

class CPasskeyChangeDlg : public SUPERCLASS
{
// Construction
public:
	typedef SUPERCLASS	super;

   CPasskeyChangeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CPasskeyChangeDlg)
   enum { IDD = IDD_KEYCHANGE_DIALOG };
   CMyString	m_confirmnew;
   CMyString	m_newpasskey;
   CMyString	m_oldpasskey;
   //}}AFX_DATA

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CPasskeyChangeDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   // Generated message map functions
   //{{AFX_MSG(CPasskeyChangeDlg)
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   virtual void OnCancel();
   afx_msg void OnHelp();
#if defined(POCKET_PC)
   afx_msg void OnPasskeySetfocus();
   afx_msg void OnPasskeyKillfocus();
#endif
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
