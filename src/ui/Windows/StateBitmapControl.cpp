/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "os/debug.h"

#include "StateBitmapControl.h"

#define TIMER_STATE_BITMAP_BLINK 0x0200

CStateBitmapControl::CStateBitmapControl()
  :
  m_pStateBitmaps(nullptr),
  m_nIdStateCurrent(BITMAP_ID_NONE),
  m_nIdStateBlinkFirst(BITMAP_ID_NONE),
  m_nIdStateBlinkLast(BITMAP_ID_NONE),
  m_nBlinkSeconds(BLINK_TIMEOUT_NONE),
  m_bBlink(false),
  m_pToolTipCtrl(nullptr),
  m_nIdToolTipText(TOOLTIP_TEXT_ID_NONE)
{
}

CStateBitmapControl::CStateBitmapControl(CStateBitmapManager* pBitmapManager, UINT nIdToolTipText)
  :
  CStateBitmapControl()
{
  m_pStateBitmaps = pBitmapManager;
  m_nIdStateCurrent = m_pStateBitmaps->GetFirstId();
  m_nIdToolTipText = nIdToolTipText;
}

CStateBitmapControl::CStateBitmapControl(
  UINT nIdBitmapFirst,
  UINT nIdBitmapLast,
  UINT nIdBitmapError,
  UINT nIdToolTipText,
  UINT rgbTransparentColor
)
  :
  CStateBitmapControl()
{
  m_pStateBitmaps = new CStateBitmapManager(nIdBitmapFirst, nIdBitmapLast, nIdBitmapError, rgbTransparentColor);
  m_nIdStateCurrent = nIdBitmapFirst;
  m_nIdToolTipText = nIdToolTipText;
}

CStateBitmapControl::~CStateBitmapControl()
{
  Deinitialize();
}

void CStateBitmapControl::Deinitialize()
{
  if (m_hWnd != NULL) {
    if (m_bBlink && ::IsWindow(m_hWnd))
      KillTimer(TIMER_STATE_BITMAP_BLINK);
    Detach();
  }
  delete m_pStateBitmaps;
  m_pStateBitmaps = nullptr;
  m_nIdStateCurrent = BITMAP_ID_NONE;
  m_nIdStateBlinkFirst = BITMAP_ID_NONE;
  m_nIdStateBlinkLast = BITMAP_ID_NONE;
  m_nBlinkSeconds = BLINK_TIMEOUT_NONE;
  m_bBlink = false;
  delete m_pToolTipCtrl;
  m_pToolTipCtrl = nullptr;
  m_nIdToolTipText = TOOLTIP_TEXT_ID_NONE;
}

void CStateBitmapControl::SetState(UINT nIdStateCurrent, UINT nIdStateBlinkFirst, UINT nIdStateBlinkLast, UINT nBlinkSeconds)
{
  ASSERT(nIdStateCurrent != BITMAP_ID_NONE);
  ASSERT(nIdStateCurrent >= m_pStateBitmaps->GetFirstId());
  ASSERT(nIdStateCurrent <= m_pStateBitmaps->GetLastId());
  ASSERT(
    (nIdStateBlinkFirst == BITMAP_ID_NONE && nIdStateBlinkLast == BITMAP_ID_NONE) ||
    (nIdStateBlinkFirst != BITMAP_ID_NONE && nIdStateBlinkLast != BITMAP_ID_NONE)
  );
  m_bBlink = nIdStateBlinkLast != BITMAP_ID_NONE;
  ASSERT(!m_bBlink || (nIdStateCurrent >= nIdStateBlinkFirst && nIdStateCurrent <= nIdStateBlinkLast));
  m_nIdStateCurrent = nIdStateCurrent;
  m_nIdStateBlinkFirst = nIdStateBlinkFirst;
  m_nIdStateBlinkLast = nIdStateBlinkLast;
  m_nBlinkSeconds = nBlinkSeconds;
  if (m_bBlink)
    SetTimer(TIMER_STATE_BITMAP_BLINK, BLINK_RATE_MSECS, NULL);
  else
    KillTimer(TIMER_STATE_BITMAP_BLINK);
  Invalidate();
  UpdateWindow();
}

