// PasskeyEntry.h
//-----------------------------------------------------------------------------

#include "SysColStatic.h"
#include "MyString.h"
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

   int GetStatus()
   { return m_status; }

// Dialog Data
   enum { IDD_BASIC = IDD_PASSKEYENTRY };
   //{{AFX_DATA(CPasskeyEntry)
	enum { IDD = IDD_PASSKEYENTRY_FIRST };
   CMyString	m_passkey;
	//}}AFX_DATA
   CString	m_message;

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CPasskeyEntry)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);
   //}}AFX_VIRTUAL

// Implementation
protected:
   CSysColStatic m_Static,m_Static2;
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
