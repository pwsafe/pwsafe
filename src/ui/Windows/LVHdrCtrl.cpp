/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// LVHdrCtrl.cpp : implementation file
//

#include "stdafx.h"

#include "LVHdrCtrl.h"

#include "resource3.h"

#include "core/itemdata.h" // For CItemData::UUID

#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h >       // MFC OLE automation classes

// LVHdrCtrl

CLVHdrCtrl::CLVHdrCtrl()
  : m_dwHDRType(-1), m_pDragImage(NULL), m_bCCActive(FALSE)
{
  // Register a clipboard format for column drag & drop. 
  // Note that it's OK to register same format more than once:
  // "If a registered format with the specified name already exists,
  // a new format is not registered and the return value identifies the existing format."

  CString cs_CPF(MAKEINTRESOURCE(IDS_CPF_CDD));
  m_ccddCPFID = (CLIPFORMAT)RegisterClipboardFormat(cs_CPF);
  ASSERT(m_ccddCPFID != 0);

  m_pHdrDataSource = new CDataSource();
  m_pHdrDropTarget = new CDropTarget();
  m_pHdrDropSource = new COleDropSource();
}

CLVHdrCtrl::~CLVHdrCtrl()
{
  m_pHdrDropTarget->Revoke();

  // Don't delete m_pDataSource but first release all references and
  // this routine will delete it when the references get to 0.
  m_pHdrDataSource->InternalRelease();

  // delete the Drop Target & Source
  delete m_pHdrDropTarget;
  delete m_pHdrDropSource;
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
  if (!pDataObject->IsDataAvailable(m_ccddCPFID, NULL))
    return FALSE;

  HGLOBAL hGlobal;
  hGlobal = pDataObject->GetGlobalData(m_ccddCPFID);

  LPCWSTR pData = (LPCWSTR)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  DWORD procID;
  int iDDType, iType;

  swscanf_s(pData, L"%08x%02x%02x", &procID, &iDDType, &iType);

  // Check if it is ours?
  // - we don't accept drop from other instances of PWS
  // - we only accept drops from our ColumnChooser or our Header
  // - standard moving within the header only available if CC dialog not visible
  if ((procID != GetCurrentProcessId()) || (iDDType != FROMCC)) {
    GlobalUnlock(hGlobal);
    return FALSE;
  }

  int iAfterIndex;
  if (iType != CItemData::UUID) {
    // Get index of column we are on
    HDHITTESTINFO hdhti;
    hdhti.pt = CPoint(::GetMessagePos());
    hdhti.flags = 0;
    ScreenToClient(&hdhti.pt);
    ::SendMessage(this->GetSafeHwnd(), HDM_HITTEST, 0, (LPARAM) &hdhti);
    iAfterIndex = hdhti.iItem;
  } else
    iAfterIndex = 0;

  // Now add it but only if not trying to place to the left of the icon
  BOOL brc = TRUE;
  HDITEM hdi = {0};
  hdi.mask = HDI_LPARAM;
  GetItem(0, &hdi);
  if (hdi.lParam == CItemData::UUID && iAfterIndex == 0)
    brc = FALSE;
  else
    ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
                  PWS_MSG_CCTOHDR_DD_COMPLETE, (WPARAM)iType, (LPARAM)iAfterIndex);

  GlobalUnlock(hGlobal);

  GetParent()->SetFocus();
  return brc;
}

void CLVHdrCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  CHeaderCtrl::OnLButtonDown(nFlags, point);

  if (!m_bCCActive)
    return;

  // Start of Drag a column (m_dwHDRType) from Header to .....

  // Get client window position
  CPoint currentClientPosition;
  currentClientPosition = ::GetMessagePos();
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
  wchar_t lpBuffer[sizeOfBuffer];
  hdi.pszText = lpBuffer;
  hdi.cchTextMax = sizeOfBuffer;

  GetItem(hdhti.iItem, &hdi);
  m_dwHDRType = hdi.lParam;

  // Can't play with TITLE or USER
  if (m_dwHDRType == CItemData::TITLE || m_dwHDRType == CItemData::USER)
    return;

  // Get the data: ColumnChooser Listbox needs the column string
  const size_t iLen = wcslen(lpBuffer);
  CString cs_text;
  cs_text.Format(L"%08x%02x%02x%04x%s", GetCurrentProcessId(),
    FROMHDR, m_dwHDRType, iLen, lpBuffer);

  // Set drag image
  m_pDragImage = CreateDragImage(hdhti.iItem);
  m_pDragImage->BeginDrag(0, CPoint(8, 8));
  m_pDragImage->DragEnter(GetDesktopWindow(), point);

  // Get client rectangle
  RECT rClient;
  GetClientRect(&rClient);

  // Start dragging
  StartDragging((BYTE *)LPCWSTR(cs_text), cs_text.GetLength() * sizeof(wchar_t),
    m_ccddCPFID, &rClient, &point);

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
                PWS_MSG_HDRTOCC_DD_COMPLETE, (WPARAM)m_dwHDRType, (LPARAM)0);
}
