// OptionsDlg.h
//-----------------------------------------------------------------------------

class COptionsDlg : public CDialog
{
// Construction
public:
   COptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(COptionsDlg)
   enum { IDD = IDD_OPTIONS };
   BOOL		m_clearclipboard;
   BOOL		m_confirmcopy;
   BOOL		m_confirmdelete;
   BOOL		m_lockdatabase;
   BOOL		m_confirmsaveonminimize;
   BOOL		m_pwshow;
   BOOL		m_usedefuser;
   CMyString	m_defusername;
   BOOL		m_querysetdef;
   BOOL		m_queryaddname;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(COptionsDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(COptionsDlg)
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
   afx_msg void OnLockbase();
   afx_msg void OnDefaultuser();
   virtual BOOL OnInitDialog();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
