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

int FindMenuItem(CMenu* Menu, LPCTSTR MenuString);
int FindMenuItem(CMenu* Menu, int MenuID);

class DboxMain;

class ThisMfcApp
   : public CWinApp
{
public:
  ThisMfcApp();
  ~ThisMfcApp();
   
  HACCEL m_ghAccelTable;

  CRecentFileList*	GetMRU()			{ return m_pMRU; }
  CSystemTray m_TrayIcon;

  DboxMain* m_maindlg;
  PWScore m_core;
  CMenu* m_mainmenu;
  BOOL m_mruonfilemenu;

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
#endif // !defined(ThisMfcApp_h)
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
