// PasskeyEntry.h
//-----------------------------------------------------------------------------

#include "SysColStatic.h"
#include "corelib/MyString.h"
#include "resource.h" // ronys
//-----------------------------------------------------------------------------
class CPasskeyEntry
   : public CDialog
{
// Construction
public:
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
	CSysColStatic	m_ctlLogo;
	CSysColStatic	m_ctlLogoText;
	CButton	m_ctlOK;
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
   //}}AFX_MSG
   afx_msg void OnBrowse();
   afx_msg void OnCreateDb();

   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
