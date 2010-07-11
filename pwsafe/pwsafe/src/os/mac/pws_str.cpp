/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of some wide-string related functionality
 */

#include <wchar.h>
#include "pws_str.h"
#include "../utf8conv.h"

int pws_os::wctoi(const wchar_t *s)
{
  return int(wcstol(s, NULL, 10));
}

double pws_os::wctof(const wchar_t *s)
{
  return double(wcstold(s, NULL));
}

/*
 * had to define this for gcc 4.0 on osx.
 * http://trac.wildfiregames.com/ticket/414
 */
int pws_os::wcscasecmp(const wchar_t* s1, const wchar_t* s2)
{
  wchar_t wc1, wc2;
	
  if (s1 == s2) 
    return 0; 

  do { 
    wc1 = towlower(*s1++); 
    wc2 = towlower(*s2++); 
    if (wc1 == L'\0') 
      break; 
  } 
  while (wc1 == wc2); 

  return wc1 - wc2; 
}


