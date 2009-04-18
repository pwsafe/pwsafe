/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// File: pws_at.h
//
#ifndef _PWS_AT_H
#define _PWS_AT_H

// No need for dllimport versions as DLL is Run-time dynamically
// used and not Load-time.

#ifdef PWS_AT_EXPORTS
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) BOOL AT_HK_Initialise(HWND hWnd);
__declspec(dllexport) BOOL AT_HK_UnInitialise(HWND hWnd);

#ifdef __cplusplus
}
#endif
#endif

#define UNIQUE_PWS_SHELL L"PasswordSafe-{FC3F78C0-1B04-40CF-A7B5-6F037436D9C0}"

#endif /* _PWS_AT_H */
