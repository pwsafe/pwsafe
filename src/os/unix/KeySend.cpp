/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <map>
#include <X11/keysym.h>

#include "../KeySend.h"
#include "../sleep.h"
#include "./xsendstring.h"
#include "../../core/Util.h"
#include "../../core/PWSprefs.h"

template <typename EnumType>
class PrefT;

template <>
struct PrefT<PWSprefs::BoolPrefs> {
  typedef bool type;
};

template <>
struct PrefT<PWSprefs::IntPrefs> {
  typedef int type;
};

template <>
struct PrefT<PWSprefs::StringPrefs> {
  typedef StringX type;
};

template <typename PrefEnum, typename Ret = typename PrefT<PrefEnum>::type >
static Ret GetPref(PrefEnum pref) {
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
  m_impl->SelectAll(m_delayMS, GetPref(PWSprefs::AutotypeSelectAllKeyCode),
                    GetPref(PWSprefs::AutotypeSelectAllModMask));
}

void CKeySend::EmulateMods(bool emulate)
{
  m_impl->EmulateMods(emulate);
}

bool CKeySend::IsEmulatingMods() const
{
  return m_impl->IsEmulatingMods();
}

void CKeySend::SendVirtualKey(WORD, bool, bool, bool)
{
}

void CKeySend::SetOldSendMethod(bool)
{
}

bool CKeySend::LookupVirtualKey(const StringX &kname, WORD &kval)
{
  static const std::map<std::wstring, WORD> vkmap = {
    {L"ENTER", XK_KP_Enter},
    {L"UP", XK_KP_Up},
    {L"DOWN", XK_KP_Down},
    {L"LEFT", XK_KP_Left},
    {L"RIGHT", XK_KP_Right},
    {L"HOME", XK_KP_Home},
    {L"END", XK_KP_End},
    {L"PGUP", XK_KP_Page_Up},
    {L"PGDN", XK_KP_Page_Down},
    {L"TAB", XK_KP_Tab},
    {L"SPACE", XK_KP_Space},
  };

  auto iter = vkmap.find(kname.c_str());
  if (iter == vkmap.end()) {
    kval = 0;
    return false;
  } else {
    kval = iter->second;
    return true;
  }
}

