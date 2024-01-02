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

#include "StateBitmapControl.h"

class CScreenCaptureStateControl : public CStateBitmapControl
{
public:
  static const UINT BLINK_DURATION_SECONDS = 10;
public:
  CScreenCaptureStateControl();
  static UINT GetCurrentCaptureStateToolTipStringId();
  static UINT GetCurrentCaptureStateBitmapId();
  static void SetLastDisplayAffinityError(DWORD dwError);
  static DWORD GetLastDisplayAffinityError();
protected:
  //{{AFX_MSG(CScreenCaptureStateControl)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
protected:
  virtual void OnSetInitialState();
protected:
  static DWORD m_dwLastSetAffinityError;
};
