/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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
  if (m_pCurrentFont != NULL) {
    m_pCurrentFont->DeleteObject();
    delete m_pCurrentFont;
    m_pCurrentFont = NULL;
  }
  if (m_pModifiedFont != NULL) {
    m_pModifiedFont->DeleteObject();
    delete m_pModifiedFont;
    m_pModifiedFont = NULL;
  }
  if (m_pDragFixFont != NULL) {
    m_pDragFixFont->DeleteObject();
    delete m_pDragFixFont;
    m_pDragFixFont = NULL;
  }
  if (m_pPasswordFont != NULL) {
    m_pPasswordFont->DeleteObject();
    delete m_pPasswordFont;
    m_pPasswordFont = NULL;
  }
  if (m_pNotesFont != NULL) {
    m_pNotesFont->DeleteObject();
    delete m_pNotesFont;
    m_pNotesFont = NULL;
  }
  delete self;
  self = NULL;
}

Fonts::Fonts() : MODIFIED_COLOR(RGB(0, 0, 128))
{
  m_pCurrentFont = new CFont;
  m_pModifiedFont = new CFont;
  m_pDragFixFont = new CFont;
  m_pPasswordFont = new CFont;
  m_pNotesFont = new CFont;
}

void Fonts::GetCurrentFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL && m_pCurrentFont != NULL);
  if (pLF == NULL || m_pCurrentFont == NULL)
    return;

  m_pCurrentFont->GetLogFont(pLF);
}

void Fonts::SetCurrentFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL);
  if (pLF == NULL)
    return;

  if (m_pCurrentFont == NULL) {
    m_pCurrentFont = new CFont;
  } else {
    m_pCurrentFont->DeleteObject();
  }
  m_pCurrentFont->CreateFontIndirect(pLF);
}

void Fonts::GetPasswordFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL && m_pPasswordFont != NULL);
  if (pLF == NULL || m_pPasswordFont == NULL)
    return;

  m_pPasswordFont->GetLogFont(pLF);
}

void Fonts::GetDefaultPasswordFont(LOGFONT &lf)
{
  memcpy(&lf, &dfltPasswordLogfont, sizeof(LOGFONT));
}

void Fonts::SetPasswordFont(LOGFONT *pLF)
{
  if (m_pPasswordFont == NULL) {
    m_pPasswordFont = new CFont;
  } else {
    m_pPasswordFont->DeleteObject();
  }
  m_pPasswordFont->CreateFontIndirect(pLF == NULL ? &dfltPasswordLogfont : pLF);
}

void Fonts::ApplyPasswordFont(CWnd* pDlgItem)
{
  ASSERT(pDlgItem != NULL);
  if (pDlgItem == NULL)
    return;

  if (m_pPasswordFont == NULL) {
    m_pPasswordFont = new CFont;
    // Initialize a CFont object with the characteristics given
    // in a LOGFONT structure.
    m_pPasswordFont->CreateFontIndirect(&dfltPasswordLogfont);
  }

  pDlgItem->SetFont(m_pPasswordFont);
}

void Fonts::GetNotesFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL && m_pNotesFont != NULL);
  if (pLF == NULL || m_pNotesFont == NULL)
    return;

  m_pNotesFont->GetLogFont(pLF);
}

void Fonts::SetNotesFont(LOGFONT *pLF)
{
  ASSERT(pLF != NULL);
  if (pLF == NULL)
    return;

  if (m_pNotesFont == NULL) {
    m_pNotesFont = new CFont;
  } else {
    m_pNotesFont->DeleteObject();
  }
  m_pNotesFont->CreateFontIndirect(pLF);
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

  wcscpy_s(lf.lfFaceName, LF_FACESIZE, s);
}


void Fonts::SetUpFont(CWnd *pWnd, CFont *pfont)
{
  // Set main font
  m_pCurrentFont = pfont;
  pWnd->SetFont(pfont);

  // Set up special fonts
  // Remove old fonts
  m_pModifiedFont->DeleteObject();
  m_pDragFixFont->DeleteObject();
  
  // Get current font
  LOGFONT lf;
  pfont->GetLogFont(&lf);

  // Make it italic and create "modified" font
  lf.lfItalic = TRUE;
  m_pModifiedFont->CreateFontIndirect(&lf);
  
  // Make DragFix font same height as user selected font
  DragFixLogfont.lfHeight = lf.lfHeight;
  // Create DragFix font
  m_pDragFixFont->CreateFontIndirect(&DragFixLogfont);
}

LONG Fonts::CalcHeight() const
{
  //Get max height from current/modified/password font
  TEXTMETRIC tm;
  HDC hDC = ::GetDC(NULL);
  
  HFONT hFontOld = (HFONT)SelectObject(hDC, m_pCurrentFont->GetSafeHandle());

  // Current
  GetTextMetrics(hDC, &tm);
  LONG height = tm.tmHeight + tm.tmExternalLeading;

  // Modified
  SelectObject(hDC, m_pModifiedFont->GetSafeHandle());
  GetTextMetrics(hDC, &tm);
  if (height < tm.tmHeight + tm.tmExternalLeading)
    height = tm.tmHeight + tm.tmExternalLeading;

  // Password
  SelectObject(hDC, m_pPasswordFont->GetSafeHandle());
  GetTextMetrics(hDC, &tm);
  if (height < tm.tmHeight + tm.tmExternalLeading)
    height = tm.tmHeight + tm.tmExternalLeading;

  // Tidy up
  SelectObject(hDC, hFontOld);
  ::ReleaseDC(NULL, hDC);

  return height;
}
