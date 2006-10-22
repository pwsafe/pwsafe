#pragma once

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define VC_EXTRALEAN     // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif
#include <afxtempl.h>
#include <afxpriv.h>

#include <htmlhelp.h>

//Don't show warning for automatic inline conversion
#pragma warning(disable: 4711)
//Don't show warning for "identifier was truncated to '255' characters" in STL.
#pragma warning(disable: 4786)
