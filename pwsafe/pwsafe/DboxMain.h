// DboxMain.h
//-----------------------------------------------------------------------------

#ifndef DboxMain_h
#define DboxMain_h

#include "corelib/PWScore.h"
#include "corelib/PwsPlatform.h"
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/MyListCtrl.h"
#else
  #include "resource.h"
#endif
#include "MyTreeCtrl.h"

#if defined(POCKET_PC) || (_MFC_VER <= 1200)
DECLARE_HANDLE(HDROP);
#endif

// custom message event used for system tray handling.
#define WM_ICON_NOTIFY (WM_APP + 10)

// timer event number used to check if the workstation is locked
#define TIMER_CHECKLOCK 0x04
// timer event number used to support lock on user-defined timeout
#define TIMER_USERLOCK 0x05


//-----------------------------------------------------------------------------
class DboxMain
   : public CDialog
{
#if defined(POCKET_PC)
friend class CMyListCtrl;
#endif

// static methods
private:
    static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

public:
    // default constructor
    DboxMain(CWnd* pParent = NULL);
    ~DboxMain();

    // Find in core by title and user name, exact match
    POSITION Find(const CMyString &a_group,
		  const CMyString &a_title, const CMyString &a_user)
        {return m_core.Find(a_group, a_title, a_user);}

    // Find  entry in core with same title and user name as the i'th entry in m_ctlItemList
    POSITION Find(int i);

    // FindAll is used by CFindDlg, returns # of finds.
    // indices allocated by caller
    int FindAll(const CString &str, BOOL CaseSensitive, int *indices);

    // Count the number of total entries.
    int GetNumEntries() const {return m_core.GetNumEntries();}

    // Set the section to the entry.  MakeVisible will scroll list, if needed.
    BOOL SelectEntry(int i, BOOL MakeVisible = FALSE);
    void RefreshList();

    void SetCurFile(const CString &arg) {m_core.SetCurFile(CMyString(arg));}

    int CheckPassword(const CMyString &filename, CMyString &passkey)
        {return m_core.CheckPassword(filename, passkey);}
    void SetChanged(bool changed); // for MyTreeCtrl
    void UpdateListItemTitle(int lindex, const CString &newTitle); // when title edited in tree
    void UpdateListItemUser(int lindex, const CString &newUser); // when user edited in tree
    void SetReadOnly(bool state) { m_IsReadOnly = state;}
	void TreeSelectionChanged();
    bool MakeRandomPassword(CDialog * const pDialog, CMyString& password);

    //{{AFX_DATA(DboxMain)
	enum { IDD = IDD_PASSWORDSAFE_DIALOG };
#if defined(POCKET_PC)
	CMyListCtrl	m_ctlItemList;
#else
	CListCtrl	m_ctlItemList;
#endif
        CMyTreeCtrl     m_ctlItemTree;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DboxMain)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


protected:
   HICON m_hIcon;
   HICON m_hIconSm;

   // used to speed up the resizable dialog so OnSize/SIZE_RESTORED isn't called
   bool	m_bSizing;


   unsigned int uGlobalMemSize;
   HGLOBAL hGlobalMemory;

#if !defined(POCKET_PC)
   CMyString m_title; // what's displayed in the title bar
#endif

#if defined(POCKET_PC)
   CCeCommandBar	*m_wndCommandBar;
   CMenu			*m_wndMenu;
#else
   CToolBar m_wndToolBar;
   CStatusBar m_statusBar;
   BOOL m_toolbarsSetup;
   UINT m_toolbarMode;
