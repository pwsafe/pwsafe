/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __WX__
#include <afxwin.h>
#endif

#include <map>

#include "../KeySend.h"
#include "../env.h"
#include "../debug.h"

class CKeySendImpl
{
public:
  void SendChar(TCHAR c);
  void OldSendChar(TCHAR c);
  void NewSendChar(TCHAR c);
  int m_delay;
  HKL m_hlocale;
  bool m_isOldOS;
};

static INPUT cinput;
static bool bFirst = true;

CKeySend::CKeySend(bool bForceOldMethod, unsigned defaultDelay)
 : m_delayMS(defaultDelay)
{
  if (bFirst) {
    cinput.type = INPUT_KEYBOARD;
    cinput.ki.wVk = 0;
    cinput.ki.wScan = 0;
    cinput.ki.dwFlags = 0;
    cinput.ki.time = 0;
    cinput.ki.dwExtraInfo = 0;
    bFirst = false;
  }

  m_impl = new CKeySendImpl;
  m_impl->m_delay = m_delayMS;

  //Set Windows Send Method
  SetOldSendMethod(bForceOldMethod);

  // get the locale of the current thread.
  // we are assuming that all window and threading in the 
  // current users desktop have the same locale.
  m_impl->m_hlocale = GetKeyboardLayout(0);
}

CKeySend::~CKeySend()
{
  delete m_impl;
}

void CKeySend::SetOldSendMethod(bool bForceOldMethod)
{
  // We want to use keybd_event (OldSendChar) for Win2K & older,
  // SendInput (NewSendChar) for newer versions.

  // However, some key strokes only seem to work with the older method even on the
  // newer versions of Windows
  m_impl->m_isOldOS = bForceOldMethod;

  if (bForceOldMethod) {
    m_impl->m_isOldOS = true;
  } else {
    m_impl->m_isOldOS = !pws_os::IsWindowsVistaOrGreater();
  }
}

void CKeySend::SendString(const StringX &data)
{
  for (StringX::const_iterator iter = data.begin();
       iter != data.end(); iter++)
    m_impl->SendChar(*iter);
}

void CKeySendImpl::SendChar(TCHAR c)
{
  if (m_isOldOS)
    OldSendChar(c);
  else
    NewSendChar(c);
}

void CKeySendImpl::NewSendChar(TCHAR c)
{
  UINT status;
  INPUT input[4] = {cinput, cinput, cinput, cinput};
  UINT num_events(2);

  bool tabWait = false; // if sending tab\newline, wait a bit after send
  //                       for the dust to settle. Thanks to Larry...

  switch (c) {
    case L'\t':
    case L'\r':
      /*
        For Tab or Carriage Return equivalent to:
          keybd_event(VK_TAB or VK_RETURN, 0, 0, 0);
          keybd_event(VK_TAB or VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
      */
      input[0].ki.wVk = input[1].ki.wVk = (c == L'\t') ? VK_TAB : VK_RETURN;
      input[1].ki.dwFlags = KEYEVENTF_KEYUP;
      tabWait = true;
      break;
    case L'\v':
      /*
        Special case for Shift+Tab equivalent to:
        (assumes Shift key is not down to start!
          keybd_event(VK_SHIFT, 0, 0, 0);
          keybd_event(VK_TAB,   0, 0, 0);
          keybd_event(VK_TAB,   0, KEYEVENTF_KEYUP, 0);
          keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
      */
      num_events = 4;
      input[0].ki.wVk = input[3].ki.wVk = VK_SHIFT;
      input[1].ki.wVk = input[2].ki.wVk = VK_TAB;
      input[2].ki.dwFlags = KEYEVENTF_KEYUP;
      input[3].ki.dwFlags = KEYEVENTF_KEYUP;
      tabWait = true;
      break;
    default:
      /*
        For all others equivalent to:
          keybd_event(0, c, KEYEVENTF_UNICODE, 0);
          keybd_event(0, c, KEYEVENTF_UNICODE | KEYEVENTF_KEYUP, 0);
      */
      input[0].ki.wScan = input[1].ki.wScan = c;
      input[0].ki.dwFlags = KEYEVENTF_UNICODE;
      input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
      break;
  }
  
  status = ::SendInput(num_events, input, sizeof(INPUT));
  if (status != num_events)
    pws_os::Trace(L"CKeySend::SendChar: SendInput failed status=%d\n", status);

  // wait at least 200 ms if we just sent a tab, regardless of m_delay
  int delay = (m_delay < 200 && tabWait) ? 200 : m_delay;
  ::Sleep(delay);
}

