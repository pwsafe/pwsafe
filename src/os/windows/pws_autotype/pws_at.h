/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// File: pws_at.h
//
#ifndef _PWS_AT_H
#define _PWS_AT_H

#define AT_DLL_VERSION 1

#ifdef PWS_AT_EXPORTS
#define AT_API __declspec(dllexport)
#else
#define AT_API __declspec(dllimport)
#endif /* PWS_AT_EXPORTS */

#ifdef __cplusplus
extern "C" {
#endif /* Start bracket of __cplusplus */

AT_API BOOL AT_HK_Initialise(HWND hWnd);
AT_API BOOL AT_HK_UnInitialise(HWND hWnd);
AT_API int  AT_HK_GetVersion();

#ifdef __cplusplus
}
#endif /* End bracket of __cplusplus */

#define UNIQUE_PWS_SHELL L"PasswordSafe-{FC3F78C0-1B04-40CF-A7B5-6F037436D9C0}"

#endif /* _PWS_AT_H */
