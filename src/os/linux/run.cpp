/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of run.h - DUMMY!
 */

#include <stdlib.h>

#include "../run.h"

StringX pws_os::getruncmd(const StringX first_part, bool &bfound)
{
	// Stub! (not sure we need more than this on Linux)
	return first_part;
}

bool pws_os::runcmd(const StringX execute_string)
{
#ifndef UNICODE
  const char *exec_str = execute_string.c_str();
#else
  size_t slen = wcstombs(NULL, execute_string.c_str(), 0) + 1;
  if (slen <= 1)
    return false;
  char *exec_str = new char[slen];
  wcstombs(exec_str, execute_string.c_str(), slen);
#endif
  int status = system(exec_str);
  if (status == -1) // e.g., fork() failed
    return false;
  if (!WIFEXITED(status)) // child didn't terminate normally
    return false;
  return (WEXITSTATUS(status) == 0);
}
