/// \file PasswordSafe.cpp
/// \brief  Defines the class behaviors for the application.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "PasskeyEntry.h"
#include "PasskeySetup.h"
#include "CryptKeyEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
/*
  The one and only CPasswordSafeApp object.  In the MFC world, creating
  this global object is what starts the application.
*/

CPasswordSafeApp app;

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPasswordSafeApp, CWinApp)
   ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CPasswordSafeApp::CPasswordSafeApp()
{
   srand((unsigned)time(NULL));
}


CPasswordSafeApp::~CPasswordSafeApp()
{
   //We no longer need the global bf_P and bf_S variables, so trash them
   trashMemory((unsigned char*)tempbf_P, 18*4);
   trashMemory((unsigned char*)tempbf_S, 256*4);
   trashMemory((unsigned char*)bf_P, 18*4);
   trashMemory((unsigned char*)bf_S, 256*4);
}


BOOL
CPasswordSafeApp::InitInstance()
{
   // Standard initialization
   // If you are not using these features and wish to reduce the size
   //  of your final executable, you should remove from the following
   //  the specific initialization routines you do not need.

#ifdef _AFXDLL
   Enable3dControls();          // Call this when using MFC in a shared DLL
#else
   Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

   if (GetOSVersion() == Win32s)
   {
      AfxMessageBox(
         "Sorry. Password Safe uses advanced Windows features and\n"
         "cannot be run under Windows 3.x with Win32s\n"
         "The program will terminate now.");
      exit(20);
   }

   /*
     save the current directory in a data member - I wonder why {jpr}
   */

   char fn[_MAX_PATH];
   int result = GetCurrentDirectory(_MAX_PATH, fn);
   if ((result == 0) || (result > _MAX_PATH))
      m_curdir = "";
   else
      m_curdir = (CMyString)fn + (CMyString) "\\";

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
      CCryptKeyEntry dlg(NULL);
      int nResponse = dlg.DoModal();

      if (nResponse==IDOK)
      {
         m_passkey = dlg.m_cryptkey1;
         manageCmdLine(m_lpCmdLine);
      }
   }
   else
   {
      m_maindlg = new DboxMain(NULL);
      m_pMainWnd = m_maindlg;

      //Set up an Accelerator table
      m_ghAccelTable = LoadAccelerators(AfxGetInstanceHandle(),
                                        MAKEINTRESOURCE(IDR_ACCS));

      //Run dialog
      //int rc  = m_maindlg->DoModal();
      (void) m_maindlg->DoModal();

      /*
        note that we don't particularly care what the response was
      */

      delete m_maindlg;
   }

   // Since the dialog has been closed, return FALSE so that we exit the
   //  application, rather than start the application's message pump.
   return FALSE;
}


//Copied from Knowledge Base article Q100770
BOOL
CPasswordSafeApp::ProcessMessageFilter(int code,
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
