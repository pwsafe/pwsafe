/*
* Copyright (c) 2009-2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

//
// pws_osk.h : main header file for the pws_osk DLL
//

#ifndef _PWS_OSK_H
#define _PWS_OSK_H
#include "voskeys.h"

#define VK_DLL_VERSION 1

#ifdef PWS_OSK_EXPORTS
#define OSK_API __declspec(dllexport)
#else
#define OSK_API __declspec(dllimport)
#endif /* PWS_OSK_EXPORTS */

#ifdef __cplusplus
extern "C" {
#endif /* Start bracket of __cplusplus */

OSK_API void OSK_ListKeyboards(UINT &uiKLID, UINT &uiCtrlID);
OSK_API BOOL OSK_GetKeyboardData(UINT uiKLID, st_KBImpl &stKBImpl);
OSK_API int  OSK_GetVersion();

#ifdef __cplusplus
}
#endif /* End bracket of __cplusplus */

#endif /* _PWS_OSK_H */
