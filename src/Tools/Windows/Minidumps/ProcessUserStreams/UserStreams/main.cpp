/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*
* Based on a similar prgram by: Oleg Starodumov (www.debuginfo.com)
*/
/// \file ProcessUserStream: main.cpp
//-----------------------------------------------------------------------------

// Process user streams from PWS minidumps

#include "stdafx.h"
#include "Copy_PWS_prefs.h"

#include <Windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <time.h>
#include "Lmcons.h"  // For UNLEN
#include <cassert>

#include <string>
#include <sstream>

#ifdef _UNICODE
  typedef std::wstring stringT;
  typedef std::wistringstream istreamT;
#else
  typedef std::string stringT;
  typedef std::istringstream istreamT;
#endif

enum {WIN32_STRUCTURED_EXCEPTION, TERMINATE_CALL, UNEXPECTED_CALL,
      NEW_OPERATOR_ERROR, PURE_CALL_ERROR, INVALID_PARAMETER_ERROR,
      SIGNAL_ABORT, SIGNAL_ILLEGAL_INST_FAULT, SIGNAL_TERMINATION,
      END_FAULTS};

wchar_t *wcType[END_FAULTS] = {
                L"WIN32_STRUCTURED_EXCEPTION",
                L"TERMINATE_CALL",
                L"UNEXPECTED_CALL",
                L"NEW_OPERATOR_ERROR",
                L"PURE_CALL_ERROR",
                L"INVALID_PARAMETER_ERROR",
                L"SIGNAL_ABORT",
                L"SIGNAL_ILLEGAL_INST_FAULT",
                L"SIGNAL_TERMINATION"};

// Function declarations
bool GetStream(PVOID pMiniDump, ULONG StreamID, PVOID &pStream, ULONG &StreamSize);
void GetUserStreams(PVOID pMiniDump);
void PrintStream0(PVOID pStream);
void PrintBoolPreferences(PVOID pStream);
void PrintIntPreferences(PVOID pStream);
void PrintStringPreferences(PVOID pStream, ULONG StreamSize);
void PrintLog(PVOID pStream, ULONG StreamSize);

FILE *pOutputFile(NULL);
FILE *pConfigFile(NULL);
FILE *pImportFile(NULL);
FILE *pLogFile(NULL);

using namespace std;

stringT getusername()
{
  TCHAR user[UNLEN + sizeof(TCHAR)];
  //  ulen INCLUDES the trailing blank
  DWORD ulen = UNLEN + sizeof(TCHAR);
  if (::GetUserName(user, &ulen) == FALSE) {
    user[0] = TCHAR('?');
    user[1] = TCHAR('\0');
    ulen = 2;
  }
  ulen--;
  stringT retval(user);
  return retval;
}

stringT gethostname()
{
  //  slen EXCLUDES the trailing blank
  TCHAR sysname[MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR)];
  DWORD slen = MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR);
  if (::GetComputerName(sysname, &slen) == FALSE) {
    sysname[0] = TCHAR('?');
    sysname[1] = TCHAR('\0');
    slen = 1;
  }
  stringT retval(sysname);
  return retval;
}

stringT ConvertToDateTimeString(const time_t &t)
{
  stringT ret;
  if (t != 0) {
    TCHAR datetime_str[80];
    struct tm st_s;
    errno_t err = localtime_s(&st_s, &t);  // secure version
    if (err != 0) // invalid time
      return ret;
    _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
                _T("%Y-%m-%dT%H:%M:%S"), &st_s);
    ret = datetime_str;
  }

  // remove the trailing EOL char.
  return ret;
}

