// QueryAddName.h
//-----------------------------------------------------------------------------

class CQueryAddName : public CDialog
{
// Construction
public:
   CQueryAddName(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CQueryAddName)
   enum { IDD = IDD_QUERYADDNAME };
   BOOL	m_dontqueryname;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CQueryAddName)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(CQueryAddName)
   virtual void OnOK();
   virtual void OnCancel();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
