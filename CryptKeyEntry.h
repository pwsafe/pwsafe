// CryptKeyEntry.h
//-----------------------------------------------------------------------------

#include "MyString.h"


class CCryptKeyEntry : public CDialog
{
// Construction
public:
   CCryptKeyEntry(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
   //{{AFX_DATA(CCryptKeyEntry)
   enum { IDD = IDD_CRYPTKEYENTRY };
   CMyString	m_cryptkey1;
   CMyString	m_cryptkey2;
   //}}AFX_DATA


// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CCryptKeyEntry)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:

   // Generated message map functions
   //{{AFX_MSG(CCryptKeyEntry)
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
