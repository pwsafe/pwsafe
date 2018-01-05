/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of debug.h
 */

// TRACE replacement
#include "../debug.h"
#include "../../core/util.h"
#include <wtypes.h>

#if defined(_DEBUG) || defined(DEBUG)

#include <stdio.h>

// Debug output - Same usage as MFC TRACE
// Adds timestamp to start of every piece of debug output
void pws_os::Trace(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);

  stringT sTimeStamp;
  PWSUtil::GetTimeStamp(sTimeStamp);
  int num_required, num_written;

  num_required = GetStringBufSize(lpszFormat, args);
  va_end(args);//after using args we should reset list
  va_start(args, lpszFormat);

  TCHAR *szBuffer = new TCHAR[num_required];
  num_written = _vsntprintf_s(szBuffer, num_required, num_required - 1, lpszFormat, args);
  ASSERT(num_required == num_written + 1);
  szBuffer[num_required - 1] = '\0';

  const size_t N = num_written + sTimeStamp.length() + 2;
  TCHAR *szDebugString = new TCHAR[N];

  _tcscpy_s(szDebugString, N, sTimeStamp.c_str());
  _tcscat_s(szDebugString, N, _T(" "));
  _tcscat_s(szDebugString, N, szBuffer);

  OutputDebugString(szDebugString);
  delete[] szDebugString;
  delete[] szBuffer;
  va_end(args);
}

void pws_os::Trace0(LPCTSTR lpszFormat)
{
  stringT sTimeStamp;
  PWSUtil::GetTimeStamp(sTimeStamp);

  const size_t N = _tcslen(lpszFormat) + sTimeStamp.length() + 2;
  TCHAR *szDebugString = new TCHAR[N];

  _tcscpy_s(szDebugString, N, sTimeStamp.c_str());
  _tcscat_s(szDebugString, N, _T(" "));
  _tcscat_s(szDebugString, N, lpszFormat);

  OutputDebugString(szDebugString);
  delete [] szDebugString;
}
#else   /* _DEBUG || DEBUG */
void pws_os::Trace(LPCTSTR , ...)
{
//  Do nothing in non-Debug mode
}
void pws_os::Trace0(LPCTSTR )
{
//  Do nothing in non-Debug mode
}
#endif  /* _DEBUG || DEBUG */

#if defined(_DEBUG) || defined(DEBUG)
#include "windows.h"
#include "../debug.h"
#include "../../core/StringX.h"

// This routine uses Windows functions
DWORD pws_os::IssueError(const stringT &csFunction, bool bMsgBox)
{
  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;

  const DWORD dwLastError = GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dwLastError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL);

  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
       (lstrlen((LPCTSTR)lpMsgBuf) + csFunction.length() + 40) * sizeof(TCHAR));
  wsprintf((LPTSTR)lpDisplayBuf, TEXT("%s failed with error %d: %s"),
           csFunction.c_str(), dwLastError, lpMsgBuf);
  if (bMsgBox)
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
  else
    OutputDebugString((LPCTSTR)lpDisplayBuf);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);

  return dwLastError;
}

void pws_os::HexDump(unsigned char *pmemory, const int &length,
                       const stringT &cs_prefix, const int &maxnum)
{
  TCHAR szBuffer[256];
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

      Format(cs_hexbuff, _T("%02x"), c);
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

    _stprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR),
                _T("%s\n"), cs_outbuff.c_str());
    OutputDebugString(szBuffer);
  };
}
#else  /* _DEBUG or DEBUG */
DWORD pws_os::IssueError(const stringT &, bool )
{
//  Do nothing in non-Debug mode
  return 0;
}

void pws_os::HexDump(unsigned char *, const int &,
                     const stringT &, const int &)
{
//  Do nothing in non-Debug mode
}
#endif  /* _DEBUG or DEBUG */

bool pws_os::DisableDumpAttach()
{
  // On non-Windows, this prevents ptrace and creation of core dumps
  // No-op here, return true to avoid error handling
  return true;
}
