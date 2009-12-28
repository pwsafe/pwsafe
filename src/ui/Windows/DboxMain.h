/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// DboxMain.h
//-----------------------------------------------------------------------------

#include "PWSclipboard.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#include "pocketpc/MyListCtrl.h"
#else
#include "resource.h"
#include "resource2.h"  // Version, Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources
#endif

#include "PWTreeCtrl.h"
#include "PWListCtrl.h"
#include "RUEList.h"
#include "CoolMenu.h"
#include "MenuTipper.h"
#include "LVHdrCtrl.h"
#include "ColumnChooserDlg.h"
#include "PWStatusBar.h"
#include "PWToolBar.h"
#include "PWFindToolBar.h"
#include "ControlExtns.h"
#include "DDStatic.h"
#include "MenuShortcuts.h"
#include "WinGUICmdIF.h"

#include "corelib/UIinterface.h"
#include "corelib/PWScore.h"
#include "corelib/StringX.h"
#include "corelib/sha256.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSFilters.h"
#include "corelib/Command.h"

#include "os/run.h"

#include <vector>
#include <map>
#include <list>
#include <stack>

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP             0x0400
#define HDF_SORTDOWN           0x0200

#define WM_WTSSESSION_CHANGE   0x02B1

#define WTS_CONSOLE_CONNECT                0x1
#define WTS_CONSOLE_DISCONNECT             0x2
#define WTS_REMOTE_CONNECT                 0x3
#define WTS_REMOTE_DISCONNECT              0x4
#define WTS_SESSION_LOGON                  0x5
#define WTS_SESSION_LOGOFF                 0x6
#define WTS_SESSION_LOCK                   0x7
#define WTS_SESSION_UNLOCK                 0x8
#define WTS_SESSION_REMOTE_CONTROL         0x9

#endif  /* WINVER < 0x0501 */

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

// Edit/Add extra context menu messages
#define WM_CALL_EXTERNAL_EDITOR  (WM_APP + 40)
#define WM_EXTERNAL_EDITOR_ENDED (WM_APP + 41)
#define WM_EDIT_WORDWRAP         (WM_APP + 42)
#define WM_EDIT_SHOWNOTES        (WM_APP + 43)

// Simulate Ctrl+F from Find Toolbar "enter"
#define WM_TOOLBAR_FIND (WM_APP + 50)

// Update current filters whilst SetFilters dialog is open
#define WM_EXECUTE_FILTERS (WM_APP + 60)

/* timer event number used to by PupText.  Here for doc. only
#define TIMER_PUPTEXT             0x03  */
// timer event number used to check if the workstation is locked
#define TIMER_LOCKONWTSLOCK       0x04
// timer event number used to support lock on user-defined idle timeout
#define TIMER_LOCKDBONIDLETIMEOUT 0x05
// Definition of a minute in milliseconds
#define MINUTE 60000
// How ofter should idle timeout timer check:
#define IDLE_CHECK_RATE 2
#define IDLE_CHECK_INTERVAL (MINUTE/IDLE_CHECK_RATE)
// timer event number used to support Find in PWListCtrl when icons visible
#define TIMER_FIND                0x06
// timer event number used to support display of notes in List & Tree controls
#define TIMER_ND_HOVER            0x07
#define TIMER_ND_SHOWING          0x08
// timer event number used to support DragBar
#define TIMER_DRAGBAR             0x09
/* timer event numbers used to by ControlExtns for ListBox tooltips.  Here for doc. only
#define TIMER_LB_HOVER            0x0A
#define TIMER_LB_SHOWING          0x0B */

/*
HOVER_TIME_ND       The length of time the pointer must remain stationary
                    within a tool's bounding rectangle before the tool tip
                    window appears.

TIMEINT_ND_SHOWING The length of time the tool tip window remains visible
                   if the pointer is stationary within a tool's bounding
                   rectangle.
*/
#define HOVER_TIME_ND      2000
#define TIMEINT_ND_SHOWING 5000

// DragBar time interval 
#define TIMER_DRAGBAR_TIME 100

// Hotkey value ID
#define PWS_HOTKEY_ID 5767

// Arbitrary string to mean that the saved DB preferences are empty.
#define EMPTYSAVEDDBPREFS L"#Empty#"

