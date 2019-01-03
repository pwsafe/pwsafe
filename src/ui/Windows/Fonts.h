/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// Fonts.h
//-----------------------------------------------------------------------------

#include <string>

class Fonts
{
public:
  enum Symbol {PROTECT, ATTACHMENT};
  enum PWSFont {TREELIST, ADDEDIT};

  static Fonts *GetInstance(); // singleton
  void DeleteInstance();

  void SetUpFont(CWnd *pWnd, CFont *pfont);

  CFont *GetTreeListFont() const { return m_pTreeListFont; }
  CFont *GetAddEditFont() const { return m_pAddEditFont; }
  CFont *GetDragFixFont() const { return m_pDragFixFont; }
  CFont *GetPasswordFont() const { return m_pPasswordFont; }
  CFont *GetNotesFont() const { return m_pNotesFont; }

  CFont *GetItalicTreeListFont() const { return m_pItalicTreeListFont; }
  CFont *GetItalicAddEditFont() const { return m_pItalicAddEditFont; }

  COLORREF GetModified_Color() {return MODIFIED_COLOR;}

  void GetTreeListFont(LOGFONT *pLF);
  void SetTreeListFont(LOGFONT *pLF, const int iPtSz);
  void GetAddEditFont(LOGFONT *pLF);
  void SetAddEditFont(LOGFONT *pLF, const int iPtSz);
  void GetPasswordFont(LOGFONT *pLF);
  void SetPasswordFont(LOGFONT *pLF, const int iPtSz);
  void GetNotesFont(LOGFONT *pLF);
  void SetNotesFont(LOGFONT *pLF, const int iPtSz);

  void ApplyPasswordFont(CWnd *pLF);

  void GetDefaultPasswordFont(LOGFONT &lf);
  void GetDefaultTreeListFont(LOGFONT &lf);
  void GetDefaultAddEditFont(LOGFONT &lf);
  void GetDefaultNotesFont(LOGFONT &lf);

  bool ExtractFont(const std::wstring& str, LOGFONT &lf);

  LONG CalcHeight(const bool bIncludeNotesFont = false) const;

  std::wstring GetProtectedSymbol(const PWSFont font = TREELIST);
  std::wstring GetAttachmentSymbol(const PWSFont font = TREELIST);
  void VerifySymbolsSupported();
  bool IsSymbolSuported(const Symbol symbol, const PWSFont font = TREELIST);

private:
  Fonts();
  ~Fonts() {}

  bool IsCharacterSupported(std::wstring &sSymbol, const bool bTreeListFont = true);

  static Fonts *self; // singleton

  CFont *m_pTreeListFont;
  CFont *m_pAddEditFont;
  CFont *m_pItalicTreeListFont;
  CFont *m_pItalicAddEditFont;
  CFont *m_pDragFixFont;  // Fix for lack of text during drag!
  CFont *m_pPasswordFont;
  CFont *m_pNotesFont;

  const COLORREF MODIFIED_COLOR;

  std::wstring m_sProtect, m_sAttachment;

  bool m_bProtectSymbolSupportedTreeList, m_bProtectSymbolSupportedAddEdit;
  bool m_bAttachmentSymbolSupportedTreeList, m_bAttachmentSymbolSupportedAddEdit;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
