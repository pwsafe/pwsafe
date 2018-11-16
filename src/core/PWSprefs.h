/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSPREFS_H
#define __PWSPREFS_H

// PWSprefs.h
//-----------------------------------------------------------------------------

/*
* A class to abstract away the persistent storage mechanism used to store and
* retrieve user preferences. Pre-2.03 implementations used the Windows
* registry. People have asked for preferences to be stored along with the
* database, so that the same preferences can be shared across computers
* (e.g., using disk-on-key).
*
* Starting with v3.05, preferences have been partitioned into two types:
* per-database and 'application'. Per-database preferences,
* as the name implies, are stored in the database. 'Application' preferences
* are stored in a separate configuration file.
* For more details, see 'config.txt' in the docs subdirectory.
*
* IMPORTANT: When adding a new preference, the new enum MUST be before last,
* that is, right before the Num*Prefs enum. This is because the prefs are
* identified in storage by their type and index.
*/

#include "StringX.h"
#include "Proxy.h"
#include "os/typedefs.h"

#include <vector>

extern HANDLE s_cfglockFileHandle;
extern int s_cfgLockCount;

struct st_prefShortcut {
  unsigned int id;
  unsigned short int siVirtKey;
  unsigned char cPWSModifier;
  stringT Menu_Name;
};

// Bool preferences unknown to this version
struct st_BP {
  int index;
  bool bValue;
};

// Integer preferences unknown to this version
struct st_IP {
  int index;
  int iValue;
};

// String preferences unknown to this version
struct st_SP {
  int index;
  TCHAR delim;
  StringX sValue;
};

class CXMLprefs;
struct PWPolicy;

class PWSprefs
{
public:
  enum ConfigOption {CF_NONE = 0, CF_REGISTRY,
                     CF_FILE_RO, CF_FILE_RW, CF_FILE_RW_NEW};

  static const stringT cfgFileName; // one place for the config filename

  static PWSprefs *GetInstance(); // singleton
  static void DeleteInstance();
  static bool SetConfigFile(const stringT &fn);
  static stringT GetConfigFile(ConfigOption &configoption)
  {configoption = m_ConfigOption; return m_configfilename;}
  static void SetReporter(Reporter *pReporter) {m_pReporter = pReporter;}
  static void XMLify(charT t, stringT &name);
  static bool UserSetCfgFile() {return m_userSetCfgFile;}
  ConfigOption GetConfigOption() {return m_ConfigOption;}

  static stringT GetDCAdescription(int dca);

  // prefString is stored in database file, format described in PWSprefs.cpp
  void Load(const StringX &prefString, bool bUseCopy = false);
  StringX Store(bool bUseCopy = false); // returns string for saving in file

  void SaveApplicationPreferences();
  void SaveShortcuts();

  enum BoolPrefs {AlwaysOnTop, ShowPWDefault,
    ShowPasswordInTree,
    SortAscending, // Obsoleted in 3.40 as moved to application config file
    UseDefaultUser, SaveImmediately, PWUseLowercase, PWUseUppercase,
    PWUseDigits, PWUseSymbols, PWUseHexDigits, PWUseEasyVision,
    DontAskQuestion, DeleteQuestion, DCShowsPassword,
    DontAskMinimizeClearYesNo, // Obsoleted in 3.13 - replaced by 2 separate
    DatabaseClear,
    DontAskSaveMinimize, // Obsoleted in 3.02
    QuerySetDef, UseNewToolbar, UseSystemTray,
    LockOnWindowLock,
    LockOnIdleTimeout, // Obsoleted in 3.19 - replaced by Database equivalent
    EscExits, IsUTF8, HotKeyEnabled, MRUOnFileMenu,
    DisplayExpandedAddEditDlg, // Obsoleted in 3.18
    MaintainDateTimeStamps,
    SavePasswordHistory,
    FindWraps, // Obsoleted in 3.11
    ShowNotesDefault,
    BackupBeforeEverySave, PreExpiryWarn,
    ExplorerTypeTree, ListViewGridLines, MinimizeOnAutotype,
    ShowUsernameInTree, PWMakePronounceable,
    ClearClipoardOnMinimize, ClearClipoardOneExit, // Both obsoleted in 3.14 - typos
    ShowToolbar, ShowNotesAsTooltipsInViews, DefaultOpenRO,
    MultipleInstances, ShowDragbar,
    ClearClipboardOnMinimize, ClearClipboardOnExit,
    ShowFindToolBarOnOpen, NotesWordWrap, LockDBOnIdleTimeout,
    HighlightChanges, HideSystemTray,
    UsePrimarySelectionForClipboard,  // Only under X-Windows
    CopyPasswordWhenBrowseToURL,
    UseAltAutoType,  // Only under X-Windows
    IgnoreHelpLoadError, // Only under WX
    VKPlaySound, // Windows only
    ListSortAscending,
    EnableWindowTransparency,
    NumBoolPrefs};

