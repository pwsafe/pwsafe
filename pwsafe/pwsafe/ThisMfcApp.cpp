/// \file ThisMfcApp.cpp
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ThisMfcApp.h"
#include "resource.h"

#include "PasskeyEntry.h"
#include "PasskeySetup.h"
//#include "CryptKeyEntry.h"

#include "BlowFish.h"

//-----------------------------------------------------------------------------
/*
  The one and only ThisMfcApp object.  In the MFC world, creating
  this global object is what starts the application.
*/

ThisMfcApp app;

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ThisMfcApp, CWinApp)
//   ON_COMMAND(ID_HELP, CWinApp::OnHelp)
   ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


ThisMfcApp::ThisMfcApp()
{
   srand((unsigned)time(NULL));
}


ThisMfcApp::~ThisMfcApp()
{
#if !defined(WITH_BACKEND)
   //We no longer need the global bf_P and bf_S variables, so trash them
   trashMemory((unsigned char*)tempbf_P, 18*4);
   trashMemory((unsigned char*)tempbf_S, 256*4);
   trashMemory((unsigned char*)bf_P, 18*4);
   trashMemory((unsigned char*)bf_S, 256*4);
#endif

   /*
     apparently, with vc7, there's a CWinApp::HtmlHelp - I'd like
     to see the docs, someday.  In the meantime, force with :: syntax
   */

   ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
}


BOOL
ThisMfcApp::InitInstance()
{
   /*
    * It's always best to start at the beginning.  [Glinda, Witch of the North]
    */

   /*
     this pulls 'Counterpane Systems' out of the string table, verifies
     that it exists (actually, only verifies that *some* IDS_COMPANY
     string exists -- it could be 'Microsoft' for all we know), and then
     instructs the app to use the registry instead of .ini files.  The
     path ends up being

     HKEY_CURRENT_USER\Software\(companyname)\(appname)\(sectionname)\(valuename)

     Assuming the open-source version of this is going to become less
     Counterpane-centric, I expect this may change, but if it does, an
     automagic migration ought to happen. -- {jpr}
   */

   CMyString companyname;
   VERIFY(companyname.LoadString(IDS_COMPANY) != 0);
   SetRegistryKey(companyname);

   /*
     Command line processing or main dialog box?
   */

   if (m_lpCmdLine[0] != '\0')
   {
#if defined(WITH_LEGACY_CMDLINE)
      CCryptKeyEntry dlg(NULL);
      int nResponse = dlg.DoModal();

      if (nResponse==IDOK)
      {
         m_passkey = dlg.m_cryptkey1;
         manageCmdLine(m_lpCmdLine);
      }
#else
      AfxMessageBox(
         "No command line support was compiled in.\n"
         "This feature is deprecated, anyway.\n"
         "\n"
         "Now, go away or I will taunt you a second time.");
      exit(20);

#endif
      /*
       * we're done with the 'commandline version' - exit the app
       */
      return FALSE;
   }

   /*
    * normal startup
    */

   /*
     Here's where PWS currently does DboxMain, which in turn will do
     the initial PasskeyEntry (the one that looks like a splash screen).
     This makes things very hard to control.
     The app object (here) should instead do the initial PasskeyEntry,
     and, if successful, move on to DboxMain.  I think. {jpr}
    */

   m_maindlg = new DboxMain(NULL);
   m_pMainWnd = m_maindlg;

   // Set up an Accelerator table
   m_ghAccelTable = LoadAccelerators(AfxGetInstanceHandle(),
                                     MAKEINTRESOURCE(IDR_ACCS));

   //Run dialog
   //int rc  = m_maindlg->DoModal();
   (void) m_maindlg->DoModal();

   /*
     note that we don't particularly care what the response was
   */

   delete m_maindlg;

   // Since the dialog has been closed, return FALSE so that we exit the
   //  application, rather than start the application's message pump.

   return FALSE;
}


//Copied from Knowledge Base article Q100770
BOOL
ThisMfcApp::ProcessMessageFilter(int code,
                                 LPMSG lpMsg)
{
   if (code < 0)
      CWinApp::ProcessMessageFilter(code, lpMsg);
	
   if (m_maindlg != NULL
       && m_ghAccelTable != NULL)
   {
      if (::TranslateAccelerator(m_maindlg->m_hWnd, m_ghAccelTable, lpMsg))
         return TRUE;
   }
   return CWinApp::ProcessMessageFilter(code, lpMsg);
}


void
ThisMfcApp::OnHelp()
{
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_intro.htm",
              HH_DISPLAY_TOPIC, 0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
