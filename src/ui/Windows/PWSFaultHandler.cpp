/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "PWSFaultHandler.h"

#include "core/StringX.h"
#include "core/PWSLog.h"

#include "os/lib.h"

#include "resource3.h"

#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include <new.h>

#include <eh.h>
#include <signal.h>

 // Use for size of static arrays, in case heap gets corrupted
#define MSGSIZE        1024
#define MSGSIZEx4      4096
#define USERSTREAMSIZE 1024

// Exception types
enum {WIN32_STRUCTURED_EXCEPTION, TERMINATE_CALL, UNEXPECTED_CALL,
      NEW_OPERATOR_ERROR, PURE_CALL_ERROR, INVALID_PARAMETER_ERROR,
      SIGNAL_ABORT, SIGNAL_ILLEGAL_INST_FAULT, SIGNAL_TERMINATION,
      END_FAULTS};

typedef BOOL (WINAPI *PMDWD) (HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
                              PMINIDUMP_EXCEPTION_INFORMATION,
                              PMINIDUMP_USER_STREAM_INFORMATION,
                              PMINIDUMP_CALLBACK_INFORMATION);

// Some forward declarations:
static PMDWD pfcnMiniDumpWriteDump(NULL);
static HMODULE hDbgHelp(NULL);
static LONG TakeMiniDump(struct _EXCEPTION_POINTERS *ExInfo, const int type,
                         struct st_invp *pinvp = NULL);
static void __cdecl terminate_dumphandler();
static void __cdecl unexpected_dumphandler();
static int __cdecl new_dumphandler(size_t);
static void __cdecl purecall_dumphandler();
static void __cdecl bad_parameter_dumphandler(const wchar_t* expression,
                                              const wchar_t* function,
                                              const wchar_t* file,
                                              unsigned int line,
                                              uintptr_t /* pReserved */);
static void signal_dumphandler(int isignal);


// Make nearly everything static so that it is available when needed and
// we do not have to allocate memory
struct st_invp {
  const wchar_t *expression;
  const wchar_t *function;
  const wchar_t *file;
  unsigned int line;
};

static wchar_t *szNA = L"N/A";
static st_invp invp;

static terminate_handler          old_terminate_handler(NULL);
static unexpected_handler         old_unexpected_handler(NULL);
static _purecall_handler          old_purecall_handler(NULL);
static _PNH                       old_new_handler(NULL);
static _invalid_parameter_handler old_bad_parameter_handler(NULL);
static int old_new_mode(0);

static wchar_t wcMsg1[MSGSIZE];
static wchar_t wcMsg2[MSGSIZE];
static wchar_t wcMsg3[MSGSIZE];
static wchar_t wcCaption[MSGSIZE];

// Minidump User streams - fixed storage just incase heap damaged
// 0 - Module information, DB open and R/W status + a few others
// 1 - Boolean preferences
// 2 - Integer preferences
// 3 - String  preferences
static wchar_t szUserStream0[USERSTREAMSIZE];
static wchar_t szUserStream1[USERSTREAMSIZE];
static wchar_t szUserStream2[USERSTREAMSIZE];
static wchar_t szUserStream3[USERSTREAMSIZE];

// User stream 0 data
static DWORD dwTimeStamp;
static int iMajor, iMinor, iBuild;
static wchar_t wcRevision[MSGSIZE];

static bool st_bOpen;
static bool st_bRW;
static int st_cfgloc;

void LocalizeFaultHandler(HINSTANCE inst) {
  LoadString(inst, IDS_MD_MSG1, wcMsg1, MSGSIZE);
  LoadString(inst, IDS_MD_MSG2, wcMsg2, MSGSIZE);
  LoadString(inst, IDS_MD_MSG3, wcMsg3, MSGSIZE);
  LoadString(inst, IDS_MD_CAPTION, wcCaption, MSGSIZE);
}

