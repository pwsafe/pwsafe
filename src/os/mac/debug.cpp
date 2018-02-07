/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "../debug.h"

#if defined(_DEBUG) || defined(DEBUG)

// TRACE replacement - only need this Debug mode
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <iostream>

enum {MAX_LOG_STATEMENT = 1024*64, STARTING_LOG_STATEMENT = 256};

#if defined(UNICODE) || defined(_UNICODE)
# define vstprintf vswprintf
# define _stprintf_s swprintf
# define _tout wcout
# define _terr wcerr
#else
# define vstprintf vsprintf
# define _stprintf_s snprintf
# define _tout cout
# define _terr cerr
#endif

// Debug output - Same usage as MFC TRACE
void pws_os::Trace(LPCTSTR lpszFormat, ...)
{
  openlog("pwsafe:", LOG_PID, LOG_USER);
  va_list args;
  va_start(args, lpszFormat);

  TCHAR* buf = 0;
  int nwritten, len = STARTING_LOG_STATEMENT;
  do {
    len *= 2;
    delete [] buf;
    buf = new TCHAR[len+1];
    memset(buf, 0, sizeof(TCHAR)*(len+1));
    nwritten = vstprintf(buf, len, lpszFormat, args);
    //apple's documentation doesn't say if nwritten is +ve, -ve, 0 or if errno is set in case of overflow
  }
  while(!(nwritten > 0 && nwritten < len) && len <= MAX_LOG_STATEMENT);

#ifdef UNICODE
  size_t N = wcstombs(NULL, buf, 0) + 1;
  char *message = new char[N];
  wcstombs(message, buf, N);
  delete[] buf;
#else
  char* message = buf;
#endif
  
  syslog(LOG_DEBUG, "%s", message);

  delete[] message;
  closelog();

  va_end(args);
}

void pws_os::Trace0(LPCTSTR lpszFormat)
{
  openlog("pwsafe:", LOG_PID, LOG_USER);

#ifdef UNICODE
  size_t N = wcstombs(NULL, lpszFormat, 0) + 1;
  char *szbuffer = new char[N];
  wcstombs(szbuffer, lpszFormat, N);

  syslog(LOG_DEBUG, "%s", szbuffer);

  delete[] szbuffer;
#else
  syslog(LOG_DEBUG, lpszFormat);
#endif

  closelog();
}

bool pws_os::DisableDumpAttach()
{
  // prevent ptrace and creation of core dumps
  // No-op under DEBUG build, return true to avoid error handling
  return true;
}

#else   /* _DEBUG || DEBUG */
#include <sys/types.h>
#include <sys/ptrace.h>

bool pws_os::DisableDumpAttach()
{
  // prevent ptrace and creation of core dumps
  return ptrace(PT_DENY_ATTACH, 0, 0, 0) == 0;
}

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
#include "../../core/StringX.h"

// This routine uses Windows functions
DWORD pws_os::IssueError(const stringT &csFunction, bool bMsgBox)
{
  // Stub?
  if (bMsgBox)
    std::_tout << csFunction;
  else
    std::_terr << csFunction;

  return 0;
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
    Trace0(szBuffer);
  };
}
#else  /* _DEBUG or DEBUG */
DWORD pws_os::IssueError(const stringT &, bool )
{
  return 0;
}

void pws_os::HexDump(unsigned char *, const int &,
                     const stringT &, const int & )
{
}
#endif /* _DEBUG or DEBUG */
