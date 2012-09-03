/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWYubi.cpp
* 
*/

#include "PWYubi.h"
#include <ykcore.h>

bool PWYubi::isInited = false;
pthread_mutex_t PWYubi::s_mutex = PTHREAD_MUTEX_INITIALIZER;

PWYubi::PWYubi()
{
  pthread_mutex_lock(&s_mutex);
  if (!isInited) {
    isInited = yk_init() != 0;
  }
  pthread_mutex_unlock(&s_mutex);
}

bool PWYubi::IsYubiInserted() const
{
  bool retval = false;
  pthread_mutex_lock(&s_mutex);
  // if yk isn't init'ed, don't bother
  if (isInited) {
    YK_KEY *ykey = yk_open_first_key();
    if (ykey != NULL) {
      yk_close_key(ykey);
      retval = true;
    }
  }
  pthread_mutex_unlock(&s_mutex);
  return retval;
}