void CKeySendImpl::OldSendChar(TCHAR c)
{
  BOOL shiftDown = false; // Assume shift key is up at start.
  BOOL ctrlDown = false;
  BOOL altDown = false;
  SHORT keyScanCode = VkKeyScanEx(c, m_hlocale);
  // high order byte of keyscancode indicates if SHIFT, CTRL etc keys should be down 
  if (keyScanCode & 0x100) {
    shiftDown = true;      
    //send a shift down -  Fixes bug #1208955
    keybd_event(VK_SHIFT, (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale), 0, 3);
  } 

  if (keyScanCode & 0x200) {
    ctrlDown = true;       
    //send a ctrl down
    keybd_event(VK_CONTROL, (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale),
               KEYEVENTF_EXTENDEDKEY, 0); 
  } 

  if (keyScanCode & 0x400) {
    altDown = true; 
    //send a alt down
    keybd_event(VK_MENU, (BYTE) MapVirtualKeyEx(VK_MENU, 0, m_hlocale),
                KEYEVENTF_EXTENDEDKEY, 0);    
  } 

  // the lower order byte has the key scan code we need.
  keyScanCode = (SHORT)(keyScanCode & 0xFF);

  keybd_event((BYTE)keyScanCode, (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale),
              0, 0);      
  keybd_event((BYTE)keyScanCode, (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale),
              KEYEVENTF_KEYUP, 0);    

  if (shiftDown) {
    //send a shift up
    keybd_event(VK_SHIFT, (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale),
                KEYEVENTF_KEYUP, 3); //Fixes bug #1208955
  }

  if (ctrlDown) {
    //send a ctrl up
    keybd_event(VK_CONTROL, (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale),
                KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0); 
  } 

  if (altDown) {
    //send a alt up
    keybd_event(VK_MENU, (BYTE) MapVirtualKeyEx(VK_MENU, 0, m_hlocale),
                KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0); 
  } 
  ::Sleep(m_delay);
}

static void newSendVK(WORD vk)
{
  UINT status;
  INPUT input[2] = {cinput, cinput};
  input[0].ki.wVk = input[1].ki.wVk = vk;
  input[1].ki.dwFlags = KEYEVENTF_KEYUP;
  status = ::SendInput(2, input, sizeof(INPUT));
  if (status != 2)
    pws_os::Trace(L"newSendVK: SendInput failed status=%d\n", status);
}

void CKeySend::SendVirtualKey(WORD wVK, bool bAlt, bool bCtrl, bool bShift)
{
  UINT status;
  INPUT input[8] = { cinput, cinput, cinput, cinput, cinput, cinput, cinput, cinput };
  int i = 0;
  if (bAlt) {
    // Add Alt key down
    input[i].ki.wVk = VK_MENU;
    i++;
  }

  if (bCtrl) {
    // Add Ctrl key down
    input[i].ki.wVk = VK_CONTROL;
    i++;
  }

  if (bShift) {
    // Add Shift key down
    input[i].ki.wVk = VK_SHIFT;
    i++;
  }

  // Add character key down
  input[i].ki.wVk = wVK;
  i++;

  // Add character key up
  input[i].ki.wVk = wVK;
  input[i].ki.dwFlags = KEYEVENTF_KEYUP;
  i++;

  if (bShift) {
    // Add Shift key up
    input[i].ki.wVk = VK_SHIFT;
    input[i].ki.dwFlags = KEYEVENTF_KEYUP;
    i++;
  }

  if (bCtrl) {
    // Add Ctrl kep up
    input[i].ki.wVk = VK_CONTROL;
    input[i].ki.dwFlags = KEYEVENTF_KEYUP;
    i++;
  }

  if (bAlt) {
    // Add Alt key up
    input[i].ki.wVk = VK_MENU;
    input[i].ki.dwFlags = KEYEVENTF_KEYUP;
    i++;
  }

  status = ::SendInput(i, input, sizeof(INPUT));
}

