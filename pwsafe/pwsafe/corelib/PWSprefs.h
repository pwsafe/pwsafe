/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "MyString.h"
#include "PWSfile.h"

extern HANDLE s_cfglockFileHandle;
extern int s_cfgLockCount;

class CXMLprefs;

class PWSprefs {
 public:
  static PWSprefs *GetInstance(); // singleton
  static void DeleteInstance();
  static void SetConfigFile(const CString &fn) {m_configfilename = fn;}
  // prefString is stored on file, format described in PWSprefs.cpp
  void Load(const CMyString &prefString);
  CMyString Store(); // returns string for saving in file

  void SaveApplicationPreferences();

  enum  BoolPrefs {AlwaysOnTop, ShowPWDefault,
       ShowPasswordInTree,
       SortAscending,
		   UseDefaultUser, SaveImmediately, PWUseLowercase, PWUseUppercase,
		   PWUseDigits, PWUseSymbols, PWUseHexDigits, PWUseEasyVision,
		   DontAskQuestion, DeleteQuestion, DCShowsPassword,
		   DontAskMinimizeClearYesNo, DatabaseClear,
       DontAskSaveMinimize, // Obsoleted in 3.02
       QuerySetDef, UseNewToolbar, UseSystemTray, 
		   LockOnWindowLock, LockOnIdleTimeout,
		   EscExits, IsUTF8, HotKeyEnabled, MRUOnFileMenu,
		   DisplayExpandedAddEditDlg, MaintainDateTimeStamps,
		   SavePasswordHistory, 
       FindWraps, // Obsoleted in 3.11
       ShowNotesDefault,
		   BackupBeforeEverySave, PreExpiryWarn,
       ExplorerTypeTree, ListViewGridLines, MinimizeOnAutotype,
       ShowUsernameInTree, PWMakePronounceable,
		   NumBoolPrefs};
  enum  IntPrefs {Column1Width, Column2Width, Column3Width, Column4Width,
		  SortedColumn, PWDefaultLength, MaxMRUItems, IdleTimeout,
		  DoubleClickAction, HotKey, MaxREItems, TreeDisplayStatusAtOpen,
		  NumPWHistoryDefault, BackupSuffix, BackupMaxIncremented,
		  PreExpiryWarnDays, ClosedTrayIconColour, PWDigitMinLength,
		  PWLowercaseMinLength, PWSymbolMinLength, PWUppercaseMinLength,
      NumIntPrefs};
  enum  StringPrefs {CurrentBackup, CurrentFile, LastView, DefaultUsername,
		  TreeFont, BackupPrefixValue, BackupDir, AltBrowser, ListColumns,
      ColumnWidths, DefaultAutotypeString, AltBrowserCmdLineParms,
      MainToolBarButtons, PasswordFont,
		  NumStringPrefs};

  // for DoubleClickAction
  enum {minDCA = 0, DoubleClickCopyPassword = 0, DoubleClickViewEdit = 1,
      DoubleClickAutoType = 2, DoubleClickBrowse = 3, 
      DoubleClickCopyNotes = 4, DoubleClickCopyUsername = 5,
      maxDCA = 5};

  // for TreeDisplayStatusAtOpen
  enum {minTDS = 0, AllCollapsed = 0, AllExpanded = 1, AsPerLastSave = 2,
	  maxTDS = 2};

  // for Backup Mask
  enum {minBKSFX = 0, BKSFX_None = 0, BKSFX_DateTime = 1, BKSFX_IncNumber = 2,
	  maxBKSFX = 2};

  // for System Tray icon color
  enum {stiBlack = 0, stiBlue = 1, stiWhite = 2, stiYellow = 3};

  // For Password Polict
  enum {PWPolicyUseLowercase =      0x8000, // Can have a minimum length field
        PWPolicyUseUppercase =      0x4000, // Can have a minimum length field
        PWPolicyUseDigits =         0x2000, // Can have a minimum length field
        PWPolicyUseSymbols =        0x1000, // Can have a minimum length field
        PWPolicyUseHexDigits =      0x0800,
        PWPolicyUseEasyVision =     0x0400,
        PWPolicyMakePronounceable = 0x0200,
        PWPolicyUnused            = 0x01ff};

