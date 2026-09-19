/*
 * Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file Version.cpp
 * 
 */

#include "version.h"

#include "core/PWSversion.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

const wxString pwsafeAppName(APPNAME);
#ifdef _DEBUG
const wchar_t *debstr = L"[debug] ";
#else
const wchar_t *debstr = L"";
#endif

const wxString pwsafeVersionString = wxString::Format(wxString(_T("v%ls%ls (%ls) %ls")),
                                                      PWSversion::GetInstance()->FormatVersionString().c_str(),
                                                      PWSversion::GetInstance()->GetSpecialBuild().c_str(),
                                                      PWSversion::GetInstance()->GetRevision().c_str(),
                                                      debstr);