void InstallFaultHandler(const int major, const int minor, const int build,
                         const wchar_t *revision, const DWORD timestamp)
{
  
  hDbgHelp = HMODULE(pws_os::LoadLibrary(L"DbgHelp.dll",
                                         pws_os::loadLibraryTypes::SYS));
  if (hDbgHelp == NULL)
    return;

  pfcnMiniDumpWriteDump = PMDWD(pws_os::GetFunction(hDbgHelp, "MiniDumpWriteDump"));

  if (pfcnMiniDumpWriteDump == NULL) {
    pws_os::FreeLibrary(hDbgHelp);
    hDbgHelp = NULL;
    return;
  }

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
  old_terminate_handler = set_terminate(terminate_dumphandler);

  // Catch unexpected calls
  old_unexpected_handler = set_unexpected(unexpected_dumphandler);

  // Catch pure virtual function calls
  old_purecall_handler = _set_purecall_handler(purecall_dumphandler);

  // Catch new operator memory allocation exceptions (CRT)
  old_new_mode = _set_new_mode(1); // Force malloc() to call new handler too
  old_new_handler = _set_new_handler(new_dumphandler);

  // Catch invalid parameter exceptions
  old_bad_parameter_handler = _set_invalid_parameter_handler(bad_parameter_dumphandler);

  // Catch an abnormal program termination
  _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
  signal(SIGABRT, signal_dumphandler);

  // Catch a termination request
  signal(SIGTERM, signal_dumphandler);

  // Catch an illegal instruction
  signal(SIGILL, signal_dumphandler);
}

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *pExInfo)
{
  return TakeMiniDump(pExInfo, WIN32_STRUCTURED_EXCEPTION, NULL);
}

static void __cdecl terminate_dumphandler()
{
  TakeMiniDump(NULL, TERMINATE_CALL);
  exit(1); // Terminate program
}

static void __cdecl unexpected_dumphandler()
{
  TakeMiniDump(NULL, UNEXPECTED_CALL);
  exit(1); // Terminate program
}

static int __cdecl new_dumphandler(size_t)
{
  TakeMiniDump(NULL, NEW_OPERATOR_ERROR);
  exit(1); // Terminate program
}

static void __cdecl purecall_dumphandler()
{
  TakeMiniDump(NULL, PURE_CALL_ERROR);
  exit(1); // Terminate program
}

static void __cdecl bad_parameter_dumphandler(const wchar_t* expression,
                const wchar_t* function, const wchar_t* file,
                unsigned int line, uintptr_t /* pReserved */)
{
  invp.expression = (expression == NULL || wcslen(expression) == 0) ? szNA : expression;
  invp.function = (function == NULL || wcslen(function) == 0) ? szNA : function;
  invp.file = (file == NULL || wcslen(file) == 0) ? szNA : file;
  invp.line = line;

  TakeMiniDump(NULL, INVALID_PARAMETER_ERROR, &invp);
  exit(1); // Terminate program
}

static void signal_dumphandler(int isignal)
{
  int itype;
  switch (isignal) {
    case SIGABRT:
      itype = SIGNAL_ABORT;
      break;
    case SIGILL:
      itype = SIGNAL_ILLEGAL_INST_FAULT;
      break;
    case SIGTERM:
      itype = SIGNAL_TERMINATION;
      break;
    default:
      return;
  }
  TakeMiniDump(NULL, itype);
  exit(1); // Terminate program
}

void PopulateMinidumpUserStreams(PWSprefs *prefs, bool bOpen, bool bRW, UserStream iStream)
{
  /*
    See PWSprefs::GetAllBoolPrefs, PWSprefs::GetAllIntPrefs & PWSprefs::GetAllStringPrefs
    Space delimiter strings
    Boolean preferences:
      # (1-byte), PrefType (1-byte), value (1-byte: 0 = false; 1 = true)
    Integer preferences:
      # (1-byte), PrefType (1-byte), value (4-hex bytes)
    String  preferences (only a safe subset):
      # (1-byte), PrefType (1-byte), value (n-bytes) - first/last char == delimiter
  */

  // Save information for user stream 0 later
  if (iStream == usAll || iStream == us0) {
    st_bOpen = bOpen;
    st_bRW = bRW;
    st_cfgloc = prefs->GetConfigOption();
  }

  StringX sx_Buffer;
  int len;
  const size_t maxLen = USERSTREAMSIZE - 5 - 1;

  if (iStream == usAll || iStream == usPrefs || iStream == us1) {
    // Get all Boolean preferences
    sx_Buffer = prefs->PWSprefs::GetAllBoolPrefs();
    len = (int)std::min(sx_Buffer.length(), maxLen);
    SecureZeroMemory(szUserStream1, sizeof(szUserStream1));
    swprintf_s(szUserStream1, USERSTREAMSIZE,
               L"US01 %s", sx_Buffer.substr(0, len).c_str());
  }

  if (iStream == usAll || iStream == usPrefs || iStream == us2) {
    // Get all Integer preferences
    sx_Buffer = prefs->PWSprefs::GetAllIntPrefs();
    len = (int)std::min(sx_Buffer.length(), maxLen);
    SecureZeroMemory(szUserStream2, sizeof(szUserStream1));
    swprintf_s(szUserStream2, USERSTREAMSIZE,
               L"US02 %s", sx_Buffer.substr(0, len).c_str());
  }

  if (iStream == usAll || iStream == usPrefs || iStream == us3) {
    // Get SOME String preferences (do not include user data)
    sx_Buffer = prefs->PWSprefs::GetAllStringPrefs();
    len = (int)std::min(sx_Buffer.length(), maxLen);
    SecureZeroMemory(szUserStream3, sizeof(szUserStream1));
    swprintf_s(szUserStream3, USERSTREAMSIZE,
               L"US03 %s", sx_Buffer.substr(0, len).c_str());
  }
}

