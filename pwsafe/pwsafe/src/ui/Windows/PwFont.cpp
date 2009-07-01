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
    lf.lfFaceName = L"Courier"; // max size = LF_FACESIZE (32)
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

static CString GetToken(CString& str, LPCWSTR c)
{
  // helper function for ExtractFont()
  int pos = str.Find(c);
  CString token = str.Left(pos);
  str = str.Mid(pos + 1);
  return token;
}

void ExtractFont(const CString &str, LOGFONT &logfont)
{
  CString s(str);
  memset(&logfont, 0, sizeof(LOGFONT));
  logfont.lfHeight      = _wtol((LPCWSTR)GetToken(s, L","));
  logfont.lfWidth       = _wtol((LPCWSTR)GetToken(s, L","));
  logfont.lfEscapement  = _wtol((LPCWSTR)GetToken(s, L","));
  logfont.lfOrientation = _wtol((LPCWSTR)GetToken(s, L","));
  logfont.lfWeight      = _wtol((LPCWSTR)GetToken(s, L","));

#pragma warning(push)
#pragma warning(disable:4244) //conversion from 'int' to 'BYTE', possible loss of data
  logfont.lfItalic         = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfUnderline      = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfStrikeOut      = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfCharSet        = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfOutPrecision   = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfClipPrecision  = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfQuality        = _wtoi((LPCWSTR)GetToken(s, L","));
  logfont.lfPitchAndFamily = _wtoi((LPCWSTR)GetToken(s, L","));
#pragma warning(pop)

#if (_MSC_VER >= 1400)
  wcscpy_s(logfont.lfFaceName, LF_FACESIZE, s);
#else
  wcscpy(logfont.lfFaceName, s;
#endif  
}