  enum IntPrefs {Column1Width, Column2Width, Column3Width, Column4Width,
    SortedColumn, PWDefaultLength, MaxMRUItems, IdleTimeout,
    DoubleClickAction, HotKey, MaxREItems, TreeDisplayStatusAtOpen,
    NumPWHistoryDefault, BackupSuffix, BackupMaxIncremented,
    PreExpiryWarnDays, ClosedTrayIconColour, PWDigitMinLength,
    PWLowercaseMinLength, PWSymbolMinLength, PWUppercaseMinLength,
    OptShortcutColumnWidth, ShiftDoubleClickAction, DefaultAutotypeDelay,
    DlgOrientation, TimedTaskChainDelay,
    AutotypeSelectAllKeyCode, AutotypeSelectAllModMask, //X only
    TreeFontPtSz, PasswordFontPtSz, NotesFontPtSz, AddEditFontPtSz, VKFontPtSz,
    WindowTransparency, DefaultExpiryDays,
    NumIntPrefs};

  enum StringPrefs {CurrentBackup, CurrentFile, LastView, DefaultUsername,
    TreeFont, BackupPrefixValue, BackupDir, AltBrowser, ListColumns,
    ColumnWidths, DefaultAutotypeString, AltBrowserCmdLineParms,
    MainToolBarButtons, PasswordFont, TreeListSampleText, PswdSampleText,
    LastUsedKeyboard, VKeyboardFontName, VKSampleText, AltNotesEditor,
    LanguageFile, DefaultSymbols, NotesFont, NotesSampleText, AutotypeTaskDelays,
    AddEditFont, AddEditSampleText, AltNotesEditorCmdLineParms,
    NumStringPrefs};

  // for DoubleClickAction and ShiftDoubleClickAction
  // NOTE: When adding items, update the pwsafe.xsd & pwsafe_filter.xsd schemas
  //       to increase the maximum value in "dcaType"
  enum {minDCA = 0, DoubleClickCopyPassword = 0, DoubleClickViewEdit = 1,
    DoubleClickAutoType = 2, DoubleClickBrowse = 3,
    DoubleClickCopyNotes = 4, DoubleClickCopyUsername = 5,
    DoubleClickCopyPasswordMinimize = 6,
    DoubleClickBrowsePlus = 7, DoubleClickRun = 8,
    DoubleClickSendEmail = 9,
    maxDCA = 9};

  // for TreeDisplayStatusAtOpen
  enum {minTDS = 0, AllCollapsed = 0, AllExpanded = 1, AsPerLastSave = 2,
    maxTDS = 2};

  // for Backup Mask
  enum {minBKSFX = 0, BKSFX_None = 0, BKSFX_DateTime = 1, BKSFX_IncNumber = 2,
    maxBKSFX = 2};

  // For Password Policy
  // Preferences changed (Database or Application or Shortcuts)
  enum {DB_PREF = 0, APP_PREF = 1, SHC_PREF = 2};

  // Dialog orientation: Determine automatically or let the system decide
  enum  {AUTO = 0, TALL = 1, WIDE = 2};

  // Preference types - values are powers of 2, except ptAll = sum of previous values
  enum PrefType {ptObsolete = 0, ptDatabase = 1, ptApplication = 2, ptAll = 3};

  bool IsDBprefsChanged() const {return m_prefs_changed[DB_PREF];}
  bool IsAPPprefsChanged() const {return m_prefs_changed[APP_PREF];}
  void ClearDBprefsChanged() {m_prefs_changed[DB_PREF] = false;}
  void ClearAPPprefsChanged() {m_prefs_changed[APP_PREF] = false;}
  void SetDBprefsChanged(const bool bChanged) {m_prefs_changed[DB_PREF] = bChanged;}
  void SetDatabasePrefsToDefaults(const bool bUseCopy = false);
  void ForceWriteApplicationPreferences()
  {m_prefs_changed[APP_PREF] = true; m_prefs_changed[SHC_PREF] = true;}

  bool GetPref(BoolPrefs pref_enum, const bool bUseCopy = false) const;
  unsigned int GetPref(IntPrefs pref_enum, const bool bUseCopy = false) const;
  StringX GetPref(StringPrefs pref_enum, const bool bUseCopy = false) const;

  // Following is for case where default value is determined at runtime
  //  Note: last parameter cannot be defaulted and must be specified in any
  //  call to distinguish it from the normal integer GetPref
  unsigned int GetPref(IntPrefs pref_enum, unsigned int defVal,
                       const bool bUseCopy) const;

  bool GetPrefDefVal(BoolPrefs pref_enum) const;
  unsigned int GetPrefDefVal(IntPrefs pref_enum) const;
  StringX GetPrefDefVal(StringPrefs pref_enum) const;

  int GetPrefMinVal(IntPrefs pref_enum) const;
  int GetPrefMaxVal(IntPrefs pref_enum) const;

  // Get all preferences for minidump user stream
  StringX GetAllBoolPrefs(const bool bUseCopy = false);
  StringX GetAllIntPrefs(const bool bUseCopy = false);
  StringX GetAllStringPrefs(const bool bUseCopy = false);