int _tmain(int argc, TCHAR *argv[])
{
  int ierr = 0;
  // Check parameters
  if (argc < 2) {
    _tprintf(_T("Usage: ProcessUserStreams <Full path to MiniDumpFile>\n"));
    return 0;
  }

  const TCHAR *pFileName = argv[1];
  _tprintf(_T("Minidump: %s \n"), pFileName);

  TCHAR sz_Drive[_MAX_DRIVE], sz_Dir[_MAX_DIR], sz_FName[_MAX_FNAME];
  TCHAR sz_output[_MAX_PATH], sz_config[_MAX_PATH], 
        sz_Import[_MAX_PATH], sz_Log[_MAX_PATH];

  _tsplitpath_s(pFileName, sz_Drive, _MAX_DRIVE, sz_Dir, _MAX_DIR,
                sz_FName, _MAX_FNAME, NULL, 0);

  // Create output files
  _tmakepath_s(sz_output, _MAX_PATH, sz_Drive, sz_Dir, sz_FName, _T("txt"));
  _tmakepath_s(sz_config, _MAX_PATH, sz_Drive, sz_Dir, _T("pwsafe"), _T("cfg"));
  _tmakepath_s(sz_Import, _MAX_PATH, sz_Drive, sz_Dir, _T("Import"), _T("xml"));
  _tmakepath_s(sz_Log, _MAX_PATH, sz_Drive, sz_Dir, _T("Log"), _T("txt"));

  // Open them
  errno_t err;
  err = _tfopen_s(&pOutputFile, sz_output, _T("wt"));
  if (err != 0) {
    _tprintf(_T("Cannot open output file: %s \n"), sz_output);
    return 90;
  }
  err = _tfopen_s(&pConfigFile, sz_config, _T("wt"));
  if (err != 0) {
    _tprintf(_T("Cannot open Config file for output: %s \n"), sz_output);
    return 91;
  }
  err = _tfopen_s(&pImportFile, sz_Import, _T("wt"));
  if (err != 0) {
    _tprintf(_T("Cannot open Import file for output: %s \n"), sz_output);
    return 92;
  }
  err = _tfopen_s(&pLogFile, sz_Log, _T("wt"));
  if (err != 0) {
    _tprintf(_T("Cannot open Logfile for output: %s \n"), sz_output);
    return 93;
  }

  const TCHAR *IMPORT_HEADER = _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<?xml-stylesheet type=\"text/xsl\" href=\"pwsafe.xsl\"?>\n\n");
  const TCHAR *PASSWORDSAFE = _T("passwordsafe");
  const TCHAR *PREFERENCES = _T("Preferences");
  const TCHAR *CONFIG_HEADER = _T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n");
  const TCHAR *CONFIG_SETTINGS = _T("Pwsafe_Settings");
  const TCHAR *LASTUPDATED = _T("LastUpdated");

  stringT username = getusername();
  stringT hostname = gethostname();
  time_t time_now;
  time(&time_now);
  const stringT now = ConvertToDateTimeString(time_now);

  _ftprintf_s(pImportFile, _T("%s"), IMPORT_HEADER);
  _ftprintf_s(pImportFile, _T("<%s delimiter=\"z\">\n"), PASSWORDSAFE);
  _ftprintf_s(pImportFile, _T("  <%s>\n"), PREFERENCES);

  _ftprintf_s(pConfigFile, _T("%s"), CONFIG_HEADER);
  _ftprintf_s(pConfigFile, _T("<%s>\n"), CONFIG_SETTINGS);
  _ftprintf_s(pConfigFile, _T("  <%s>\n"), hostname.c_str());
  _ftprintf_s(pConfigFile, _T("    <%s>\n"), username.c_str());
  _ftprintf_s(pConfigFile, _T("      <%s>%s</%s>\n"), LASTUPDATED, now.c_str(), LASTUPDATED);
  _ftprintf_s(pConfigFile, _T("      <%s>\n"), PREFERENCES);

  // Read the user data streams and display their contents
  HANDLE hFile = NULL;
  HANDLE hMapFile = NULL;
  PVOID pViewOfFile = NULL;

  // Map the minidump into memory
  hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, 0, NULL);

  if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
    _tprintf(_T("Error: CreateFile failed. Error: %u \n"), GetLastError());
    ierr = 99;
    goto exit;
  }

  hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
  if (hMapFile == NULL) {
    _tprintf(_T("Error: CreateFileMapping failed. Error: %u \n"), GetLastError());
    ierr = 98;
    goto exit;
  }

  pViewOfFile = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
  if (pViewOfFile == NULL) {
    _tprintf(_T("Error: MapViewOfFile failed. Error: %u \n"), GetLastError());
    ierr = 97;
    goto exit;
  } else {
    // Show the contents of user data streams
    GetUserStreams(pViewOfFile);
  }

  _ftprintf_s(pImportFile, _T("  </%s>\n"), PREFERENCES);
  _ftprintf_s(pImportFile, _T("</%s>\n"), PASSWORDSAFE);

  _ftprintf_s(pConfigFile, _T("      </%s>\n"), PREFERENCES);
  _ftprintf_s(pConfigFile, _T("    </%s>\n"), username.c_str());
  _ftprintf_s(pConfigFile, _T("  </%s>\n"), hostname.c_str());
  _ftprintf_s(pConfigFile, _T("</%s>\n"), CONFIG_SETTINGS);

