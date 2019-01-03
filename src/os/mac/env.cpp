/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of env.h
 */

#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>

#include "../env.h"
#include "../utf8conv.h" // for pws_os::towc

/* http://www.mail-archive.com/bug-gnulib@gnu.org/msg13682.html */
#if not defined(HOST_NAME_MAX)
#include <sys/param.h>
#if defined(MAXHOSTNAMELEN)
enum { HOST_NAME_MAX = MAXHOSTNAMELEN };
#else
#include <limits.h>
#if defined (_POSIX_HOST_NAME_MAX)
enum { HOST_NAME_MAX = _POSIX_HOST_NAME_MAX+1 } ;
#else
enum { HOST_NAME_MAX = 256 };
#endif
#endif
#endif

stringT pws_os::getenv(const char *env, bool is_path)
{
  assert(env != NULL);
  stringT retval;
  char *value = std::getenv(env);
  if (value != NULL) {
#ifdef UNICODE
    retval = pws_os::towc(value);
#else
    retval = value;
#endif
    if (is_path) {
      // make sure path has trailing '\'
      if (retval[retval.length()-1] != charT('/'))
        retval += _S("/");
    } // is_path
  } // value != NULL
  return retval;
}

void pws_os::setenv(const char *name, const char *value)
{
  ASSERT(name != NULL && value != NULL);
  ::setenv(name, value, 1); // 1 => overwrite the variable with this value, if its already set
}

stringT pws_os::getusername()
{
  stringT retval;
  const char *user = getlogin();
  if (user == NULL)
    user = "?";
#ifdef UNICODE
  retval = pws_os::towc(user);
#else
  retval = user;
#endif
  return retval;
}

stringT pws_os::gethostname()
{
  stringT retval;
  char name[HOST_NAME_MAX];
  if (::gethostname(name, HOST_NAME_MAX) != 0) {
    assert(0);
    name[0] = '?'; name[1] = '\0';
  }
#ifdef UNICODE
  retval = pws_os::towc(name);
#else
  retval = name;
#endif
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
  os << getpid();

  return os.str();
}
