// PasskeyEntry.h
//-----------------------------------------------------------------------------

#include "SysColStatic.h"
#include "corelib/MyString.h"
#include "PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #include "resource.h" //ronys
  #define SUPERCLASS	CDialog
#endif

//-----------------------------------------------------------------------------
class CPasskeyEntry
   : public SUPERCLASS
{
// Construction
public:
	typedef	SUPERCLASS	super;

   CPasskeyEntry(CWnd* pParent,
                 const CString& a_filespec,
                 bool first = false); 

   int GetStatus() const
   { return m_status; }
  const CMyString &GetPasskey() const {return m_passkey;}
private:
// Dialog Data
   enum { IDD_BASIC = IDD_PASSKEYENTRY };
   //{{AFX_DATA(CPasskeyEntry)
	enum { IDD = IDD_PASSKEYENTRY_FIRST };
#if !defined(POCKET_PC)
	CSysColStatic	m_ctlLogo;
	CSysColStatic	m_ctlLogoText;
	CButton	m_ctlOK;
#endif
	CEdit	m_ctlPasskey;
  CMyString	m_passkey;
	//}}AFX_DATA
   CString	m_message;
  const CMyString m_filespec;

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CPasskeyEntry)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);
   //}}AFX_VIRTUAL

// Implementation
protected:
   int m_tries;
   int m_status;
   bool m_first;

   // Generated message map functions
   //{{AFX_MSG(CPasskeyEntry)
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
#if defined(POCKET_PC)
   afx_msg void OnPasskeySetfocus();
   afx_msg void OnPasskeyKillfocus();
#endif
   //}}AFX_MSG
   afx_msg void OnBrowse();
   afx_msg void OnCreateDb();

   DECLARE_MESSAGE_MAP()
};

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
