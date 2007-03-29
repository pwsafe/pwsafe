/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

// CColumnChooserLC.cpp : implementation file

// CColumnChooserLC (Coloumn Chooser ListCtrl)

#include "stdafx.h"
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h >       // MFC OLE automation classes
#include "ColumnChooserLC.h"
#include "DboxMain.h"       // For WM_HDR_DD_COMPLETE and enum FROMCC & FROMHDR
#include "PasswordSafe.h"   // for access to external gbl_ccddCPFID

// CColumnChooserLC

CColumnChooserLC::CColumnChooserLC()
 : m_iItem(-1), m_pDragImage(NULL)
{
}

CColumnChooserLC::~CColumnChooserLC()
{
}

BEGIN_MESSAGE_MAP(CColumnChooserLC, CListCtrl)
  //{{AFX_MSG_MAP(CColumnChooserLC)
  ON_WM_LBUTTONDOWN()
  ON_WM_DESTROY()
  ON_WM_ERASEBKGND()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CColumnChooserLC message handlers

DROPEFFECT CColumnChooserLC::OnDragEnter(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                  DWORD /* dwKeyState */, CPoint /* point */ )
{
  // We only allow MOVE - not COPY
  return DROPEFFECT_MOVE;
}

DROPEFFECT CColumnChooserLC::OnDragOver(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                  DWORD /* dwKeyState */, CPoint /* point */)
{
  // We only allow MOVE - not COPY
  return DROPEFFECT_MOVE;
}

BOOL CColumnChooserLC::OnDrop(CWnd* /* pWnd */, COleDataObject* pDataObject,
                              DROPEFFECT /* dropEffect */, CPoint /* point */)
{
  // On Drop of column from Header onto Column Chooser Dialog
  HGLOBAL hGlobal;

  hGlobal = pDataObject->GetGlobalData(gbl_ccddCPFID);

  LPCSTR pData = (LPCSTR)GlobalLock(hGlobal);
  ASSERT(pData != NULL);

  DWORD randID;
  int iDDType, dw_type, iLen;

#if _MSC_VER >= 1400
  _stscanf_s(pData, _T("%08x%02x%02x%04x"), &randID, &iDDType, &dw_type, &iLen);
#else
  _stscanf(pData, _T("08x%02x%02x%04x"), &randID, &iDDType, &dw_type, &iLen);
#endif

  // Check if it is ours?
  // - we don't accept drop from other instances of PWS
  if (randID != gbl_randID)
    return FALSE;

  // Check if it is from List View HeaderCtrl?
  // - we don't accept drop from anything else
  if (iDDType != FROMHDR)
    return FALSE;

  // Now add it
  const CString cs_header(pData + 16, iLen);
  int iItem = InsertItem(0, cs_header);
  SetItemData(iItem, dw_type);
  SortItems(CCLCCompareProc, (LPARAM)this);

  GlobalUnlock(hGlobal);

  GetParent()->SetFocus();
  return TRUE;
}

void CColumnChooserLC::OnLButtonDown(UINT nFlags, CPoint point)
{
  CListCtrl::OnLButtonDown(nFlags, point);

  // Move our text
  m_iItem = HitTest(point);
  if (m_iItem == -1)
    return;

  // Start of Drag of column (m_iItem) from Column Chooser dialog to.....
  CString cs_text;
  DWORD dw_type;

  dw_type = GetItemData(m_iItem);

  // ListView HeaderCtrl only needs the type as it uses main routine
  // to add/delete columns via SendMessage
  cs_text.Format("%08x%02x%02x", gbl_randID, FROMCC, dw_type);

  // Get client window position
  CPoint currentClientPosition;
  currentClientPosition = ::GetMessagePos();
  ScreenToClient(&currentClientPosition);

  // Set drag image
  m_pDragImage = CreateDragImage(m_iItem, &currentClientPosition);
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
  delete m_pDragImage;
}

void CColumnChooserLC::CompleteMove()
{
  // After we have dragged successfully from Column Chooser to Header
  if (m_iItem < 0)
    return;

  DeleteItem(m_iItem);
  m_iItem = -1;
}

void CColumnChooserLC::OnDestroy()
{
  m_CCDropTarget.Revoke();
}

// Sort the items based on iType
int CALLBACK CColumnChooserLC::CCLCCompareProc(LPARAM lParam1, LPARAM lParam2,
                                               LPARAM /* lParamSort */)
{
   // lParamSort contains a pointer to the list view control.
   // The lParam of an item is its type.
  if (lParam1 < lParam2)
    return -1;
  else if (lParam1 > lParam2)
    return 1;
  else
    return 0;  // should never happen!!!
}

BOOL CColumnChooserLC::OnEraseBkgnd(CDC* pDC)
{
  if (GetItemCount() <= 0) {
    int nSavedDC = pDC->SaveDC(); //save the current DC state

    // Set up variables
    COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);  //system text color
    COLORREF clrBack = ::GetSysColor(COLOR_WINDOW);    //system background color
    CBrush cbBack(clrBack);

    CRect rc;
    GetClientRect(&rc);  //get client area of the ListCtrl

    // If there is a header, we need to account for the space it occupies
    CHeaderCtrl* pHC = GetHeaderCtrl();
    if (pHC != NULL) {
      CRect rcH;
      pHC->GetClientRect(&rcH);
      rc.top += rcH.bottom;
    }

   // Here is the string we want to display (or you can use a StringTable entry
   const CString cs_emptytext(MAKEINTRESOURCE(IDS_NOITEMS));

   // Now we actually display the text
   // set the text color
   pDC->SetTextColor(clrText);
   // set the background color
   pDC->SetBkColor(clrBack);
   // fill the client area rect
   pDC->FillRect(&rc, &cbBack);
   // select a font
   pDC->SelectStockObject(ANSI_VAR_FONT);
   // and draw the text
   pDC->DrawText(cs_emptytext, -1, rc,
                 DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);

   // Restore dc
   pDC->RestoreDC(nSavedDC);
   ReleaseDC(pDC);
  } else {
    //  If there are items in the ListCtrl, we need to call the base class function
    CListCtrl::OnEraseBkgnd(pDC);
  }

  return true;
}
