/// \file AddDlg.h
//-----------------------------------------------------------------------------

class CAddDlg : public CDialog
{
// Construction
public:
   CAddDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CAddDlg)
   enum { IDD = IDD_ADD };
   CMyString	m_password;
   CMyString	m_notes;
   CMyString	m_username;
   CMyString	m_title;
   //}}AFX_DATA

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CAddDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   virtual BOOL OnInitDialog();
   // Generated message map functions
   //{{AFX_MSG(CAddDlg)
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
   afx_msg void OnRandom();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
