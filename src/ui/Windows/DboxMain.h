/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "SystemTray.h"

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

// TODO: Remove once winver support increased
#ifndef _DPI_AWARENESS_CONTEXTS_
#define _DPI_AWARENESS_CONTEXTS_
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#endif

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#endif

// For ShutdownBlockReasonCreate & ShutdownBlockReasonDestroy
typedef BOOL (WINAPI *PSBR_CREATE) (HWND, LPCWSTR);
typedef BOOL (WINAPI *PSBR_DESTROY) (HWND);
typedef DPI_AWARENESS_CONTEXT (WINAPI *PSBR_DPIAWARE) (DPI_AWARENESS_CONTEXT);

// Entry to GUI mapping
// Following used to keep track of display vs data
// stored as opaque data in m_MapEntryToGUI
// Exposed here because PWTreeCtrl needs to update it after drag&drop
struct DisplayInfo {
  int list_index;
  HTREEITEM tree_item;

  DisplayInfo() :list_index(-1), tree_item(0) {}
  ~DisplayInfo() {}

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
class DboxMain : public CDialog, public Observer
{
public:
  DECLARE_DYNAMIC(DboxMain)

  DboxMain(PWScore &core, CWnd* pParent = NULL);
  ~DboxMain();

  // To enable DPI awareness
  virtual INT_PTR DoModal();

  enum SaveType {ST_INVALID = -1, ST_NORMALEXIT = 0, ST_SAVEIMMEDIATELY,
                 ST_ENDSESSIONEXIT, ST_WTSLOGOFFEXIT, ST_FAILSAFESAVE};

  enum DBSTATE { LOCKED, UNLOCKED, CLOSED };

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
                 std::vector<int> &vIndices, std::vector<pws_os::CUUID> &vFoundUUIDs);
  size_t FindAll(const CString &str, BOOL CaseSensitive,
                 std::vector<int> &vIndices,
                 std::vector<pws_os::CUUID> &vFoundUUIDs,
                 const CItemData::FieldBits &bsFields, 
                 const CItemAtt::AttFieldBits &bsAttFields, 
                 const bool &subgroup_set, const std::wstring &subgroup_name,
                 const int subgroup_object, const int subgroup_function);

  void SetFilterFindEntries(std::vector<pws_os::CUUID> *pvFoundUUIDs);

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
  enum ViewType { NONE = 0, LISTONLY = 1, TREEONLY = 2, BOTHVIEWS = 3 };

  void RefreshViews(const ViewType iView = BOTHVIEWS);

  // Set the section to the entry.  MakeVisible will scroll list, if needed.
  BOOL SelectEntry(const int i, BOOL MakeVisible = FALSE);
  BOOL SelectFindEntry(const int i, BOOL MakeVisible = FALSE);
  void SelectFirstEntry();

  int CheckPasskey(const StringX &filename, const StringX &passkey, PWScore *pcore = NULL);

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
  
  void SetHeaderInfo(const bool bSetWidths = true);
  void RestoreColumnWidths();
  void SaveColumnWidths();
  void GetHeaderColumnProperties(const int &iType, CString &csText, int &iWidth,
    int &iSortColumn);
  void CalcHeaderWidths();
  void UnFindItem();
  void SetLocalStrings();
  void PerformAutoType(); // 'public' version called by Tree/List

  void UpdateToolBarROStatus(const bool bIsRO);
  void UpdateToolBarForSelectedItem(const CItemData *pci);
  void SetToolBarPositions();
  void ResumeOnDBNotification() {m_core.ResumeOnDBNotification();}
  void SuspendOnDBNotification() {m_core.SuspendOnDBNotification();}
  bool GetDBNotificationState() {return m_core.GetDBNotificationState();}
  bool IsDBReadOnly() const {return m_core.IsReadOnly();}
  bool IsDBOpen() const { return m_bOpen; }
  void SetDBprefsState(const bool bState) { m_bDBState = bState; }
  void SetStartSilent() {m_InitMode = SilentInit;} // start minimized, forces UseSystemTray
  void SetStartClosed() {m_InitMode = ClosedInit;} // start with main window, no password prompt
  void SetStartMinimized() {m_InitMode = MinimizedInit;} // Like closed, but also minimized
  void SetStartNoDB() {m_IsStartNoDB = true;} // start with no db, w/o password prompt
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
  bool ClearClipboardData() {return m_clipboard.ClearCBData();}
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