exit:
  // Cleanup
  if (hMapFile != NULL)
    CloseHandle(hMapFile);

  if (hFile != NULL)
    CloseHandle(hFile);

  if (pOutputFile)
    fclose(pOutputFile);

  if (pConfigFile)
    fclose(pConfigFile);

  if (pImportFile)
    fclose(pImportFile);

  if (pLogFile)
    fclose(pLogFile);

  // Complete
  return ierr;
}

void GetUserStreams(PVOID pMiniDump)
{
  // Show the contents of user data streams
  // 0 - Module information, DB open and R/W status, Config location
  // 1 - Boolean preferences
  // 2 - Integer preferences
  // 3 - String  preferences
  // 4 - Log records

  bool brc;

  // First user stream - Module information!
  ULONG StreamID = LastReservedStream + 1;

  PVOID pStream = NULL;
  ULONG StreamSize = 0;

  // Get stream
  brc = GetStream(pMiniDump, StreamID, pStream, StreamSize);

  if (brc) {
    // Process stream
    wchar_t *wcUS0 = L"US00 ";
    _ftprintf_s(pOutputFile, _T("Module information: %s \n"), (TCHAR *)pStream);
    _ftprintf_s(pOutputFile, _T("\n"));
    if (memcmp(pStream, wcUS0, sizeof(wcUS0)) == 0)
      PrintStream0(pStream);
  }
  _ftprintf_s(pOutputFile, _T("\n"));

  // Second user stream - Boolean preferences
  StreamID++;

  // Reset values
  pStream = NULL;
  StreamSize = 0;

  // Get stream
  brc = GetStream(pMiniDump, StreamID, pStream, StreamSize);

  if (brc) {
    // Process stream
    wchar_t *wcUS1 = L"US01 ";
    _ftprintf_s(pOutputFile, _T("Boolean prefs: %s \n"), (TCHAR *)pStream);
    _ftprintf_s(pOutputFile, _T("\n"));
    if (memcmp(pStream, wcUS1, sizeof(wcUS1)) == 0)
      PrintBoolPreferences(pStream);
  }
  _ftprintf_s(pOutputFile, _T("\n"));

  // Third user stream - Integer preferences
  StreamID++;

  // Reset values
  pStream = NULL;
  StreamSize = 0;

  // Get stream
  brc = GetStream(pMiniDump, StreamID, pStream, StreamSize);

  if (brc) {
    // Process stream
    wchar_t *wcUS2 = L"US02 ";
    _ftprintf_s(pOutputFile, _T("Integer prefs: %s \n"), (TCHAR *)pStream);
    _ftprintf_s(pOutputFile, _T("\n"));
    if (memcmp(pStream, wcUS2, sizeof(wcUS2)) == 0)
      PrintIntPreferences(pStream);
  }
  _ftprintf_s(pOutputFile, _T("\n"));

  // Fourth user stream - String preferences
  StreamID++;

  // Reset values
  pStream = NULL;
  StreamSize = 0;

  // Get stream
  brc = GetStream(pMiniDump, StreamID, pStream, StreamSize);

  if (brc) {
    // Process stream
    wchar_t *wcUS3 = L"US03 ";
    _ftprintf_s(pOutputFile, _T("String prefs: %s \n"), (TCHAR *)pStream);
    _ftprintf_s(pOutputFile, _T("\n"));
    if (memcmp(pStream, wcUS3, sizeof(wcUS3)) == 0)
      PrintStringPreferences(pStream, StreamSize);
  }
  _ftprintf_s(pOutputFile, _T("\n"));
  
  // Fifth user stream - Log records
  StreamID++;

  // Reset values
  pStream = NULL;
  StreamSize = 0;

  // Get stream
  brc = GetStream(pMiniDump, StreamID, pStream, StreamSize);

  if (brc) {
    // Process stream
    wchar_t *wcUS4 = L"US04 ";
    _ftprintf_s(pOutputFile, _T("Log entries:\n %s \n"), (TCHAR *)pStream);
    _ftprintf_s(pOutputFile, _T("\n"));
    if (memcmp(pStream, wcUS4, sizeof(wcUS4)) == 0)
      PrintLog(pStream, StreamSize);
  }
  _ftprintf_s(pOutputFile, _T("\n"));
}

