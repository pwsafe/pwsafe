/// \file ThisMfcApp.h
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------
#if !defined(ThisMfcApp_h)
#define ThisMfcApp_h

#include "MyString.h"
#include "Util.h"
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

  virtual BOOL InitInstance();
  virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

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
