/// \file PwFont.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "PwFont.h"
#include "corelib/PwsPlatform.h"

static CFont *pw_font = NULL;

void ReleasePasswordFont()
{
  if (pw_font != NULL) {
    ::DeleteObject(pw_font);
	pw_font = NULL;
  }
}

void
SetPasswordFont(CWnd* pDlgItem)
{
#if !defined(POCKET_PC)
  if (pw_font == NULL) {
    HFONT hfont;
    TCHAR* FONT_NAME;
    const int FONT_SIZE = 14;

    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx = TRUE, bOsVersionInfo = TRUE;

    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.
    /*
      dwMajorVersion
      Major version number of the operating system and can be one of the following values:
      4 The operating system is NT 4.0, Me, 98 or 95.
      5 The operating system is Server 2003 R2, Server 2003, XP or 2000.
      6 The operating system is Vista or Server "Longhorn".

      dwMinorVersion
      Minor version number of the operating system and can be one of the following values:
      0 The operating system is Vista, Server "Longhorn", 2000, NT 4.0 or 95.
      1 The operating system is XP.
      2 The operating system is Server 2003 R2, Server 2003 or XP Professional x64 Edition.
      10 The operating system is 98.
      90 The operating system is Me.
    */

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi);
    if( !bOsVersionInfoEx) {
      osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      bOsVersionInfo = GetVersionEx((OSVERSIONINFO *) &osvi);
    }

    // Windows XP default font = Tahoma
    // Prior versions of Windows have default font = Courier
    // Who knows about Vista?
    if (bOsVersionInfoEx || bOsVersionInfo) {
      if (((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1))	// XP, Server 2003 R2, Server 2003 or XP Pro x64 
      		|| (osvi.dwMajorVersion > 5)) {								// Vista or Server "Longhorn"
        FONT_NAME = _T("Tahoma");
      } else {
        FONT_NAME = _T("Courier");
      }
    } else {
      FONT_NAME = _T("Courier");  // default if we could not get version info.
    }
		
    // Note these font names are less than the max. permitted length (LF_FACESIZE = 31 + null)
    // no need to check length before copy.

    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));	  // clear out structure
    lf.lfHeight = FONT_SIZE;		  // request a 14-pixel-height font
    _tcsncpy(lf.lfFaceName, FONT_NAME, _tcslen(FONT_NAME));      // UNICODE safe string copy
    hfont = ::CreateFontIndirect(&lf);	  // create the font (must be deleted with ::DeleteObject()
    // Convert the existing HFONT to CFont*.
    pw_font = CFont::FromHandle(hfont);
    ::DeleteObject(hfont);
  }

  pDlgItem->SetFont(pw_font);
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
