/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// QuerySetDef.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #define SUPERCLASS	CDialog
#endif

class CQuerySetDef : public SUPERCLASS
{
// Construction
public:
	typedef SUPERCLASS	super;

   CQuerySetDef(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CQuerySetDef)
   enum { IDD = IDD_QUERYSETDEF };
   BOOL	m_querycheck;
   //}}AFX_DATA
   CString	m_message;


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CQuerySetDef)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   //{{AFX_MSG(CQuerySetDef)
   virtual void OnOK();
   virtual void OnCancel();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
