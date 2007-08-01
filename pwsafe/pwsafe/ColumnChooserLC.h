/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#pragma once

// CColumnChooserLC (Coloumn Chooser "Drag" ListCtrl)

#include "DropTarget.h"
#include "DropSource.h"

class CColumnChooserLC : public CListCtrl, public CDropTarget, public CDataSource
{
public:
  CColumnChooserLC();
  ~CColumnChooserLC();

  BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
    DROPEFFECT dropEffect, CPoint point);
  DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
    DWORD dwKeyState, CPoint point);
  DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, 
    DWORD dwKeyState, CPoint point);

  static int CALLBACK CColumnChooserLC::CCLCCompareProc(LPARAM , LPARAM , LPARAM );

public:
   void operator delete(void* p)
        { CDropTarget::operator delete(p); }

protected:
  virtual void CompleteMove();
  //{{AFX_MSG(CColumnChooserLC)
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnDestroy();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CDataSource m_CCDataSource;
  CDropTarget m_CCDropTarget;
  CImageList* m_pDragImage;
  int m_iItem;
  // Clipboard format for Column Chooser Drag & Drop
  CLIPFORMAT m_ccddCPFID;
};
