/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWFontDialog

enum {PWFONT, TLFONT};

extern LOGFONT dfltTreeListFont;

class CPWFontDialog : public CFontDialog
{
  DECLARE_DYNAMIC(CPWFontDialog)

public:
  CPWFontDialog(LPLOGFONT lplfInitial = NULL,
    DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
    CDC* pdcPrinter = NULL,
    CWnd* pParentWnd = NULL,
    int iType = PWFONT);
#ifndef _AFX_NO_RICHEDIT_SUPPORT
  CPWFontDialog(const CHARFORMAT& charformat,
    DWORD dwFlags = CF_SCREENFONTS,
    CDC* pdcPrinter = NULL,
    CWnd* pParentWnd = NULL,
    int iType = PWFONT);
#endif
  virtual ~CPWFontDialog();

  CString m_sampletext, m_title;
  int m_iType;

// Dialog Data
  //{{AFX_DATA(CXFontDialog)
  //enum { IDD = IDD_PWFONTDIALOG };
  //}}AFX_DATA

protected:
  DECLARE_MESSAGE_MAP()
};
