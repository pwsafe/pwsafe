/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWSPREFS_H
#define __PWSPREFS_H

/*
  THIS IS A COPY OF "A SUBSET" OF THE PREFERENCES OUT OF CORE'S PWSPREFS.CPP

  THIS IS NEEDED "ONLY" TO GET THE PREFERENCE "NAME"

  COPY TAKEN V3.34.2 - LAST CHANGED 2014-07-25
    SHA-1: 6086623d66adcae1c959097118f438fc3662d3cc
*/

class PWSprefs
{
public:

  enum ConfigOption {CF_NONE = 0, CF_REGISTRY,
                     CF_FILE_RO, CF_FILE_RW, CF_FILE_RW_NEW};

  enum BoolPrefs {AlwaysOnTop, ShowPWDefault,
    ShowPasswordInTree,
    SortAscending,
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
    UsePrimarySelectionForClipboard,  //Only under X-Windows
    CopyPasswordWhenBrowseToURL,
    UseAltAutoType,  //Only under X-Windows
    NumBoolPrefs};

  enum IntPrefs {Column1Width, Column2Width, Column3Width, Column4Width,
    SortedColumn, PWDefaultLength, MaxMRUItems, IdleTimeout,
    DoubleClickAction, HotKey, MaxREItems, TreeDisplayStatusAtOpen,
    NumPWHistoryDefault, BackupSuffix, BackupMaxIncremented,
    PreExpiryWarnDays, ClosedTrayIconColour, PWDigitMinLength,
    PWLowercaseMinLength, PWSymbolMinLength, PWUppercaseMinLength,
    OptShortcutColumnWidth, ShiftDoubleClickAction, DefaultAutotypeDelay,
    NumIntPrefs};

  enum StringPrefs {CurrentBackup, CurrentFile, LastView, DefaultUsername,
    TreeFont, BackupPrefixValue, BackupDir, AltBrowser, ListColumns,
    ColumnWidths, DefaultAutotypeString, AltBrowserCmdLineParms,
    MainToolBarButtons, PasswordFont, TreeListSampleText, PswdSampleText,
    LastUsedKeyboard, VKeyboardFontName, VKSampleText, AltNotesEditor,
    LanguageFile, DefaultSymbols,
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

  // for System Tray icon color
  enum {stiBlack = 0, stiBlue = 1, stiWhite = 2, stiYellow = 3};

  // Preference types - values are powers of 2, except ptAll = sum of previous values
  enum PrefType {ptObsolete = 0, ptDatabase = 1, ptApplication = 2, ptAll = 3};

  static const struct boolPref {
    const TCHAR *name; bool defVal; PrefType ptype;} bool_prefs[NumBoolPrefs];
  static const struct intPref {
    const TCHAR *name; unsigned int defVal; PrefType ptype; int minVal; int maxVal;} int_prefs[NumIntPrefs];
  static const struct stringPref {
    const TCHAR *name; const TCHAR *defVal; PrefType ptype;} string_prefs[NumStringPrefs];

  static const TCHAR *stringTypes[];
  static const TCHAR *stringDisplay[];
  static const TCHAR *stringCfgLoc[];
};
#endif /*  __PWSPREFS_H */