/*
  Go get individual user stream
*/
bool GetStream(PVOID pMiniDump, ULONG StreamID, PVOID &pStream, ULONG &StreamSize)
{
  PMINIDUMP_DIRECTORY pMiniDumpDir = NULL;

  if (!MiniDumpReadDumpStream(pMiniDump, StreamID, &pMiniDumpDir,
                              &pStream, &StreamSize)) {
    DWORD ErrCode = GetLastError();
    if (ErrCode != 0) // 0 -> no such stream in the dump
      _tprintf(_T("Error: MiniDumpReadDumpStream failed. Error: %u \n"), ErrCode);
    else
      _tprintf(_T("User Stream (id %u) not found in the minidump.\n"),
               StreamID - LastReservedStream);
  } else {
    // Show the contents
    if ((pStream == 0) || (StreamSize == 0)) {
      _tprintf(_T("Invalid user stream (id %u).\n"), StreamID - LastReservedStream);
    } else if (IsBadStringPtrA((LPCSTR)pStream, StreamSize)) {
      _tprintf(_T("Invalid user stream data (id %u).\n"), StreamID - LastReservedStream);
    } else {
      return true;
    }
  }
  return false;
}

void PrintStream0(PVOID pStream)
{
  stringT sName, sRevision, sTimeStamp;
  int iVersion, iMajor, iMinor, iBuild;
  int iOpen, iMode, iCfgLoc, itype;
  time_t tTimeStamp(0);
  istreamT iss((TCHAR *)pStream);
  iss >> sName;  // Skip over header

  // Standard values: IVERSION = 1
  // Future versions will add more to this list
  /*
    IVERSION,
    iMajor, iMinor, iBuild, wcRevision, dwTimeStamp,
    DB Open (1 = yes, 0 = no)
    Mode    (1 = R/W, 0 = R-O, -1 = n/a)
    Cfgloc (as per PWSprefs::ConfigOption
    Exception type
  */

  iss >> iVersion >> iMajor >> iMinor >> iBuild >> sRevision;
  iss >> hex >> tTimeStamp;
  iss >> dec >> iOpen >> iMode >> iCfgLoc >> itype;

  sTimeStamp = ConvertToDateTimeString(tTimeStamp);

  _ftprintf_s(pOutputFile, _T("Password Safe V%d.%d.%d(%s)\n"), iMajor, iMinor, iBuild, sRevision.c_str());
  _ftprintf_s(pOutputFile, _T("Module Timestamp: %s (0x%08x)\n"), sTimeStamp.c_str(), tTimeStamp);
  _ftprintf_s(pOutputFile, _T("%s %s\n"), iOpen == 1 ? L"A database is open" : L"No database is open",
                      iMode == 1 ? L"- mode R/W" : (iMode == 0 ? L"- mode R-O" : L" "));
  _ftprintf_s(pOutputFile, _T("Config location: %s (%d)\n"), PWSprefs::stringCfgLoc[iCfgLoc], iCfgLoc);
  _ftprintf_s(pOutputFile, _T("Exception type: %s\n"), itype < END_FAULTS ? wcType[itype] : L"Unknown");
}

void PrintBoolPreferences(PVOID pStream)
{
  /*
    Boolean preferences:
      # (1-byte), PrefType (1-byte), value (1-byte: 0 = false; 1 = true)
  */
  stringT sName;
  int n, type, value;
  istreamT iss((TCHAR *)pStream);
  iss >> sName;  // Skip over header

  while(iss) {
    bool bUnknown = false;
    iss >> n >> type >> value;
    if (!iss.good())
      break;

    _ASSERT(type >= PWSprefs::ptObsolete && type <= PWSprefs::ptAll);

    if (n < PWSprefs::NumBoolPrefs) {
      sName = PWSprefs::bool_prefs[n].name;
    } else {
      bUnknown = true;
      sName = _T("<unknown preference>");
    }

    _ftprintf_s(pOutputFile, _T("%d, %s, %s, %s \n"), n, sName.c_str(),
                        PWSprefs::stringTypes[type], value == 1 ? _T("true") : _T("false"));

    if (n == PWSprefs::IsUTF8) {
      // Not used but valid and will cause issue with inport schema
      continue;
    } else
    if (!bUnknown) {
      switch (type) {
        case PWSprefs::ptDatabase:
          _ftprintf_s(pImportFile, _T("    <%s>%d</%s>\n"), sName.c_str(),
                          value, sName.c_str());
          break;
        case PWSprefs::ptApplication:
          _ftprintf_s(pConfigFile, _T("        <%s>%d</%s>\n"), sName.c_str(),
                          value, sName.c_str());
          break;
        default:
          break;
      }
    }
  }
}

