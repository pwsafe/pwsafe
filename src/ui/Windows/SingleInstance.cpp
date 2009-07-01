/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file SingleInstance.cpp

/*
*  This comes from the article "Avoiding Multiple Instances of an Application"
*  Joseph M. Newcomer [MVP]; http://www.flounder.com
*  
*  However, the pure-C version by Daniel Lohmann  [http://www.netexec.de/]
*  for creating the appropriate unique mutex name, as referenced by Joseph,
*  was used in preference to his MFC implementation.
*
*  Minor changes to require the caller to provide the length of the buffer
*  supplied to prevent overruns.
*/

// Note: Option "SI_TRUSTEE_UNIQUE" (Allow one instance per user account)
// has been implemented for Password Safe


#include "stdafx.h"
#include "SingleInstance.h"
#include <malloc.h>  // Use the _malloca/_freea functions (changed from original)

////////////////////////////////////////////////////////////////////////////////
// LPWSTR CreateUniqueName(pszGUID, pszBuffer, nMode)
//
// Creates a "unique" name, where the meaning of "unique" depends on the nMode 
// flag values. Returns pszBuffer
//
// pszGUID:    Copied to the beginning of pszBuffer, should be an GUID
// pszBuffer:  Buffer for unique name. Length (in chars) must be >= MAX_PATH
// nMode:    Information, that should be used to create the unique name.
//        Can be one of the following values:
//
//
//        SI_SESSION_UNIQUE            - Allow one instance per login session
//        SI_DESKTOP_UNIQUE            - Allow one instance per desktop
//        SI_TRUSTEE_UNIQUE            - Allow one instance per user account
//        SI_SESSION_UNIQUE | SI_DESKTOP_UNIQUE
//                                     - Allow one instance per login session,
//                                       instances in different login sessions
//                                       must also reside on a different desktop
//        SI_TRUSTEE_UNIQUE | SI_DESKTOP_UNIQUE  
//                                     - Allow one instance per user account,
//                                       instances in login sessions running a
//                                       different user account must also reside  
//                                       on different desktops.
//        SI_SYSTEM_UNIQUE             - Allow only one instance on the whole system  
//
LPWSTR CreateUniqueName(LPCWSTR pszGUID, LPWSTR pszBuffer, int iBuffLen,
                        int nMode  /* = SI_TRUSTEE_UNIQUE */)
{
  if (pszBuffer == NULL) {
    SetLastError(ERROR_INVALID_PARAMETER);
    return NULL;
  }

  // First copy GUID to destination buffer
  if (pszGUID)
#if _MSC_VER >= 1400
    wcscpy_s(pszBuffer, iBuffLen, pszGUID);
#else
    wcscpy(pszBuffer, pszGUID);
#endif
  else
    *pszBuffer = 0;

  if (nMode & SI_DESKTOP_UNIQUE) {
    // Name should be desktop unique, so add current desktop name
#if _MSC_VER >= 1400
    wcscat_s(pszBuffer, iBuffLen, L"-");
#else
    wcscat(pszBuffer, L"-");
#endif
    HDESK hDesk    = GetThreadDesktop(GetCurrentThreadId());
    ULONG cchDesk  = MAX_PATH - wcslen(pszBuffer) - 1;

    if (!GetUserObjectInformation(hDesk, UOI_NAME, pszBuffer + wcslen(pszBuffer), 
                                  cchDesk, &cchDesk))
      // Call will fail on Win9x
#if _MSC_VER >= 1400
      wcsncat_s(pszBuffer, iBuffLen, L"Win9x", cchDesk);
#else
      wsncat(pszBuffer, L"Win9x", cchDesk);
#endif
  }
  if (nMode & SI_SESSION_UNIQUE) {
    // Name should be session unique, so add current session id
    
    HANDLE hToken = NULL;
    // Try to open the token (fails on Win9x) and check necessary buffer size
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) &&
        (MAX_PATH - wcslen(pszBuffer) > 9)) {
      DWORD cbBytes = 0;

      if (!GetTokenInformation(hToken, TokenStatistics, NULL, cbBytes, &cbBytes) &&
           GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        PTOKEN_STATISTICS pTS = (PTOKEN_STATISTICS)_malloca(cbBytes);
        if (GetTokenInformation(hToken, TokenStatistics, (LPVOID) pTS, cbBytes, &cbBytes)) {
          wsprintf(pszBuffer + wcslen(pszBuffer), L"-%08x%08x", 
                   pTS->AuthenticationId.HighPart, pTS->AuthenticationId.LowPart);
        }
        _freea(pTS);
      }
    }
  }
  if (nMode & SI_TRUSTEE_UNIQUE) {
    // Name should be unique to the current user
    wchar_t szUser[64] = {0};
    wchar_t szDomain[64] = {0};
    DWORD cchUser  = 64;
    DWORD cchDomain  = 64;

    if (GetUserName(szUser, &cchUser)) {
      // Since NetApi() calls are quite time consuming
      // we retrieve the domain name from an environment variable
      cchDomain = GetEnvironmentVariable(L"USERDOMAIN", szDomain, cchDomain);
      UINT cchUsed = wcslen(pszBuffer);
      if (MAX_PATH - cchUsed > cchUser + cchDomain + 3) {
        wsprintf(pszBuffer + cchUsed, L"-%s-%s", szDomain, szUser);
      }
    }
  }
  return pszBuffer;
}
