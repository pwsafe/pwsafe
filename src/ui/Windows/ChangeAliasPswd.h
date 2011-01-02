/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// ChangeAliasPswd.h
//-----------------------------------------------------------------------------
#include "PWDialog.h"

// CChangeAliasPswd dialog

class CChangeAliasPswd : public CPWDialog
{
	DECLARE_DYNAMIC(CChangeAliasPswd)

public:
	CChangeAliasPswd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangeAliasPswd();

  // Dialog Data
	enum { IDD = IDD_ALIAS_PSWDCHANGE };

  enum { CHANGEBASE = -1, CHANGEALIAS = -2};

  CString m_BaseEntry;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  afx_msg void OnChangeBasePswd();
  afx_msg void OnChangeAliasPswd();

	DECLARE_MESSAGE_MAP()
};
