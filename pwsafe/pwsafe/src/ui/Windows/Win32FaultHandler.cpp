/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file Win32FaultHandler.cpp
//-----------------------------------------------------------------------------

// Only produce minidumps in release code

/*
 * Get everything set up now.
 * Use only standard Windows functions in FaultHandler
 *
 * *** *** *** *** ***
 *  NOTE: NO MFC CODE
 * *** *** *** *** ***
 */

#ifndef _DEBUG

#include "stdafx.h"

#include "resource3.h"

#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

#include "Dbghelp.h"

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *ExInfo);
const int MSGSIZE = 1024; // use static arrays, in case heap gets corrupted

static DWORD dwTimeStamp;
static int iMajor, iMinor, iBuild;
static wchar_t wcRevision[MSGSIZE];

static wchar_t wcMsg1[MSGSIZE];
static wchar_t wcMsg2[MSGSIZE];
static wchar_t wcMsg3[MSGSIZE];
static wchar_t wcCaption[MSGSIZE];

void InstallFaultHandler(const int major, const int minor, const int build,
                         const wchar_t *revision, const DWORD timestamp)
{
  iMajor = major;
  iMinor = minor;
  iBuild = build;
  wcscpy_s(wcRevision, revision);
  dwTimeStamp = timestamp;

  // Set up message now - just in case!
  LoadString(GetModuleHandle(NULL), IDS_MD_MSG1, wcMsg1, MSGSIZE);
  LoadString(GetModuleHandle(NULL), IDS_MD_MSG2, wcMsg2, MSGSIZE);
  LoadString(GetModuleHandle(NULL), IDS_MD_MSG3, wcMsg3, MSGSIZE);
  LoadString(GetModuleHandle(NULL), IDS_MD_CAPTION, wcCaption, MSGSIZE);

  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)Win32FaultHandler);
}

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *ExInfo)
{
  // Use standard functions as much as possible
  wchar_t szTempName[MAX_PATH + 1];

  wchar_t szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME];
  wchar_t wcTempPath[4096];
  DWORD dwBufSize(4096);

  // Get the temp path
  DWORD dwrc = GetTempPathW(dwBufSize, wcTempPath);
  if (dwrc == 0 || dwrc > dwBufSize)
    goto exit;

  // Create a temporary file.
  struct tm xt;
  struct __timeb32 timebuffer; 
  _ftime32_s(&timebuffer);
  localtime_s(&xt, &(timebuffer.time));

  _wsplitpath_s(wcTempPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, 
                NULL, 0, NULL, 0);
  swprintf_s(szFName, _MAX_FNAME, L"PWS_Minidump_%04d%02d%02d_%02d%02d%02d%03d",
             xt.tm_year + 1900, xt.tm_mon + 1, xt.tm_mday,
             xt.tm_hour, xt.tm_min, xt.tm_sec, timebuffer.millitm);
  _wmakepath_s(szTempName, MAX_PATH + 1, szDrive, szDir, szFName, L"dmp");

  wchar_t szUserData[MAX_PATH];
  swprintf_s(szUserData, MAX_PATH, L"PasswordSafe V%d.%d.%d(%s). Module timestamp: %08x",
             iMajor, iMinor, iBuild, wcRevision, dwTimeStamp);

  HANDLE hFile = CreateFile(szTempName, GENERIC_READ | GENERIC_WRITE, 
                            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION excpInfo;
    excpInfo.ClientPointers = FALSE;
    excpInfo.ExceptionPointers = ExInfo;
    excpInfo.ThreadId = GetCurrentThreadId();

    MINIDUMP_USER_STREAM UserStreams[1];

    UserStreams[0].Type = LastReservedStream + 1;
    UserStreams[0].Buffer = (void *)szUserData;
    UserStreams[0].BufferSize = sizeof(szUserData);

    MINIDUMP_USER_STREAM_INFORMATION musi;
    musi.UserStreamCount = 1;
    musi.UserStreamArray = UserStreams;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                      MiniDumpNormal, &excpInfo, &musi, NULL);
    
    CloseHandle(hFile);

    size_t len = wcslen(wcMsg1) + wcslen(szFName) + wcslen(wcMsg2) + 
      wcslen(wcTempPath) + wcslen(wcMsg3) + 3;
    wchar_t sz_errormsg[4*MSGSIZE]; // not a good place for malloc, as the heap
    //                                 may be corrupt...
    wcscpy_s(sz_errormsg, len, wcMsg1);
    wcscat_s(sz_errormsg, len, szFName);
    wcscat_s(sz_errormsg, len, wcMsg2);
    wcscat_s(sz_errormsg, len, wcTempPath);
    wcscat_s(sz_errormsg, len, wcMsg3);

    // Issue error message
    wchar_t szCaption[MAX_PATH];
    swprintf_s(szCaption, MAX_PATH, wcCaption, iMajor, iMinor, iBuild, wcRevision);
    int irc = ::MessageBox(GetForegroundWindow(), sz_errormsg, szCaption, 
                           MB_YESNO | MB_APPLMODAL | MB_ICONERROR);
    if (irc == IDNO)
      _wremove(szTempName);
  } // valid file handle
 exit:
  return EXCEPTION_CONTINUE_SEARCH;
}

void RemoveFaultHandler()
{
  SetUnhandledExceptionFilter(NULL);
}
#endif
