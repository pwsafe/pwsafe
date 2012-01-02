/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "PWDialog.h"

// CInputBox dialog

class CInputBox : public CPWDialog
{
	DECLARE_DYNAMIC(CInputBox)

public:
	CInputBox(UINT nIDCaption, CString csInit, int maxlen = 0,
              const bool bReadOnly = false, CWnd *pParent = NULL);   // standard constructor
	virtual ~CInputBox();

  CString GetText() {return m_csText;}

// Dialog Data
	enum { IDD = IDD_INPUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg void OnOK();
  afx_msg void OnInputChanged();
  afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);


  CEdit m_edText;
  UINT m_nIDCaption;
  CString m_csText;
  int m_maxlen;
  bool m_bReadOnly, m_bInitDone;

	DECLARE_MESSAGE_MAP()
};
