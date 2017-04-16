/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file SBIndexDlg.h
//

#include "PWDialog.h"
#include "ControlExtns.h"

// CSBIndexDlg dialog

class CNPEdit : public CEdit
{
  // Even though CEdit supports ES_NUMBER, this only works for
  // character input.  The user could still paste in anything.
  // This prevents this.
public:
  CNPEdit() {}
  virtual ~CNPEdit() {}

protected:
  afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);

  DECLARE_MESSAGE_MAP()
};

class CSBIndexDlg : public CPWDialog
{
	DECLARE_DYNAMIC(CSBIndexDlg)

public:
	CSBIndexDlg(CWnd *pParent, int iIndex = 0);
	virtual ~CSBIndexDlg();

// Dialog Data
	enum { IDD = IDD_SETDBINDEX };

  HANDLE GetMutexHandle() { return m_hMutexDBIndex; }

protected:
  virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnOK();
  afx_msg void OnCancel();
  afx_msg void OnHelp();

	DECLARE_MESSAGE_MAP()

  CNPEdit m_edtSBIndex;
  int m_iDBIndex, m_iInitialDBIndex;

  HANDLE m_hMutexDBIndex;
};
