#pragma once

// 
// KeySend.h
// thedavecollins 2004-08-07
// sends keystrokes
//-----------------------------------------------------------------------------

#include "corelib/PWScore.h"
#include "PasswordSafe.h"

class CKeySend
{
public:
	CKeySend(void);
	~CKeySend(void);
	void SendString(const CMyString &data);
	void ResetKeyboardState();
	void SendChar(TCHAR c);
	void SetDelay(int d);
	void SetAndDelay(int d);

private:
	int m_delay;
	HKL m_hlocale;
};

