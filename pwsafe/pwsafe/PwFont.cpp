/// \file PwFont.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "corelib/PwsPlatform.h"
#include "corelib/Util.h"

void
SetPasswordFont(CWnd* pDlgItem)
{
#if !defined(POCKET_PC)
	// for now, this keeps the leak down to just one
	static HFONT hfont = NULL;
	static TCHAR* FONT_NAME;
	static const int FONT_SIZE = 14;

	if ( _winmajor == 5 && _winminor != 0 ) // Windows XP default font = Tahoma
	{
		FONT_NAME = _T("Tahoma");
	}
	else
	{
		FONT_NAME = _T("Courier");
	}

	if ( hfont == NULL )
	{
		// Initialize a CFont object with the characteristics given
		// in a LOGFONT structure.

		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));	  // clear out structure
		lf.lfHeight = FONT_SIZE;		  // request a 14-pixel-height font
		strCopy( lf.lfFaceName, FONT_NAME );      // UNICODE safe string copy
		hfont = ::CreateFontIndirect(&lf);	  // create the font (must be deleted with ::DeleteObject()
	}

	if (hfont != NULL)
	{
		// Convert the HFONT to CFont*.
		CFont* pfont = CFont::FromHandle(hfont);
		if (pfont != NULL)
			pDlgItem->SetFont(pfont);
	}
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
