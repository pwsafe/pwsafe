// DboxMain.h
//-----------------------------------------------------------------------------

#ifndef DboxMain_h
#define DboxMain_h

#include "ItemData.h"
#include "util.h"
#include "resource.h"

//-----------------------------------------------------------------------------
class DboxMain
   : public CDialog
{
public:
   // default constructor
   DboxMain(CWnd* pParent = NULL);

   // the password database
   CList<CItemData,CItemData> m_pwlist;

   POSITION Find(const CMyString &lpszString);

   void RefreshList();

  void SetCurFile(const CString &arg) {m_currfile = CMyString(arg);} // set to argv

   enum retvals
   {
      CANT_OPEN_FILE = -10,
      USER_CANCEL,
      WRONG_PASSWORD,
      NOT_SUCCESS,
      SUCCESS = 0
   };

   enum { IDD = IDD_PASSWORDSAFE_DIALOG };

protected:
   virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
   HICON m_hIcon;
   CListBox* m_listctrl;

   unsigned int uGlobalMemSize;
   HGLOBAL hGlobalMemory;

   CMyString m_currfile; // current pw db filespec
   CMyString m_currbackup;
   CMyString m_title; // what's displayed in the title bar

   CToolBar m_wndToolBar;
   CStatusBar m_statusBar;
   BOOL m_toolbarsSetup;

   BOOL m_changed;
   BOOL m_needsreading;
   bool m_windowok;
   BOOL m_existingrestore;

   void ChangeOkUpdate();
   BOOL SelItemOk();
   void ClearClipboard();
   void setupBars();
   BOOL OpenOnInit();

   void ClearData();
   int NewFile(void);
   int WriteFile(const CMyString &filename);
   int CheckPassword(const CMyString &filename, CMyString &passkey,
                     bool first = false);
   int ReadFile(const CMyString &filename, const CMyString &passkey);

   //Version of message functions with return values
   int Save(void);
   int SaveAs(void);
   int Open(void);
   int BackupSafe(void);
   int New(void);
   int Restore(void);

   // Generated message map functions
   virtual BOOL OnInitDialog();
   afx_msg void OnDestroy();
   afx_msg void OnPaint();
   afx_msg HCURSOR OnQueryDragIcon();
   virtual void OnCancel();
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnAbout();
   afx_msg void OnCopyUsername();
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
   afx_msg void OnCopyPassword();
   afx_msg void OnNew();
   afx_msg void OnOpen();
   afx_msg void OnRestore();
   afx_msg void OnSaveAs();
   afx_msg void OnBackupSafe();
   afx_msg void OnUpdateBackups();
   afx_msg void OnPasswordChange();
   afx_msg void OnClearclipboard();
   afx_msg void OnDelete();
   afx_msg void OnEdit();
   afx_msg void OnOptions();
   afx_msg void OnSave();
   afx_msg void OnAdd();
   afx_msg void OnOK();
   afx_msg void OnSetfocusItemlist();
   afx_msg void OnKillfocusItemlist();
   afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnDropFiles(HDROP hDrop);

   DECLARE_MESSAGE_MAP()

   // Following moved from Util.{h,cpp} and constified
public:
   void MakeName(CMyString&, const CMyString &, const CMyString &) const; // used also by AddDlg, hence public

private:
  int WriteCBC(int fp, const CString &data, const unsigned char *salt, unsigned char *ipthing);
  int ReadCBC(int fp, CMyString &data, const unsigned char *salt, unsigned char *ipthing);
  void MakeFullNames(CList<CItemData, CItemData>* plist,
		     const CMyString &defusername);
  void DropDefUsernames(CList<CItemData, CItemData>* plist,
			const CMyString &defusername);
  int CheckVersion(CList<CItemData, CItemData>* plist);
  void SetBlankToDef(CList<CItemData, CItemData>* plist);
  void SetBlankToName(CList<CItemData, CItemData>* plist, const CMyString &username);
  BOOL CheckExtension(const CMyString &name, const CMyString &ext) const;
  int SplitName(const CMyString &name , CMyString &title, CMyString &username) const;
};

//-----------------------------------------------------------------------------
#endif // DboxMain_h
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
