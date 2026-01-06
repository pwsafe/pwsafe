/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of rand.h
 */

#include <Winsock2.h>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <process.h>
#include <bcrypt.h>
#include "../rand.h"
#include "../lib.h"


bool pws_os::InitRandomDataFunction()
{
  // Check if BCryptGenRandom is available
  HMODULE hBcrypt = (HMODULE)pws_os::LoadLibrary(TEXT("bcrypt.dll"), loadLibraryTypes::SYS);
  if (hBcrypt == nullptr) {
    return false; // BCryptGenRandom not available
  }
  // Check if BCryptGenRandom function is available
  FARPROC pFunc = GetProcAddress(hBcrypt, "BCryptGenRandom");
  FreeLibrary(hBcrypt);

  return pFunc != nullptr;
}


bool pws_os::GetRandomData(void *p, unsigned long len)
{
  NTSTATUS status = BCryptGenRandom(nullptr, static_cast<PUCHAR>(p), len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  return status == 0;
}

void pws_os::GetRandomSeed(void *p, unsigned &slen)
{
  time_t t;
  int pid;
  DWORD ticks;

  if (p == nullptr) {
    slen = sizeof(t) + sizeof(pid) + sizeof(ticks);
  } else {
    ASSERT(slen == sizeof(t) + sizeof(pid) + sizeof(ticks));

    // BR1475 - if we have a good crypto source, use it here too.
    // Use BCryptGenRandom for seeding if available
    if (GetRandomData(p, slen))
      return; // adding a time-based "seed" is wrong when using BCryptGenRandom


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
