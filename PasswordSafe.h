// PasswordSafe.h
// main header file for the PasswordSafe application
//-----------------------------------------------------------------------------

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"  // main symbols
#include "Util.h"      // for StuffSize
#include "DboxMain.h"

//-----------------------------------------------------------------------------
class CPasswordSafeApp
   : public CWinApp
{
public:
   CPasswordSafeApp();
   ~CPasswordSafeApp();
   
   CMyString m_passkey; // the main one, in memory?!? yikes {jpr}
   CMyString m_curdir; /* only used once, in DboxMain constructor,
                          to make m_deffile */

   unsigned char m_randstuff[StuffSize];
   unsigned char m_randhash[SaltSize];

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

extern CPasswordSafeApp app;

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
