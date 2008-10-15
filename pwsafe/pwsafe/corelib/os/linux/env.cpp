/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of env.h
 */

#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>


#include "../env.h"

#ifdef UNICODE
static stringT towc(const char *val)
{
  stringT retval;
  assert(val != NULL);
  int len = std::strlen(val);
  int wsize = std::mbtowc(NULL, val, len);
  ASSERT(wsize > 0);
  wchar_t *wvalue = new wchar_t[wsize+1];
  wsize = std::mbtowc(wvalue, val, len);
  wvalue[wsize] = 0;
  retval = wvalue;
  delete[] wvalue;
  return retval;
}
#endif

stringT pws_os::getenv(const char *env, bool is_path)
{
  assert(env != NULL);
  stringT retval;
  char *value = std::getenv(env);
  if (value != NULL) {
#ifdef UNICODE
    retval = towc(value);
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

stringT pws_os::getusername()
{
  stringT retval;
  const char *user = getlogin();
  if (user == NULL)
    user = "?";
#ifdef UNICODE
  retval = towc(user);
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
  retval = towc(name);
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