  void SaveGroupDisplayState(const bool bClear = false); // call when tree expansion state changes
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
  void UpdateEntryImages(const CItemData &ci, bool bAllowFail = false);

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
                 const bool bExportDBFilters,
                 const StringX &sx_ExportKey, int &numExported, CReport *prpt);

  int TestSelection(const bool bAdvanced,
                    const std::wstring &subgroup_name,
                    const int &subgroup_object,
                    const int &subgroup_function,
                    const OrderedItemList *pOIL) const
  {return m_core.TestSelection(bAdvanced, subgroup_name,
                               subgroup_object, subgroup_function, pOIL);}

  std::vector<pws_os::CUUID> MakeOrderedItemList(OrderedItemList &OIL, HTREEITEM hItem = NULL);
  bool MakeMatchingGTUSet(GTUSet &setGTU, const StringX &sxPolicyName) const
  {return m_core.InitialiseGTU(setGTU, sxPolicyName);}
  CItemData *getSelectedItem();
  void GetSelectedItems(pws_os::CUUID &entry_uuid,
                        pws_os::CUUID &tree_find_entry_uuid, pws_os::CUUID &list_find_entry_uuid,
                        StringX &sxGroupPath);
  void ReSelectItems(pws_os::CUUID entry_uuid,
                     pws_os::CUUID &tree_find_entry_uuid, pws_os::CUUID &list_find_entry_uuid,
                     StringX sxGroupPath);
  void UpdateGUIDisplay();
  CString ShowCompareResults(const StringX sx_Filename1, const StringX sx_Filename2,
                             PWScore *pothercore, CReport *prpt);
  bool IsInRename() const {return m_bInRename;}
  bool IsInAddGroup() const {return m_bInAddGroup;}
  void ResetInAddGroup() {m_bInAddGroup = false;}
  void SetRenameGroups(const StringX sxNewPath)
  { m_sxNewPath = sxNewPath; }

  int GetDBIndex() { return m_iDBIndex; }
  COLORREF GetLockedIndexColour() { return m_DBLockedIndexColour; }
  COLORREF GetUnlockedIndexColour() { return m_DBUnlockedIndexColour; }

  DBSTATE GetSystemTrayState() const { return m_TrayLockedState; }
  BOOL IsIconVisible() const { return m_pTrayIcon->Visible(); }

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

  // Get link between entry and GUI
  DisplayInfo *GetEntryGUIInfo(const CItemData &ci, bool bAllowFail = false);

  // Mapping Group to Tree Item to save searching all the time!
  // Be nice to have a bitmap implementation
  std::map<StringX, HTREEITEM> m_mapGroupToTreeItem;
  std::map<HTREEITEM, StringX> m_mapTreeItemToGroup;
  void GetAllGroups(std::vector<std::wstring> &vGroups) const;

  // Process Special Shortcuts for Tree & List controls
  bool CheckPreTranslateDelete(MSG* pMsg);
  bool CheckPreTranslateRename(MSG* pMsg);
  bool CheckPreTranslateAutoType(MSG* pMsg);

  void SetSetup() {m_bSetup = true;}                     // called when '--setup' passed
  void NoValidation() {m_bNoValidation = true;}          // called when '--novalidate' passed
  void AllowCompareEntries() {m_bCompareEntries = true;} // called when '--cetreeview' passed

  // Needed public function for ComapreResultsDialog
  void CPRInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
  {OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);}

  const MapMenuShortcuts &GetMapMenuShortcuts() {return m_MapMenuShortcuts;}
  const std::vector<UINT> &GetExcludedMenuItems() {return m_ExcludedMenuItems;}
  const std::vector<st_MenuShortcut> &GetReservedShortcuts() {return m_ReservedShortcuts;}
  const unsigned int GetMenuShortcut(const unsigned short int &siVirtKey,
                                     const unsigned char &cModifier, StringX &sxMenuItemName);
  
  bool ChangeMode(bool promptUser); // r-o <-> r/w

  // If we have processed it returns 0 else 1
  BOOL ProcessEntryShortcut(WORD &wVirtualKeyCode, WORD &wWinModifiers);
  bool IsWorkstationLocked() const;
  void BlockLogoffShutdown(const bool bChanged);

  std::set<StringX> GetAllMediaTypes() const
  {return m_core.GetAllMediaTypes();}

  // For latered Windows
  PSLWA GetSetLayeredWindowAttributes() { return m_pfcnSetLayeredWindowAttributes; }
  bool GetInitialTransparencyState() { return m_bOnStartupTransparancyEnabled; }
  bool SetLayered(CWnd *pWnd, const int value = -1);
  void SetThreadDpiAwarenessContext();

 protected:
   friend class CSetDBID;  // To access icon creation etc.

   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(DboxMain)
   virtual BOOL PreTranslateMessage(MSG *pMsg);
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   virtual void DoDataExchange(CDataExchange *pDX);  // DDX/DDV support
   // override following to reset idle timeout on any event
   virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
   //}}AFX_VIRTUAL

  HICON m_hIcon;
  HICON m_hIconSm;

  // used to speed up the resizable dialog so OnSize/SIZE_RESTORED isn't called
  bool m_bSizing;
  bool m_bIsRestoring;

  // Is DB open
  bool m_bOpen;

  // Other bool
  bool m_bInRestoreWindowsData;
  bool m_bUserDeclinedSave;
  bool m_bRestoredDBUnsaved;
  bool m_bSuspendGUIUpdates;
  ViewType m_iNeedRefresh;

  bool m_bSetup;          // invoked with '--setup'?
  bool m_bNoValidation;   // invoked with '--novalidate'?
  bool m_bCompareEntries; // invoked with '--cetreeview'?
  
  CString m_titlebar; // what's displayed in the title bar

  CPWFindToolBar m_FindToolBar;  // Find toolbar
  CPWStatusBar m_StatusBar;
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
  int m_LastFoundListItem, m_iCurrentItemFound;
  bool m_bBoldItem;
  bool m_bFindToolBarVisibleAtLock;

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
  void OnUpdateMRU(CCmdUI *pCmdUI);

  void ConfigureSystemMenu();

  void UpdateSystemTray(const DBSTATE s);
  BOOL SetTooltipText(LPCWSTR ttt) { return m_pTrayIcon->SetTooltipText(ttt); }
  void ShowIcon() { m_pTrayIcon->ShowIcon(); }
  void HideIcon() { m_pTrayIcon->HideIcon(); }

  void SetSystemTrayState(DBSTATE s);
  void SetSystemTrayTarget(CWnd *pWnd) { m_pTrayIcon->SetTarget(pWnd); }

  HICON CreateIcon(const HICON &hIcon, const int &iIndex,
                   const COLORREF clrText = RGB(255, 255, 0));

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

  void UpdateAlwaysOnTop();
  void ClearAppData(const bool bClearMRE = true);
  int NewFile(StringX &filename);

  void SetListView();
  void SetTreeView();
  void SetToolbar(const int menuItem, bool bInit = false);
  void UpdateStatusBar();
  void UpdateMenuAndToolBar(const bool bOpen);
  void SortListView();

  // Version of message functions with return values
  int Save(const SaveType savetype = DboxMain::ST_INVALID);
  int SaveAs();
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

  void SetupUserFonts();
  void ChangeFont(const CFontsDialog::FontType iType);

  // Generated message map functions
  //{{AFX_MSG(DboxMain)
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnTrayLockUnLock();
  afx_msg void OnTrayClearRecentEntries();
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
  afx_msg void OnGotoDependant(UINT nID);
  afx_msg void OnUpdateGotoDependant(CCmdUI *pCmdUI);

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
  afx_msg void OnSetDBID();
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
  afx_msg void OnExportFilteredDB();
  afx_msg void OnCancelFilter();
  afx_msg void OnApplyFilter();
  afx_msg void OnSetFilter();
  afx_msg void OnRefreshWindow();
  afx_msg void OnShowUnsavedEntries();
  afx_msg void OnShowExpireList();
  afx_msg void OnShowFoundEntries();
  afx_msg void OnMinimize();
  afx_msg void OnRestore();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnAutoType();
  afx_msg void OnGotoBaseEntry();
  afx_msg void OnEditBaseEntry();
  afx_msg void OnViewAttachment();
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
  afx_msg void OnShowFindToolbar();
  afx_msg void OnHideFindToolbar();

  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  afx_msg BOOL OnOpenMRU(UINT nID);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  int GetAndCheckPassword(const StringX &filename, StringX& passkey,
                          int index, int flags = 0);

