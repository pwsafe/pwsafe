/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SyncScrollRichEdit.cpp : implementation file
//

#include "stdafx.h"

#include "SyncScrollRichEdit.h"

#include "os/debug.h"

// CSyncScrollRichEdit

CSyncScrollRichEdit::CSyncScrollRichEdit()
  : m_nSBPos(-1), m_nSBTrackPos(-1)
{
}

CSyncScrollRichEdit::~CSyncScrollRichEdit()
{
}

BEGIN_MESSAGE_MAP(CSyncScrollRichEdit, CRichEditCtrlExtn)
  ON_WM_KEYDOWN()
  ON_WM_MOUSEWHEEL()
  ON_WM_VSCROLL()
END_MESSAGE_MAP()

// CSyncScrollRichEdit message handlers

void CSyncScrollRichEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

  if (nChar == VK_DOWN || nChar == VK_UP || nChar == VK_PRIOR || nChar == VK_NEXT) {
    int iFirstVisibleLine = GetFirstVisibleLine();

    m_pPartner->PartnerOnKeyDown(iFirstVisibleLine);
  }
}

BOOL CSyncScrollRichEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  BOOL brc = CRichEditCtrl::OnMouseWheel(nFlags, zDelta, pt);

  // pt are screen coordinates - convert to client to pass to partner
  CPoint pt_partner(pt);
  ScreenToClient(&pt_partner);

  m_pPartner->PartnerOnMouseWheel(nFlags, zDelta, pt_partner);

  return brc;
}

void CSyncScrollRichEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  CRichEditCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

  SCROLLINFO si = { 0 };
  si.cbSize = sizeof(SCROLLINFO);
  GetScrollInfo(SB_VERT, &si, SIF_ALL);

  m_pPartner->PartnerOnVScroll(nSBCode, nPos, pScrollBar, &si);
}

void CSyncScrollRichEdit::PartnerOnKeyDown(int iFirstVisibleLine)
{
  int nFirstVisibleLine = GetFirstVisibleLine();

  if (nFirstVisibleLine != iFirstVisibleLine) {
    LineScroll(iFirstVisibleLine - nFirstVisibleLine);
  }
}

BOOL CSyncScrollRichEdit::PartnerOnMouseWheel(UINT nFlags, short zDelta, CPoint pt_partner)
{
  // pt_partner is relative to client - convert to screen
  CPoint pt(pt_partner);
  ClientToScreen(&pt);

  BOOL brc = CRichEditCtrl::OnMouseWheel(nFlags, zDelta, pt);

  return brc;
}

void CSyncScrollRichEdit::PartnerOnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar, SCROLLINFO *psi)
{
  CScrollBar *pSB(NULL);
  if (pScrollBar != NULL) {
    pSB = GetScrollBarCtrl(SB_VERT);
  }

  CRichEditCtrl::OnVScroll(nSBCode, nPos, pSB);

  // Need to move scroll bar - user may have dragged scrol box or
  // clicked on the scroll bar - handle both
  if (m_nSBPos != psi->nPos) {
    SetScrollPos(SB_VERT, psi->nPos, TRUE);
    m_nSBPos = psi->nPos;
  }

  if (m_nSBTrackPos != psi->nTrackPos) {
    SetScrollPos(SB_VERT, psi->nTrackPos, TRUE);
    m_nSBTrackPos = psi->nTrackPos;
  }
}
