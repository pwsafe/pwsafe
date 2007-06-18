/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

// PasswordSafe.h
// main header file for the PasswordSafe application
//-----------------------------------------------------------------------------


#if defined(WIN32)
#include "stdafx.h"
#endif

#include "corelib/PwsPlatform.h"

#if !defined(WCE_INS)
#if defined(POCKET_PC)
#define WCE_INS
#else
#define WCE_INS /##/
#endif
#endif

#if !defined(WCE_DEL)
#if defined(POCKET_PC)
#define WCE_DEL /##/
#else
#define WCE_DEL
#endif
#endif

/*
  jpr debug stuff
*/
#include "jprdebug.h"


/*
  eventually, this breaks off into pws_mfc.h
*/

/*
  a globally available reference to the app object, which is a whole lot
  cleaner (in my mind) than constantly calling AfxGetApp() for the same
  thing... {jpr}
*/

class ThisMfcApp;
extern ThisMfcApp app;

/*
 * This is the string to be displayed instead of the actual password, unless
 * the user chooses to see the password:
 */

extern const TCHAR *HIDDEN_PASSWORD;

// Clipboard format for Column Chooser Drag & Drop
extern CLIPFORMAT gbl_ccddCPFID;

// Clipboard format for TreeCtrl Drag & Drop
extern CLIPFORMAT gbl_tcddCPFID;

// PWS Instance (unique) class name = "PWS" + "UUID string (36)" + NULL
extern TCHAR gbl_classname[40];

// Save all the trailing NULL "sizeof(TCHAR)"
#define DD_CLASSNAME_SIZE (sizeof(gbl_classname) - sizeof(TCHAR))

// Size of mandatory data:
//   D&D type: %02x, bufferlength (entry D&D) %08x = 10
//   D&D type: %02x, column type: %04x, column name length (column D&D) %04x = 10
#define DD_REQUIRED_DATA_SIZE (10 * sizeof(TCHAR))

// Minimum Drag & Drop memory buffer size = 
//   sizeof(classname) + DD_REQUIRED_DATA_SIZE (+ trailing NULL for Column D&D)
#define DD_MEMORY_MINSIZE (DD_CLASSNAME_SIZE + DD_REQUIRED_DATA_SIZE)

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
