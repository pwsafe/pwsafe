/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// DboxMain.h
//-----------------------------------------------------------------------------

#include "PWSclipboard.h"

#include "resource.h"
#include "resource2.h"  // Version, Menu, Toolbar & Accelerator resources
#include "resource3.h"  // String resources

#include "PWTreeCtrl.h"
#include "PWListCtrl.h"
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
#include "AdvancedDlg.h"
#include "FontsDialog.h"

#include "core/UIinterface.h"
#include "core/PWScore.h"
#include "core/StringX.h"
#include "core/sha256.h"
#include "core/PwsPlatform.h"
#include "core/PWSFilters.h"
#include "core/Command.h"
#include "core/RUEList.h"

#include "os/run.h"
#include "os/UUID.h"

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

// Custom message event used for system tray handling
#define PWS_MSG_ICON_NOTIFY             (WM_APP + 10)

// To catch post Header drag
#define PWS_MSG_HDR_DRAG_COMPLETE       (WM_APP + 20)
#define PWS_MSG_CCTOHDR_DD_COMPLETE     (WM_APP + 21)
#define PWS_MSG_HDRTOCC_DD_COMPLETE     (WM_APP + 22)

// Process Compare Result Dialog click/menu functions
#define PWS_MSG_COMPARE_RESULT_FUNCTION (WM_APP + 30)
#define PWS_MSG_COMPARE_RESULT_ALLFNCTN (WM_APP + 31)

// Equivalent one from Expired Password dialog
#define PWS_MSG_EXPIRED_PASSWORD_EDIT   (WM_APP + 32)

// Edit/Add extra context menu messages
#define PWS_MSG_CALL_EXTERNAL_EDITOR    (WM_APP + 40)
#define PWS_MSG_EXTERNAL_EDITOR_ENDED   (WM_APP + 41)
#define PWS_MSG_EDIT_WORDWRAP           (WM_APP + 42)
#define PWS_MSG_EDIT_SHOWNOTES          (WM_APP + 43)
#define PWS_MSG_EDIT_APPLY              (WM_APP + 44)
#define PWS_MSG_CALL_NOTESZOOMIN        (WM_APP + 45)
#define PWS_MSG_CALL_NOTESZOOMOUT       (WM_APP + 46)

// Simulate Ctrl+F from Find Toolbar "enter"
#define PWS_MSG_TOOLBAR_FIND            (WM_APP + 50)

// Perform Drag Autotype
#define PWS_MSG_DRAGAUTOTYPE            (WM_APP + 55)

// Update current filters whilst SetFilters dialog is open
#define PWS_MSG_EXECUTE_FILTERS         (WM_APP + 60)

// Notification from tree control that a file was dropped on it
#define PWS_MSG_DROPPED_FILE            (WM_APP + 65)

/* Message to get Virtual Keyboard buffer.  Here for doc. only. See VKeyBoardDlg.h
#define PWS_MSG_INSERTBUFFER            (WM_APP + 70)
#define PWS_MSG_RESETTIMER              (WM_APP + 71)
*/


/*
  Timer related values (note - all documented her but some defined only where needed.
*/

/* Timer event number used to by PupText.  Here for doc. only
#define TIMER_PUPTEXT             0x03 */
// Timer event number used to check if the workstation is locked
#define TIMER_LOCKONWTSLOCK       0x04
// Timer event number used to support lock on user-defined idle timeout
#define TIMER_LOCKDBONIDLETIMEOUT 0x05
// Definition of a minute in milliseconds
#define MINUTE 60000
// How ofter should idle timeout timer check:
#define IDLE_CHECK_RATE 2
#define IDLE_CHECK_INTERVAL (MINUTE/IDLE_CHECK_RATE)
// Timer event number used to support Find in PWListCtrl when icons visible
#define TIMER_FIND                0x06
// Timer event number used to support display of notes in List & Tree controls
#define TIMER_ND_HOVER            0x07
#define TIMER_ND_SHOWING          0x08
// Timer event number used to support DragBar
#define TIMER_DRAGBAR             0x09
/* Timer event numbers used to by ControlExtns for ListBox tooltips.  Here for doc. only
#define TIMER_LB_HOVER            0x0A
#define TIMER_LB_SHOWING          0x0B 
/* Timer event numbers used by StatusBar for tooltips.  Here for doc. only
#define TIMER_SB_HOVER            0x0C
#define TIMER_SB_SHOWING          0x0D */
// Timer event for daily expired entries check
#define TIMER_EXPENT              0x0E

/*
HOVER_TIME_ND       The length of time the pointer must remain stationary
                    within a tool's bounding rectangle before the tool tip
                    window appears.
*/
#define HOVER_TIME_ND      2000

/*
TIMEINT_ND_SHOWING The length of time the tool tip window remains visible
                   if the pointer is stationary within a tool's bounding
                   rectangle.
*/
#define TIMEINT_ND_SHOWING 5000

// DragBar time interval 
#define TIMER_DRAGBAR_TIME 100

// Hotkey value ID to maximum value allowed by Windows for an app.
#define PWS_HOTKEY_ID      0xBFFF

