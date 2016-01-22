
#include "StdAfx.h"

#include "utility.h"

#include <sys/timeb.h>
#include <time.h>
#include <sstream>
#include <iomanip>

#if defined(_DEBUG)

std::wstring TrimRight(std::wstring &s, const wchar_t *set)
{
  const wchar_t *ws = L" \t\r\n";
  const wchar_t *tset = (set == NULL) ? ws : set;

  std::wstring::size_type e = s.find_last_not_of(tset);
  if (e == std::wstring::npos) {
    s.clear();
  } else {
    std::wstring t(s.begin(), s.end() - (s.length() - e) + 1);
    s = t;
  }
  return s;
}

std::wstring ConvertToDateTimeString(const time_t &t)
{
  std::wstring ret;

  if (t != 0) {
    wchar_t datetime_str[80];
    struct tm *st;
    struct tm st_s;
    errno_t err;
    err = localtime_s(&st_s, &t);  // secure version
    if (err != 0) // invalid time
      return ConvertToDateTimeString(0);

    st = &st_s; // hide difference between versions
    _tcsftime(datetime_str, sizeof(datetime_str) / sizeof(datetime_str[0]),
      _T("%Y/%m/%d %H:%M:%S"), st);
    ret = datetime_str;
  } else { // t == 0
    ret = _T("");
  }

  // remove the trailing EOL char.
  TrimRight(ret);
  return ret;
}

void GetTimeStamp(std::wstring &sTimeStamp, const bool bShort)
{
  // Gets datetime stamp in format YYYY/MM/DD HH:MM:SS.mmm
  // If bShort == true, don't add milli-seconds
  struct _timeb *ptimebuffer;
  ptimebuffer = new _timeb;
  _ftime_s(ptimebuffer);

  std::wstring cmys_now = ConvertToDateTimeString(ptimebuffer->time);

  if (bShort) {
    sTimeStamp = cmys_now.c_str();
  } else {
    std::wostringstream *p_os;
    p_os = new std::wostringstream;
    *p_os << cmys_now << wchar_t('.') << std::setw(3) << std::setfill(wchar_t('0'))
      << static_cast<unsigned int>(ptimebuffer->millitm);

    sTimeStamp = p_os->str();
    delete p_os;
  }
  delete ptimebuffer;
}

void Trace(LPCWSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);

  std::wstring sTimeStamp;
  GetTimeStamp(sTimeStamp);
  int num_required, num_written;

  num_required = _vsctprintf(lpszFormat, args) + 1;
  va_end(args);  // after using args we should reset list
  va_start(args, lpszFormat);

  wchar_t *szBuffer = new wchar_t[num_required];
  num_written = _vsntprintf_s(szBuffer, num_required, num_required - 1, lpszFormat, args);
  ASSERT(num_required == num_written + 1);
  szBuffer[num_required - 1] = L'\0';

  const size_t N = num_written + sTimeStamp.length() + 2;
  wchar_t *szDebugString = new wchar_t[N];

  wcscpy_s(szDebugString, N, sTimeStamp.c_str());
  wcscat_s(szDebugString, N, L" ");
  wcscat_s(szDebugString, N, szBuffer);

  OutputDebugString(szDebugString);
  delete[] szDebugString;
  delete[] szBuffer;
  va_end(args);
}

#else

void Trace(LPCWSTR lpszFormat, ...)
{
  UNREFERENCED_PARAMETER(lpszFormat);
}

#endif // _DEBUG

bool splitpath(const std::wstring &path,
  std::wstring &drive, std::wstring &dir,
  std::wstring &file, std::wstring &ext)
{
  wchar_t tdrv[_MAX_DRIVE];
  wchar_t tdir[_MAX_DIR];
  wchar_t tname[_MAX_FNAME];
  wchar_t text[_MAX_EXT];

  std::wmemset(tdrv, 0, sizeof(tdrv) / sizeof(wchar_t));
  std::wmemset(tdir, 0, sizeof(tdir) / sizeof(wchar_t));
  std::wmemset(tname, 0, sizeof(tname) / sizeof(wchar_t));
  std::wmemset(text, 0, sizeof(text) / sizeof(wchar_t));

  if (_wsplitpath_s(path.c_str(), tdrv, tdir, tname, text) == 0) {
    drive = tdrv;
    dir = tdir;
    file = tname;
    ext = text;
    return true;
  } else {
    return false;
  }
}
