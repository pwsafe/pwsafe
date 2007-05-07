/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

// LVHdrCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h >       // MFC OLE automation classes
#include "LVHdrCtrl.h"
#include "DboxMain.h"       // For WM_CCTOHDR_DD_COMPLETE and enum FROMCC & FROMHDR
#include "PasswordSafe.h"   // For global variables gbl_ccddCPFID and gbl_classname

// LVHdrCtrl

CLVHdrCtrl::CLVHdrCtrl()
 : m_iHDRType(-1), m_pDragImage(NULL), m_bCCActive(FALSE)
{
}

CLVHdrCtrl::~CLVHdrCtrl()
{
  m_HdrDropTarget.Revoke();
}

BEGIN_MESSAGE_MAP(CLVHdrCtrl, CHeaderCtrl)
  //{{AFX_MSG_MAP(CLVHdrCtrl)
  ON_WM_LBUTTONDOWN()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLVHdrCtrl message handlers

DROPEFFECT CLVHdrCtrl::OnDragEnter(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                  DWORD /* dwKeyState */, CPoint /* point */ )
{
  // We only allow MOVE - not COPY
  return DROPEFFECT_MOVE;
}

DROPEFFECT CLVHdrCtrl::OnDragOver(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                  DWORD /* dwKeyState */, CPoint /* point */)
{
  // We only allow MOVE - not COPY
  return DROPEFFECT_MOVE;
}

BOOL CLVHdrCtrl::OnDrop(CWnd* /* pWnd */, COleDataObject* pDataObject,
                        DROPEFFECT /* dropEffect */, CPoint /* point */)
{
  // On Drop of column from Column Chooser Dialog onto Header
  if (!pDataObject->IsDataAvailable(gbl_ccddCPFID, NULL))
    return FALSE;

  HGLOBAL hGlobal;
  hGlobal = pDataObject->GetGlobalData(gbl_ccddCPFID);

  LPCTSTR pData = (LPCTSTR)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  SIZE_T memsize = GlobalSize(hGlobal);

  if (memsize < DD_MEMORY_MINSIZE)
    goto ignore;

  // Check if it is ours?
  // - we don't accept drop from other instances of PWS
  if (memcmp(gbl_classname, pData, sizeof(gbl_classname) - 1) != 0)
    goto ignore;

  int iDDType(0), iLen, iType;

#if _MSC_VER >= 1400
  _stscanf_s(pData + sizeof(gbl_classname) - 1, _T("%02x%04x%04x"), &iDDType, &iType, &iLen);
#else
  _stscanf(pData + sizeof(gbl_classname) - 1, _T("%02x%04x%04x"), &iDDType, &iType, &iLen);
#endif

  // - we only accept drops from our ColumnChooser or our Header
  // - standard moving within the header only available if CC dialog not visible
  if ((iDDType != FROMCC) || ((long)memsize < (DD_MEMORY_MINSIZE + iLen)))
    goto ignore;

  // Get index of column we are on
  HDHITTESTINFO hdhti;
  hdhti.pt = CPoint(::GetMessagePos());
  hdhti.flags = 0;
  ScreenToClient(&hdhti.pt);
  ::SendMessage(this->GetSafeHwnd(), HDM_HITTEST, 0, (LPARAM) &hdhti);

  // Now add it
  ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
      WM_CCTOHDR_DD_COMPLETE, (WPARAM)iType, (LPARAM)hdhti.iItem);

  GlobalUnlock(hGlobal);

  GetParent()->SetFocus();
  return TRUE;

ignore:
  GlobalUnlock(hGlobal);
  return FALSE;
}

void CLVHdrCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  CHeaderCtrl::OnLButtonDown(nFlags, point);

  if (!m_bCCActive)
    return;

  // Start of Drag a column (m_iHDRType) from Header to .....

  // Get client window position
  CPoint currentClientPosition = ::GetMessagePos();
  ScreenToClient(&currentClientPosition);

  // Get index of column we are on
  HDHITTESTINFO hdhti;
  hdhti.pt = currentClientPosition;
  hdhti.flags = 0;
  ::SendMessage(this->GetSafeHwnd(), HDM_HITTEST, 0, (LPARAM) &hdhti);

  // Get column name and type
  HD_ITEM hdi;
  hdi.mask = HDI_WIDTH | HDI_LPARAM | HDI_TEXT;
  enum { sizeOfBuffer = 256 };
  TCHAR lpBuffer[sizeOfBuffer];
  hdi.pszText = lpBuffer;
  hdi.cchTextMax = sizeOfBuffer;

  GetItem(hdhti.iItem, &hdi);
  m_iHDRType = hdi.lParam;

  // Can't play with TITLE or USER
  if (m_iHDRType == CItemData::TITLE || m_iHDRType == CItemData::USER)
    return;

  // Get the data: ColumnChooser Listbox needs the column string
  const int iLen = _tcslen(lpBuffer);
  CString cs_text;
  cs_text.Format(_T("%s%02x%04x%04x%s"), gbl_classname, FROMHDR, m_iHDRType, iLen, lpBuffer);

  // Set drag image
  m_pDragImage = CreateDragImage(hdhti.iItem);
  m_pDragImage->BeginDrag(0, CPoint(8, 8));
  m_pDragImage->DragEnter(GetDesktopWindow(), point);

  // Get client rectangle
  RECT rClient;
  GetClientRect(&rClient);

  // Start dragging
  StartDragging(cs_text, cs_text.GetLength(), gbl_ccddCPFID, &rClient, &point);

  // End dragging image
  m_pDragImage->DragLeave(GetDesktopWindow());
  m_pDragImage->EndDrag();
  //delete m_pDragImage;
}

void CLVHdrCtrl::CompleteMove()
{
  // After we have dragged successfully from Header to Column Chooser
  // Now delete it
  ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
      WM_HDRTOCC_DD_COMPLETE, (WPARAM)m_iHDRType, (LPARAM)0);
}
