/// \file PwFont.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void
SetPasswordFont(CWnd* pDlgItem)
{
	// for now, this keeps the leak down to just one
	static HFONT hfont		= NULL;

	if ( hfont == NULL )
	{
		// Initialize a CFont object with the characteristics given
		// in a LOGFONT structure.

		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));		  // clear out structure
		lf.lfHeight = 14;						  // request a 14-pixel-height font
		strcpy(lf.lfFaceName, "Courier");		  // request a face name "Courier"
		hfont = ::CreateFontIndirect(&lf);		  // create the font (must be deleted with ::DeleteObject()
	}

	if (hfont != NULL)
	{
		// Convert the HFONT to CFont*.
		CFont* pfont = CFont::FromHandle(hfont);
		if (pfont != NULL)
			pDlgItem->SetFont(pfont);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
