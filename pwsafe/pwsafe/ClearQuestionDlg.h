// ClearQuestionDlg.h : header file
//-----------------------------------------------------------------------------

#include "resource.h"


class CClearQuestionDlg : public CDialog
{
public:
   CClearQuestionDlg(CWnd* pParent = NULL);   // standard constructor

   enum { IDD = IDD_SECURECLEAR };
   BOOL	m_dontaskquestion;

protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   virtual void OnCancel();
   virtual void OnOK();

   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
