/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of utf8conv.h
 */

#include <Windows.h>
#include "../typedefs.h"
#include "../utf8conv.h"

size_t pws_os::wcstombs(char *dst, size_t maxdstlen,
                        const wchar_t *src, size_t srclen)
{
  if (dst != NULL && maxdstlen != 0) // resolve ambiguity
    return WideCharToMultiByte(CP_ACP, 0,
                               src, srclen, dst, maxdstlen,
                               NULL, NULL);
  else
    return WideCharToMultiByte(CP_ACP, 0,
                               src, srclen, NULL, 0,
                               NULL, NULL);
}

size_t pws_os::mbstowcs(wchar_t *dst, size_t maxdstlen,
                        const char *src, size_t srclen)
{
  if (dst != NULL && maxdstlen != 0) // resolve ambiguity
    return MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
                               src, srclen,
                               dst, maxdstlen);
  else
    return MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
                               src, srclen,
                               NULL, 0);
}
