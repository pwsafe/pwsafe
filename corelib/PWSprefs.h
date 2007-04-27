/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
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

class CXMLprefs;

class PWSprefs {
 public:
  static PWSprefs *GetInstance(); // singleton
  static void DeleteInstance();

  // prefString is stored on file, format described in PWSprefs.cpp
  void Load(const CMyString &prefString);
  CMyString Store(); // returns string for saving in file

  void SaveApplicationPreferences();

  enum  BoolPrefs {AlwaysOnTop, ShowPWDefault, ShowPWInList, SortAscending,
		   UseDefUser, SaveImmediately, PWUseLowercase, PWUseUppercase,
		   PWUseDigits, PWUseSymbols, PWUseHexDigits, PWEasyVision,
		   DontAskQuestion, DeleteQuestion, DCShowsPassword,
		   DontAskMinimizeClearYesNo, DatabaseClear,
       DontAskSaveMinimize, // Obsoleted in 3.02
       QuerySetDef, UseNewToolbar, UseSystemTray, 
		   LockOnWindowLock, LockOnIdleTimeout,
		   EscExits, IsUTF8, HotKeyEnabled, MRUOnFileMenu,
		   DisplayExpandedAddEditDlg, MaintainDateTimeStamps,
		   SavePasswordHistory, FindWraps, ShowNotesDefault,
		   BackupBeforeEverySave, PreExpiryWarn,
       ExplorerTypeTree, ListViewGridLines, MinimizeOnAutotype,
		   NumBoolPrefs};
  enum  IntPrefs {Column1Width, Column2Width, Column3Width, Column4Width,
		  SortedColumn, PWLenDefault, MaxMRUItems, IdleTimeout,
		  DoubleClickAction, HotKey, MaxREItems, TreeDisplayStatusAtOpen,
		  NumPWHistoryDefault, BackupSuffix, BackupMaxIncremented,
		  PreExpiryWarnDays,
      NumIntPrefs};
  enum  StringPrefs {CurrentBackup, CurrentFile, LastView, DefUserName,
		  TreeFont, BackupPrefixValue, BackupDir, AltBrowser, ListColumns,
      ColumnWidths, DefaultAutotypeString, AltBrowserCmdLineParms,
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

  // for display in status bar (debug)
  int GetConfigIndicator() const;

  // for OptionSystem property sheet - support removing registry traces
  bool OfferDeleteRegistry() const;
  void DeleteRegistryEntries();  
  
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
  CXMLprefs *m_XML_Config;

  CString m_configfilename;
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
