// OptionsDlg.h
//-----------------------------------------------------------------------------

class COptionsDlg : public CDialog
{
public:
   COptionsDlg(CWnd* pParent = NULL);   // standard constructor

   enum { IDD = IDD_OPTIONS };
   BOOL		m_clearclipboard;
   BOOL		m_confirmcopy;
   BOOL		m_confirmdelete;
   BOOL		m_lockdatabase;
   BOOL		m_confirmsaveonminimize;
   BOOL		m_pwshowinedit;
   BOOL		m_pwshowinlist;
   BOOL		m_usedefuser;
   CMyString	m_defusername;
   BOOL		m_querysetdef;
   BOOL		m_queryaddname;
   BOOL		m_saveimmediately;
   BOOL		m_alwaysontop;
   CMyString	m_pwlendefault;

protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   virtual void OnCancel();
   virtual void OnOK();
   afx_msg void OnHelp();
   afx_msg void OnLockbase();
   afx_msg void OnDefaultuser();
   virtual BOOL OnInitDialog();

   DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
