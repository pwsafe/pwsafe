/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>

#include "../env.h"
#include "../utf8conv.h" // for pws_os::towc
#ifndef HOST_NAME_MAX
# include "netdb.h" /* for MAXHOSTNAMELEN */
# if defined(_POSIX_HOST_NAME_MAX)
#  define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
# elif defined(MAXHOSTNAMELEN)
#  define HOST_NAME_MAX MAXHOSTNAMELEN
# else
#  define HOST_NAME_MAX	255
# endif
#endif /* HOST_NAME_MAX */

stringT pws_os::getenv(const char *env, bool is_path)
{
  assert(env != nullptr);
  stringT retval;
  char *value = std::getenv(env);
  if (value != nullptr) {
    retval = pws_os::towc(value);
    if (is_path) {
      // make sure path has trailing '\'
      if (retval[retval.length()-1] != charT('/'))
        retval += _S("/");
    } // is_path
  } // value != nullptr
  return retval;
}

void pws_os::setenv(const char *name, const char *value)
{
  ASSERT(name != nullptr && value != nullptr);
  ::setenv(name, value, 1); // Shouldn't this be under std:: ?
}

stringT pws_os::getusername()
{
  stringT retval;
  struct passwd *pw_s = ::getpwuid(::getuid());
  const char *user = (pw_s != nullptr) ? pw_s->pw_name : "?";
  retval = pws_os::towc(user);
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
  retval = pws_os::towc(name);
  return retval;
}

stringT pws_os::getprocessid()
{
  std::wostringstream os;
  os.width(8);
  os.fill(charT('0'));
  os << getpid();

  return os.str();
}

/**
 * Checks whether a process with the given target process id (pid) exists.
 * 
 * @param pid the id of the process that should be checked for existance
 * @return ProcessCheckResult::FOUND if a process with the given pid exists, 
 *         ProcessCheckResult::NOT_FOUND if it doesn't exists and 
 *         ProcessCheckResult::ERROR_ on error
 * @see http://man7.org/linux/man-pages/man2/kill.2.html
 */
pws_os::ProcessCheckResult pws_os::processExists(int pid)
{
  const int NO_SIGNAL = 0;
  const int NO_ERROR  = 0;
  const int INIT_PID  = 1;

  if (pid > INIT_PID) {
    if (kill(pid, NO_SIGNAL) == NO_ERROR) {
      return ProcessCheckResult::FOUND;
    } else {
      if (errno == ESRCH) {
        return ProcessCheckResult::NOT_FOUND;
      }
    }
  }

  return ProcessCheckResult::ERROR_;
}
