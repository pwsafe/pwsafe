/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// ClearQuestionDlg.h : header file
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"
#include "PWDialog.h"

class CClearQuestionDlg : public CPWDialog
{
public:
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
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
