/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "afxole.h"

/////////////////////////////////////////////////////////////////////////////
// COleDropWndTarget window

class CDropTarget : public COleDropTarget
{
  // Construction
public:
  CDropTarget();

  // Implementation
public:
  BOOL Initialize(CWnd* wnd);
  void Terminate();
  virtual ~CDropTarget();

  virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
    DWORD dwKeyState, CPoint point);
  virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, 
    DWORD dwKeyState, CPoint point);
  virtual void OnDragLeave(CWnd* pWnd);
  virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, 
    DROPEFFECT dropEffect, CPoint point);

private:
  BOOL m_bRegistered;
};
