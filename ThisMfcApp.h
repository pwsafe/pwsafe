/// \file ThisMfcApp.h
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------
#if !defined(ThisMfcApp_h)
#define ThisMfcApp_h

#include "PasswordSafe.h"

#include "DboxMain.h"
#include "MyString.h"

//-----------------------------------------------------------------------------
class ThisMfcApp
   : public CWinApp
{
public:
   ThisMfcApp();
   ~ThisMfcApp();
   
   CMyString m_passkey; // the main one, in memory?!? yikes {jpr}
#if 0
   CMyString m_curdir; /* only used once, in DboxMain constructor,
                          to make m_deffile */
#endif

   unsigned char m_randstuff[10];  // StuffSize
   unsigned char m_randhash[20];   // HashSize

   DboxMain* m_maindlg;

   HACCEL m_ghAccelTable;

public:
   virtual BOOL InitInstance();
   virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

   afx_msg void OnHelp();

   DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------

/*
  a globally available reference to the app object, which is a whole lot
  cleaner (in my mind) than constantly calling AfxGetApp() for the same
  thing... {jpr}
*/

extern ThisMfcApp app;

//-----------------------------------------------------------------------------
#endif // !defined(ThisMfcApp_h)
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
