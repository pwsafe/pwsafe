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

// static methods
private:
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int SplitName(const CMyString &name , CMyString &title, CMyString &username);

public:
   // default constructor
   DboxMain(CWnd* pParent = NULL);

 // Find in m_pwlist by title and user name, exact match
  POSITION Find(const CMyString &a_title, const CMyString &a_user);
 // Find in m_pwlist entry with same title and user name as the i'th entry in m_ctlItemList
  POSITION Find(int i);
  // FindAll is used by CFindDlg, returns # of finds.
  // indices allocated by caller
  int FindAll(const CString &str, BOOL CaseSensitive, int *indices);
  int GetNumEntries() const {return m_pwlist.GetCount();}
  BOOL SelectEntry(int i, BOOL MakeVisible = FALSE); // MakeVisible will scroll list, if needed
   void RefreshList();

  void SetCurFile(const CString &arg) {m_currfile = CMyString(arg);} // set to argv

   enum retvals
   {
      CANT_OPEN_FILE = -10,
      USER_CANCEL,
      WRONG_PASSWORD,
      NOT_SUCCESS,
	  ALREADY_OPEN,
      SUCCESS = 0,
   };

	//{{AFX_DATA(DboxMain)
	enum { IDD = IDD_PASSWORDSAFE_DIALOG };
	CListCtrl	m_ctlItemList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DboxMain)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:
   HICON m_hIcon;

   // used to speed up the resizable dialog so OnSize/SIZE_RESTORED isn't called
   bool	m_bSizing;

   // the password database
   CList<CItemData,CItemData> m_pwlist;

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

   bool m_bSortAscending;
   int m_iSortedColumn;

	bool m_bShowPassword;
	BOOL m_bAlwaysOnTop;

	int insertItem(CItemData &itemData, int iIndex = -1);
	int getSelectedItem();

   void ChangeOkUpdate();
   BOOL SelItemOk();
   void ClearClipboard();
   void setupBars();
   BOOL OpenOnInit();

   void ConfigureSystemMenu();
   void OnSysAlwaysOnTop();
	afx_msg void OnSysCommand( UINT nID, LPARAM lParam );

   void UpdateAlwaysOnTop();

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
   int Open( const char* pszFilename );
   int BackupSafe(void);
   int New(void);
   int Restore(void);

   // Generated message map functions
	//{{AFX_MSG(DboxMain)
   virtual BOOL OnInitDialog();
   afx_msg void OnDestroy();
   afx_msg void OnPaint();
   afx_msg HCURSOR OnQueryDragIcon();
   virtual void OnCancel();
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnAbout();
   afx_msg void OnCopyUsername();
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeydownItemlist(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnListDoubleClick( NMHDR * pNotifyStruct, LRESULT * result );
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
   afx_msg void OnFind();
   afx_msg void OnOptions();
   afx_msg void OnSave();
   afx_msg void OnAdd();
   afx_msg void OnOK();
   afx_msg void OnSetfocusItemlist( NMHDR * pNotifyStruct, LRESULT * result );
   afx_msg void OnKillfocusItemlist( NMHDR * pNotifyStruct, LRESULT * result );
   afx_msg void OnDropFiles(HDROP hDrop);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateMRU(CCmdUI* pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	//}}AFX_MSG

   afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenMRU(UINT nID);
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
};

//-----------------------------------------------------------------------------
#endif // DboxMain_h
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
