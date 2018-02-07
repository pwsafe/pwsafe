/*
* Copyright (c) 2009-2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This is a conversion from the C# code published by Michael Kaplan in his
* series of MSDN Blogs "Sorting it all Out, Getting all you can out of a
* keyboard layout" (Part 0 to Part 9b) between March 23 and April 13, 2006.
* This code has been modified for Password Safe needs.
* It is Unicode ONLY.
*/

#pragma once

/// \file VKShiftState.h
//-----------------------------------------------------------------------------

// Different Ket state values for Virtual Keyboard
// Note: the Alt key never affects the keyboard characters
// AltGr is equivalent to "Alt + LCtrl"

enum VKShiftState {
  VST_BASE          = 0x00,                             //  0
  VST_SHIFT         = 0x01,                             //  1
  VST_LCTRL         = 0x02,                             //  2
  VST_SHIFTCTRL     = VST_SHIFT | VST_LCTRL,            //  3
  VST_MENU          = 0x04,                             //  4 -- NOT USED
  VST_SHIFTMENU     = VST_SHIFT | VST_MENU,             //  5 -- NOT USED
  VST_ALTGR         = 0x06,                             //  6
  VST_SHIFTALTGR    = VST_SHIFT | VST_ALTGR,            //  7
  VST_RCTRL         = 0x08,                             //  8
  VST_SHIFTRCTRL    = VST_SHIFT | VST_RCTRL,            //  9
  // 0x0a - 0x0f not used
  VST_CAPSLOCK      = 0x10,                             // 16
};
