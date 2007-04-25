/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------
#include "stdafx.h" // thomas
#include "PasswordSafe.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PocketPC.h"
#else
  #include "resource.h"
  #include "resource3.h"  // String resources
#endif
#include "OptionsPasswordPolicy.h"
#include "corelib/Util.h"
#include "corelib/PWCharPool.h"
//#include "corelib/PWSprefs.h"

#include "DboxMain.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-----------------------------------------------------------------------------
CMyString
DboxMain::GetPassword(void)
{
  CPasswordCharPool pwchars(app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwlendefault"), 8),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwuselowercase"), TRUE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwuseuppercase"), TRUE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusedigits"), TRUE),
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusesymbols"), FALSE),
				FALSE, /// \todo add hexdigits when implemented
			    app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pweasyvision"), FALSE));

  return pwchars.MakePassword();
}



