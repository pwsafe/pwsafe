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
#include "../debug.h"
#include <ykcore.h>
#include <ykpers.h>

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

void PWYubi::report_error()
{
  if (ykp_errno)
    pws_os::Trace(_S("Yubikey personalization error: %s\n"),
                  ykp_strerror(ykp_errno));
  if (yk_errno) {
    if (yk_errno == YK_EUSBERR) {
      pws_os::Trace(_S("USB error: %s\n"),
                    yk_usb_strerror());
    } else {
      pws_os::Trace(_S("Yubikey core error: %s\n"),
                    yk_strerror(yk_errno));
    }
  }
}
