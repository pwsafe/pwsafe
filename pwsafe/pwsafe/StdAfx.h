// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(stdafx_h)
#define stdafx_h


#define VC_EXTRALEAN     // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif
#include <afxtempl.h>
#include <afxpriv.h>

// Due to the compiler being a bit brain-dead this will give warnings about
// a missing file when you compile or clean the project (Pocket PC only).
#ifndef _WIN32_WCE
  #include <htmlhelp.h>
#endif

//#include "MyString.h"

//Don't show warning for automatic inline conversion
#pragma warning(disable: 4711)


#endif // stdafx_h

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
