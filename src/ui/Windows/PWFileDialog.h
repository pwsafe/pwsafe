/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CPWFileDialog : public CFileDialog
{
public:
  CPWFileDialog(BOOL bOpenFileDialog, LPCWSTR lpszDefExt = NULL, LPCWSTR lpszFileName = NULL,
                DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCWSTR lpszFilter = NULL,
                CWnd* pParentWnd = NULL, DWORD dwSize = 0, BOOL bVistaStyle = TRUE)
  : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter,
                pParentWnd, dwSize, bVistaStyle) {}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  // Following override to stop accelerators interfering
  virtual INT_PTR DoModal();

  DECLARE_DYNAMIC(CPWFileDialog)
};
