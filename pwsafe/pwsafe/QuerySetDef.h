/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// QuerySetDef.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"
#include "PWDialog.h"

class CQuerySetDef : public CPWDialog
{
  // Construction
public:
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
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
