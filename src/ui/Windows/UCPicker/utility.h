
#pragma once

#include <string>

bool splitpath(const std::wstring &path,
               std::wstring &drive, std::wstring &dir,
               std::wstring &file, std::wstring &ext);

// Debug only
std::wstring TrimRight(std::wstring &s, const wchar_t *set = NULL);
std::wstring ConvertToDateTimeString(const time_t &t);
void GetTimeStamp(std::wstring &sTimeStamp, const bool bShort = false);
void Trace(LPCTSTR lpszFormat, ...);

int GetStringBufSize(const TCHAR *fmt, va_list args);
