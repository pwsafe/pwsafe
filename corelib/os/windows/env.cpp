/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of env.h
 */

#include <sstream>
#include <afx.h>
#include <Windows.h> // for GetCurrentProcessId()
#include <LMCONS.H> // for UNLEN definition
#include "../env.h"

stringT pws_os::getenv(const char *env, bool is_path)
{
  ASSERT(env != NULL);
  stringT retval;
#if _MSC_VER < 1400
  retval = getenv(env);
#else
  char* value;
  size_t requiredSize;
  getenv_s(&requiredSize, NULL, 0, env);
  if (requiredSize > 0) {
    value = new char[requiredSize];
    ASSERT(value);
    if (value != NULL) {
      getenv_s(&requiredSize, value, requiredSize, env);
#ifdef UNICODE
      int wsize;
      wchar_t wvalue;
      char *p = value;
      do {
        wsize = mbtowc(&wvalue, p, MB_CUR_MAX);
        if (wsize <= 0)
          break;
        retval += wvalue;
        p += wsize;
        requiredSize -= wsize;
      } while (requiredSize != 1);
#else
      retval = value;
#endif
      delete[] value;
      if (is_path) {
        // make sure path has trailing '\'
        if (retval[retval.length()-1] != charT('\\'))
          retval += _T("\\");
      }
    }
  }
#endif // _MSC_VER < 1400
  return retval;
}

stringT pws_os::getusername()
{
  TCHAR user[UNLEN + sizeof(TCHAR)];
  //  ulen INCLUDES the trailing blank
  DWORD ulen = UNLEN + sizeof(TCHAR);
  if (::GetUserName(user, &ulen)== FALSE) {
    user[0] = TCHAR('?');
    user[1] = TCHAR('\0');
    ulen = 2;
  }
  ulen--;
  stringT retval(user);
  return retval;
}

stringT pws_os::gethostname()
{
  //  slen EXCLUDES the trailing blank
  TCHAR sysname[MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR)];
  DWORD slen = MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR);
  if (::GetComputerName(sysname, &slen) == FALSE) {
    sysname[0] = TCHAR('?');
    sysname[1] = TCHAR('\0');
    slen = 1;
  }
  stringT retval(sysname);
  return retval;
}

stringT pws_os::getprocessid()
{
#ifdef UNICODE
  std::wostringstream os;
#else
  std::ostringstream os;
#endif
  os.width(8);
  os.fill(charT('0'));
  os << GetCurrentProcessId();

  return os.str();
}
