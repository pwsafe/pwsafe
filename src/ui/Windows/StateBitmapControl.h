/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// StateBitmapControl.h
//-----------------------------------------------------------------------------

#include "StateBitmapManager.h"

class CStateBitmapControl : public CStatic
{
public:
  static const UINT BITMAP_ID_NONE = 0;
  static const UINT TOOLTIP_TEXT_ID_NONE = 0;
  static const UINT BLINK_RATE_MSECS = 530;
  static const UINT BLINK_TIMEOUT_NONE = 0;
public:
  CStateBitmapControl(CStateBitmapManager* pBitmapManager, UINT nIdToolTipText = TOOLTIP_TEXT_ID_NONE);
  CStateBitmapControl(
    UINT nIdBitmapFirst,
    UINT nIdBitmapLast,
    UINT nIdBitmapError,
    UINT nIdToolTipText,
    UINT rgbTransparentColor = CStateBitmapManager::RGB_COLOR_NOT_TRANSPARENT
  );
  virtual ~CStateBitmapControl();
  void SetState(
    UINT nIdStateCurrent,
    UINT nIdStateBlinkFirst = BITMAP_ID_NONE,
    UINT nIdStateBlinkLast = BITMAP_ID_NONE,
    UINT nBlinkSeconds = BLINK_TIMEOUT_NONE
  );
  UINT GetState(UINT* nIdStateBlinkFirst = nullptr, UINT* nIdStateBlinkLast = nullptr);
  void SetToolTipText(LPCTSTR pszToolTipText);
  void SetToolTipText(UINT nIdToolTipText);
protected:
  virtual void OnSetInitialState() {}
  virtual void PreSubclassWindow();
  virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
  //{{AFX_MSG(CStateBitmapControl)
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
private:
  CStateBitmapControl();
  void Deinitialize();
private:
  CStateBitmapManager* m_pStateBitmaps;
  UINT m_nIdStateCurrent;
  UINT m_nIdStateBlinkFirst;
  UINT m_nIdStateBlinkLast;
  UINT m_nBlinkSeconds;
  bool m_bBlink;
  CToolTipCtrl* m_pToolTipCtrl;
  UINT m_nIdToolTipText;
};
