// EditDlg.h
//-----------------------------------------------------------------------------

#include "afxwin.h"
class CItemData;

class CEditDlg
  : public CDialog
{

public:
  // default constructor
  CEditDlg(CItemData *ci, CWnd* pParent = NULL);
  virtual ~CEditDlg();

  enum { IDD = IDD_EDIT };
  CMyString m_defusername, m_username;

  POSITION  m_listindex;

  bool m_IsReadOnly;
  void  ShowPassword();
  void  HidePassword();
  bool IsModified() const {return m_bIsModified;}

private:
  CListCtrl m_PWHistListCtrl;
  PWHistList m_PWHistList;
  CItemData *m_ci; // The entry being edited
  CMyString m_title;
  CMyString m_group;
  CMyString m_realpassword, m_oldRealPassword;
  CMyString m_password, m_password2;
  CMyString m_notes;
  CMyString m_URL;
  CMyString m_autotype;
  CMyString m_ascCTime;
  CMyString m_ascPMTime, m_ascATime, m_ascLTime, m_ascRMTime;
  time_t m_tttLTime;
  int m_MaxPWHistory;
  int m_NumPWHistory;
  bool m_bIsModified;
  bool m_ClearPWHistory;
  BOOL m_SavePWHistory;
  int m_iSortedColumn;
  BOOL m_bSortAscending;
  TCHAR m_passwordchar;
  bool m_isPwHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  // following two are not directly derived from CItemData
  CMyString m_oldascLTime;
  int m_oldMaxPWHistory;
  void ResizeDialog();
  void UpdateHistory();

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

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
  public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
  afx_msg void OnBnClickedClearLTime();
  afx_msg void OnBnClickedSetLTime();
  afx_msg void OnBnClickedShowPasswordHistory();
  afx_msg void OnCheckedSavePasswordHistory();
  afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHistListClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnBnClickedClearPWHist();

  CButton m_MoreLessBtn;

private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
