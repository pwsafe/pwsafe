/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// LVHdrCtrl

#include "DropTarget.h"
#include "DropSource.h"

class CLVHdrCtrl : public CHeaderCtrl, public CDropTarget, public CDataSource
{
public:
  CLVHdrCtrl();
  ~CLVHdrCtrl();

  BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
    DROPEFFECT dropEffect, CPoint point);
  DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
    DWORD dwKeyState, CPoint point);
  DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, 
    DWORD dwKeyState, CPoint point);

  void SetLVState(const BOOL bCCActive) {m_bCCActive = bCCActive;}
  BOOL GetLVState() {return m_bCCActive;}

public:
  void operator delete(void* p)
  { CDropTarget::operator delete(p); }

protected:
  virtual void CompleteMove();

  //{{AFX_MSG(CLVHdrCtrl)
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CDataSource *m_pHdrDataSource;
  CDropTarget *m_pHdrDropTarget;
  COleDropSource *m_pHdrDropSource;

  CImageList* m_pDragImage;
  LPARAM m_dwHDRType;
  int m_iDDType;
  BOOL m_bCCActive;
  // Clipboard format for Column Chooser Drag & Drop
  CLIPFORMAT m_ccddCPFID;
};
