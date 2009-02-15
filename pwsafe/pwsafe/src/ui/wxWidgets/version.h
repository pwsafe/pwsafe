/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file version.h
 * 
 */

#ifndef _VERSION_H_
#define _VERSION_H_

#include <wx/string.h>

#ifdef linux
#define APPNAME _T("PasswordSafe (linux)")
#else
#define APPNAME _T("PasswordSafe (wxWidgets)")
#endif

#define MAJORVERSION 0
#define MINORVERSION 1

// define following for custom/experimental versions
#define SPECIALBUILD _T("pre-alpha")

#ifndef SPECIALBUILD
#define SPECIALBUILD _T("")
#endif

// SVN_VERSION should be provided via -D from Makefile
#ifndef SVN_VERSION
#define SVN_VERSION 0
#endif

extern const wxString pwsafeAppName;
extern const wxString pwsafeVersionString;

#endif /* _VERSION_H_ */
