/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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
  static void DeleteInstance();

  void SetUpFont(CWnd *pWnd, CFont *pfont);

  CFont *GetCurrentFont() {return pCurrentFont;}
  CFont *GetDragFixFont() {return pDragFixFont;}
  CFont *GetPasswordFont() {return pPasswordFont;}
  CFont *GetModifiedFont() {return pModifiedFont;}

  COLORREF GetModified_Color() {return MODIFIED_COLOR;}

  void GetCurrentFont(LOGFONT *pLF);
  void SetCurrentFont(LOGFONT *pLF);
  void GetPasswordFont(LOGFONT *pLF);
  void SetPasswordFont(LOGFONT *pLF);
  void ApplyPasswordFont(CWnd* pLF);
  void GetDefaultPasswordFont(LOGFONT &lf);

  void ExtractFont(const CString& str, LOGFONT &lf);

private:
  Fonts();
  ~Fonts() {}
  static Fonts *self; // singleton

  static CFont *pCurrentFont;
  static CFont *pModifiedFont;
  static CFont *pDragFixFont;  // Fix for lack of text during drag!
  static CFont *pPasswordFont;
  static const COLORREF MODIFIED_COLOR;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
