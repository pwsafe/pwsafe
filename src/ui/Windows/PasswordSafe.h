/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasswordSafe.h
// main header file for the PasswordSafe application
//-----------------------------------------------------------------------------

#if defined(WIN32)
#include "stdafx.h"
#endif

#include "core/PwsPlatform.h"

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

extern const wchar_t *HIDDEN_PASSWORD;

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