void PrintIntPreferences(PVOID pStream)
{
  /*
    Integer preferences:
      # (1-byte), PrefType (1-byte), value (4-hex bytes)
  */
  stringT sName;
  int n, type, value;
  istreamT iss((TCHAR *)pStream);
  iss >> sName;  // Skip over header

  while(iss) {
    bool bUnknown = false;
    iss >> dec >> n >> type >> hex >> value;
    if (!iss.good())
      break;

    _ASSERT(type >= PWSprefs::ptObsolete && type <= PWSprefs::ptAll);

    if (n < PWSprefs::NumIntPrefs) {
      sName = PWSprefs::int_prefs[n].name;
    } else {
      bUnknown = true;
      sName = _T("<unknown preference>");
    }

    _ftprintf_s(pOutputFile, _T("%d, %s, %s, %d \n"), n, sName.c_str(),
                        PWSprefs::stringTypes[type], value);

    if (n == PWSprefs::TreeDisplayStatusAtOpen) {
       _ftprintf_s(pImportFile, _T("    <%s>%s</%s>\n"), sName.c_str(),
                          PWSprefs::stringDisplay[value], sName.c_str());
    } else
    if (!bUnknown) {
      switch (type) {
        case PWSprefs::ptDatabase:
          _ftprintf_s(pImportFile, _T("    <%s>%d</%s>\n"), sName.c_str(),
                          value, sName.c_str());
          break;
        case PWSprefs::ptApplication:
          _ftprintf_s(pConfigFile, _T("        <%s>%d</%s>\n"), sName.c_str(),
                          value, sName.c_str());
          break;
        default:
          break;
      }
    }
  }
}

void PrintStringPreferences(PVOID pStream, ULONG StreamSize)
{
  /*
    String  preferences (only a safe subset):
      # (1-byte), PrefType (1-byte), value (n-bytes)
          - first/last character of 'value' is the delimiter character
  */
  stringT sName;
  int n, type;
  TCHAR *buffer = new TCHAR[StreamSize];
  TCHAR delim[1];
  istreamT iss((TCHAR *)pStream);
  iss >> sName;  // Skip over header

  while(iss) {
    bool bEmpty = false;
    bool bUnknown = false;
    iss >> n >> type;
    if (!iss.good())
      break;

    iss.ignore(1, TCHAR(' '));    // skip over space
    if (!iss.good())
      break;

    iss.get(delim[0]);            // get string delimiter
    if (!iss.good())
      break;

    memset(buffer, 0, StreamSize * sizeof(TCHAR));
    iss.getline(buffer, StreamSize, delim[0]); // get string value
    if (iss.eof())  // do not use !iss.good() as the resulting buffer may be empty!
      break;

    if (_tcslen(buffer) == 0) {
      bEmpty = true;
      iss.setstate(0);
      _tcscat_s(buffer, StreamSize, _T("<empty>"));
    }

    iss.ignore(1, TCHAR(' '));    // skip over space
    if (!iss.good())
      break;

    _ASSERT(type >= PWSprefs::ptObsolete && type <= PWSprefs::ptAll);

    if (n < PWSprefs::NumStringPrefs) {
      sName = PWSprefs::string_prefs[n].name;
    } else {
      bUnknown = true;
      sName = _T("<unknown preference>");
    }

    _ftprintf_s(pOutputFile, _T("%d, %s, %s, %s\n"), n, sName.c_str(),
                        PWSprefs::stringTypes[type], buffer);

    if (!bEmpty && !bUnknown) {
      switch (type) {
        case PWSprefs::ptDatabase:
          _ftprintf_s(pImportFile, _T("    <%s>%s</%s>\n"), sName.c_str(),
                          buffer, sName.c_str());
          break;
        case PWSprefs::ptApplication:
          _ftprintf_s(pConfigFile, _T("        <%s>%s</%s>\n"), sName.c_str(),
                          buffer, sName.c_str());
          break;
        default:
          break;
      }
    }
  }

  delete[] buffer;
}

void PrintLog(PVOID pStream, ULONG StreamSize)
{
  /*
    Log records
  */
  stringT sName;
  int num, len;

  TCHAR *buffer = new TCHAR[StreamSize];

  istreamT iss((TCHAR *)pStream);
  iss >> sName;  // Skip over header
  
  // Get number of records
  iss >> num;
  iss.ignore(1, TCHAR(' '));    // skip over space
  if (!iss.good())
    goto exit;

  for (int i = 0; i < num; i++) {
    iss >> len;
    iss.ignore(1, TCHAR(' '));    // skip over space
    if (!iss.good())
      break;

    SecureZeroMemory(buffer, StreamSize);

    iss.read(buffer, len);
    stringT sRecord(buffer);
    _ftprintf_s(pLogFile, _T("%s\n"), sRecord.c_str());
  }

exit:
  delete [] buffer;
}
