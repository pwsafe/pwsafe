/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file Debug.cpp
// Functions that are used mainly (or only) for debugging.
//-----------------------------------------------------------------------------
#include "Debug.h"

#ifndef DEBUG
void PWSDebug::IssueError(const stringT &)
{
}
void PWSDebug::HexDump(unsigned char *, const int, 
                       const stringT &, const int)
{
}
#else
void PWSDebug::IssueError(const stringT &csFunction)
{
  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;

  const DWORD dw = GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL);

  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
                                    (lstrlen((LPCTSTR)lpMsgBuf) + csFunction.length() + 40) * sizeof(TCHAR)); 
  wsprintf((LPTSTR)lpDisplayBuf, TEXT("%s failed with error %d: %s"), 
           csFunction.c_str(), dw, lpMsgBuf); 
  MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}

void PWSDebug::HexDump(unsigned char *pmemory, const int length, 
                       const stringT &cs_prefix, const int maxnum)
{
  unsigned char *pmem;
  stringT cs_outbuff, cs_hexbuff, cs_charbuff;
  int i, j, len(length);
  unsigned char c;

  pmem = pmemory;
  while (len > 0) {
    // Show offset for this line.
    cs_charbuff.clear();
    cs_hexbuff.clear();
    Format(cs_outbuff, _T("%s: %08x *"), cs_prefix.c_str(), pmem);

    // Format hex portion of line and save chars for ascii portion
    if (len > maxnum)
      j = maxnum;
    else
      j = len;

    for (i = 0; i < j; i++) {
      c = *pmem++;

      if ((i % 4) == 0 && i != 0)
        cs_outbuff += _T(' ');

      cs_hexbuff.Format(_T("%02x"), c);
      cs_outbuff += cs_hexbuff;

      if (c >= 32 && c < 127)
        cs_charbuff += (TCHAR)c;
      else
        cs_charbuff += _T('.');
    }

    j = maxnum - j;

    // Fill out hex portion of short lines.
    for (i = j; i > 0; i--) {
      if ((i % 4) != 0)
        cs_outbuff += _T("  ");
      else
        cs_outbuff += _T("   ");
    }

    // Add ASCII character portion to line.
    cs_outbuff += _T("* |");
    cs_outbuff += cs_charbuff;

    // Fill out end of short lines.
    for (i = j; i > 0; i--)
      cs_outbuff += _T(' ');

    cs_outbuff += _T('|');

    // Next line
    len -= maxnum;

    TRACE(_T("%s\n"), cs_outbuff.c_str());
  };
}
#endif
