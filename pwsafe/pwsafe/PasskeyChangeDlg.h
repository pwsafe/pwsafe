// PasskeyChangeDlg.h
//-----------------------------------------------------------------------------

class CPasskeyChangeDlg : public CDialog
{
// Construction
public:
   CPasskeyChangeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CPasskeyChangeDlg)
   enum { IDD = IDD_KEYCHANGE_DIALOG };
   CMyString	m_confirmnew;
   CMyString	m_newpasskey;
   CMyString	m_oldpasskey;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CPasskeyChangeDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(CPasskeyChangeDlg)
   virtual void OnOK();
   virtual void OnCancel();
   afx_msg void OnHelp();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
