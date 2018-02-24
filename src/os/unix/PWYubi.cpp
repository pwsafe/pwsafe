/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

bool PWYubi::s_yubiDetected = false;
pthread_mutex_t PWYubi::s_mutex = PTHREAD_MUTEX_INITIALIZER;

PWYubi::PWYubi() : m_isInited(false), m_reqstat(ERROR)
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
    if (ykey != nullptr) {
      yk_close_key(ykey);
      retval = true;
    } else {
      // reset s.t. we'll init next time
      yk_release();
      m_isInited = false;
    }
  } else { // try again
    m_isInited = yk_init() != 0;
  }
  pthread_mutex_unlock(&s_mutex);
  if (retval)
    s_yubiDetected = true;
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
  YK_KEY *ykey = nullptr;
  pthread_mutex_lock(&s_mutex);
  // if yk isn't init'ed, don't bother
  if (m_isInited) {
    ykey = yk_open_first_key();
    if (ykey != nullptr) {
      if (!check_firmware_version(ykey)) {
        m_ykerrstr = _S("YubiKey firmware version unsupported");
        goto done;
      }
      if (!yk_get_serial(ykey, 0, 0, &serial)) {
        m_ykerrstr = _S("Failed to read serial number");
        goto done;
      }
      retval = true;
    } else { // nullptr ykey, perhaps removed?
      report_error();
    }
  }
  done:
  if (ykey != nullptr)
    yk_close_key(ykey);
  pthread_mutex_unlock(&s_mutex);
  return retval;
}

bool PWYubi::WriteSK(const unsigned char *yubi_sk_bin, size_t sklen)
{
  bool retval = false;
  YK_KEY *ykey = nullptr;
  YKP_CONFIG *cfg = ykp_alloc();
  YK_STATUS *st = ykds_alloc();
  pthread_mutex_lock(&s_mutex);
  // if yk isn't init'ed, don't bother
  if (m_isInited) {
    ykey = yk_open_first_key();
    if (ykey == nullptr)
      goto done;
    if (!yk_get_status(ykey, st) ||
        (ykp_configure_version(cfg, st), !ykp_set_tktflag_CHAL_RESP(cfg,true)) ||
        !ykp_set_cfgflag_CHAL_HMAC(cfg, true) ||
        !ykp_set_cfgflag_HMAC_LT64(cfg, true) ||
        !ykp_set_cfgflag_CHAL_BTN_TRIG(cfg, true) ||
        !ykp_set_extflag_SERIAL_API_VISIBLE(cfg, true)
        ) {
      m_ykerrstr = _S("Internal error: couldn't set configuration");
      goto done;
    }
    if (!ykp_configure_command(cfg, SLOT_CONFIG2)) { // _UPDATE2?
      m_ykerrstr = _S("Internal error: couldn't configure command");
      goto done;
    }
    // ykp_HMAC_key_from_raw() was added in version 1.15.0 of ykpers
#if ((YKPERS_VERSION_MAJOR >= 1) && (YKPERS_VERSION_MINOR >= 15))
    if (ykp_HMAC_key_from_raw(cfg, reinterpret_cast<const char *>(yubi_sk_bin))) {
      m_ykerrstr = _S("Internal error: couldn't configure key");
      goto done;
    }
#else
    ostringstream os;
    for (size_t i = 0; i < sklen; i++)
      os << setfill('0') << setw(2) << hex << int(yubi_sk_bin[i]);
    if (ykp_HMAC_key_from_hex(cfg, os.str().c_str())) {
      m_ykerrstr = _S("Internal error: couldn't configure key");
      goto done;
    }
#endif

    if (!yk_write_command(ykey,
                          ykp_core_config(cfg), ykp_command(cfg),
                          nullptr)) {
          m_ykerrstr = _S("Internal error: couldn't configure key");
          goto done;
    }
    retval = true;
  } // m_isInited
  done:
  if (ykey != nullptr)
    yk_close_key(ykey);
  pthread_mutex_unlock(&s_mutex);
  free(cfg);
  free(st);
  return retval;
}

bool PWYubi::RequestHMacSHA1(const unsigned char *challenge, unsigned int len)
{
  bool retval = false;
  m_reqstat = ERROR;
  YK_KEY *ykey = nullptr;
  pthread_mutex_lock(&s_mutex);
  // if yk isn't init'ed, don't bother
  if (m_isInited) {
    ykey = yk_open_first_key();
    if (ykey == nullptr)
      goto done;
  if (yk_write_to_key(ykey, SLOT_CHAL_HMAC2, challenge, len)) {
      m_reqstat = PENDING;
      retval = true;
    }
  }
 done:
  if (ykey != nullptr)
    yk_close_key(ykey);
  pthread_mutex_unlock(&s_mutex);
  return retval;
}

PWYubi::RequestStatus PWYubi::GetResponse(unsigned char resp[PWYubi::RESPLEN])
{
 YK_KEY *ykey = nullptr;
  pthread_mutex_lock(&s_mutex);
  // if yk isn't init'ed, don't bother
  if (m_isInited && m_reqstat == PENDING) {
    ykey = yk_open_first_key();
    if (ykey == nullptr) {
      m_reqstat = ERROR;
      goto done;
    }
    unsigned char response[64];
    unsigned int response_len = 0;
    if (yk_read_response_from_key(ykey, 2, YK_FLAG_MAYBLOCK,
                                  response, sizeof(response),
                                  20, &response_len)) {
      memcpy(resp, response, RESPLEN);
      m_reqstat = DONE;
    } else {
      if (yk_errno == YK_ETIMEOUT)
        m_reqstat = TIMEOUT;
      // It's unclear what's returned if the user hasn't
      // pressed the button. We'll leave the status untouched (PENDING)
      // if read failed but hasn't timed out, so that next time
      // it could possibly succeed.
    }
  }
 done:
  if (ykey != nullptr)
    yk_close_key(ykey);
  pthread_mutex_unlock(&s_mutex);
  return m_reqstat;
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
