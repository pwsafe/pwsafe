/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PwFont.h
//-----------------------------------------------------------------------------

void GetPasswordFont(LOGFONT *plogfont);
void SetPasswordFont(LOGFONT *plogfont);
void ApplyPasswordFont(CWnd* pDlgItem);
void GetDefaultPasswordFont(LOGFONT &lf);
void DeletePasswordFont();
void ExtractFont(const CString& str, LOGFONT &logfont);

struct PWFonts {
  PWFonts() : m_pCurrentFont(NULL), m_pModifiedFont(NULL), m_pDragFixFont(NULL) {}
  ~PWFonts() {delete m_pModifiedFont; delete m_pDragFixFont;}

  void SetUpFont(CWnd *pWnd, CFont *pfont);
  CFont *GetCurrentFont() {return m_pCurrentFont;}
  CFont *GetDragFixFont() {return m_pDragFixFont;}

  CFont *m_pCurrentFont;  // Do NOT delete - done in DboxMain
  CFont *m_pModifiedFont;
  CFont *m_pDragFixFont;  // Fix for lack of text during drag!
  static const COLORREF MODIFIED_COLOR;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
