/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "resource.h"

#include "ExcludeCaptureStateBitmapManager.h"

CExcludeCaptureStateBitmapManager::CExcludeCaptureStateBitmapManager()
  :
  CStateBitmapManager(
    IDB_SCRCAP_FIRST,
    IDB_SCRCAP_LAST,
    IDB_SCRCAP_STATE_ERROR,
    RGB_BITMAP_TRANSPARENT_COLOR
  )
{
}
