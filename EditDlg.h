// EditDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "afxwin.h"
#include "ControlExtns.h"

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
  void  ShowNotes();
  void  HideNotes();

private:
  CItemData *m_ci; // The entry being edited
  CMyString m_title;
  CMyString m_group;
  CMyString m_realpassword, m_oldRealPassword;
  CMyString m_password, m_password2;
  CMyString m_notes, m_realnotes;
  CMyString m_URL;
  CMyString m_autotype;
  CMyString m_ascCTime;
  CMyString m_ascPMTime, m_ascATime, m_ascLTime, m_ascRMTime;
  time_t m_tttLTime;
  bool m_bIsModified;
  // Password History related stuff
  int m_NumPWHistory;
  int m_MaxPWHistory;
  BOOL m_SavePWHistory;
  PWHistList m_PWHistList;
  CMyString m_PWHistory;

  bool m_isPwHidden, m_isNotesHidden;
  // Are we showing more or less details?
  bool m_isExpanded;
  // following two are not directly derived from CItemData
  CMyString m_oldascLTime;
  int m_oldMaxPWHistory;
  void ResizeDialog();
  void UpdateHistory();

  CComboBoxExtn m_ex_group;
  CEditExtn m_ex_password;
  CEditExtn m_ex_password2;
  CEditExtn m_ex_notes;
  CEditExtn m_ex_username;
  CEditExtn m_ex_title;
  CEditExtn m_ex_URL;
  CEditExtn m_ex_autotype;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  afx_msg void OnShowPassword();
  afx_msg void OnShowNotes();
  virtual void OnOK();
  virtual BOOL OnInitDialog();
  afx_msg void OnRandom();
  afx_msg void OnHelp();
  afx_msg void OnPasskeySetfocus();
  afx_msg void OnPasskeyKillfocus();

  DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedMore();
  afx_msg void OnBnClickedClearLTime();
  afx_msg void OnBnClickedSetLTime();
  afx_msg void OnBnClickedPwhist();

  CButton m_MoreLessBtn;

};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
