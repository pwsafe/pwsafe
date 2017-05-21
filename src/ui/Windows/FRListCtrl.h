/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FRListCtrl.h : header file
//

#pragma once

#include "FRHdrCtrl.h"  // For FRState

class CFindReplaceDlg;

class CFRListCtrl : public CListCtrl
{
// Construction
public:
	CFRListCtrl();
  virtual ~CFRListCtrl();

  void Init(CFindReplaceDlg *pGERdlg);
  void SetHeaderImage(FRState state);
  void SetHeaderSortArrows(const int &iSortedColumn, const bool &bSortAscending);

// Operations
protected:
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG(CFRListCtrl)
  afx_msg void OnDestroy();
  afx_msg void DrawImage(CDC *pDC, CRect &rect, FRState nImage);
  afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
  afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  CFRHdrCtrl m_CheckHeadCtrl;

private:
  void UpdateRowHeight(bool bInvalidate);

  bool bInitdone;
  int m_ibmHeight;
  COLORREF m_clrDisabled;
  CFont *m_pCurrentFont, *m_pModifiedFont;

  CImageList *m_pCheckImageList;
  CFindReplaceDlg *m_pGEDlg;
};
