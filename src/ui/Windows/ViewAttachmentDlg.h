/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// ViewAttahmentDlg.h
//-----------------------------------------------------------------------------
#include "PWResizeDialog.h"
#include "ImgStatic.h"

#include "core/ItemAtt.h"

/////////////////////////////////////////////////////////////////////////////
// CFloatEdit

class CFloatEdit : public CEdit
{
  // Construction
public:
  CFloatEdit();

  void SetMinMax(float fMin, float fMax);
  UINT OnGetDlgCode() { return CEdit::OnGetDlgCode() | DLGC_WANTALLKEYS; }

protected:
  //{{AFX_MSG(CFloatEdit)
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg BOOL OnChange();
  afx_msg BOOL OnKillFocus();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CString m_csPreviousValue;
  int m_nLastSel;
  float m_fMin, m_fMax;
};

/////////////////////////////////////////////////////////////////////////////
// CZoomSliderCtrl

class CViewAttachmentDlg;

class CZoomSliderCtrl : public CSliderCtrl
{
  // Construction
public:
  CZoomSliderCtrl();
  virtual ~CZoomSliderCtrl();

protected:
  //{{AFX_MSG(CZoomSliderCtrl)
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CViewAttachmentDlg *m_pParent;
};

/////////////////////////////////////////////////////////////////////////////
// CViewAttachmentDlg

class CViewAttachmentDlg : public CPWResizeDialog
{
	DECLARE_DYNAMIC(CViewAttachmentDlg)

public:
	CViewAttachmentDlg(CWnd *pParent, CItemAtt *patt);   // standard constructor
  virtual ~CViewAttachmentDlg() { delete m_pToolTipCtrl; }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEWATTACHMENT };
#endif

  enum { MinZoom = 10, MaxZoom = 100 };
  void SetZoomEditValue(int nPos);

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  //{{AFX_MSG(CViewAttachmentDlg)
  afx_msg void OnOK();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  afx_msg void OnZoomChange();
  afx_msg void OnZoomTooltipText(NMHDR *pNMHDR, LRESULT *pLResult);
  //}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  CToolTipCtrl *m_pToolTipCtrl;
  CItemAtt *m_pattachment;
  CImgStatic m_stImgAttachment;

  CZoomSliderCtrl m_sldrZoom;
  CFloatEdit m_feZoomEdit;
  int m_iZoomValue;
};
