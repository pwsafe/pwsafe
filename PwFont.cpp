/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
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

void
SetPasswordFont(CWnd* pDlgItem)
{
#if !defined(POCKET_PC)
	CFont *pw_font;
    HFONT hfont;
    TCHAR* tch_fontname;
    const int ifontsize = -16;
    tch_fontname = _T("MS Sans Serif");

    // Note these font names are less than the max. permitted length (LF_FACESIZE = 31 + null)
    // no need to check length before copy.

    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));	  // clear out structure
    lf.lfHeight = ifontsize;
    _tcsncpy(lf.lfFaceName, tch_fontname, _tcslen(tch_fontname));      // UNICODE safe string copy
	lf.lfPitchAndFamily = FF_SWISS;
    hfont = ::CreateFontIndirect(&lf);	  // create the font (must be deleted with ::DeleteObject()
    // Convert the existing HFONT to CFont*.
    pw_font = CFont::FromHandle(hfont);
	pDlgItem->SetFont(pw_font);
	::DeleteObject(pw_font);
    ::DeleteObject(hfont);
#endif
}