static LONG TakeMiniDump(struct _EXCEPTION_POINTERS *pExInfo, const int itype,
                         st_invp *pinvp)
{
  // Use standard functions as much as possible
  // Remove all handlers - but don't free the library just yet
  RemoveFaultHandler(false);

  // Won't do anything with this - as long in dump if needed
  UNREFERENCED_PARAMETER(pinvp);

  wchar_t sz_TempName[_MAX_PATH + 1];

  wchar_t sz_Drive[_MAX_DRIVE], sz_Dir[_MAX_DIR], sz_FName[_MAX_FNAME];
  wchar_t sz_TempPath[MSGSIZE];
  DWORD dwBufSize(MSGSIZE);

  // Get the temp path
  SecureZeroMemory(sz_TempPath, sizeof(sz_TempPath));
  DWORD dwrc = GetTempPathW(dwBufSize, sz_TempPath);
  if (dwrc == 0 || dwrc > dwBufSize)
    goto exit;

  // Create a temporary file
  // Shouldn't really use system calls in a signal handler!
  struct tm xt;
  struct __timeb64 timebuffer;
  _ftime_s(&timebuffer);
  localtime_s(&xt, &(timebuffer.time));

  _wsplitpath_s(sz_TempPath, sz_Drive, _MAX_DRIVE, sz_Dir, _MAX_DIR,
                NULL, 0, NULL, 0);
  swprintf_s(sz_FName, _MAX_FNAME,
             L"PWS_Minidump_%04d%02d%02d_%02d%02d%02d%03d",
             xt.tm_year + 1900, xt.tm_mon + 1, xt.tm_mday,
             xt.tm_hour, xt.tm_min, xt.tm_sec, timebuffer.millitm);

  SecureZeroMemory(sz_TempName, sizeof(sz_TempName));
  _wmakepath_s(sz_TempName, MAX_PATH + 1, sz_Drive, sz_Dir, sz_FName, L"dmp");

  SecureZeroMemory(szUserStream0, sizeof(szUserStream0));
  swprintf_s(szUserStream0, USERSTREAMSIZE,
             L"US00 %d %d %d %d %s %08x %d %d %d %d", IVERSION,
             iMajor, iMinor, iBuild, wcRevision, dwTimeStamp,
             st_bOpen ? 1 : 0,
             st_bOpen ? (st_bRW ? 1 : 0) : -1,
             st_cfgloc, itype);

  wchar_t sz_errormsg[MSGSIZEx4]; // not a good place for malloc, as the heap
                                  // may be corrupt...should be big enough!
  SecureZeroMemory(sz_errormsg, sizeof(sz_errormsg));

  wcscpy_s(sz_errormsg, MSGSIZEx4, wcMsg1);       // Max size 'MSGSIZE'
  wcscat_s(sz_errormsg, MSGSIZEx4, sz_FName);     // Max size '_MAX_FNAME'
  wcscat_s(sz_errormsg, MSGSIZEx4, wcMsg2);       // Max size 'MSGSIZE'
  wcscat_s(sz_errormsg, MSGSIZEx4, sz_TempPath);  // Max size '_MAX_PATH'
  wcscat_s(sz_errormsg, MSGSIZEx4, wcMsg3);       // Max size 'MSGSIZE'

  wchar_t sz_Caption[MAX_PATH];
  SecureZeroMemory(sz_Caption, sizeof(sz_Caption));
  swprintf_s(sz_Caption, MAX_PATH, wcCaption, iMajor, iMinor, iBuild, wcRevision);

  HANDLE hFile = CreateFile(sz_TempName, GENERIC_READ | GENERIC_WRITE,
                            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION excpInfo;
    excpInfo.ClientPointers = FALSE;
    excpInfo.ExceptionPointers = pExInfo;
    excpInfo.ThreadId = GetCurrentThreadId();

    MINIDUMP_USER_STREAM UserStreams[5];

    UserStreams[0].Type = LastReservedStream + 1;
    UserStreams[0].Buffer = (void *)&szUserStream0[0];
    UserStreams[0].BufferSize = (ULONG)((wcslen(szUserStream0) + 1) * sizeof(wchar_t));
    UserStreams[1].Type = LastReservedStream + 2;
    UserStreams[1].Buffer = (void *)&szUserStream1[0];
    UserStreams[1].BufferSize = (ULONG)((wcslen(szUserStream1) + 1) * sizeof(wchar_t));
    UserStreams[2].Type = LastReservedStream + 3;
    UserStreams[2].Buffer = (void *)&szUserStream2[0];
    UserStreams[2].BufferSize = (ULONG)((wcslen(szUserStream2) + 1) * sizeof(wchar_t));
    UserStreams[3].Type = LastReservedStream + 4;
    UserStreams[3].Buffer = (void *)&szUserStream3[0];
    UserStreams[3].BufferSize = (ULONG)((wcslen(szUserStream3) + 1) * sizeof(wchar_t));

    std::wstring us4 = PWSLog::GetLog()->DumpLog();

    if (us4.length() > 0) {
      UserStreams[4].Type = LastReservedStream + 5;
      UserStreams[4].Buffer = (void *)us4.c_str();
      UserStreams[4].BufferSize = (ULONG)(us4.length() * sizeof(wchar_t));
    }

    MINIDUMP_USER_STREAM_INFORMATION musi;
    musi.UserStreamCount = (us4.length() > 0) ? 5 : 4;
    musi.UserStreamArray = UserStreams;

    pfcnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                          MiniDumpNormal, (pExInfo != NULL) ? &excpInfo : NULL,
                          &musi, NULL);

    CloseHandle(hFile);

    // Issue error message
    // Use standard MessageBox - NOT CGeneralMsgBox.
    int irc = MessageBox(GetForegroundWindow(), sz_errormsg, sz_Caption,
                         MB_YESNO | MB_APPLMODAL | MB_ICONERROR);

    // If user doesn't want to send us the minidump - delete it.
    if (irc == IDNO)
      _wremove(sz_TempName);
  } // valid file handle

 exit:
  // Now we can free the library
  pws_os::FreeLibrary(hDbgHelp);
  hDbgHelp = NULL;
  pfcnMiniDumpWriteDump = NULL;

  return EXCEPTION_CONTINUE_SEARCH;
}

