/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "afxole.h"
#include "DropTarget.h"

CDropTarget::CDropTarget()
: m_bRegistered(FALSE)
{}

CDropTarget::~CDropTarget() {}

DROPEFFECT CDropTarget::OnDragEnter(CWnd* /* pWnd */,
                                    COleDataObject* /* pDataObject */, DWORD dwKeyState, CPoint /* point */ )
{
  if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
    return DROPEFFECT_COPY; // Copy the source
  else
    return DROPEFFECT_MOVE; // Move the source
}

void CDropTarget::OnDragLeave(CWnd* pWnd)
{
  COleDropTarget::OnDragLeave(pWnd);
}

DROPEFFECT CDropTarget::OnDragOver(CWnd* /* pWnd */,
                                   COleDataObject* /* pDataObject */, DWORD dwKeyState, CPoint /* point */)
{
  if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
    return DROPEFFECT_COPY;
  else
    return DROPEFFECT_MOVE;
}

BOOL CDropTarget::OnDrop(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                         DROPEFFECT /* dropEffect */, CPoint /* point */)
{
  return TRUE;
}

BOOL CDropTarget::Initialize(CWnd* wnd)
{
  if (m_bRegistered == TRUE)
    return FALSE;

  m_bRegistered = Register(wnd);
  return m_bRegistered;
}

void CDropTarget::Terminate()
{
  m_bRegistered = FALSE;
  Revoke();
}
