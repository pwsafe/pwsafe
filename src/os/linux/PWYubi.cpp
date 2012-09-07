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
#include "../utf8conv.h"

#include <ykcore.h>
#include <ykpers.h>
#include <string>

pthread_mutex_t PWYubi::s_mutex = PTHREAD_MUTEX_INITIALIZER;

PWYubi::PWYubi() : m_isInited(false)
{
  pthread_mutex_lock(&s_mutex);
  m_isInited = yk_init() != 0;
  pthread_mutex_unlock(&s_mutex);
}

PWYubi::~PWYubi()
{
  pthread_mutex_lock(&s_mutex);
  if (m_isInited)
    yk_release();
  pthread_mutex_unlock(&s_mutex);
}

bool PWYubi::IsYubiInserted() const
{
  bool retval = false;
  pthread_mutex_lock(&s_mutex);
  if (m_isInited) {
    YK_KEY *ykey = yk_open_first_key();
    if (ykey != NULL) {
      yk_close_key(ykey);
      retval = true;
    } else {
      report_error(); // debug only
      // reset s.t. we'll init next time
      yk_release();
      m_isInited = false;
    }
  } else { // try again
    m_isInited = yk_init() != 0;
  }
  pthread_mutex_unlock(&s_mutex);
  return retval;
}

// Following not a member function as we don't want to expose
// YK_KEY in the interface (header file).

static bool check_firmware_version(YK_KEY *yk)
{
  YK_STATUS *st = ykds_alloc();
  bool retval = false;

  if (yk_get_status(yk, st) && 
      (ykds_version_major(st) > 2 ||
       (ykds_version_major(st) == 2
        && ykds_version_minor(st) >= 2))) {
    retval = true;
  }
  free(st);
  return retval;
}


bool PWYubi::GetSerial(unsigned int &serial) const
{
  bool retval = false;
  YK_KEY *ykey = NULL;
  pthread_mutex_lock(&s_mutex);
  // if yk isn't init'ed, don't bother
  if (m_isInited) {
    ykey = yk_open_first_key();
    if (ykey != NULL) {
      if (!check_firmware_version(ykey)) {
        m_ykerrstr = _S("YubiKey firmware version unsupported");
        goto done;
      }
      if (!yk_get_serial(ykey, 0, 0, &serial)) {
        m_ykerrstr = _S("Failed to read serial number");
        goto done;
      }
      retval = true;
    } else { // NULL ykey, perhaps removed?
      report_error();
    }
  }
  done:
  if (ykey != NULL)
    yk_close_key(ykey);
    pthread_mutex_unlock(&s_mutex);
    return retval;
}


void PWYubi::report_error() const
{
  std::string yk_errstr;
  if (ykp_errno) {
    pws_os::Trace(_S("Yubikey personalization error(%d)\n"), ykp_errno);
    yk_errstr = ykp_strerror(ykp_errno);
  }
  if (yk_errno) {
    if (yk_errno == YK_EUSBERR) {
      pws_os::Trace(_S("USB error(%d)\n"), yk_errno);
      yk_errstr += yk_usb_strerror();
    } else {
      pws_os::Trace(_S("Yubikey core error(%d)\n"), yk_errno);
      yk_errstr += yk_strerror(yk_errno);
    }
  }
  if (yk_errstr.empty())
    m_ykerrstr = L"";
  else
    m_ykerrstr = pws_os::towc(yk_errstr.c_str());
}
