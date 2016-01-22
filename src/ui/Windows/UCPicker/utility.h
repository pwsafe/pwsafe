
#pragma once

#include <string>

typedef unsigned __int32 uint32_t;

bool splitpath(const std::wstring &path,
               std::wstring &drive, std::wstring &dir,
               std::wstring &file, std::wstring &ext);

// Debug only
std::wstring TrimRight(std::wstring &s, const wchar_t *set = NULL);
std::wstring ConvertToDateTimeString(const time_t &t);
void GetTimeStamp(std::wstring &sTimeStamp, const bool bShort = false);
void Trace(LPCTSTR lpszFormat, ...);
