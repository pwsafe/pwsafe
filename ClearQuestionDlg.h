// ClearQuestionDlg.h : header file
//-----------------------------------------------------------------------------

#include "resource.h"


class CClearQuestionDlg : public CDialog
{
// Construction
public:
   CClearQuestionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CClearQuestionDlg)
   enum { IDD = IDD_SECURECLEAR };
   BOOL	m_dontaskquestion;
   //}}AFX_DATA

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CClearQuestionDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   // Generated message map functions
   //{{AFX_MSG(CClearQuestionDlg)
   virtual void OnCancel();
   virtual void OnOK();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