// For ShutdownBlockReasonCreate & ShutdownBlockReasonDestroy
typedef BOOL (WINAPI *PSBR_CREATE) (HWND, LPCWSTR);
typedef BOOL (WINAPI *PSBR_DESTROY) (HWND);

enum SearchDirection {FIND_UP = -1, FIND_DOWN = 1};

// List of all Popup menus in the Main Menu
enum PopupMenus {FILEMENU = 0, EXPORTMENU, IMPORTMENU, 
                 EDITMENU, VIEWMENU, FILTERMENU, 
                 CHANGEFONTMENU, REPORTSMENU, MANAGEMENU, 
                 HELPMENU, NUMPOPUPMENUS};

// Index values for which dialog to show during GetAndCheckPassword
enum {GCP_FIRST      = 0,   // At startup of PWS
      GCP_NORMAL     = 1,   // Only OK, CANCEL & HELP buttons
      GCP_RESTORE    = 2,   // Only OK, CANCEL & HELP buttons
      GCP_WITHEXIT   = 3,   // OK, CANCEL, EXIT & HELP buttons
      GCP_ADVANCED   = 4};  // OK, CANCEL, HELP buttons & ADVANCED checkbox

//-----------------------------------------------------------------------------
class DboxMain : public CDialog, public UIinterface
{
#if defined(POCKET_PC)
  friend class CMyListCtrl;
#endif

public:
  DECLARE_DYNAMIC(DboxMain)

  // default constructor
  DboxMain(CWnd* pParent = NULL);
  ~DboxMain();

  // Find entry by title and user name, exact match
  ItemListIter Find(const StringX &a_group,
                    const StringX &a_title, const StringX &a_user)
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

  // FindAll is used by CPWFindToolBar, returns # of finds.
  size_t FindAll(const CString &str, BOOL CaseSensitive,
                 std::vector<int> &indices);
  size_t FindAll(const CString &str, BOOL CaseSensitive,
                 std::vector<int> &indices,
                 const CItemData::FieldBits &bsFields, const int subgroup_set, 
                 const CString &subgroup_name, const int subgroup_object,
                 const int subgroup_function);

  // Used by ListCtrl KeyDown
  bool IsImageVisible() {return m_bImageInLV;}

  // Count the number of total entries.
  size_t GetNumEntries() const {return m_core.GetNumEntries();}

  // Get CItemData @ position
  CItemData &GetEntryAt(ItemListIter iter)
  {return m_core.GetEntry(iter);}

  // Set the section to the entry.  MakeVisible will scroll list, if needed.
  BOOL SelectEntry(int i, BOOL MakeVisible = FALSE);
  BOOL SelectFindEntry(int i, BOOL MakeVisible = FALSE);
  void SelectFirstEntry();

  // For InsertItemIntoGUITreeList and RefreshViews (mainly when refreshing views)
  // Note: iBothViews = iListOnly + iTreeOnly
  enum {iListOnly = 1, iTreeOnly = 2, iBothViews = 3};
  void RefreshViews(const int iView = iBothViews);

  int CheckPasskey(const StringX &filename, const StringX &passkey)
  {return m_core.CheckPasskey(filename, passkey);}
  enum ChangeType {Clear, Data, TimeStamp, DBPrefs, ClearDBPrefs};
  void SetChanged(ChangeType changed);
  void ChangeOkUpdate();

  // when Group, Title or User edited in tree
  void UpdateListItem(const int lindex, const int type, const StringX &newText);
  void UpdateTreeItem(const HTREEITEM hItem, const StringX &newText);
  void UpdateListItemGroup(const int lindex, const StringX &newGroup)
  {UpdateListItem(lindex, CItemData::GROUP, newGroup);}
  void UpdateListItemTitle(const int lindex, const StringX &newTitle)
  {UpdateListItem(lindex, CItemData::TITLE, newTitle);}
  void UpdateListItemUser(const int lindex, const StringX &newUser)
  {UpdateListItem(lindex, CItemData::USER, newUser);}
  void UpdateListItemPassword(const int lindex, const StringX &newPassword)
  {UpdateListItem(lindex, CItemData::PASSWORD, newPassword);}
  void SetHeaderInfo();
  CString GetHeaderText(const int iType);
  int GetHeaderWidth(const int iType);
  void CalcHeaderWidths();
  void UnFindItem();
  void SetLocalStrings();

