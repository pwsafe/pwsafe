/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
  wstring retval;
  assert(val != NULL);
  int len = strlen(val);
  int wsize;
  const char *p = val;
  wchar_t wvalue;
  do {
    wsize = mbtowc(&wvalue, p, MB_CUR_MAX);
    if (wsize <= 0)
      break;
    retval += wvalue;
    p += wsize;
    len -= wsize;
  } while (len != 1);
  return retval;
}
