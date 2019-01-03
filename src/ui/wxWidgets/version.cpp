/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file version.cpp
 * 
 */

#include "version.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

const wxString pwsafeAppName(APPNAME);
#ifdef _DEBUG
const wchar_t *debstr = L"[debug] ";
#else
const wchar_t *debstr = L"";
#endif

#if defined(REVISION) && (REVISION != 0)
const wxString pwsafeVersionString = wxString::Format(wxString(_T("v%d.%.2d.%d (%ls) %ls%ls")),
                                                      MAJORVERSION, MINORVERSION, REVISION,
                                                      _T(VCS_VERSION), debstr, SPECIALBUILD);
#else
const wxString pwsafeVersionString = wxString::Format(wxString(_T("v%d.%.2d (%ls) %ls%ls")),
                                                      MAJORVERSION, MINORVERSION,
                                                      _T(VCS_VERSION), debstr, SPECIALBUILD);
#endif
