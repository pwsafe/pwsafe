// TryAgainDlg.h
//-----------------------------------------------------------------------------

//Globally useful values...
enum
{
   TAR_INVALID,
   TAR_NEW,
   TAR_OPEN,
   TAR_CANCEL,
   TAR_EXIT
};


//-----------------------------------------------------------------------------
class CTryAgainDlg : public CDialog
{
// Construction
public:
   CTryAgainDlg(CWnd* pParent = NULL);   // standard constructor
   int GetCancelReturnValue();

// Dialog Data
   //{{AFX_DATA(CTryAgainDlg)
   enum { IDD = IDD_TRYAGAIN };
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CTryAgainDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   int cancelreturnval;

   // Generated message map functions
   //{{AFX_MSG(CTryAgainDlg)
   afx_msg void OnQuit();
   afx_msg void OnTryagain();
   afx_msg void OnHelp();
   afx_msg void OnOpen();
   afx_msg void OnNew();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
