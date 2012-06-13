/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of utf8conv.h
 */

#include "../typedefs.h"
#include "../utf8conv.h"
#include <cstdlib>
#include <clocale>
#include <cassert>
#include <cstring>
#include <string>
#include <unistd.h>

using namespace std;

class Startup {
public:
  Startup() {
    // Sice this can fail before main(), we can't rely on cerr etc.
    // being intialized. At least give the user a clue...
    char *gl = setlocale(LC_ALL, NULL);
    char *sl = setlocale(LC_ALL, "");
    if (sl == NULL) {
      // Couldn't get environment-specified locale, warn user
      // and punt to default "C"
      char wrnmess[] = "Couldn't load locale, falling back to default\n";
      write(STDERR_FILENO, wrnmess, sizeof(wrnmess)/sizeof(*wrnmess)-1);
      sl = setlocale(LC_ALL, gl);
    }
    if (sl == NULL) {
      // If we can't get the default,we're really FUBARed
      char errmess[] = "Couldn't initialize locale - bailing out\n";
      write(STDERR_FILENO, errmess, sizeof(errmess)/sizeof(*errmess)-1);
      _exit(2);
    }
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


