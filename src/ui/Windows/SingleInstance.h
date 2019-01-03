/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file SingleInstance.cpp

/*
*  This comes from the article "Avoiding Multiple Instances of an Application"
*  Joseph M. Newcomer [MVP]; http://www.flounder.com/nomultiples.htm
*  
*  However, the pure-C version by Daniel Lohmann  [http://www.netexec.de/]
*  for creating the appropriate unique mutex name, as referenced by Joseph,
*  was used in preference to his MFC implementation.
*
*  Minor changes to require the caller to provide the length of the buffer
*  supplied to prevent overruns.
*/

#pragma once

const int SI_SESSION_UNIQUE = 0x0001;  // Allow only one instance per login session
const int SI_DESKTOP_UNIQUE = 0x0002;  // Allow only one instance on current desktop
const int SI_TRUSTEE_UNIQUE = 0x0004;  // Allow only one instance for current user
const int SI_SYSTEM_UNIQUE  = 0x0000;  // Allow only one instance at all (on the whole system)

// Note: SI_SESSION_UNIQE and SI_TRUSTEE_UNIQUE can
// be combined with SI_DESKTOP_UNIQUE

LPWSTR CreateUniqueName(const LPCWSTR pszGUID, LPWSTR pszBuffer, const int iBuffLen,
                        const int nMode = SI_DESKTOP_UNIQUE);
