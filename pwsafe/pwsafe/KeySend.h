// 
// KeySend.h
// thedavecollins 2004-08-07
// sends keystrokes
//-----------------------------------------------------------------------------

#ifndef CKeySend_h
#define CKeySend_h

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

	// functions to prevent users from moving windows etc while passwords are being typed.
	void BlockUserInput();
	void UnBlockUserInput();
private:
	int m_delay;
	HKL m_hlocale;
	bool m_bIsKeyboardLocked;
};

#endif
