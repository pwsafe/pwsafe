// TryAgainDlg.h
//-----------------------------------------------------------------------------

#include "corelib/PwsPlatform.h"

//Globally useful values...
enum
{
   TAR_INVALID,
   TAR_NEW,
   TAR_OPEN
};

#if defined(POCKET_PC)
  #include "pocketpc/PwsPopupDialog.h"
  #define SUPERCLASS	CPwsPopupDialog
#else
  #define SUPERCLASS	CDialog
#endif


//-----------------------------------------------------------------------------
class CTryAgainDlg : public SUPERCLASS
{
// Construction
public:
	typedef SUPERCLASS	super;

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

#undef SUPERCLASS
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
