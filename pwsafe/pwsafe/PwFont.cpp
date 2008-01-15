/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

static CFont *pPasswordFont(NULL);

void GetPasswordFont(LOGFONT *plogfont)
{
  ASSERT(plogfont != NULL);
  pPasswordFont->GetLogFont(plogfont);
}

void SetPasswordFont(LOGFONT *plogfont)
{
  ASSERT(plogfont != NULL);
  if (pPasswordFont == NULL) {
    pPasswordFont = new CFont;
  } else {
    pPasswordFont->DeleteObject();
  }
  pPasswordFont->CreateFontIndirect(plogfont);
}

void ApplyPasswordFont(CWnd* pDlgItem)
{
#if !defined(POCKET_PC)
  ASSERT(pDlgItem != NULL);
  if (pPasswordFont == NULL) {
    pPasswordFont = new CFont;
    TCHAR* tch_fontname;
    tch_fontname = _T("Courier");

    // Note these font names are less than the max. permitted length (LF_FACESIZE = 31 + null)
    // no need to check length before copy.

    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = -16;
    lf.lfWeight = FW_NORMAL;
    _tcsncpy(lf.lfFaceName, tch_fontname, _tcslen(tch_fontname));
    lf.lfPitchAndFamily = FF_MODERN | FIXED_PITCH;
    pPasswordFont->CreateFontIndirect(&lf);
  }

  pDlgItem->SetFont(pPasswordFont);
#endif
}

void DeletePasswordFont()
{
  delete pPasswordFont;
  pPasswordFont = NULL;
}
