/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWSFaultHandler.cpp
//-----------------------------------------------------------------------------

// Only produce minidumps in release code

/*
 * Get everything set up now.
 * Use only standard Windows functions in the FaultHandlers
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
#include <eh.h>
#include <signal.h>

// Exception types
enum {WIN32_STRUCTURED_EXCEPTION, TERMINATE_CALL, UNEXPECTED_CALL,
      NEW_OPERATOR_ERROR, PURE_CALL_ERROR, INVALID_PARAMETER_ERROR,
      SIGNAL_ABORT, SIGNAL_ILLEGAL_INST_FAULT, SIGNAL_TERMINATION,
      END_FAULTS};

static wchar_t *wcType[END_FAULTS] = {
                L"WIN32_STRUCTURED_EXCEPTION",
                L"TERMINATE_CALL",
                L"UNEXPECTED_CALL",
                L"NEW_OPERATOR_ERROR",
                L"PURE_CALL_ERROR",
                L"INVALID_PARAMETER_ERROR",
                L"SIGNAL_ABORT",
                L"SIGNAL_ILLEGAL_INST_FAULT",
                L"SIGNAL_TERMINATION"};

struct st_invp {
  const wchar_t* expression;
  const wchar_t* function;
  const wchar_t* file;
  unsigned int line;
};

LONG TakeMiniDump(struct _EXCEPTION_POINTERS *ExInfo, const int type,
                  struct st_invp *pinvp = NULL);

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *ExInfo);

void __cdecl terminate_dumphandler();
void __cdecl unexpected_dumphandler();
void __cdecl purecall_dumphandler();
int  __cdecl new_dumphandler(size_t);
void __cdecl invalid_parameter_dumphandler(const wchar_t* expression,
                const wchar_t* function, const wchar_t* file,
                unsigned int line, uintptr_t pReserved);

void sigabrt_dumphandler(int);
void sigill_dumphandler(int);
void sigterm_dumphandler(int);

const int MSGSIZE = 1024; // use static arrays, in case heap gets corrupted

static DWORD dwTimeStamp;
static int iMajor, iMinor, iBuild;
static wchar_t wcRevision[MSGSIZE];

static wchar_t wcMsg1[MSGSIZE];
static wchar_t wcMsg2[MSGSIZE];
static wchar_t wcMsg3[MSGSIZE];
static wchar_t wcCaption[MSGSIZE];

void LocalizeFaultHandler(HINSTANCE inst){
  LoadString(inst, IDS_MD_MSG1, wcMsg1, MSGSIZE);
  LoadString(inst, IDS_MD_MSG2, wcMsg2, MSGSIZE);
  LoadString(inst, IDS_MD_MSG3, wcMsg3, MSGSIZE);
  LoadString(inst, IDS_MD_CAPTION, wcCaption, MSGSIZE);
}

void InstallFaultHandler(const int major, const int minor, const int build,
                         const wchar_t *revision, const DWORD timestamp)
{
  iMajor = major;
  iMinor = minor;
  iBuild = build;
  wcscpy_s(wcRevision, revision);
  dwTimeStamp = timestamp;

  // Don't show the standard Application error box - we will handle it.
  // Note, there is no way to 'Add' an error mode. Only way is to
  // change it twice, first returns previous state, second adds what we want
  // to the original settings.
  DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
  SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);

  // Filling strings with default values, for handling errors while loading
  // localized resources.
  // After loading localized resources LocalizeFaultHandler should be called once more
  LocalizeFaultHandler(GetModuleHandle(NULL));

  // Catch Windows Unhandled Exceptions including SIGSEGV.
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)Win32FaultHandler);

  // Catch a program termination
  set_terminate(terminate_dumphandler);

  // Catch unexpected calls
  set_unexpected(unexpected_dumphandler);

  // Catch pure virtual function calls
  _set_purecall_handler(purecall_dumphandler);

  // Catch new operator memory allocation exceptions
  _set_new_mode(1); // Force malloc() to call new handler too
  _set_new_handler(new_dumphandler);

  // Catch invalid parameter exceptions
  _set_invalid_parameter_handler(invalid_parameter_dumphandler);

  // Catch an abnormal program termination
  _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
  signal(SIGABRT, sigabrt_dumphandler);

  // Catch a termination request
  signal(SIGTERM, sigterm_dumphandler);

  // Catch an illegal instruction
  signal(SIGILL, sigill_dumphandler);
}

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *ExInfo)
{
  return TakeMiniDump(ExInfo, WIN32_STRUCTURED_EXCEPTION);
}

void __cdecl terminate_dumphandler()
{
  TakeMiniDump(NULL, TERMINATE_CALL);
  exit(1); // Terminate program
}

void __cdecl unexpected_dumphandler()
{
  TakeMiniDump(NULL, UNEXPECTED_CALL);
  exit(1); // Terminate program
}

int __cdecl new_dumphandler(size_t)
{
  TakeMiniDump(NULL, NEW_OPERATOR_ERROR);
  exit(1); // Terminate program
}

void __cdecl purecall_dumphandler()
{
  TakeMiniDump(NULL, PURE_CALL_ERROR);
  exit(1); // Terminate program
}

void __cdecl invalid_parameter_dumphandler(const wchar_t* expression,
                const wchar_t* function, const wchar_t* file,
                unsigned int line, uintptr_t /* pReserved */)
{
  st_invp invp;
  invp.expression = expression;
  invp.function = function;
  invp.file = file;
  invp.line = line;

  TakeMiniDump(NULL, INVALID_PARAMETER_ERROR, &invp);
  exit(1); // Terminate program
}