  bool IsDBprefsChanged() const {return m_prefs_changed[DB_PREF];}
  bool IsAPPprefsChanged() const {return m_prefs_changed[APP_PREF];}
  void ClearDBprefsChanged() {m_prefs_changed[DB_PREF] = false;}
  void ClearAPPprefsChanged() {m_prefs_changed[APP_PREF] = false;}
  void SetDatabasePrefsToDefaults();

  bool GetPref(BoolPrefs pref_enum) const;
  unsigned int GetPref(IntPrefs pref_enum) const;
  // Following for case where default value is determined @ runtime
  unsigned int GetPref(IntPrefs pref_enum, unsigned int defVal) const;
  CMyString GetPref(StringPrefs pref_enum) const;

  // Special cases
  void GetPrefRect(long &top, long &bottom, long &left, long &right) const;
  void SetPrefRect(long top, long bottom, long left, long right);
  int GetMRUList(CString *MRUFiles);
  int SetMRUList(const CString *MRUFiles, int n, int max_MRU);

  void SetPref(BoolPrefs pref_enum, bool value);
  void SetPref(IntPrefs pref_enum, unsigned int value);
  void SetPref(StringPrefs pref_enum, const CMyString &value);

  // CPSWRecentFileList needs to know if it can use registry or not:
  bool IsUsingRegistry() const {return m_ConfigOptions == CF_REGISTRY;}

  // Get database preferences in XML format for export
  CString GetXMLPreferences();

  // for display in status bar (debug)
  int GetConfigIndicator() const;

  // for OptionSystem property sheet - support removing registry traces
  bool OfferDeleteRegistry() const;
  void DeleteRegistryEntries();  

  static bool LockCFGFile(const CMyString &filename, CMyString &locker)
    {return PWSfile::LockFile(filename, locker, 
                         s_cfglockFileHandle, s_cfgLockCount);}
  static void UnlockCFGFile(const CMyString &filename)
    {return PWSfile::UnlockFile(filename,
                         s_cfglockFileHandle, s_cfgLockCount);}
  static bool IsLockedCFGFile(const CMyString &filename)
    {return PWSfile::IsLockedFile(filename);}

 private:
  PWSprefs();
  ~PWSprefs();
  
  // Preferences changed (Database or Application)
  enum {DB_PREF = 0, APP_PREF = 1};
  
  bool WritePref(const CMyString &name, bool val);
  bool WritePref(const CMyString &name, unsigned int val);
  bool WritePref(const CMyString &name, const CMyString &val);
  void UpdateTimeStamp();
  bool DeletePref(const CMyString &name);
  void InitializePreferences();
  void LoadProfileFromDefaults();
  bool LoadProfileFromFile();
  void LoadProfileFromRegistry();
  bool CheckRegistryExists() const;

  // Handle old (pre-3.05 registry-based) prefs.
  void ImportOldPrefs();
  bool OldPrefsExist() const;
  void DeleteOldPrefs();
  
  static PWSprefs *self; // singleton
  static CString m_configfilename; // may be set before singleton created
  CXMLprefs *m_XML_Config;

  bool m_bRegistryKeyExists;
  enum {CF_NONE, CF_REGISTRY, CF_FILE_RO,
        CF_FILE_RW, CF_FILE_RW_NEW} m_ConfigOptions;
  CString m_csHKCU, m_csHKCU_MRU, m_csHKCU_POS, m_csHKCU_PREF;

  CWinApp *m_app;

  bool m_prefs_changed[2];  // 0 - DB stored pref; 1 - App related pref

  // below, isStoredinDB means stored in db, !isStoredinDB means application related
  static const struct boolPref {
    TCHAR *name; bool defVal; bool isStoredinDB;} m_bool_prefs[NumBoolPrefs];
  static const struct intPref {
    TCHAR *name; unsigned int defVal; bool isStoredinDB; int minVal; int maxVal;} m_int_prefs[NumIntPrefs];
  static const struct stringPref {
    TCHAR *name; TCHAR *defVal; bool isStoredinDB;} m_string_prefs[NumStringPrefs];

  // current values
  bool m_boolValues[NumBoolPrefs];
  unsigned int m_intValues[NumIntPrefs];
  CMyString m_stringValues[NumStringPrefs];
  struct {long top, bottom, left, right; bool changed;} m_rect;
  bool m_boolChanged[NumBoolPrefs];
  bool m_intChanged[NumIntPrefs];
  bool m_stringChanged[NumStringPrefs];

  CString *m_MRUitems;
};
#endif /*  __PWSPREFS_H */
