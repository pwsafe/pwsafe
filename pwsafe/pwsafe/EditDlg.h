// EditDlg.h
//-----------------------------------------------------------------------------

class CEditDlg
   : public CDialog
{

public:
   // default constructor
   CEditDlg(CWnd* pParent = NULL)
      : CDialog(CEditDlg::IDD, pParent),
        m_isPwHidden(true)
   {}

   enum { IDD = IDD_EDIT };
   CMyString m_notes;
   CMyString m_password;
   CMyString m_username;
   CMyString m_title;
   CMyString m_group;

   CMyString m_realpassword;

   POSITION  m_listindex;

   void  ShowPassword(void);
   void  HidePassword(void);

private:
   bool m_isPwHidden;

protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   afx_msg void OnShowpassword();
   virtual void OnOK();
   virtual void OnCancel();
   virtual BOOL OnInitDialog();
   afx_msg void OnRandom();
   afx_msg void OnHelp();
#if defined(POCKET_PC)
   afx_msg void OnPasskeySetfocus();
   afx_msg void OnPasskeyKillfocus();
#endif
   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
