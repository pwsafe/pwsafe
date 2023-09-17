/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "ThisMfcApp.h"
#include "resource.h"
#include "resource3.h"
#include "PWSprefs.h"
#include "ExcludeCaptureStateBitmapManager.h"

#include "ScreenCaptureStateControl.h"

CScreenCaptureStateControl::CScreenCaptureStateControl()
  :
  CStateBitmapControl(new CExcludeCaptureStateBitmapManager())
{
}

UINT CScreenCaptureStateControl::GetCurrentCaptureStateToolTipStringId()
{
  bool bExcludeFromScreenCapture = PWSprefs::GetInstance()->GetPref(PWSprefs::ExcludeFromScreenCapture);
  if (!bExcludeFromScreenCapture)
    return IDS_SCRCAP_TT_ALLOWED_DBPREF;
  else if (app.ForceAllowScreenCapture())
    return IDS_SCRCAP_TT_OVERRIDE_CMDLINE;
  else 
    return IDS_SCRCAP_TT_DISALLOWED_DEFAULT;
}

void CScreenCaptureStateControl::OnSetInitialState()
{
  bool bExcludeFromScreenCapture = PWSprefs::GetInstance()->GetPref(PWSprefs::ExcludeFromScreenCapture);

  UINT nIdToolTipString = GetCurrentCaptureStateToolTipStringId();

  if (!bExcludeFromScreenCapture) {
    SetState(IDB_SCRCAP_ALLOWED);
    SetToolTipText(nIdToolTipString);
    return;
  }

  if (app.ForceAllowScreenCapture()) {
    SetState(IDB_SCRCAP_ALLOWED_FORCED1, IDB_SCRCAP_ALLOWED_FORCED1, IDB_SCRCAP_ALLOWED_FORCED2, BLINK_DURATION_SECONDS);
    SetToolTipText(nIdToolTipString);
  } else {
    SetState(IDB_SCRCAP_EXCLUDED);
    SetToolTipText(nIdToolTipString);
  }
}

BEGIN_MESSAGE_MAP(CScreenCaptureStateControl, CStateBitmapControl)
  //{{AFX_MSG_MAP(CStateBitmapControl)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
