// PasskeySetup.h
//-----------------------------------------------------------------------------

#include "PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #define SUPERCLASS	CDialog
#endif

class CPasskeySetup : public SUPERCLASS
{
// Construction
public:
	typedef SUPERCLASS	super;

   CPasskeySetup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CPasskeySetup)
   enum { IDD = IDD_PASSKEYSETUP };
   CMyString	m_passkey;
   CMyString	m_verify;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CPasskeySetup)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   char GetRandAlphaNumChar();

   // Generated message map functions
   //{{AFX_MSG(CPasskeySetup)
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
#if defined(POCKET_PC)
   afx_msg void OnPasskeySetfocus();
   afx_msg void OnPasskeyKillfocus();
#endif
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
