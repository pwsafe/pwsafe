/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// ClearQuestionDlg.h : header file
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


class CClearQuestionDlg : public SUPERCLASS
{
public:
	typedef SUPERCLASS	super;

   CClearQuestionDlg(CWnd* pParent = NULL);   // standard constructor

   enum { IDD = IDD_SECURECLEAR };
   bool	m_dontaskquestion;

protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   virtual void OnCancel();
   virtual void OnOK();

   DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
