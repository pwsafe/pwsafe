/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file ImgStatic.h
//-----------------------------------------------------------------------------

#pragma once

#include "StdAfx.h"

class CImgStatic : public CStatic
{
public:
  CImgStatic();
  ~CImgStatic();

  bool IsImageLoaded() { return m_bImageLoaded; }
  HRESULT Load(CString &szFilePath);
  HRESULT Load(IStream* piStream);

  void EnableScrollBars(const bool bEnable) { m_bUseScrollBars = bEnable; }
  void SetZoomFactor(int iZoom);
  void ClearImage();
  void IssueError(int rc, HRESULT hr);

protected:
  virtual void PreSubclassWindow();
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

  //{{AFX_MSG(CImgStatic)
  afx_msg BOOL OnEraseBkgnd(CDC *pDC);
  afx_msg void OnDropFiles(HDROP hDropInfo);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void FreeStream();

  CScrollBar m_HScroll, m_VScroll;
  IStream *m_pStream;
  bool m_bInitDone, m_bImageLoaded, m_bUseScrollBars, m_bZooming;
  ULONG_PTR m_gdiplusToken;
  int m_iZoomFactor, m_iHPos, m_iVPos;
};
