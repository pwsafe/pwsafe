// ConfirmDeleteDlg.h
//-----------------------------------------------------------------------------
#include "corelib/PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #include "resource.h"
  #define SUPERCLASS	CDialog
#endif

class CConfirmDeleteDlg : public SUPERCLASS
{
	typedef SUPERCLASS		super;

// Construction
public:
   CConfirmDeleteDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CConfirmDeleteDlg)
   enum { IDD = IDD_CONFIRMDELETE_DIALOG };
   bool	m_dontaskquestion;
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

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