  void UpdateToolBarROStatus(bool state);
  void UpdateToolBarForSelectedItem(CItemData *pci);
  void SetToolBarPositions();
  void InvalidateSearch() {m_FindToolBar.InvalidateSearch();}
  void ResumeOnDBNotification() {m_core.ResumeOnDBNotification();}
  void SuspendOnDBNotification() {m_core.SuspendOnDBNotification();}
  bool IsMcoreReadOnly() const {return m_core.IsReadOnly();};
  void SetStartSilent(bool state);
  void SetStartClosed(bool state) {m_IsStartClosed = state;}
  void SetValidate(bool state) {m_bValidate = state;}
  void MakeRandomPassword(StringX& password, PWPolicy &pwp, 
                          bool bIssueMsg = false);
  BOOL LaunchBrowser(const CString &csURL, const StringX &sxAutotype,
                     const bool bDoAutotype);
  BOOL SendEmail(const CString &cs_email);
  void SetInitialDatabaseDisplay();
  void U3ExitNow(); // called when U3AppStop sends message to Pwsafe Listener
  bool ExitRequested() const {return m_inExit;}
  void SetCapsLock(const bool bState);
  void AutoResizeColumns();
  void ResetIdleLockCounter(UINT event = WM_SIZE); // default arg always resets
  bool ClearClipboardData() {return m_clipboard.ClearData();}
  bool SetClipboardData(const StringX &data)
  {return m_clipboard.SetData(data.c_str());}
  void AddEntries(CDDObList &in_oblist, const StringX &DropGroup);
  StringX GetUniqueTitle(const StringX &path, const StringX &title,
                           const StringX &user, const int IDS_MESSAGE)
  {return m_core.GetUniqueTitle(path, title, user, IDS_MESSAGE);}
  void FixListIndexes();
  void Delete(MultiCommands *pmulticmds, WinGUICmdIF *pGUICmdIF, bool inRecursion = false);
  void SaveGroupDisplayState(); // call when tree expansion state changes
  bool CheckNewPassword(const StringX &group, const StringX &title,
                        const StringX &user, const StringX &password,
                        const bool bIsEdit, const CItemData::EntryType &InputType, 
                        uuid_array_t &base_uuid, int &ibasedata, bool &b_msg_issued);
  void GetAliasBaseUUID(const uuid_array_t &entry_uuid, uuid_array_t &base_uuid)
  {m_core.GetAliasBaseUUID(entry_uuid, base_uuid);}
  void GetShortcutBaseUUID(const uuid_array_t &entry_uuid, uuid_array_t &base_uuid)
  {m_core.GetShortcutBaseUUID(entry_uuid, base_uuid);}

  int GetEntryImage(const CItemData &ci);
  HICON GetEntryIcon(const int nImage) const;
  void SetEntryImage(const int &index, const int nImage, const bool bOneEntry = false);
  void SetEntryImage(HTREEITEM &ti, const int nImage, const bool bOneEntry = false);

  void RefreshImages();
  bool FieldsNotEqual(StringX a, StringX b);
  void CreateShortcutEntry(CItemData *pci, const StringX &cs_group,
                           const StringX &cs_title, const StringX &cs_user);
  bool SetNotesWindow(const CPoint point, const bool bVisible = true);
  bool IsFilterActive() {return m_bFilterActive;}
  int GetNumPassedFiltering() {return m_bNumPassedFiltering;}
  CItemData *GetLastSelected();
  StringX GetGroupName();
  void UpdateGroupNamesInMap(const StringX sxOldPath, const StringX sxNewPath);

  void SetFilter(FilterPool selectedpool, CString selectedfiltername)
  {m_currentfilterpool = selectedpool; m_selectedfiltername = selectedfiltername;}
  void ImportFilters();
  bool ApplyFilter(bool bJustDoIt = false);
  bool EditFilter(st_filters *pfilters, const bool &bAllowSet = true);
  void ClearFilter();
  void ExportFilters(PWSFilters &MapFilters);

