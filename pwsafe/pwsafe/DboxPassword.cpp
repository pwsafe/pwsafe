/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "corelib/Util.h"
#include "corelib/PWCharPool.h"
#include "corelib/PWSprefs.h"

#include "DboxMain.h"


//-----------------------------------------------------------------------------

// Generate a new random password.
CMyString
DboxMain::GetPassword(void) const
{
  PWSprefs *prefs = PWSprefs::GetInstance();
  CPasswordCharPool pwchars(prefs->GetPref(PWSprefs::IntPrefs::PWLenDefault),
			    prefs->GetPref(PWSprefs::BoolPrefs::PWUseLowercase),
			    prefs->GetPref(PWSprefs::BoolPrefs::PWUseUppercase),
			    prefs->GetPref(PWSprefs::BoolPrefs::PWUseDigits),
			    prefs->GetPref(PWSprefs::BoolPrefs::PWUseSymbols),
			    prefs->GetPref(PWSprefs::BoolPrefs::PWUseHexDigits),
			    prefs->GetPref(PWSprefs::BoolPrefs::PWEasyVision));

  return pwchars.MakePassword();
}



