/// \file PwFont.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "EditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void
SetPasswordFont(CWnd* pDlgItem)
{
	// Initialize a CFont object with the characteristics given
	// in a LOGFONT structure.
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));          // clear out structure
	lf.lfHeight = 10;                         // request a 10-pixel-height font
	strcpy(lf.lfFaceName, "Courier");         // request a face name "Courier"
	HFONT hfont = ::CreateFontIndirect(&lf);  // create the font

	// Convert the HFONT to CFont*.
	CFont* pfont = CFont::FromHandle(hfont);
   pDlgItem->SetFont(pfont);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
