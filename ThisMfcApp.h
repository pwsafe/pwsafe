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

  DboxMain* m_maindlg;
  PWScore m_core;
  CMenu* m_mainmenu;
  BOOL m_mruonfilemenu;
  CString m_csDefault_Browser;
    
  virtual BOOL InitInstance();
WCE_DEL  virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

  void		EnableAccelerator()						{ m_bUseAccelerator = true; }
  void		DisableAccelerator()					{ m_bUseAccelerator = false; }

  BOOL SetTooltipText(LPCTSTR ttt) {return m_TrayIcon->SetTooltipText(ttt);}
  BOOL SetMenuDefaultItem(UINT uItem) {return m_TrayIcon->SetMenuDefaultItem(uItem, FALSE);}
  BOOL IsIconVisible() {return m_TrayIcon->Visible();}
  void ShowIcon() {m_TrayIcon->ShowIcon();}
  void HideIcon() {m_TrayIcon->HideIcon();}

  afx_msg void OnHelp();
  enum STATE {LOCKED, UNLOCKED};
  void SetSystemTrayState(STATE);
  STATE GetSystemTrayState() const {return m_TrayLockedState;}
  static void	StripFileQuotes( CString& strFilename );
  DECLARE_MESSAGE_MAP()
  

protected:
  CRecentFileList*		m_pMRU;
  bool					m_bUseAccelerator;
private:
  HICON m_LockedIcon;
  HICON m_UnLockedIcon;
  CSystemTray *m_TrayIcon; // DboxMain needs to be constructed first
  STATE m_TrayLockedState;
};


//-----------------------------------------------------------------------------
#endif // !defined(ThisMfcApp_h)
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
