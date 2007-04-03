/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------
#include "stdafx.h" // thomas
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "corelib/Util.h"
#include "corelib/PWCharPool.h"
#include "DboxMain.h"


//-----------------------------------------------------------------------------
CMyString
DboxMain::GetPassword(void)
{
  CPasswordCharPool pwchars(app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwlendefault"), 8),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwuselowercase"), TRUE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwuseuppercase"), TRUE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusedigits"), TRUE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusesymbols"), FALSE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pweasyvision"), FALSE));

  return pwchars.MakePassword();
}



