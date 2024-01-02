/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "StateBitmapManager.h"

class CExcludeCaptureStateBitmapManager : public CStateBitmapManager
{
public:
  static const UINT RGB_BITMAP_TRANSPARENT_COLOR = RGB(254, 254, 254);
public:
  CExcludeCaptureStateBitmapManager();
};