  // Special cases
  void GetPrefRect(long &top, long &bottom, long &left, long &right) const;
  void SetPrefRect(long top, long bottom, long left, long right);
  void GetPrefPSSRect(long &top, long &bottom, long &left, long &right) const;
  void SetPrefPSSRect(long top, long bottom, long left, long right);
  int GetMRUList(stringT *MRUFiles) const;
  int SetMRUList(const stringT *MRUFiles, int n, int max_MRU);
  PWPolicy GetDefaultPolicy(const bool bUseCopy = false) const;
  void SetDefaultPolicy(const PWPolicy &pol, const bool bUseCopy = false);

  void SetupCopyPrefs();
  void UpdateFromCopyPrefs(const PWSprefs::PrefType ptype);
  void SetPref(BoolPrefs pref_enum, bool value, const bool bUseCopy = false);
  void SetPref(IntPrefs pref_enum, unsigned int value, const bool bUseCopy = false);
  void SetPref(StringPrefs pref_enum, const StringX &value, const bool bUseCopy = false);

  void ResetPref(BoolPrefs pref_enum);
  void ResetPref(IntPrefs pref_enum);
  void ResetPref(StringPrefs pref_enum);

  // CPSWRecentFileList needs to know if it can use registry or not:
  bool IsUsingRegistry() const {return m_ConfigOption == CF_REGISTRY;}

  // Get database preferences in XML format for export
  stringT GetXMLPreferences();

  // for display in status bar (debug)
  int GetConfigIndicator() const;

  // Get & set vector of user shortcuts (only in XML cnfig file)
  std::vector<st_prefShortcut> GetPrefShortcuts() const {return m_vShortcuts;}
  void SetPrefShortcuts(const std::vector<st_prefShortcut> &vShortcuts);

  // for OptionSystem property sheet - support removing registry traces
  bool OfferDeleteRegistry() const;
  void DeleteRegistryEntries();

  // Default User information from supplied DB preference string
  void GetDefaultUserInfo(const StringX &sxDBPreferences,
                          bool &bIsDefUserSet, StringX &sxDefUserValue);

  static bool LockCFGFile(const stringT &filename, stringT &locker);
  static void UnlockCFGFile(const stringT &filename);
  static bool IsLockedCFGFile(const stringT &filename);

  void ClearUnknownPrefs(); // Clear unknown preferences vectors

private:
  PWSprefs();
  ~PWSprefs();

  bool WritePref(const StringX &name, bool val);
  bool WritePref(const StringX &name, unsigned int val);
  bool WritePref(const StringX &name, const StringX &val);
  void UpdateTimeStamp();
  bool DeletePref(const StringX &name);
  void InitializePreferences();
  void LoadProfileFromDefaults();
  bool LoadProfileFromFile();
  void LoadProfileFromRegistry();
  bool CheckRegistryExists() const;
  void FindConfigFile();

  // Handle old (pre-3.05 registry-based) prefs.
  void ImportOldPrefs();
  bool OldPrefsExist() const;
  void DeleteOldPrefs();

  static PWSprefs *self; // singleton
  static stringT m_configfilename; // may be set before singleton created
  static Reporter *m_pReporter; // set as soon as possible to show errors
  static bool m_userSetCfgFile;
  CXMLprefs *m_pXML_Config;

  bool m_bRegistryKeyExists;
  static ConfigOption m_ConfigOption;
  stringT m_csHKCU, m_csHKCU_MRU, m_csHKCU_POS, m_csHKCU_PREF, m_csHKCU_SHCT;

  bool m_prefs_changed[3];  // 0 - DB stored pref; 1 - App related pref; 2 - Shortcut

  static const struct boolPref {
    const TCHAR *name; bool defVal; PrefType ptype;} m_bool_prefs[NumBoolPrefs];
  static const struct intPref {
    const TCHAR *name; unsigned int defVal; PrefType ptype; int minVal; int maxVal;} m_int_prefs[NumIntPrefs];
  static const struct stringPref {
    const TCHAR *name; const TCHAR *defVal; PrefType ptype;} m_string_prefs[NumStringPrefs];

  // current values
  bool m_boolValues[NumBoolPrefs];
  unsigned int m_intValues[NumIntPrefs];
  StringX m_stringValues[NumStringPrefs];
  struct {long top, bottom, left, right; bool changed;} m_rect, m_PSSrect;
  bool m_boolChanged[NumBoolPrefs];
  bool m_intChanged[NumIntPrefs];
  bool m_stringChanged[NumStringPrefs];

  // COPIES of current values - used to generate DB preference string for database
  // without actually updating the preferences
  bool m_boolCopyValues[NumBoolPrefs];
  unsigned int m_intCopyValues[NumIntPrefs];
  StringX m_stringCopyValues[NumStringPrefs];

  stringT *m_MRUitems;
  std::vector<st_prefShortcut> m_vShortcuts;

  // Preferences we don't know in this version of PWS
  std::vector<st_BP> m_vUnknownBPrefs;
  std::vector<st_IP> m_vUnknownIPrefs;
  std::vector<st_SP> m_vUnknownSPrefs;
};
#endif /*  __PWSPREFS_H */