void CKeySend::ResetKeyboardState() const
{
  // We need to make sure that the Control Key is still not down. 
  // It will be down while the user presses ctrl-T the shortcut for autotype.

  BYTE keys[256];

  GetKeyboardState((LPBYTE)&keys);

  while((keys[VK_CONTROL] & 0x80) != 0) {
    // VK_CONTROL is down so send a key down and an key up...
    if (m_impl->m_isOldOS) {
      keybd_event(VK_CONTROL, (BYTE)MapVirtualKeyEx(VK_CONTROL, 0, m_impl->m_hlocale),
                  KEYEVENTF_EXTENDEDKEY, 0);
      keybd_event(VK_CONTROL, (BYTE)MapVirtualKeyEx(VK_CONTROL, 0, m_impl->m_hlocale),
                  KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    } else {
      newSendVK(VK_CONTROL); // Send Ctrl keydown/keyup via SendInput
    }

    //now we let the messages be processed by the applications to set the keyboard state
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
      // so there is a message process it.
#ifndef __WX__
      if (!AfxGetThread()->PumpMessage())
        break;
#else
      // Not sure this is correct!
      if (msg.message == WM_QUIT) {
        // Put it back on the queue and leave now
        ::PostQuitMessage(0);
        return;
      }
      
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
#endif
    }

    ::Sleep(10);
    SecureZeroMemory(keys, sizeof(keys));
    GetKeyboardState((LPBYTE)&keys);
  } // while
}

void CKeySend::SetDelay(unsigned d)
{
  m_delayMS = m_impl->m_delay = d;
}

// SetAndDelay allows users to put \d500\d10 in autotype and
// then it will cause a delay of half a second then subsequent
// key stokes will be delayed by 10 ms 
// thedavecollins 2004-08-05

void CKeySend::SetAndDelay(unsigned d) {
  SetDelay(d);
  ::Sleep(m_delayMS);
}

void CKeySend::SetCapsLock(const bool bState)
{
  BYTE keyState[256];

  GetKeyboardState((LPBYTE)&keyState);
  if ((bState && !(keyState[VK_CAPITAL] & 0x01)) ||
      (!bState && (keyState[VK_CAPITAL] & 0x01))) {
    if (m_impl->m_isOldOS) {
      // Simulate a key press
      keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
      // Simulate a key release
      keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    } else {
      newSendVK(VK_CAPITAL); // Send CapLock keydown/keyup via SendInput
    }
  }

  MSG msg;
  while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    // so there is a message process it.
#ifndef __WX__
    if (!AfxGetThread()->PumpMessage())
      break;
#else
    // Not sure this is correct!
    if (msg.message == WM_QUIT) {
      // Put it back on the queue and leave now
      ::PostQuitMessage(0);
      return;
    }
      
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
#endif
  }
}

bool CKeySend::isCapsLocked() const
{
  return ::GetKeyState(VK_CAPITAL) != 0;
}

void CKeySend::BlockInput(bool bi) const
{
  ::BlockInput(bi ? TRUE : FALSE);
}

void CKeySend::SelectAll() const
{
}

void CKeySend::EmulateMods(bool /*emulate*/)
{
}

bool CKeySend::IsEmulatingMods() const
{
  return false;
}

bool CKeySend::LookupVirtualKey(const StringX &kname, WORD &kval)
{
  static const std::map<std::wstring, WORD> vkmap = {
    {L"ENTER", (WORD)VK_RETURN},
    {L"UP", (WORD)VK_UP},
    {L"DOWN", (WORD)VK_DOWN},
    {L"LEFT", (WORD)VK_LEFT},
    {L"RIGHT", (WORD)VK_RIGHT},
    {L"HOME", (WORD)VK_HOME},
    {L"END", (WORD)VK_END},
    {L"PGUP", (WORD)VK_PRIOR},
    {L"PGDN", (WORD)VK_NEXT},
    {L"TAB", (WORD)VK_TAB},
    {L"SPACE", (WORD)VK_SPACE},
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

stringT CKeySend::GetKeyName(WORD wVirtualKeyCode, bool bExtended)
{
  /*
    Note the KeyName is taken from the current keyboard.  It is also the name 
    of the UNSHIFTED character and so the actual key is keyboard dependent.
    For example, UK keyboards have the double quote (") as SHIFT+2 and US keyboards
    have the AT symbol (@) symbol as SHIFT+2.  This shouldn't be a problem for the user
    as they rarely change keyboards!
  */
  stringT sKeyName;

  if (wVirtualKeyCode != 0) {
    TCHAR lpString[256];
    LPARAM lParam;

    // Get scan code
    UINT sc = MapVirtualKey(wVirtualKeyCode, 0);
    lParam = sc << 16;

    lParam |= 0x1 << 25;
    if (bExtended)
      lParam |= 0x1 << 24;

    if (GetKeyNameText((LONG)lParam, lpString, sizeof(lpString) / sizeof(TCHAR)) != 0)
      sKeyName = lpString;
  }

  return sKeyName;
}
