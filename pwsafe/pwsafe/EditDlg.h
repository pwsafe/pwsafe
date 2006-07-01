// EditDlg.h
//-----------------------------------------------------------------------------

#include "afxwin.h"
class CEditDlg
   : public CDialog
{

public:
   // default constructor
   CEditDlg(CWnd* pParent = NULL);
   virtual ~CEditDlg();

   enum { IDD = IDD_EDIT };
   CMyString m_notes;
   CMyString m_password;
   CMyString m_username;
   CMyString m_title;
   CMyString m_group;
   CMyString m_URL;
   CMyString m_autotype;
   CMyString m_ascCTime;
   CMyString m_ascPMTime;
   CMyString m_ascATime;
   CMyString m_ascLTime;
   CMyString m_ascRMTime;
   time_t m_tttLTime;
   int m_MaxPWHistory;
   int m_NumPWHistory;

   CListCtrl m_PWHistListCtrl;

   PWHistList* m_pPWHistList;

   CMyString m_realpassword;

   POSITION  m_listindex;

   bool m_IsReadOnly;
   void  ShowPassword(void);
   void  HidePassword(void);
   bool m_ClearPWHistory;
   BOOL m_SavePWHistory;
   int m_iSortedColumn;
   BOOL m_bSortAscending;
   bool m_bSavePWHistory;

private:
   bool m_isPwHidden;
   // Are we showing more or less details?
   bool m_isExpanded;
   // Are we showing EVEN more or less details?
   bool m_isMoreExpanded;
   void ResizeDialog();
   void MakeDialogWider();

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
	afx_msg void OnBnClickedCopyToClipboard();
	afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedClearPWHist();

	CButton m_MoreLessBtn, m_EvenMoreLessBtn;

private:
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
