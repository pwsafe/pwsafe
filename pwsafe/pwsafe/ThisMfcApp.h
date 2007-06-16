/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file ThisMfcApp.h
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------
#if !defined(ThisMfcApp_h)
#define ThisMfcApp_h

#include "corelib/MyString.h"
#include "corelib/Util.h"
#include "corelib/PWScore.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
  #include "pocketpc/resource.h"
  #include "resource3.h"  // String resources
#else
  #include <errno.h>
  #include <io.h>
  #include "resource.h"
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif
//-----------------------------------------------------------------------------
class DboxMain;

class ThisMfcApp
   : public CWinApp
{
public:
  ThisMfcApp();
  ~ThisMfcApp();
   
  CMyString m_passkey; // the main one, in memory?!? yikes {jpr}

  // Following used to verify passkey against file's passkey
  unsigned char m_randstuff[StuffSize];
  unsigned char m_randhash[20];   // HashSize

  HACCEL m_ghAccelTable;

  CRecentFileList*	GetMRU()			{ return m_pMRU; }

public:
  DboxMain* m_maindlg;
  PWScore m_core;

  virtual BOOL InitInstance();
WCE_DEL  virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

  void		EnableAccelerator()						{ m_bUseAccelerator = true; }
  void		DisableAccelerator()					{ m_bUseAccelerator = false; }

  afx_msg void OnHelp();

  static void	StripFileQuotes( CString& strFilename );

  DECLARE_MESSAGE_MAP()

protected:
	CRecentFileList*		m_pMRU;
	bool					m_bUseAccelerator;
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
