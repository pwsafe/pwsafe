/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once

// DboxMain.h
//-----------------------------------------------------------------------------

#include "corelib/PWScore.h"
#include "corelib/sha256.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSClipboard.h"
#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
  #include "pocketpc/MyListCtrl.h"
#else
  #include "resource.h"
  #include "resource2.h"  // Version, Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif
#include "PWTreeCtrl.h"
#include "RUEList.h"
#include "MenuTipper.h"
#include "LVHdrCtrl.h"
#include "ColumnChooserDlg.h"
#include <vector>

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif

class CDDObList;

#if defined(POCKET_PC) || (_MFC_VER <= 1200)
DECLARE_HANDLE(HDROP);
#endif

// custom message event used for system tray handling.
#define WM_ICON_NOTIFY (WM_APP + 10)

// to catch post Header drag
#define WM_HDR_DRAG_COMPLETE (WM_APP + 20)
#define WM_CCTOHDR_DD_COMPLETE (WM_APP + 21)
#define WM_HDRTOCC_DD_COMPLETE (WM_APP + 22)

// Process Compare Result Dialog click/menu functions
#define WM_COMPARE_RESULT_FUNCTION (WM_APP + 30)

// External Editor
#define WM_CALL_EXTERNAL_EDITOR  (WM_APP + 40)
#define WM_EXTERNAL_EDITOR_ENDED (WM_APP + 41)

// timer event number used to check if the workstation is locked
#define TIMER_CHECKLOCK 0x04
// timer event number used to support lock on user-defined timeout
#define TIMER_USERLOCK 0x05

// Hotkey value ID
#define PWS_HOTKEY_ID 5767

// Index values for which dialog to show during GetAndCheckPassword
enum {GCP_FIRST = 0,		// At startup of PWS
	  GCP_NORMAL = 1,		// Only OK, CANCEL & HELP buttons
	  GCP_UNMINIMIZE = 2,	// Only OK, CANCEL & HELP buttons
	  GCP_WITHEXIT = 3,	// OK, CANCEL, EXIT & HELP buttons
	  GCP_ADVANCED = 4};	// OK, CANCEL, HELP buttons & ADVANCED checkbox

