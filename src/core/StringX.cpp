/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <ctype.h>
#include <string.h>
#include <cstdarg>
#include "StringX.h"
#include "Util.h"

#include "PwsPlatform.h"
#include "os/pws_tchar.h"

#if !defined(_WIN32) || defined(__WX__)
#include "core_st.h"
#endif

// A few convenience functions for StringX & stringT

template<class T> int CompareNoCase(const T &s1, const T &s2)
{
  // case insensitive string comparison
  return _tcsicmp(s1.c_str(), s2.c_str());
}

template<class T> void ToLower(T &s)
{
  for (typename T::iterator iter = s.begin(); iter != s.end(); iter++)
    *iter = TCHAR(_totlower(*iter));
}

template<class T> void ToUpper(T &s)
{
  for (typename T::iterator iter = s.begin(); iter != s.end(); iter++)
    *iter = TCHAR(_totupper(*iter));
}

template<class T> T &Trim(T &s, const TCHAR *set)
{
  const TCHAR *ws = _T(" \t\r\n");
  const TCHAR *tset = (set == NULL) ? ws : set;

  typename T::size_type b = s.find_first_not_of(tset);
  if (b == T::npos) {
    s.clear();
  } else {
    typename T::size_type e = s.find_last_not_of(tset);
    T t(s.begin() + b, s.end() - (s.length() - e) + 1);
    s = t;
  }
  return s;
}

template<class T> T &TrimRight(T &s, const TCHAR *set)
{
  const TCHAR *ws = _T(" \t\r\n");
  const TCHAR *tset = (set == NULL) ? ws : set;

  typename T::size_type e = s.find_last_not_of(tset);
  if (e == T::npos) {
    s.clear();
  } else {
    T t(s.begin(), s.end() - (s.length() - e) + 1);
    s = t;
  }
  return s;
}

template<class T> T &TrimLeft(T &s, const TCHAR *set)
{
  const TCHAR *ws = _T(" \t\r\n");
  const TCHAR *tset = (set == NULL) ? ws : set;

  typename T::size_type b = s.find_first_not_of(tset);
  if (b == T::npos) {
    s.clear();
  } else {
    T t(s.begin() + b, s.end());
    s = t;
  }
  return s;
}

template<class T> void EmptyIfOnlyWhiteSpace(T &s)
{
  const TCHAR *ws = _T(" \t\r\n");
  typename T::size_type b = s.find_first_not_of(ws);
  if (b == T::npos)
    s.clear();
}

template<class T> int Replace(T &s, TCHAR from, TCHAR to)
{
  int retval = 0;
  T r;
  r.reserve(s.length());
  for (typename T::iterator iter = s.begin(); iter != s.end(); iter++) {
    if (*iter == from) {
      r.append(1, to);
      retval++;
    } else
      r.append(1, *iter);
  }
  s = r;
  return retval;
}

template<class T> int Replace(T &s, const T &from, const T &to)
{
  int retval = 0;
  T r;
  typename T::size_type i = 0;
  do {
   typename T::size_type j = s.find(from, i);
    r.append(s, i, j - i);
    if (j != StringX::npos) {
      r.append(to);
      retval++;
      i = j + from.length();
    } else
      i = j;
  } while (i != StringX::npos);
  s = r;
  return retval;
}

template<class T> int Remove(T &s, TCHAR c)
{
  int retval = 0;
  T t;
  for (typename T::iterator iter = s.begin(); iter != s.end(); iter++)
    if (*iter != c)
      t += *iter;
    else
      retval++;
  if (retval != 0)
    s = t;
  return retval;
}

template<class T> void LoadAString(T &s, int id)
{
#if defined(_WIN32) && !defined(__WX__)
  CString cs;
  cs.LoadString(id);
  s = cs;
#else
  s = core_st[id];
#endif
}

#if defined(__GNUC__)  && (defined(UNICODE) || defined(_UNICODE))
#define CONVERT_GLIBC_FORMATSPECS
#endif

