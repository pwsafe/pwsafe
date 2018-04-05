/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "../KeySend.h"
#include "../sleep.h"
#include "./macsendstring.h"
#include "../../core/UTF8Conv.h"

CKeySend::CKeySend(bool, unsigned defaultDelay)
  : m_delayMS(defaultDelay)
{
}

CKeySend::~CKeySend()
{
}

void CKeySend::SendString(const StringX &data)
{
  CUTF8Conv conv;
  const unsigned char* str = 0;
  size_t len = 0;
  conv.ToUTF8(data, str, len);
  if (len && str && str[0])
    pws_os::SendString(reinterpret_cast<const char *>(str), m_delayMS);
}

void CKeySend::SetDelay(unsigned d)
{
  m_delayMS = d;
}

// SetAndDelay allows users to put \d500\d10 in autotype and
// then it will cause a delay of half a second then subsequent
// key stokes will be delayed by 10 ms 
// thedavecollins 2004-08-05

void CKeySend::SetAndDelay(unsigned d) {
  SetDelay(d);
  pws_os::sleep_ms(m_delayMS);
}

//it doesn't matter in X what the CAPSLOCK state is
void CKeySend::SetCapsLock(const bool /*bState*/)
{
}

bool CKeySend::isCapsLocked() const
{
  return false; // as X doesn't care
}

void CKeySend::BlockInput(bool) const
{
  // XXX TBD
}

void CKeySend::ResetKeyboardState() const
{
  // XXX Need we implement this for X?
}

#ifdef __PWS_MACINTOSH__
bool CKeySend::SimulateApplicationSwitch(void)
{
  return pws_os::MacSimulateApplicationSwitch(m_delayMS);
}
#endif

void CKeySend::SelectAll() const
{
  const bool selectedAll = pws_os::SelectAll();
  VERIFY(selectedAll);
}

void CKeySend::EmulateMods(bool /*emulate*/)
{
  // I just don't know how to do it!
}

bool CKeySend::IsEmulatingMods() const
{
  return false;
}

void CKeySend::SendVirtualKey(WORD, bool, bool, bool)
{
}
void CKeySend::SetOldSendMethod(bool)
{
}

bool CKeySend::LookupVirtualKey(const StringX &, WORD &)
{
  return false;
}

stringT CKeySend::GetKeyName(WORD , bool)
{
  return stringT(_T(""));
}
