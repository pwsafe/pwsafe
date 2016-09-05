/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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

  //Frees the image data
  void ClearImage();

protected:
  virtual void PreSubclassWindow();
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

  //{{AFX_MSG(CImgStatic)
  afx_msg BOOL OnEraseBkgnd(CDC *pDC);
  afx_msg void OnDropFiles(HDROP hDropInfo);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  void FreeStream();

  IStream *m_pStream;
  bool m_bImageLoaded;
  ULONG_PTR m_gdiplusToken;
};
