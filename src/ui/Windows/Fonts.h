/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// Fonts.h
//-----------------------------------------------------------------------------

class Fonts
{
public:
  static Fonts *GetInstance(); // singleton
  void DeleteInstance();

  void SetUpFont(CWnd *pWnd, CFont *pfont);

  CFont *GetCurrentFont() const { return m_pCurrentFont; }
  CFont *GetDragFixFont() const { return m_pDragFixFont; }
  CFont *GetPasswordFont() const { return m_pPasswordFont; }
  CFont *GetModifiedFont() const { return m_pModifiedFont; }
  CFont *GetNotesFont() const { return m_pNotesFont; }

  COLORREF GetModified_Color() {return MODIFIED_COLOR;}

  void GetCurrentFont(LOGFONT *pLF);
  void SetCurrentFont(LOGFONT *pLF);
  void GetPasswordFont(LOGFONT *pLF);
  void SetPasswordFont(LOGFONT *pLF);
  void GetNotesFont(LOGFONT *pLF);
  void SetNotesFont(LOGFONT *pLF);

  void ApplyPasswordFont(CWnd* pLF);

  void GetDefaultPasswordFont(LOGFONT &lf);

  void ExtractFont(const CString& str, LOGFONT &lf);

  LONG CalcHeight() const;

private:
  Fonts();
  ~Fonts() {}
  static Fonts *self; // singleton

  CFont *m_pCurrentFont;
  CFont *m_pModifiedFont;
  CFont *m_pDragFixFont;  // Fix for lack of text during drag!
  CFont *m_pPasswordFont;
  CFont *m_pNotesFont;

  const COLORREF MODIFIED_COLOR;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
