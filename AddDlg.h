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
  CMyString	m_password;
  CMyString	m_notes;
  CMyString	m_username;
  CMyString	m_title;
  CMyString	m_group;
  CMyString m_URL;
  CMyString m_autotype;
  CMyString m_ascLTime;
  time_t m_tttLTime;
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAddDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
private:
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
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
    public:
	afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
	afx_msg void OnBnClickedClearLTime();
	afx_msg void OnBnClickedSetLTime();
	CButton m_moreLessBtn;
    afx_msg void OnStnClickedStaticLtime();
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