  void DoAutoType(const StringX &sxAutotype, const StringX &group = L"", 
                  const StringX &title = L"", const StringX &user = L"", 
                  const StringX &pwd = L"", const StringX &notes = L"");
  void UpdateLastClipboardAction(const int iaction);
  void PlaceWindow(CWnd *pwnd, CRect *prect, UINT showCmd);
  void SetDCAText(CItemData * pci = NULL);
  void OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  bool IsNodeModified(StringX &path)
  {return m_core.IsNodeModified(path);}

  GUICommand * CreateGUICommand(WinGUICmdIF *pGUICmdIF, PWScore *pcore = NULL);
  MultiCommands * CreateMultiCommands(PWScore *pcore = NULL);
  void Execute(Command *c, PWScore *pcore = NULL);
  void UpdateToolBarDoUndo();

  //{{AFX_DATA(DboxMain)
  enum { IDD = IDD_PASSWORDSAFE_DIALOG };
#if defined(POCKET_PC)
  CMyListCtrl m_ctlItemList;
#else
  CPWListCtrl m_ctlItemList;
#endif
  CPWTreeCtrl m_ctlItemTree;
  CImageList *m_pImageList;
  CImageList *m_pImageList0;
  CLVHdrCtrl m_LVHdrCtrl;
  CColumnChooserDlg *m_pCC;
  CPoint m_RCMousePos;
  CDDStatic m_DDGroup, m_DDTitle, m_DDUser, m_DDPassword, m_DDNotes, m_DDURL, m_DDemail;
  //}}AFX_DATA

  CRUEList m_RUEList;   // recent entry lists

  wchar_t *m_eye_catcher;
  bool m_bInAddGroup;

  bool m_bDoAutoType;
  StringX m_AutoType;

  // Used in Add & Edit & OptionsPasswordPolicy
  PWPolicy m_pwp;

  // Mapping Group to Tree Item to save searching all the time!
  std::map<StringX, HTREEITEM> m_mapGroupToTreeItem;
  
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(DboxMain)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
  //}}AFX_VIRTUAL

  HICON m_hIcon;
  HICON m_hIconSm;

  // used to speed up the resizable dialog so OnSize/SIZE_RESTORED isn't called
  bool m_bSizing;
  bool m_bIsRestoring;
  bool m_bOpen;
  bool m_bValidate; // do validation after reading db

#if !defined(POCKET_PC)
  CString m_titlebar; // what's displayed in the title bar
#endif

#if defined(POCKET_PC)
  CCeCommandBar *m_wndCommandBar;
  CMenu *m_wndMenu;
#else
  CPWToolBar m_MainToolBar;   // main toolbar
  CPWFindToolBar m_FindToolBar;  // Find toolbar
  CPWStatusBar m_statusBar;
  BOOL m_toolbarsSetup;
  UINT m_toolbarMode;
  UINT statustext[CPWStatusBar::SB_TOTAL];
  CString m_lastclipboardaction;
#endif

  bool m_bInitDone;
  bool m_needsreading;

  bool m_bSortAscending;
  int m_iTypeSortColumn;

  bool m_bTSUpdated;
  int m_iSessionEndingStatus;

  // Used for Advanced functions
  CItemData::FieldBits m_bsFields;
  bool m_bAdvanced;
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function;
  int m_treatwhitespaceasempty;

  HTREEITEM m_LastFoundTreeItem;
  int m_LastFoundListItem;
  bool m_bBoldItem;

  WCHAR *m_pwchTip;
  char *m_pchTip;

  StringX m_TreeViewGroup; // used by OnAdd & OnAddGroup
  CCoolMenuManager m_menuManager;
  CMenuTipManager m_menuTipManager;

  int InsertItemIntoGUITreeList(CItemData &itemData, int iIndex = -1, 
                 const bool bSort = true, const int iView = iBothViews);
  CItemData *getSelectedItem();

  BOOL SelItemOk();
  void setupBars();
  BOOL OpenOnInit();
  void InitPasswordSafe();

  // For UPDATE_UI
  int OnUpdateMenuToolbar(const UINT nID);
  int OnUpdateViewReports(const int nID);
  void OnUpdateMRU(CCmdUI* pCmdUI);

