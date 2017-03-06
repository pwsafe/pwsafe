#pragma once

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// The following is needed because gdiplustypes.h uses the Windows macros
// min & max but these "can" cause the error "C3861: 'min': identifier not found"
// when building under VS2015.
// However, although the code below prevents this, it REQUIRES the use
// of std::min & std::max everywhere.  As not macros, these also require
// the arguments to be the same type e.g. "int, int" or "long, long" etc.
// unlike the Windows macros min & max.
#define NOMINMAX
#include <algorithm>
namespace Gdiplus {
  using std::min;
  using std::max;
};

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

// Show warnings
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS 
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif

#include <afxtempl.h>
#include <afxpriv.h>

#include <htmlhelp.h>
#include <afxdlgs.h>
#include <afxcontrolbars.h>

// Don't show warning for automatic inline conversion
#pragma warning(disable: 4711)

// Don't show warning for "identifier was truncated to '255' characters" in STL.
#pragma warning(disable: 4786)

// Ensure that switch enum statements without a "default" case statement catch all
// possible enum values
#pragma warning(error: 4062)

// Save including it everywhere!
#include "Windowsdefs.h"
