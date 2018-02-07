/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of utf8conv.h
 */

#include "../typedefs.h"
#include "../utf8conv.h"
#include <cstdlib>
#include <clocale>
#include <cassert>
#include <cstring>
#include <string>

using namespace std;

class Startup {
public:
  Startup() {
    char *sl = setlocale(LC_ALL, "");
    if (sl == NULL)
      throw "Couldn't initialize locale - bailing out";
  }
};

static Startup startup;

size_t pws_os::wcstombs(char *dst, size_t maxdstlen,
                        const wchar_t *src, size_t , bool )
{
  return ::wcstombs(dst, src, maxdstlen) + 1;
}

size_t pws_os::mbstowcs(wchar_t *dst, size_t maxdstlen,
                        const char *src, size_t , bool )
{
  return ::mbstowcs(dst, src, maxdstlen) + 1;
}

wstring pws_os::towc(const char *val)
{
  wstring retval(L"");
  assert(val != NULL);
  int len = strlen(val);
  int wsize;
  const char *p = val;
  wchar_t wvalue;
  while (len > 0) {
    wsize = mbtowc(&wvalue, p, MB_CUR_MAX);
    if (wsize <= 0)
      break;
    retval += wvalue;
    p += wsize;
    len -= wsize;
  };
  return retval;
}

#ifdef UNICODE
std::string pws_os::tomb(const stringT& val)
{
  if (!val.empty()) {
    const size_t N = std::wcstombs(NULL, val.c_str(), 0);
    assert(N > 0);
    char* szstr = new char[N+1];
    szstr[N] = 0;
    std::wcstombs(szstr, val.c_str(), N);
    std::string retval(szstr);
    delete[] szstr;
    return retval;
  } else
    return string();
}
#else
std::string pws_os::tomb(const stringT& val)
{
  return val;
}
#endif
