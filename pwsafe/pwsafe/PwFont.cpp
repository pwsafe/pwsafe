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

// Select the font style and size for the password box
#if !defined(POCKET_PC)
  #define FONT_NAME	_T("Courier")
  #define FONT_SIZE	14
#endif

void
SetPasswordFont(CWnd* pDlgItem)
{
#if !defined(POCKET_PC)
	// for now, this keeps the leak down to just one
	static HFONT hfont		= NULL;

	if ( hfont == NULL )
	{
		// Initialize a CFont object with the characteristics given
		// in a LOGFONT structure.

		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));		  // clear out structure
		lf.lfHeight = FONT_SIZE;				  // request a 14-pixel-height font
		strCopy( lf.lfFaceName, FONT_NAME );      // UNICODE safe string copy
		hfont = ::CreateFontIndirect(&lf);		  // create the font (must be deleted with ::DeleteObject()
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
