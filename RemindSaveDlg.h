// RemindSaveDlg.h
//-----------------------------------------------------------------------------
class CRemindSaveDlg : public CDialog
{
// Construction
public:
   CRemindSaveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CRemindSaveDlg)
   enum { IDD = IDD_REMIND_SAVE };
   BOOL	m_dontask;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CRemindSaveDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(CRemindSaveDlg)
   virtual void OnCancel();
   virtual void OnOK();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