/*
 * In UNICODE builds (more specifically, the wide versions of printf functions family) Microsoft's libc
 * and glibc interpret the %s format specifier differently. Microsoft interprets the corresponding
 * string argument to be wide-char, while glibc interprets it as single-char.  glibc requires either %S
 * or %ls to interpret the corresponding string argument as wide-char.
 * 
 * Since we *always* use single or wide char functions/strings depending on UNICODE, we convert
 * all %s to %S while building with GNU/glibc and UNICODE defined using the functions below
 * 
 * http://msdn.microsoft.com/en-us/library/hf4y5e3w(v=VS.100).aspx
 * http://msdn.microsoft.com/en-us/library/tcxf1dw6(v=VS.100).aspx
 * http://www.gnu.org/software/libc/manual/html_node/Other-Output-Conversions.html#Other-Output-Conversions
 * 
 * Note that this is not a Linux vs. Windows difference.  The same issue exists if we build this on WIN32
 * with GNU with UNICODE defined.  Also, we don't do a similar conversion for ANSI builds (i.e. %S => %s)
 * since all our format specs are always %s anyway.
 */
 
#ifdef CONVERT_GLIBC_FORMATSPECS
template <typename T>
inline void ConvertFormatSpecs(T& specs)
{
  for(typename T::size_type pos = 0; (pos = specs.find(L"%s", pos)) != T::npos; pos += 2) {
    specs[pos+1] = L'S';
  }
}

template <typename T>
inline T ConvertFormatSpecs(const wchar_t* fmt)
{
  T specs(fmt);
  ConvertFormatSpecs(specs);
  return specs;
}

#endif

template<class T> void Format(T &s, const TCHAR *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

#ifdef CONVERT_GLIBC_FORMATSPECS
  T fmt_s(ConvertFormatSpecs<T>(fmt));
  const TCHAR* newfmt = fmt_s.c_str();
#else
  const TCHAR* newfmt = fmt;
#endif

  int len = GetStringBufSize(newfmt, args);
  va_end(args);//after using args we should reset list
  va_start(args, fmt);

  TCHAR *buffer = new TCHAR[len];

  _vstprintf_s(buffer, len, newfmt, args);
  s = buffer;
  delete[] buffer;
  va_end(args);
}

template<class T> void Format(T &s, int fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  T fmt_str;
  LoadAString(fmt_str, fmt);

#ifdef CONVERT_GLIBC_FORMATSPECS
  ConvertFormatSpecs(fmt_str);
#endif

  int len = GetStringBufSize(fmt_str.c_str(), args);
  va_end(args);//after using args we should reset list
  va_start(args, fmt);

  TCHAR *buffer = new TCHAR[len];

  _vstprintf_s(buffer, len, fmt_str.c_str(), args);
  s = buffer;
  delete[] buffer;
  va_end(args);
}


// instantiations for StringX & stringT
template int CompareNoCase(const StringX &s1, const StringX &s2);
template int CompareNoCase(const stringT &s1, const stringT &s2);
template void ToLower(StringX &s);
template void ToLower(stringT &s);
template void ToUpper(StringX &s);
template void ToUpper(stringT &s);
template StringX &Trim(StringX &s, const TCHAR *set);
template stringT &Trim(stringT &s, const TCHAR *set);
template StringX &TrimRight(StringX &s, const TCHAR *set);
template stringT &TrimRight(stringT &s, const TCHAR *set);
template StringX &TrimLeft(StringX &s, const TCHAR *set);
template stringT &TrimLeft(stringT &s, const TCHAR *set);
template void EmptyIfOnlyWhiteSpace(StringX &s);
template void EmptyIfOnlyWhiteSpace(stringT &s);
template int Replace(StringX &s, TCHAR from, TCHAR to);
template int Replace(stringT &s, TCHAR from, TCHAR to);
template int Replace(StringX &s, const StringX &from, const StringX &to);
template int Replace(stringT &s, const stringT &from, const stringT &to);
template int Remove(StringX &s, TCHAR c);
template int Remove(stringT &s, TCHAR c);
template void LoadAString(stringT &s, int id);
template void LoadAString(StringX &s, int id);
template void Format(stringT &s, int fmt, ...);
template void Format(StringX &s, int fmt, ...);
template void Format(stringT &s, const TCHAR *fmt, ...);
template void Format(StringX &s, const TCHAR *fmt, ...);


#ifdef TEST_TRIM
int main(int argc, char *argv[])
{
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " string-to-trim" << endl;
    exit(1);
  }

  StringX s(argv[1]), sl(argv[1]), sr(argv[1]);
  Trim(s); TrimLeft(sl); TrimRight(sr);
  cout << "Trim(\"" << argv[1] << "\") = \"" << s <<"\"" << endl;
  cout << "TrimLeft(\"" << argv[1] << "\") = \"" << sl <<"\"" << endl;
  cout << "TrimRight(\"" << argv[1] << "\") = \"" << sr <<"\"" << endl;
  return 0;
}
#endif
