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
#include "corelib/util.h"

// LVHdrCtrl

CLVHdrCtrl::CLVHdrCtrl()
 : m_iHDRType(-1), m_bCCActive(FALSE)
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
  // *****
  // NOTE: Even in a Unicode build, we work in "unsigned char"
  //       as all fields are in Latin alphabet PWS + a-z + 0-9!
  // *****

  // On Drop of column from Column Chooser Dialog onto Header
  if (!pDataObject->IsDataAvailable(gbl_ccddCPFID, NULL))
    return FALSE;

  HGLOBAL hGlobal;
  hGlobal = pDataObject->GetGlobalData(gbl_ccddCPFID);

  BYTE *pData = (BYTE *)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  SIZE_T memsize = GlobalSize(hGlobal);

#ifdef _DEBUG
  // In Column D&D, a trailing NULL is appended to the data to allow tracing of pData
  // without error
   CString cs_timestamp;
   cs_timestamp = PWSUtil::GetTimeStamp();
   TRACE(_T("%s: LV::Drop: length %d/0x%04x, value:\n"), cs_timestamp, memsize, memsize);
   PWSUtil::HexDump(pData, memsize, cs_timestamp);
#endif /* DEBUG */

  if (memsize < DD_MEMORY_MINSIZE)
    goto ignore;

  // Check if it is ours?
  // - we don't accept drop from other instances of PWS
  if (memcmp(gbl_classname, pData, DD_CLASSNAME_SIZE) != 0)
    goto ignore;

  // iDDType = D&D type FROMCC, FROMHDR or for entry D&D only FROMTREE
  // iType   = Column type (as defined in CItemData::GROUP etc.
  int iDDType(0), iType;

#if _MSC_VER >= 1400
  sscanf_s((char *)pData + DD_CLASSNAME_SIZE, "%02x%04x", &iDDType, &iType);
#else
  sscanf((char *)pData + DD_CLASSNAME_SIZE, "%02x%04x", &iDDType, &iType);
#endif

  // - we only accept drops from our ColumnChooser or our Header
  // - standard moving within the header only available if CC dialog not visible
  if (iDDType != FROMCC)
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
  CPoint currentClientPosition;
  currentClientPosition = ::GetMessagePos();
  ScreenToClient(&currentClientPosition);

  // Get index of column we are on
  HDHITTESTINFO hdhti;
  hdhti.pt = currentClientPosition;
  hdhti.flags = 0;
  ::SendMessage(this->GetSafeHwnd(), HDM_HITTEST, 0, (LPARAM) &hdhti);

  // Get column type
  HD_ITEM hdi;
  hdi.mask = HDI_LPARAM;

  GetItem(hdhti.iItem, &hdi);
  m_iHDRType = hdi.lParam;

  // Can't play with TITLE or USER
  if (m_iHDRType == CItemData::TITLE || m_iHDRType == CItemData::USER)
    return;

  BYTE *pData = new BYTE[DD_MEMORY_MINSIZE + 2];
  memset((void *)pData, 0x00, DD_MEMORY_MINSIZE + 2);
#if _MSC_VER >= 1400
  sprintf_s((char *)pData, DD_MEMORY_MINSIZE, "%s", gbl_classname);
  sprintf_s((char *)pData + DD_CLASSNAME_SIZE, DD_REQUIRED_DATA_SIZE + 1,
            "%02x%04x0000", FROMHDR, m_iHDRType);
#else
  sprintf((char *)pData, "%s", gbl_classname);
  sprintf((char *)pData + DD_CLASSNAME_SIZE, "%02x%04x0000", FROMHDR, m_iHDRType);
#endif

#ifdef _DEBUG
  // In Column D&D, a trailing NULL is appended to the data to allow tracing of pData
  // without error
   CString cs_timestamp;
   cs_timestamp = PWSUtil::GetTimeStamp();
   TRACE(_T("%s: LV::LBD: length %d/0x%04x, value:\n"), cs_timestamp,
     DD_MEMORY_MINSIZE, DD_MEMORY_MINSIZE);
   PWSUtil::HexDump(pData, DD_MEMORY_MINSIZE, cs_timestamp);
#endif /* DEBUG */

  // Set drag image
  CImageList* pDragImageList;
  pDragImageList = CreateDragImage(hdhti.iItem);
  pDragImageList->BeginDrag(0, CPoint(8, 8));
  pDragImageList->DragEnter(GetDesktopWindow(), point);

  // Get client rectangle
  RECT rClient;
  GetClientRect(&rClient);

  // Start dragging - note double trailing NULL in length parameter
  StartDragging(pData, DD_MEMORY_MINSIZE + 2,
                gbl_ccddCPFID, &rClient, &point);

  // End dragging image
  pDragImageList->DragLeave(GetDesktopWindow());
  pDragImageList->EndDrag();
  // For some reason uncommenting this damages the heap but....
  // The are no reported memory leaks and it works OK in
  // ColumnChooserLC.cpp and TVTreeCtrl.cpp ???
  //delete pDragImageList;
  free(pData);
}

void CLVHdrCtrl::CompleteMove()
{
  // After we have dragged successfully from Header to Column Chooser
  // Now delete it
  ::SendMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
      WM_HDRTOCC_DD_COMPLETE, (WPARAM)m_iHDRType, (LPARAM)0);
}
