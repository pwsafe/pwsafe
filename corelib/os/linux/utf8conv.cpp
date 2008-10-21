/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include <stdlib.h>
#include <locale.h>

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
