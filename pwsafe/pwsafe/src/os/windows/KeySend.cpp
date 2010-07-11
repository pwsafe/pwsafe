/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __WX__
#include <afxwin.h>
#endif

#include "../KeySend.h"
#include "../env.h"
#include "../debug.h"

class CKeySendImpl {
public:
  void SendChar(TCHAR c);
  void OldSendChar(TCHAR c);
  void NewSendChar(TCHAR c);
  int m_delay;
  HKL m_hlocale;
  bool m_isOldOS;
};

CKeySend::CKeySend(bool bForceOldMethod)
 : m_delayMS(10)
{
  m_impl = new CKeySendImpl;
  m_impl->m_delay = m_delayMS;
  // We want to use keybd_event (OldSendChar) for Win2K & older,
  // SendInput (NewSendChar) for newer versions.
  if (bForceOldMethod)
    m_impl->m_isOldOS = true;
  else {
    DWORD majorVersion, minorVersion;
    pws_os::getosversion(majorVersion, minorVersion);
    m_impl->m_isOldOS = ((majorVersion <= 4) ||
                         (majorVersion == 5 && minorVersion == 0));
  }
  // get the locale of the current thread.
  // we are assuming that all window and threading in the 
  // current users desktop have the same locale.
  m_impl->m_hlocale = GetKeyboardLayout(0);
}

CKeySend::~CKeySend()
{
  delete m_impl;
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
  INPUT input[2];
  input[0].ki.time = input[1].ki.time = 0; //probably needed
  input[0].ki.dwExtraInfo = input[1].ki.dwExtraInfo = 0; //probably not
  
  input[0].type = input[1].type = INPUT_KEYBOARD;

  bool tabWait = false; // if sending tab\newline, wait a bit after send
  //                       for the dust to settle. Thanks to Larry...
  switch (c) {
    case L'\t':
    case L'\r':
      input[0].ki.wVk = c == L'\t' ? VK_TAB : VK_RETURN;
      input[0].ki.wScan = 0;
      input[0].ki.dwFlags = 0;
      tabWait = true;
      break;
    default:
      input[0].ki.wVk = 0;
      input[0].ki.wScan = c;
      input[0].ki.dwFlags = KEYEVENTF_UNICODE;
      break;
  }
  // add the key-up event
  input[1].ki = input[0].ki;
  input[1].ki.dwFlags |= KEYEVENTF_KEYUP;

  status = ::SendInput(2, input, sizeof(INPUT));
  if (status != 2)
    pws_os::Trace(L"CKeySend::SendChar: SendInput failed status=%d\n", status);
  // wait at least 200 mS if we just sent a tab, regardless of m_delay
  int delay = ( m_delay < 200 && tabWait) ? 200 : m_delay;
  ::Sleep(delay);
}

void CKeySendImpl::OldSendChar(TCHAR c)
{
  BOOL shiftDown = false; //assume shift key is up at start.
  BOOL ctrlDown = false;
  BOOL altDown = false;
  SHORT keyScanCode = VkKeyScanEx(c, m_hlocale);
  // high order byte of keyscancode indicates if SHIFT, CTRL etc keys should be down 
  if (keyScanCode & 0x100) {
    shiftDown=true;      
    //send a shift down
    keybd_event(VK_SHIFT, (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale), 0, 3); //Fixes bug #1208955
  } 

  if (keyScanCode & 0x200) {
    ctrlDown=true;       
    //send a ctrl down
    keybd_event(VK_CONTROL, (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale), KEYEVENTF_EXTENDEDKEY, 0); 
  } 

  if (keyScanCode & 0x400) {
    altDown=true; 
    //send a alt down
    keybd_event(VK_MENU, (BYTE) MapVirtualKeyEx(VK_MENU, 0, m_hlocale), KEYEVENTF_EXTENDEDKEY, 0);    
  } 

  // the lower order byte has the key scan code we need.
  keyScanCode =(SHORT)(keyScanCode & 0xFF);

  keybd_event((BYTE)keyScanCode, (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale), 0, 0);      
  keybd_event((BYTE)keyScanCode, (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale), KEYEVENTF_KEYUP, 0);    

  if (shiftDown) {
    //send a shift up
    keybd_event(VK_SHIFT, (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale), KEYEVENTF_KEYUP, 3); //Fixes bug #1208955
    shiftDown=false;
  }

  if (ctrlDown) {
    //send a ctrl up
    keybd_event(VK_CONTROL, (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale),
                KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0); 
    ctrlDown=false;
  } 

  if (altDown) {
    //send a alt up
    keybd_event(VK_MENU, (BYTE) MapVirtualKeyEx(VK_MENU, 0, m_hlocale), KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0); 
    altDown=false;       
  } 
  ::Sleep(m_delay);
}

static void newSendVK(WORD vk)
{
  UINT status;
  INPUT input[2];
  input[0].ki.time = input[1].ki.time = 0; //probably needed
  input[0].ki.dwExtraInfo = input[1].ki.dwExtraInfo = 0; //probably not
  input[0].type = input[1].type = INPUT_KEYBOARD;
  input[0].ki.wVk = input[1].ki.wVk = vk;
  input[0].ki.dwFlags = 0;
  input[1].ki.dwFlags = KEYEVENTF_KEYUP;
  status = ::SendInput(2, input, sizeof(INPUT));
  if (status != 2)
    pws_os::Trace(L"newSendVK: SendInput failed status=%d\n", status);
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
      keybd_event(VK_CONTROL, (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_impl->m_hlocale),
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

