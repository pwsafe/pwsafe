/// \file ThisMfcApp.h
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------
#if !defined(ThisMfcApp_h)
#define ThisMfcApp_h
#include "PasswordSafe.h"
#include "stdafx.h"
#include "corelib/MyString.h"
#include "corelib/Util.h"
#include "corelib/PWScore.h"
#include "SystemTray.h"
//-----------------------------------------------------------------------------
class DboxMain;

class ThisMfcApp
   : public CWinApp
{
public:
  ThisMfcApp();
  ~ThisMfcApp();
   
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
  CSystemTray m_TrayIcon;
};

//-----------------------------------------------------------------------------

/*
  a globally available reference to the app object, which is a whole lot
  cleaner (in my mind) than constantly calling AfxGetApp() for the same
  thing... {jpr}
*/

//extern ThisMfcApp app;


//-----------------------------------------------------------------------------
#endif // !defined(ThisMfcApp_h)
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