// Arbitrary string to mean that the saved DB preferences are empty.
#define EMPTYSAVEDDBPREFS L"#Empty#"

// Maximum number of characters that can be added to a MFC CEdit control
// by default (i.e. without calling SetLimitText). Note: 30000 not 32K!
// Although this limit can be changed to up to 2GB of characters
// (4GB memory if Unicode), it would make the database size absolutely enormous!
#define MAXTEXTCHARS       30000

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
enum {
      // Normal dialogs
      GCP_FIRST         = 0,   // At startup of PWS
      GCP_NORMAL        = 1,   // Only OK, CANCEL & HELP buttons
      GCP_RESTORE       = 2,   // Only OK, CANCEL & HELP buttons
      GCP_WITHEXIT      = 3,   // OK, CANCEL, EXIT & HELP buttons
      GCP_CHANGEMODE    = 4,   // Only OK, CANCEL & HELP buttons

      NUM_PER_ENVIRONMENT = 5,
};

// GCP read only flags - tested via AND, set via OR, must be power of 2.
enum {GCP_READONLY = 1,
      GCP_FORCEREADONLY = 2,
      GCP_HIDEREADONLY = 4};

class CDDObList;
class ExpiredList;
class CAddEdit_PropertySheet;
class CPasskeyEntry;

//-----------------------------------------------------------------------------
class DboxMain : public CDialog, public UIInterFace
{
public:
  DECLARE_DYNAMIC(DboxMain)

  // default constructor
  DboxMain(CWnd* pParent = NULL);
  ~DboxMain();

  enum SaveType {ST_INVALID = -1, ST_NORMALEXIT = 0, ST_SAVEIMMEDIATELY,
                 ST_ENDSESSIONEXIT, ST_WTSLOGOFFEXIT, ST_FAILSAFESAVE};

  // Find entry by title and user name, exact match
  ItemListIter Find(const StringX &a_group,
                    const StringX &a_title, const StringX &a_user)
  {return m_core.Find(a_group, a_title, a_user);}

  // Find entry with same title and user name as the
  // i'th entry in m_ctlItemList
  ItemListIter Find(int i);

