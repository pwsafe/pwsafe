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

class CColumnChooserLC : public CListCtrl, public CDropTarget, public CDropSource
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

public:
   void operator delete(void* p)
        { CDropTarget::operator delete(p); }

protected:
  virtual void CompleteMove();
  static int CALLBACK CCLCCompareProc(LPARAM lParam1, LPARAM lParam2,
                                      LPARAM lParamSort);
  //{{AFX_MSG(CColumnChooserLC)
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnDestroy();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CDropSource m_CCDropSource;
  CDropTarget m_CCDropTarget;
  CImageList* m_pDragImage;
  int m_iItem;
};
