// PasswordSafe.h
// main header file for the PasswordSafe application
//-----------------------------------------------------------------------------

#define PWS_MFC 1

#if defined(PWS_MFC)
#include "stdafx.h"
#endif

/*
  jpr debug stuff
*/
#include "jprdebug.h"


/*
  eventually, this breaks off into pws_mfc.h
*/

/*
  a globally available reference to the app object, which is a whole lot
  cleaner (in my mind) than constantly calling AfxGetApp() for the same
  thing... {jpr}
*/

class ThisMfcApp;
extern ThisMfcApp app;

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