//-----------------------------------------------------------------------------
class DboxMain
   : public CDialog
{
#if defined(POCKET_PC)
  friend class CMyListCtrl;
#endif

  // static methods and variables
private:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  static CString CS_EDITENTRY, CS_VIEWENTRY, CS_EXPCOLGROUP;
  static CString CS_DELETEENTRY, CS_DELETEGROUP, CS_RENAMEENTRY, CS_RENAMEGROUP;
  static const CString DEFAULT_AUTOTYPE;

public:
  // default constructor
  DboxMain(CWnd* pParent = NULL);
  ~DboxMain();

  // Find entry by title and user name, exact match
  ItemListIter Find(const CMyString &a_group,
                    const CMyString &a_title, const CMyString &a_user)
  {return m_core.Find(a_group, a_title, a_user);}

  // Find entry with same title and user name as the
  // i'th entry in m_ctlItemList
  ItemListIter Find(int i);

  // Find entry by UUID
  ItemListIter Find(const uuid_array_t &uuid)
  {return m_core.Find(uuid);}

  // End of list markers
  ItemListConstIter End() const
  {return m_core.GetEntryEndIter();}
  ItemListIter End()
  {return m_core.GetEntryEndIter();}

  // FindAll is used by CFindDlg, returns # of finds.
  size_t FindAll(const CString &str, BOOL CaseSensitive,
              std::vector<int> &indices);
  size_t FindAll(const CString &str, BOOL CaseSensitive,
              std::vector<int> &indices,
              const CItemData::FieldBits &bsFields, const int subgroup_set, 
              const CString &subgroup_name, const int subgroup_object,
              const int subgroup_function);

  // Count the number of total entries.
  size_t GetNumEntries() const {return m_core.GetNumEntries();}

  // Get CItemData @ position
  CItemData &GetEntryAt(ItemListIter iter)
    {return m_core.GetEntry(iter);}

  // Set the section to the entry.  MakeVisible will scroll list, if needed.
  BOOL SelectEntry(int i, BOOL MakeVisible = FALSE);
  BOOL SelectFindEntry(int i, BOOL MakeVisible = FALSE);
  void RefreshList();

  int CheckPassword(const CMyString &filename, CMyString &passkey)
  {return m_core.CheckPassword(filename, passkey);}
  enum ChangeType {Clear, Data, TimeStamp};
  void SetChanged(ChangeType changed);

  // when Group, Title or User edited in tree
  void UpdateListItem(const int lindex, const int type, const CString &newText);
  void UpdateListItemGroup(const int lindex, const CString &newGroup)
  {UpdateListItem(lindex, CItemData::GROUP, newGroup);}
  void UpdateListItemTitle(const int lindex, const CString &newTitle)
  {UpdateListItem(lindex, CItemData::TITLE, newTitle);}
  void UpdateListItemUser(const int lindex, const CString &newUser)
  {UpdateListItem(lindex, CItemData::USER, newUser);}
  void UpdateListItemPassword(const int lindex, const CString &newPassword)
  {UpdateListItem(lindex, CItemData::PASSWORD, newPassword);}
  void SetHeaderInfo();
  CString GetHeaderText(const int iType);
  int GetHeaderWidth(const int iType);
  void CalcHeaderWidths();
  void UnFindItem();

  void UpdateToolBar(bool state);
  void UpdateToolBarForSelectedItem(CItemData *ci);
  bool IsMcoreReadOnly() const {return m_core.IsReadOnly();};
  void SetStartSilent(bool state);
  void SetStartClosed(bool state) {m_IsStartClosed = state;}
  void SetValidate(bool state) { m_bValidate = state;}
  bool MakeRandomPassword(CDialog * const pDialog, CMyString& password);
  BOOL LaunchBrowser(const CString &csURL);
  void SetFindActive() {m_bFindActive = true;}
  void SetFindInActive() {m_bFindActive = false;}
  bool GetCurrentView() {return m_IsListView;}
  void UpdatePasswordHistory(int iAction, int num_default);
  void SetInitialDatabaseDisplay();
  void U3ExitNow(); // called when U3AppStop sends message to Pwsafe Listener
  bool ExitRequested() const {return m_inExit;}
  void SetCapsLock(const bool bState);
  void AutoResizeColumns();
  void ResetIdleLockCounter();
  bool ClearClipboardData() {return m_clipboard.ClearData();}
  bool SetClipboardData(const CMyString &data)
  {return m_clipboard.SetData(data);}
  void AddEntries(CDDObList &in_oblist, const CMyString &DropGroup);
  int AddEntry(const CItemData &cinew);
  CMyString GetUniqueTitle(const CMyString &path, const CMyString &title,
                           const CMyString &user, const int IDS_MESSAGE)
  {return m_core.GetUniqueTitle(path, title, user, IDS_MESSAGE);}
  void FixListIndexes();
  void Delete(bool inRecursion = false);
  void SaveDisplayStatus(); // call when tree expansion state changes
  int GetBaseEntry(CMyString &Password, uuid_array_t &base_uuid, bool &bBase_was_Alias,
      CMyString &csPwdGroup, CMyString &csPwdTitle, CMyString &csPwdUser)
  {return m_core.GetBaseEntry(Password, base_uuid, bBase_was_Alias,
                              csPwdGroup, csPwdTitle, csPwdUser);}
  void GetBaseUUID(const uuid_array_t &alias_uuid, uuid_array_t &base_uuid)
  {m_core.GetBaseUUID(alias_uuid, base_uuid);}

  //{{AFX_DATA(DboxMain)
  enum { IDD = IDD_PASSWORDSAFE_DIALOG };
#if defined(POCKET_PC)
  CMyListCtrl m_ctlItemList;
#else
  CListCtrl m_ctlItemList;
#endif
  CPWTreeCtrl  m_ctlItemTree;
  CLVHdrCtrl m_LVHdrCtrl;
  CColumnChooserDlg *m_pCC;
  CPoint m_RCMousePos;
  //}}AFX_DATA

  CRUEList m_RUEList;   // recent entry lists

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
  bool  m_bOpen;
  bool m_bValidate; // do validation after reading db

#if !defined(POCKET_PC)
  CMyString m_titlebar; // what's displayed in the title bar
#endif

#if defined(POCKET_PC)
  CCeCommandBar	*m_wndCommandBar;
  CMenu			*m_wndMenu;
#else
  CToolBar m_wndToolBar;
  CStatusBar m_statusBar;
  BOOL m_toolbarsSetup;
  UINT m_toolbarMode;
  enum {SB_DBLCLICK = 0, SB_CONFIG, SB_MODIFIED, SB_READONLY, SB_NUM_ENT,
        SB_TOTAL /* this must be the last entry */};
  UINT statustext[SB_TOTAL];
#endif

  bool m_windowok;
  bool m_needsreading;
  bool m_passphraseOK;

  bool m_bSortAscending;
  int m_iTypeSortColumn;

  bool m_bTSUpdated;
  int m_iSessionEndingStatus;
  bool m_bFindActive;

  // Used for Advanced functions
  CItemData::FieldBits m_bsFields;
  bool m_bAdvanced;
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function;

  HTREEITEM m_LastFoundItem;
  bool m_bBoldItem;

  WCHAR *m_pwchTip;
  TCHAR *m_pchTip;

  CMyString m_TreeViewGroup; // used by OnAdd & OnAddGroup
  CMenuTipManager m_menuTipManager;

  int insertItem(CItemData &itemData, int iIndex = -1, bool bSort = true);
  CItemData *getSelectedItem();

  void ChangeOkUpdate();
  BOOL SelItemOk();
  void setupBars();
  BOOL OpenOnInit();
  void InitPasswordSafe();
  // override following to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  void ConfigureSystemMenu();
  afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
  LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
  LRESULT OnCCToHdrDragComplete(WPARAM wParam, LPARAM lParam);
  LRESULT OnHdrToCCDragComplete(WPARAM wParam, LPARAM lParam);
  LRESULT OnHeaderDragComplete(WPARAM wParam, LPARAM lParam);
  enum STATE {LOCKED, UNLOCKED, CLOSED};  // Really shouldn't be here it, ThisMfcApp own it
  void UpdateSystemTray(const STATE s);
  LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

  LRESULT OnProcessCompareResultFunction(WPARAM wParam, LPARAM lParam);
  LRESULT ViewCompareResult(PWScore *pcore, uuid_array_t &uuid);
  LRESULT EditCompareResult(PWScore *pcore, uuid_array_t &uuid);
  LRESULT CopyCompareResult(PWScore *pfromcore, PWScore *ptocore,
                            uuid_array_t &fromuuid, uuid_array_t &touuid);

  BOOL PreTranslateMessage(MSG* pMsg);

  void UpdateAlwaysOnTop();
  void ClearData(bool clearMRE = true);
  int NewFile(CMyString &filename);

  void SetListView();
  void SetTreeView();
  void SetToolbar(int menuItem);
  void UpdateStatusBar();
  void UpdateMenuAndToolBar(const bool bOpen);
  void SetDCAText();
  void SortListView();

  //Version of message functions with return values
  int Save(void);
  int SaveAs(void);
  int SaveCore(PWScore *pcore);
  int Open(void);
  int Open( const CMyString &pszFilename );
  int Close(void);
  int Merge(void);
  int Merge( const CMyString &pszFilename );
  int Compare(const CMyString &cs_Filename1, const CMyString &cs_Filename2);

  int BackupSafe(void);
  int New(void);
  int Restore(void);

  void AutoType(const CItemData &ci);
  bool EditItem(CItemData *ci, PWScore *pcore = NULL);
  void SortAliasEntries(UUIDList &aliaslist, CMyString &csAliases);
  void ViewReport(const CString cs_ReportFileName);

#if !defined(POCKET_PC)
	afx_msg void OnTrayLockUnLock();
  afx_msg void OnUpdateTrayLockUnLockCommand(CCmdUI *pCmdUI);
  afx_msg void OnTrayClearRecentEntries();
  afx_msg void OnUpdateTrayClearRecentEntries(CCmdUI *pCmdUI);
	afx_msg void OnTrayCopyUsername(UINT nID);
	afx_msg void OnUpdateTrayCopyUsername(CCmdUI *pCmdUI);
	afx_msg void OnTrayCopyPassword(UINT nID);
	afx_msg void OnUpdateTrayCopyPassword(CCmdUI *pCmdUI);
	afx_msg void OnTrayCopyNotes(UINT nID);
	afx_msg void OnUpdateTrayCopyNotes(CCmdUI *pCmdUI);
	afx_msg void OnTrayBrowse(UINT nID);
	afx_msg void OnUpdateTrayBrowse(CCmdUI *pCmdUI);
	afx_msg void OnTrayDeleteEntry(UINT nID);
	afx_msg void OnUpdateTrayDeleteEntry(CCmdUI *pCmdUI);
	afx_msg void OnTrayAutoType(UINT nID);
	afx_msg void OnUpdateTrayAutoType(CCmdUI *pCmdUI);
#endif

  // Generated message map functions
  //{{AFX_MSG(DboxMain)
  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  afx_msg BOOL OnQueryEndSession();
  afx_msg void OnEndSession(BOOL bEnding);
  afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
  afx_msg void OnMove(int x, int y);
  virtual void OnCancel();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnAbout();
  afx_msg void OnU3ShopWebsite();
  afx_msg void OnPasswordSafeWebsite();
  afx_msg void OnBrowse();
  afx_msg void OnCopyUsername();
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnListItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnKeydownItemlist(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemDoubleClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderRClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderNotify(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderEndDrag(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnCopyPassword();
  afx_msg void OnCopyNotes();
  afx_msg void OnCopyURL();
  afx_msg void OnNew();
  afx_msg void OnOpen();
  afx_msg void OnClose();
  afx_msg void OnClearMRU();
  afx_msg void OnMerge();
  afx_msg void OnCompare();
  afx_msg void OnProperties();
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
  afx_msg void OnValidate();
  afx_msg void OnSave();
  afx_msg void OnAdd();
  afx_msg void OnAddGroup();
  afx_msg void OnOK();
  afx_msg void OnOldToolbar();
  afx_msg void OnNewToolbar();
  afx_msg void OnExpandAll();
  afx_msg void OnCollapseAll();
  afx_msg void OnChangeFont();
  afx_msg void OnViewReports(UINT nID);
  afx_msg void OnUpdateViewReports(CCmdUI *pCmdUI);
  afx_msg void OnMinimize();
  afx_msg void OnUnMinimize();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnAutoType();
  afx_msg void OnColumnPicker();
  afx_msg void OnResetColumns();
#if defined(POCKET_PC)
  afx_msg void OnShowPassword();
#else
  afx_msg void OnChangeItemFocus( NMHDR * pNotifyStruct, LRESULT * result );
  afx_msg void OnDropFiles(HDROP hDrop);
#endif
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnUpdateMRU(CCmdUI* pCmdUI);
  afx_msg void OnUpdateROCommand(CCmdUI *pCmdUI);
  afx_msg void OnUpdateClosedCommand(CCmdUI *pCmdUI);
  afx_msg void OnUpdateTVCommand(CCmdUI *pCmdUI);
  afx_msg void OnUpdateViewCommand(CCmdUI *pCmdUI);
  afx_msg void OnUpdateNSCommand(CCmdUI *pCmdUI);  // Make entry unsupported (grayed out)
  afx_msg void OnInitMenu(CMenu* pMenu);
  afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
  afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
  //}}AFX_MSG

  afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnExportVx(UINT nID);
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

  int GetAndCheckPassword(const CMyString &filename, CMyString& passkey,
                          int index, bool bReadOnly = false, bool bForceReadOnly = false,
                          PWScore *pcore = 0, int adv_type = -1);

private:
  CMyString m_BrowseURL; // set by OnContextMenu(), used by OnBrowse()
  PWScore &m_core;
  CMyString m_lastFindStr;
  BOOL m_lastFindCS;
  bool m_IsStartSilent;
  bool m_IsStartClosed;
  bool m_bStartHiddenAndMinimized;
  bool m_IsListView;
  bool m_bAlreadyToldUserNoSave;
  bool m_bPasswordColumnShowing;
  int m_iDateTimeFieldWidth;
  int m_nColumns;
  int m_nColumnIndexByOrder[CItemData::LAST];
  int m_nColumnIndexByType[CItemData::LAST];
  int m_nColumnTypeByIndex[CItemData::LAST];
  int m_nColumnWidthByIndex[CItemData::LAST];
  int m_nColumnHeaderWidthByType[CItemData::LAST];
  int m_iheadermaxwidth;
  CFont *m_pFontTree;
  CItemData *m_selectedAtMinimize; // to restore selection upon un-minimize
  bool m_inExit; // help U3ExitNow

  PWSclipboard m_clipboard;

  BOOL IsWorkstationLocked() const;
  void startLockCheckTimer();
  UINT m_IdleLockCountDown;
  void SetIdleLockCounter(UINT i) {m_IdleLockCountDown = i;}
  bool DecrementAndTestIdleLockCounter();
  void ExtractFont(CString& str, LOGFONT *ptreefont);
  int SaveIfChanged();
  void CheckExpiredPasswords();
  void UnMinimize(bool update_windows);
  void UpdateAccessTime(CItemData *ci);
  void RestoreDisplayStatus();
  std::vector<bool> GetGroupDisplayStatus(); // get current display state from window
  void SetGroupDisplayStatus(const std::vector<bool> &displaystatus); // changes display
  void MakeOrderedItemList(OrderedItemList &il);
  int CountChildren(HTREEITEM hStartItem);
  void SetColumns();  // default order
  void SetColumns(const CString cs_ListColumns);
  void SetColumnWidths(const CString cs_ListColumnsWidths);
  void SetupColumnChooser(const bool bShowHide);
  void AddColumn(const int iType, const int iIndex);
  void DeleteColumn(const int iType);
};

// Following used to keep track of display vs data
// stored as opaque data in CItemData.{Get,Set}DisplayInfo()
// Exposed here because PWTreeCtrl needs to update it after drag&drop
struct DisplayInfo {
  int list_index;
  HTREEITEM tree_item;
};


//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