private:
  enum InitType {NormalInit, SilentInit, ClosedInit, MinimizedInit};

  // Observer interface implementations:
  virtual void DatabaseModified(bool bChanged);
  virtual void UpdateGUI(UpdateGUICommand::GUI_Action ga,
                         const pws_os::CUUID &entry_uuid,
                         CItemData::FieldType ft);
  // Version for groups
  virtual void UpdateGUI(UpdateGUICommand::GUI_Action ga,
                         const std::vector<StringX> &vGroups);
  
  virtual void GUIRefreshEntry(const CItemData &ci, bool bAllowFail = false);
  virtual void UpdateWizard(const std::wstring &s);

  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  // Only need these if not on default menus
  static CString CS_SETFILTERS, CS_CLEARFILTERS,  CS_READWRITE, CS_READONLY;

  StringX m_BrowseURL; // set by OnContextMenu(), used by OnBrowse()
  PWScore &m_core;

  CPasskeyEntry *m_pPasskeyEntryDlg;

  CSystemTray *m_pTrayIcon; // DboxMain needs to be constructed first
  DBSTATE m_TrayLockedState;

  HICON m_LockedIcon;
  HICON m_UnLockedIcon;
  HICON m_ClosedIcon;
  HICON m_IndexIcon;

  InitType m_InitMode = NormalInit;
  bool m_IsStartNoDB;
  bool m_IsListView;
  bool m_bAlreadyToldUserNoSave;
  bool m_bPasswordColumnShowing;
  bool m_bInRefresh, m_bInRestoreWindows;
  bool m_bDBInitiallyRO;
  bool m_bViaDCA;
  bool m_bFindBarShown;
  int m_iDateTimeFieldWidth;
  int m_nColumns;
  int m_nColumnIndexByOrder[CItem::LAST_DATA];
  int m_nColumnIndexByType[CItem::LAST_DATA];
  int m_nColumnTypeByIndex[CItem::LAST_DATA];
  int m_nColumnWidthByIndex[CItem::LAST_DATA];
  int m_nColumnHeaderWidthByType[CItem::LAST_DATA];
  int m_nSaveColumnHeaderWidthByType[CItem::LAST_DATA];
  int m_iheadermaxwidth;
  int m_iListHBarPos, m_iTreeHBarPos;

  pws_os::CUUID m_LUUIDSelectedAtMinimize; // to restore List entry selection upon un-minimize
  pws_os::CUUID m_TUUIDSelectedAtMinimize; // to restore Tree entry selection upon un-minimize
  StringX m_sxSelectedGroup;               // to restore Tree group selection upon un-minimize
  pws_os::CUUID m_LUUIDVisibleAtMinimize;  // to restore List entry position  upon un-minimize
  pws_os::CUUID m_TUUIDVisibleAtMinimize;  // to restore Tree entry position  upon un-minimize
  StringX m_sxVisibleGroup;                // to restore Tree group position  upon un-minimize
 
  // Here lies the mapping between an entry and its place on the GUI (Tree/List views)
  std::map<pws_os::CUUID, DisplayInfo, std::less<pws_os::CUUID> > m_MapEntryToGUI;

  // Mapping between visible dependants and their base (might not be visible if filter active)
  std::vector<int> m_vGotoDependants;
  
  // Set link between entry and GUI
  void SetEntryGUIInfo(const CItemData &ci, const DisplayInfo &di)
  { m_MapEntryToGUI[ci.GetUUID()] = di; } // often used to update, so map::insert() is inappropriate

  // Used in SaveGUIStatus to remember position during rename group - set by CPWTreeCtrl
  StringX m_sxNewPath;

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
  void SetDefaultColumns();  // default order
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
  void UpdateEditViewAccelerator(bool isRO);
  bool ProcessLanguageMenu(CMenu *pPopupMenu);
  void DoBrowse(const bool bDoAutotype, const bool bSendEmail);
  bool GetSubtreeEntriesProtectedStatus(int &numProtected, int &numUnprotected);
  void ChangeSubtreeEntriesProtectStatus(const UINT nID);
  void CopyDataToClipBoard(const CItemData::FieldType ft, const bool bSpecial = false);
  void RestoreWindows(); // extended ShowWindow(SW_RESTORE), sort of
  void CancelPendingPasswordDialog();

  void RemoveFromGUI(CItemData &ci);
  void AddToGUI(CItemData &ci);
  void RefreshEntryFieldInGUI(CItemData &ci, CItemData::FieldType ft);
  void RefreshEntryPasswordInGUI(CItemData &ci);
  void RebuildGUI(const ViewType iView = BOTHVIEWS);
  void UpdateEntryInGUI(CItemData &ci);
  void UpdateGroupsInGUI(const std::vector<StringX> &vGroups);
  StringX GetListViewItemText(CItemData &ci, const int &icolumn);
  void DoCommand(Command *pcmd = NULL, PWScore *pcore = NULL, const bool bUndo = true);
  
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
  bool m_bFilterActive, m_bUnsavedDisplayed, m_bExpireDisplayed, m_bFindFilterDisplayed;
  // Current filter
  st_filters &CurrentFilter() {return m_FilterManager.m_currentfilter;}

  // Global Filters
  PWSFilterManager m_FilterManager;
  PWSFilters m_MapAllFilters;  // Includes DB and temporary (added, imported, autoloaded etc.)
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

  // For Layered Windows
  PSLWA m_pfcnSetLayeredWindowAttributes;

  // Delete/Rename/AutoType Shortcuts
  WPARAM m_wpDeleteMsg, m_wpDeleteKey;
  WPARAM m_wpRenameMsg, m_wpRenameKey;
  WPARAM m_wpAutotypeUPMsg, m_wpAutotypeDNMsg, m_wpAutotypeKey;
  bool m_bDeleteCtrl, m_bDeleteShift;
  bool m_bRenameCtrl, m_bRenameShift;
  bool m_bAutotypeCtrl, m_bAutotypeShift;
  bool m_bOnStartupTransparancyEnabled;

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
  // When Wizard dialog is active
  bool m_bWizardActive;

  // Change languages on the fly
  void SetLanguage(LCID lcid);
  int m_ilastaction;  // Last action
  void SetDragbarToolTips();

  // Database index on Tray icon
  int m_iDBIndex;
  COLORREF m_DBLockedIndexColour, m_DBUnlockedIndexColour;
  HANDLE m_hMutexDBIndex;

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

inline DisplayInfo *DboxMain::GetEntryGUIInfo(const CItemData &ci,
                                              bool bAllowFail)
{
  auto E2G_iter = m_MapEntryToGUI.find(ci.GetUUID());
  if (E2G_iter != m_MapEntryToGUI.end()) {
    return &E2G_iter->second;
  }

  if (!bAllowFail) {
    ASSERT(0); // caller expected the find to succeed
  }
  return nullptr;  
}
