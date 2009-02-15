/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file version.cpp
 * 
 */

#include "version.h"

const wxString pwsafeAppName(APPNAME);
#ifndef _DEBUG
const wxString pwsafeVersionString = wxString::Format(_T("v%d.%d (%s) %s"),
                                                      MAJORVERSION, MINORVERSION,
                                                      SVN_VERSION, SPECIALBUILD);
#else
const wxString pwsafeVersionString = wxString::Format(_T("v%d.%d (%s) [debug] %s"),
                                                      MAJORVERSION, MINORVERSION,
                                                      SVN_VERSION, SPECIALBUILD);
#endif
