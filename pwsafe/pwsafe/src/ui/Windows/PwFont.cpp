/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

/* Only the following set:
    lf.lfHeight = -16;
    lf.lfWeight = FW_NORMAL;
    lf.lfPitchAndFamily = FF_MODERN | FIXED_PITCH;
    lf.lfFaceName = TCHAR("Courier"); // max size = LF_FACESIZE (32)
*/
static LOGFONT dfltPWFont = {
  -16, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, FF_MODERN | FIXED_PITCH,
  L'C', L'o', L'u', L'r', L'i', L'e', L'r', L'\0'};

void GetPasswordFont(LOGFONT *plogfont)
{
  ASSERT(plogfont != NULL);
  if (plogfont != NULL)
    pPasswordFont->GetLogFont(plogfont);
}

void GetDefaultPasswordFont(LOGFONT &lf)
{
  memcpy(&lf, &dfltPWFont, sizeof(LOGFONT));
}

void SetPasswordFont(LOGFONT *plogfont)
{
  ASSERT(plogfont != NULL);
  if (plogfont == NULL)
    return;

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
  if (pDlgItem == NULL)
    return;

  if (pPasswordFont == NULL) {
    pPasswordFont = new CFont;
    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    pPasswordFont->CreateFontIndirect(&dfltPWFont);
  }

  pDlgItem->SetFont(pPasswordFont);
#endif
}

void DeletePasswordFont()
{
  if (pPasswordFont != NULL) {
    pPasswordFont->DeleteObject();
    delete pPasswordFont;
    pPasswordFont = NULL;
  }
}