  // override following to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  void ConfigureSystemMenu();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
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
  LRESULT OnToolBarFindMessage(WPARAM wParam, LPARAM lParam);
  LRESULT OnExecuteFilters(WPARAM wParam, LPARAM lParam);

  BOOL PreTranslateMessage(MSG* pMsg);

  void UpdateAlwaysOnTop();
  void ClearData(bool clearMRE = true);
  int NewFile(StringX &filename);

  void SetListView();
  void SetTreeView();
  void SetToolbar(const int menuItem, bool bInit = false);
  void UpdateStatusBar();
  void UpdateMenuAndToolBar(const bool bOpen);
  void SortListView();

  //Version of message functions with return values
  int Save(void);
  int SaveAs(void);
  int SaveCore(PWScore *pcore);
  int Open(void);
  int Open(const StringX &pszFilename, const bool bReadOnly);
  int Close(void);

  void DoOtherDBProcessing(UINT uiftn);
  int Merge(const StringX &sx_Filename2);
  int MergeDependents(PWScore *pothercore, MultiCommands *pmulticmds,
                      uuid_array_t &base_uuid, uuid_array_t &new_base_uuid, 
                      const bool bTitleRenamed, CString &timeStr, 
                      const CItemData::EntryType et, std::vector<StringX> &vs_added);
  int Compare(const StringX &sx_Filename1, const StringX &sx_Filename2);
  int Synchronize(const StringX &sx_Filename2);
  void ReportAdvancedOptions(CReport *prpt, const UINT uimsgftn);

  int BackupSafe(void);
  int RestoreSafe(void);
  int New(void);

  void AutoType(const CItemData &ci);
  bool EditItem(CItemData *pci, PWScore *pcore = NULL);
  bool EditShortcut(CItemData *pci, PWScore *pcore = NULL);
  void SortDependents(UUIDList &dlist, StringX &csDependents);
  void ViewReport(const CString &cs_ReportFileName);
  void ViewReport(CReport &rpt);
  bool GetDriveAndDirectory(const StringX &cs_infile, CString &cs_directory,
                            CString &cs_drive);
  void SetFindToolBar(bool bShow);
  void ApplyFilters();

  HRGN GetWorkAreaRegion();
  void GetMonitorRect(HWND hwnd, RECT *prc, BOOL fWork);
  void ClipRectToMonitor(HWND hwnd, RECT *prc, BOOL fWork);
  static BOOL CALLBACK EnumScreens(HMONITOR hMonitor, HDC hdc, LPRECT prc, LPARAM lParam);
  bool PassesFiltering(CItemData &ci, const st_filters &filters);
  bool PassesPWHFiltering(CItemData *pci, const st_filters &filters);
  bool PassesPWPFiltering(CItemData *pci, const st_filters &filters);

  void SaveGUIStatus();
  void RestoreGUIStatus();

#if !defined(POCKET_PC)
  afx_msg void OnTrayLockUnLock();
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
  afx_msg void OnTrayCopyURL(UINT nID);
  afx_msg void OnUpdateTrayCopyURL(CCmdUI *pCmdUI);
  afx_msg void OnTrayRunCommand(UINT nID);
  afx_msg void OnUpdateTrayRunCommand(CCmdUI *pCmdUI);
  afx_msg void OnTrayCopyEmail(UINT nID);
  afx_msg void OnUpdateTrayCopyEmail(CCmdUI *pCmdUI);
  afx_msg void OnTraySendEmail(UINT nID);
  afx_msg void OnUpdateTraySendEmail(CCmdUI *pCmdUI);
#endif

