/// \file AddDlg.h
//-----------------------------------------------------------------------------

#include "afxwin.h"
class CAddDlg : public CDialog
{
  // Construction
public:
  CAddDlg(CWnd* pParent = NULL);   // standard constructor

  // Dialog Data
  //{{AFX_DATA(CAddDlg)
  enum { IDD = IDD_ADD };
  CMyString	m_password, m_password2;
  CMyString	m_notes;
  CMyString	m_username;
  CMyString	m_title;
  CMyString	m_group;
  CMyString m_URL;
  CMyString m_autotype;
  CMyString m_ascLTime;
  time_t m_tttLTime;
  BOOL m_SavePWHistory;
  int m_MaxPWHistory;

  void  ShowPassword();
  void  HidePassword();

  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAddDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
private:
  TCHAR m_passwordchar;
  bool m_isPwHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  void ResizeDialog();

  // Implementation
protected:

  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAddDlg)
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnHelp();
  afx_msg void OnRandom();
  afx_msg void OnShowpassword();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
    public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedMore();
	afx_msg void OnBnClickedClearLTime();
	afx_msg void OnBnClickedSetLTime();
	afx_msg void OnCheckedSavePasswordHistory();
	CButton m_moreLessBtn;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
