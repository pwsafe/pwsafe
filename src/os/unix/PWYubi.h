/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWYUBI_H
#define __PWYUBI_H

#include <pthread.h> // for pthread_mutex_*
#include <string>

class PWYubi {
public:
  PWYubi();
  ~PWYubi();
  bool IsYubiInserted() const;
  bool GetSerial(unsigned int &serial) const;
  bool WriteSK(const unsigned char *sk, size_t sklen);

  // This will return true iff user inserts a YubiKey in the lifetime
  // of the process.
  // We use it to show the YubiKey controls in the UI.
  static bool YubiExists() {return s_yubiDetected;}

  // request is non-blocking
  // GetResponse returns pending until done or timeout. If ERROR returned,
  // call GetErrStr for details.
  bool RequestHMacSHA1(const unsigned char *challenge, unsigned int len);
  enum RequestStatus {DONE, PENDING, TIMEOUT, ERROR};
  enum {RESPLEN=20, SHA1_MAX_BLOCK_SIZE=64};

  RequestStatus GetResponse(unsigned char resp[RESPLEN]);

  // if GetErrStr().empty(), then no error:
  const std::wstring &GetErrStr() const {return m_ykerrstr;}
private:
  void report_error() const;
  mutable bool m_isInited;
  static bool s_yubiDetected;
  static pthread_mutex_t s_mutex;
  mutable std::wstring m_ykerrstr;
  RequestStatus m_reqstat;
};
#endif /* __PWYUBI_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
