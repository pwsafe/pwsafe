/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "keysend.h"

#include  <Winable.h>

CKeySend::CKeySend(void) : m_delay(10)
{
	// get the locale of the current thread.
	// we are assuming that all window and threading in the 
	// current users desktop have the same locale.
	m_hlocale = GetKeyboardLayout(0);
}

CKeySend::~CKeySend(void)
{
}


void CKeySend::SendString(const CMyString &data)
{
  const int N = data.GetLength();

  for(int n=0;n<N;n++){
    SendChar(data[n]);
  }
}

void CKeySend::SendChar(TCHAR c)
{
       BOOL shiftDown=false; //assume shift key is up at start.
       BOOL ctrlDown=false;
       BOOL altDown=false;
       SHORT keyScanCode=VkKeyScanEx(c, m_hlocale );

       // high order byte of keyscancode indicates if SHIFT, CTRL etc keys should be down 
       if(keyScanCode & 0x100){
              shiftDown=true;      
              //send a shift down
              keybd_event(VK_SHIFT,  (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale ), 0,3); //Fixes bug #1208955
       } 
 
       if(keyScanCode & 0x200){
              ctrlDown=true;       
              //send a ctrl down
              keybd_event(VK_CONTROL,  (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale ), KEYEVENTF_EXTENDEDKEY, 0); 
       } 
 
       if(keyScanCode & 0x400){
              altDown=true; 
              //send a alt down
              keybd_event(VK_MENU,  (BYTE) MapVirtualKeyEx(VK_MENU, 0, m_hlocale ), KEYEVENTF_EXTENDEDKEY, 0);    
       } 
 
       // the lower order byte has the key scan code we need.
       keyScanCode =(SHORT)( keyScanCode & 0xFF);
 
       keybd_event((BYTE)keyScanCode,  (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale ), 0, 0);      
       keybd_event((BYTE)keyScanCode,  (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale ), KEYEVENTF_KEYUP, 0);    
 
       if(shiftDown){
              //send a shift up
              keybd_event(VK_SHIFT,  (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale ), KEYEVENTF_KEYUP, 3); //Fixes bug #1208955
              shiftDown=false;
       }
 
       if(ctrlDown){
                     
              //send a ctrl up
              keybd_event(VK_CONTROL,  (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale ), KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0); 
              ctrlDown=false;
       } 
 
       if(altDown){
              //send a alt up
              keybd_event(VK_MENU,  (BYTE) MapVirtualKeyEx(VK_MENU, 0, m_hlocale ), KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0); 
              altDown=false;       
       } 
       ::BlockInput(true);
       ::Sleep(m_delay);
       ::BlockInput(false);
}



void CKeySend::ResetKeyboardState()
{
	// We need to make sure that the Control Key is still not down. 
	// It will be down while the user presses ctrl-T the shortcut for autotype.

	BYTE keys[256];


	GetKeyboardState((LPBYTE)&keys);

	while((keys[VK_CONTROL] & 0x80)!=0){
		// VK_CONTROL is down so send a key down and an key up...

		keybd_event(VK_CONTROL, (BYTE)MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale), KEYEVENTF_EXTENDEDKEY, 0);

		keybd_event(VK_CONTROL,  (BYTE) MapVirtualKeyEx(VK_CONTROL, 0, m_hlocale), KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY, 0);

		//now we let the messages be processed by the applications to set the keyboard state
		MSG msg;
		//BOOL m_bCancel=false;
		while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) )
		{
			// so there is a message process it.
			if (!AfxGetThread()->PumpMessage())
				break;
		}


		Sleep(10);
		memset((void*)&keys,0,256);
		GetKeyboardState((LPBYTE)&keys);
	}

}

// SetAndDelay allows users to put \d500\d10 in autotype and
// the it will cause a delay of half a second then subsequent
// key stokes will be delayed by 10 ms 
// thedavecollins 2004-08-05

void CKeySend::SetAndDelay(int d){
	SetDelay(d);
	::BlockInput(true);
	::Sleep(m_delay);
	::BlockInput(false);
}

void CKeySend::SetDelay(int d){
	m_delay=d;
}

void CKeySend::SetCapsLock(const bool bState)
{
  BYTE keyState[256];

  GetKeyboardState((LPBYTE)&keyState);
  if((bState && !(keyState[VK_CAPITAL] & 1)) ||
     (!bState && (keyState[VK_CAPITAL] & 1))) {
      // Simulate a key press
      keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
      // Simulate a key release
      keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
  }

  MSG msg;
  while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) ) {
    // so there is a message process it.
    if (!AfxGetThread()->PumpMessage())
      break;
  }
}
