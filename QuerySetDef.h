// QuerySetDef.h
//-----------------------------------------------------------------------------
class CQuerySetDef : public CDialog
{
// Construction
public:
   CQuerySetDef(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CQuerySetDef)
   enum { IDD = IDD_QUERYSETDEF };
   BOOL	m_querycheck;
   //}}AFX_DATA
   CString	m_message;


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CQuerySetDef)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   //{{AFX_MSG(CQuerySetDef)
   virtual void OnOK();
   virtual void OnCancel();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
