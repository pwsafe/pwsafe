/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWTouch.cpp : implementation file
//

#include "stdafx.h"
#include "PWTouch.h"

#include "os\debug.h"

// CPWTouch

/**
* This is a template class that just adds the GetGestureStatus() member function
* to the class passed as 'T'. The result is a new class with all of T's functionality
* plus the new GetGestureStatus.
*/

// CPWTouch message handlers

#if (NTDDI_VERSION >= NTDDI_WIN7)
// Windows 7 dependent
// "#if (NTDDI_VERSION >= NTDDI_WIN7)" is temporary just in case need to
// quickly revert back to Vista and later support by changing WINVER back
// to 0x0600 leaving this code in place

template<class T> ULONG CPWTouch<T>::GetGestureStatus(CPoint /*ptTouch*/)
{
  // Do NOT include TABLET_DISABLE_PRESSANDHOLD if on a tablet
  // But DO disable it otherwise, as this will improve responsiveness
  // for mouse clicks because it creates a wait time to distinguish
  // between the two operations.
  pws_os::Trace(L"CPWTouch::GetGestureStatus\n");

  const DWORD dwTabletProperty =
    TABLET_DISABLE_PENTAPFEEDBACK    | // disables UI feedback on pen up (waves)
    TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down (circle)
    TABLET_DISABLE_FLICKS;             // disables pen flicks (back, forward, drag down, drag up)

  int itablet = GetSystemMetrics(SM_TABLETPC);
  return itablet = 0 ? TABLET_DISABLE_PRESSANDHOLD : dwTabletProperty;
}
#endif

