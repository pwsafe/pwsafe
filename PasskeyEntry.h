// PasskeyEntry.h
//-----------------------------------------------------------------------------

#include "SysColStatic.h"

//-----------------------------------------------------------------------------
class CPasskeyEntry
   : public CDialog
{
// Construction
public:
   CPasskeyEntry(CWnd* pParent,
                 const CString& a_filespec,
                 BOOL first = FALSE); 

   int GetCancelReturnValue();

// Dialog Data
   //{{AFX_DATA(CPasskeyEntry)
   enum { IDD = IDD_PASSKEYENTRY, IDDFIRST = IDD_PASSKEYENTRY_FIRST };
   CMyString	m_passkey;
   //}}AFX_DATA
   CString	m_message;

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CPasskeyEntry)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   CSysColStatic m_Static,m_Static2,m_Static3;
   int numtimes;
   int tryagainreturnval;
   BOOL m_first;

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