void sigabrt_dumphandler(int)
{
  TakeMiniDump(NULL, SIGNAL_ABORT);
  exit(1); // Terminate program
}

void sigill_dumphandler(int)
{
  TakeMiniDump(NULL, SIGNAL_ILLEGAL_INST_FAULT);
  exit(1); // Terminate program
}

void sigterm_dumphandler(int)
{
  TakeMiniDump(NULL, SIGNAL_TERMINATION);
  exit(1); // Terminate program
}

LONG TakeMiniDump(struct _EXCEPTION_POINTERS *pExInfo, const int type,
                  st_invp *pinvp)
{
  // Use standard functions as much as possible
  wchar_t sz_TempName[MAX_PATH + 1];

  wchar_t sz_Drive[_MAX_DRIVE], sz_Dir[_MAX_DIR], sz_FName[_MAX_FNAME];
  wchar_t sz_TempPath[MSGSIZE];
  DWORD dwBufSize(MSGSIZE);

  // Get the temp path
  SecureZeroMemory(sz_TempPath, sizeof(sz_TempPath));
  DWORD dwrc = GetTempPathW(dwBufSize, sz_TempPath);
  if (dwrc == 0 || dwrc > dwBufSize)
    goto exit;

  // Create a temporary file.
  struct tm xt;
  struct __timeb32 timebuffer;
  _ftime32_s(&timebuffer);
  localtime_s(&xt, &(timebuffer.time));

  _wsplitpath_s(sz_TempPath, sz_Drive, _MAX_DRIVE, sz_Dir, _MAX_DIR,
                NULL, 0, NULL, 0);
  swprintf_s(sz_FName, _MAX_FNAME,
             L"PWS_Minidump_%04d%02d%02d_%02d%02d%02d%03d",
             xt.tm_year + 1900, xt.tm_mon + 1, xt.tm_mday,
             xt.tm_hour, xt.tm_min, xt.tm_sec, timebuffer.millitm);

  SecureZeroMemory(sz_TempName, sizeof(sz_TempName));
  _wmakepath_s(sz_TempName, MAX_PATH + 1, sz_Drive, sz_Dir, sz_FName, L"dmp");

  wchar_t sz_UserData[MAX_PATH];
  int numchars;

  SecureZeroMemory(sz_UserData, sizeof(sz_UserData));
  numchars = swprintf_s(sz_UserData, MAX_PATH,
                        L"PasswordSafe V%d.%d.%d(%s). Module timestamp: %08x; Type: %s",
                        iMajor, iMinor, iBuild, wcRevision, dwTimeStamp,
                        wcType[type]) + 1;

  HANDLE hFile = CreateFile(sz_TempName, GENERIC_READ | GENERIC_WRITE,
                            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION excpInfo;
    excpInfo.ClientPointers = FALSE;
    excpInfo.ExceptionPointers = pExInfo;
    excpInfo.ThreadId = GetCurrentThreadId();

    MINIDUMP_USER_STREAM UserStreams[2];

    UserStreams[0].Type = LastReservedStream + 1;
    UserStreams[0].Buffer = (void *)sz_UserData;
    UserStreams[0].BufferSize = numchars * sizeof(wchar_t);

    wchar_t sz_UserData2[MSGSIZE];
    SecureZeroMemory(sz_UserData2, sizeof(sz_UserData2));
    if (pinvp != NULL) {
      wchar_t sz_line[8] = {L'N', L'/', L'A', L'\0'};
      if (pinvp->line != 0)
        _itow_s(pinvp->line, sz_line, 8, 10);

      numchars = swprintf_s(sz_UserData2, MSGSIZE,
                            L"Expression: %s; Function: %s; File: %s; Line: %s",
                            wcslen(pinvp->expression) == 0 ? L"N/A" : pinvp->expression,
                            wcslen(pinvp->function)   == 0 ? L"N/A" : pinvp->function,
                            wcslen(pinvp->file)       == 0 ? L"N/A" : pinvp->file,
                            sz_line) + 1;
      UserStreams[1].Type = LastReservedStream + 2;
      UserStreams[1].Buffer = (void *)sz_UserData2;
      UserStreams[1].BufferSize = numchars * sizeof(wchar_t);
    }

    MINIDUMP_USER_STREAM_INFORMATION musi;
    musi.UserStreamCount = (pinvp == NULL) ? 1 : 2;
    musi.UserStreamArray = UserStreams;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                      MiniDumpNormal, (pExInfo != NULL) ? &excpInfo : NULL,
                      &musi, NULL);

    CloseHandle(hFile);

    wchar_t sz_errormsg[MSGSIZE * 4]; // not a good place for malloc, as the heap
                                      // may be corrupt...should be big enough!
    SecureZeroMemory(sz_errormsg, sizeof(sz_errormsg));

    wcscpy_s(sz_errormsg, MSGSIZE * 4, wcMsg1);      // Max size 'MSGSIZE'
    wcscat_s(sz_errormsg, MSGSIZE * 4, sz_FName);     // Max size '_MAX_FNAME'
    wcscat_s(sz_errormsg, MSGSIZE * 4, wcMsg2);      // Max size 'MSGSIZE'
    wcscat_s(sz_errormsg, MSGSIZE * 4, sz_TempPath);  // Max size 'MSGSIZE'
    wcscat_s(sz_errormsg, MSGSIZE * 4, wcMsg3);      // Max size 'MSGSIZE'

    // Issue error message
    wchar_t sz_Caption[MAX_PATH];
    SecureZeroMemory(sz_Caption, sizeof(sz_Caption));
    swprintf_s(sz_Caption, MAX_PATH, wcCaption, iMajor, iMinor, iBuild, wcRevision);
    int irc = ::MessageBox(GetForegroundWindow(), sz_errormsg, sz_Caption,
                           MB_YESNO | MB_APPLMODAL | MB_ICONERROR);

    // If user doesn't want to send us the minidump - delete it.
    if (irc == IDNO)
      _wremove(sz_TempName);
  } // valid file handle

 exit:
  return EXCEPTION_CONTINUE_SEARCH;
}

void RemoveFaultHandler()
{
  SetUnhandledExceptionFilter(NULL);
}
#endif
