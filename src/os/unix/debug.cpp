/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "../debug.h"
#include "../core/Util.h"

#if defined(_DEBUG) || defined(DEBUG)

// TRACE replacement - only need this Debug mode
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

// Debug output - Same usage as MFC TRACE
void pws_os::Trace(LPCTSTR lpszFormat, ...)
{
  openlog("pwsafe:", LOG_PID|LOG_PERROR, LOG_USER);
  va_list args;
  va_start(args, lpszFormat);

  unsigned int num_required = GetStringBufSize(lpszFormat, args);
  va_end(args);//after using args we should reset list
  va_start(args, lpszFormat);

  wchar_t *wcbuffer = new wchar_t[num_required];
  int num_written = vswprintf(wcbuffer, num_required, lpszFormat, args);
  assert(static_cast<int>(num_required) == num_written+1);
  wcbuffer[num_required-1] = L'\0';

  size_t N = wcstombs(nullptr, wcbuffer, 0) + 1;
  char *szbuffer = new char[N];
  wcstombs(szbuffer, wcbuffer, N);
  delete[] wcbuffer;
  syslog(LOG_DEBUG, "%s", szbuffer); // NOT %ls!

  delete[] szbuffer;
  closelog();

  va_end(args);
}

void pws_os::Trace0(LPCTSTR lpszFormat)
{
  openlog("pwsafe:", LOG_PID|LOG_PERROR, LOG_USER);

  size_t N = wcstombs(nullptr, lpszFormat, 0) + 1;
  char *szbuffer = new char[N];
  wcstombs(szbuffer, lpszFormat, N);

  syslog(LOG_DEBUG, "%s", szbuffer); // NOT %ls!

  delete[] szbuffer;
  closelog();
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
#include "../../core/StringX.h"
#include <iostream>
#include "../pws_tchar.h"

// This routine uses Windows functions
DWORD pws_os::IssueError(const stringT &csFunction, bool bMsgBox)
{
  // Stub?
  if (bMsgBox)
    std::cout << csFunction.c_str();
  else
    std::cerr << csFunction.c_str();

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
    Format(cs_outbuff, _T("%ls: %08x *"), cs_prefix.c_str(), pmem);

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
        cs_charbuff += static_cast<TCHAR>(c);
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
                _T("%ls\n"), cs_outbuff.c_str());
    Trace0(szBuffer);
  };
}

bool pws_os::DisableDumpAttach()
{
  // prevent ptrace and creation of core dumps
  // No-op under DEBUG build, return true to avoid error handling
  return true;
}

#else  /* _DEBUG or DEBUG */
#ifdef __FreeBSD__
/*bool pws_os::DisableDumpAttach()
{
  // prevent ptrace and creation of core dumps
  // No-op under DEBUG build, return true to avoid error handling
  return true;
}*/
#include <unistd.h>
#include <sys/procctl.h>
#include <sys/resource.h>
bool pws_os::DisableDumpAttach()
{
  // prevent ptrace and creation of core dumps
  int mode = PROC_TRACE_CTL_DISABLE;
  procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  return true;
}
#else
#include <sys/prctl.h>

bool pws_os::DisableDumpAttach()
{
  // prevent ptrace and creation of core dumps
  return prctl(PR_SET_DUMPABLE, 0) == 0;
}
#endif /* __FreeBSD__ */

DWORD pws_os::IssueError(const stringT &, bool )
{
  return 0;
}

void pws_os::HexDump(unsigned char *, const int &,
                     const stringT &, const int & )
{
}
#endif /* _DEBUG or DEBUG */
