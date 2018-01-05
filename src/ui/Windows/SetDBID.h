/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file SetDBID.h
//

#include "PWDialog.h"
#include "ControlExtns.h"

// CSetDBID dialog

class DboxMain;

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

class CSetDBID : public CPWDialog
{
	DECLARE_DYNAMIC(CSetDBID)

public:
	CSetDBID(CWnd *pParent, int iIndex = 0);
	virtual ~CSetDBID();

// Dialog Data
	enum { IDD = IDD_SETDBID };

  HANDLE GetMutexHandle() { return m_hMutexDBIndex; }
  COLORREF GetLockedIndexColour() { return m_clrLockedTextColour; }
  COLORREF GetUnlockedIndexColour() { return m_clrUnlockedTextColour; }

protected:
  virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnDestroy();
  afx_msg void OnOK();
  afx_msg void OnCancel();
  afx_msg void OnHelp();
  afx_msg void OnSetLockedColour(UINT nID);
  afx_msg void OnSetUnlockedColour(UINT nID);
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

  CNPEdit m_edtSBIndex;
  CStatic m_stLockedImage, m_stUnlockedImage;
  int m_iDBIndex, m_iInitialDBIndex, m_iLockedTextColour, m_iUnLockedTextColour;

  HANDLE m_hMutexDBIndex;
 
private:
  void SetBitmapBackground(CBitmap &bm);
  void CreateIndexBitmap(const int iIndex, const COLORREF clrText, const bool bLocked);

  COLORREF m_clrLockedTextColour, m_clrUnlockedTextColour, m_clrBackground;
  COLORREF m_clrLockedTextOptions[4], m_clrUnlockedTextOptions[4];

  CBitmap m_bmLocked, m_bmUnlocked;
  CBrush  m_Brush;
  bool m_bInitDone;
  DboxMain *m_pParent;
};
