// ConfirmDeleteDlg.h
//-----------------------------------------------------------------------------

class CConfirmDeleteDlg : public CDialog
{
// Construction
public:
   CConfirmDeleteDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CConfirmDeleteDlg)
   enum { IDD = IDD_CONFIRMDELETE_DIALOG };
   BOOL	m_dontaskquestion;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CConfirmDeleteDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   // Generated message map functions
   //{{AFX_MSG(CConfirmDeleteDlg)
   virtual void OnCancel();
   virtual void OnOK();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
