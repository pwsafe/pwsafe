// UsernameEntry.h
//-----------------------------------------------------------------------------
class CUsernameEntry : public CDialog
{
// Construction
public:
   CUsernameEntry(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CUsernameEntry)
   enum { IDD = IDD_USERNAMEENTRY };
   BOOL		m_makedefuser;
   CMyString	m_username;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CUsernameEntry)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(CUsernameEntry)
   virtual void OnOK();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
