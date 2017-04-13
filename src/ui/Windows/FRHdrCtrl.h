/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FRHdrCtrl.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFRHdrCtrl window

class CFindReplaceDlg;
class CItemData;

enum FRState { FR_INVALID = -1, FR_UNCHECKED, FR_CHECKED, FR_CHANGED };

struct st_FRResults {
  CItemData *pci;
  FRState state;

  st_FRResults()
    : pci(NULL), state(FR_INVALID) {}

  st_FRResults(const st_FRResults &that)
    : pci(that.pci), state(that.state) {}

  st_FRResults &operator=(const st_FRResults &that)
  {
    if (this != &that) {
      pci = that.pci;
      state = that.state;
    }
    return *this;
  }
};

class CFRHdrCtrl : public CHeaderCtrl
{
// Construction
public:
	CFRHdrCtrl();
	virtual ~CFRHdrCtrl();

  void SetParent(CFindReplaceDlg *pGERdlg) { m_pGEDlg = pGERdlg; }
  FRState GetState() { return m_state; }
  void SetState(const FRState &state) { m_state = state; }

	// Generated message map functions
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFRHdrCtrl)
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFRHdrCtrl)
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHeaderColumnClicked(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
  afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  void UpdateRowHeight();

  CFindReplaceDlg *m_pGEDlg;

  FRState m_state;
};
