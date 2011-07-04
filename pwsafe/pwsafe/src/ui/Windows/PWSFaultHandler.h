/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// Used for Windows minidumps
enum UserStream {usAll = -2, usPrefs = -1, us0 = 0, us1 = 1, us2 = 2, us3 = 3};

#ifndef _DEBUG

#include "stdafx.h"
#include "core\PWSprefs.h"

#include <dbghelp.h>

// User stream 0 version
#define IVERSION 1

// Function call definitions
void InstallFaultHandler(const int major, const int minor, const int build,
                         const wchar_t *revision, const DWORD timestamp);
void LocalizeFaultHandler(HINSTANCE inst);

LONG TakeMiniDump(struct _EXCEPTION_POINTERS *ExInfo, const int type,
                  struct st_invp *pinvp = NULL);

LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *ExInfo);
void RemoveFaultHandler(bool bFreeLibrary = true);
void PopulateMinidumpUserStreams(PWSprefs *prefs, bool bOpen, bool bRW, UserStream iStream);

static void __cdecl terminate_dumphandler();
static void __cdecl unexpected_dumphandler();
static void __cdecl purecall_dumphandler();
static int  __cdecl new_dumphandler(size_t);
static void __cdecl bad_parameter_dumphandler(const wchar_t* expression,
                const wchar_t* function, const wchar_t* file,
                unsigned int line, uintptr_t pReserved);
static void signal_dumphandler(int);

// Exception types
enum {WIN32_STRUCTURED_EXCEPTION, TERMINATE_CALL, UNEXPECTED_CALL,
      NEW_OPERATOR_ERROR, PURE_CALL_ERROR, INVALID_PARAMETER_ERROR,
      SIGNAL_ABORT, SIGNAL_ILLEGAL_INST_FAULT, SIGNAL_TERMINATION,
      END_FAULTS};

typedef BOOL (WINAPI *PMDWD) (HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
                              PMINIDUMP_EXCEPTION_INFORMATION,
                              PMINIDUMP_USER_STREAM_INFORMATION,
                              PMINIDUMP_CALLBACK_INFORMATION);

#endif
