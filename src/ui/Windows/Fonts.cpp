/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file Fonts.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Fonts.h"
#include "core/PwsPlatform.h"

Fonts *Fonts::self = NULL;

CFont *Fonts::pPasswordFont = NULL;
CFont *Fonts::pCurrentFont = NULL;  // Do NOT delete - done in DboxMain
CFont *Fonts::pModifiedFont = NULL;
CFont *Fonts::pDragFixFont = NULL;  // Fix for lack of text during drag!

/*
  Only the following set:
    lf.lfHeight = -16;
    lf.lfWeight = FW_NORMAL;
    lf.lfPitchAndFamily = FF_MODERN | FIXED_PITCH;
    lf.lfFaceName = L"Courier"; // max size = LF_FACESIZE (32)
*/
static LOGFONT dfltPasswordLogfont = {
  -16, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, FF_MODERN | FIXED_PITCH,
  L'C', L'o', L'u', L'r', L'i', L'e', L'r', L'\0'};

// Bug in MS TreeCtrl and CreateDragImage.  During Drag, it doesn't show
// the entry's text as well as the drag image if the font is not MS Sans Serif !!!!
static LOGFONT DragFixLogfont = {
  -16, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, DEFAULT_PITCH | FF_SWISS,
  L'M', L'S', L' ', L'S', L'a', L'n', L's', L' ', L'S', L'e', L'r', L'i', L'f', L'\0'};

Fonts *Fonts::GetInstance()
{
  if (self == NULL) {
    self = new Fonts();
  }
  return self;
}

void Fonts::DeleteInstance()
{
  if (pCurrentFont != NULL) {
    pCurrentFont->DeleteObject();
    delete pCurrentFont;
    pCurrentFont = NULL;
  }
  if (pModifiedFont != NULL) {
    pModifiedFont->DeleteObject();
    delete pModifiedFont;
    pModifiedFont = NULL;
  }
  if (pDragFixFont != NULL) {
    pDragFixFont->DeleteObject();
    delete pDragFixFont;
    pDragFixFont = NULL;
  }
  if (pPasswordFont != NULL) {
    pPasswordFont->DeleteObject();
    delete pPasswordFont;
    pPasswordFont = NULL;
  }
  delete self;
  self = NULL;
}

Fonts::Fonts()
{
  pCurrentFont = new CFont;
  pModifiedFont = new CFont;
  pDragFixFont = new CFont;
  pPasswordFont = new CFont;
}

void Fonts::GetCurrentFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL || pCurrentFont != NULL);
  if (pLF == NULL|| pCurrentFont == NULL)
    return;

  pCurrentFont->GetLogFont(pLF);
}

void Fonts::SetCurrentFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL);
  if (pLF == NULL)
    return;

  if (pCurrentFont == NULL) {
    pCurrentFont = new CFont;
  } else {
    pCurrentFont->DeleteObject();
  }
  pCurrentFont->CreateFontIndirect(pLF);
}

void Fonts::GetPasswordFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL || pPasswordFont != NULL);
  if (pLF == NULL|| pPasswordFont == NULL)
    return;

  pPasswordFont->GetLogFont(pLF);
}

void Fonts::GetDefaultPasswordFont(LOGFONT &lf)
{
  memcpy(&lf, &dfltPasswordLogfont, sizeof(LOGFONT));
}

void Fonts::SetPasswordFont(LOGFONT *pLF)
{
  if (pPasswordFont == NULL) {
    pPasswordFont = new CFont;
  } else {
    pPasswordFont->DeleteObject();
  }
  pPasswordFont->CreateFontIndirect(pLF == NULL ? &dfltPasswordLogfont : pLF);
}

void Fonts::ApplyPasswordFont(CWnd* pDlgItem)
{
#if !defined(POCKET_PC)
  ASSERT(pDlgItem != NULL);
  if (pDlgItem == NULL)
    return;

  if (pPasswordFont == NULL) {
    pPasswordFont = new CFont;
    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    pPasswordFont->CreateFontIndirect(&dfltPasswordLogfont);
  }

  pDlgItem->SetFont(pPasswordFont);
#endif
}

static CString GetToken(CString& str, LPCWSTR c)
{
  // helper function for ExtractFont()
  int pos = str.Find(c);
  CString token = str.Left(pos);
  str = str.Mid(pos + 1);
  return token;
}

void Fonts::ExtractFont(const CString &str, LOGFONT &lf)
{
  CString s(str);
  SecureZeroMemory(&lf, sizeof(lf));
  lf.lfHeight      = _wtol((LPCWSTR)GetToken(s, L","));
  lf.lfWidth       = _wtol((LPCWSTR)GetToken(s, L","));
  lf.lfEscapement  = _wtol((LPCWSTR)GetToken(s, L","));
  lf.lfOrientation = _wtol((LPCWSTR)GetToken(s, L","));
  lf.lfWeight      = _wtol((LPCWSTR)GetToken(s, L","));

#pragma warning(push)
#pragma warning(disable:4244) //conversion from 'int' to 'BYTE', possible loss of data
  lf.lfItalic         = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfUnderline      = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfStrikeOut      = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfCharSet        = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfOutPrecision   = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfClipPrecision  = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfQuality        = _wtoi((LPCWSTR)GetToken(s, L","));
  lf.lfPitchAndFamily = _wtoi((LPCWSTR)GetToken(s, L","));
#pragma warning(pop)

#if (_MSC_VER >= 1400)
  wcscpy_s(lf.lfFaceName, LF_FACESIZE, s);
#else
  wcscpy(lf.lfFaceName, s);
#endif  
}

const COLORREF Fonts::MODIFIED_COLOR = RGB(0, 0, 128);

void Fonts::SetUpFont(CWnd *pWnd, CFont *pfont)
{
  // Set main font
  pCurrentFont = pfont;
  pWnd->SetFont(pfont);

  // Set up special fonts
  // Remove old fonts
  pModifiedFont->DeleteObject();
  pDragFixFont->DeleteObject();
  
  // Get current font
  LOGFONT lf;
  pfont->GetLogFont(&lf);

  // Make it italic and create "modified" font
  lf.lfItalic = TRUE;
  pModifiedFont->CreateFontIndirect(&lf);
  
  // Make DragFix font same height as user selected font
  DragFixLogfont.lfHeight = lf.lfHeight;
  // Create DragFix font
  pDragFixFont->CreateFontIndirect(&DragFixLogfont);
}
