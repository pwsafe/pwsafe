#include "keysend.h"

CKeySend::CKeySend(void)
{
	// get the locale of the current thread.
	// we are assuming that all window and threading in the 
	// current users desktop have the same locale.
	m_hlocale = GetKeyboardLayout(0);
}

CKeySend::~CKeySend(void)
{
}


void CKeySend::SendString(CMyString data)
{
	
	for(int n=0;n<data.GetLength();n++){
		SendChar(data[n]);
	}
	

}

void CKeySend::SendChar(TCHAR c){
	
	BOOL shiftDown=false; //assume shift key is up at start.
	SHORT keyScanCode=VkKeyScanEx(c, m_hlocale );
	// high order byte of keyscancode indicates if SHIFT, CTRL etc keys should be down 
	// We only process the shift key at this stage
	if(keyScanCode & 0x100){
		shiftDown=true;	
		//send a shift down
		keybd_event(VK_SHIFT,  (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale ), KEYEVENTF_EXTENDEDKEY, 0);	
		
	} 
	// the lower order byte has the key scan code we need.
	keyScanCode =(SHORT)( keyScanCode & 0xFF);

	keybd_event((BYTE)keyScanCode,  (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale ), 0, 0);	
	keybd_event((BYTE)keyScanCode,  (BYTE) MapVirtualKeyEx(keyScanCode, 0, m_hlocale ), KEYEVENTF_KEYUP, 0);	

	if(shiftDown){
		//send a shift up
		keybd_event(VK_SHIFT,  (BYTE) MapVirtualKeyEx(VK_SHIFT, 0, m_hlocale ), KEYEVENTF_KEYUP |KEYEVENTF_EXTENDEDKEY, 0);	
		shiftDown=false;
	}

	::Sleep(m_delay);
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
	Sleep(m_delay);
}

void CKeySend::SetDelay(int d){
	m_delay=d;
	
}