/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "corelib/Util.h"
#include "corelib/PWCharPool.h"
#include "DboxMain.h"


//-----------------------------------------------------------------------------
CMyString
DboxMain::GetPassword(void) const
{
  CPasswordCharPool pwchars(app.GetProfileInt("", "pwlendefault", 8),
			    app.GetProfileInt("", "pwuselowercase", TRUE),
			    app.GetProfileInt("", "pwuseuppercase", TRUE),
			    app.GetProfileInt("", "pwusedigits", TRUE),
			    app.GetProfileInt("", "pwusesymbols", FALSE),
			    app.GetProfileInt("", "pweasyvision", FALSE));

  return pwchars.MakePassword();
}



