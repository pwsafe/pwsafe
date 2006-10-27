#pragma once

// PasskeyEntry.h
//-----------------------------------------------------------------------------

#include "SysColStatic.h"
#include "corelib/MyString.h"
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #include "resource.h" //ronys
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
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
                 const CString& a_filespec, int index = 1 /* GCP_NORMAL */,
		         bool bReadOnly = false, bool bForceReadOnly = false); 

   int GetStatus() const {return m_status;}
  bool IsReadOnly() const {return m_ReadOnly == TRUE;}
  const CMyString &GetPasskey() const {return m_passkey;}
  CString m_appversion;
private:
// Dialog Data
   enum { IDD_BASIC = IDD_PASSKEYENTRY };
   enum { IDD_WEXIT = IDD_PASSKEYENTRY_WITHEXIT };
   //{{AFX_DATA(CPasskeyEntry)
	enum { IDD = IDD_PASSKEYENTRY_FIRST };
#if !defined(POCKET_PC)
	CSysColStatic	m_ctlLogo;
	CSysColStatic	m_ctlLogoText;
	CButton	m_ctlOK;
#endif
	CEdit m_ctlPasskey;
    CMyString m_passkey;
    BOOL m_ReadOnly;
    bool m_bForceReadOnly;
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
   int m_index;

   static int dialog_lookup[4];

   HICON m_hIcon;

   // Generated message map functions
   //{{AFX_MSG(CPasskeyEntry)
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
   afx_msg void OnExit();
   afx_msg void OnReadOnly();
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
