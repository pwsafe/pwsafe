/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

////////////////////////////////////////////////////////////////////
// winutils.h - file for various windows-specific related utility functions,
// macros, classes, etc

#ifndef __WINUTILS_H__
#define __WINUTILS_H__

#include <xstring>
#include <afxwin.h>

namespace WinUtil {
  void RelativizePath(std::wstring &);
  HRGN GetWorkAreaRegion();
  bool OfferConfigMigration();
  bool PerformConfigMigration();
  UINT GetDPI(HWND hwnd = nullptr); // wrapper for debugging and hiding Win10 compatibility breakage
  int  GetSystemMetrics(int nIndex, HWND hwnd = nullptr); // Hide Win10 compatibility breakage
  void ResizeBitmap(CBitmap& bmp_src, CBitmap& bmp_dst, int dstW, int dstH);
  void FixBitmapBackground(CBitmap& bm);
  BOOL LoadScaledBitmap(CBitmap& bitmap, UINT nID, bool fixBckgrnd = true, HWND hwnd = nullptr);
}
#endif // __WINUTILS_H__

