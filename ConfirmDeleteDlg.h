/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// ConfirmDeleteDlg.h
//-----------------------------------------------------------------------------
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #include "resource.h"
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
  #define SUPERCLASS	CDialog
#endif

class CConfirmDeleteDlg : public SUPERCLASS
{
	typedef SUPERCLASS		super;

// Construction
public:
   CConfirmDeleteDlg(CWnd* pParent = NULL, int numchildren = 0);
private:
// Dialog Data
   //{{AFX_DATA(CConfirmDeleteDlg)
   enum { IDD = IDD_CONFIRMDELETE_DIALOG };
   bool	m_dontaskquestion;
   int m_numchildren;
   //}}AFX_DATA

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CConfirmDeleteDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   // Generated message map functions
   //{{AFX_MSG(CConfirmDeleteDlg)
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   virtual void OnOK();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
