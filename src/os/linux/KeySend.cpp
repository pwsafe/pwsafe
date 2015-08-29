/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "../KeySend.h"
#include "../sleep.h"
#include "./xsendstring.h"
#include "../../core/Util.h"
#include "../../core/PWSprefs.h"

static bool GetPref(PWSprefs::BoolPrefs pref) {
  return PWSprefs::GetInstance()->GetPref(pref);
}

static pws_os::AutotypeMethod DefaultAutytypeMethod() {
  return GetPref(PWSprefs::UseAltAutoType)? pws_os::ATMETHOD_XTEST: pws_os::ATMETHOD_XSENDKEYS;
}


////////////////////////////////////////////////////
// CKeySend - The generic implementation
CKeySend::CKeySend(bool, unsigned defaultDelay)
  : m_delayMS(defaultDelay),
    m_impl(new CKeySendImpl(DefaultAutytypeMethod()))
{
}

CKeySend::~CKeySend()
{
  delete m_impl;
}

void CKeySend::SendString(const StringX &data)
{
  m_impl->SendString(data, m_delayMS);
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

void CKeySend::SelectAll() const
{
  m_impl->SelectAll(m_delayMS);
}

void CKeySend::EmulateMods(bool emulate)
{
  m_impl->EmulateMods(emulate);
}

bool CKeySend::IsEmulatingMods() const
{
  return m_impl->IsEmulatingMods();
}