  // Find entry by UUID
  ItemListIter Find(const pws_os::CUUID &uuid)
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
                 const CItemData::FieldBits &bsFields, 
                 const CItemAtt::AttFieldBits &bsAttFields, 
                 const bool &subgroup_set, const std::wstring &subgroup_name,
                 const int subgroup_object, const int subgroup_function);

  // Used by ListCtrl KeyDown
  bool IsImageVisible() const {return m_bImageInLV;}

  // Count the number of total entries.
  size_t GetNumEntries() const {return m_core.GetNumEntries();}

  // Has the GUI been built and has entries
  bool IsGUIEmpty() const { return m_ctlItemTree.GetCount() == 0;}

  // Get CItemData @ position
  CItemData &GetEntryAt(ItemListIter iter) const
  {return m_core.GetEntry(iter);}

  // For InsertItemIntoGUITreeList and RefreshViews (mainly when refreshing views)
  // Note: iBothViews = iListOnly + iTreeOnly
  enum ViewType {LISTONLY = 1, TREEONLY = 2, BOTHVIEWS = 3};

  void RefreshViews(const ViewType iView = BOTHVIEWS);

  // Set the section to the entry.  MakeVisible will scroll list, if needed.
  BOOL SelectEntry(const int i, BOOL MakeVisible = FALSE);
  BOOL SelectFindEntry(const int i, BOOL MakeVisible = FALSE);
  void SelectFirstEntry();

  int CheckPasskey(const StringX &filename, const StringX &passkey, PWScore *pcore = NULL);

  // We should not need this as the DB is only changed by executing a command
  // that affects a DB preference - see if can be removed from the functions
  // RestoreWindows, OnSize & OnColumnClick so that can be removed completely
  void SetDBPrefsChanged(const bool bState) {m_core.SetDBPrefsChanged(bState);}

  // These specific changed states are only needed when no other change has been made
  // AND the user has requested that:
  // 1. Maintain timestamps or
  // 2. Open database with the group display the same as when last saved
  // NOTE: The DB core is not informed of these changes as they are only used by the UI
  // to determine if the DB should be saved on close/exit if no other changes have been made
  void SetEntryTimestampsChanged(const bool bEntryTimestampsChanged)
  {m_bEntryTimestampsChanged = bEntryTimestampsChanged;}

  // This updates the Save menu/toolbar button depending if there are unsaved changes
  void ChangeOkUpdate();

  // when Group, Title or User edited in tree
  void UpdateListItemField(const int lindex, const int type, const StringX &sxnewText);
  void UpdateTreeItem(const HTREEITEM hItem, const CItemData &ci);
  void UpdateListItemGroup(const int lindex, const StringX &sxnewGroup)
  {UpdateListItemField(lindex, CItemData::GROUP, sxnewGroup);}
  void UpdateListItemTitle(const int lindex, const StringX &sxnewTitle)
  {UpdateListItemField(lindex, CItemData::TITLE, sxnewTitle);}
  void UpdateListItemUser(const int lindex, const StringX &sxnewUser)
  {UpdateListItemField(lindex, CItemData::USER, sxnewUser);}
  void UpdateListItemPassword(const int lindex, const StringX &sxnewPassword)
  {UpdateListItemField(lindex, CItemData::PASSWORD, sxnewPassword);}
  
  void SetHeaderInfo();
  CString GetHeaderText(int iType) const;
  int GetHeaderWidth(int iType) const;
  void CalcHeaderWidths();
  void UnFindItem();
  void SetLocalStrings();
  void PerformAutoType(); // 'public' version called by Tree/List

  void UpdateToolBarROStatus(const bool bIsRO);
  void UpdateToolBarForSelectedItem(const CItemData *pci);
  void SetToolBarPositions();
  void InvalidateSearch() {m_FindToolBar.InvalidateSearch();}
  void ResumeOnDBNotification() {m_core.ResumeOnDBNotification();}
  void SuspendOnDBNotification() {m_core.SuspendOnDBNotification();}
  bool GetDBNotificationState() {return m_core.GetDBNotificationState();}
  bool IsDBReadOnly() const {return m_core.IsReadOnly();}
  void SetDBprefsState(const bool bState) { m_bDBState = bState; }
  void SetStartSilent(bool state);
  void SetStartClosed(bool state) {m_IsStartClosed = state;}
  void SetDBInitiallyRO(bool state) {m_bDBInitiallyRO = state;}
  void MakeRandomPassword(StringX &password, PWPolicy &pwp, bool bIssueMsg = false);
  BOOL LaunchBrowser(const CString &csURL, const StringX &sxAutotype,
                     const std::vector<size_t> &vactionverboffsets,
                     const bool &bDoAutotype);
  BOOL SendEmail(const CString &cs_email);
  void SetInitialDatabaseDisplay();
  void U3ExitNow(); // called when U3AppStop sends message to Pwsafe Listener
  bool ExitRequested() const {return m_inExit;}
  void AutoResizeColumns();
  void ResetIdleLockCounter(UINT event = WM_SIZE); // default arg always resets
  bool ClearClipboardData() {return m_clipboard.ClearData();}
  bool SetClipboardData(const StringX &data)
  {return m_clipboard.SetData(data.c_str());}
  void AddDDEntries(CDDObList &in_oblist, const StringX &DropGroup,
    const std::vector<StringX> &vsxEmptyGroups);
  StringX GetUniqueTitle(const StringX &group, const StringX &title,
                         const StringX &user, const int IDS_MESSAGE) const
  {return m_core.GetUniqueTitle(group, title, user, IDS_MESSAGE);}
  void FixListIndexes();
  void Delete(MultiCommands *pmcmd); // "Top level" delete, calls the following 2 and Execute()
  Command *Delete(const CItemData *pci); // create command for deleting a single item
  // For deleting a group:
  void Delete(HTREEITEM ti,
              std::vector<Command *> &vbases,
              std::vector<Command *> &vdeps,
              std::vector<Command *> &vemptygrps,
              bool bExcludeTopGroup = false); 

  void SaveGroupDisplayState(); // call when tree expansion state changes
  void RestoreGUIStatusEx();
  void SaveGUIStatusEx(const ViewType iView);

  const CItemData *GetBaseEntry(const CItemData *pAliasOrSC) const
  {return m_core.GetBaseEntry(pAliasOrSC);}
  CItemData *GetBaseEntry(const CItemData *pAliasOrSC)
  {return m_core.GetBaseEntry(pAliasOrSC);}

  int GetEntryImage(const CItemData &ci) const;
  HICON GetEntryIcon(const int nImage) const;
  void SetEntryImage(const int &index, const int nImage, const bool bOneEntry = false);
  void SetEntryImage(HTREEITEM &ti, const int nImage, const bool bOneEntry = false);
  void UpdateEntryImages(const CItemData &ci);

  void RefreshImages();
  void CreateShortcutEntry(CItemData *pci, const StringX &cs_group,
                           const StringX &cs_title, const StringX &cs_user,
                           StringX &sxNewDBPrefsString);
  bool SetNotesWindow(const CPoint ptClient, const bool bVisible = true);
  bool IsFilterActive() const {return m_bFilterActive;}
  int GetNumPassedFiltering() const {return m_bNumPassedFiltering;}
  CItemData *GetLastSelected() const;
  StringX GetGroupName(const bool bFullPath = false) const;
  void UpdateGroupNamesInMap(const StringX sxOldPath, const StringX sxNewPath);
  void UpdateNotesTooltipFont();

  void SetFilter(FilterPool selectedpool, CString selectedfiltername)
  {m_currentfilterpool = selectedpool; m_selectedfiltername = selectedfiltername;}
  void ImportFilters();
  bool ApplyFilter(bool bJustDoIt = false);
  bool EditFilter(st_filters *pfilters, const bool &bAllowSet = true);
  void ClearFilter();
  void ExportFilters(PWSFilters &MapFilters);

  void DoAutoType(const StringX &sx_autotype, 
                  const std::vector<size_t> &vactionverboffsets);
  void UpdateLastClipboardAction(const int iaction);
  void PlaceWindow(CWnd *pWnd, CRect *pRect, UINT uiShowCmd);
  void SetDCAText(CItemData *pci = NULL);
  void OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult, const bool bTreeView);
  bool IsNodeModified(StringX &path) const
  {return m_core.IsNodeModified(path);}
  StringX GetCurFile() const {return m_core.GetCurFile();}

  bool EditItem(CItemData *pci, PWScore *pcore = NULL);
  bool GetPolicyFromName(const StringX &sxPolicyName, PWPolicy &st_pp)
  {return m_core.GetPolicyFromName(sxPolicyName, st_pp);}
  void GetPolicyNames(std::vector<std::wstring> &vNames)
  {m_core.GetPolicyNames(vNames);}
  const PSWDPolicyMap &GetPasswordPolicies()
  {return m_core.GetPasswordPolicies();}

  bool IsEmptyGroup(const StringX &sxEmptyGroup)
  {return m_core.IsEmptyGroup(sxEmptyGroup);}
  
  // Entry keyboard shortcuts
  const KBShortcutMap &GetAllKBShortcuts() const {return m_core.GetAllKBShortcuts();}

  // HashIters relaying
  uint32 GetHashIters() const {return m_core.GetHashIters();}
  void SetHashIters(uint32 value) {m_core.SetHashIters(value);}

  // Need this to be public
  bool LongPPs(CWnd *pWnd);

  bool GetShortCut(const unsigned int &uiMenuItem, unsigned short int &siVirtKey,
                   unsigned char &cModifier);

  // Following to simplify Command creation in child dialogs:
  CommandInterface *GetCore() {return &m_core;}
  
  void Execute(Command *pcmd, PWScore *pcore = NULL);
  void UpdateToolBarDoUndo();

  void ViewReport(const CString &cs_ReportFileName) const;
  void ViewReport(CReport &rpt) const;
  void SetUpdateWizardWindow(CWnd *pWnd)
  {m_pWZWnd = pWnd;}

  std::wstring DoMerge(PWScore *pothercore,
                  const bool bAdvanced, CReport *prpt, bool *pbCancel);
  bool DoCompare(PWScore *pothercore,
                 const bool bAdvanced, CReport *prpt, bool *pbCancel);
  void DoSynchronize(PWScore *pothercore,
                     const bool bAdvanced, int &numUpdated, CReport *prpt, bool *pbCancel);
  int DoExportText(const StringX &sx_Filename, const UINT nID,
                   const wchar_t &delimiter, const bool bAdvanced,
                   int &numExported, CReport *prpt);
  int DoExportXML(const StringX &sx_Filename, const UINT nID,
                  const wchar_t &delimiter, const bool bAdvanced,
                  int &numExported, CReport *prpt);

  int DoExportDB(const StringX &sx_Filename, const UINT nID,
                 const StringX &sx_ExportKey, int &numExported, CReport *prpt);

  int TestSelection(const bool bAdvanced,
                    const std::wstring &subgroup_name,
                    const int &subgroup_object,
                    const int &subgroup_function,
                    const OrderedItemList *pOIL) const
  {return m_core.TestSelection(bAdvanced, subgroup_name,
                               subgroup_object, subgroup_function, pOIL);}

  void MakeOrderedItemList(OrderedItemList &OIL, HTREEITEM hItem = NULL);
  bool MakeMatchingGTUSet(GTUSet &setGTU, const StringX &sxPolicyName) const
  {return m_core.InitialiseGTU(setGTU, sxPolicyName);}
  CItemData *getSelectedItem();
  void UpdateGUIDisplay();
  CString ShowCompareResults(const StringX sx_Filename1, const StringX sx_Filename2,
                             PWScore *pothercore, CReport *prpt);
  bool IsInRename() const {return m_bInRename;}
  bool IsInAddGroup() const {return m_bInAddGroup;}
  void ResetInAddGroup() {m_bInAddGroup = false;}

  //{{AFX_DATA(DboxMain)
  enum { IDD = IDD_PASSWORDSAFE_DIALOG };
  CPWListCtrl m_ctlItemList;
  CPWToolBar m_MainToolBar;   // main toolbar
  CPWTreeCtrl m_ctlItemTree;
  CImageList *m_pImageList;
  CImageList *m_pImageList0;
  CLVHdrCtrl m_LVHdrCtrl;
  CColumnChooserDlg *m_pCC;
  CPoint m_RCMousePos;
  CDDStatic m_DDGroup, m_DDTitle, m_DDUser, m_DDPassword, m_DDNotes, m_DDURL, m_DDemail,
            m_DDAutotype;
  //}}AFX_DATA

  CRUEList m_RUEList;   // recent entry lists

  wchar_t *m_eye_catcher;

  bool m_bDoAutoType;
  StringX m_sxAutoType;
  std::vector<size_t> m_vactionverboffsets;

  // Used in Add & Edit & PasswordPolicyDlg
  PWPolicy m_pwp;

  // Mapping Group to Tree Item to save searching all the time!
  // Be nice to have a bimap implementation
  std::map<StringX, HTREEITEM> m_mapGroupToTreeItem;
  std::map<HTREEITEM, StringX> m_mapTreeItemToGroup;
  void GetAllGroups(std::vector<std::wstring> &vGroups) const;

  // Process Special Shortcuts for Tree & List controls
  bool CheckPreTranslateDelete(MSG* pMsg);
  bool CheckPreTranslateRename(MSG* pMsg);
  bool CheckPreTranslateAutoType(MSG* pMsg);

  void SetSetup() {m_bSetup = true;}                     // called by app when '--setup' passed
  void NoValidation() {m_bNoValidation = true;}          // called by app when '--novalidate' passed
  void AllowCompareEntries() {m_bCompareEntries = true;} // called by app when '--cetreeview' passed

  // Needed public function for ComapreResultsDialog
  void CPRInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
  {OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);}

  const MapMenuShortcuts &GetMapMenuShortcuts() {return m_MapMenuShortcuts;}
  const std::vector<UINT> &GetExcludedMenuItems() {return m_ExcludedMenuItems;}
  const std::vector<st_MenuShortcut> &GetReservedShortcuts() {return m_ReservedShortcuts;}
  const unsigned int GetMenuShortcut(const unsigned short int &siVirtKey,
                                     const unsigned char &cModifier, StringX &sxMenuItemName);
  
  void ChangeMode(bool promptUser); // r-o <-> r/w

  // If we have processed it returns 0 else 1
  BOOL ProcessEntryShortcut(WORD &wVirtualKeyCode, WORD &wModifiers);
  bool IsWorkstationLocked() const;

  std::set<StringX> GetAllMediaTypes() const
  {return m_core.GetAllMediaTypes();}

 protected:
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(DboxMain)
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
   // override following to reset idle timeout on any event
   virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
   //}}AFX_VIRTUAL

  HICON m_hIcon;
  HICON m_hIconSm;

  // used to speed up the resizable dialog so OnSize/SIZE_RESTORED isn't called
  bool m_bSizing;
  bool m_bIsRestoring;
  bool m_bOpen;
  bool m_bInRestoreWindowsData;

  bool m_bSetup;          // invoked with '--setup'?
  bool m_bNoValidation;   // invoked with '--novalidate'?
  bool m_bCompareEntries; // invoked with '--cetreeview'?
  
  CString m_titlebar; // what's displayed in the title bar

  CPWFindToolBar m_FindToolBar;  // Find toolbar
  CPWStatusBar m_statusBar;
  BOOL m_toolbarsSetup;
  UINT m_toolbarMode;
  UINT statustext[CPWStatusBar::SB_TOTAL];
  CString m_lastclipboardaction;

  bool m_bInitDone;
  bool m_bDBNeedsReading;

  bool m_bSortAscending;
  int m_iTypeSortColumn;

  bool m_bDBState;
  bool m_bEntryTimestampsChanged;
  bool m_bGroupDisplayChanged;
  INT_PTR m_iSessionEndingStatus;

  // Used for Advanced functions
  CItemData::FieldBits m_bsFields;
  CString m_subgroup_name;
  int m_subgroup_set, m_subgroup_object, m_subgroup_function;
  int m_treatwhitespaceasempty;

  HTREEITEM m_LastFoundTreeItem;
  int m_LastFoundListItem;
  bool m_bBoldItem;

  WCHAR *m_pwchTip;
  
  StringX m_TreeViewGroup; // used by OnAdd & OnAddGroup
  CCoolMenuManager m_menuManager;
  CMenuTipManager m_menuTipManager;

  int InsertItemIntoGUITreeList(CItemData &itemData, int iIndex = -1, 
                 const bool bSort = true, const ViewType iView = BOTHVIEWS);

  BOOL SelItemOk();
  void setupBars();
  BOOL OpenOnInit();
  void InitPasswordSafe();

  // For UPDATE_UI
  int OnUpdateMenuToolbar(const UINT nID);
  int OnUpdateViewReports(const int nID);
  void OnUpdateMRU(CCmdUI* pCmdUI);

  void ConfigureSystemMenu();

  // 'STATE' also defined in ThisMfcApp.h - ensure identical
  enum STATE { LOCKED, UNLOCKED, CLOSED };
  void UpdateSystemTray(const STATE s);

  LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
  LRESULT OnCCToHdrDragComplete(WPARAM wParam, LPARAM lParam);
  LRESULT OnHdrToCCDragComplete(WPARAM wParam, LPARAM lParam);
  LRESULT OnHeaderDragComplete(WPARAM wParam, LPARAM lParam);
  LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
  LRESULT OnProcessCompareResultFunction(WPARAM wParam, LPARAM lParam);
  LRESULT OnProcessCompareResultAllFunction(WPARAM wParam, LPARAM lParam);
  LRESULT OnEditExpiredPasswordEntry(WPARAM wParam, LPARAM lParam);
  LRESULT ViewCompareResult(PWScore *pcore, const pws_os::CUUID &uuid);
  LRESULT EditCompareResult(PWScore *pcore, const pws_os::CUUID &uuid);
  LRESULT CopyCompareResult(PWScore *pfromcore, PWScore *ptocore,
                            const pws_os::CUUID &fromuuid, const pws_os::CUUID &touuid);
  LRESULT SynchCompareResult(PWScore *pfromcore, PWScore *ptocore,
                             const pws_os::CUUID &fromuuid, const pws_os::CUUID &touuid);
  LRESULT CopyAllCompareResult(WPARAM wParam);
  LRESULT SynchAllCompareResult(WPARAM wParam);
  LRESULT OnToolBarFindMessage(WPARAM wParam, LPARAM lParam);
  LRESULT OnDragAutoType(WPARAM wParam, LPARAM lParam);
  LRESULT OnExecuteFilters(WPARAM wParam, LPARAM lParam);
  LRESULT OnApplyEditChanges(WPARAM wParam, LPARAM lParam);
  LRESULT OnDroppedFile(WPARAM wParam, LPARAM lParam);

  BOOL PreTranslateMessage(MSG* pMsg);

  void UpdateAlwaysOnTop();
  void ClearData(const bool clearMRE = true);
  int NewFile(StringX &filename);

  void SetListView();
  void SetTreeView();
  void SetToolbar(const int menuItem, bool bInit = false);
  void UpdateStatusBar();
  void UpdateMenuAndToolBar(const bool bOpen);
  void SortListView();

  //Version of message functions with return values
  int Save(const SaveType savetype = DboxMain::ST_INVALID);
  int SaveAs(void);
  int Open(const UINT uiTitle = IDS_CHOOSEDATABASE);
  int Open(const StringX &sx_Filename, const bool bReadOnly, const bool bHideReadOnly = false);
  int CheckEmergencyBackupFiles(StringX sx_Filename, StringX &passkey);
  void PostOpenProcessing();
  int Close(const bool bTrySave = true);

  void ReportAdvancedOptions(CReport *prpt, const bool bAdvanced, const WZAdvanced::AdvType type);

  int BackupSafe(void);
  int RestoreSafe(void);
  int New(void);

  void AutoType(const CItemData &ci);
  void UpdateEntry(CAddEdit_PropertySheet *pentry_psh);
  bool EditShortcut(CItemData *pci, PWScore *pcore = NULL);
  void SetFindToolBar(bool bShow);
  void ApplyFilters();
  
  void GetMonitorRect(HWND hwnd, RECT *prc, BOOL fWork);
  void ClipRectToMonitor(HWND hwnd, RECT *prc, BOOL fWork);

  void SaveGUIStatus();
  void RestoreGUIStatus();

  void ChangeFont(const CFontsDialog::FontType iType);

  // Generated message map functions
  //{{AFX_MSG(DboxMain)
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
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
  afx_msg void OnTraySelect(UINT nID);
  afx_msg void OnUpdateTraySelect(CCmdUI *pCmdUI);

  afx_msg LRESULT OnAreYouMe(WPARAM, LPARAM);
  afx_msg LRESULT OnWH_SHELL_CallBack(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnSessionChange(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnQueryEndSession(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnEndSession(WPARAM wParam, LPARAM lParam);
 
  afx_msg void OnHelp();
  afx_msg void OnUpdateMenuToolbar(CCmdUI *pCmdUI);
  afx_msg void OnDestroy();
  afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
  afx_msg void OnMove(int x, int y);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnAbout();
  afx_msg void OnPasswordSafeWebsite();
  afx_msg void OnBrowse();
  afx_msg void OnBrowsePlus();
  afx_msg void OnSendEmail();
  afx_msg void OnCopyUsername();
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnListItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnTreeItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnKeydownItemlist(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemDoubleClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHeaderRClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHeaderNotify(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHeaderBeginDrag(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnHeaderEndDrag(NMHDR *pNotifyStruct, LRESULT *pLResult);
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
  afx_msg void OnChangeMode();
  afx_msg void OnProperties();
  afx_msg void OnRestoreSafe();
  afx_msg void OnSaveAs();
  afx_msg void OnToggleView();
  afx_msg void OnListView();
  afx_msg void OnTreeView();
  afx_msg void OnBackupSafe();
  afx_msg void OnPassphraseChange();
  afx_msg void OnClearClipboard();
  afx_msg void OnDelete();
  afx_msg void OnEdit();
  afx_msg void OnRename();
  afx_msg void OnDuplicateEntry();
  afx_msg void OnOptions();
  afx_msg void OnManagePasswordPolicies();
  afx_msg void OnGeneratePassword();
  afx_msg void OnYubikey();
  afx_msg void OnSave();
  afx_msg void OnAdd();
  afx_msg void OnAddGroup();
  afx_msg void OnDuplicateGroup();
  afx_msg void OnProtect(UINT nID);
  afx_msg void OnCompareEntries();
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
  afx_msg void OnChangeNotesFont();
  afx_msg void OnChangeVKFont();
  afx_msg void OnChangeAddEditFont();
  afx_msg void OnViewReportsByID(UINT nID);  // From View->Reports menu
  afx_msg void OnViewReports();
  afx_msg void OnManageFilters(); // From Toolbar button
  afx_msg void OnCancelFilter();
  afx_msg void OnApplyFilter();
  afx_msg void OnSetFilter();
  afx_msg void OnRefreshWindow();
  afx_msg void OnShowUnsavedEntries();
  afx_msg void OnShowExpireList();
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
  afx_msg void OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnUpdateNSCommand(CCmdUI *pCmdUI);  // Make entry unsupported (grayed out)
  afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
  afx_msg void OnSizing(UINT fwSide, LPRECT pRect);

  afx_msg BOOL OnToolTipText(UINT, NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnExportVx(UINT nID);

  afx_msg void OnExportText();
  afx_msg void OnExportXML();
  afx_msg void OnExportEntryText();
  afx_msg void OnExportEntryXML();
  afx_msg void OnExportEntryDB();
  afx_msg void OnExportGroupText();
  afx_msg void OnExportGroupXML();
  afx_msg void OnExportGroupDB();
  afx_msg void OnExportAttachment();
  afx_msg void OnImportText();
  afx_msg void OnImportKeePassV1CSV();
  afx_msg void OnImportKeePassV1TXT();
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
  afx_msg BOOL OnOpenMRU(UINT nID);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  int GetAndCheckPassword(const StringX &filename, StringX& passkey,
                          int index, int flags = 0,
                          PWScore *pcore = NULL);

private:
  // UIInterFace implementations:
  virtual void DatabaseModified(bool bChanged);
  virtual void UpdateGUI(UpdateGUICommand::GUI_Action ga,
                         const pws_os::CUUID &entry_uuid,
                         CItemData::FieldType ft, bool bUpdateGUI);
  virtual void GUISetupDisplayInfo(CItemData &ci);
  virtual void GUIRefreshEntry(const CItemData &ci);
  virtual void UpdateWizard(const std::wstring &s);

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  // Only need these if not on default menus
  static CString CS_SETFILTERS, CS_CLEARFILTERS,  CS_READWRITE, CS_READONLY;

  StringX m_BrowseURL; // set by OnContextMenu(), used by OnBrowse()
  PWScore &m_core;

  CPasskeyEntry *m_pPasskeyEntryDlg;

  bool m_IsStartSilent;
  bool m_IsStartClosed;
  bool m_bStartHiddenAndMinimized;
  bool m_IsListView;
  bool m_bAlreadyToldUserNoSave;
  bool m_bPasswordColumnShowing;
  bool m_bInRefresh, m_bInRestoreWindows;
  bool m_bDBInitiallyRO;
  bool m_bViaDCA;
  int m_iDateTimeFieldWidth;
  int m_nColumns;
  int m_nColumnIndexByOrder[CItem::LAST_DATA];
  int m_nColumnIndexByType[CItem::LAST_DATA];
  int m_nColumnTypeByIndex[CItem::LAST_DATA];
  int m_nColumnWidthByIndex[CItem::LAST_DATA];
  int m_nColumnHeaderWidthByType[CItem::LAST_DATA];
  int m_iheadermaxwidth;

  pws_os::CUUID m_LUUIDSelectedAtMinimize; // to restore List entry selection upon un-minimize
  pws_os::CUUID m_TUUIDSelectedAtMinimize; // to restore Tree entry selection upon un-minimize
  StringX m_sxSelectedGroup;               // to restore Tree group selection upon un-minimize
  pws_os::CUUID m_LUUIDVisibleAtMinimize;  // to restore List entry position  upon un-minimize
  pws_os::CUUID m_TUUIDVisibleAtMinimize;  // to restore Tree entry position  upon un-minimize
  StringX m_sxVisibleGroup;                // to restore Tree group position  upon un-minimize

  StringX m_sxOriginalGroup;                 // Needed when doing recursive deletions of groups

  bool m_inExit; // help U3ExitNow
  std::vector<bool> m_vGroupDisplayState; // used to save/restore display state over minimize/restore
  StringX m_savedDBprefs;                 // used across minimize/restore events

  PWSclipboard m_clipboard;

  // Split up OnOK to support various ways to exit
  int SaveDatabaseOnExit(const SaveType saveType);
  void SavePreferencesOnExit();
  void CleanUpAndExit(const bool bNormalExit = true);

  void RegisterSessionNotification(const bool bRegister);
  bool LockDataBase();
  void startLockCheckTimer();
  UINT m_IdleLockCountDown;
  void SetIdleLockCounter(UINT iMinutes); // set to timer counts
  bool DecrementAndTestIdleLockCounter();
  int SaveIfChanged();
  int SaveImmediately();
  void CheckExpireList(const bool bAtOpen = false); // Upon open, timer + menu, check list, show exp.
  void TellUserAboutExpiredPasswords();
  bool RestoreWindowsData(bool bUpdateWindows, bool bShow = true);
  void UpdateAccessTime(const pws_os::CUUID &uuid);
  void RestoreGroupDisplayState();
  std::vector<bool> GetGroupDisplayState(); // get current display state from window
  void SetGroupDisplayState(const std::vector<bool> &displaystatus); // changes display
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
  void SetupSpecialShortcuts();
  bool ProcessLanguageMenu(CMenu *pPopupMenu);
  void DoBrowse(const bool bDoAutotype, const bool bSendEmail);
  bool GetSubtreeEntriesProtectedStatus(int &numProtected, int &numUnprotected);
  void ChangeSubtreeEntriesProtectStatus(const UINT nID);
  void CopyDataToClipBoard(const CItemData::FieldType ft, const bool bSpecial = false);
  void UpdateSystemMenu();
  void RestoreWindows(); // extended ShowWindow(SW_RESTORE), sort of
  void CancelPendingPasswordDialog();

  void RemoveFromGUI(CItemData &ci, bool bUpdateGUI);
  void AddToGUI(CItemData &ci);
  void RefreshEntryFieldInGUI(CItemData &ci, CItemData::FieldType ft);
  void RefreshEntryPasswordInGUI(CItemData &ci);
  void RebuildGUI(const ViewType iView = BOTHVIEWS);
  void UpdateEntryinGUI(CItemData &ci);
  StringX GetListViewItemText(CItemData &ci, const int &icolumn);
  
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
  bool m_bFilterActive, m_bUnsavedDisplayed, m_bExpireDisplayed;
  // Current filter
  st_filters &CurrentFilter() {return m_FilterManager.m_currentfilter;}

  // Global Filters
  PWSFilterManager m_FilterManager;
  PWSFilters m_MapFilters;
  FilterPool m_currentfilterpool;
  CString m_selectedfiltername;

  int m_bNumPassedFiltering;

  PWSRun m_runner; // for executing external programs

  // Menu Shortcuts
  MapMenuShortcuts m_MapMenuShortcuts;

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
  bool m_bWSLocked, m_bWTSRegistered, m_bBlockShutdown;

  // Need this in case not running on Vista or later
  HMODULE m_hUser32;
  PSBR_CREATE m_pfcnShutdownBlockReasonCreate;
  PSBR_DESTROY m_pfcnShutdownBlockReasonDestroy;

  // Delete/Rename/AutoType Shortcuts
  WPARAM m_wpDeleteMsg, m_wpDeleteKey;
  WPARAM m_wpRenameMsg, m_wpRenameKey;
  WPARAM m_wpAutotypeUPMsg, m_wpAutotypeDNMsg, m_wpAutotypeKey;
  bool m_bDeleteCtrl, m_bDeleteShift;
  bool m_bRenameCtrl, m_bRenameShift;
  bool m_bAutotypeCtrl, m_bAutotypeShift;

  // Do Autotype
  bool m_bInAT;
  std::bitset<3> m_btAT;  // Representing the Key, Ctrl Key and Shift key

  // Save Advanced settings
  st_SaveAdvValues m_SaveWZAdvValues[WZAdvanced::LAST];
  st_SaveAdvValues m_SaveAdvValues[CAdvancedDlg::LAST];

  // Used to update static text on the Wizard for Compare, Merge etc.
  CWnd *m_pWZWnd;

  // Compare vectors - when comparing databases
  CompareData m_list_OnlyInCurrent;
  CompareData m_list_OnlyInComp;
  CompareData m_list_Conflicts;
  CompareData m_list_Identical;

  // Flag to tell user there are some expired entries
  bool m_bTellUserExpired;

  // Prevent rename of entries in Tree mode by clicking on entry
  bool m_bInRename;
  // When in AddGroup and where AddGroup initiated
  bool m_bInAddGroup, m_bWhitespaceRightClick;

  // Change languages on the fly
  void SetLanguage(LCID lcid);
  int m_ilastaction;  // Last action
  void SetDragbarToolTips();

  // The following is for saving information over an execute/undo/redo
  // Might need to add more e.g. if filter is active and which one?
  struct st_SaveGUIInfo {
    bool blSelectedValid, btSelectedValid, btGroupValid;
    pws_os::CUUID lSelected; // List selected item
    pws_os::CUUID tSelected; // Tree selected item
    StringX sxGroupName;
    std::vector<bool> vGroupDisplayState;

    st_SaveGUIInfo()
    : blSelectedValid(false), btSelectedValid(false), btGroupValid(false),
      lSelected(pws_os::CUUID::NullUUID()), tSelected(pws_os::CUUID::NullUUID()) {}

    st_SaveGUIInfo(const st_SaveGUIInfo &that)
    : blSelectedValid(that.blSelectedValid), btSelectedValid(that.btSelectedValid),
      btGroupValid(that.btGroupValid), vGroupDisplayState(that.vGroupDisplayState),
      lSelected(that.lSelected), tSelected(that.tSelected),
      sxGroupName(that.sxGroupName) {}

    st_SaveGUIInfo &operator=(const st_SaveGUIInfo &that)
    {
      if (this != &that) {
        blSelectedValid = that.blSelectedValid;
        btSelectedValid = that.blSelectedValid;
        btGroupValid = that.btGroupValid;
        lSelected = that.lSelected;
        tSelected = that.tSelected;
        sxGroupName = that.sxGroupName;
        vGroupDisplayState = that.vGroupDisplayState;
      }
      return *this;
    }
  };

  std::stack<st_SaveGUIInfo> m_stkSaveGUIInfo;
};

// Following used to keep track of display vs data
// stored as opaque data in CItemData.{Get,Set}DisplayInfo()
// Exposed here because PWTreeCtrl needs to update it after drag&drop
struct DisplayInfo : public DisplayInfoBase {
  int list_index;
  HTREEITEM tree_item;

 DisplayInfo() :list_index(-1), tree_item(0) {}
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
