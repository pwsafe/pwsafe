/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file CPWTouch.h

#pragma once
#include "os\debug.h"

// CPWTouch

/**
* This is a template class that just adds the GetGestureStatus() member function
* to the class passed as 'T'. The result is a new class with all of T's functionality
* plus the new GetGestureStatus.
*/

template<class T>
class CPWTouch : public T
{
protected:
#if (NTDDI_VERSION >= NTDDI_WIN7)
  virtual ULONG GetGestureStatus(CPoint)
  {
    // Windows 7 dependent
    // "#if (NTDDI_VERSION >= NTDDI_WIN7)" is temporary just in case need to
    // quickly revert back to Vista and later support by changing WINVER back
    // to 0x0600 leaving this code in place

    // Do NOT include TABLET_DISABLE_PRESSANDHOLD if on a tablet
    // But DO include it otherwise, as this will improve responsiveness
    // for mouse clicks because, if not disabled, it creates a wait time to
    // distinguish between the two operations.
    const DWORD dwTabletProperty =
      TABLET_DISABLE_PENTAPFEEDBACK    | // disables UI feedback on pen up (waves)
      TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down (circle)
      TABLET_DISABLE_FLICKS;             // disables pen flicks (back, forward, drag down, drag up)

    bool bTablet = GetSystemMetrics(SM_TABLETPC) != 0;
    pws_os::Trace(L"CPWTouch::GetGestureStatus - running on %s\n", bTablet ? L"Tablet" : L"Desktop");
    return bTablet ? dwTabletProperty : TABLET_DISABLE_PRESSANDHOLD;
  }
#endif
};
