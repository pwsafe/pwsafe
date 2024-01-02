/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

DWORD CScreenCaptureStateControl::m_dwLastSetAffinityError = ERROR_SUCCESS;

void CScreenCaptureStateControl::SetLastDisplayAffinityError(DWORD dwError)
{
  m_dwLastSetAffinityError = dwError;
  // In pwsafe's usage of the OS SetWindowAffinity API, failure is not expected.
  ASSERT(m_dwLastSetAffinityError == ERROR_SUCCESS);
}

DWORD CScreenCaptureStateControl::GetLastDisplayAffinityError()
{
  return m_dwLastSetAffinityError;
}

CScreenCaptureStateControl::CScreenCaptureStateControl()
  :
  CStateBitmapControl(new CExcludeCaptureStateBitmapManager())
{
}

UINT CScreenCaptureStateControl::GetCurrentCaptureStateToolTipStringId()
{
  if (GetLastDisplayAffinityError() != ERROR_SUCCESS)
    return IDS_SCRCAP_TT_STATE_ERROR;

  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ExcludeFromScreenCapture))
    return IDS_SCRCAP_TT_ALLOWED_DBPREF;

  if (!app.IsExcludeFromScreenCapture())
    return app.ResolveAllowScreenCaptureStateResourceId(IDS_SCRCAP_TT_OVERRIDE_NONE);

  return IDS_SCRCAP_TT_DISALLOWED_DEFAULT;
}

UINT CScreenCaptureStateControl::GetCurrentCaptureStateBitmapId()
{
  if (GetLastDisplayAffinityError() != ERROR_SUCCESS)
    return IDB_SCRCAP_STATE_ERROR;

  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ExcludeFromScreenCapture))
    return IDB_SCRCAP_ALLOWED;

  if (app.IsCommandLineForcedAllowScreenCapture())
    return IDB_SCRCAP_ALLOWED_FORCED1;

  if (!app.IsExcludeFromScreenCapture())
    return IDB_SCRCAP_ALLOWED_IMPLICIT;

  return IDB_SCRCAP_EXCLUDED;
}

void CScreenCaptureStateControl::OnSetInitialState()
{
  SetToolTipText(GetCurrentCaptureStateToolTipStringId());

  if (GetLastDisplayAffinityError() != ERROR_SUCCESS)
    SetState(IDB_SCRCAP_STATE_ERROR);
  else if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ExcludeFromScreenCapture))
    SetState(IDB_SCRCAP_ALLOWED);
  else if (app.IsCommandLineForcedAllowScreenCapture())
    SetState(IDB_SCRCAP_ALLOWED_FORCED1, IDB_SCRCAP_ALLOWED_FORCED1, IDB_SCRCAP_ALLOWED_FORCED2, BLINK_DURATION_SECONDS);
  else if (!app.IsExcludeFromScreenCapture())
    SetState(IDB_SCRCAP_ALLOWED_IMPLICIT);
  else
    SetState(IDB_SCRCAP_EXCLUDED);
}

BEGIN_MESSAGE_MAP(CScreenCaptureStateControl, CStateBitmapControl)
  //{{AFX_MSG_MAP(CStateBitmapControl)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
