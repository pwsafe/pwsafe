/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of rand.h
 */
#include <afx.h>
#include <Windows.h>
#include <stdlib.h>
#include <process.h>
#include "../rand.h"

// See the MSDN documentation for RtlGenRandom. We will try to load it
// and if that fails, use our own random number generator. The function
// call is indirected through a function pointer

static BOOLEAN (APIENTRY *pfnGetRandomData)(void*, ULONG) = NULL;

bool pws_os::InitRandomDataFunction()
{
  // Qualify full path name.  (Lockheed Martin) Secure Coding  11-14-2007
  TCHAR szFileName[ MAX_PATH ];
  memset( szFileName, 0, MAX_PATH );
  GetSystemDirectory( szFileName, MAX_PATH );
  int nLen = _tcslen( szFileName );
  if (nLen > 0) {
    if (szFileName[ nLen - 1 ] != '\\')
      _tcscat_s( szFileName, MAX_PATH, _T("\\") );
  }
  _tcscat_s( szFileName, MAX_PATH, _T("ADVAPI32.DLL") );

  HMODULE hLib = LoadLibrary( szFileName );
  // End of change.  (Lockheed Martin) Secure Coding  11-14-2007

  BOOLEAN (APIENTRY *pfnGetRandomDataT)(void*, ULONG) = NULL;
  if (hLib != NULL) {
    pfnGetRandomDataT = (BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib,"SystemFunction036");
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
    t = time(NULL);
    pid = _getpid();
    ticks = GetTickCount();
    unsigned char *pc = static_cast<unsigned char *>(p);
    memcpy(pc, &t, sizeof(t));
    memcpy(pc + sizeof(t), &pid, sizeof(pid));
    memcpy(pc + sizeof(t) + sizeof(pid), &ticks, sizeof(ticks));
  }
}    