void RemoveFaultHandler(bool bFreeLibrary)
{
  // If we don't have the function pointer - we never set anything up
  if (pfcnMiniDumpWriteDump == NULL)
    return;

  // Free library if called during application normal termination
  if (bFreeLibrary && hDbgHelp != NULL) {
    pws_os::FreeLibrary(hDbgHelp);
    hDbgHelp = NULL;
  }

  // Remove our Windows Exception Filter
  SetUnhandledExceptionFilter(NULL);

  // Put back previous handlers
  if (old_terminate_handler != NULL) {
    set_terminate(old_terminate_handler);
    old_terminate_handler = NULL;
  }

  if (old_unexpected_handler != NULL) {
    set_unexpected(old_unexpected_handler);
    old_unexpected_handler = NULL;
  }

  if (old_purecall_handler != NULL) {
    _set_purecall_handler(old_purecall_handler);
    old_purecall_handler = NULL;
  }

  _set_new_mode(old_new_mode);
  if (old_new_handler != NULL) {
    _set_new_handler(old_new_handler);
    old_new_handler = NULL;
  }

  if (old_bad_parameter_handler != NULL) {
    _set_invalid_parameter_handler(old_bad_parameter_handler);
    old_bad_parameter_handler = NULL;
  }

  // Reset signal processing to default actions
  signal(SIGABRT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGILL, SIG_DFL);
}
#endif
