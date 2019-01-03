/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of rand.h
 */
#include <time.h>
#include <stdlib.h>
#include <process.h>

#include "../rand.h"
#include "../lib.h"

// See the MSDN documentation for RtlGenRandom. We will try to load it
// and if that fails, use our own random number generator. The function
// call is indirected through a function pointer

static BOOLEAN (APIENTRY *pfnGetRandomData)(void*, ULONG) = NULL;

bool pws_os::InitRandomDataFunction()
{
  HMODULE hLib = HMODULE(pws_os::LoadLibrary(_T("ADVAPI32.DLL"), loadLibraryTypes::SYS));

  BOOLEAN (APIENTRY *pfnGetRandomDataT)(void*, ULONG) = NULL;
  if (hLib != NULL) {
    pfnGetRandomDataT =
      (BOOLEAN (APIENTRY *)(void*,ULONG))pws_os::GetFunction(hLib, "SystemFunction036");
    if (pfnGetRandomDataT) {
      pfnGetRandomData = pfnGetRandomDataT;
    }
  }
  return (hLib != NULL && pfnGetRandomDataT != NULL);
}

bool pws_os::GetRandomData(void *p, unsigned long len)
{
  if (pfnGetRandomData != NULL)
    return (*pfnGetRandomData)(p, len) == TRUE;
  else
    return false;
}

void pws_os::GetRandomSeed(void *p, unsigned &slen)
{
  time_t t;
  int pid;
  DWORD ticks;

  if (p == NULL) {
    slen = sizeof(t) + sizeof(pid) + sizeof(ticks);
  } else {
    ASSERT(slen == sizeof(t) + sizeof(pid) + sizeof(ticks));

    // BR1475 - if we have a good crypto source, use it here too.
    if ((pfnGetRandomData != nullptr) && (*pfnGetRandomData)(p, slen) == TRUE)
      return; // adding a time-based "seed" is wrong when using RtlGenRandom

    SYSTEMTIME st;
    struct tm tms;
    GetSystemTime(&st);

    memset(&tms, 0, sizeof(tm));
    tms.tm_year = st.wYear - 1900;
    tms.tm_mon = st.wMonth - 1;
    tms.tm_mday = st.wDay;
    tms.tm_hour = st.wHour;
    tms.tm_min = st.wMinute;
    tms.tm_sec = st.wSecond;
    t = mktime(&tms);

    pid = _getpid();
    ticks = GetTickCount();
    unsigned char *pc = static_cast<unsigned char *>(p);
    memcpy(pc, &t, sizeof(t));
    memcpy(pc + sizeof(t), &pid, sizeof(pid));
    memcpy(pc + sizeof(t) + sizeof(pid), &ticks, sizeof(ticks));
  }
}    