UINT CStateBitmapControl::GetState(UINT* nIdStateBlinkFirst, UINT* nIdStateBlinkLast)
{
  if (nIdStateBlinkFirst)
    *nIdStateBlinkFirst = m_nIdStateBlinkFirst;
  if (nIdStateBlinkLast)
    *nIdStateBlinkLast = m_nIdStateBlinkLast;
  return m_nIdStateCurrent;
}

void CStateBitmapControl::SetToolTipText(LPCTSTR pszToolTipText)
{
  if (!m_pToolTipCtrl)
    return;
  m_pToolTipCtrl->AddTool(this, pszToolTipText);
  m_nIdToolTipText = TOOLTIP_TEXT_ID_NONE;
}

void CStateBitmapControl::SetToolTipText(UINT nIdToolTipText)
{
  if (!m_pToolTipCtrl) {
    // Not initialized yet, save ID for later.
    m_nIdToolTipText = nIdToolTipText;
    return;
  }

  if (nIdToolTipText == TOOLTIP_TEXT_ID_NONE)
    return;

  CString csTemp;
  csTemp.LoadString(nIdToolTipText);
  SetToolTipText(csTemp);
  // Save ID after SetToolTipText since it clears the ID.
  m_nIdToolTipText = nIdToolTipText;
}

BEGIN_MESSAGE_MAP(CStateBitmapControl, CStatic)
  //{{AFX_MSG_MAP(CStateBitmapControl)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CStateBitmapControl::PreSubclassWindow()
{
  OnSetInitialState();

  if (m_pToolTipCtrl)
    return;

  m_pToolTipCtrl = new CToolTipCtrl();
  if (!m_pToolTipCtrl->Create(GetParent(), TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"%S: m_pToolTipCtrl->Create failed.\n", __FUNCTION__);
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = nullptr;
  }
  else {
    m_pToolTipCtrl->Activate(TRUE);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 8000);
    m_pToolTipCtrl->SetMaxTipWidth(300);
    SetToolTipText(m_nIdToolTipText);
  }
}

BOOL CStateBitmapControl::PreTranslateMessage(MSG* pMsg)
{
  m_pToolTipCtrl->RelayEvent(pMsg);
  return CStatic::PreTranslateMessage(pMsg);
}

BOOL CStateBitmapControl::OnEraseBkgnd(CDC* pDC)
{
  CRect rc;
  GetClientRect(&rc);
  ::FillRect(pDC->GetSafeHdc(), rc, GetSysColorBrush(COLOR_3DFACE));
  return 1;
}

void CStateBitmapControl::OnPaint()
{
  CRect rect;
  GetClientRect(&rect);

  LONG bmWidthDpi; // use m_bmWidth, remove after testing.
  LONG bmHeightDpi;
  m_pStateBitmaps->GetBitmapInfo(m_nIdStateCurrent, nullptr, &bmWidthDpi, &bmHeightDpi);

  int ileft = rect.left + rect.Width() / 2 - bmWidthDpi / 2;
  int itop = rect.top + rect.Height() / 2 - bmHeightDpi / 2;
  PAINTSTRUCT ps;
  CDC& dc = *BeginPaint(&ps);
  m_pStateBitmaps->BitBltStateBitmap(m_nIdStateCurrent, ileft, itop, dc);
  EndPaint(&ps);
}

void CStateBitmapControl::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == TIMER_STATE_BITMAP_BLINK) {

    if (m_nIdStateBlinkFirst == BITMAP_ID_NONE || m_nIdStateBlinkLast == BITMAP_ID_NONE) {
      KillTimer(TIMER_STATE_BITMAP_BLINK);
      return;
    }

    if (++m_nIdStateCurrent > m_nIdStateBlinkLast)
      m_nIdStateCurrent = m_nIdStateBlinkFirst;

    if (m_nBlinkSeconds != BLINK_TIMEOUT_NONE) {
      m_nBlinkSeconds--;
      if (m_nBlinkSeconds == BLINK_TIMEOUT_NONE) {
        KillTimer(TIMER_STATE_BITMAP_BLINK);
        m_nIdStateCurrent = m_nIdStateBlinkFirst;
      }
    }

    Invalidate(TRUE);
    UpdateWindow();
  }
}
