/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <iomanip>
#include <sstream>

#include "YubiMixin.h"
#include "core/Util.h" // for trashMemory

bool CYubiMixin::s_yubiDetected = false;

CYubiMixin::CYubiMixin() : m_yubiPollDisable(false), m_pending(false)
{}

void CYubiMixin::YubiPoll()
{
  // If an operation is pending, check if it has completed
  if (m_pending) {
    yubiCheckCompleted();
  } else {
    // No HMAC operation is pending - check if one and only one key is present
    bool inserted = IsYubiInserted();
    // call relevant callback if something's changed
    if (inserted != m_present) {
      m_present = inserted;
      if (m_present) {
        SetYubiExists(); // proof that user has a yubikey!
        yubiInserted();
      } else
        yubiRemoved();
    }
  }
}

bool CYubiMixin::IsYubiInserted() const
{
  if (m_pending)
    return true; // can't check in the middle of a request
  else {
    CSingleLock singeLock(&m_mutex);
    singeLock.Lock();
    return (m_yk.enumPorts() == 1);
  }
}

void CYubiMixin::yubiRequestHMACSha1(const CSecString &challenge)
{
  if (m_pending) {
    // no-op if a request's already in the air
  } else {
    CSingleLock singeLock(&m_mutex);
    singeLock.Lock();
    // open key
    // if zero or >1 key, we'll fail
    if (m_yk.openKey() != YKLIB_OK) {
      return;
    }

    // Prepare the HMAC-SHA1 challenge here

    BYTE chalBuf[SHA1_MAX_BLOCK_SIZE];
    BYTE chalLength = BYTE(challenge.GetLength() * sizeof(wchar_t));
    memset(chalBuf, 0, SHA1_MAX_BLOCK_SIZE);
    if (chalLength > SHA1_MAX_BLOCK_SIZE)
      chalLength = SHA1_MAX_BLOCK_SIZE;

    memcpy(chalBuf, challenge, chalLength);

    // Initiate HMAC-SHA1 operation now

    if (m_yk.writeChallengeBegin(YKLIB_SECOND_SLOT, YKLIB_CHAL_HMAC,
                                 chalBuf, chalLength) == YKLIB_OK) {
      m_pending = true;
      yubiShowChallengeSent(); // request's in the air, setup GUI to wait for reply
    } else {
      pws_os::Trace(L"m_yk.writeChallengeBegin() failed");
    }
    trashMemory(chalBuf, chalLength);
  }
}

void CYubiMixin::yubiCheckCompleted()
{
  // We now wait for a response with the HMAC-SHA1 digest
  BYTE respBuf[SHA1_DIGEST_SIZE];
  memset(respBuf, 0, sizeof(respBuf));
  unsigned short timer;
  CSingleLock singeLock(&m_mutex);
  singeLock.Lock();
  YKLIB_RC rc = m_yk.waitForCompletion(YKLIB_NO_WAIT,
                                       respBuf, sizeof(respBuf), &timer);
  if (rc != YKLIB_PROCESSING && rc != YKLIB_TIMER_WAIT) {
    m_pending = false;
    m_yk.closeKey();
  }
  yubiProcessCompleted(rc, timer, respBuf);
  trashMemory(respBuf, sizeof(respBuf));
}

StringX CYubiMixin::Bin2Hex(const unsigned char *buf, int len)
{
  std::wostringstream os;
  os << std::setw(2);
  os << std::setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << std::hex << std::setw(2) << int(buf[i]);
  }
  return StringX(os.str().c_str());
}