#endif

   bool m_windowok;
   BOOL m_existingrestore;
   bool m_needsreading;

   bool m_bSortAscending;
   int m_iSortedColumn;

  bool m_bShowPasswordInEdit;
  bool m_bShowPasswordInList;
  bool m_bAlwaysOnTop;

  CMyString m_TreeViewGroup; // used by OnAdd & OnAddGroup

  int insertItem(CItemData &itemData, int iIndex = -1);
   CItemData *getSelectedItem();

   void ChangeOkUpdate();
   BOOL SelItemOk();
   void ClearClipboard();
  bool m_clipboard_set; // To verify that we're erasing *our* data
  unsigned char m_clipboard_digest[SHA1::HASHLEN]; // ditto
   void setupBars();
   BOOL OpenOnInit();
   // override following to reset idle timeout on any event
   virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);


   void ConfigureSystemMenu();
   void OnSysAlwaysOnTop();
   afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
   LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

   BOOL PreTranslateMessage(MSG* pMsg);

   void UpdateAlwaysOnTop();

   void ClearData();
   int NewFile(void);

   void SetListView();
   void SetTreeView();
   void SetToolbar(int menuItem);

   //Version of message functions with return values
   int Save(void);
   int SaveAs(void);
   int Open(void);
   int Open( const CMyString &pszFilename );
   int Merge(void);
   int Merge( const CMyString &pszFilename );

   int BackupSafe(void);
   int New(void);
   int Restore(void);

   // Generated message map functions
	//{{AFX_MSG(DboxMain)
   virtual BOOL OnInitDialog();
   afx_msg void OnDestroy();
   virtual void OnCancel();
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnAbout();
   afx_msg void OnPasswordSafeWebsite();
   afx_msg void OnBrowse();
   afx_msg void OnCopyUsername();
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnKeydownItemlist(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnItemDoubleClick( NMHDR * pNotifyStruct, LRESULT * result );
   afx_msg void OnCopyPassword();
   afx_msg void OnNew();
   afx_msg void OnOpen();
   afx_msg void OnMerge();
   afx_msg void OnRestore();
   afx_msg void OnSaveAs();
   afx_msg void OnListView();
   afx_msg void OnTreeView();
   afx_msg void OnBackupSafe();
   afx_msg void OnPasswordChange();
   afx_msg void OnClearClipboard();
   afx_msg void OnDelete();
   afx_msg void OnEdit();
   afx_msg void OnRename();
   afx_msg void OnFind();
   afx_msg void OnDuplicateEntry();
   afx_msg void OnOptions();
   afx_msg void OnSave();
   afx_msg void OnAdd();
   afx_msg void OnAddGroup();
   afx_msg void OnOK();
   afx_msg void OnOldToolbar();
   afx_msg void OnNewToolbar();
   afx_msg void OnExpandAll();
   afx_msg void OnCollapseAll();
   afx_msg void OnChangeFont();
   afx_msg void OnMinimize();
   afx_msg void OnUnMinimize();
   afx_msg void OnTimer(UINT nIDEvent);
   afx_msg void OnAutoType();
#if defined(POCKET_PC)
   afx_msg void OnShowPassword();
#else
   afx_msg void OnSetfocusItemlist( NMHDR * pNotifyStruct, LRESULT * result );
   afx_msg void OnKillfocusItemlist( NMHDR * pNotifyStruct, LRESULT * result );
   afx_msg void OnDropFiles(HDROP hDrop);
#endif
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateMRU(CCmdUI* pCmdUI);
    afx_msg void OnUpdateROCommand(CCmdUI *pCmdUI);
    afx_msg void OnUpdateTVCommand(CCmdUI *pCmdUI);
    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
    //}}AFX_MSG

    afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnExportV17();
    afx_msg void OnExportText();
    afx_msg void OnExportXML();
    afx_msg void OnImportText();
    afx_msg void OnImportKeePass();
    afx_msg void OnImportXML();

#if _MFC_VER > 1200
    afx_msg BOOL OnOpenMRU(UINT nID);
#else
    afx_msg void OnOpenMRU(UINT nID);
#endif

   DECLARE_MESSAGE_MAP()

  BOOL CheckExtension(const CMyString &name, const CMyString &ext) const;
  int GetAndCheckPassword(const CMyString &filename, CMyString& passkey,
			  int index = 1);
  // flag:
  //	0 first
  //	1 normal
  //  2 with Exit button

private:
  PWScore  &m_core;
  CMyString m_BrowseURL; // set by OnContextMenu(), used by OnBrowse()
  CMyString m_lastFindStr;
  BOOL m_lastFindCS;
  bool m_IsReadOnly;
  bool m_IsListView;
  HFONT m_hFontTree;
  LOGFONT m_treefont;
  CItemData *m_selectedAtMinimize; // to restore selection upon un-minimize
  BOOL IsWorkstationLocked();
  void startLockCheckTimer();
  void ExtractAutoTypeCmd(CMyString &str);
  bool m_LockDisabled; // set when a dialog box is open, to avoid confusion
  UINT m_IdleLockCountDown;
  void SetIdleLockCounter(UINT i) {m_IdleLockCountDown = i;}
  void ResetIdleLockCounter();
  bool DecrementAndTestIdleLockCounter();
  void ToClipboard(const CMyString &data);
  CMyString FromClipboard();
  void ExtractFont(CString& str);
  CString GetToken(CString& str, LPCTSTR c);
  int SaveIfChanged();
  afx_msg void OnGenerateresponse();
};

// Following used to keep track of display vs data
// stored as opaque data in CItemData.{Get,Set}DisplayInfo()
// Exposed here because MyTreeCtrl needs to update it after drag&drop
struct DisplayInfo {
  int list_index;
  HTREEITEM tree_item;
};


//-----------------------------------------------------------------------------
#endif // DboxMain_h
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