  // Generated message map functions
  //{{AFX_MSG(DboxMain)
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  
  afx_msg LRESULT OnAreYouMe(WPARAM, LPARAM);
  afx_msg LRESULT OnWH_SHELL_CallBack(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnSessionChange(WPARAM wParam, LPARAM lParam);

  afx_msg void OnUpdateMenuToolbar(CCmdUI *pCmdUI);
  afx_msg void OnDestroy();
  afx_msg BOOL OnQueryEndSession();
  afx_msg void OnEndSession(BOOL bEnding);
  afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
  afx_msg void OnMove(int x, int y);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnAbout();
  afx_msg void OnU3ShopWebsite();
  afx_msg void OnPasswordSafeWebsite();
  afx_msg void OnBrowse();
  afx_msg void OnBrowsePlus();
  afx_msg void OnSendEmail();
  afx_msg void OnCopyUsername();
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnListItemSelected(NMHDR *pNMHDR, LRESULT *pLResult);
  afx_msg void OnTreeItemSelected(NMHDR *pNMHDR, LRESULT *pLResult);
  afx_msg void OnKeydownItemlist(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnItemDoubleClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderRClick(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderNotify(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderBeginDrag(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnHeaderEndDrag(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnCopyPassword();
  afx_msg void OnCopyPasswordMinimize();
  afx_msg void OnDisplayPswdSubset();
  afx_msg void OnCopyNotes();
  afx_msg void OnCopyURL();
  afx_msg void OnCopyEmail();
  afx_msg void OnCopyRunCommand();
  afx_msg void OnNew();
  afx_msg void OnOpen();
  afx_msg void OnClose();
  afx_msg void OnClearMRU();
  afx_msg void OnMerge();
  afx_msg void OnCompare();
  afx_msg void OnSynchronize();
  afx_msg void OnProperties();
  afx_msg void OnRestoreSafe();
  afx_msg void OnSaveAs();
  afx_msg void OnToggleView();
  afx_msg void OnListView();
  afx_msg void OnTreeView();
  afx_msg void OnBackupSafe();
  afx_msg void OnPasswordChange();
  afx_msg void OnClearClipboard();
  afx_msg void OnDelete();
  afx_msg void OnEdit();
  afx_msg void OnRename();
  afx_msg void OnDuplicateEntry();
  afx_msg void OnOptions();
  afx_msg void OnValidate();
  afx_msg void OnGeneratePassword();
  afx_msg void OnSave();
  afx_msg void OnAdd();
  afx_msg void OnAddGroup();
  afx_msg void OnCreateShortcut();
  afx_msg void OnOK();
  afx_msg void OnShowHideToolbar();
  afx_msg void OnShowHideDragbar();
  afx_msg void OnOldToolbar();
  afx_msg void OnNewToolbar();
  afx_msg void OnShowFindToolbar();
  afx_msg void OnExpandAll();
  afx_msg void OnCollapseAll();
  afx_msg void OnChangeTreeFont();
  afx_msg void OnChangePswdFont();
  afx_msg void OnChangeVKFont();
  afx_msg void OnViewReports(UINT nID);  // From View->Reports menu
  afx_msg void OnViewReports();
  afx_msg void OnManageFilters(); // From Toolbar button
  afx_msg void OnApplyFilter();
  afx_msg void OnSetFilter();
  afx_msg void OnRefreshWindow();
  afx_msg void OnShowUnsavedEntries();
  afx_msg void OnMinimize();
  afx_msg void OnRestore();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnAutoType();
  afx_msg void OnGotoBaseEntry();
  afx_msg void OnEditBaseEntry();
  afx_msg void OnUndo();
  afx_msg void OnRedo();
  afx_msg void OnRunCommand();
  afx_msg void OnColumnPicker();
  afx_msg void OnResetColumns();
#if defined(POCKET_PC)
  afx_msg void OnShowPassword();
#else
  afx_msg void OnDropFiles(HDROP hDrop);
#endif
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnUpdateNSCommand(CCmdUI *pCmdUI);  // Make entry unsupported (grayed out)
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

  afx_msg void OnToolBarFind();
  afx_msg void OnToolBarFindUp();
  afx_msg void OnCustomizeToolbar();
  afx_msg void OnToolBarFindCase();
  afx_msg void OnToolBarFindAdvanced();
  afx_msg void OnToolBarFindReport();
  afx_msg void OnToolBarClearFind();
  afx_msg void OnHideFindToolBar();

  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

#if _MFC_VER > 1200
  afx_msg BOOL OnOpenMRU(UINT nID);
#else
  afx_msg void OnOpenMRU(UINT nID);
#endif

  DECLARE_MESSAGE_MAP()

  int GetAndCheckPassword(const StringX &filename, StringX& passkey,
                          int index, bool bReadOnly = false, bool bForceReadOnly = false,
                          PWScore *pcore = 0, int adv_type = -1);

private:
  // UIinterface implementations:
  void DatabaseModified(bool bChanged);
  void UpdateGUI(Command::GUI_Action ga,
                 uuid_array_t &entry_uuid,
                 CItemData::FieldType ft);
  void GUIUpdateEntry(CItemData &ci);
  
  // static methods and variables
  static void GUICommandInterface(LPARAM instance, const Command::ExecuteFn &when, PWSGUICmdIF *pGUICmdIF);
  void RedoDelete(WinGUICmdIF *pGUICmdIF);

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  // Only need these if not on default menus
  static CString CS_SETFILTERS, CS_CLEARFILTERS;

  StringX m_BrowseURL; // set by OnContextMenu(), used by OnBrowse()
  PWScore &m_core;
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
  std::vector<bool> m_grpdispstate;  // used to save/restore display state over minimize/restore
  StringX m_savedDBprefs;  // used across minimize/restore events

  PWSclipboard m_clipboard;

  void SessionNotification(const bool bRegister);
  bool IsWorkstationLocked() const;
  bool LockDataBase();
  void startLockCheckTimer();
  UINT m_IdleLockCountDown;
  void SetIdleLockCounter(UINT i); // i in minutes, set to timer counts
  bool DecrementAndTestIdleLockCounter();
  int SaveIfChanged();
  void CheckExpiredPasswords();
  bool RestoreWindowsData(bool bUpdateWindows, bool bShow = true);
  void UpdateAccessTime(CItemData *pci);
  void RestoreGroupDisplayState();
  std::vector<bool> GetGroupDisplayState(); // get current display state from window
  void SetGroupDisplayState(const std::vector<bool> &displaystatus); // changes display
  void MakeOrderedItemList(OrderedItemList &il);
  int CountChildren(HTREEITEM hStartItem);
  void SetColumns();  // default order
  void SetColumns(const CString cs_ListColumns);
  void SetColumnWidths(const CString cs_ListColumnsWidths);
  void SetupColumnChooser(const bool bShowHide);
  void AddColumn(const int iType, const int iIndex);
  void DeleteColumn(const int iType);
  void RegistryAnonymity();
  void CustomiseMenu(CMenu *pPopupMenu, const UINT uiMenuID, const bool bDoShortcuts);
  void SetUpMenuStrings(CMenu *pPopupMenu);
  void SetUpInitialMenuStrings();
  void UpdateAccelTable();
  void DoBrowse(const bool bDoAutotype, const bool bSendEmail);
  void CopyDataToClipBoard(const CItemData::FieldType ft, const bool special = false);
  void UpdateSystemMenu();
  void RestoreWindows(); // extended ShowWindow(SW_RESTORE), sort of
  void RemoveFromGUI(CItemData &ci, LPARAM lparam = NULL);
  void AddToGUI(CItemData &ci);
  void RefreshEntryFieldInGUI(CItemData &ci, CItemData::FieldType ft);
  void RefreshEntryPasswordInGUI(CItemData &ci);
  void RebuildGUI();
  
  static const struct UICommandTableEntry {
    UINT ID;
    enum {InOpenRW=0, InOpenRO=1, InEmpty=2, InClosed=3, bOK_LAST};
    bool bOK[bOK_LAST];
  } m_UICommandTable[];

  // For efficiency & general coolness, we use a map
  // between the ID and the index in the above array
  // rather than a sequential search
  typedef std::map<UINT, int> MapUICommandTable;
  typedef MapUICommandTable::const_iterator MapUICommandTableConstIter;
  MapUICommandTable m_MapUICommandTable;

  // Images in List View
  bool m_bImageInLV;
  CInfoDisplay *m_pNotesDisplay;

  // Filters
  bool m_bFilterActive, m_bFilterForDelete, m_bFilterForStatus, m_bUnsavedDisplayed;
  // Current filter
  st_filters m_currentfilter;
  // Special Show Unsaved Changes filter
  st_filters m_showunsavedfilter;

  // Sorted Groups
  vfiltergroups m_vMflgroups;
  vfiltergroups m_vHflgroups;
  vfiltergroups m_vPflgroups;

  // Global Filters
  PWSFilters m_MapFilters;
  FilterPool m_currentfilterpool;
  CString m_selectedfiltername;

  void CreateGroups();
  int m_bNumPassedFiltering;

  PWSRun m_runner; // for executing external programs

  // Menu Shortcuts
  MapMenuShortcuts m_MapMenuShortcuts;

  // Mapping of virtual key to key name in user's locale
  MapKeyNameID m_MapKeyNameID;

  // Menu items we don't allow the user to modify or see in Options
  // Shortcuts CListCtrl
  // Any popup menu and Exit and Help + a few special one
  std::vector<UINT> m_ExcludedMenuItems;

  // These are the mnemonics to the highest level menu e.g. File, Edit etc.
  // Of form Alt+F, E, V, M, H - but using user's I18N letters.
  std::vector<st_MenuShortcut> m_ReservedShortcuts;

  bool m_bDoShortcuts[NUMPOPUPMENUS];

  // Used for DragBar Tooltips
  CToolTipCtrl* m_pToolTipCtrl;

  // Workstation Locked
  bool m_bWSLocked, m_bRegistered, m_bBlockShutdown;
  DWORD m_WindowsMajorVersion, m_WindowsMinorVersion;

  // Need this in case not running on Vista or later
  HMODULE m_hUser32;
  PSBR_CREATE m_pfcnShutdownBlockReasonCreate;
  PSBR_DESTROY m_pfcnShutdownBlockReasonDestroy;

  // The following is for saving information over an execute/undo/redo
  // Might need to add more e.g. if filter is active and which one?
  struct st_SaveGUIInfo {
    bool blSelectedValid, btSelectedValid, btGroupValid;
    uuid_array_t lSelected; // List selected item
    uuid_array_t tSelected; // Tree selected item
    StringX sxGroupName;
    std::vector<bool> vGroupDisplayState;

    st_SaveGUIInfo()
    : blSelectedValid(false), btSelectedValid(false), btGroupValid(false)
    {
      memset((void *)lSelected, 0, sizeof(uuid_array_t));
      memset((void *)tSelected, 0, sizeof(uuid_array_t));
    }

    st_SaveGUIInfo(const st_SaveGUIInfo &that)
    : blSelectedValid(that.blSelectedValid), btSelectedValid(that.btSelectedValid),
      btGroupValid(that.btGroupValid), vGroupDisplayState(that.vGroupDisplayState),
      sxGroupName(that.sxGroupName)
    {
      memcpy((void *)lSelected, (void *)that.lSelected, sizeof(uuid_array_t));
      memcpy((void *)tSelected, (void *)that.tSelected, sizeof(uuid_array_t));
    }

    st_SaveGUIInfo &operator=(const st_SaveGUIInfo &that)
    {
      if (this != &that) {
        blSelectedValid = that.blSelectedValid;
        btSelectedValid = that.blSelectedValid;
        btGroupValid = that.btGroupValid;
        memcpy((void *)lSelected, (void *)that.lSelected, sizeof(uuid_array_t));
        memcpy((void *)tSelected, (void *)that.tSelected, sizeof(uuid_array_t));
        sxGroupName = that.sxGroupName;
        vGroupDisplayState = that.vGroupDisplayState;
      }
      return *this;
    }
  };

  std::stack<st_SaveGUIInfo> m_stkSaveGUIInfo;
};

inline bool DboxMain::FieldsNotEqual(StringX a, StringX b)
{
  if (m_treatwhitespaceasempty == TRUE) {
    EmptyIfOnlyWhiteSpace(a);
    EmptyIfOnlyWhiteSpace(b);
  }
  return a != b;
}

// Following used to keep track of display vs data
// stored as opaque data in CItemData.{Get,Set}DisplayInfo()
// Exposed here because PWTreeCtrl needs to update it after drag&drop
struct DisplayInfo : public DisplayInfoBase {
  int list_index;
  HTREEITEM tree_item;

  DisplayInfo() {}
  virtual ~DisplayInfo() {}
  virtual DisplayInfo *clone() const // virtual c'tor idiom
  { return new DisplayInfo(*this); }
 
  DisplayInfo(const DisplayInfo &that)
  : list_index(that.list_index), tree_item(that.tree_item)
  {}

  DisplayInfo &operator=(const DisplayInfo &that)
  {
    if (this != &that) {
      list_index = that.list_index;
      tree_item = that.tree_item;
    }
    return *this;
  }
};

